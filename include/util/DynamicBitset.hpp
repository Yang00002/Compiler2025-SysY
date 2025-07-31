#pragma once
#include <string>
#include <functional>

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
	unsigned bitlen_;
	unsigned dataSize_;

public:
	DynamicBitset(int len);
	DynamicBitset(unsigned len);
	~DynamicBitset();
	[[nodiscard]] bool test(int i) const;
	void set(int i) const;
	[[nodiscard]] bool test(unsigned i) const;
	void set(unsigned i) const;
	// 设置一个范围, 不包含末尾
	void set(unsigned f, unsigned t) const;
	void reset(unsigned i) const;
	void reset(int i) const;
	void reset() const;
	void operator|=(const DynamicBitset& bit) const;
	void operator-=(const DynamicBitset& bit) const;
	DynamicBitset operator-(const DynamicBitset& bit) const;
	DynamicBitset operator|(const DynamicBitset& bit) const;
	bool operator==(const DynamicBitset& bit) const;
	bool operator!=(const DynamicBitset& bit) const;
	[[nodiscard]] bool allZeros() const;
	[[nodiscard]] bool include(const DynamicBitset& bit) const;
	[[nodiscard]] unsigned len() const;
	[[nodiscard]] std::string print() const;
	[[nodiscard]] std::string print(const std::function<std::string(unsigned)>& sf) const;

	[[nodiscard]] unsigned dataSize() const
	{
		return dataSize_;
	}


	class Iterator
	{
		const DynamicBitset* bs_;
		unsigned long long word_idx_; // 当前 64-bit 字的下标
		unsigned long long mask_; // 当前字中还没报告的 1

		// 跳到下一个非零字
		void skip_zeros();

	public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = size_t;
		using difference_type = std::ptrdiff_t;

		Iterator(const DynamicBitset* bs, bool begin);

		size_t operator*() const;

		Iterator& operator++();

		Iterator operator++(int);

		bool operator==(const Iterator& b) const;
		bool operator!=(const Iterator& b) const;
	};


	[[nodiscard]] Iterator begin() const { return {this, true}; }
	[[nodiscard]] Iterator end() const { return {this, false}; }
};
