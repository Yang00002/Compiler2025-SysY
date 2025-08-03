#include "Print.hpp"


#define DEBUG 1
#include "Util.hpp"

void Print::run()
{
	m_->print();
	LOG(m_->print());
}
