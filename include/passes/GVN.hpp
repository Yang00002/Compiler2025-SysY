#pragma once

#include "Dominators.hpp"
#include "PassManager.hpp"
#include <memory>
#include <map>

class GVN final : public Pass
{
    enum gvn_state_t : uint8_t { firstdef, redundant, invalid };

    std::map<std::string, Value*> expr_val_map_;
    std::unique_ptr<Dominators> dom_;

    static bool to_be_visited(const Instruction* i);
    gvn_state_t visit_inst(Instruction* i);
    void erase_inst(Instruction* i);

    static bool is_commutative(const Instruction* i);
    static std::string get_expr(const Instruction* i);
    static std::string get_equiv_expr(const Instruction* i);

public:
	GVN(const GVN&) = delete;
	GVN(GVN&&) = delete;
	GVN& operator=(const GVN&) = delete;
	GVN& operator=(GVN&&) = delete;

	explicit GVN(Module* m) : Pass(m)
	{ dom_ = std::make_unique<Dominators>(m); }

	~GVN() override = default;

	void run() override;    
    void run(Function* f);
};
