#include "../../include/util/System.hpp"

namespace 
{
	bool isSmallEnd()
	{
		int a = 1;
		char b = *reinterpret_cast<char*>(&a);
		if (b == 0) return false;
		return true;
	}
}

namespace system_about
{
	bool SMALL_END = isSmallEnd();
	int LOGICAL_LEFT_END_8 = SMALL_END ? 7 : 0;
	int LOGICAL_RIGHT_END_8 = SMALL_END ? 0 : 7;
	int LOGICAL_RIGHT_END_2 = SMALL_END ? 0 : 1;
}
