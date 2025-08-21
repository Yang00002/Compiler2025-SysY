#include "PhiEliminate.hpp"


#include "Instruction.hpp"
#include "BasicBlock.hpp"


#define DEBUG 0
#include "Util.hpp"

using namespace std;

void PhiEliminate::run()
{
	LOG(m_->print());
	PREPARE_PASS_MSG;
	LOG(color::cyan("Run PhiEliminate Pass"));
	PUSH;
	for (auto f : m_->get_functions())
	{
		if (f->is_lib_)continue;
		f_ = f;
		runOnFunc();
	}
	POP;
	PASS_SUFFIX;
	LOG(color::cyan("PhiEliminate Done"));
}

void PhiEliminate::runOnFunc()
{
	LOG(color::cyan("Run PhiEliminate At Func ") + f_->get_name());
	PUSH;
	auto& ppq = pq;
	auto& iiq = inq;
	std::function<bool(Use)> pred = [&ppq, &iiq](const Use& u)-> bool
	{
		auto phi = dynamic_cast<PhiInst*>(u.val_);
		if (phi != nullptr && !iiq.count(phi))
		{
			ppq.emplace(phi);
			iiq.emplace(phi);
		}
		return true;
	};
	for (auto bb : f_->get_basic_blocks())
	{
		if (bb->is_entry_block()) continue;
		for (auto phi : bb->get_instructions().phi_and_allocas())
		{
			pq.emplace(dynamic_cast<PhiInst*>(phi));
			inq.emplace(dynamic_cast<PhiInst*>(phi));
		}
	}
	while (!pq.empty())
	{
		auto top = pq.front();
		pq.pop();
		auto as = top->all_same_operand_exclude_self();
		if (as != nullptr)
		{
			LOG(color::green("Remove Phi ") + top->print() + color::green(" with ") + as->get_name());
			top->replace_use_with_if(as, pred);
			inq.erase(top);
			top->get_parent()->erase_instr(top);
			delete top;
		}
		else
			inq.erase(top);
	}
	POP;
}
