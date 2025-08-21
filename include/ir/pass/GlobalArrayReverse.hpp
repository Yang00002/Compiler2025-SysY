#pragma once
#include <set>
#include <unordered_set>

#include "PassManager.hpp"

class Loop;
class GetElementPtrInst;
class LoopDetection;
class FuncInfo;

class GlobalArrayReverse final : public Pass
{
	std::unordered_set<Value*> needReverseSrc_;
	std::unordered_set<GetElementPtrInst*> importance_;
	std::unordered_set<Loop*> noValue_;
	FuncInfo* info_;
	int varDimSize_ = 0;
public:
	GlobalArrayReverse(PassManager* manager, Module* m)
		: Pass(manager, m), info_(nullptr)
	{
	}

	// 函数是否是某变量专用函数
	static bool isOnlyFunc(const Function* f, int argNo);
	bool legalGlobalVar(const Value* val, bool inCall);
	bool noValueForReverse(BasicBlock* bb);
	void run() override;
};
