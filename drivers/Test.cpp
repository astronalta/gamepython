/*
 * Copyright (c) 2011 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a 
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom the 
 * Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#include "Environment.hpp"
#include "Parser.hpp"
#include "SemanticAnalyzer.hpp"
#include "BasicBlockPrinter.hpp"
#include "BasicBlockGenerator.hpp"
#include "LivenessAnalyzer.hpp"
#include "Intel64CodeGenerator.hpp"
#include "TreePrinter.hpp"
#include "Options.hpp"

#include <iostream>
#include <fstream>
#include <set>
#include <cstdlib>

int main(int argc, char** argv) {
    Environment::Ptr env(new Environment());
    //env->output(tmpnam(NULL));
    env->output("/tmp/out.asm");
    
    Options(env, argc, argv);

    std::fstream in((std::string(argv[1]) + ".ap").c_str());
    std::set<std::string> test_options;
    std::string line;
    std::getline(in, line);
    while (!line.empty() && line[0] == '#') {
        test_options.insert(line);
        std::getline(in, line);
    } 

    Machine::Ptr machine = Machine::intel64();
    Parser::Ptr parser(new Parser(env));
    if (test_options.find("# aptest print_tree off") == test_options.end()) {
        TreePrinter::Ptr printer(new TreePrinter(env));
    }
    SemanticAnalyzer::Ptr checker(new SemanticAnalyzer(env));
    BasicBlockGenerator::Ptr generator(new BasicBlockGenerator(env, machine));
    if (test_options.find("# aptest print_ir off") == test_options.end()) {
        BasicBlockPrinter::Ptr printer(new BasicBlockPrinter(env, machine));
    }
    if (test_options.find("# aptest run_program on") != test_options.end()) {
        RegisterAllocator::Ptr alloc(new RegisterAllocator(env, machine));
        if (test_options.find("# aptest print_ir off") == test_options.end()) {
            BasicBlockPrinter::Ptr printer(new BasicBlockPrinter(env, machine));
        }
        Stream::Ptr out(new Stream("/tmp/out.asm"));
        Intel64CodeGenerator::Ptr gen(new Intel64CodeGenerator(env, out));

        std::stringstream ss;
        std::string ofile = tmpnam(NULL);
#if defined(WINDOWS)
        ss << "nasm -fobj64 " << env->output() << " -o " << ofile; 
        system(ss.str().c_str());
#elif defined(LINUX)
        ss << "nasm -felf64 " << env->output() << " -o " << ofile; 
        system(ss.str().c_str());
        ss.str("");
        ss << "gcc -m64 -L../lib -lapollo " << ofile << " -o /tmp/out";
        system(ss.str().c_str());
        unlink(ofile.c_str());
        //unlink(env->output().c_str());
#elif defined(DARWIN)
        ss << "nasm -fmacho64 " << env->output() << " -o " << ofile;
        system(ss.str().c_str());
        ss.str("");
        ss << "gcc -L../lib -lapollo " << ofile << " -o /tmp/out";
        system(ss.str().c_str());
        unlink(ofile.c_str());
        //unlink(env->output().c_str());
        //system("gcc -Wl,-no_pie -L../lib -lapollo /tmp/out.o -o /tmp/out");
#else
#error Unsupported system
#endif
        system("/tmp/out");
    }
   
    
    return 0;
}
