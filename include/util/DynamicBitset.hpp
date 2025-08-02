#pragma once
#include <string>
#include <functional>

#include "System.hpp"

class DynamicBitset
{
	friend class Iterator;

public:
	DynamicBitset();
	DynamicBitset(const DynamicBitset& other);

	DynamicBitset(DynamicBitset&& other) noexcept;

	DynamicBitset& operator=(const DynamicBitset& other);

	DynamicBitset& operator=(DynamicBitset&& other) noexcept;

private:
	unsigned long long* data_;
	int bitlen_;
	int dataSize_;

public:
	DynamicBitset(int len);
	~DynamicBitset();
	[[nodiscard]] bool test(int i) const;
	void set(int i);
	bool setAndGet(int i);
	bool resetAndGet(int i);
	// 设置一个范围, 不包含末尾
	void set(int f, int t);
	void reset(int i);
	void reset() const;
	void operator|=(const DynamicBitset& bit);
	void operator-=(const DynamicBitset& bit);
	DynamicBitset operator-(const DynamicBitset& bit) const;
	DynamicBitset operator|(const DynamicBitset& bit) const;
	bool operator==(const DynamicBitset& bit) const;
	bool operator!=(const DynamicBitset& bit) const;
	[[nodiscard]] bool allZeros() const;
	[[nodiscard]] bool include(const DynamicBitset& bit) const;
	[[nodiscard]] unsigned len() const;
	[[nodiscard]] std::string print() const;
	[[nodiscard]] std::string print(const std::function<std::string(int)>& sf) const;

	[[nodiscard]] int dataSize() const
	{
		return dataSize_;
	}


	class Iterator
	{
		const DynamicBitset* bs_;
		int word_idx_; // 当前 64-bit 字的下标
		unsigned long long mask_; // 当前字中还没报告的 1

		// 跳到下一个非零字
		void skip_zeros();

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = int;
		using difference_type = std::ptrdiff_t;

		Iterator(const DynamicBitset* bs, bool begin);

		int operator*() const;

		Iterator& operator++();

		Iterator operator++(int);

		bool operator==(const Iterator& b) const;
		bool operator!=(const Iterator& b) const;
	};


	[[nodiscard]] Iterator begin() const { return {this, true}; }
	[[nodiscard]] Iterator end() const { return {this, false}; }
};
