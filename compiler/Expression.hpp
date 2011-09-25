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

#pragma once

#include "Apollo.hpp"
#include "TreeNode.hpp"
#include "String.hpp"
#include "Type.hpp"

/* This file includes interfaces for expression nodes */
class Expression : public TreeNode {
public:
    Expression(Location loc) :
        TreeNode(loc) {
    }
	
	Expression* next() const { return next_; }
    Type* parent_type() const { return parent_type_; }
    Type* type() const { return type_; }
    bool is_logic_op() const;
	void next(Expression* next) { next_ = next; }
    void last(Expression* last) {
        Expression* expr = this;
        while (expr->next()) {
            expr = expr->next();
        }
        expr->next(last);
    }
    void parent_type(Type* type) { parent_type_ = type; }
    void type(Type* type) { type_ = type; }
    typedef Pointer<Expression> Ptr;

private:
	Expression::Ptr next_;
    Type::Ptr type_;
    Type::Ptr parent_type_;
};

/* Literal expression (integers, strings, booleans, hashes, etc.) */
class StringLiteral : public Expression {
public:
    StringLiteral(Location loc, String* value) :
        Expression(loc),
        value_(value) {
    }
    String* value() const { return value_; } 

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr value_;
};

/* Literal expression (integers, strings, booleans, hashes, etc.) */
class IntegerLiteral : public Expression {
public:
    IntegerLiteral(Location loc, String* value) :
        Expression(loc),
        value_(value) {
    }
    String* value() const { return value_; } 

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr value_;
};

/* Literal expression (integers, strings, booleans, hashes, etc.) */
class NilLiteral : public Expression {
public:
    NilLiteral(Location loc) : Expression(loc) {}

private:
    void operator()(Functor* functor) { functor->operator()(this); }
};

/* Literal expression (integers, strings, booleans, hashes, etc.) */
class FloatLiteral : public Expression {
public:
    FloatLiteral(Location loc, String* value) :
        Expression(loc),
        value_(value) {
    }
    String* value() const { return value_; } 

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr value_;
};

/* Boolean literal expression */
class BooleanLiteral : public Expression {
public:
    BooleanLiteral(Location loc, String* value) :
        Expression(loc),
        value_(value) {
    }
    String* value() const { return value_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr value_;
};    

/* Hash literal */
class HashLiteral : public Expression {
public:
    HashLiteral(Location loc, Expression* args) :
        Expression(loc),
        arguments_(args) {
    }

    Expression* arguments() const { return arguments_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr arguments_;
};

/* Array literal */
class ArrayLiteral : public Expression {
public:
    ArrayLiteral(Location loc, Expression* args) :
        Expression(loc),
        arguments_(args) {
    }

    Expression* arguments() const { return arguments_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr arguments_;
};

/* Simple binary expression */
class Binary : public Expression {
public:
    Binary(Location loc, String* op, Expression* left, Expression* right) :
        Expression(loc),
        operation_(op),
        left_(left),
        right_(right) {
    }

    String* operation() const { return operation_; }
    Expression* left() const { return left_; }
    Expression* right() const { return right_; } 

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr operation_;
    Expression::Ptr left_;
    Expression::Ptr right_;
};

/* Simple unary expression */
class Unary : public Expression {
public:
    Unary(Location loc, String* op, Expression* child) :
        Expression(loc),
        operation_(op),
        child_(child) {
    }

    String* operation() const { return operation_; }
    Expression* child() { return child_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr operation_;
    Expression::Ptr child_;
};

/* Normal function all */
class Call : public Expression {
public:
    Call(Location loc, File* file, String* mod, String* id, Expression* args) :
        Expression(loc),
        file_(file),
        scope_(mod),
        identifier_(id),
        arguments_(args),
        function_(0) {
    }

    String* scope() const { return scope_; }
    String* identifier() const { return identifier_; }
    File* file() const { return file_; } 
    Expression* arguments() const { return arguments_; }
    Function* function() const { return function_; }
    void arguments(Expression* args) { arguments_ = args; }
    void function(Function* function) { function_ = function; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    File* file_;
    String::Ptr scope_;
    String::Ptr identifier_;
    Expression::Ptr arguments_;
    Function* function_;
};

/* Object-oriented function call dispatch */
class Dispatch : public Expression {
public:
    Dispatch(Location loc, String* ident, Expression* self, Expression* args) :
        Expression(loc),
        identifier_(ident),
        function_(0) {
        
        self->next(args);
        arguments_ = self;
    }

    String* identifier() const { return identifier_; }
    Expression* arguments() const { return arguments_; }
    Function* function() const { return function_; }
    void function(Function* function) { function_ = function; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr identifier_;
    Expression::Ptr arguments_;
    Function* function_;
};

/* Constructor call */
class Construct : public Expression {
public:
    Construct(Location loc, Type* type, Expression* args) :
        Expression(loc),
        arguments_(args) {
        
        Expression::type(type);
    }

    Expression* arguments() const { return arguments_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Type::Ptr clazz_;
    Expression::Ptr arguments_;
};

/* Identifier expression (variable access) */
class Identifier : public Expression {
public:
    Identifier(Location loc, String* ident) :
        Expression(loc),
        identifier_(ident) {
    }

    String* identifier() const { return identifier_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    String::Ptr identifier_;
};

/* Variable declaration */
class Assignment : public Expression {
public:
    Assignment(Location loc, String* ident, Type* type, Expression* expr) :
        Expression(loc),
        identifier_(ident),
		declared_type_(type),
        initializer_(expr) {
    }

    String* identifier() const { return identifier_; }
    Type* declared_type() { return declared_type_; }
    Expression* initializer() const { return initializer_; }
    void operator()(Functor* functor) { functor->operator()(this); }
    typedef Pointer<Assignment> Ptr;

private:
    String::Ptr identifier_;
	Type::Ptr declared_type_;
    Expression::Ptr initializer_;
};

/* Empty expression */
class Empty : public Expression {
public:
    Empty(Location loc) :
        Expression(loc) {
    }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
};

