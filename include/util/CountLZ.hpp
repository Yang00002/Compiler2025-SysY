#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#define CZ_MSVC 1
#include <intrin.h>
#endif

inline int m_countr_zero(unsigned long long x) noexcept
{
	if (x == 0) return sizeof(unsigned long long) * 8;

#if CZ_MSVC
	unsigned long index;
	_BitScanForward64(&index, x);
	return static_cast<int>(index);

#else
     return __builtin_ctzll(x);
#endif
}
