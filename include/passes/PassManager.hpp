#pragma once

#include <memory>
#include <vector>
#include "Module.hpp"

class Pass
{
public:
	explicit Pass(Module* m) : m_(m)
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
};

class PassManager
{
public:
	explicit PassManager(Module* m) : m_(m)
	{
	}

	template <typename PassType, typename... Args>
	void add_pass(Args&&... args)
	{
		passes_.emplace_back(new PassType(m_, std::forward<Args>(args)...));
	}

	void run() const
	{
		for (auto& pass : passes_)
		{
			pass->run();
		}
	}

private:
	std::vector<std::unique_ptr<Pass>> passes_;
	Module* m_;
};
