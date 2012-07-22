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
#include "File.hpp"
#include <vector>

Environment::Ptr env(new Environment());

void print_usage() {
    Stream::Ptr out = Stream::stout();
    out << "Usage: apmake [OPTIONS] PACKAGE...\n\n";
    out << "  -h, --help    Print this help message.\n";
    out << "  -v, --verbose Print extra information during compilation.\n";
    out << "  --version     Print the program version number.\n";
    out << "\n";
    out->flush();
}

void print_version() {
    Stream::Ptr out = Stream::stout();
    out << "Apmake version " << VERSION << ", compiled on ";
    out << __DATE__ << " " << __TIME__ << "\n";
    out->flush();
}

void parse_options(int argc, char** argv) {
    Stream::Ptr err = Stream::sterr();

    std::string flag;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '-') {
            flag = argv[i] + 1;
            if ("v" == flag) { flag = "verbose"; }
            else if ("h" == flag) { flag = "help"; } 
            else {
                print_usage();
                err << "Unknown flag '" << argv[i] << "'\n\n";
                err->flush();
                exit(1);
            }
        } else if (argv[i][0] == '-') {
            flag = argv[i] + 2;
        } else if (flag.empty()) {
            env->input(argv[i]);
        }
    
        if ("help" == flag) {
            print_usage();
            exit(0);
        } else if ("version" == flag) {
            print_version();
            exit(0);
        } else if ("verbose" == flag) {
            env->verbose(true);
        }
    } 
}

int main(int argc, char** argv) {
    // This program recursively finds and builds all files in the source
    // directory, and then generates the output in the lib/bin directory.
    parse_options(argc, argv);

    for (int i = 0; i < env->inputs(); i++) {
        std::stringstream ss;
        ss << "bin/apollo " << "-o lib/" << env->input(i) << " -i lib -i src -m ";
        ss << "--build-dir build " << env->input(i) << " -i runtime";
        // FIXME: Use the real apollo 
        if (env->verbose()) {
            ss << " -v";
            Stream::stout() << ss.str() << "\n";
            Stream::stout()->flush();
        }
        if (system(ss.str().c_str())) {
            exit(1);
        }
    }

    return 0;
}
