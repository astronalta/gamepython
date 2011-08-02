/*
 * Copyright (c) 2011 Matt Fichman
 *
 * Permission is hereby granted, free of charge, to any person obtaisg a copy
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

#include "LivenessAnalyzer.hpp"

using namespace std;

void LivenessAnalyzer::operator()(Function* feature) {
    in_.clear();
    out_.clear();
    finished_ = false;
    function_ = feature;

    // Iterate the liveness computation until the all the liveness rules are
    // fully statisfied.
    while (!finished_) {
        finished_ = true;
        entry_block_ = true;
        for (int i = 0; i < feature->basic_blocks(); i++) {
            operator()(feature->basic_block(i));
            entry_block_ = false;
        }
    }

}

void LivenessAnalyzer::operator()(BasicBlock* block) {
    // Compute liveness information for the basic block.  This algorithm is 
    // partly based on the notes found here, with slight optimizations: 
    // http://www.classes.cs.uchicago.edu/archive/2004/spring/22620-1/docs/liveness.pdf 
    
    for (int i = block->instrs()-1; i >= 0; i--) {
        // Note: We iterate through the list of instructions backwards, since
        // liveness analysis is a reverse type of analysis - that is, we 
        // propagate liveness information from the use of a temporary back to
        // its last write.
        const Instruction& instr = block->instr(i);
        set<int>& in = in_[&instr];
        set<int>& out = out_[&instr];

        // in[n] := use[n] U (out[n] - def[n]).  Since the in/out sets can 
        // never decrease in size, don't worry about resetting either of the
        // two sets.
        if (instr.first().temp()) {
            finished_ &= !in.insert(instr.first().temp()).second;
        }
        if (instr.second().temp()) {
            finished_ &= !in.insert(instr.second().temp()).second;
        }

      
        // If this is the first instruction of the function, then we need to
        // add all the registers belonging to the caller to the def[n] set.
        if (entry_block_ && i == 0) {
            for (int k = 0; k < machine_->caller_regs(); k++) {
                finished_ &= !in.insert(-machine_->caller_reg(k)->id()).second;
            }
        }

        // If this is a return instruction, then we need to add all registers
        // belonging to the caller to the use[n] set.
        if (instr.opcode() == RET) {
            for (int k = 0; k < machine_->caller_regs(); k++) {
                finished_ &= !out.insert(-machine_->caller_reg(k)->id()).second;
            }
        }
        
        // out[n] - def[n]
        for (set<int>::iterator j = out.begin(); j != out.end(); j++) {
            if (*j != instr.result().temp()) {
                // In/out sets can never decrease in size, don't worry about
                // removing def[n] temporaries from the set
                finished_ &= !in.insert(*j).second;
            }
        }

        // Recompute the 'out' set from the 'in' set of the next instruction.
        if (i == block->instrs()-1) { // Last instruction of the block
            if (block->branch()) {
                set<int>& s = in_[&block->branch()->instr(0)];
                for (set<int>::iterator j = s.begin(); j != s.end(); j++) {
                    finished_ &= !out.insert(*j).second;        
                }
            } 
            if (block->next()) {
                set<int>& s = in_[&block->next()->instr(0)];
                for (set<int>::iterator j = s.begin(); j != s.end(); j++) {
                    finished_ &= !out.insert(*j).second;        
                }
            } 
        } else { // Get the 'in' set from the next instruction
            set<int>& s = in_[&block->instr(i + 1)];
            for (set<int>::iterator j = s.begin(); j != s.end(); j++) {
                finished_ &= !out.insert(*j).second;
            } 
        }
    }
}

bool LivenessAnalyzer::live(const Instruction& inst, int temp) {
    std::set<int>& in = in_[&inst];
    std::set<int>::iterator i = in.find(temp);
    return (i == in.end()) ? false : true;
}


