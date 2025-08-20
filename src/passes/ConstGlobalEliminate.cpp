#include "ConstGlobalEliminate.hpp"

#include "Ast.hpp"
#include "Constant.hpp"
#include "FuncInfo.hpp"
#include "Instruction.hpp"
#include "Tensor.hpp"
#include "Type.hpp"
#include "Util.hpp"

void ConstGlobalEliminate::run()
{
	manager_->flushGlobalInfo<FuncInfo>();
	for (auto glob : m_->get_global_variable())
	{
		auto ty = glob->get_type()->toPointerType()->typeContained();
		if (ty->isBasicType())
		{
			bool ok = true;
			for (auto use : glob->get_use_list())
			{
				if (dynamic_cast<StoreInst*>(use.val_) != nullptr)
				{
					ok = false;
					break;
				}
			}
			if (!ok) continue;
			Constant* ct;
			if (ty == Types::FLOAT)
			{
				float val = 0.0f;
				auto seg = glob->get_init()->segment(0);
				if (seg.second == 0) val = seg.first->front().getFloatConstant();
				ct = Constant::create(m_, val);
			}
			else
			{
				int val = 0;
				auto seg = glob->get_init()->segment(0);
				if (seg.second == 0) val = seg.first->front().getIntConstant();
				ct = Constant::create(m_, val);
			}
			std::vector<LoadInst*> rm;
			for (auto use : glob->get_use_list())
			{
				auto loadInst = dynamic_cast<LoadInst*>(use.val_);
				ASSERT(loadInst != nullptr);
				loadInst->replace_all_use_with(ct);
				rm.emplace_back(loadInst);
			}
			for (auto i : rm)
			{
				i->get_parent()->erase_instr(i);
				delete i;
			}
		}
	}
}
