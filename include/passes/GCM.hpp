#pragma once

#include "FuncInfo.hpp"
#include "Instruction.hpp"
#include "Dominators.hpp"
#include "PassManager.hpp"
#include <memory>
#include <unordered_set>

class GlobalCodeMotion final : public Pass
{
    std::unique_ptr<Dominators> dom_;
    std::unordered_set<Instruction*> visited_;
    
    void run(Function* f);
    bool is_pinned(Instruction* i);
    BasicBlock* LCA(BasicBlock* bb1, BasicBlock* bb2);
    void moveEarly(Instruction* i, Function* f); // 计算最早可插入位置并移动指令
    void postpone(Instruction* i);  // 尽可能延迟计算
public:
	GlobalCodeMotion(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion(GlobalCodeMotion&&) = delete;
	GlobalCodeMotion& operator=(const GlobalCodeMotion&) = delete;
	GlobalCodeMotion& operator=(GlobalCodeMotion&&) = delete;

	explicit GlobalCodeMotion(Module* m) : Pass(m)
	{ dom_ = std::make_unique<Dominators>(m); }

	~GlobalCodeMotion() override = default;

	void run() override
    {
        for(auto f:m_->get_functions()){
            if( !f->is_declaration() && !f->is_lib_ && f->get_num_basic_blocks()>1 ) 
                run(f);
        }
    }
};
