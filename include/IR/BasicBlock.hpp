#ifndef BASICBLOCK_HPP
#define BASICBLOCK_HPP

#include "Instruction.hpp"
#include "Value.hpp"

#include <list>
#include <set>
#include <string>

class Function;
class Instruction;
class Module;

class BasicBlock : public Value {
  public:
    ~BasicBlock() = default;
    static BasicBlock *create(Module *m, const std::string &name,
                              Function *parent) {
        auto prefix = name.empty() ? "" : "label_";
        return new BasicBlock(m, prefix + name, parent);
    }

    /****************api about cfg****************/
    std::list<BasicBlock *> &get_pre_basic_blocks() { return pre_bbs_; }
    std::list<BasicBlock *> &get_succ_basic_blocks() { return succ_bbs_; }

    void add_pre_basic_block(BasicBlock *bb) { pre_bbs_.push_back(bb); }
    void add_succ_basic_block(BasicBlock *bb) { succ_bbs_.push_back(bb); }
    void remove_pre_basic_block(BasicBlock *bb) { pre_bbs_.remove(bb); }
    void remove_succ_basic_block(BasicBlock *bb) { succ_bbs_.remove(bb); }

    // If the Block is terminated by ret/br
    bool is_terminated() const;
    // Get terminator, only accept valid case use
    Instruction *get_terminator();

    /****************api about Instruction****************/
    void add_instruction(Instruction *instr);
    void add_instr_begin(Instruction *instr) { instr_list_.push_front(instr); }
    void erase_instr(Instruction* instr) { instr_list_.erase(std::find(instr_list_.begin(),instr_list_.end(),instr)); }

    std::list<Instruction*>& get_instructions() { return instr_list_; }
    bool empty() const { return instr_list_.empty(); }
    int get_num_of_instr() const { return instr_list_.size(); }

    /****************api about accessing parent****************/
    Function *get_parent() { return parent_; }
    Module *get_module();
    void erase_from_parent();

    virtual std::string print() override;

    BasicBlock(const BasicBlock &) = delete;
    explicit BasicBlock(Module *m, const std::string &name, Function *parent);
private:
    std::list<BasicBlock*> pre_bbs_;
    std::list<BasicBlock*> succ_bbs_;
    std::list<Instruction*> instr_list_;
    Function *parent_;
};

#endif