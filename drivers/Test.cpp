/*
 * Copyright (c) 2010 Matt Fichman
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
#include "TypeChecker.hpp"
#include "BasicBlockPrinter.hpp"
#include "BasicBlockGenerator.hpp"
#include "TreePrinter.hpp"

#include <iostream>
#include <fstream>
#include <set>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Illegal argument\n";
        return 1;
    }

    std::fstream in(argv[1]);
    std::set<std::string> options;
    std::string line;
    std::getline(in, line);
    while (!line.empty() && line[0] == '#') {
        options.insert(line);
        std::getline(in, line);
    } 
    


    Environment::Ptr environment(new Environment());
    Parser::Ptr parser(new Parser(environment));
    parser->input(argv[1]);
    if (options.find("# aptest print_tree off") == options.end()) {
        TreePrinter::Ptr printer(new TreePrinter(environment));
    }
    TypeChecker::Ptr checker(new TypeChecker(environment));
    if (options.find("# aptest print_ir off") == options.end()) {
        BasicBlockGenerator::Ptr generator(new BasicBlockGenerator(environment));
        BasicBlockPrinter::Ptr printer(new BasicBlockPrinter(environment));
    }
    
    return 0;
}
