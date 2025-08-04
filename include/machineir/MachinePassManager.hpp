#pragma once

#include <memory>
#include <vector>
#include "MachineModule.hpp"

class MachinePass
{
public:
	explicit MachinePass(MModule* m) : m_(m)
	{
	}

	virtual ~MachinePass() = default;
	virtual void run() = 0;
	MachinePass(const MachinePass&) = delete;
	MachinePass(MachinePass&&) = delete;
	MachinePass& operator=(const MachinePass&) = delete;
	MachinePass& operator=(MachinePass&&) = delete;

protected:
	MModule* m_;
};

class MachinePassManager
{
public:
	explicit MachinePassManager(MModule* m) : m_(m)
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
	std::vector<std::unique_ptr<MachinePass>> passes_;
	MModule* m_;
};
