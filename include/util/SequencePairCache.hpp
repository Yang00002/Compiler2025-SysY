#pragma once

template <typename K, typename V>
class SequencePairCache
{
	K** k_;
	V** v_;
	int idx = 0;

public:
	SequencePairCache(const SequencePairCache&) = delete;
	SequencePairCache(SequencePairCache&&) = delete;
	SequencePairCache& operator=(const SequencePairCache&) = delete;
	SequencePairCache& operator=(SequencePairCache&&) = delete;
	SequencePairCache(int size);
	SequencePairCache(size_t size);
	~SequencePairCache();
	void append(K* k, V* v);
	K* key(int idx);
	V* value(int idx);
	[[nodiscard]] int size() const;
};

template <typename K, typename V>
SequencePairCache<K, V>::SequencePairCache(const int size)
{
	k_ = new K*[size];
	v_ = new V*[size];
}

template <typename K, typename V>
SequencePairCache<K, V>::SequencePairCache(const size_t size)
{
	k_ = new K*[size];
	v_ = new V*[size];
}

template <typename K, typename V>
SequencePairCache<K, V>::~SequencePairCache()
{
	delete[] k_;
	delete[] v_;
}

template <typename K, typename V>
void SequencePairCache<K, V>::append(K* k, V* v)
{
	k_[idx] = k;
	v_[idx] = v;
	++idx;
}

template <typename K, typename V>
K* SequencePairCache<K, V>::key(int idx)
{
	return k_[idx];
}

template <typename K, typename V>
V* SequencePairCache<K, V>::value(int idx)
{
	return v_[idx];
}

template <typename K, typename V>
int SequencePairCache<K, V>::size() const
{
	return idx;
}
