#include "LocalConstGlobalMatching.hpp"

#include "Ast.hpp"
#include "Instruction.hpp"
#include "Type.hpp"
#include "FuncInfo.hpp"

void LocalConstGlobalMatching::run()
{
	auto info = manager_->flushAndGetGlobalInfo<FuncInfo>();
	std::unordered_set<Function*> funcs;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		for (auto ld : info->loadDetail(f).globals_)
		{
			if (info->storeDetail(f).globals_.count(ld))continue;
			f->constForSelf_.emplace(ld);
		}
	}
}
