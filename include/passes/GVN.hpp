#pragma once

#include "Dominators.hpp"
#include "PassManager.hpp"
#include <memory>
#include <map>

#include "Instruction.hpp"

// 消除公共表达式
// 只要 phi 不被消除, 则只要支配树逆后序遍历一次即可消除所有公共表达式
class GVN final : public Pass
{
	struct ValueHash
	{
		ValueHash(Value** vals, int vc, Instruction::OpID ty);
		ValueHash();
		ValueHash(const Instruction* inst);

		ValueHash(const ValueHash& other);

		ValueHash(ValueHash&& other) noexcept;

		ValueHash& operator=(const ValueHash& other);

		ValueHash& operator=(ValueHash&& other) noexcept;

		Value** vals_;
		int vc_;
		Instruction::OpID type_;
		~ValueHash();

		bool operator<(const ValueHash& r)const;
	};

	enum gvn_state_t : uint8_t { firstdef, redundant, invalid };

	std::map<ValueHash, Value*> expr_val_map_;
	std::unique_ptr<Dominators> dom_;

	gvn_state_t visit_inst(Instruction* i);

public:
	GVN(const GVN&) = delete;
	GVN(GVN&&) = delete;
	GVN& operator=(const GVN&) = delete;
	GVN& operator=(GVN&&) = delete;

	explicit GVN(Module* m) : Pass(m)
	{
		dom_ = std::make_unique<Dominators>(m);
	}

	~GVN() override = default;

	void run() override;
	void run(Function* f);
};
