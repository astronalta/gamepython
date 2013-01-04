/*
 * Copyright (c) 2010 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#include "Builder.hpp"
#include "CodeExpander.hpp"
#include "IrGenerator.hpp"
#include "RegisterAllocator.hpp"
#include "Nasm64Generator.hpp"
#include "Intel64Generator.hpp"
#include "CCodeGenerator.hpp"
#include "IrBlockPrinter.hpp"
#include "DeadCodeEliminator.hpp"
#include "CopyPropagator.hpp"
#include "TreePrinter.hpp"
#include "InterfaceGenerator.hpp"
#include "SemanticAnalyzer.hpp"
#include "Parser.hpp"
#include "OutputFormat.hpp"
#if defined(DARWIN)
#include "Mach64Output.hpp"
#elif defined(WINDOWS)
#elif defined(LINUX
#endif

#include <cstdlib>

Builder::Builder(Environment* env) :
    env_(env),
    errors_(0) {

    // Initialize default includes to be used if the user-defined includes
    // do not point to the Jogo standard libraries.
#ifndef WINDOWS
    env->include("/usr/local/lib");
    env->include("/usr/local/include/jogo");
#else
    std::string program_files = getenv("PROGRAMFILES");
    std::string program_files_x86 = getenv("PROGRAMFILES(x86)");
    env->include(program_files + "\\Jogo\\lib");
    env->include(program_files_x86 + "\\Jogo\\lib");
    env->include(program_files + "\\Jogo\\include\\jogo");
    env->include(program_files_x86 + "\\Jogo\\include\\jogo");
#endif

    // Run the compiler.  Output to a temporary file if the compiler will
    // continue on to another stage; otherwise,  the file directly.
    Parser::Ptr parser(new Parser(env));
    if (env_->errors()) {
        errors_++;
    }

    // Semantic analysis/type checking phase.
    SemanticAnalyzer::Ptr checker(new SemanticAnalyzer(env));

    // Code expansion
    if (!env_->errors()) {
        CodeExpander::Ptr expander(new CodeExpander(env));
    }

    if (env->dump_ast()) {
        TreePrinter::Ptr tprint(new TreePrinter(env, Stream::stout()));
        return;
    }
    if (env_->errors()) {
        errors_++;
        return;
    }

    // Final output generatation and linking phase.
    if (env_->monolithic_build()) {
        monolithic_build();
    } else {
        modular_build();
    }
}

void Builder::monolithic_build() {
    // Builds all object files/inputs, etc. into one library or executable.
    // The builds result goes to the file specified by the 'output' option.
    std::stringstream ss;

    for (File::Itr file = env_->files(); file; ++file) {
        if (file->is_output_file()) {
            operator()(file); 
            ss << file->jgo_file() << " ";
            if (File::is_reg(file->native_file())) {
                ss << file->o_file() << " ";
            }
        }
    }
    procs_.wait();
    errors_ += procs_.errors();
    if (!env_->link() || errors_ || env_->errors()) { return; }
    if (env_->dump_ir()) { return; }

    if (env_->root()->function(env_->name("main"))) {
#ifdef WINDOWS
        std::string exe = env_->output()+".exe";
#else
        std::string exe = env_->output();        
#endif
        link(ss.str(), exe);
    } else {
        File::mkdir(File::dir_name(env_->output()));
        Stream::Ptr fout(new Stream(env_->output() + ".jgi"));
        InterfaceGenerator::Ptr iface(new InterfaceGenerator(env_, fout));
        for (File::Itr file = env_->files(); file; ++file) {
            if (file->is_output_file()) {
                iface(file.pointer());
            }
        }
#ifdef WINDOWS
        std::string lib = env_->output()+".lib";
        ss << "build\\runtime\\Coroutine.Intel64.obj";
#else
        std::string dir = File::dir_name(env_->output());
        std::string out = File::base_name(env_->output());
        std::string lib = dir + FILE_SEPARATOR + "lib" + out + ".a";
        ss << "build/runtime/Coroutine.Intel64.o";
#endif
        archive(ss.str(), lib);
    }

    if (!env_->execute()) { return; }
    execute(env_->output());
}

void Builder::modular_build() {
    // Build one static library per module, and build one executable per 
    // executable module with a "main" function specified, using the "main"
    // function for the module as the entry point.
     
    // Create any out-of-date static libraries first, then create the static
    // libraries.
    for (int i = 0; i < env_->inputs(); i++) {
        std::string name = Import::module_name(env_->input(i)); 
        Module::Ptr m = env_->module(env_->name(name));
        if (!m) {
            Stream::sterr() << "Module '" << env_->input(i) << "' not found\n";
            Stream::sterr()->flush();
            errors_++;
        } else if (!m->function(env_->name("main"))) {
            env_->lib(m->name()->string());
            m(this);
        }
    }

    for (int i = 0; i < env_->inputs(); i++) {
        std::string name = Import::module_name(env_->input(i)); 
        Module::Ptr m = env_->module(env_->name(name));
        if (m && m->function(env_->name("main"))) {
            m(this);
        }
    }
}

void Builder::operator()(Module* module) {
    // Generates the output for the module.  This function iterates over each
    // file in the module, and outputs the code for those files, then either
    // archives the results into a single static library, or performs the link
    // step if the output is an executable.
    if (env_->errors()) { return; }
    if (env_->make() && module->is_up_to_date()) { return; }

    // Set the entry point for the executable module
    env_->entry_point(module->label()->string() + "_main");

    for (int i = 0; i < module->files(); i++) {
        operator()(module->file(i));
    }     
    procs_.wait();
    errors_ += procs_.errors();
    if (!env_->link() || errors_ || env_->errors()) { return; }

    if (module->function(env_->name("main"))) {
        link(module); 
    } else  {
        File::mkdir(File::dir_name(module->jgi_file()));
        Stream::Ptr fout(new Stream(module->jgi_file()));
        InterfaceGenerator::Ptr iface(new InterfaceGenerator(env_, fout));
        iface->operator()(module);
        archive(module);
    }

    if (!env_->execute()) { return; }
    execute(module->exe_file());
}

void Builder::operator()(File* file) {
    // Generates the output files for the given source file.  Depending on the
    // options, this function will output the AST, IR, .jgo, or .c file.
    if (file->is_interface_file()) { return; }
    if (env_->errors()) { return; }

    // Generate native machine code, and then compile or assemble it.
    if (!env_->make() || !file->is_up_to_date(File::JGO)) {
        File::mkdir(File::dir_name(file->jgo_file()));
        if (env_->verbose()) {
            Stream::stout() << "Compiling " << file->name() << "\n";
            Stream::stout()->flush();
        }
        if (env_->generator() == "Nasm64") {
            irgen(file);
            if (env_->dump_ir()) { return; }
            nasm64gen(file);
            if (!env_->assemble()) { return; }
            nasm(file->asm_file(), file->jgo_file());
        } else if (env_->generator() == "C") {
            cgen(file);
            if (!env_->assemble()) { return; }
            cc(file->c_file(), file->jgo_file());
        } else if (env_->generator() == "Intel64") {
            irgen(file);
            if (env_->dump_ir()) { return; }
            if (!env_->assemble()) { return; } 
            intel64gen(file);
        }
    }

    // Compile any native files.  Native files have the same name as the source
    // file, but with a .c extension.
    if (File::is_reg(file->native_file())) {
        bool ok1 = File::is_up_to_date(file->native_file(), file->o_file());
        bool ok2 = File::is_up_to_date(env_->program_path(), file->o_file());
        if (!env_->make() || !(ok1 && ok2)) {
            cc(file->native_file(), file->o_file());
        }
    }
}

void Builder::link(Module* module) {
    // Links the current module, and outputs an executable.
    std::stringstream ss;

    // Output object files and native object files.
    for (int i = 0; i < module->files(); i++) {
        ss << module->file(i)->jgo_file() <<  " ";
        if (File::is_reg(module->file(i)->native_file())) {
            ss << module->file(i)->o_file() << " ";
        }
    }

    link(ss.str(), module->exe_file());
}

void Builder::link(const std::string& in, const std::string& out) {
    // Links all the files specified in 'in' (space-delimited) and generates 
    // the output file 'out'.
    // Select the correct linker command for the current OS/platform.
    std::stringstream ss;
#if defined(WINDOWS)
    ss << "link.exe /SUBSYSTEM:console /NOLOGO /MACHINE:X64 ";
#elif defined(LINUX)
    ss << "gcc -m64 ";
#elif defined(DARWIN)
    ss << "gcc -Wl,-no_pie ";
#endif
    env_->entry_module(out);

    // Link the main() routine, which is custom-generated for each linked
    // executable.
    std::string main = std::string("Boot") + FILE_SEPARATOR + "Main.jg";
    File::Ptr mf = env_->file(env_->name(main));
    operator()(mf);
    procs_.wait();

#ifdef WINDOWS
    for (int i = 0; i < env_->includes(); i++) {
        ss << "/LIBPATH:\"" << env_->include(i) << "\" ";
    }
    for (int i = 0; i < env_->libs(); i++) {
        ss << env_->lib(i) << ".lib ";
    }
#else
    for (int i = 0; i < env_->includes(); i++) {
        if (File::is_dir(env_->include(i))) {
            ss << "-L\"" << env_->include(i) << "\" ";
        }
    }
    for (int i = 0; i < env_->libs(); i++) {
        ss << "-l" << env_->lib(i) << " ";
    }
#endif
    ss << mf->jgo_file() << " ";

    // Output link options for libraries and module dependencies.
#ifdef WINDOWS
    ss << in << " /OUT:" << out;
#else
    ss << in << "-o " << out;
#endif

    if (env_->verbose()) {
        Stream::stout() << ss.str() << "\n";
        Stream::stout()->flush();
    } 
    File::mkdir(File::dir_name(out).c_str());
    if (system(ss.str().c_str())) { 
        errors_++;
    } 
}

void Builder::archive(Module* module) {
    // Links the current module, and outputs an archive and interface file.
    std::stringstream ss;
    for (int i = 0; i < module->files(); i++) {
        ss << module->file(i)->jgo_file() <<  " ";
        if (File::is_reg(module->file(i)->o_file())) {
            ss << module->file(i)->o_file() << " ";
        }
    }
    archive(ss.str(), module->lib_file());
}

void Builder::archive(const std::string& in, const std::string& out) {
    // Output object files and native object files
    std::stringstream ss;
    remove(out.c_str());

    // Select the correct archive program for the current OS/platform.
#ifdef WINDOWS
    ss << "lib.exe /SUBSYSTEM:console /MACHINE:X64 /NOLOGO /OUT:" << out << " " << in;
#else
    ss << "ar rcs " << out << " " << in;
#endif
    if (env_->verbose()) {
        Stream::stout() << ss.str() << "\n";
        Stream::stout()->flush();
    } 
    File::mkdir(File::dir_name(out).c_str());;
    if (system(ss.str().c_str())) { 
        errors_++;
    } 
}

void Builder::irgen(File* file) {
    // Generates code for all basic blocks using the Jogo intermediate
    // represenation.  Also performs optimizations, if enabled by the
    // environment options.
    Machine::Ptr machine = Machine::intel64();
    IrGenerator::Ptr bgen(new IrGenerator(env_, machine));
    RegisterAllocator::Ptr alloc(new RegisterAllocator(env_, machine));
    IrBlockPrinter::Ptr bprint(new IrBlockPrinter(env_, machine));
    Stream::Ptr out = Stream::stout();
    out->machine(machine);
    bprint->out(out);

    bgen->operator()(file);
    if (env_->dump_ir() && env_->verbose()) {
        bprint->operator()(file);
    }
    if (env_->optimize()) {
        CopyPropagator::Ptr copy(new CopyPropagator(env_, machine));
        DeadCodeEliminator::Ptr elim(new DeadCodeEliminator(env_, machine));
        copy->operator()(file);
        elim->operator()(file);
    }

    if (env_->dump_ir() && env_->verbose()) {
        bprint->operator()(file);
    }
    alloc->operator()(file);

    if (env_->dump_ir()) {
        bprint->operator()(file);
    }
}

void Builder::cgen(File* file) {
    // Generates C code for all functions/classes in 'file.'  Outputs to a 
    // temporary file if this is an intermediate step; otherwise, outputs
    // to a named file in the build directory.
    CCodeGenerator::Ptr c(new CCodeGenerator(env_));
    c->out(new Stream(file->jgo_file()));  
    if (c->out()->error()) {
        std::string msg = c->out()->message();
        Stream::sterr() << file->asm_file() << msg << "\n";
        Stream::sterr()->flush();
        Stream::stout()->flush();
        errors_++;
        return;
    }
    c->operator()(file);
}

void Builder::nasm64gen(File* file) {
    // Generates NASM Intel 64 code for all functions/classes in 'file.'
    // Outputs to a temporary file if this is an intermediate step; otherwise,
    // outputs to a named file in the build directory.
    Nasm64Generator::Ptr nasm64gen(new Nasm64Generator(env_));
    nasm64gen->out(new Stream(file->asm_file()));  
    if (nasm64gen->out()->error()) {
        std::string msg = nasm64gen->out()->message();
        Stream::sterr() << file->asm_file() << msg << "\n";
        Stream::sterr()->flush();
        Stream::stout()->flush();
        errors_++;
        return;
    }
    nasm64gen->operator()(file);
}

void Builder::intel64gen(File* file) {
    // Output Intel64 (x86-64) machine code directly to the correct object file
    // format for the current system.
    Intel64Generator::Ptr intel64gen(new Intel64Generator(env_));
    intel64gen->out(new Stream(file->jgo_file()));
    if (intel64gen->out()->error()) {
        std::string msg = intel64gen->out()->message();
        Stream::sterr() << file->jgo_file() << msg << "\n";
        Stream::sterr()->flush();
        Stream::stout()->flush();
        errors_++;
        return;
    }
#if defined(DARWIN)
    OutputFormat::Ptr format(new Mach64Output);
    intel64gen->format(format);
    intel64gen->operator()(file);
#elif defined(WINDOWS)
    assert(!"Not supported");
#elif defined(LINUX)
    assert(!"Not supported");
#endif
}

void Builder::cc(const std::string& in, const std::string& out) {
    // Compiles a single C source file, and outputs the result to 'out.'
    std::stringstream ss;
#if defined(WINDOWS)
    ss << "cl.exe " << in << " /nologo /c /TC /Fo\"" << out << "\"";
    if (env_->optimize()) {
        ss << " /O2";
    }
    ss << " /DCOROUTINE_STACK_SIZE=" << COROUTINE_STACK_SIZE;
    if (!env_->verbose()) {
        ss << " > NUL";
    }
#else
    ss << "gcc " << in << " -c -o " << out;
    if (env_->optimize()) {
        ss << " -O2";
    } else {
        ss << " -O0 -g";
    }
    ss << " -DCOROUTINE_STACK_SIZE=" << COROUTINE_STACK_SIZE;
#endif

#ifdef WINDOWS
    ss << " /DWINDOWS";
#elif defined(DARWIN)
    ss << " -DDARWIN";
#elif defined(LINUX)
    ss << " -DLINUX -m64 -lm";
#else
    #error "Unknown platform"
#endif

    for (int i = 0; i < env_->includes(); i++) {
        if (File::is_dir(env_->include(i))) {
#if defined(WINDOWS)
            ss << " /I \"" << env_->include(i) << "\""; 
#else
            ss << " -I " << env_->include(i);
#endif
        }
    }
    if (env_->verbose()) {
        Stream::stout() << ss.str() << "\n";
        Stream::stout()->flush();
    }
    procs_.process(ss.str());
    //if (system(ss.str().c_str())) {
    //    errors_++;
    //} 
}

void Builder::nasm(const std::string& in, const std::string& out) {
    // Assembles a single NASM assembly file, and outputs the result to 'out.'
    std::stringstream ss;
#if defined(WINDOWS)
    ss << "nasm -fwin64 ";
#elif defined(LINUX)
    ss << "nasm -felf64 ";
#elif defined(DARWIN)
    ss << "/usr/local/bin/nasm -fmacho64 ";
#endif 
    ss << in << " -o " << out;
    if (env_->verbose()) {
        Stream::stout() << ss.str() << "\n";
        Stream::stout()->flush();
    }
    procs_.process(ss.str());
}

void Builder::execute(const std::string& exe) {
    // Executes the program generated by the given module.
    std::stringstream ss;
#ifdef WINDOWS
    ss << exe << ".exe";
#else
    if (exe[0] != '/') {
        ss << "./";
    }
    ss << exe << std::endl;
#endif
    if (system(ss.str().c_str())) {
        errors_++;
    } 
}
