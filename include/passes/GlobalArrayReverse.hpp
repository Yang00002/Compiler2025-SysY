#pragma once
#include <unordered_set>

#include "PassManager.hpp"

class GetElementPtrInst;
class LoopDetection;
class FuncInfo;

class GlobalArrayReverse final : public Pass
{
	std::unordered_set<Value*> needReverseSrc_;
	std::vector<GetElementPtrInst*> importance_;
	FuncInfo* info_;
	int varDimSize_ = 0;
public:
	GlobalArrayReverse(PassManager* manager, Module* m)
		: Pass(manager, m), info_(nullptr)
	{
	}

	bool legalGlobalVar(const Value* val, bool inCall);
	void run() override;
};
