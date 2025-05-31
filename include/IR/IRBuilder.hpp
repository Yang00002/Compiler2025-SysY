#pragma once

#include "Constant.hpp"
#include "Function.hpp"
#include "Instruction.hpp"
#include "Value.hpp"

class IRBuilder
{
	BasicBlock* BB_;
	Module* m_;

public:
	IRBuilder(const IRBuilder&) = delete;
	IRBuilder(IRBuilder&&) = delete;
	IRBuilder& operator=(const IRBuilder&) = delete;
	IRBuilder& operator=(IRBuilder&&) = delete;

	IRBuilder(BasicBlock* bb, Module* m) : BB_(bb), m_(m)
	{
	}

	~IRBuilder() = default;
	[[nodiscard]] Module* get_module() const { return m_; }
	[[nodiscard]] BasicBlock* get_insert_block() const { return this->BB_; }

	void set_insert_point(BasicBlock* bb)
	{
		this->BB_ = bb;
	} // 在某个基本块中插入指令
	IBinaryInst* create_iadd(Value* lhs, Value* rhs) const
	{
		return IBinaryInst::create_add(lhs, rhs, this->BB_);
	} // 创建加法指令（以及其他算术指令）
	IBinaryInst* create_isub(Value* lhs, Value* rhs) const
	{
		return IBinaryInst::create_sub(lhs, rhs, this->BB_);
	}

	IBinaryInst* create_imul(Value* lhs, Value* rhs) const
	{
		return IBinaryInst::create_mul(lhs, rhs, this->BB_);
	}

	IBinaryInst* create_isdiv(Value* lhs, Value* rhs) const
	{
		return IBinaryInst::create_sdiv(lhs, rhs, this->BB_);
	}

	IBinaryInst* create_isrem(Value* lhs, Value* rhs) const
	{
		return IBinaryInst::create_srem(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_eq(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_eq(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_ne(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_ne(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_gt(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_gt(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_ge(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_ge(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_lt(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_lt(lhs, rhs, this->BB_);
	}

	ICmpInst* create_icmp_le(Value* lhs, Value* rhs) const
	{
		return ICmpInst::create_le(lhs, rhs, this->BB_);
	}

	CallInst* create_call(Value* func, const std::vector<Value*>& args) const
	{
		return CallInst::create_call(dynamic_cast<Function*>(func), args,
		                             this->BB_);
	}

	PhiInst* create_phi(Type* ty,
	                    const std::vector<Value*>& vals = {},
	                    const std::vector<BasicBlock*>& val_bbs = {})
	{
		return PhiInst::create_phi(ty, BB_, vals, val_bbs);
	}

	BranchInst* create_br(BasicBlock* if_true) const
	{
		return BranchInst::create_br(if_true, this->BB_);
	}

	BranchInst* create_cond_br(Value* cond, BasicBlock* if_true,
	                           BasicBlock* if_false) const
	{
		return BranchInst::create_cond_br(cond, if_true, if_false, this->BB_);
	}

	ReturnInst* create_ret(Value* val) const
	{
		return ReturnInst::create_ret(val, this->BB_);
	}

	ReturnInst* create_void_ret() const
	{
		return ReturnInst::create_void_ret(this->BB_);
	}

	GetElementPtrInst* create_gep(Value* ptr, const std::vector<Value*>& idxs) const
	{
		return GetElementPtrInst::create_gep(ptr, idxs, this->BB_);
	}

	MemCpyInst* create_memcpy(Value* from, Value* to, int bytes) const
	{
		return MemCpyInst::create_memcpy(from, to, bytes, this->BB_);
	}

	MemClearInst* create_memclear(Value* target, int bytes) const
	{
		return MemClearInst::create_memclear(target, bytes, this->BB_);
	}

	Nump2CharpInst* create_nump2charp(Value* val) const
	{
		return Nump2CharpInst::create_nump2charp(val, this->BB_);
	}

	GlobalFixInst* create_global_fix(GlobalVariable* var) const
	{
		return GlobalFixInst::create_global_fix(var, this->BB_);
	}

	StoreInst* create_store(Value* val, Value* ptr) const
	{
		return StoreInst::create_store(val, ptr, this->BB_);
	}

	LoadInst* create_load(Value* ptr) const
	{
		assert(ptr->get_type()->isPointerType() &&
			"ptr must be pointer type");
		return LoadInst::create_load(ptr, this->BB_);
	}

	AllocaInst* create_alloca(Type* ty) const
	{
		return AllocaInst::create_alloca(ty, this->BB_);
	}

	ZextInst* create_zext_to_i32(Value* val) const
	{
		return ZextInst::create_zext_to_i32(val, this->BB_);
	}

	SiToFpInst* create_sitofp(Value* val) const
	{
		return SiToFpInst::create_sitofp(val, this->BB_);
	}

	FpToSiInst* create_fptosi_to_i32(Value* val) const
	{
		return FpToSiInst::create_fptosi_to_i32(val, this->BB_);
	}

	FCmpInst* create_fcmp_ne(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_fne(lhs, rhs, this->BB_);
	}

	FCmpInst* create_fcmp_lt(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_flt(lhs, rhs, this->BB_);
	}

	FCmpInst* create_fcmp_le(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_fle(lhs, rhs, this->BB_);
	}

	FCmpInst* create_fcmp_ge(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_fge(lhs, rhs, this->BB_);
	}

	FCmpInst* create_fcmp_gt(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_fgt(lhs, rhs, this->BB_);
	}

	FCmpInst* create_fcmp_eq(Value* lhs, Value* rhs) const
	{
		return FCmpInst::create_feq(lhs, rhs, this->BB_);
	}

	FBinaryInst* create_fadd(Value* lhs, Value* rhs) const
	{
		return FBinaryInst::create_fadd(lhs, rhs, this->BB_);
	}

	FBinaryInst* create_fsub(Value* lhs, Value* rhs) const
	{
		return FBinaryInst::create_fsub(lhs, rhs, this->BB_);
	}

	FBinaryInst* create_fmul(Value* lhs, Value* rhs) const
	{
		return FBinaryInst::create_fmul(lhs, rhs, this->BB_);
	}

	FBinaryInst* create_fdiv(Value* lhs, Value* rhs) const
	{
		return FBinaryInst::create_fdiv(lhs, rhs, this->BB_);
	}

	[[nodiscard]] Constant* create_constant(const int i) const
	{
		return Constant::create(m_, i);
	}

	[[nodiscard]] Constant* create_constant(const float i) const
	{
		return Constant::create(m_, i);
	}

	[[nodiscard]] Constant* create_constant(const bool i) const
	{
		return Constant::create(m_, i);
	}

	[[nodiscard]] Constant* create_constant(const ConstantValue i) const
	{
		if (i.isIntConstant()) return Constant::create(m_, i.getIntConstant());
		if (i.isFloatConstant()) return Constant::create(m_, i.getFloatConstant());
		return Constant::create(m_, i.getBoolConstant());
	}
};
