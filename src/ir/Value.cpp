#include <Value.hpp>

#include <cassert>

Value::Value(Type* ty, std::string name)
	: type_(ty), name_(std::move(name))
{
}

Type* Value::get_type() const { return type_; }

const std::list<Use>& Value::get_use_list() const { return use_list_; }

bool Value::set_name(const std::string& name)
{
	if (name_.empty())
	{
		name_ = name;
		return true;
	}
	return false;
}

void Value::add_use(User* user, int arg_no)
{
	use_list_.emplace_back(user, arg_no);
};

void Value::remove_use(User* user, int arg_no)
{
	auto target_use = Use(user, arg_no);
	use_list_.remove_if([&](const Use& use) { return use == target_use; });
}

void Value::replace_all_use_with(Value* new_val) const
{
	if (this == new_val)
		return;
	while (!use_list_.empty())
	{
		auto use = use_list_.begin();
		use->val_->set_operand(use->arg_no_, new_val);
	}
}

void Value::replace_use_with_if(Value* new_val,
                                const std::function<bool(Use)>& pred)
{
	if (this == new_val)
		return;
	for (auto iter = use_list_.begin(); iter != use_list_.end();)
	{
		auto use = *iter++;
		if (!pred(use))
			continue;
		use.val_->set_operand(use.arg_no_, new_val);
	}
}


User::User(Type* ty, const std::string& name) : Value(ty, name)
{
}

const std::vector<Value*>& User::get_operands() const { return operands_; }

Value* User::get_operand(int i) const { return operands_.at(i); }

void User::set_operand(int i, Value* v)
{
	assert(i <u2iNegThrow(operands_.size()) && "set_operand out of index");
	if (operands_[i])
	{
		// old operand
		operands_[i]->remove_use(this, i);
	}
	if (v)
	{
		// new operand
		v->add_use(this, i);
	}
	operands_[i] = v;
}

void User::add_operand(Value* v)
{
	assert(v != nullptr && "bad use: add_operand(nullptr)");
	v->add_use(this, u2iNegThrow(operands_.size()));
	operands_.push_back(v);
}

void User::remove_all_operands()
{
	int size = u2iNegThrow(operands_.size());
	for (int i = 0; i != size; ++i)
	{
		if (operands_[i])
		{
			operands_[i]->remove_use(this, i);
		}
	}
	operands_.clear();
}

void User::remove_operand(int idx)
{
	int size = u2iNegThrow(operands_.size());
	assert(idx < size && "remove_operand out of index");
	// influence on other operands
	for (int i = idx + 1; i < size; ++i)
	{
		operands_[i]->remove_use(this, i);
		operands_[i]->add_use(this, i - 1);
	}
	// remove the designated operand
	operands_[idx]->remove_use(this, idx);
	operands_.erase(operands_.begin() + idx);
}
