#pragma once

// 系统是大端还是小端
namespace system_about
{
	extern bool SMALL_END;
	// 逻辑上 8 个元素数组的左端
	extern int LOGICAL_LEFT_END_8;
	// 逻辑上 8 个元素数组的右端
	extern int LOGICAL_RIGHT_END_8;
	// 逻辑上 2 个元素数组的右端
	extern int LOGICAL_RIGHT_END_2;
}