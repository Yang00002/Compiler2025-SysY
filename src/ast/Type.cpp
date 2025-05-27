#include "../../include/ast/Type.hpp"

#include <iostream>
#include <set>


namespace
{
	unsigned basicTypeSizeInBitesInArm64(const TypeIDs& type)
	{
		switch (type)
		{
			case TypeIDs::Void:
			case TypeIDs::Function:
			case TypeIDs::Label: return 0;
			case TypeIDs::Integer: return 32;
			case TypeIDs::Boolean: return 1;
			case TypeIDs::Array: return 0;
			case TypeIDs::ArrayInParameter: return 64;
			case TypeIDs::Float: return 32;
		}
		return 0;
	}
}

struct CompareArrayType
{
	bool operator()(const ArrayType* a, const ArrayType* b) const
	{
		if (a->basic_type_ < b->basic_type_) return true;
		if (a->basic_type_ > b->basic_type_) return false;
		if (a->contained_ < b->contained_) return true;
		if (a->contained_ > b->contained_) return false;
		const size_t len1 = a->arrayDims_.size();
		const size_t len2 = b->arrayDims_.size();
		if (len1 < len2) return true;
		if (len1 > len2) return false;
		for (size_t i = 0; i < len1; i++)
		{
			auto& n1 = a->arrayDims_[i];
			auto& n2 = b->arrayDims_[i];
			if (n1 < n2) return true;
			if (n1 > n2) return false;
		}
		return false;
	}
};

struct CompareFuncType
{
	bool operator()(const FuncType* a, const FuncType* b) const
	{
		if (a->returnType_ < b->returnType_) return true;
		if (a->returnType_ > b->returnType_) return false;
		const size_t len1 = a->argTypes_.size();
		const size_t len2 = b->argTypes_.size();
		if (len1 < len2) return true;
		if (len1 > len2) return false;
		for (size_t i = 0; i < len1; i++)
		{
			auto& n1 = a->argTypes_[i];
			auto& n2 = b->argTypes_[i];
			if (n1 < n2) return true;
			if (n1 > n2) return false;
		}
		return false;
	}
};

class TypeAllocator
{
public:
	std::vector<Type*> basicTypes_;
	std::set<ArrayType*, CompareArrayType> arrayTypes_;
	std::set<FuncType*, CompareFuncType> funcTypes_;
	TypeAllocator() = default;
	TypeAllocator(const TypeAllocator& other) = delete;
	TypeAllocator(TypeAllocator&& other) = delete;
	TypeAllocator& operator=(const TypeAllocator& other) = delete;
	TypeAllocator& operator=(TypeAllocator&& other) = delete;

	~TypeAllocator()
	{
		for (const auto& i : basicTypes_) delete i;
		for (const auto& i : arrayTypes_) delete i;
		for (const auto& i : funcTypes_) delete i;
	}
};

namespace
{
	TypeAllocator allocator{};
}

std::string to_string(const TypeIDs e)
{
	switch (e)
	{
		case TypeIDs::Void: return "void";
		case TypeIDs::Label: return "label";
		case TypeIDs::Integer: return "int";
		case TypeIDs::Boolean: return "bool";
		case TypeIDs::Function: return "function";
		case TypeIDs::Array: return "array";
		case TypeIDs::ArrayInParameter: return "arrayInParameter";
		case TypeIDs::Float: return "float";
	}
	return "unknown";
}

Type::Type(const TypeIDs& id)
{
	basic_type_ = id;
}

unsigned Type::sizeInBitsInArm64()
{
	return basicTypeSizeInBitesInArm64(basic_type_);
}

std::string Type::toString() const
{
	return to_string(basic_type_);
}

TypeIDs Type::getTypeID() const
{
	return basic_type_;
}

bool Type::isArrayType() const
{
	return basic_type_ == TypeIDs::Array || basic_type_ == TypeIDs::ArrayInParameter;
}

ArrayType::ArrayType(const TypeIDs& contained, const bool inParameter,
                     const std::initializer_list<unsigned> dims): Type(
	inParameter ? TypeIDs::ArrayInParameter : TypeIDs::Array)
{
	contained_ = contained;
	// size 的拓展被在 arrayType 中推后计算, 以提高性能
	size_ = basicTypeSizeInBitesInArm64(contained_);
	arrayDims_ = dims;
}

ArrayType::ArrayType(const TypeIDs& contained, const bool inParameter, const std::vector<unsigned>& dims) : Type(
	inParameter ? TypeIDs::ArrayInParameter : TypeIDs::Array)
{
	contained_ = contained;
	// size 的拓展被在 arrayType 中推后计算, 以提高性能
	size_ = basicTypeSizeInBitesInArm64(contained_);
	arrayDims_ = dims;
}

unsigned ArrayType::sizeInBitsInArm64()
{
	return size_;
}

std::string ArrayType::toString() const
{
	std::string base = to_string(contained_);
	if (basic_type_ == TypeIDs::ArrayInParameter) base += "[]";
	for (const auto& i : arrayDims_)
	{
		base += std::string("[") + std::to_string(i) + "]";
	}
	return base;
}

std::string ArrayType::suffixToString() const
{
	std::string base;
	if (basic_type_ == TypeIDs::ArrayInParameter) base += "[]";
	for (const auto& i : arrayDims_)
	{
		base += std::string("[") + std::to_string(i) + "]";
	}
	return base;
}

bool ArrayType::isInParameter() const
{
	return basic_type_ == TypeIDs::ArrayInParameter;
}

const std::vector<unsigned>& ArrayType::dimensions() const
{
	return arrayDims_;
}

unsigned ArrayType::elementCount() const
{
	return size_ / basicTypeSizeInBitesInArm64(contained_);
}

ArrayType* ArrayType::toFuncParameter() const
{
	if (basic_type_ == TypeIDs::Array)
		return Types::arrayType(contained_, true, {arrayDims_.begin() + 1, arrayDims_.end()});
	return const_cast<ArrayType*>(this);
}

Type* ArrayType::getSubType(const int confirmedDimension) const
{
	if (basic_type_ == TypeIDs::Array)
	{
		if (confirmedDimension == 0) return const_cast<ArrayType*>(this);
		const auto maxDim = static_cast<int>(arrayDims_.size());
		if (confirmedDimension == maxDim) return Types::simpleType(contained_);
		if (confirmedDimension > maxDim) return nullptr;
		return Types::arrayType(contained_, false, {arrayDims_.begin() + confirmedDimension, arrayDims_.end()});
	}
	if (confirmedDimension == 0) return const_cast<ArrayType*>(this);
	const auto maxDim = static_cast<int>(arrayDims_.size() + 1);
	if (confirmedDimension == maxDim) return Types::simpleType(contained_);
	if (confirmedDimension > maxDim) return nullptr;
	return Types::arrayType(contained_, false, {arrayDims_.begin() + confirmedDimension - 1, arrayDims_.end()});
}

bool ArrayType::canHaveFuncParameterOf(const ArrayType* target) const
{
	// 函数参数不能是 Array, 只能说 ArrayInParameter
	if (target->basic_type_ == TypeIDs::Array) return false;
	if (target->contained_ != contained_) return false;
	auto myTestDim = arrayDims_.size();
	if (basic_type_ == TypeIDs::Array) myTestDim--;
	if (myTestDim < target->arrayDims_.size()) return false;
	if (target->arrayDims_.empty()) return true;
	auto i = arrayDims_.size();
	auto j = target->arrayDims_.size();
	while (j > 0)
	{
		j--;
		i--;
		if (arrayDims_[i] != target->arrayDims_[j]) return false;
	}
	return true;
}

bool ArrayType::canPassToFuncOf(const ArrayType* target) const
{
	// 函数参数不能是 Array, 只能说 ArrayInParameter
	if (target->basic_type_ == TypeIDs::Array) return false;
	if (target->contained_ != contained_) return false;
	auto myTestDim = arrayDims_.size();
	if (basic_type_ == TypeIDs::Array) myTestDim--;
	if (myTestDim != target->arrayDims_.size()) return false;
	if (target->arrayDims_.empty()) return true;
	auto i = arrayDims_.size();
	auto j = target->arrayDims_.size();
	while (j > 0)
	{
		j--;
		i--;
		if (arrayDims_[i] != target->arrayDims_[j]) return false;
	}
	return true;
}

Type* ArrayType::typeContained() const
{
	return Types::simpleType(contained_);
}

FuncType::FuncType(const TypeIDs& returnType, const std::initializer_list<Type*> argTypes): Type(TypeIDs::Function)
{
	returnType_ = returnType;
	argTypes_ = argTypes;
}

FuncType::FuncType(const TypeIDs& returnType, const std::vector<Type*>& argTypes) : Type(TypeIDs::Function)
{
	returnType_ = returnType;
	argTypes_ = argTypes;
}

std::string FuncType::toString() const
{
	std::string ret = to_string(returnType_);
	ret += " " + to_string(basic_type_) + "(";
	for (const auto& i : argTypes_)
	{
		ret += i->toString() + ",";
	}
	ret.pop_back();
	ret += ")";
	return ret;
}

TypeIDs FuncType::returnType() const
{
	return returnType_;
}

const std::vector<Type*>& FuncType::argumentTypes() const
{
	return argTypes_;
}

unsigned FuncType::argumentCount() const
{
	return static_cast<unsigned>(argTypes_.size());
}

Type* Types::simpleType(const TypeIDs& contained)
{
	switch (contained)
	{
		case TypeIDs::Void: return VOID;
		case TypeIDs::Label: return LABEL;
		case TypeIDs::Integer: return INT;
		case TypeIDs::Boolean: return BOOL;
		case TypeIDs::Function:
		case TypeIDs::Array:
		case TypeIDs::ArrayInParameter: return nullptr;
		case TypeIDs::Float: return FLOAT;
	}
	return nullptr;
}

Type* basicType(const TypeIDs id)
{
	if (id != TypeIDs::Array && id != TypeIDs::ArrayInParameter && id != TypeIDs::Function)
	{
		auto* t = new Type{id};
		allocator.basicTypes_.emplace_back(t);
		return t;
	}
	throw std::runtime_error("Basic Type can not have ID of " + to_string(id));
}

namespace Types
{
	Type* VOID = basicType(TypeIDs::Void);
	Type* LABEL = basicType(TypeIDs::Label);
	Type* INT = basicType(TypeIDs::Integer);
	Type* BOOL = basicType(TypeIDs::Boolean);
	Type* FLOAT = basicType(TypeIDs::Float);

	ArrayType* arrayType(const TypeIDs& contained, const bool inParameter, const std::initializer_list<unsigned> dims)
	{
		if (contained == TypeIDs::Float || contained == TypeIDs::Integer)
		{
			if (dims.size() == 0 && !inParameter)
				throw std::runtime_error("Array Not in Parameter must have at least one certain dimension");
			auto type = new ArrayType(contained, inParameter, dims);
			const auto i = allocator.arrayTypes_.find(type);
			if (i == allocator.arrayTypes_.end())
			{
				allocator.arrayTypes_.emplace(type);
				return type;
			}
			if (type != *i)
			{
				delete type;
				return *i;
			}
			if (type->getTypeID() == TypeIDs::Array)
				for (const unsigned dim : type->arrayDims_)
					type->size_ *= dim;
			return type;
		}
		throw std::runtime_error("Array Type can not have Element ID of " + to_string(contained));
	}

	ArrayType* arrayType(const TypeIDs& contained, const bool inParameter, const std::vector<unsigned>& dims)
	{
		if (contained == TypeIDs::Float || contained == TypeIDs::Integer)
		{
			if (dims.empty() && !inParameter)
				throw std::runtime_error("Array Not in Parameter must have at least one certain dimension");
			auto type = new ArrayType(contained, inParameter, dims);
			const auto i = allocator.arrayTypes_.find(type);
			if (i == allocator.arrayTypes_.end())
			{
				allocator.arrayTypes_.emplace(type);
				return type;
			}
			if (type != *i)
			{
				delete type;
				return *i;
			}
			if (type->getTypeID() == TypeIDs::Array)
				for (const unsigned dim : type->arrayDims_)
					type->size_ *= dim;
			return type;
		}
		throw std::runtime_error("Array Type can not have Element ID of " + to_string(contained));
	}

	ArrayType* arrayType(Type* contained, const bool inParameter, const std::initializer_list<unsigned> dims)
	{
		if (contained == FLOAT || contained == INT)
		{
			return arrayType(contained->getTypeID(), inParameter, dims);
		}
		throw std::runtime_error("Array Type can not have Element ID of " + contained->toString());
	}

	ArrayType* arrayType(Type* contained, const bool inParameter, const std::vector<unsigned int>& dims)
	{
		if (contained == FLOAT || contained == INT)
		{
			return arrayType(contained->getTypeID(), inParameter, dims);
		}
		throw std::runtime_error("Array Type can not have Element ID of " + contained->toString());
	}

	FuncType* functionType(const TypeIDs& returnType, const std::initializer_list<Type*> argTypes)
	{
		if (returnType == TypeIDs::Float || returnType == TypeIDs::Integer || returnType == TypeIDs::Void)
		{
			for (const auto arg_type : argTypes)
			{
				if (arg_type != FLOAT && arg_type != INT && arg_type->getTypeID() != TypeIDs::ArrayInParameter)
				{
					throw std::runtime_error(
						"Func Type can not have argument type of " + arg_type->toString()
					);
				}
			}
			auto type = new FuncType(returnType, argTypes);
			const auto i = allocator.funcTypes_.find(type);
			if (i == allocator.funcTypes_.end())
			{
				allocator.funcTypes_.emplace(type);
				return type;
			}
			if (type != *i)
			{
				delete type;
				return *i;
			}
			return type;
		}
		throw std::runtime_error("Func Type can not have return type of " + to_string(returnType));
	}

	FuncType* functionType(const TypeIDs& returnType, const std::vector<Type*>& argTypes)
	{
		if (returnType == TypeIDs::Float || returnType == TypeIDs::Integer || returnType == TypeIDs::Void)
		{
			for (const auto arg_type : argTypes)
			{
				if (arg_type != FLOAT && arg_type != INT && arg_type->getTypeID() != TypeIDs::ArrayInParameter)
				{
					throw std::runtime_error(
						"Func Type can not have argument type of " + arg_type->toString()
					);
				}
			}
			auto type = new FuncType(returnType, argTypes);
			const auto i = allocator.funcTypes_.find(type);
			if (i == allocator.funcTypes_.end())
			{
				allocator.funcTypes_.emplace(type);
				return type;
			}
			if (type != *i)
			{
				delete type;
				return *i;
			}
			return type;
		}
		throw std::runtime_error("Func Type can not have return type of " + to_string(returnType));
	}

	FuncType* functionType(Type* returnType, const std::initializer_list<Type*> argTypes)
	{
		if (returnType == FLOAT || returnType == INT || returnType == VOID)
		{
			return functionType(returnType->getTypeID(), argTypes);
		}
		throw std::runtime_error("Func Type can not have return type of " + returnType->toString());
	}

	FuncType* functionType(Type* returnType, const std::vector<Type*>& argTypes)
	{
		if (returnType == FLOAT || returnType == INT || returnType == VOID)
		{
			return functionType(returnType->getTypeID(), argTypes);
		}
		throw std::runtime_error("Func Type can not have return type of " + returnType->toString());
	}

	FuncType* functionType(const TypeIDs& returnType)
	{
		return functionType(returnType, {});
	}

	FuncType* functionType(Type* returnType)
	{
		return functionType(returnType, {});
	}
}
