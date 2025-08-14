#include "FuncInfo.hpp"

#include <queue>

#include "Function.hpp"
#include "Instruction.hpp"
#include "Type.hpp"

void FuncInfo::UseMessage::add(Value* val)
{
	auto g = dynamic_cast<GlobalVariable*>(val);
	if (g != nullptr)
	{
		globals_.emplace(g);
		return;
	}
	auto arg = dynamic_cast<Argument*>(val);
	arguments_.emplace(arg);
}

bool FuncInfo::UseMessage::have(Value* val) const
{
	auto g = dynamic_cast<GlobalVariable*>(val);
	if (g != nullptr) return globals_.count(g);
	return arguments_.count(dynamic_cast<Argument*>(val));
}

bool FuncInfo::UseMessage::empty() const
{
	return globals_.empty() && arguments_.empty();
}

FuncInfo::FuncInfo(Module* m): Pass(m)
{
}

void FuncInfo::run()
{
	std::unordered_map<Value*, Value*> spMap;
	for (auto glob : m_->get_global_variable())
		spread(glob, spMap);
	std::queue<Function*> worklist;
	std::unordered_set<Function*> visited;
	for (auto& func : m_->get_functions())
	{
		if (func->is_lib_) continue;
		useImpureLibs[func] = false;
		worklist.emplace(func);
		visited.emplace(func);
		for (auto& arg : func->get_args())
		{
			if (arg->get_type()->isPointerType())
			{
				spread(arg, spMap);
			}
		}
	}
	while (!worklist.empty())
	{
		auto f = worklist.front();
		worklist.pop();
		visited.erase(f);
		for (auto& use : f->get_use_list())
		{
			auto inst = dynamic_cast<CallInst*>(use.val_);
			auto cf = inst->get_parent()->get_parent();
			auto& lld = loads[cf];
			auto& rld = loads[f];
			auto& lst = stores[cf];
			auto& rst = stores[f];
			auto ps = lld.globals_.size() + lld.arguments_.size() + lst.globals_.size() + lst.arguments_.size();
			for (auto i : rld.globals_) lld.globals_.emplace(i);
			for (auto i : rst.globals_) lst.globals_.emplace(i);
			auto& ops = inst->get_operands();
			for (auto& arg : f->get_args())
			{
				if (arg->get_type()->isPointerType())
				{
					auto carg = ops[arg->get_arg_no() + 1];
					auto traceP = spMap.find(carg);
					if (traceP == spMap.end()) continue;
					auto trace = traceP->second;
					if (rld.have(arg)) lld.add(trace);
					if (rst.have(arg)) lst.add(trace);
				}
			}
			if (lld.globals_.size() + lld.arguments_.size() + lst.globals_.size() + lst.arguments_.size() != ps)
			{
				if (!visited.count(cf))
				{
					visited.emplace(cf);
					worklist.emplace(cf);
				}
			}
		}
	}

	for (auto& func : m_->get_functions())
	{
		if (func->is_lib_)
		{
			for (auto& use : func->get_use_list())
			{
				auto call = dynamic_cast<CallInst*>(use.val_);
				useImpureLibs[call->get_parent()->get_parent()] = true;
			}
		}
		worklist.emplace(func);
		visited.emplace(func);
	}
	while (!worklist.empty())
	{
		auto f = worklist.front();
		worklist.pop();
		visited.erase(f);
		if (useImpureLibs[f])
			for (auto& use : f->get_use_list())
			{
				auto inst = dynamic_cast<CallInst*>(use.val_);
				auto cf = inst->get_parent()->get_parent();
				if (!useImpureLibs[cf])
				{
					useImpureLibs[cf] = true;
					if (!visited.count(cf))
					{
						visited.emplace(cf);
						worklist.emplace(cf);
					}
				}
			}
	}
}

FuncInfo::UseMessage& FuncInfo::loadDetail(Function* function)
{
	return loads[function];
}

FuncInfo::UseMessage& FuncInfo::storeDetail(Function* function)
{
	return stores[function];
}

bool FuncInfo::is_pure_function(Function* func)
{
	if (func->is_lib_) return false;
	return loads[func].empty() && stores[func].empty() && !useImpureLibs[func];
}

bool FuncInfo::useOrIsImpureLib(Function* function)
{
	if (function->is_lib_) return true;
	return useImpureLibs[function];
}

void FuncInfo::spread(Value* val, std::unordered_map<Value*, Value*>& spMap)
{
	auto glob = dynamic_cast<GlobalVariable*>(val);
	if (glob != nullptr && glob->is_const()) return;
	std::unordered_set<Value*> visited;
	std::queue<Value*> q;
	visited.emplace(val);
	spMap[val] = val;
	q.emplace(val);
	while (!q.empty())
	{
		auto v = q.front();
		ASSERT(v->get_type()->isPointerType());
		q.pop();
		for (auto& use : v->get_use_list())
		{
			auto inst = dynamic_cast<Instruction*>(use.val_);
			auto f = inst->get_parent()->get_parent();
			int idx = use.arg_no_;
			switch (inst->get_instr_type()) // NOLINT(clang-diagnostic-switch-enum)
			{
				case Instruction::load:
					{
						loads[f].add(val);
						break;
					}
				case Instruction::store:
					{
						ASSERT(dynamic_cast<StoreInst*>(inst));
						ASSERT(inst->get_operands().size() == 2);
						ASSERT(idx == 1);
						stores[f].add(val);
						break;
					}
				case Instruction::call:
					{
						// 不能确定库函数的行为, 因此认为其对参数同时进行了读写
						auto callF = dynamic_cast<Function*>(inst->get_operand(0));
						if (callF->is_lib_)
						{
							stores[f].add(val);
							loads[f].add(val);
						}
						break;
					}
				case Instruction::getelementptr:
					{
						ASSERT(idx == 0);
						if (!visited.count(inst))
						{
							visited.emplace(inst);
							spMap[inst] = val;
							q.emplace(inst);
						}
						break;
					}
				case Instruction::memcpy_:
					{
						if (idx == 0) loads[f].add(val);
						else stores[f].add(val);
						break;
					}
				case Instruction::memclear_:
					{
						stores[f].add(val);
						break;
					}
				case Instruction::nump2charp:
				case Instruction::global_fix:
					{
						if (!visited.count(inst))
						{
							visited.emplace(inst);
							spMap[inst] = val;
							q.emplace(inst);
						}
						break;
					}
				default:
					ASSERT(false);
					break;
			}
		}
	}
}
