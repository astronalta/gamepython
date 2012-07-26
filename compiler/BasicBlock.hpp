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
#include "String.hpp"
#include "Expression.hpp"
#include <vector>
#include <set>

/* A register ID identifies a register by type and number. */
class RegisterId {
public:
    // A colored register has the COLORED flag set.  This means the register is
    // an actual hardware register.  A floating-point register has the FLOAT
    // flag set.
    static int const FLOAT = 0x8000;
    static int const COLORED = 0x4000;
    static int const FLAGS = FLOAT|COLORED;
    static int const ID = ~FLAGS;

    RegisterId(int id, int flags) : value_((id & ID) | (flags & FLAGS)) {}

    bool is_colored() const { return value_ & COLORED; }
    bool is_int() const { return value_ & FLOAT; }
    bool is_float() const { return !is_int(); }
    int id() const { return value_ & ID; }
    
private:
    int value_;
};

/* Represents a 'logical' address, expressed in words (not bytes) */
class Address {
public:
    Address() : value_(0) {}
    Address(int value) : value_(value) {}

    int value() const { return value_; }
    bool operator!() const { return !value_; }
    bool operator==(const Address& ad) const { return value_ == ad.value_; }
    bool operator!=(const Address& ad) const { return value_ != ad.value_; }

private:
    int value_;

};

/* Operands for three-address code SSA instructions */
class Operand {
public:
    Operand(String* label) 
        : label_(label), temp_(0), indirect_(false) {
        // This constructor is only used for CALL instructions and instructions
        // that load a label directly.
    }
    Operand(Expression* literal) 
        : literal_(literal), temp_(0), indirect_(false) {
        // This constructor is used for operands created via literal expr.
    }
    Operand() 
        : temp_(0), indirect_(false) {
    }
    Operand(int temp) 
        : temp_(temp), indirect_(false) {
        // Constructs an operand from an integer temporary variable.
    }

    static Operand addr(Address addr);
    static Operand addr(int temp, Address addr);

    const Operand& operator++() { temp_++; return *this; }
    Expression* literal() const { return literal_; }
    String* label() const { return label_; }
    int temp() const { return temp_; }
    Address addr() const { return addr_; }
    bool indirect() const { return indirect_; }
    bool colored() const { return temp_ < 0; }
    void indirect(bool indirect) { indirect_ = indirect; }
    void temp(int temp) { temp_ = temp; }
    bool operator==(const Operand& other) const;
    bool operator!=(const Operand& other) const;

private:
    Expression::Ptr literal_; // Literal expr, if this is a literal
    String::Ptr label_; // FixMe: Should not be necessary...needed for CALL
    Type::Ptr type_; // Type of the value held in the operand
    int temp_; // Temporary variable ID (negative = machine register)
    Address addr_; // Address offset (in words)
    bool indirect_; // Set to true if this is an indirect memory access
};

Stream::Ptr operator<<(Stream::Ptr out, const Operand& op);

/* Enumeration of opcodes available to the TAC code */
enum Opcode { 
    MOV, ADD, SUB, MUL, DIV, NEG, ANDB, ORB, PUSH, POP, LOAD, STORE, NOTB,
    CALL, JUMP, BNE, BE, BNZ, BZ, BG, BL, BGE, BLE, RET, NOP, POPN 
    // Note: BNE through BLE must be contiguous
};

/* Class for liveness information related to the instruction */
class Liveness : public Object {
public:
    const std::set<int>& in() const { return in_; }
    const std::set<int>& out() const { return out_; }
    std::set<int>& in() { return in_; }
    std::set<int>& out() { return out_; }
    typedef Pointer<Liveness> Ptr;

private:
    std::set<int> in_; // Live variables on incoming edge
    std::set<int> out_; // Live variables on outgoing edge
};

/* Class for three-address code instructions */
class Instruction {
public:
    Instruction(Opcode op, Operand result, Operand first, Operand second):
        opcode_(op),
        first_(first),
        second_(second),
        result_(result),
        liveness_(new Liveness) {
    }

    Opcode opcode() const { return opcode_; }
    Operand first() const { return first_; }
    Operand second() const { return second_; }
    Operand result() const { return result_; }
    Liveness* liveness() const { return liveness_; }
    void opcode(Opcode opcode) { opcode_ = opcode; }
    void first(Operand first) { first_ = first; }
    void second(Operand second) { second_ = second; }
    void result(Operand result) { result_ = result; }

private:
    Opcode opcode_;
    Operand first_;
    Operand second_;
    Operand result_;
    Liveness::Ptr liveness_;
};

/* Class for basic block nodes */
class BasicBlock : public Object {
public:
    BasicBlock() : branch_(0), next_(0), round_(0) {}
    BasicBlock* branch() const { return branch_; }
    BasicBlock* next() const { return next_; }
    String* label() const { return label_; }
    const Instruction& instr(size_t index) const { return instrs_[index]; }
    Instruction& instr(size_t index) { return instrs_[index]; }
    int round() const { return round_; }
    int instrs() const { return instrs_.size(); }
    bool is_terminated() const;
    bool is_ret() const;
    void swap(BasicBlock* other) { instrs_.swap(other->instrs_); }
    void branch(BasicBlock* branch) { branch_ = branch; }
    void next(BasicBlock* branch) { next_ = branch; }
    void label(String* label) { label_ = label; }
    void instr(const Instruction& inst);
    void instr(Opcode op, Operand res, Operand one, Operand two);
    void round_inc() { round_++; }
    typedef Pointer<BasicBlock> Ptr; 

private:
    std::vector<Instruction> instrs_;
    BasicBlock* branch_;
    BasicBlock* next_;
    String::Ptr label_;
    int round_;
};


