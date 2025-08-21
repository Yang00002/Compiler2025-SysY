#pragma once

#include "PassManager.hpp"

#include <unordered_map>
#include <unordered_set>


class StoreInst;
class LoadInst;
class Instruction;
class Value;
class Function;
class Module;
/**
 * 计算哪些函数是纯函数
 *
 * 为非纯函数收集三种信息:
 * 1. 直接或通过 call 写入的全局变量 / 函数参数
 * 2. 直接或通过 call 加载的全局变量 / 函数参数
 * 3. 是否直接或通过 call 调用过非纯库函数(或它自己就是非纯库函数)
 *
 * 三种情况都不存在的函数是纯函数
 */
class FuncInfo : public GlobalInfoPass
{
public:
	// 非纯函数对值的影响信息
	struct UseMessage
	{
		// 影响的全局变量(注意此处不包含常量)
		std::unordered_set<GlobalVariable*> globals_;
		// 影响的参数(第一个参数序号为 0)
		std::unordered_set<Argument*> arguments_;

		void add(Value* val);
		bool have(Value* val) const;
		[[nodiscard]] bool empty() const;
	};

	FuncInfo(PassManager* mng, Module* m);

	void run() override;
	void flushAbout(std::unordered_set<Function*>& f);
	void removeFunc(Function* f);
	void flushLoadsAbout(std::unordered_set<Function*>& f);
	UseMessage& loadDetail(Function* function);
	UseMessage& storeDetail(Function* function);


	bool is_pure_function(Function* func);

	bool useOrIsImpureLib(Function* function);

private:
	// 函数存储的值
	std::unordered_map<Function*, UseMessage> stores;
	// 函数加载的值
	std::unordered_map<Function*, UseMessage> loads;
	// 函数是否因为调用库函数而变得非纯函数
	std::unordered_map<Function*, bool> useImpureLibs;

	void spread(Value* val, std::unordered_map<Value*, Value*>& spMap);
	void spreadLoadsIn(Value* val, std::unordered_map<Value*, Value*>& spMap, std::unordered_set<Function*>& fs);
	void spreadGlobalIn(Value* val, std::unordered_map<Value*, Value*>& spMap, std::unordered_set<Function*>& fs);
};
