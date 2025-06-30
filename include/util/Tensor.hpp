#pragma once
#include <vector>
#include <functional>
#include <stack>
#include <stdexcept>
#include <string>

#include "System.hpp"


namespace error
{
	// 张量初始化错误
	std::runtime_error TensorInit(const std::vector<int>& dim);
	std::runtime_error TensorInit(const std::vector<unsigned>& dim);

	// 张量维度索引错误
	std::runtime_error TensorDimIndex(int dimSize, int index);
}


template <typename Element>
class TensorData;

// 迭代器指向目标类型
enum class TensorIterateType : uint8_t
{
	// 子张量, 发生在开始迭代某张量时(除了最外面的那层)
	SUB_TENSOR_BEGIN,
	// 子张量, 发生在迭代某张量结束时(除了最外面的那层)
	SUB_TENSOR_END,
	// 值
	VALUE,
	// 默认值
	DEFAULT_VALUES
};

// 铺平的张量, 拥有成段的数据, 每一段要么是一段默认值, 要么是一段附加的值
template <typename Element>
class PlainTensor
{
public:
	PlainTensor() = default;

	[[nodiscard]] const Element& default_value() const
	{
		return _defaultValue;
	}

private:
	template <typename U>
	friend class Tensor;

public:
	PlainTensor(const PlainTensor&) = delete;
	PlainTensor(PlainTensor&&) = delete;
	PlainTensor& operator=(const PlainTensor&) = delete;
	PlainTensor& operator=(PlainTensor&&) = delete;

	~PlainTensor()
	{
		for (auto& i : _segments)
		{
			delete i;
		}
	}

	// 段数
	int segmentCount();
	std::pair<std::vector<Element>*, int> segment(int index);

private:
	Element _defaultValue;

	// 铺平的张量段
	union PlainTensorSegment
	{
		// 附加的数据
		std::vector<Element>* _elements;
		int _len[2];
		// 区分段类别
		char _type[8];

		PlainTensorSegment()
		{
			_elements = nullptr;
			_type[system_about::LOGICAL_LEFT_END_8] = 1;
		}

		PlainTensorSegment(std::vector<Element>* e)
		{
			_elements = e;
			_type[system_about::LOGICAL_LEFT_END_8] = 0;
		}

		PlainTensorSegment(int l)
		{
			_len[system_about::LOGICAL_RIGHT_END_2] = l;
			_type[system_about::LOGICAL_LEFT_END_8] = 1;
		}

		~PlainTensorSegment()
		{
			if (_type[system_about::LOGICAL_LEFT_END_8] == 0) delete _elements;
		}

		PlainTensorSegment(const PlainTensorSegment&) = delete;
		PlainTensorSegment(PlainTensorSegment&&) = delete;
		PlainTensorSegment& operator=(const PlainTensorSegment&) = delete;
		PlainTensorSegment& operator=(PlainTensorSegment&&) = delete;

		[[nodiscard]] bool isVector() const
		{
			return _type[system_about::LOGICAL_LEFT_END_8] == 0;
		}

		[[nodiscard]] int& asLen()
		{
			return _len[system_about::LOGICAL_RIGHT_END_2];
		}

		[[nodiscard]] std::vector<Element>*& asVec()
		{
			return _elements;
		}
	};

	std::vector<PlainTensorSegment*> _segments;
};

// 初始化张量
template <typename Element>
class Tensor
{
	template <typename U>
	friend class TensorData;

public:
	Tensor(const Tensor&) = delete;
	Tensor(Tensor&&) = delete;
	Tensor& operator=(const Tensor&) = delete;
	Tensor& operator=(Tensor&&) = delete;

private:
	Element _defaultValue;
	// 每个维度的大小
	std::vector<int> _shape;
	// 每个维度空间
	std::vector<int> _dimLen;
	// 实际存储数据
	TensorData<Element>* _data;

public:
	~Tensor();
	// 张量形状, 当为 [] 代表是标量; 否则, 任何一维都应该是正整数.
	[[nodiscard]] const std::vector<int>& getShape() const;
	explicit Tensor(const std::vector<int>& shape, const Element& defaultV);
	explicit Tensor(const std::vector<unsigned>& shape, const Element& defaultV);
	// 每一维的容量, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	// 小于 0 的数输出张量总容量
	[[nodiscard]] int getDimCapacity(int dim) const;
	// 每一维的容量, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	[[nodiscard]] const std::vector<int>& getDimCapacities() const;
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量存储的数据
	[[nodiscard]] TensorData<Element>* getData();
	// 获取张量存储的数据
	[[nodiscard]] const TensorData<Element>* visitData() const;
	// 获取默认值
	[[nodiscard]] Element defaultValue() const;
	// 获取张量的元素 idx (从 0 开始) 的索引
	[[nodiscard]] std::vector<int> index(int idx) const;

	// 为张量定义的迭代器, 可以通过 ++ 迭代张量的每一个元素, 实现高效的张量访问
	class Iterator
	{
		const Tensor* _tensor; // 所属张量
		std::stack<const TensorData<Element>*> _data; // 目前的位置
		std::stack<int> _index; // 位置索引
		bool _end = false;

	public:
		explicit Iterator(const Tensor* tensor) : _tensor(tensor)
		{
			_data.emplace(tensor->visitData());
			_index.emplace(0);
		}

		// 目前迭代器指向什么, 有三种可能
		// TensorIterateType::SUB_TENSOR_BEGIN 子张量, 发生在开始迭代某张量时(除了最外面的那层)
		// TensorIterateType:SUB_TENSOR_END 子张量, 发生在迭代某张量结束时(除了最外面的那层)
		// TensorIterateType::VALUE 值
		// TensorIterateType::DEFAULT_VALUES 默认值
		TensorIterateType getCurrentIterateType();

		// 如果迭代器指向的是值, 获取那个值, 否则报错
		Element getValue();

		// 如果迭代器指向的是默认值, 获取那个默认值和连续默认值的个数, 否则报错
		std::pair<Element, int> getDefaultValues();


		// 如果迭代器指向的是子张量, 获取那个子张量, 否则报错
		const TensorData<Element>* getSubTensor();

		// 前置递增操作符
		Iterator& operator++();

		// 是否迭代结束
		[[nodiscard]] bool isEnd() const;
	};

	/* 获取迭代器, 用法如
	for (auto it = tensor.getIterator(); !it.isEnd(); ++it)
	{
		switch (it.getCurrentIterateType())
		{
			case TensorIterateType::SUB_TENSOR_BEGIN:
				break;
			case TensorIterateType::SUB_TENSOR_END:
				break;
			case TensorIterateType::VALUE:
				break;
			case TensorIterateType::DEFAULT_VALUES:
				break;
		}
	}
	 */
	[[nodiscard]] Iterator getIterator() const;

	// 获得铺平的张量, 拥有成段的数据, 每一段要么是一段默认值, 要么是一段附加的值. 小于等于 gate 个的默认值会被视为附加值
	[[nodiscard]] PlainTensor<Element>* toPlain(int gate = 0) const;


	// 获得铺平的张量, 拥有成段的数据, 每一段要么是一段默认值, 要么是一段附加的值. 小于等于 gate 个的默认值会被视为附加值
	// 通过转换函数, 可以将张量的数据转化为另一类型 
	template <typename Target>
	[[nodiscard]] PlainTensor<Target>* toPlain(std::function<Target(const Element&)> func, int gate = 0) const;
};

// 张量数据, 张量允许其中存储一系列数据, 其中可以包括各种容量的子张量, 当子张量起始维度是 -1 时, 它是标量.
// 所有子张量容量之和应当不大于张量的容量.
// 当分配一个子张量时, 会寻找可以分配的容量最大的子张量(但不会和自身一样大), 使得当前张量存储的数据大小是子张量容量的整数倍.
// 当这样分配的目标张量只剩下 0 维时, 代表分配失败.
template <typename Element>
class TensorData
{
	template <typename U>
	friend class Tensor;

public:
	TensorData(const TensorData& other) = delete;
	TensorData(TensorData&& other) = delete;
	TensorData& operator=(const TensorData& other) = delete;
	TensorData& operator=(TensorData&& other) = delete;

private:
	union TensorDataStore
	{
		TensorDataStore(const TensorDataStore& other) = delete;
		TensorDataStore(TensorDataStore&& other) = delete;
		TensorDataStore& operator=(const TensorDataStore& other) = delete;
		TensorDataStore& operator=(TensorDataStore&& other) = delete;

		std::vector<TensorData*>* _sub_tensors;
		Element _data;

		TensorDataStore()
		{
			_sub_tensors = nullptr;
		}

		~TensorDataStore()
		{
		}
	};

	// 数据所属张量
	Tensor<Element>* _tensor;
	// 该张量数据在张量中起始的维度
	int _beginDim;
	// 实际存储的数据
	TensorDataStore _data;
	// 已经分配的空间大小
	int _size;

public:
	~TensorData();
	// 子张量所属父亲
	[[nodiscard]] Tensor<Element>* tensorBelong() const;
	// 子张量的第一维所在父亲中的维度, 对标量而言是 -1
	[[nodiscard]] int getBeginDim() const;
	bool append(const Element& element);
	// 尝试分配一个子张量, 以进行聚合初始化.
	// 只有张量维度大于 1, 前面的填充值数量刚好是某一维空间的倍数, 且拥有空间时能够分配成功, 否则返回 nullptr
	TensorData* makeSubTensor();
	// 每一维的容量, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	// 小于 0 的数输出张量总容量
	[[nodiscard]] int getDimCapacity(int dim) const;
	// 获取该张量已经分配的空间大小
	[[nodiscard]] int getSpaceAllocated() const;
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量的一个元素, 标记 strict 代表任意维度都不允许越界, 否则只要总体上不越界就不会抛出异常
	[[nodiscard]] Element getElement(const std::vector<int>& index, bool strict = false) const;
	// 获取张量的一个元素
	[[nodiscard]] Element getElement(int index) const;
	// 获取张量的子张量
	[[nodiscard]] const std::vector<TensorData*>* getSubTensors() const;

private:
	explicit TensorData(Tensor<Element>* parent, int beginDim);
};


template <typename Element>
int PlainTensor<Element>::segmentCount()
{
	return static_cast<int>(_segments.size());
}

template <typename Element>
std::pair<std::vector<Element>*, int> PlainTensor<Element>::segment(int index)
{
	if (index < 0 || index >= segmentCount()) throw std::runtime_error("index out of bound");
	auto i = _segments[index];
	if (i->isVector()) return {i->asVec(), 0};
	return {nullptr, i->asLen()};
}

template <typename Element>
Tensor<Element>::~Tensor()
{
	delete _data;
}

template <typename Element>
const std::vector<int>& Tensor<Element>::getShape() const
{
	return _shape;
}

template <typename Element>
Tensor<Element>::Tensor(const std::vector<int>& shape, const Element& defaultV)
{
	_defaultValue = defaultV;
	_shape = shape;
	_dimLen.resize(_shape.size());
	int i = static_cast<int>(shape.size()) - 1;
	if (i >= 0)
	{
		_dimLen[i] = 1;
		for (; i > 0; --i)
		{
			if (_shape[i] <= 0) throw error::TensorInit(shape);
			_dimLen[i - 1] = _dimLen[i] * _shape[i];
		}
		if (_shape[0] <= 0) throw error::TensorInit(shape);
		_data = new TensorData<Element>{this, 0};
	}
	else
		_data = new TensorData<Element>{this, -1};
}

template <typename Element>
Tensor<Element>::Tensor(const std::vector<unsigned>& shape, const Element& defaultV) : _defaultValue(defaultV)
{
	for (auto& i : shape) _shape.emplace_back(static_cast<int>(i));
	_dimLen.resize(_shape.size());
	int i = static_cast<int>(shape.size()) - 1;
	if (i >= 0)
	{
		_dimLen[i] = 1;
		for (; i > 0; --i)
		{
			if (_shape[i] <= 0) throw error::TensorInit(shape);
			_dimLen[i - 1] = _dimLen[i] * _shape[i];
		}
		if (_shape[0] <= 0) throw error::TensorInit(shape);
		_data = new TensorData<Element>{this, 0};
	}
	else
		_data = new TensorData<Element>{this, -1};
}

template <typename Element>
int Tensor<Element>::getDimCapacity(const int dim) const
{
	if (dim < 0) return _shape.empty() ? 1 : _shape[0] * _dimLen[0];
	if (dim >= static_cast<int>(_dimLen.size())) throw error::TensorDimIndex(static_cast<int>(_dimLen.size()), dim);
	return _dimLen[dim];
}

template <typename Element>
const std::vector<int>& Tensor<Element>::getDimCapacities() const
{
	return _dimLen;
}

template <typename Element>
std::string Tensor<Element>::toString(const std::function<std::string(const Element&)>& outFunc) const
{
	return _data->toString(outFunc);
}

template <typename Element>
TensorData<Element>* Tensor<Element>::getData()
{
	return _data;
}

template <typename Element>
const TensorData<Element>* Tensor<Element>::visitData() const
{
	return _data;
}

template <typename Element>
Element Tensor<Element>::defaultValue() const
{
	return _defaultValue;
}

template <typename Element>
std::vector<int> Tensor<Element>::index(int idx) const
{
	std::vector<int> ret;
	int ed = static_cast<int>(_dimLen.size());
	for (int i = 0; i < ed; i++)
	{
		int x = idx / _dimLen[i];
		idx -= x * _dimLen[i];
		ret.emplace_back(x);
	}
	return ret;
}

template <typename Element>
TensorIterateType Tensor<Element>::Iterator::getCurrentIterateType()
{
	if (_end) throw std::runtime_error("tensor iterator stay on end");
	auto dataTop = _data.top();
	if (dataTop->getBeginDim() == -1) return TensorIterateType::VALUE;
	auto idx = _index.top();
	auto& subs = *dataTop->getSubTensors();
	if (idx == -1) return TensorIterateType::SUB_TENSOR_END;
	if (subs.size() > idx)
	{
		if (auto& sub = subs[idx]; sub->getBeginDim() == -1) return TensorIterateType::VALUE;
		return TensorIterateType::SUB_TENSOR_BEGIN;
	}
	else return TensorIterateType::DEFAULT_VALUES;
}

template <typename Element>
Element Tensor<Element>::Iterator::getValue()
{
	if (_end) throw std::runtime_error("tensor iterator stay on end");
	auto dataTop = _data.top();
	if (dataTop->getBeginDim() == -1) return dataTop->getElement(0);
	auto idx = _index.top();
	auto& subs = *dataTop->getSubTensors();
	if (idx == -1) throw std::runtime_error("tensor iterator stay on sub tensor, not value, can not get value");
	if (subs.size() > idx)
	{
		if (auto& sub = subs[idx]; sub->getBeginDim() == -1) return sub->getElement(0);
		throw std::runtime_error("tensor iterator stay on sub tensor, not value, can not get value");
	}
	throw std::runtime_error("tensor iterator stay on default values, not value, can not get value");
}

template <typename Element>
std::pair<Element, int> Tensor<Element>::Iterator::getDefaultValues()
{
	if (_end) throw std::runtime_error("tensor iterator stay on end");
	auto dataTop = _data.top();
	if (dataTop->getBeginDim() == -1)
		throw
			std::runtime_error("tensor iterator stay on value, not default values, can not get default values");
	auto idx = _index.top();
	auto& subs = *dataTop->getSubTensors();
	if (idx == -1)
		throw std::runtime_error(
			"tensor iterator stay on sub tensor, not default values, can not get default values");
	if (subs.size() > idx)
	{
		if (auto& sub = subs[idx]; sub->getBeginDim() == -1)
			throw
				std::runtime_error("tensor iterator stay on value, not default values, can not get default values");
		throw std::runtime_error("tensor iterator stay on sub tensor, not default values, can not get default values");
	}
	auto allocated = dataTop->getSpaceAllocated();
	auto all = dataTop->getDimCapacity(-1);
	return {dataTop->tensorBelong()->defaultValue(), all - allocated};
}

template <typename Element>
const TensorData<Element>* Tensor<Element>::Iterator::getSubTensor()
{
	if (_end) throw std::runtime_error("tensor iterator stay on end");
	auto dataTop = _data.top();
	if (dataTop->getBeginDim() == -1)
		throw
			std::runtime_error("tensor iterator stay on value, not sub tensor, can not get sub tensor");
	auto idx = _index.top();
	auto& subs = *dataTop->getSubTensors();
	if (idx == -1) return dataTop;
	if (subs.size() > idx)
	{
		auto& sub = subs[idx];
		if (sub->getBeginDim() == -1)
			throw
				std::runtime_error("tensor iterator stay on value, not sub tensor, can not get sub tensor");
		return sub;
	}
	throw std::runtime_error("tensor iterator stay on default values, not sub tensor, can not get sub tensor");
}

template <typename Element>
typename Tensor<Element>::Iterator& Tensor<Element>::Iterator::operator++()
{
	if (_end) return *this;
	auto dataTop = _data.top();
	if (dataTop->getBeginDim() == -1)
	{
		_end = true;
		return *this;
	}
	auto idx = _index.top();
	if (idx == -1)
	{
		_index.pop();
		_data.pop();
		int size = _data.top()->getSubTensors()->size();
		auto allocated = _data.top()->getSpaceAllocated();
		auto all = _data.top()->getDimCapacity(-1);
		idx = _index.top();
		_index.pop();
		if ((((idx + 1 == size) && (all <= allocated)) || (idx + 1 > size)))
		{
			if (_index.empty())
			{
				_end = true;
				return *this;
			}
			_index.push(-1);
			return *this;
		}
		_index.emplace(idx + 1);
		return *this;
	}
	auto& subs = *dataTop->getSubTensors();
	if (subs.size() > idx)
	{
		auto& sub = subs[idx];
		if (sub->getBeginDim() != -1)
		{
			_index.emplace(0);
			_data.emplace(sub);
			return *this;
		}
	}
	_index.pop();
	int size = _data.top()->getSubTensors()->size();
	auto allocated = dataTop->getSpaceAllocated();
	auto all = dataTop->getDimCapacity(-1);
	if ((idx + 1 < size) || (idx + 1 == size && all > allocated))
		_index.emplace(idx + 1);
	else
	{
		if (_index.empty())
		{
			_end = true;
			return *this;
		}
		_index.emplace(-1);
	}
	return *this;
}

template <typename Element>
bool Tensor<Element>::Iterator::isEnd() const
{
	return _end;
}

template <typename Element>
typename Tensor<Element>::Iterator Tensor<Element>::getIterator() const
{
	return Iterator{this};
}

template <typename Element>
PlainTensor<Element>* Tensor<Element>::toPlain(int gate) const
{
	auto p = new PlainTensor<Element>{};
	p->_defaultValue = _defaultValue;
	std::vector<Element>* v = nullptr;
	int len = 0;
	for (auto it = getIterator(); !it.isEnd(); ++it)
	{
		switch (it.getCurrentIterateType())
		{
			case TensorIterateType::VALUE:
				{
					auto value = it.getValue();
					if (value == _defaultValue)
					{
						len++;
						if (len > gate && v != nullptr)
						{
							auto n = new typename PlainTensor<Element>::PlainTensorSegment{v};
							p->_segments.emplace_back(n);
							v = nullptr;
						}
					}
					else
					{
						if (len > 0)
						{
							if (len > gate)
							{
								auto n = new typename PlainTensor<Element>::PlainTensorSegment{len};
								p->_segments.emplace_back(n);
								len = 0;
							}
							else
							{
								if (v == nullptr) v = new std::vector<Element>{};
								for (; len > 0; len--) v->emplace_back(_defaultValue);
							}
						}
						if (v == nullptr) v = new std::vector<Element>{};
						v->emplace_back(value);
					}
					break;
				}
			case TensorIterateType::DEFAULT_VALUES:
				{
					auto [l, r] = it.getDefaultValues();
					len += r;
					if (len > gate && v != nullptr)
					{
						auto n = new typename PlainTensor<Element>::PlainTensorSegment{v};
						p->_segments.emplace_back(n);
						v = nullptr;
					}
					break;
				}
		}
	}
	if (len > 0)
	{
		if (len > gate)
		{
			auto n = new typename PlainTensor<Element>::PlainTensorSegment{len};
			p->_segments.emplace_back(n);
		}
		else
		{
			if (v == nullptr) v = new std::vector<Element>{};
			for (; len > 0; len--) v->emplace_back(_defaultValue);
		}
	}
	if (v != nullptr)
	{
		auto n = new typename PlainTensor<Element>::PlainTensorSegment{v};
		p->_segments.emplace_back(n);
		v = nullptr;
	}
	return p;
}

template <typename Element>
template <typename Target>
PlainTensor<Target>* Tensor<Element>::toPlain(std::function<Target(const Element&)> func, int gate) const
{
	auto p = new PlainTensor<Target>{};
	p->_defaultValue = func(_defaultValue);
	std::vector<Target>* v = nullptr;
	int len = 0;
	for (auto it = getIterator(); !it.isEnd(); ++it)
	{
		switch (it.getCurrentIterateType())
		{
			case TensorIterateType::VALUE:
				{
					auto value = it.getValue();
					auto trans = func(value);
					if (trans == p->_defaultValue)
					{
						len++;
						if (len > gate && v != nullptr)
						{
							auto n = new typename PlainTensor<Target>::PlainTensorSegment{v};
							p->_segments.emplace_back(n);
							v = nullptr;
						}
					}
					else
					{
						if (len > 0)
						{
							if (len > gate)
							{
								auto n = new typename PlainTensor<Target>::PlainTensorSegment{len};
								p->_segments.emplace_back(n);
								len = 0;
							}
							else
							{
								if (v == nullptr) v = new std::vector<Target>{};
								for (; len > 0; len--) v->emplace_back(p->_defaultValue);
							}
						}
						if (v == nullptr) v = new std::vector<Target>{};
						v->emplace_back(func(value));
					}
					break;
				}
			case TensorIterateType::DEFAULT_VALUES:
				{
					auto [l, r] = it.getDefaultValues();
					len += r;
					if (len > gate && v != nullptr)
					{
						auto n = new typename PlainTensor<Target>::PlainTensorSegment{v};
						p->_segments.emplace_back(n);
						v = nullptr;
					}
					break;
				}
		}
	}
	if (len > 0)
	{
		if (len > gate)
		{
			auto n = new typename PlainTensor<Target>::PlainTensorSegment{len};
			p->_segments.emplace_back(n);
		}
		else
		{
			if (v == nullptr) v = new std::vector<Target>{};
			for (; len > 0; len--) v->emplace_back(p->_defaultValue);
		}
	}
	if (v != nullptr)
	{
		auto n = new typename PlainTensor<Target>::PlainTensorSegment{v};
		p->_segments.emplace_back(n);
		v = nullptr;
	}
	return p;
}

template <typename Element>
TensorData<Element>::~TensorData()
{
	if (_beginDim != -1)
	{
		for (auto& i : *_data._sub_tensors) delete i;
		delete _data._sub_tensors;
	}
}

template <typename Element>
Tensor<Element>* TensorData<Element>::tensorBelong() const
{
	return _tensor;
}

template <typename Element>
int TensorData<Element>::getBeginDim() const
{
	return _beginDim;
}

template <typename Element>
bool TensorData<Element>::append(const Element& element)
{
	if (_beginDim == -1)
	{
		if (_size == 0)
		{
			_data._data = element;
			_size = 1;
			return true;
		}
		return false;
	}
	if (_size < getDimCapacity(-1))
	{
		auto data = new TensorData{_tensor, -1};
		data->_data._data = element;
		data->_size = 1;
		_data._sub_tensors->emplace_back(data);
		_size++;
		return true;
	}
	return false;
}

template <typename Element>
TensorData<Element>* TensorData<Element>::makeSubTensor()
{
	// 维度小于等于 1
	if (static_cast<int>(_tensor->_shape.size()) <= 1 + _beginDim) return nullptr;
	int nextBegin = _beginDim + 1;
	int currentDataTake = _size / _tensor->_dimLen[nextBegin - 1];
	while (_size - currentDataTake * _tensor->_dimLen[nextBegin - 1] != 0)
	{
		nextBegin++;
		// 不能整除
		if (nextBegin >= static_cast<int>(_tensor->_shape.size())) return nullptr;
		currentDataTake = _size / _tensor->_dimLen[nextBegin - 1];
	}
	// 满了
	if (currentDataTake + 1 > _tensor->_shape[nextBegin - 1])
		return nullptr;
	auto ret = new TensorData{_tensor, nextBegin};
	_data._sub_tensors->emplace_back(ret);
	_size += ret->getDimCapacity(-1);
	return ret;
}

template <typename Element>
int TensorData<Element>::getDimCapacity(const int dim) const
{
	if (dim < 0) return _beginDim == -1 ? 1 : _tensor->_shape[_beginDim] * _tensor->_dimLen[_beginDim];
	if (dim + _beginDim >= static_cast<int>(_tensor->_dimLen.size()))
		throw error::TensorDimIndex(
			static_cast<int>(_tensor->_dimLen.size()), dim);
	return _tensor->_dimLen[dim + _beginDim];
}

template <typename Element>
int TensorData<Element>::getSpaceAllocated() const
{
	return _size;
}

template <typename Element>
std::string TensorData<Element>::toString(const std::function<std::string(const Element&)>& outFunc) const
{
	if (_beginDim == -1)
	{
		if (_size == 0) return outFunc(_tensor->defaultValue());
		return outFunc(_data._data);
	}
	std::string ret = "{";
	for (auto& i : *_data._sub_tensors) ret += i->toString(outFunc) + ", ";
	if (ret.back() == ' ')
	{
		ret.pop_back();
		ret.pop_back();
	}
	ret += "}";
	return ret;
}

template <typename Element>
Element TensorData<Element>::getElement(const std::vector<int>& index, const bool strict) const
{
	if (index.size() + _beginDim != _tensor->_dimLen.size())
		throw
			std::runtime_error("Tensor's index should have same dimension with shape.");
	const int size = static_cast<int>(index.size());
	if (strict)
		for (int i = 0; i < size; i++)
		{
			if (index[i] >= _tensor->_shape[i + _beginDim])
				throw
					std::runtime_error(
						"Tensor index out of bound at dimension " + std::to_string(i) + ". max " +
						std::to_string(_tensor->_shape[i + _beginDim]) + " get " + std::to_string(index[i]));
		}
	int totalIdx = 0;
	for (int i = 0; i < size; i++)
	{
		totalIdx += index[i] * _tensor->_dimLen[i + _beginDim];
	}
	return getElement(totalIdx);
}

template <typename Element>
Element TensorData<Element>::getElement(int index) const
{
	if (index < 0 || index >= getDimCapacity(-1)) throw std::runtime_error("Tensor index out of bound");
	if (_beginDim == -1)
		return _size == 0 ? _tensor->defaultValue() : _data._data;
	if (index >= _size) return _tensor->defaultValue();
	std::vector<TensorData*>& vec = *_data._sub_tensors;
	for (auto& i : vec)
	{
		const int size = i->getDimCapacity(-1);
		if (index < size) return i->getElement(index);
		index -= size;
	}
	throw std::runtime_error("Unexpected Error Occur");
}

template <typename Element>
const std::vector<TensorData<Element>*>* TensorData<Element>::getSubTensors() const
{
	return _beginDim == -1 ? nullptr : _data._sub_tensors;
}

template <typename Element>
TensorData<Element>::TensorData(Tensor<Element>* parent, int beginDim)
{
	_tensor = parent;
	_beginDim = beginDim;
	if (beginDim != -1) _data._sub_tensors = new std::vector<TensorData*>{};
	_size = 0;
}
