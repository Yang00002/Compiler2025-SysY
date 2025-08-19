#include "GetElementSplit.hpp"

#include "BasicBlock.hpp"
#include "Constant.hpp"
#include "Instruction.hpp"

#define DEBUG 0
#include "Util.hpp"

using namespace std;

void GetElementSplit::runInner() const
{
	for (auto bb : f_->get_basic_blocks())
	{
		auto& insts = bb->get_instructions();
		auto it = insts.begin();
		while (it != insts.end())
		{
			auto inst = *it;
			if (inst->is_gep())
			{
				if (inst->get_operands().size() >= 3)
				{
					auto i0 = it;
					vector<Value*> indexes;
					int size = u2iNegThrow(inst->get_operands().size());
					for (int i = 1; i < size; i++)
					{
						indexes.emplace_back(inst->get_operand(i));
					}
					Value* ptr = inst->get_operand(0);
					if (auto c = dynamic_cast<Constant*>(indexes[0]); c == nullptr || c->getIntConstant() != 0)
					{
						auto d = GetElementPtrInst::create_gep(ptr, {indexes.begin(), indexes.begin() + 1}, nullptr);
						ptr = d;
						d->set_parent(bb);
						insts.emplace_common_inst_after(d, it);
						++it;
					}
					std::vector<Value*> idx;
					idx.resize(2);
					idx[0] = Constant::create(m_, 0);
					int s = u2iNegThrow(indexes.size());
					for (int i = 1; i < s; i++)
					{
						idx[1] = indexes[i];
						auto d = GetElementPtrInst::create_gep(ptr, idx, nullptr);
						ptr = d;
						d->set_parent(bb);
						insts.emplace_common_inst_after(d, it);
						++it;
					}
					inst->replace_all_use_with(ptr);
					insts.remove(i0);
					delete inst;
				}
			}
			++it;
		}
	}
}

void GetElementSplit::run()
{
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		runInner();
		RUN(f_->checkBlockRelations());
	}
}
