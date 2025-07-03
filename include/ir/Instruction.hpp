#pragma once

#include <cstdint>
#include <Value.hpp>
#include <BasicBlock.hpp>

class GlobalVariable;
class FuncType;
class Module;
class BasicBlock;
class Function;

class Instruction : public User
{
public:
	Instruction(const Instruction&) = delete;
	Instruction(Instruction&&) = delete;
	Instruction& operator=(const Instruction&) = delete;
	Instruction& operator=(Instruction&&) = delete;

	enum OpID : uint8_t
	{
		// Terminator Instructions
		ret,
		br,
		// Standard binary operators
		add,
		sub,
		mul,
		sdiv,
		srem,
		// float binary operators
		fadd,
		fsub,
		fmul,
		fdiv,
		// Memory operators
		alloca_,
		load,
		store,
		// Int compare operators
		ge,
		gt,
		le,
		lt,
		eq,
		ne,
		// Float compare operators
		fge,
		fgt,
		fle,
		flt,
		feq,
		fne,
		// Other operators
		phi,
		call,
		getelementptr,
		zext, // zero extend
		fptosi,
		sitofp,
		// float binary operators Logical operators

		// mem 系列, 对内存使用 SIMD 快速填充
		// 内存复制
		memcpy_,
		// 内存填充
		memclear_,
		// 将数字指针类型转换为 char 指针类型
		nump2charp,
		// 转换数组全局变量的类型以适应它应该有的类型
		// llvm 的全局变量定义会导致其类型与预期不一致(仅仅是形状)
		global_fix
	};

	/**
   * 
   * @param ty 返回类型
   * @param id 函数指令类型
   * @param parent 如果 parent != nullptr, 自动插入 basicblock
   */
	Instruction(Type* ty, OpID id, BasicBlock* parent = nullptr);
	~Instruction() override = default;

	// 指令所属基本块
	BasicBlock* get_parent() { return parent_; }
	// 指令所属基本块
	[[nodiscard]] const BasicBlock* get_parent() const { return parent_; }
	// 设置指令所属基本块
	void set_parent(BasicBlock* parent) { this->parent_ = parent; }

	// 指令所属函数
	[[nodiscard]] Function* get_function() const;
	// 指令所属模块
	[[nodiscard]] Module* get_module() const;

	// 指令类型
	[[nodiscard]] OpID get_instr_type() const { return op_id_; }

	[[nodiscard]] std::string get_instr_op_name() const;

	[[nodiscard]] bool is_void() const;

	[[nodiscard]] bool is_phi() const { return op_id_ == phi; }
	[[nodiscard]] bool is_store() const { return op_id_ == store; }
	[[nodiscard]] bool is_alloca() const { return op_id_ == alloca_; }
	[[nodiscard]] bool is_ret() const { return op_id_ == ret; }
	[[nodiscard]] bool is_load() const { return op_id_ == load; }
	[[nodiscard]] bool is_br() const { return op_id_ == br; }

	[[nodiscard]] bool is_add() const { return op_id_ == add; }
	[[nodiscard]] bool is_sub() const { return op_id_ == sub; }
	[[nodiscard]] bool is_mul() const { return op_id_ == mul; }
	[[nodiscard]] bool is_div() const { return op_id_ == sdiv; }
	[[nodiscard]] bool is_rem() const { return op_id_ == srem; }

	[[nodiscard]] bool is_fadd() const { return op_id_ == fadd; }
	[[nodiscard]] bool is_fsub() const { return op_id_ == fsub; }
	[[nodiscard]] bool is_fmul() const { return op_id_ == fmul; }
	[[nodiscard]] bool is_fdiv() const { return op_id_ == fdiv; }
	[[nodiscard]] bool is_fp2si() const { return op_id_ == fptosi; }
	[[nodiscard]] bool is_si2fp() const { return op_id_ == sitofp; }

	[[nodiscard]] bool is_cmp() const { return ge <= op_id_ and op_id_ <= ne; }
	[[nodiscard]] bool is_fcmp() const { return fge <= op_id_ and op_id_ <= fne; }

	[[nodiscard]] bool is_call() const { return op_id_ == call; }
	[[nodiscard]] bool is_gep() const { return op_id_ == getelementptr; }
	[[nodiscard]] bool is_zext() const { return op_id_ == zext; }
	[[nodiscard]] bool is_memcpy() const { return op_id_ == memcpy_; }
	[[nodiscard]] bool is_memclear() const { return op_id_ == memclear_; }
	[[nodiscard]] bool is_nump2charp() const { return op_id_ == nump2charp; }

	[[nodiscard]] bool isBinary() const
	{
		return (is_add() || is_sub() || is_mul() || is_div() || is_fadd() ||
		        is_fsub() || is_fmul() || is_fdiv()) &&
		       (get_num_operand() == 2);
	}

	[[nodiscard]] bool isTerminator() const { return is_br() || is_ret(); }

private:
	OpID op_id_;
	BasicBlock* parent_;
};

template <typename Inst>
class BaseInst : public Instruction
{
protected:
	template <typename... Args>
	static Inst* create(Args&&... args)
	{
		return new Inst(std::forward<Args>(args)...);
	}

	template <typename... Args>
	BaseInst(Args&&... args) : Instruction(std::forward<Args>(args)...)
	{
	}
};

class IBinaryInst : public BaseInst<IBinaryInst>
{
	friend BaseInst<IBinaryInst>;

	IBinaryInst(OpID id, Value* v1, Value* v2, BasicBlock* bb);

public:
	static IBinaryInst* create_add(Value* v1, Value* v2, BasicBlock* bb);
	static IBinaryInst* create_sub(Value* v1, Value* v2, BasicBlock* bb);
	static IBinaryInst* create_mul(Value* v1, Value* v2, BasicBlock* bb);
	static IBinaryInst* create_sdiv(Value* v1, Value* v2, BasicBlock* bb);
	static IBinaryInst* create_srem(Value* v1, Value* v2, BasicBlock* bb);

	std::string print() override;
};

class FBinaryInst : public BaseInst<FBinaryInst>
{
	friend BaseInst<FBinaryInst>;

	FBinaryInst(OpID id, Value* v1, Value* v2, BasicBlock* bb);

public:
	static FBinaryInst* create_fadd(Value* v1, Value* v2, BasicBlock* bb);
	static FBinaryInst* create_fsub(Value* v1, Value* v2, BasicBlock* bb);
	static FBinaryInst* create_fmul(Value* v1, Value* v2, BasicBlock* bb);
	static FBinaryInst* create_fdiv(Value* v1, Value* v2, BasicBlock* bb);

	std::string print() override;
};

class ICmpInst : public BaseInst<ICmpInst>
{
	friend BaseInst<ICmpInst>;

	ICmpInst(OpID id, Value* lhs, Value* rhs, BasicBlock* bb);

public:
	static ICmpInst* create_ge(Value* v1, Value* v2, BasicBlock* bb);
	static ICmpInst* create_gt(Value* v1, Value* v2, BasicBlock* bb);
	static ICmpInst* create_le(Value* v1, Value* v2, BasicBlock* bb);
	static ICmpInst* create_lt(Value* v1, Value* v2, BasicBlock* bb);
	static ICmpInst* create_eq(Value* v1, Value* v2, BasicBlock* bb);
	static ICmpInst* create_ne(Value* v1, Value* v2, BasicBlock* bb);

	std::string print() override;
};

class FCmpInst : public BaseInst<FCmpInst>
{
	friend BaseInst<FCmpInst>;

	FCmpInst(OpID id, Value* lhs, Value* rhs, BasicBlock* bb);

public:
	static FCmpInst* create_fge(Value* v1, Value* v2, BasicBlock* bb);
	static FCmpInst* create_fgt(Value* v1, Value* v2, BasicBlock* bb);
	static FCmpInst* create_fle(Value* v1, Value* v2, BasicBlock* bb);
	static FCmpInst* create_flt(Value* v1, Value* v2, BasicBlock* bb);
	static FCmpInst* create_feq(Value* v1, Value* v2, BasicBlock* bb);
	static FCmpInst* create_fne(Value* v1, Value* v2, BasicBlock* bb);

	std::string print() override;
};

class CallInst : public BaseInst<CallInst>
{
	friend BaseInst<CallInst>;

protected:
	CallInst(Function* func, const std::vector<Value*>& args, BasicBlock* bb);

public:
	static CallInst* create_call(Function* func, const std::vector<Value*>& args,
	                             BasicBlock* bb);
	[[nodiscard]] FuncType* get_function_type() const;

	std::string print() override;
};

class BranchInst : public BaseInst<BranchInst>
{
	friend BaseInst<BranchInst>;

	BranchInst(Value* cond, BasicBlock* if_true, BasicBlock* if_false,
	           BasicBlock* bb);
	~BranchInst() override;

public:
	BranchInst(const BranchInst&) = delete;
	BranchInst(BranchInst&&) = delete;
	BranchInst& operator=(const BranchInst&) = delete;
	BranchInst& operator=(BranchInst&&) = delete;
	static BranchInst* create_cond_br(Value* cond, BasicBlock* if_true,
	                                  BasicBlock* if_false, BasicBlock* bb);
	static BranchInst* create_br(BasicBlock* if_true, BasicBlock* bb);

	[[nodiscard]] bool is_cond_br() const { return get_num_operand() == 3; }

	std::string print() override;
};

class ReturnInst : public BaseInst<ReturnInst>
{
	friend BaseInst<ReturnInst>;

	ReturnInst(Value* val, BasicBlock* bb);

public:
	static ReturnInst* create_ret(Value* val, BasicBlock* bb);
	static ReturnInst* create_void_ret(BasicBlock* bb);
	[[nodiscard]] bool is_void_ret() const;

	std::string print() override;
};

class GetElementPtrInst : public BaseInst<GetElementPtrInst>
{
	friend BaseInst<GetElementPtrInst>;

	GetElementPtrInst(Value* ptr, const std::vector<Value*>& idxs, BasicBlock* bb);

public:
	static Type* get_element_type(const Value* ptr, const std::vector<Value*>& idxs);
	static GetElementPtrInst* create_gep(Value* ptr, const std::vector<Value*>& idxs,
	                                     BasicBlock* bb);
	[[nodiscard]] Type* get_element_type() const;

	std::string print() override;
};

class StoreInst : public BaseInst<StoreInst>
{
	friend BaseInst<StoreInst>;

	StoreInst(Value* val, Value* ptr, BasicBlock* bb);

public:
	static StoreInst* create_store(Value* val, Value* ptr, BasicBlock* bb);

	[[nodiscard]] Value* get_rval() const { return this->get_operand(0); }
	[[nodiscard]] Value* get_lval() const { return this->get_operand(1); }

	std::string print() override;
};

// 将内存从 from 复制到 to, length_ 为 16 的倍数(至少目前是)
// 因而可以使用 SIMD 寄存器进行复制
class MemCpyInst : public BaseInst<MemCpyInst>
{
	friend BaseInst<MemCpyInst>;

	MemCpyInst(Value* from, Value* to, int size, BasicBlock* bb);

	int length_;

public:
	static MemCpyInst* create_memcpy(Value* from, Value* to, int size, BasicBlock* bb);

	[[nodiscard]] Value* get_from() const { return this->get_operand(0); }
	[[nodiscard]] Value* get_to() const { return this->get_operand(1); }
	[[nodiscard]] int get_copy_bytes() const { return length_; }

	std::string print() override;
};

// 将内存从 from 复制到 to, length_ 为 16 的倍数(至少目前是)
// 因而可以使用 SIMD 寄存器进行复制
class MemClearInst : public BaseInst<MemClearInst>
{
	friend BaseInst<MemClearInst>;

	MemClearInst(Value* target, int size, BasicBlock* bb);

	int length_;

public:
	static MemClearInst* create_memclear(Value* target, int size, BasicBlock* bb);

	[[nodiscard]] Value* get_target() const { return this->get_operand(0); }
	[[nodiscard]] int get_clear_bytes() const { return length_; }

	std::string print() override;
};

// 将数字指针类型转换为 char 指针类型
class Nump2CharpInst : public BaseInst<Nump2CharpInst>
{
	friend BaseInst<Nump2CharpInst>;

	Nump2CharpInst(Value* val, BasicBlock* bb);

public:
	static Nump2CharpInst* create_nump2charp(Value* val, BasicBlock* bb);

	[[nodiscard]] Value* get_val() const { return this->get_operand(0); }

	std::string print() override;
};


// 转换数组全局变量的类型以适应它应该有的类型
// llvm 的全局变量定义会导致其类型与预期不一致(仅仅是形状)
class GlobalFixInst : public BaseInst<GlobalFixInst>
{
	friend BaseInst<GlobalFixInst>;

	GlobalFixInst(GlobalVariable* val, BasicBlock* bb);

public:
	static GlobalFixInst* create_global_fix(GlobalVariable* val, BasicBlock* bb);

	[[nodiscard]] GlobalVariable* get_var() const;

	std::string print() override;
};


class LoadInst : public BaseInst<LoadInst>
{
	friend BaseInst<LoadInst>;

	LoadInst(Value* ptr, BasicBlock* bb);

public:
	static LoadInst* create_load(Value* ptr, BasicBlock* bb);

	[[nodiscard]] Value* get_lval() const { return this->get_operand(0); }
	[[nodiscard]] Type* get_load_type() const { return get_type(); }

	std::string print() override;
};

class AllocaInst : public BaseInst<AllocaInst>
{
	friend BaseInst<AllocaInst>;

	AllocaInst(Type* ty, BasicBlock* bb);

public:
	static AllocaInst* create_alloca(Type* ty, BasicBlock* bb);

	[[nodiscard]] Type* get_alloca_type() const;

	std::string print() override;
};

class ZextInst : public BaseInst<ZextInst>
{
	friend BaseInst<ZextInst>;

	ZextInst(Value* val, Type* ty, BasicBlock* bb);

public:
	static ZextInst* create_zext_to_i32(Value* val, BasicBlock* bb);

	[[nodiscard]] Type* get_dest_type() const { return get_type(); }

	std::string print() override;
};

class FpToSiInst : public BaseInst<FpToSiInst>
{
	friend BaseInst<FpToSiInst>;

	FpToSiInst(Value* val, Type* ty, BasicBlock* bb);

public:
	static FpToSiInst* create_fptosi(Value* val, Type* ty, BasicBlock* bb);
	static FpToSiInst* create_fptosi_to_i32(Value* val, BasicBlock* bb);

	[[nodiscard]] Type* get_dest_type() const { return get_type(); }

	std::string print() override;
};

class SiToFpInst : public BaseInst<SiToFpInst>
{
	friend BaseInst<SiToFpInst>;

	SiToFpInst(Value* val, Type* ty, BasicBlock* bb);

public:
	static SiToFpInst* create_sitofp(Value* val, BasicBlock* bb);

	[[nodiscard]] Type* get_dest_type() const { return get_type(); }

	std::string print() override;
};

class PhiInst : public BaseInst<PhiInst>
{
	friend BaseInst<PhiInst>;

	PhiInst(Type* ty, const std::vector<Value*>& vals,
	        const std::vector<BasicBlock*>& val_bbs, BasicBlock* bb);

public:
	static PhiInst* create_phi(Type* ty, BasicBlock* bb,
	                           const std::vector<Value*>& vals = {},
	                           const std::vector<BasicBlock*>& val_bbs = {});

	void add_phi_pair_operand(Value* val, Value* pre_bb)
	{
		this->add_operand(val);
		this->add_operand(pre_bb);
	}

    void remove_phi_operand(Value *pre_bb) {
        for (unsigned i = 0; i < this->get_num_operand(); i += 2) {
            if (this->get_operand(i + 1) == pre_bb) {
                this->remove_operand(i);
                this->remove_operand(i);
                return;
            }
        }
    }

    std::vector<std::pair<Value *, BasicBlock *>> get_phi_pairs() {
        std::vector<std::pair<Value *, BasicBlock *>> res;
        for (size_t i = 0; i < get_num_operand(); i += 2) {
            res.push_back({this->get_operand(i), dynamic_cast<BasicBlock*>(this->get_operand(i + 1))});
        }
        return res;
    }

	std::string print() override;
};
