#pragma once
#include <string>
#include <vector>

class PointerType;
class FuncType;
class ArrayType;

enum class TypeIDs : int8_t
{
	Void, // Void
	Integer, // Integers 32 bit
	Boolean, // Boolean 1 bit
	Function, // Functions
	Array, // Arrays
	ArrayInParameter, // Array In Parameter, 仅在 AST 部分会用到该类型, 它应该在 IR 被转换为指针
	Float, // float 32 bit

	Label, // Labels, e.g., BasicBlock
	Pointer, // 指针类型, 仅在 IR 部分会用到指针类型
	Char // llvm IR 中 mem 系列操作数类型, 正常而言, 其作为 mem 指令组的一部分, 不会被操作
};

std::string to_string(TypeIDs e);

class Type
{
	friend struct CompareArrayType;
	friend Type* basicType(TypeIDs id);

protected:
	TypeIDs _basic_type;
	explicit Type(const TypeIDs& id);

public:
	Type(const Type&) = delete;
	Type(Type&&) = delete;
	Type& operator=(const Type&) = delete;
	Type& operator=(Type&&) = delete;
	virtual ~Type() = default;
	// 对于数组, 是它的所有元素所占空间; 对于参数中数组, 是指针的大小 (64)
	virtual unsigned sizeInBitsInArm64();
	[[nodiscard]] virtual std::string toString() const;
	// IR 输出
	[[nodiscard]] virtual std::string print() const;
	// 对数组返回 Arrays, 参数中数组返回 ArrayInParameter, 函数返回 Functions, 指针返回 Pointer
	[[nodiscard]] TypeIDs getTypeID() const;
	// 是否是 Arrays 或 ArrayInParameter
	[[nodiscard]] bool isArrayType() const;
	// 是否是函数类型
	[[nodiscard]] bool isFunctionType() const;
	// 是否是指针类型
	[[nodiscard]] bool isPointerType() const;
	// 是否是复杂类型, 即数组或函数或指针类型
	[[nodiscard]] bool isComplexType() const;
	// 是否是基本类型, 即非数组非函数非指针类型
	[[nodiscard]] bool isBasicType() const;
	// 是否是基本数值类型, 即 FLOAT / INT / BOOL
	[[nodiscard]] bool isBasicValueType() const;
	// 尝试转换为数组类型
	[[nodiscard]] ArrayType* toArrayType() const;
	// 尝试转换为函数类型S
	[[nodiscard]] FuncType* toFuncType() const;
	// 尝试转换为指针类型
	[[nodiscard]] PointerType* toPointerType() const;
	// 获取包含自身的指针类型, 对于 ArrayInParameter 来说, 获取其确定的 Array 的指针类型
	[[nodiscard]] PointerType* getPointerTypeContainSelf() const;
};

namespace Types
{
	// 获取指向特定类型的指针类型
	// 值得注意的是, 所有 sysy 中分配的命名变量在 ir 全是指针
	PointerType* pointerType(Type* contained);
	// 当你需要一个函数参数里的 int a[] 时, 填写 (Integer, true, {}) 省掉空的那维
	ArrayType* arrayType(const TypeIDs& contained, bool inParameter, std::initializer_list<unsigned> dims);
	// 当你需要一个函数参数里的 int a[] 时, 填写 (Integer, true, {}) 省掉空的那维
	ArrayType* arrayType(const TypeIDs& contained, bool inParameter, const std::vector<unsigned>& dims);
	// 它是 arrayType(const TypeIDs& contained,... 的一个重载
	ArrayType* arrayType(const Type* contained, bool inParameter, std::initializer_list<unsigned> dims);
	// 它是 arrayType(const TypeIDs& contained,... 的一个重载
	ArrayType* arrayType(const Type* contained, bool inParameter, const std::vector<unsigned>& dims);
	FuncType* functionType(const TypeIDs& returnType, std::initializer_list<Type*> argTypes);
	FuncType* functionType(const TypeIDs& returnType, const std::vector<Type*>& argTypes);
	// 它是 functionType(const TypeIDs& returnType,... 的一个重载
	FuncType* functionType(const Type* returnType, std::initializer_list<Type*> argTypes);
	FuncType* functionType(const Type* returnType, const std::vector<Type*>& argTypes);
	// 它是 functionType(const TypeIDs& returnType,... 的一个无参数重载
	FuncType* functionType(const TypeIDs& returnType);
	// 它是 functionType(const TypeIDs& returnType) 的一个重载
	FuncType* functionType(const Type* returnType);
}

class ArrayType final : public Type
{
	friend ArrayType* Types::arrayType(const TypeIDs& contained, bool inParameter,
	                                   std::initializer_list<unsigned> dims);
	friend ArrayType* Types::arrayType(const TypeIDs& contained, bool inParameter, const std::vector<unsigned>& dims);
	friend struct CompareArrayType;

protected:
	TypeIDs _contained;
	std::vector<unsigned> _arrayDims;
	unsigned _size;
	ArrayType(const TypeIDs& contained, bool inParameter, std::initializer_list<unsigned> dims);
	ArrayType(const TypeIDs& contained, bool inParameter, const std::vector<unsigned>& dims);

public:
	// 对于数组, 是它的所有元素所占空间; 对于参数中数组, 是指针的大小 (64)
	unsigned sizeInBitsInArm64() override;
	[[nodiscard]] std::string toString() const override;
	[[nodiscard]] std::string print() const override;
	// 将其后面的维度单独转换为字符串, 可用于输出声明中对应的数组维度
	[[nodiscard]] std::string suffixToString() const;
	// getTypeID 一样可以区分
	[[nodiscard]] bool isInParameter() const;
	// 对于参数中数组, 去掉了空的那维, 所以导致维度可能是空的
	[[nodiscard]] const std::vector<unsigned>& dimensions() const;
	// 元素数量. 对于参数中数组, 去掉了空的那维, 如果没维度了(例如 a[]), 那就是 1
	[[nodiscard]] unsigned elementCount() const;
	// 当它是 Array 时, 将其转化为传入函数后的函数参数 ArrayInParameter 类型
	// 例如将 int[2][3] 转化为 int[][3]
	// 注意如果想要定义 int[][1], 应该调用 arrayType(Integer, true, {1}) 或 arrayType(Integer, false, {任意正数, 1})->toFuncParameter
	[[nodiscard]] ArrayType* toFuncParameter() const;
	// 获取确定了 confirmedDimension 维之后得到的类型, 如果不合法则返回 nullptr
	// 例如 confirmedDimension = 1, int[2][3] 转化为 int[3]; int[][3] 转化为 int[3]; int[2] 转化为 int
	// 转化后的类型需要调用 toFuncParameter 才能得到函数参数类型
	[[nodiscard]] Type* getSubType(int confirmedDimension) const;
	// target 类型作为参数的函数, 是否能够传入本类型或本类型的一部分作为参数
	// 例如 target = a[][2] 可以传入 a[4][3][2] 的一部分 a[m]; 可以传入 a[][1]; 不能传入 a[1]
	[[nodiscard]] bool canHaveFuncParameterOf(const ArrayType* target) const;
	// target 类型作为参数的函数, 是否能够传入本类型作为参数
	// 例如 target = a[][2] 不能传入 a[4][3][2] 的一部分 a[m]; 可以传入 a[][1]; 不能传入 a[1]
	[[nodiscard]] bool canPassToFuncOf(const ArrayType* target) const;
	// 数组内含基本类型, INT 或  FLOAT
	[[nodiscard]] Type* typeContained() const;
};

class FuncType final : public Type
{
	friend FuncType* Types::functionType(const TypeIDs& returnType, std::initializer_list<Type*> argTypes);
	friend FuncType* Types::functionType(const TypeIDs& returnType, const std::vector<Type*>& argTypes);
	friend struct CompareFuncType;
	Type* _returnType;
	std::vector<Type*> _argTypes;
	FuncType(Type* returnType, std::initializer_list<Type*> argTypes);
	FuncType(Type* returnType, const std::vector<Type*>& argTypes);

public:
	[[nodiscard]] std::string toString() const override;
	// 只可能是 Void, Integer, Float
	[[nodiscard]] Type* returnType() const;
	// 返回参数列表
	[[nodiscard]] const std::vector<Type*>& argumentTypes() const;
	// 返回指定参数
	[[nodiscard]] Type* argumentType(int i) const;
	// 参数数量
	[[nodiscard]] unsigned argumentCount() const;
};

class PointerType final : public Type
{
	friend PointerType* Types::pointerType(Type* contained);
	friend struct ComparePointerType;

protected:
	Type* _contained;
	PointerType(Type* contained);

public:
	[[nodiscard]] std::string print() const override;
	// 理论上该函数没有任何作用
	[[nodiscard]] std::string toString() const override;
	// 理论上该函数没有任何作用
	[[nodiscard]] std::string suffixToString() const;
	// 获取内含类型
	[[nodiscard]] Type* typeContained() const;
};

namespace Types
{
	extern Type* VOID;
	extern Type* LABEL;
	extern Type* INT;
	extern Type* BOOL;
	extern Type* FLOAT;
	extern Type* CHAR;
	// 获得 TypeIDs 对应的 simpleType, 对于复合类型的 TypeIDs 返回 nullptr
	Type* simpleType(const TypeIDs& contained);
}
