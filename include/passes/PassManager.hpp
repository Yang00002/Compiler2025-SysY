#pragma once

#include <memory>
#include <vector>
#include "Module.hpp"

class PassManager;

class Pass
{
public:
	explicit Pass(PassManager* manager, Module* m) : m_(m), manager_(manager)
	{
	}

	virtual ~Pass() = default;
	virtual void run() = 0;
	Pass(const Pass&) = delete;
	Pass(Pass&&) = delete;
	Pass& operator=(const Pass&) = delete;
	Pass& operator=(Pass&&) = delete;

protected:
	Module* m_;
	PassManager* manager_;
};

class GlobalInfoPass
{
public:
	explicit GlobalInfoPass(PassManager* mng, Module* m) : m_(m), manager_(mng)
	{
	}

	virtual ~GlobalInfoPass() = default;
	virtual void run() = 0;
	GlobalInfoPass(const GlobalInfoPass&) = delete;
	GlobalInfoPass(GlobalInfoPass&&) = delete;
	GlobalInfoPass& operator=(const GlobalInfoPass&) = delete;
	GlobalInfoPass& operator=(GlobalInfoPass&&) = delete;

protected:
	Module* m_;
	PassManager* manager_;
};

class FuncInfoPass
{
public:
	explicit FuncInfoPass(PassManager* mng, Function* f) : f_(f), manager_(mng)
	{
	}

	virtual ~FuncInfoPass() = default;
	virtual void run() = 0;
	FuncInfoPass(const FuncInfoPass&) = delete;
	FuncInfoPass(FuncInfoPass&&) = delete;
	FuncInfoPass& operator=(const FuncInfoPass&) = delete;
	FuncInfoPass& operator=(FuncInfoPass&&) = delete;

protected:
	Function* f_;
	PassManager* manager_;
};

class PassManager
{
public:
	PassManager(const PassManager& other) = delete;
	PassManager(PassManager&& other) noexcept = delete;
	PassManager& operator=(const PassManager& other) = delete;
	PassManager& operator=(PassManager&& other) noexcept = delete;

	explicit PassManager(Module* m) : m_(m)
	{
	}

	template <typename PassType, typename... Args>
	void add_pass(Args... args)
	{
		passes_.emplace_back(new PassType(this, m_, std::forward<Args>(args)...));
	}

	void run() const
	{
		for (auto& pass : passes_)
		{
			pass->run();
			delete pass;
		}
	}

	template <typename PassType>
	void flushGlobalInfo()
	{
		auto ty = &typeid(PassType);
		delete globalMsg_[ty];
		globalMsg_[ty] = nullptr;
	}

	template <typename PassType>
	void flushFuncInfo(Function* f)
	{
		auto ty = &typeid(PassType);
		auto& infos = funcMsg_[ty];
		auto fd = infos.find(f);
		if (fd == infos.end()) return;
		delete fd->second;
		infos.erase(fd);
	}

	void flushFuncInfo(Function* f)
	{
		for (auto& [i, infos] : funcMsg_)
		{
			auto fd = infos.find(f);
			if (fd == infos.end()) continue;
			delete fd->second;
			infos.erase(fd);
		}
	}

	template <typename PassType>
	void flushFuncInfo()
	{
		auto ty = &typeid(PassType);
		for (auto [i, j] : funcMsg_[ty]) delete j;
		funcMsg_[ty].clear();
	}

	template <typename PassType>
	PassType* flushAndGetGlobalInfo()
	{
		auto ty = &typeid(PassType);
		delete globalMsg_[ty];
		auto get = new PassType(this, m_);
		globalMsg_[ty] = get;
		get->run();
		return get;
	}

	template <typename PassType>
	PassType* flushAndGetFuncInfo(Function* f)
	{
		auto ty = &typeid(PassType);
		auto& infos = funcMsg_[ty];
		delete infos[f];
		auto get = new PassType(this, f);
		infos[f] = get;
		get->run();
		return get;
	}

	template <typename PassType>
	PassType* getGlobalInfo()
	{
		auto ty = &typeid(PassType);
		auto get = globalMsg_[ty];
		if (get == nullptr)
		{
			get = new PassType(this, m_);
			get->run();
			globalMsg_[ty] = get;
		}
		return dynamic_cast<PassType*>(get);
	}

	template <typename PassType>
	PassType* getGlobalInfoIfPresent()
	{
		auto ty = &typeid(PassType);
		return dynamic_cast<PassType*>(globalMsg_[ty]);
	}

	template <typename PassType>
	PassType* getFuncInfo(Function* f)
	{
		auto ty = &typeid(PassType);
		auto& infos = funcMsg_[ty];
		auto get = infos[f];
		if (get == nullptr)
		{
			get = new PassType(this, f);
			get->run();
			infos[f] = get;
		}
		return dynamic_cast<PassType*>(get);
	}

	void flushAllInfo()
	{

		for (auto& [i, j] : funcMsg_)
		{
			for (auto& [p, q] : j) delete q;
		}
		for (auto& [i, j] : globalMsg_) delete j;
		funcMsg_.clear();
		globalMsg_.clear();
	}

	~PassManager()
	{
		for (auto& [i,j] : funcMsg_)
		{
			for (auto& [p, q] : j) delete q;
		}
		for (auto& [i, j] : globalMsg_) delete j;
	}
private:
	std::vector<Pass*> passes_;
	std::unordered_map<const std::type_info*, std::unordered_map<Function*, FuncInfoPass*>> funcMsg_;
	std::unordered_map<const std::type_info*, GlobalInfoPass*> globalMsg_;
	Module* m_;
};
