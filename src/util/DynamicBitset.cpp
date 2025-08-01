#include "DynamicBitset.hpp"
#include "Type.hpp"
#include "CountLZ.hpp"
#include <cstring>

using namespace std;


#define UPONES(start) static_cast<unsigned long long>(~((1ll << (start)) - 1))
#define UPZEROS(start) static_cast<unsigned long long>((1 << (start)) - 1)
#define ULLOFBITS(bits) ((bits) >> 6)
#define OFFSETOFBITS(bits) ((bits) & 63)

DynamicBitset::DynamicBitset(): data_(nullptr), bitlen_(0), dataSize_(0)
{
}

DynamicBitset::DynamicBitset(const DynamicBitset& other) :
	bitlen_(other.bitlen_)
{
	dataSize_ = (bitlen_ + 63) >> 6;
	data_ = new unsigned long long[dataSize_];
	memcpy(data_, other.data_, dataSize_ << 3u);
}

DynamicBitset::DynamicBitset(DynamicBitset&& other) noexcept
	: data_(other.data_),
	  bitlen_(other.bitlen_), dataSize_(other.dataSize_)
{
	other.data_ = nullptr;
	other.bitlen_ = 0;
	other.dataSize_ = 0;
}

DynamicBitset& DynamicBitset::operator=(const DynamicBitset& other)
{
	if (this == &other)
		return *this;
	if (dataSize_ < other.dataSize_)
	{
		delete[] data_;
		data_ = new unsigned long long[other.dataSize_];
	}
	bitlen_ = other.bitlen_;
	dataSize_ = other.dataSize_;
	memcpy(data_, other.data_, dataSize_ << 3u);
	return *this;
}

DynamicBitset& DynamicBitset::operator=(DynamicBitset&& other) noexcept
{
	if (this == &other)
		return *this;
	delete[] data_;
	data_ = other.data_;
	bitlen_ = other.bitlen_;
	dataSize_ = other.dataSize_;
	other.data_ = nullptr;
	other.bitlen_ = 0;
	return *this;
}

DynamicBitset::DynamicBitset(int len)
{
	assert(len >= 0);
	bitlen_ = len;
	dataSize_ = (bitlen_ + 63) >> 6;
	data_ = new unsigned long long[dataSize_]{};
}

DynamicBitset::~DynamicBitset()
{
	delete[] data_;
}

bool DynamicBitset::test(int i) const
{
	assert(i >= 0);
	int idx1 = i >> 6;
	int idx2 = i & 63;
	return data_[idx1] & (1ull << idx2);
}

void DynamicBitset::set(int i) 
{
	assert(i >= 0);
	int idx1 = i >> 6;
	int idx2 = i & 63;
	data_[idx1] |= 1ull << idx2;
}

void DynamicBitset::set(int f, int t) 
{
	assert(f >= 0 && t >= 0);
	auto ullf = ULLOFBITS(f);
	auto offf = OFFSETOFBITS(f);
	auto ullt = ULLOFBITS(t);
	auto offt = OFFSETOFBITS(t);
	if (ullf == ullt)
	{
		data_[ullf] |= UPONES(offf) & UPZEROS(offt);
		return;
	}
	data_[ullf] |= UPONES(offf);
	auto range = ullt - ullf - 1;
	if (range > 0) memset(data_ + ullf + 1, 0XFF, range << 6);
	data_[ullt] |= UPZEROS(offt);
}

void DynamicBitset::reset(int i) 
{
	assert(i >= 0);
	int idx1 = i >> 6;
	int idx2 = i & 63;
	data_[idx1] &= ~(1ull << idx2);
}

void DynamicBitset::reset() const
{
	memset(data_, 0, dataSize_ << 3);
}

void DynamicBitset::operator|=(const DynamicBitset& bit)
{
	for (int i = 0; i < dataSize_; i++) data_[i] |= bit.data_[i];
}

void DynamicBitset::operator-=(const DynamicBitset& bit)
{
	for (int i = 0; i < dataSize_; i++) data_[i] &= ~bit.data_[i];
}

DynamicBitset DynamicBitset::operator-(const DynamicBitset& bit) const
{
	DynamicBitset nb = *this;
	for (int i = 0; i < dataSize_; i++) nb.data_[i] &= ~bit.data_[i];
	return nb;
}

DynamicBitset DynamicBitset::operator|(const DynamicBitset& bit) const
{
	DynamicBitset nb = *this;
	for (int i = 0; i < dataSize_; i++) nb.data_[i] |= bit.data_[i];
	return nb;
}

bool DynamicBitset::operator==(const DynamicBitset& bit) const
{
	for (int i = 0; i < dataSize_; i++)
		if (data_[i] != bit.data_[i])return false;
	return true;
}

bool DynamicBitset::operator!=(const DynamicBitset& bit) const
{
	for (int i = 0; i < dataSize_; i++)
		if (data_[i] != bit.data_[i])return true;
	return false;
}

bool DynamicBitset::allZeros() const
{
	for (int i = 0; i < dataSize_; i++)
		if (data_[i] != 0)return false;
	return true;
}

bool DynamicBitset::include(const DynamicBitset& bit) const
{
	for (int i = 0; i < dataSize_; i++)
		if (~data_[i] & bit.data_[i]) return false;
	return true;
}

unsigned DynamicBitset::len() const
{
	return bitlen_;
}

std::string DynamicBitset::print() const
{
	std::string ret;
	for (int i = 0; i < bitlen_; i++)
	{
		if (test(i)) ret += to_string(i) + " ";
	}
	if (!ret.empty()) ret.pop_back();
	return ret;
}

std::string DynamicBitset::print(const std::function<std::string(int)>& sf) const
{
	std::string ret;
	for (int i = 0; i < bitlen_; i++)
	{
		if (test(i)) ret += sf(i) + " ";
	}
	if (!ret.empty()) ret.pop_back();
	return ret;
}

void DynamicBitset::Iterator::skip_zeros()
{
	while (word_idx_ < bs_->dataSize_ && bs_->data_[word_idx_] == 0)
		++word_idx_;
	if (word_idx_ < bs_->dataSize_)
		mask_ = bs_->data_[word_idx_];
	else
		mask_ = 0;
}

DynamicBitset::Iterator::Iterator(const DynamicBitset* bs, bool begin)
	: bs_(bs), word_idx_(0), mask_(0)
{
	if (begin)
	{
		skip_zeros();
	}
	else
	{
		word_idx_ = bs_->dataSize_;
	}
}

int DynamicBitset::Iterator::operator*() const
{
	// 当前字中最低位的 1 对应的全局 bit 下标
	return (word_idx_ << 6) + m_countr_zero(mask_);
}

DynamicBitset::Iterator& DynamicBitset::Iterator::operator++()
{
	mask_ &= mask_ - 1; // 打掉最低位的 1
	if (mask_ == 0)
	{
		// 当前字的 1 已经取完
		++word_idx_;
		skip_zeros();
	}
	return *this;
}

DynamicBitset::Iterator DynamicBitset::Iterator::operator++(int)
{
	auto tmp = *this;
	++*this;
	return tmp;
}

bool DynamicBitset::Iterator::operator==(const Iterator& b) const
{
	return word_idx_ == b.word_idx_ && mask_ == b.mask_;
}

bool DynamicBitset::Iterator::operator!=(const Iterator& b) const
{
	return word_idx_ != b.word_idx_ || mask_ != b.mask_;
}
