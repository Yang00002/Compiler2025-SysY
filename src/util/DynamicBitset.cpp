#include "DynamicBitset.hpp"
#include "Type.hpp"
#include "CountLZ.hpp"
#include <cstring>

using namespace std;


#define UPONES(start) ~((1ull << static_cast<unsigned>(start)) - 1)
#define UPZEROS(start) ((1ull << static_cast<unsigned>(start)) - 1)
#define ULLOFBITS(bits) (static_cast<unsigned>(bits) >> 6)
#define OFFSETOFBITS(bits) (static_cast<unsigned>(bits) & 63u)

DynamicBitset::DynamicBitset(): data_(nullptr), bitlen_(0), dataSize_(0)
{
}

DynamicBitset::DynamicBitset(const DynamicBitset& other) :
	bitlen_(other.bitlen_)
{
	dataSize_ = (bitlen_ + 63) >> 6;
	data_ = new unsigned long long[dataSize_];
	memcpy(data_, other.data_, dataSize_ << 3);
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
	memcpy(data_, other.data_, dataSize_ << 3);
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
	bitlen_ = static_cast<unsigned>(len);
	dataSize_ = (bitlen_ + 63) >> 6;
	data_ = new unsigned long long[dataSize_]{};
}

DynamicBitset::DynamicBitset(unsigned len)
{
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
	int idx1 = i >> 6;
	int idx2 = i & 63;
	return data_[idx1] & (1ull << idx2);
}

void DynamicBitset::set(int i) const
{
	int idx1 = i >> 6;
	int idx2 = i & 63;
	data_[idx1] |= 1ull << idx2;
}

bool DynamicBitset::test(unsigned i) const
{
	unsigned idx1 = i >> 6;
	unsigned idx2 = i & 63;
	return data_[idx1] & 1ull << idx2;
}

void DynamicBitset::set(unsigned i) const
{
	unsigned idx1 = i >> 6;
	unsigned idx2 = i & 63;
	data_[idx1] |= 1ull << idx2;
}

void DynamicBitset::set(unsigned f, unsigned t) const
{
	unsigned ullf = ULLOFBITS(f);
	unsigned offf = OFFSETOFBITS(f);
	unsigned ullt = ULLOFBITS(t);
	unsigned offt = OFFSETOFBITS(t);
	if (ullf == ullt)
	{
		auto a = UPONES(offf);
		auto b = UPZEROS(offt);
		data_[ullf] |= UPONES(offf) & UPZEROS(offt);
		return;
	}
	data_[ullf] |= UPONES(offf);
	unsigned range = ullt - ullf - 1;
	if (range > 0) memset(data_ + ullf + 1, 0XFF, range << 6);
	data_[ullt] |= UPZEROS(offt);
}

void DynamicBitset::reset(unsigned i) const
{
	unsigned idx1 = i >> 6;
	unsigned idx2 = i & 63;
	data_[idx1] &= ~(1ull << idx2);
}

void DynamicBitset::reset(int i) const
{
	int idx1 = i >> 6;
	int idx2 = i & 63;
	data_[idx1] &= ~(1ull << idx2);
}

void DynamicBitset::reset() const
{
	memset(data_, 0, dataSize_ << 3);
}

void DynamicBitset::operator|=(const DynamicBitset& bit) const
{
	for (unsigned i = 0; i < dataSize_; i++) data_[i] |= bit.data_[i];
}

void DynamicBitset::operator-=(const DynamicBitset& bit) const
{
	for (unsigned i = 0; i < dataSize_; i++) data_[i] &= ~bit.data_[i];
}

DynamicBitset DynamicBitset::operator-(const DynamicBitset& bit) const
{
	DynamicBitset nb = *this;
	for (unsigned i = 0; i < dataSize_; i++) nb.data_[i] &= ~bit.data_[i];
	return nb;
}

DynamicBitset DynamicBitset::operator|(const DynamicBitset& bit) const
{
	DynamicBitset nb = *this;
	for (unsigned i = 0; i < dataSize_; i++) nb.data_[i] |= bit.data_[i];
	return nb;
}

bool DynamicBitset::operator==(const DynamicBitset& bit) const
{
	for (unsigned i = 0; i < dataSize_; i++)
		if (data_[i] != bit.data_[i])return false;
	return true;
}

bool DynamicBitset::operator!=(const DynamicBitset& bit) const
{
	for (unsigned i = 0; i < dataSize_; i++)
		if (data_[i] != bit.data_[i])return true;
	return false;
}

bool DynamicBitset::allZeros() const
{
	for (unsigned i = 0; i < dataSize_; i++)
		if (data_[i] != 0)return false;
	return true;
}

bool DynamicBitset::include(const DynamicBitset& bit) const
{
	for (unsigned i = 0; i < dataSize_; i++)
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
	for (unsigned i = 0; i < bitlen_; i++)
	{
		if (test(i)) ret += to_string(i) + " ";
	}
	if (!ret.empty()) ret.pop_back();
	return ret;
}

std::string DynamicBitset::print(const std::function<std::string(unsigned)>& sf) const
{
	std::string ret;
	for (unsigned i = 0; i < bitlen_; i++)
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

size_t DynamicBitset::Iterator::operator*() const
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
