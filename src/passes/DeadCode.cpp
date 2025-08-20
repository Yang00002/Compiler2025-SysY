#include "DeadCode.hpp"

#include <queue>
#include <unordered_set>
#include <vector>
#include <cassert>
#include <list>

#include "BasicBlock.hpp"
#include "Instruction.hpp"
#include "Type.hpp"

#define DEBUG 0
#include "Config.hpp"
#include "Constant.hpp"
#include "Util.hpp"


void DeadCode::WorkList::addClearParameters(Function* f)
{
	if (o1Optimization && !in_clear_parameters_.count(f))
	{
		in_clear_parameters_.emplace(f);
		clear_parameters_.emplace(f);
	}
}

void DeadCode::WorkList::addClearNotUseAllocas(Function* f)
{
	if (o1Optimization && !in_clear_not_use_allocas_.count(f))
	{
		in_clear_not_use_allocas_.emplace(f);
		clear_not_use_allocas_.emplace(f);
	}
}

DeadCode::~DeadCode()
{
	delete work_list;
}

void DeadCode::run()
{
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run DeadCode Pass"));
	PUSH;

	// 提前删除不可达基本块, 因为任何后续的操作都不能删除 br, 所以永远不会产生不可达基本块
	std::unordered_set<Function*> changeFuncs;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		if (clear_basic_blocks(f)) changeFuncs.emplace(f);
	}
	func_info = manager_->getGlobalInfo<FuncInfo>();
	if (!changeFuncs.empty()) func_info->flushAbout(changeFuncs);

	LOG(color::green("Function Info Collected "));
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_) continue;
		work_list->addClearInstructions(f);
		work_list->addClearNotUseAllocas(f);
		work_list->addClearParameters(f);
	}
	bool change;
	do
	{
		change = false;
		changeFuncs.clear();
		while (!work_list->clear_instructions_.empty())
		{
			auto b = work_list->clear_instructions_.front();
			work_list->clear_instructions_.pop();
			work_list->in_clear_instructions_.erase(b);
			if (clear_instructions(b)) changeFuncs.emplace(b);
		}
		if (!changeFuncs.empty())
		{
			change = true;
			func_info->flushLoadsAbout(changeFuncs);
		}

		while (!work_list->clear_not_use_allocas_.empty())
		{
			auto b = work_list->clear_not_use_allocas_.front();
			work_list->clear_not_use_allocas_.pop();
			work_list->in_clear_not_use_allocas_.erase(b);
			change |= clear_not_use_allocas(b);
		}

		if (change) continue;

		while (!work_list->clear_parameters_.empty())
		{
			auto b = work_list->clear_parameters_.front();
			work_list->clear_parameters_.pop();
			work_list->in_clear_parameters_.erase(b);
			change |= clear_parameters(b);
		}
	}
	while (change);

	sweep_globally();
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("DeadCode Done"));
}

// 删除不可达基本块
bool DeadCode::clear_basic_blocks(Function* func) const
{
	bool c = false;
	LOG(color::blue("Begin Clean Basic Block of ") + func->get_name());
	PUSH;
	std::unordered_set<BasicBlock*> visited;
	std::queue<BasicBlock*> toVisit;
	toVisit.emplace(func->get_entry_block());
	visited.emplace(func->get_entry_block());
	while (!toVisit.empty())
	{
		auto bb = toVisit.front();
		toVisit.pop();
		for (auto suc : bb->get_succ_basic_blocks())
		{
			if (!visited.count(suc))
			{
				visited.emplace(suc);
				toVisit.emplace(suc);
			}
		}
	}

	std::unordered_set<BasicBlock*> toRM;
	for (auto& bb : func->get_basic_blocks())
	{
		if (!visited.count(bb))
		{
			LOG(color::pink("Block ") + bb->get_name() + color::pink(" empty, remove"));
			toRM.emplace(bb);
		}
	}

	if (!toRM.empty()) c = true;

	for (auto i : visited) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
	{
		for (auto j : i->get_instructions().phi_and_allocas())
		{
			auto phi = dynamic_cast<PhiInst*>(j);
			if (phi == nullptr) break;
			phi->remove_phi_operandIfIn(toRM);
		}
	}

	for (auto i : toRM) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
	{
		for (const auto& use : i->get_use_list())
		{
			auto n = dynamic_cast<Instruction*>(use.val_)->get_parent();
			if (!toRM.count(n)) n->remove_pre_basic_block(i);
		}
		i->get_parent()->get_basic_blocks().remove(i);
		delete i;
	}

	// 消除冗余 Phi
	for (auto i : visited) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
	{
		if (i->get_pre_basic_blocks().size() > 1) continue;
		auto insts = i->get_instructions().phi_and_allocas();
		auto it = insts.begin();
		auto ed = insts.end();
		while (it != ed)
		{
			auto phi = dynamic_cast<PhiInst*>(it.get_and_add());
			if (phi == nullptr) break;
			phi->replace_all_use_with(phi->get_operand(0));
			delete it.remove_pre();
		}
	}

	POP;

	if (c) 
		manager_->flushFuncInfo(func);
	return c;
}

// -> clear_instructions +

// 删除未使用的参数
bool DeadCode::clear_parameters(Function* func) const
{
	bool c = false;
	LOG(color::blue("Begin Clean Parameters of ") + func->get_name());
	PUSH;
	auto& sts = func_info->storeDetail(func);
	auto& lds = func_info->loadDetail(func);
	for (int i = 0, size = u2iNegThrow(func->get_args().size()); i < size; i++)
	{
		auto arg = func->get_arg(i);
		arg->set_arg_no(i);
		// 没有任何使用
		if (arg->get_use_list().empty())
		{
			for (auto use : func->get_use_list())
			{
				auto inst = dynamic_cast<CallInst*>(use.val_);
				auto caller = inst->get_parent()->get_parent();
				work_list->addClearInstructions(caller);
			}
			delete func->removeArgWithoutUpdate(arg);
			i--;
			size--;
			c = true;
			continue;
		}
		// 指针
		if (arg->get_type()->isPointerType())
		{
			// 没有存入也没有读取
			if (!sts.have(arg) && !lds.have(arg))
			{
				for (auto use : func->get_use_list())
				{
					auto inst = dynamic_cast<CallInst*>(use.val_);
					auto caller = inst->get_parent()->get_parent();
					work_list->addClearInstructions(caller);
				}
				func->removeArgWithoutUpdate(arg);
				eraseValue(arg);
				delete arg;
				i--;
				size--;
				c = true;
			}
		}
	}
	POP;
	return c;
}

// -> clear_instructions

// 删除未 load 的 alloca
bool DeadCode::clear_not_use_allocas(Function* func) const
{
	LOG(color::blue("Begin Clean Not Use Alloca of ") + func->get_name());
	PUSH;
	bool c = false;
	auto et = func->get_entry_block();
	auto insts = InstructionList{};
	insts.addAllPhiAndAllocas(et->get_instructions());
	auto it = insts.begin();
	auto ed = insts.end();
	while (it != ed)
	{
		auto alloc = dynamic_cast<AllocaInst*>(it.get_and_add());
		bool haveLoad = false;

		std::unordered_set<Instruction*> visited;
		std::queue<Instruction*> q;
		visited.emplace(alloc);
		q.emplace(alloc);
		while (!q.empty())
		{
			auto v = q.front();
			q.pop();
			for (auto& use : v->get_use_list())
			{
				auto inst = dynamic_cast<Instruction*>(use.val_);
				ASSERT(inst != nullptr);
				int idx = use.arg_no_;
				switch (inst->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
				{
					case Instruction::load:
						{
							haveLoad = true;
							break;
						}
					case Instruction::call:
						haveLoad = true;
						break;
					case Instruction::getelementptr:
						{
							ASSERT(idx == 0);
							if (!visited.count(inst))
							{
								visited.emplace(inst);
								q.emplace(inst);
							}
							break;
						}
					case Instruction::memcpy_:
						{
							if (idx == 0) haveLoad = true;
							break;
						}
					case Instruction::nump2charp:
						{
							if (!visited.count(inst))
							{
								visited.emplace(inst);
								q.emplace(inst);
							}
							break;
						}
					case Instruction::store:
					case Instruction::memclear_:
						break;
					default:
						ASSERT(false);
						break;
				}
				if (haveLoad) break;
			}
			if (haveLoad) break;
		}

		if (!haveLoad)
		{
			LOG(color::green("Remove Alloca ") + alloc->print());
			et->erase_instr(alloc);
			eraseValue(alloc);
			delete alloc;
			work_list->addClearInstructions(func);
			c = true;
		}
	}
	POP;
	return c;
}

// 删除 load -> no load: clear_not_use_allocas / clear_parameters

// 删除无用指令, 保留 store 和包含 store 的 call 等
bool DeadCode::clear_instructions(Function* func) const
{
	mark(func);
	if (sweep(func))
	{
		work_list->addClearParameters(func);
		work_list->addClearNotUseAllocas(func);
		return true;
	}
	return false;
}

void DeadCode::mark(Function* func) const
{
	LOG(color::blue("Begin Mark Unused Instructions of ") + func->get_name());
	PUSH;
	work_list->work_list.clear();
	work_list->marked.clear();

	for (auto& bb : func->get_basic_blocks())
	{
		for (auto ins : bb->get_instructions())
		{
			if (is_critical(ins))
			{
				work_list->marked[ins] = true;
				work_list->work_list.push_back(ins);
			}
		}
	}

	while (work_list->work_list.empty() == false)
	{
		auto now = work_list->work_list.front();
		work_list->work_list.pop_front();
		mark(now);
	}

	POP;
}

void DeadCode::mark(const Instruction* ins) const
{
	for (auto op : ins->get_operands())
	{
		auto def = dynamic_cast<Instruction*>(op);
		if (def == nullptr)
			continue;
		if (work_list->marked[def])
			continue;
		ASSERT(def->get_function() == ins->get_function());
		work_list->marked[def] = true;
		work_list->work_list.push_back(def);
	}
}

bool DeadCode::sweep(Function* func) const
{
	LOG(color::blue("Begin Sweep ") + func->get_name());
	PUSH;
	int count = 0;
	for (const auto& bb : func->get_basic_blocks())
	{
		LOG(color::pink("BasicBlock ") + bb->get_name());
		auto& instructions = bb->get_instructions();
		auto it = instructions.begin();
		PUSH;
		while (it != instructions.end())
		{
			auto n = it.get_and_add();
			if (work_list->marked[n]) continue;
			LOG(color::pink("Remove Instruction ") + n->get_name() + " " + n->get_instr_op_name());
			n->remove_all_operands();
			// ReSharper disable once CppNoDiscardExpression
			it.remove_pre();
			delete n;
			count++;
		}
		POP;
	}
	POP;
	return count;
}

bool DeadCode::is_critical(Instruction* ins) const
{
	// 删除未存储值或调用 impure 库的函数调用
	if (ins->is_call())
	{
		auto call_inst = dynamic_cast<CallInst*>(ins);
		auto callee = dynamic_cast<Function*>(call_inst->get_operand(0));
		if (func_info->useOrIsImpureLib(callee) || !func_info->storeDetail(callee).empty())
			return true;
		return false;
	}
	if (ins->is_memcpy() || ins->is_memclear())
		return true;
	if (ins->is_br() || ins->is_ret())
		return true;
	if (ins->is_store())
		return true;
	return false;
}

void DeadCode::sweep_globally() const
{
	std::vector<Function*> unused_funcs;
	std::vector<GlobalVariable*> unused_globals;
	for (auto& f_r : m_->get_functions())
	{
		if (!f_r->is_lib_ && f_r->get_use_list().empty() && f_r->get_name() != "main")
			unused_funcs.push_back(f_r);
	}
	for (auto& glob_var_r : m_->get_global_variable())
	{
		if (glob_var_r->get_use_list().empty())
			unused_globals.push_back(glob_var_r);
	}
	for (auto func : unused_funcs)
	{
		m_->get_functions().remove(func);
		manager_->flushFuncInfo(func);
		func_info->removeFunc(func);
		delete func;
	}
	for (auto glob : unused_globals)
	{
		m_->get_global_variable().remove(glob);
		delete glob;
	}
}

void DeadCode::eraseValue(const Value* val) const
{
	std::queue<Instruction*> worklist;
	std::unordered_set<Instruction*> collected;
	for (auto use : val->get_use_list())
	{
		auto inst = dynamic_cast<Instruction*>(use.val_);
		// 防止将 call 一并删除 (但最后, DeadCode 会删除该参数, 因为这属于其它函数的未使用参数)
		if (inst->is_call())
		{
			inst->set_operand(use.arg_no_, Constant::create(m_, 0));
			work_list->addClearParameters(dynamic_cast<Function*>(inst->get_operand(0)));
			continue;
		}
		if (!collected.count(inst))
		{
			ASSERT(!inst->is_phi());
			collected.emplace(inst);
			worklist.emplace(inst);
		}
	}
	while (!worklist.empty())
	{
		auto p = worklist.front();
		worklist.pop();
		for (auto use : p->get_use_list())
		{
			auto inst = dynamic_cast<Instruction*>(use.val_);
			// 防止将 call 一并删除 (但最后, DeadCode 会删除该参数, 因为这属于其它函数的未使用参数)
			if (inst->is_call())
			{
				inst->set_operand(use.arg_no_, Constant::create(m_, 0));
				work_list->addClearParameters(dynamic_cast<Function*>(inst->get_operand(0)));
				continue;
			}
			if (!collected.count(inst))
			{
				ASSERT(!inst->is_phi());
				collected.emplace(inst);
				worklist.emplace(inst);
			}
		}
	}
	for (auto p : collected) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
	{
		p->get_parent()->erase_instr(p);
		delete p;
	}
}
