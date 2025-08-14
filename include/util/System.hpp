#pragma once
#include <cstddef>
#include <cstring>
#include <climits>
#include "Util.hpp"

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

#ifndef INT2UNSIGNEDKEEPBITS
#define INT2UNSIGNEDKEEPBITS 1
#endif


#ifndef UNSIGNED2INTKEEPBITS
#define UNSIGNED2INTKEEPBITS 1
#endif

#ifndef RIGHTSHIFTINMATH
#define RIGHTSHIFTINMATH 1
#endif


// 将 4 字节整型逐字节转换为无符号
inline unsigned i2uKeepBits(int i)
{
#if INT2UNSIGNEDKEEPBITS
	return static_cast<unsigned>(i);
#else
	if (!(i & INT_MIN)) return static_cast<unsigned>(i);
	i &= INT_MAX;
	return static_cast<unsigned>(i) | 0x80000000u;
#endif
}

// 将 8 字节整型逐字节转换为无符号
inline unsigned long long ll2ullKeepBits(long long i)
{
#if INT2UNSIGNEDKEEPBITS
	return static_cast<unsigned long long>(i);
#else
	if (!(i & LLONG_MIN)) return static_cast<unsigned long long>(i);
	i &= LLONG_MAX;
	return static_cast<unsigned long long>(i) | 0x8000000000000000ull;
#endif
}

// 将 4 字节无符号逐字节转换为整型
inline int u2iKeepBits(unsigned i)
{
#if UNSIGNED2INTKEEPBITS
	return static_cast<int>(i);
#else
	if (i & 0X80000000U) return (static_cast<int>(i & 0X7FFFFFFFU)) | INT_MIN;
	return static_cast<int>(i);
#endif
}

// 将 8 字节无符号逐字节转换为整型
inline long long ULL2LLKeepBits(unsigned long long i)
{
#if UNSIGNED2INTKEEPBITS
	return static_cast<long long>(i);
#else
	if (i & 0X8000000000000000U) return (static_cast<long long>(i & 0X7FFFFFFFFFFFFFFFU)) | LLONG_MIN;
	return static_cast<long long>(i);
#endif
}

// 将无符号转换为整型, 如果是负的或溢出就报错, 例如用于转换 size_t
template<typename TYT>
int u2iNegThrow(TYT i)
{
	ASSERT((sizeof(TYT) == 4) ?( (i & 0x80000000) == 0) :((i & 0xFFFFFFFF80000000) == 0));
	return static_cast<int>(i);
}

// 将无符号转换为 8 字节整型, 如果是负的或溢出就报错, 例如用于转换 size_t
inline long long u2llNegThrow(unsigned i)
{
	return i;
}

// 将无符号转换为 8 字节整型, 如果是负的或溢出就报错, 例如用于转换 size_t
inline long long u2llNegThrow(unsigned long long i)
{
	ASSERT((i & 0x8000000000000000ull) == 0);
	return static_cast<long long>(i);
}

// 逻辑右移
inline int logicalRightShift(int i, int amount)
{
	ASSERT(amount >= 0 && amount < 32);
#if RIGHTSHIFTINMATH
	unsigned i2 = i2uKeepBits(i);
	i2 >>= amount;
	return u2iKeepBits(i2);
#else
	return i >> amount;
#endif
}
// 逻辑右移
inline long long logicalRightShift(long long i, int amount)
{
	ASSERT(amount >= 0 && amount < 64);
#if RIGHTSHIFTINMATH
	unsigned long long i2 = ll2ullKeepBits(i);
	i2 >>= amount;
	return ULL2LLKeepBits(i2);
#else
	return i >> amount;
#endif
}
// 逻辑右移
inline unsigned int logicalRightShift(unsigned int i, int amount)
{
	ASSERT(amount >= 0 && amount < 32);
	return i >> amount;
}
// 逻辑右移
inline unsigned long long logicalRightShift(unsigned long long i, int amount)
{
	ASSERT(amount >= 0 && amount < 64);
	return i >> amount;
}

// 算数右移
inline int arithmeticRightShift(int i, int amount)
{
	ASSERT(amount >= 0 && amount < 32);
#if RIGHTSHIFTINMATH
	return i >> amount;
#else
	if (!(i & INT_MIN)) return i >> amount;
	return ~((~i) >> amount);
#endif
}
// 算数右移
inline long long arithmeticRightShift(long long i, int amount)
{
	ASSERT(amount >= 0 && amount < 64);
#if RIGHTSHIFTINMATH
	return i >> amount;
#else
	if (!(i & LLONG_MIN)) return i >> amount;
	return ~((~i) >> amount);
#endif
}
// 算数右移
inline unsigned int arithmeticRightShift(unsigned int i, int amount)
{
	ASSERT(amount >= 0 && amount < 32);
	if (!(i & 0X80000000U)) return i >> amount;
	return ~((~i) >> amount);
}
// 算数右移
inline unsigned long long arithmeticRightShift(unsigned long long i, int amount)
{
	ASSERT(amount >= 0 && amount < 64);
	if (!(i & 0X8000000000000000U)) return i >> amount;
	return ~((~i) >> amount);
}


inline bool testInt2UnsignedKeepBits()
{
	int i = -2;
	unsigned i2 = static_cast<unsigned>(i);
	unsigned i3;
	memcpy(&i3, &i, sizeof(int));
	if (i3 != i2) return false;
	long long j = -2;
	unsigned long long j2 = static_cast<unsigned long long>(i);
	unsigned long long j3;
	memcpy(&j3, &j, sizeof(unsigned long long));
	return j3 == j2;
}

inline bool testUnsigned2IntKeepBits()
{
	unsigned i = UINT_MAX;
	return static_cast<int>(i) == -1;
}

inline bool testRightShiftIsMath()
{
	int i = -2;
	return ((i >> 1) & INT_MIN);
}

inline bool testSizeTNotSoLong()
{
	return sizeof(size_t) <= 8;
}
