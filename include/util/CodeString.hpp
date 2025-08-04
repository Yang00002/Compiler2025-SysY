#pragma once
#include <string>

class CodeString
{
public:
	CodeString(const CodeString& other) = delete;
	CodeString(CodeString&& other) noexcept = delete;
	CodeString& operator=(const CodeString& other) = delete;
	CodeString& operator=(CodeString&& other) noexcept = delete;

private:
	struct CodeStringNode
	{
		std::string text_;
		CodeStringNode* next_;
	};

	CodeStringNode* begin_;
	CodeStringNode* end_;
	int codeLines_;

public:
	void addSection(const char* secName);
	void addAlign(int dataSize, bool glob);
	void addAlign(long long dataSize, bool glob);
	void addCommonStr(std::string str);
	void addInstruction(const std::string& str);
	void addInstruction(const std::string& name, const std::string& op1);
	void addInstruction(const std::string& name, const std::string& op1, const std::string& op2);
	void addInstruction(const std::string& name, const std::string& op1, const std::string& op2,
	                    const std::string& op3);
	void addInstruction(const std::string& name, const std::string& op1, const std::string& op2, const std::string& op3,
	                    const std::string& op4);
	void addCommonStr(const char* str);
	void objectTypeDeclare(const std::string& name);
	void addLabel(const std::string& name);
	void functionTypeDeclare(const std::string& name);
	void addGlobal(const std::string& globName);
	void replaceBack(const std::string& from, const std::string& to) const;
	void copyBefore(const CodeString* str);
	friend std::ostream& operator<<(std::ostream& os, const CodeString* str);
	[[nodiscard]] int lines() const;
	CodeString();
	~CodeString();
};
