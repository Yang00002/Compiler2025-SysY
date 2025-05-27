#pragma once

#include <iostream>
#include <vector>
#include <functional>
#include <string>


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
	Element defaultValue_;
	// 每个维度的大小
	std::vector<int> shape_;
	// 每个维度空间
	std::vector<int> dimLen_;
	// 实际存储数据
	TensorData<Element>* data_;

public:
	~Tensor();
	// 张量形状, 当为 [] 代表是标量; 否则, 任何一维都应该是正整数.
	const std::vector<int>& getShape();
	explicit Tensor(const std::vector<int>& shape, const Element& defaultV);
	explicit Tensor(const std::vector<unsigned>& shape, const Element& defaultV);
	// 每一维占用的空间, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	// 小于 0 的数输出张量总大小, 大于等于维度总数的数输出 -1
	[[nodiscard]] int getDimSize(int dim) const;
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量存储的数据
	[[nodiscard]] TensorData<Element>* getData();
	// 获取张量存储的数据
	[[nodiscard]] const TensorData<Element>* visitData() const;
	// 获取默认值
	[[nodiscard]] Element defaultValue() const;
};

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
	Tensor<Element>* tensor_;
	int beginDim_;
	std::vector<TensorData*> sub_tensors_;
	std::vector<Element> data_;

public:
	~TensorData();
	// 子张量所属父亲
	[[nodiscard]] Tensor<Element>* tensorBelong() const;
	// 子张量的第一维所在父亲中的维度, 对标量而言是 -1
	[[nodiscard]] int getBeginDim() const;
	bool append(Element element);
	// 尝试分配一个子张量, 以进行聚合初始化.
	// 只有张量维度大于 1, 前面的填充值数量刚好是第一维空间的倍数, 且拥有空间时能够分配成功, 否则返回 nullptr
	TensorData* makeSubTensor();
	// 每一维占用的空间, 例如 Tensor[4][3][2], 0 维占用 6, 1 维占用 2, 2 维占用 1.
	// 小于 0 的数输出张量总大小, 大于等于维度总数的数输出 -1
	[[nodiscard]] int getDimSize(int dim) const;
	std::string toString(const std::function<std::string(const Element&)>& outFunc) const;
	// 获取张量的一个元素, 标记 strict 代表任意维度都不允许越界, 否则只要总体上不越界就不会抛出异常
	[[nodiscard]] Element getElement(const std::vector<int>& index, bool strict = false) const;
	// 获取张量的子张量. 例如 a[4][3] 有 a[0], a[1], a[2], a[3] 4 个子张量.
	// 该方法只能获得分配了的子张量. 例如 a[3][2] = [[1, 2], 2, 3, 4] 则只能获得 [1, 2], 剩下 {2, 3, 4} 位于 data 中
	// 对于 a[3][2] = [1, 2, [3, 4], 5], 可以得到 [1, 2], [3, 4]
	[[nodiscard]] const std::vector<TensorData>& getSubTensors() const;
	// 获取附加进张量的数据.
	// 例如 a[3][2] = [[1, 2], 2, 3, 4], 可以得到 {2, 3, 4}
	// 对于 a[3][2] = [1, 2, [3, 4], 5], 只能得到 {5}
	[[nodiscard]] const std::vector<TensorData>& getAppendedData() const;

private:
	explicit TensorData(Tensor<Element>* parent, const std::vector<Element>& data, int beginDim);
};

template <typename Element>
Tensor<Element>::~Tensor()
{
	delete data_;
}

template <typename Element>
const std::vector<int>& Tensor<Element>::getShape()
{
	return shape_;
}

template <typename Element>
Tensor<Element>::Tensor(const std::vector<int>& shape, const Element& defaultV)
{
	defaultValue_ = defaultV;
	shape_ = shape;
	dimLen_.resize(shape_.size());
	int i = static_cast<int>(shape.size()) - 1;
	if (i >= 0)
	{
		dimLen_[i] = 1;
		for (; i > 0; --i)
		{
			if (shape_[i] <= 0) throw std::runtime_error("Tensor can not have a dimension size of 0!");
			dimLen_[i - 1] = dimLen_[i] * shape_[i];
		}
		if (shape_[0] <= 0) throw std::runtime_error("Tensor can not have a dimension size of 0!");
		data_ = new TensorData<Element>{ this, {}, 0 };
	}
	else
		data_ = new TensorData<Element>{ this, {}, -1 };
}

template <typename Element>
Tensor<Element>::Tensor(const std::vector<unsigned>& shape, const Element& defaultV)
{
	defaultValue_ = defaultV;
	for (auto& i : shape) shape_.emplace_back(static_cast<int>(i));
	dimLen_.resize(shape_.size());
	int i = static_cast<int>(shape.size()) - 1;
	if (i >= 0)
	{
		dimLen_[i] = 1;
		for (; i > 0; --i)
		{
			if (shape_[i] <= 0) throw std::runtime_error("Tensor can not have a dimension size of 0!");
			dimLen_[i - 1] = dimLen_[i] * shape_[i];
		}
		if (shape_[0] <= 0) throw std::runtime_error("Tensor can not have a dimension size of 0!");
		data_ = new TensorData<Element>{ this, {}, 0 };
	}
	else
		data_ = new TensorData<Element>{ this, {}, -1 };
}

template <typename Element>
int Tensor<Element>::getDimSize(const int dim) const
{
	if (dim < 0) return shape_.empty() ? 1 : shape_[0] * dimLen_[0];
	if (dim >= static_cast<int>(dimLen_.size())) return -1;
	return dimLen_[dim];
}

template <typename Element>
std::string Tensor<Element>::toString(const std::function<std::string(const Element&)>& outFunc) const
{
	return data_->toString(outFunc);
}

template <typename Element>
TensorData<Element>* Tensor<Element>::getData()
{
	return data_;
}

template <typename Element>
const TensorData<Element>* Tensor<Element>::visitData() const
{
	return data_;
}

template <typename Element>
Element Tensor<Element>::defaultValue() const
{
	return defaultValue_;
}

template <typename Element>
TensorData<Element>::~TensorData()
{
	for (auto& i : sub_tensors_) delete i;
}

template <typename Element>
Tensor<Element>* TensorData<Element>::tensorBelong() const
{
	return tensor_;
}

template <typename Element>
int TensorData<Element>::getBeginDim() const
{
	return beginDim_;
}

template <typename Element>
bool TensorData<Element>::append(Element element)
{
	if (beginDim_ == -1)
	{
		if (data_.empty())
		{
			data_.emplace_back(element);
			return true;
		}
		return false;
	}
	const int current = static_cast<int>(sub_tensors_.size()) * tensor_->dimLen_[beginDim_] + static_cast<int>(data_.
		                    size());
	if (current < tensor_->dimLen_[beginDim_] * tensor_->shape_[beginDim_])
	{
		data_.emplace_back(element);
		return true;
	}
	return false;
}

template <typename Element>
TensorData<Element>* TensorData<Element>::makeSubTensor()
{
	// 维度小于等于 1
	if (static_cast<int>(tensor_->shape_.size()) <= 1 + beginDim_) return nullptr;
	const int dataTake = static_cast<int>(data_.size()) / tensor_->dimLen_[beginDim_];
	// 前面的填充值数量不是 getSubTensorSize 倍数
	if (static_cast<int>(data_.size()) - dataTake * tensor_->dimLen_[beginDim_] != 0) return nullptr;
	// 满了
	if (dataTake + static_cast<int>(sub_tensors_.size()) >= tensor_->shape_[beginDim_]) return nullptr;
	for (int i = 0; i < dataTake; i++)
	{
		auto tensor = new TensorData{
			tensor_,
			{data_.begin() + i * tensor_->dimLen_[beginDim_], data_.begin() + (i + 1) * tensor_->dimLen_[beginDim_]},
			1 + beginDim_
		};
		sub_tensors_.emplace_back(tensor);
	}
	data_.clear();
	auto ret = new TensorData{tensor_, {}, 1 + beginDim_};
	sub_tensors_.emplace_back(ret);
	return ret;
}

template <typename Element>
int TensorData<Element>::getDimSize(const int dim) const
{
	if (dim < 0) return tensor_->shape_[beginDim_] * tensor_->dimLen_[beginDim_];
	if (dim + beginDim_ >= static_cast<int>(tensor_->dimLen_.size())) return -1;
	return tensor_->dimLen_[dim + beginDim_];
}

template <typename Element>
std::string TensorData<Element>::toString(const std::function<std::string(const Element&)>& outFunc) const
{
	if (beginDim_ == -1)
	{
		if (data_.empty()) return outFunc(tensor_->defaultValue_);
		return outFunc(data_[0]);
	}
	std::string ret = "{ ";
	for (auto& i : sub_tensors_) ret += i->toString(outFunc) + ", ";
	for (auto& i : data_) ret += outFunc(i) + ", ";
	if (!sub_tensors_.empty() || !data_.empty())
	{
		ret.pop_back();
		ret.pop_back();
	}
	ret += " }";
	return ret;
}

template <typename Element>
Element TensorData<Element>::getElement(const std::vector<int>& index, const bool strict) const
{
	if (beginDim_ == -1)
	{
		if (index.empty()) return data_.empty() ? tensor_->defaultValue_ : data_[0];
		throw std::runtime_error("Scalar Tensor do not have index.");
	}
	if (index.size() + beginDim_ != tensor_->dimLen_.size())
		throw
			std::runtime_error("Tensor's index should have same dimension with shape.");
	if (index.size() == 1) return static_cast<int>(data_.size()) > index[0] ? data_[index[0]] : tensor_->defaultValue_;
	const int size = static_cast<int>(index.size());
	if (strict)
		for (int i = 0; i < size; i++)
		{
			if (index[i] >= tensor_->shape_[i + beginDim_])
				throw
					std::runtime_error(
						"Tensor index out of bound at dimension " + std::to_string(i) + ". max " +
						std::to_string(tensor_->shape_[i + beginDim_]) + " get " + std::to_string(index[i]));
		}
	int totalIdx = 0;
	for (int i = 0; i < size; i++)
	{
		totalIdx += index[i] * tensor_->dimLen_[i + beginDim_];
	}
	if (int subLen = tensor_->dimLen_[beginDim_] * static_cast<int>(sub_tensors_.size()); totalIdx >= subLen)
		totalIdx -= subLen;
	else
	{
		int at = totalIdx / tensor_->dimLen_[beginDim_];
		auto tensor = sub_tensors_[at];
		totalIdx -= at * tensor_->dimLen_[beginDim_];
		int idx = 1;
		const int ed = static_cast<int>(tensor_->dimLen_.size()) - 1 - beginDim_;
		while (idx < ed)
		{
			subLen = tensor_->dimLen_[idx + beginDim_] * static_cast<int>(tensor->sub_tensors_.size());
			if (totalIdx >= subLen)
			{
				totalIdx -= subLen;
				if (totalIdx >= static_cast<int>(tensor->data_.size())) return tensor_->defaultValue_;
				return tensor->data_[totalIdx];
			}
			at = totalIdx / tensor_->dimLen_[idx + beginDim_];
			tensor = tensor->sub_tensors_[at];
			totalIdx -= at * tensor_->dimLen_[idx + beginDim_];
			idx++;
		}
		if (totalIdx >= static_cast<int>(tensor->data_.size())) return tensor_->defaultValue_;
		return tensor->data_[totalIdx];
	}
	if (totalIdx >= static_cast<int>(data_.size())) return tensor_->defaultValue_;
	return data_[totalIdx];
}

template <typename Element>
const std::vector<TensorData<Element>>& TensorData<Element>::getSubTensors() const
{
	return sub_tensors_;
}

template <typename Element>
const std::vector<TensorData<Element>>& TensorData<Element>::getAppendedData() const
{
	return data_;
}

template <typename Element>
TensorData<Element>::TensorData(Tensor<Element>* parent, const std::vector<Element>& data, const int beginDim)
{
	tensor_ = parent;
	data_ = data;
	beginDim_ = beginDim;
}
