#include "LocalConstGlobalMatching.hpp"

#include "Ast.hpp"
#include "Instruction.hpp"
#include "Type.hpp"
#include "FuncInfo.hpp"

#define DEBUG 0
#include "Util.hpp"

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
			if (!ld->get_type()->toPointerType()->typeContained()->isBasicType()) continue;
			f->constForSelf_.emplace(ld);
			LOG(color::green("GlobalVar ") + ld->print() + color::green(" cosnt for ") + f->get_name());
		}
	}
}
