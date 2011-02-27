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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, APEXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */  

#ifndef STATEMENT_HPP
#define STATEMENT_HPP

#include "Apollo.hpp"
#include "TreeNode.hpp"
#include "Name.hpp"
#include "Expression.hpp"
#include "Type.hpp"

/* This file includes interfaces for statement nodes */
class Statement : public TreeNode {
public:
    Statement(Location loc) :
        TreeNode(loc) {
    }
    
    Statement* next() const { return next_; }
    void next(Statement* next) { next_ = next; }
    typedef Pointer<Statement> Ptr;

private:
    Statement::Ptr next_;
};

/* Simple statement with only one expression */
class Simple : public Statement {
public:
    Simple(Location loc, Expression* expr) :
        Statement(loc),
        expression_(expr) {
    }

    Expression* expression() const { return expression_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr expression_;
};

/* When statement */
class When : public Statement {
public:
    When(Location loc, Name* var, Type* type, Statement* block) : 
        Statement(loc),
        variable_(var),
		type_(type),
        block_(block) {
    }

    Name* variable() const { return variable_; }
    Type* type() const { return type_; }
    Statement* block() const { return block_; }
    typedef Pointer<When> Ptr;

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Name::Ptr variable_;
	Type::Ptr type_;
    Statement::Ptr block_;
};

/* Case statement */
class Case : public Statement {
public:
    Case(Location loc, Expression* guard, When* branches) :
        Statement(loc),
		guard_(guard),
        branches_(branches) {
    }

	Expression* guard() const { return guard_; }
    When* branches() const { return branches_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
	Expression::Ptr guard_;
    When::Ptr branches_;
};

/* Block statement */
class Block : public Statement {
public:
    Block(Location loc, Statement* children) :
        Statement(loc),
        children_(children) {
    }
    
    Statement* children() const { return children_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Statement::Ptr children_;
};

/* For-each loop statement */
class For : public Statement {
public:
    For(Location loc, Name* var, Type* type, Expression* expr, Statement* block) :
        Statement(loc),
        variable_(var),
		type_(type),
        expression_(expr),
        block_(block) {
    }

    Name* variable() const { return variable_; }
	Type* type() const { return type_; }
    Expression* expression() const { return expression_; }
    Statement* block() const { return block_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Name::Ptr variable_;
	Type::Ptr type_;
    Expression::Ptr expression_;
    Statement::Ptr block_;
};

/* Variable declaration */
class Variable : public Statement {
public:
    Variable(Location loc, Name* ident, Type* type, Expression* expr) :
        Statement(loc),
        identifier_(ident),
		type_(type),
        initializer_(expr) {
    }

    Name* identifier() const { return identifier_; }
    Type* type() const { return type_; }
    Expression* initializer() const { return initializer_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Name::Ptr identifier_;
	Type::Ptr type_;
    Expression::Ptr initializer_;
};

/* While loop */
class While : public Statement {
public:
    While(Location loc, Expression* guard, Statement* block) :
        Statement(loc), 
        guard_(guard),
        block_(block) {
    }

    Expression* guard() const { return guard_; }
    Statement* block() const { return block_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr guard_;
    Statement::Ptr block_;
};

/* Conditional */
class Conditional : public Statement {
public:
    Conditional(Location loc, Expression* ex, Statement* yes, Statement* no) :
        Statement(loc),
        guard_(ex),
        true_branch_(yes),
        false_branch_(no) {
    }

    Expression* guard() const { return guard_; }
    Statement* true_branch() const { return true_branch_; }
    Statement* false_branch() const { return false_branch_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr guard_;
    Statement::Ptr true_branch_;
    Statement::Ptr false_branch_;
};

/* Return statement */
class Return : public Statement {
public:
    Return(Location loc, Expression* expr) :
        Statement(loc),
        expression_(expr) {
    }

    Expression* expression() const { return expression_; }

private:
    void operator()(Functor* functor) { functor->operator()(this); }
    Expression::Ptr expression_;
};

#endif