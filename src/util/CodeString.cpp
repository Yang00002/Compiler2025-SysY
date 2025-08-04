#include "CodeString.hpp"

#include "ostream"
#include "Config.hpp"

using namespace std;

namespace
{
	const char* align(int dataSize, bool glob)
	{
		if ((dataSize > alignTo16NeedBytes) || (glob && dataSize > 8)) return "\t.align 4";
		if (dataSize > 4) return "\t.align 3";
		if (dataSize > 2) return "\t.align 2";
		if (dataSize > 1) return "\t.align 1";
		return "\t.align 0";
	}

	const char* align(long long dataSize, bool glob)
	{
		if ((dataSize > alignTo16NeedBytes) || (glob && dataSize > 8)) return "\t.align 4";
		if (dataSize > 4) return "\t.align 3";
		if (dataSize > 2) return "\t.align 2";
		if (dataSize > 1) return "\t.align 1";
		return "\t.align 0";
	}
}

void CodeString::addSection(const char* secName)
{
	end_->next_ = new CodeStringNode{std::string("\t.") + secName, nullptr};
	end_ = end_->next_;
}

void CodeString::addAlign(int dataSize, bool glob)
{
	end_->next_ = new CodeStringNode{align(dataSize, glob), nullptr};
	end_ = end_->next_;
}

void CodeString::addAlign(long long dataSize, bool glob)
{
	end_->next_ = new CodeStringNode{align(dataSize, glob), nullptr};
	end_ = end_->next_;
}

void CodeString::addCommonStr(std::string str)
{
	end_->next_ = new CodeStringNode{std::move(str), nullptr};
	end_ = end_->next_;
}

void CodeString::addInstruction(const std::string& str)
{
	end_->next_ = new CodeStringNode{"\t" + str, nullptr};
	end_ = end_->next_;
	codeLines_++;
}

void CodeString::addInstruction(const std::string& name, const std::string& op1)
{
	end_->next_ = new CodeStringNode{"\t" + name + " " + op1, nullptr};
	end_ = end_->next_;
	codeLines_++;
}

void CodeString::addInstruction(const std::string& name, const std::string& op1, const std::string& op2)
{
	end_->next_ = new CodeStringNode{"\t" + name + " " + op1 + ", " + op2, nullptr};
	end_ = end_->next_;
	codeLines_++;
}

void CodeString::addInstruction(const std::string& name, const std::string& op1, const std::string& op2,
                                const std::string& op3)
{
	end_->next_ = new CodeStringNode{"\t" + name + " " + op1 + ", " + op2 + ", " + op3, nullptr};
	end_ = end_->next_;
	codeLines_++;
}

void CodeString::addInstruction(const std::string& name, const std::string& op1, const std::string& op2,
                                const std::string& op3, const std::string& op4)
{
	end_->next_ = new CodeStringNode{"\t" + name + " " + op1 + ", " + op2 + ", " + op3 + ", " + op4, nullptr};
	end_ = end_->next_;
	codeLines_++;
}

void CodeString::addCommonStr(const char* str)
{
	end_->next_ = new CodeStringNode{str, nullptr};
	end_ = end_->next_;
}

void CodeString::objectTypeDeclare(const std::string& name)
{
	end_->next_ = new CodeStringNode{"\t.type " + name + ", @object", nullptr};
	end_ = end_->next_;
}

void CodeString::addLabel(const std::string& name)
{
	end_->next_ = new CodeStringNode{name + ":", nullptr};
	end_ = end_->next_;
}

void CodeString::functionTypeDeclare(const std::string& name)
{
	end_->next_ = new CodeStringNode{"\t.type " + name + ", @function", nullptr};
	end_ = end_->next_;
}

void CodeString::addGlobal(const std::string& globName)
{
	end_->next_ = new CodeStringNode{"\t.globl " + globName, nullptr};
	end_ = end_->next_;
}

void CodeString::replaceBack(const std::string& from, const std::string& to) const
{
	if (end_ == begin_) return;
	auto fd = end_->text_.find(from);
	end_->text_.replace(fd, from.size(), to);
}

void CodeString::copyBefore(const CodeString* str)
{
	auto it = str->begin_->next_;
	auto p = begin_;
	while (it != nullptr)
	{
		p->next_ = new CodeStringNode{it->text_, p->next_};
		p = p->next_;
		it = it->next_;
	}
	if (end_ == begin_) end_ = p;
	codeLines_ += str->codeLines_;
}

std::ostream& operator<<(std::ostream& os, const CodeString* str)
{
	auto it = str->begin_->next_;
	while (it != nullptr)
	{
		os << it->text_ << '\n';
		it = it->next_;
	}
	return os;
}

int CodeString::lines() const
{
	return codeLines_;
}

CodeString::CodeString()
{
	begin_ = new CodeStringNode{"", nullptr};
	end_ = begin_;
	codeLines_ = 0;
}

CodeString::~CodeString()
{
	while (begin_ != end_)
	{
		auto b = begin_;
		begin_ = begin_->next_;
		delete b;
	}
	delete begin_;
}
