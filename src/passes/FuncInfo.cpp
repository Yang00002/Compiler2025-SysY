#include "FuncInfo.hpp"
#include "Function.hpp"
#include "Instruction.hpp"
#include "Type.hpp"

FuncInfo::FuncInfo(Module* m): Pass(m)
{
}

void FuncInfo::run()
{
	for (auto& func : m_->get_functions())
	{
		trivial_mark(func);
		if (not is_pure[func])
			worklist.push_back(func);
	}
	while (worklist.empty() == false)
	{
		const auto now = worklist.front();
		worklist.pop_front();
		process(now);
	}
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
bool FuncInfo::is_pure_function(Function* func) const
{
	return is_pure.at(func);
}

// 有 store 操作的函数非纯函数来处理
void FuncInfo::trivial_mark(Function* func)
{
	if (func->is_declaration() or func->get_name() == "main")
	{
		is_pure[func] = false;
		return;
	}
	// 只要传入数组，都作为非纯函数处理
	for (const auto arg_type : func->get_function_type()->argumentTypes())
	{
		if (arg_type->isComplexType())
		{
			is_pure[func] = false;
			return;
		}
	}
	for (const auto& bb : func->get_basic_blocks())
		for (auto inst : bb->get_instructions())
		{
			if (is_side_effect_inst(inst))
			{
				is_pure[func] = false;
				return;
			}
		}
	is_pure[func] = true;
}

void FuncInfo::process(const Function* func)
{
	for (auto& use : func->get_use_list())
	{
		if (const auto inst = dynamic_cast<Instruction*>(use.val_))
		{
			if (auto f = inst->get_parent()->get_parent(); is_pure[f])
			{
				is_pure[f] = false;
				worklist.push_back(f);
			}
		}
	}
}

// 对局部变量进行 store 没有副作用
bool FuncInfo::is_side_effect_inst(Instruction* inst)
{
	if (inst->is_store())
	{
		if (is_local_store(dynamic_cast<StoreInst*>(inst)))
			return false;
		return true;
	}
	if (inst->is_load())
	{
		if (is_local_load(dynamic_cast<LoadInst*>(inst)))
			return false;
		return true;
	}
	// call 指令的副作用会在后续 bfs 中计算
	return false;
}

bool FuncInfo::is_local_load(const LoadInst* inst)
{
	auto addr =
		dynamic_cast<Instruction*>(get_first_addr(inst->get_operand(0)));
	if (addr and addr->is_alloca())
		return true;
	return false;
}

bool FuncInfo::is_local_store(const StoreInst* inst)
{
	auto addr = dynamic_cast<Instruction*>(get_first_addr(inst->get_lval()));
	if (addr and addr->is_alloca())
		return true;
	return false;
}

Value* FuncInfo::get_first_addr(Value* val)
{
	if (const auto inst = dynamic_cast<Instruction*>(val))
	{
		if (inst->is_alloca())
			return inst;
		if (inst->is_gep())
			return get_first_addr(inst->get_operand(0));
		if (inst->is_load())
			return val;
		for (const auto op : inst->get_operands())
		{
			if (op->get_type()->isPointerType())
				return get_first_addr(op);
		}
	}
	return val;
}
