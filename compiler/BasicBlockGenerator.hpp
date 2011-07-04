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
#include "Environment.hpp"
#include "BasicBlock.hpp"
#include "Object.hpp"
#include <vector>
#include <map>

/* Code generator structure; creates basic block flow graphs */
class BasicBlockGenerator : public TreeNode::Functor {
public:
    BasicBlockGenerator(Environment* env);
    typedef Pointer<BasicBlockGenerator> Ptr; 

private:
    void operator()(Class* unit);
    void operator()(Module* unit);
    void operator()(Formal* formal);
    void operator()(StringLiteral* expression);
    void operator()(IntegerLiteral* expression);
    void operator()(FloatLiteral* expression);
    void operator()(BooleanLiteral* expression);
    void operator()(Binary* expression);
    void operator()(Unary* expression);
    void operator()(Call* expression);
    void operator()(Dispatch* expression);
    void operator()(Construct* expression);
    void operator()(Identifier* expression);
    void operator()(Empty* expression);
    void operator()(Block* statement);
    void operator()(Simple* statement);
    void operator()(Let* let);
    void operator()(While* statement);
    void operator()(Conditional* statement);
    void operator()(Variable* statement);
    void operator()(Return* statement);
    void operator()(When* statement);
    void operator()(Case* statement);
    void operator()(Fork* statement);
    void operator()(Yield* statement);
    void operator()(Function* feature);
    void operator()(Attribute* feature);
    void operator()(Import* feature);
    void operator()(Type* type);

    Operand emit(TreeNode* node) {
        node->operator()(this);
        return return_;
    }

    Operand add(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(ADD, ++temp_, t2, t3);
        return temp_;
    }

    Operand sub(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(SUB, ++temp_, t2, t3);    
        return temp_;
    }

    Operand mul(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(MUL, ++temp_, t2, t3);    
        return temp_;
    }

    Operand div(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(DIV, ++temp_, t2, t3);    
        return temp_;
    }

    Operand notl(BasicBlock* block, Operand t2) {
        block->instr(NOTL, ++temp_, t2, 0);
        return temp_;
    }       

    Operand andl(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(ANDL, ++temp_, t2, t3);    
        return temp_;
    }

    Operand orl(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(ORL, ++temp_, t2, t3);    
        return temp_;
    }

    Operand andb(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(ANDB, ++temp_, t2, t3);    
        return temp_;
    }

    Operand orb(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(ORB, ++temp_, t2, t3);    
        return temp_;
    }

    Operand load(BasicBlock* block, Operand t2) {
        block->instr(LOAD, ++temp_, t2, 0);    
        return temp_;
    }

    Operand li(BasicBlock* block, Operand immediate) {
        block->instr(LI, ++temp_, immediate, 0);
        return temp_;
    }

    Operand pop(BasicBlock* block) {
        block->instr(POP, ++temp_, 0, 0);    
        return temp_;
    }

    void push(BasicBlock* block, Operand t2) {
        block->instr(PUSH, 0, t2, 0);    
    }

    void store(BasicBlock* block, Operand t2, Operand t3) {
        block->instr(STORE, 0, t2, t3);    
    }

    void call(BasicBlock* block, String* name) {
        block->instr(CALL, name, 0, 0);
    }
        
    void ret(BasicBlock* block) {
        block->instr(RET, 0, 0, 0);
    }
    
    BasicBlock* beqz(BasicBlock* block, Operand t2) {
        block->instr(BEQZ, 0, t2, 0);
        block->next(new BasicBlock);
        block->branch(new BasicBlock);
        block->next()->label(environment_->name("l" + stringify(++label_)));
        block->branch()->label(environment_->name("l" + stringify(++label_)));
        return block;
    }

    BasicBlock* bneqz(BasicBlock* block, Operand t2) {
        block->instr(BNEQZ, 0, t2, 0);
        block->next(new BasicBlock);
        block->branch(new BasicBlock);
        block->next()->label(environment_->name("l" + stringify(++label_)));
        block->branch()->label(environment_->name("l" + stringify(++label_)));
        return block;
    }

    BasicBlock* jump(BasicBlock* block) {
        block->instr(JUMP, 0, 0, 0);
        block->branch(new BasicBlock);
        block->branch()->label(environment_->name("l" + stringify(++label_)));
        return block->branch();
    }

    Operand variable(String* name);
    void variable(String* name, Operand temporary);
    void enter_scope();
    void exit_scope();

    Environment::Ptr environment_;
    Class::Ptr class_;
    Module::Ptr module_;
    Function::Ptr function_;
    BasicBlock::Ptr block_;
    Operand return_;
    
    // Mapping from var to temporary
    std::vector<std::map<String::Ptr, Operand> > variable_;

    // Next temporary to use
    Operand temp_;
    int label_;
};

