#pragma once
#include <functional>
#include <list>
#include <string>

#include "System.hpp"

class Function;
class Type;
class User;
class Use;


// Value, 对值的抽象, 继承 Value 的类具有名称, 类别以及被使用列表
class Value
{
	friend Function;
public:
	Value(const Value& other) = delete;
	Value(Value&& other) = delete;
	Value& operator=(const Value& other) = delete;
	Value& operator=(Value&& other) noexcept = delete;
	/**
	 * 
	 * @param ty 类型
	 * @param name 名称(默认为空)
	 */
	explicit Value(Type* ty, std::string name = "");
	virtual ~Value() { replace_all_use_with(nullptr); }
	// 名称
	[[nodiscard]] std::string get_name() const { return name_; }
	// 值类型
	[[nodiscard]] Type* get_type() const;
	// 该值的被使用列表
	[[nodiscard]] const std::list<Use>& get_use_list() const;
	// 设置名称
	bool set_name(const std::string& name);
	void force_set_name(const std::string& name);
	/**
	 * 标记该值在某处被使用
	 * @param user 使用者
	 * @param arg_no 使用者在第几个操作数使用它
	 */
	void add_use(User* user, int arg_no);
	/**
	 * 移除该值在某处的被使用
	 * @param user 使用者
	 * @param arg_no 使用者在第几个操作数使用它
	 */
	void remove_use(User* user, int arg_no);
	/**
	 * 将该值的所有用法替换为另一个值, 并自动维护使用情况
	 * @param new_val 替换它的新值
	 */
	void replace_all_use_with(Value* new_val) const;
	/**
	 * 将该值的满足条件的用法替换为另一个值, 并自动维护使用情况
	 * @param new_val 替换它的新值
	 * @param pred 条件
	 */
	void replace_use_with_if(Value* new_val, const std::function<bool(Use)>& pred);

	virtual std::string print() = 0;

private:
	// 该值的类型
	Type* type_;
	// 该值的被使用列表
	std::list<Use> use_list_;
	// 该值的名称
	std::string name_;
};


// 对使用者的抽象
// 使用者可以使用某个值, 这些使用记录在被使用者的被使用列表中
// 使用者使用的这些值作为使用者的操作数
class User : public Value
{
public:
	User(const User& other) = delete;
	User(User&& other) = delete;
	User& operator=(const User& other) = delete;
	User& operator=(User&& other) = delete;

	User(Type* ty, const std::string& name = "");

	~User() override { remove_all_operands(); }

	// 所有操作数
	[[nodiscard]] const std::vector<Value*>& get_operands() const;

	// 操作数数量
	[[nodiscard]] int get_num_operand() const { return u2iNegThrow(operands_.size()); }

	/**
	 * 获取特定操作数
	 * @param i 操作数索引, 从 0 开始
	 */
	[[nodiscard]] Value* get_operand(int i) const;
	/**
	 * 设置操作数, 并自动维护使用列表
	 * @param i 索引
	 * @param v 操作数
	 */
	void set_operand(int i, Value* v);
	/**
	 * 添加操作数, 并自动维护使用列表
	 */
	void add_operand(Value* v);

	void remove_all_operands();
	void remove_operand(int i);

private:
	// operands of this value
	std::vector<Value*> operands_;
};

// 对使用的抽象
// 对于 op = func(a, b)
// a 被 op 在第一个操作数使用, 所以是 Use(op, 0)
// b 被 op 在第二个操作数使用, 所以是 Use(op, 1)
class Use
{
public:
	// 使用者
	User* val_;
	// 在使用者使用其的表达式中, 其是第几个操作数
	int arg_no_;

	/**
	 *
	 * @param val 使用者
	 * @param no 使用者在第几个操作数使用它
	 */
	Use(User* val, int no) : val_(val), arg_no_(no)
	{
	}

	/**
	 *
	 * @param other 是否具有相同的使用者以及相同的使用位置
	 */
	bool operator==(const Use& other) const
	{
		return val_ == other.val_ and arg_no_ == other.arg_no_;
	}
};
