#pragma once
#include <vector>
#include <functional>
#include <stdexcept>
#include <string>


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
	const std::vector<int>& getShape();
	explicit Tensor(const std::vector<int>& shape, const Element& defaultV);
	explicit Tensor(const std::vector<unsigned>& shape, const Element& defaultV);
	// 每一维的容量, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	// 小于 0 的数输出张量总容量
	[[nodiscard]] int getDimCapacity(int dim) const;
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量存储的数据
	[[nodiscard]] TensorData<Element>* getData();
	// 获取张量存储的数据
	[[nodiscard]] const TensorData<Element>* visitData() const;
	// 获取默认值
	[[nodiscard]] Element defaultValue() const;
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
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量的一个元素, 标记 strict 代表任意维度都不允许越界, 否则只要总体上不越界就不会抛出异常
	[[nodiscard]] Element getElement(const std::vector<int>& index, bool strict = false) const;
	// 获取张量的一个元素
	[[nodiscard]] Element getElement(int index) const;
	// 获取张量的子张量.
	[[nodiscard]] const std::vector<TensorData>* getSubTensors() const;

private:
	explicit TensorData(Tensor<Element>* parent, int beginDim);
};

template <typename Element>
Tensor<Element>::~Tensor()
{
	delete _data;
}

template <typename Element>
const std::vector<int>& Tensor<Element>::getShape()
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
	if (dim + _beginDim >= static_cast<int>(_tensor->_dimLen.size()))  throw error::TensorDimIndex(static_cast<int>(_tensor->_dimLen.size()), dim);
	return _tensor->_dimLen[dim + _beginDim];
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
const std::vector<TensorData<Element>>* TensorData<Element>::getSubTensors() const
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
