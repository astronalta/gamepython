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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#pragma once

#include "Apollo.hpp"
#include "Environment.hpp"
#include "Feature.hpp"
#include "Object.hpp"
#include <fstream>
#include <set>

/* Primary parser structure; creates compilation units */
class Parser : public Object {
public:
    Parser(Environment* env);
    ~Parser();

    Environment* environment() const { return environment_; }
    Module* module() const { return module_; }
	File* file() const { return file_; }
    std::fstream& input() { return input_; }
    int column() const { return column_; }
    void input(const std::string& file);
    void column(int column) { column_ = column; }
    void force_separator();
    void recover_line();
    bool is_recovering();
    typedef Pointer<Parser> Ptr;

private:
    Environment::Ptr environment_;
    Module::Ptr module_;
    File::Ptr file_;
    int column_;
    std::fstream input_;
    void *scanner_;

    void file(const std::string& prefix, const std::string& file);
    void dir(const std::string& prefix, const std::string& dir);
};

/* Union used by Bison and Flex for actions */
union ParseNode {
	Expression* expression;
	Statement* statement;
	Formal* formal;
	When* when;
	Feature* feature;
	String* string;
    Type* type;
    Generic* generic;
    Variable* variable;
    Block* block;
	Feature::Flags flag;
};


