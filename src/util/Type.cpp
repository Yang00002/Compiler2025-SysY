#include "../../include/util/Type.hpp"

#include <iostream>
#include <map>
#include <set>

namespace error
{
	namespace
	{
		std::runtime_error FuncTypeArgIndex(int limit, int get)
		{
			return std::runtime_error(
				"FuncTypeArgIndex Error: Function have " + std::to_string(limit) + " arguments, index " +
				std::to_string(get) + " out of bound");
		}
	}
}


namespace
{
	unsigned basicTypeSizeInBitesInArm64(const TypeIDs& type)
	{
		switch (type)
		{
			case TypeIDs::Void:
			case TypeIDs::Function:
			case TypeIDs::Label:
			case TypeIDs::Array:
				return 0;
			case TypeIDs::Boolean: return 1;
			case TypeIDs::Float:
			case TypeIDs::Integer: return 32;
			case TypeIDs::Pointer:
			case TypeIDs::ArrayInParameter: return 64;
			case TypeIDs::Char: return 8;
		}
		return 0;
	}
}

struct CompareArrayType
{
	bool operator()(const ArrayType* a, const ArrayType* b) const
	{
		if (a->_basic_type < b->_basic_type) return true;
		if (a->_basic_type > b->_basic_type) return false;
		if (a->_contained < b->_contained) return true;
		if (a->_contained > b->_contained) return false;
		const size_t len1 = a->_arrayDims.size();
		const size_t len2 = b->_arrayDims.size();
		if (len1 < len2) return true;
		if (len1 > len2) return false;
		for (size_t i = 0; i < len1; i++)
		{
			auto& n1 = a->_arrayDims[i];
			auto& n2 = b->_arrayDims[i];
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
		if (a->_returnType < b->_returnType) return true;
		if (a->_returnType > b->_returnType) return false;
		const size_t len1 = a->_argTypes.size();
		const size_t len2 = b->_argTypes.size();
		if (len1 < len2) return true;
		if (len1 > len2) return false;
		for (size_t i = 0; i < len1; i++)
		{
			auto& n1 = a->_argTypes[i];
			auto& n2 = b->_argTypes[i];
			if (n1 < n2) return true;
			if (n1 > n2) return false;
		}
		return false;
	}
};

struct ComparePointerType
{
	bool operator()(const PointerType* a, const PointerType* b) const
	{
		return a->_contained < b->_contained;
	}
};

class TypeAllocator
{
public:
	std::vector<Type*> basicTypes_;
	std::set<ArrayType*, CompareArrayType> arrayTypes_;
	std::set<FuncType*, CompareFuncType> funcTypes_;
	std::set<PointerType*, ComparePointerType> pointerTypes_;
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
		for (const auto& i : pointerTypes_) delete i;
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
		case TypeIDs::Pointer: return "pointer";
		case TypeIDs::Char: return "char";
	}
	return "unknown";
}

Type::Type(const TypeIDs& id)
{
	_basic_type = id;
}

unsigned Type::sizeInBitsInArm64()
{
	return basicTypeSizeInBitesInArm64(_basic_type);
}

std::string Type::toString() const
{
	return to_string(_basic_type);
}

std::string Type::print() const
{
	switch (_basic_type)
	{
		case TypeIDs::Void:
			return "void";
		case TypeIDs::Label:
			return "label";
		case TypeIDs::Integer:
			return "i32";
		case TypeIDs::Boolean:
			return "i1";
		case TypeIDs::Function:
			return "function";
		case TypeIDs::Array:
		case TypeIDs::ArrayInParameter:
			return "array";
		case TypeIDs::Float:
			return "float";
		case TypeIDs::Pointer:
			return "pointer";
		case TypeIDs::Char:
			return "i8";
	}
	return "";
}

TypeIDs Type::getTypeID() const
{
	return _basic_type;
}

bool Type::isArrayType() const
{
	return _basic_type == TypeIDs::Array || _basic_type == TypeIDs::ArrayInParameter;
}

bool Type::isFunctionType() const
{
	return _basic_type == TypeIDs::Function;
}

bool Type::isPointerType() const
{
	return _basic_type == TypeIDs::Pointer;
}

bool Type::isComplexType() const
{
	return isArrayType() || isFunctionType() || isPointerType();
}

bool Type::isBasicType() const
{
	return !isComplexType();
}

bool Type::isBasicValueType() const
{
	return _basic_type == TypeIDs::Integer || _basic_type == TypeIDs::Boolean || _basic_type == TypeIDs::Float;
}

ArrayType* Type::toArrayType() const
{
	return dynamic_cast<ArrayType*>(const_cast<Type*>(this));
}

FuncType* Type::toFuncType() const
{
	return dynamic_cast<FuncType*>(const_cast<Type*>(this));
}

PointerType* Type::toPointerType() const
{
	return dynamic_cast<PointerType*>(const_cast<Type*>(this));
}

PointerType* Type::getPointerTypeContainSelf() const
{
	if (_basic_type == TypeIDs::ArrayInParameter)
		return Types::pointerType(toArrayType()->getSubType(1));
	return Types::pointerType(const_cast<Type*>(this));
}

PointerType* Types::pointerType(Type* contained)
{
	if (contained == Types::VOID || contained == Types::LABEL)
		throw std::runtime_error("void or label can not have pointer");
	auto type = new PointerType(contained);
	const auto i = allocator.pointerTypes_.find(type);
	if (i == allocator.pointerTypes_.end())
	{
		allocator.pointerTypes_.emplace(type);
		return type;
	}
	delete type;
	return *i;
}

ArrayType::ArrayType(const TypeIDs& contained, const bool inParameter,
                     const std::initializer_list<unsigned> dims): Type(
	inParameter ? TypeIDs::ArrayInParameter : TypeIDs::Array)
{
	_contained = contained;
	// size 的拓展被在 arrayType 中推后计算, 以提高性能
	_size = basicTypeSizeInBitesInArm64(_contained);
	_arrayDims = dims;
}

ArrayType::ArrayType(const TypeIDs& contained, const bool inParameter, const std::vector<unsigned>& dims) : Type(
	inParameter ? TypeIDs::ArrayInParameter : TypeIDs::Array)
{
	_contained = contained;
	// size 的拓展被在 arrayType 中推后计算, 以提高性能
	_size = basicTypeSizeInBitesInArm64(_contained);
	_arrayDims = dims;
}

unsigned ArrayType::sizeInBitsInArm64()
{
	return _size;
}

std::string ArrayType::toString() const
{
	std::string base = to_string(_contained);
	if (_basic_type == TypeIDs::ArrayInParameter) base += "[]";
	for (const auto& i : _arrayDims)
	{
		base += std::string("[") + std::to_string(i) + "]";
	}
	return base;
}

std::string ArrayType::print() const
{
	std::string base = Types::simpleType(_contained)->print();
	for (auto it = _arrayDims.rbegin(); it != _arrayDims.rend(); ++it)
	{
		base = '[' + std::to_string(*it) + " x " + base + "]";
	}
	if (_basic_type == TypeIDs::ArrayInParameter) base += "*";
	return base;
}

std::string ArrayType::suffixToString() const
{
	std::string base;
	if (_basic_type == TypeIDs::ArrayInParameter) base += "[]";
	for (const auto& i : _arrayDims)
	{
		base += std::string("[") + std::to_string(i) + "]";
	}
	return base;
}

bool ArrayType::isInParameter() const
{
	return _basic_type == TypeIDs::ArrayInParameter;
}

const std::vector<unsigned>& ArrayType::dimensions() const
{
	return _arrayDims;
}

unsigned ArrayType::elementCount() const
{
	return _size / basicTypeSizeInBitesInArm64(_contained);
}

ArrayType* ArrayType::toFuncParameter() const
{
	if (_basic_type == TypeIDs::Array)
		return Types::arrayType(_contained, true, {_arrayDims.begin() + 1, _arrayDims.end()});
	return const_cast<ArrayType*>(this);
}

Type* ArrayType::getSubType(const int confirmedDimension) const
{
	if (_basic_type == TypeIDs::Array)
	{
		if (confirmedDimension == 0) return const_cast<ArrayType*>(this);
		const auto maxDim = static_cast<int>(_arrayDims.size());
		if (confirmedDimension == maxDim) return Types::simpleType(_contained);
		if (confirmedDimension > maxDim) return nullptr;
		return Types::arrayType(_contained, false, {_arrayDims.begin() + confirmedDimension, _arrayDims.end()});
	}
	if (confirmedDimension == 0) return const_cast<ArrayType*>(this);
	const auto maxDim = static_cast<int>(_arrayDims.size() + 1);
	if (confirmedDimension == maxDim) return Types::simpleType(_contained);
	if (confirmedDimension > maxDim) return nullptr;
	return Types::arrayType(_contained, false, {_arrayDims.begin() + confirmedDimension - 1, _arrayDims.end()});
}

bool ArrayType::canHaveFuncParameterOf(const ArrayType* target) const
{
	// 函数参数不能是 Array, 只能说 ArrayInParameter
	if (target->_basic_type == TypeIDs::Array) return false;
	if (target->_contained != _contained) return false;
	auto myTestDim = _arrayDims.size();
	if (_basic_type == TypeIDs::Array) myTestDim--;
	if (myTestDim < target->_arrayDims.size()) return false;
	if (target->_arrayDims.empty()) return true;
	auto i = _arrayDims.size();
	auto j = target->_arrayDims.size();
	while (j > 0)
	{
		j--;
		i--;
		if (_arrayDims[i] != target->_arrayDims[j]) return false;
	}
	return true;
}

bool ArrayType::canPassToFuncOf(const ArrayType* target) const
{
	// 函数参数不能是 Array, 只能说 ArrayInParameter
	if (target->_basic_type == TypeIDs::Array) return false;
	if (target->_contained != _contained) return false;
	auto myTestDim = _arrayDims.size();
	if (_basic_type == TypeIDs::Array) myTestDim--;
	if (myTestDim != target->_arrayDims.size()) return false;
	if (target->_arrayDims.empty()) return true;
	auto i = _arrayDims.size();
	auto j = target->_arrayDims.size();
	while (j > 0)
	{
		j--;
		i--;
		if (_arrayDims[i] != target->_arrayDims[j]) return false;
	}
	return true;
}

Type* ArrayType::typeContained() const
{
	return Types::simpleType(_contained);
}

FuncType::FuncType(Type* returnType, const std::initializer_list<Type*> argTypes): Type(TypeIDs::Function)
{
	_returnType = returnType;
	_argTypes = argTypes;
}

FuncType::FuncType(Type* returnType, const std::vector<Type*>& argTypes) : Type(TypeIDs::Function)
{
	_returnType = returnType;
	_argTypes = argTypes;
}

std::string FuncType::toString() const
{
	std::string ret = _returnType->toString();
	ret += " " + to_string(_basic_type) + "(";
	for (const auto& i : _argTypes)
	{
		ret += i->toString() + ",";
	}
	ret.pop_back();
	ret += ")";
	return ret;
}

Type* FuncType::returnType() const
{
	return _returnType;
}

const std::vector<Type*>& FuncType::argumentTypes() const
{
	return _argTypes;
}

Type* FuncType::argumentType(const int i) const
{
	if (i < 0 || i >= static_cast<int>(_argTypes.size()))
		throw error::FuncTypeArgIndex(
			static_cast<int>(_argTypes.size()), i);
	return _argTypes[i];
}

unsigned FuncType::argumentCount() const
{
	return static_cast<unsigned>(_argTypes.size());
}

PointerType::PointerType(Type* contained) : Type(TypeIDs::Pointer)
{
	_contained = contained;
}

std::string PointerType::print() const
{
	return _contained->print() + "*";
}

std::string PointerType::toString() const
{
	if (_contained->isArrayType())
	{
		return to_string(_contained->getTypeID()) + "[]" + dynamic_cast<ArrayType*>(_contained)->suffixToString();
	}
	return _contained->toString() + "[]";
}

std::string PointerType::suffixToString() const
{
	if (_contained->isArrayType())
	{
		return "[]" + dynamic_cast<ArrayType*>(_contained)->suffixToString();
	}
	return "[]";
}

Type* PointerType::typeContained() const
{
	return _contained;
}

Type* Types::simpleType(const TypeIDs& contained)
{
	switch (contained)
	{
		case TypeIDs::Char: return CHAR;
		case TypeIDs::Void: return VOID;
		case TypeIDs::Label: return LABEL;
		case TypeIDs::Integer: return INT;
		case TypeIDs::Boolean: return BOOL;
		case TypeIDs::Pointer:
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
	Type* CHAR = basicType(TypeIDs::Char);

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
				if (type->getTypeID() == TypeIDs::Array)
					for (const unsigned dim : type->_arrayDims)
						type->_size *= dim;
				return type;
			}
			delete type;
			return *i;
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
				if (type->getTypeID() == TypeIDs::Array)
					for (const unsigned dim : type->_arrayDims)
						type->_size *= dim;
				return type;
			}
			delete type;
			return *i;
		}
		throw std::runtime_error("Array Type can not have Element ID of " + to_string(contained));
	}

	ArrayType* arrayType(const Type* contained, const bool inParameter, const std::initializer_list<unsigned> dims)
	{
		if (contained == FLOAT || contained == INT)
		{
			return arrayType(contained->getTypeID(), inParameter, dims);
		}
		throw std::runtime_error("Array Type can not have Element ID of " + contained->toString());
	}

	ArrayType* arrayType(const Type* contained, const bool inParameter, const std::vector<unsigned int>& dims)
	{
		if (contained == FLOAT || contained == INT)
		{
			return arrayType(contained->getTypeID(), inParameter, dims);
		}
		throw std::runtime_error("Array Type can not have Element ID of " + contained->toString());
	}

	FuncType* functionType(const TypeIDs& returnType, const std::initializer_list<Type*> argTypes)
	{
		if (returnType == TypeIDs::Integer || returnType == TypeIDs::Float || returnType == TypeIDs::Void)
		{
			for (const auto arg_type : argTypes)
			{
				if (arg_type != FLOAT && arg_type != INT && arg_type->getTypeID() != TypeIDs::ArrayInParameter)
				{
					if (arg_type->isPointerType())
					{
						auto ty = arg_type->toPointerType()->typeContained();
						if (ty == INT || ty == FLOAT || ty->getTypeID() == TypeIDs::Array) continue;
					}
					throw std::runtime_error(
						"Func Type can not have argument type of " + arg_type->toString()
					);
				}
			}
			auto type = new FuncType(simpleType(returnType), argTypes);
			const auto i = allocator.funcTypes_.find(type);
			if (i == allocator.funcTypes_.end())
			{
				allocator.funcTypes_.emplace(type);
				return type;
			}
			delete type;
			return *i;
		}
		throw std::runtime_error("Func Type can not have return type of " + to_string(returnType));
	}

	FuncType* functionType(const TypeIDs& returnType, const std::vector<Type*>& argTypes)
	{
		if (returnType == TypeIDs::Integer || returnType == TypeIDs::Float || returnType == TypeIDs::Void)
		{
			for (const auto arg_type : argTypes)
			{
				if (arg_type != FLOAT && arg_type != INT && arg_type->getTypeID() != TypeIDs::ArrayInParameter)
				{
					if (arg_type->isPointerType())
					{
						auto ty = arg_type->toPointerType()->typeContained();
						if (ty == INT || ty == FLOAT || ty->getTypeID() == TypeIDs::Array) continue;
					}
					throw std::runtime_error(
						"Func Type can not have argument type of " + arg_type->toString()
					);
				}
			}
			auto type = new FuncType(simpleType(returnType), argTypes);
			const auto i = allocator.funcTypes_.find(type);
			if (i == allocator.funcTypes_.end())
			{
				allocator.funcTypes_.emplace(type);
				return type;
			}
			delete type;
			return *i;
		}
		throw std::runtime_error("Func Type can not have return type of " + to_string(returnType));
	}

	FuncType* functionType(const Type* returnType, const std::initializer_list<Type*> argTypes)
	{
		if (returnType == FLOAT || returnType == INT || returnType == VOID)
		{
			return functionType(returnType->getTypeID(), argTypes);
		}
		throw std::runtime_error("Func Type can not have return type of " + returnType->toString());
	}

	FuncType* functionType(const Type* returnType, const std::vector<Type*>& argTypes)
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

	FuncType* functionType(const Type* returnType)
	{
		return functionType(returnType, {});
	}
}
