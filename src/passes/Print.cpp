#include "Print.hpp"


#define DEBUG 0
#include "Dominators.hpp"
#include "LoopDetection.hpp"
#include "Util.hpp"

void Print::run()
{
	GAP;
	GAP;
	LOG(m_->print());
	GAP;
	GAP;
	manager_->flushAllInfo();
	for (auto i : m_->get_functions())
	{
		if (i->is_lib_)continue;
		i->checkBlockRelations();
		//manager_->getFuncInfo<Dominators>(i);
		//manager_->getFuncInfo<LoopDetection>(i);
	}
	//manager_->getGlobalInfo<FuncInfo>();
}
