#include <Instruction.hpp>
#include <BasicBlock.hpp>
#include <Function.hpp>
#include <IRPrinter.hpp>
#include <GlobalVariable.hpp>
#include <Type.hpp>

#include <algorithm>
#include <array>
#include <cassert>
#include <stdexcept>
#include <string>
#include <vector>

using namespace Types;

Instruction::Instruction(Type* ty, OpID id, BasicBlock* parent)
	: User(ty, ""), op_id_(id), parent_(parent)
{
	if (parent)
	{
		parent->add_instruction(this);
	}
}

Function* Instruction::get_function() const { return parent_->get_parent(); }

Module* Instruction::get_module() const { return parent_->get_module(); }

std::string Instruction::get_instr_op_name() const
{
	return print_instr_op_name(op_id_);
}

bool Instruction::is_void() const
{
	return ((op_id_ == ret) || (op_id_ == br) || (op_id_ == store) || (op_id_ == memcpy_) || (op_id_ == memclear_) ||
	        (op_id_ == call && this->get_type() == Types::VOID));
}

void Instruction::replaceAllOperandMatchs(const Value* from, Value* to)
{
	int size = static_cast<int>(get_operands().size());
	for (int i = 0; i < size; i++)
	{
		auto pre = get_operand(i);
		if (pre == from)
			set_operand(i, to);
	}
}

IBinaryInst::IBinaryInst(OpID id, Value* v1, Value* v2, BasicBlock* bb)
	: BaseInst<IBinaryInst>(INT, id, bb)
{
	assert(v1->get_type()==INT && v2->get_type()==INT&&
		"IBinaryInst operands are not both i32");
	add_operand(v1);
	add_operand(v2);
}

IBinaryInst* IBinaryInst::create_add(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(add, v1, v2, bb);
}

IBinaryInst* IBinaryInst::create_sub(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(sub, v1, v2, bb);
}

IBinaryInst* IBinaryInst::create_mul(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(mul, v1, v2, bb);
}

IBinaryInst* IBinaryInst::create_sdiv(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(sdiv, v1, v2, bb);
}

IBinaryInst* IBinaryInst::create_srem(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(srem, v1, v2, bb);
}

FBinaryInst::FBinaryInst(OpID id, Value* v1, Value* v2, BasicBlock* bb)
	: BaseInst<FBinaryInst>(FLOAT, id, bb)
{
	assert(v1->get_type()==FLOAT && v2->get_type()==FLOAT &&
		"FBinaryInst operands are not both float");
	add_operand(v1);
	add_operand(v2);
}

FBinaryInst* FBinaryInst::create_fadd(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fadd, v1, v2, bb);
}

FBinaryInst* FBinaryInst::create_fsub(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fsub, v1, v2, bb);
}

FBinaryInst* FBinaryInst::create_fmul(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fmul, v1, v2, bb);
}

FBinaryInst* FBinaryInst::create_fdiv(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fdiv, v1, v2, bb);
}

ICmpInst::ICmpInst(OpID id, Value* lhs, Value* rhs, BasicBlock* bb)
	: BaseInst<ICmpInst>(BOOL, id, bb)
{
	assert(lhs->get_type() == INT &&
		rhs->get_type() == INT &&
		"CmpInst operands are not both i32");
	add_operand(lhs);
	add_operand(rhs);
}

ICmpInst* ICmpInst::create_ge(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(ge, v1, v2, bb);
}

ICmpInst* ICmpInst::create_gt(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(gt, v1, v2, bb);
}

ICmpInst* ICmpInst::create_le(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(le, v1, v2, bb);
}

ICmpInst* ICmpInst::create_lt(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(lt, v1, v2, bb);
}

ICmpInst* ICmpInst::create_eq(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(eq, v1, v2, bb);
}

ICmpInst* ICmpInst::create_ne(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(ne, v1, v2, bb);
}

FCmpInst::FCmpInst(OpID id, Value* lhs, Value* rhs, BasicBlock* bb)
	: BaseInst<FCmpInst>(BOOL, id, bb)
{
	assert(lhs->get_type()== FLOAT &&
		rhs->get_type() == FLOAT &&
		"FCmpInst operands are not both float");
	add_operand(lhs);
	add_operand(rhs);
}

MemCpyInst::MemCpyInst(Value* from, Value* to, int size, BasicBlock* bb) : BaseInst<MemCpyInst>(VOID, memcpy_, bb)
{
	auto t1 = from->get_type();
	auto t2 = to->get_type();
	assert(
		t1 == t2 && t1->isPointerType() && t1->toPointerType()->typeContained() == CHAR &&
		"MemCpyInst operands are not both char*");
	assert(size > 0 && "MemCpyInst should have size > 0");
	add_operand(from);
	add_operand(to);
	length_ = size;
}

MemCpyInst* MemCpyInst::create_memcpy(Value* from, Value* to, int size, BasicBlock* bb)
{
	return create(from, to, size, bb);
}

MemClearInst::MemClearInst(Value* target, int size, BasicBlock* bb) : BaseInst<MemClearInst>(VOID, memclear_, bb)
{
	auto t1 = target->get_type();
	assert(
		t1->isPointerType() && t1->toPointerType()->typeContained() == CHAR &&
		"MemClearInst operand is not char*");
	assert(size > 0 && "MemClearInst should have size > 0");
	add_operand(target);
	length_ = size;
}

MemClearInst* MemClearInst::create_memclear(Value* target, int size, BasicBlock* bb)
{
	return create(target, size, bb);
}


Nump2CharpInst::Nump2CharpInst(Value* val, BasicBlock* bb) : BaseInst<Nump2CharpInst>(pointerType(CHAR), nump2charp, bb)
{
	assert(
		val->get_type()->isPointerType() && (val->get_type()->toPointerType()->typeContained() == FLOAT || val->get_type
			()->toPointerType()->
			typeContained() == INT) &&
		"Nump2CharpInst operand is not int/float*");
	add_operand(val);
}

Nump2CharpInst* Nump2CharpInst::create_nump2charp(Value* val, BasicBlock* bb)
{
	return create(val, bb);
}

GlobalFixInst::GlobalFixInst(GlobalVariable* val, BasicBlock* bb) : BaseInst<GlobalFixInst>(
	val->get_type(), global_fix, bb)
{
	add_operand(val);
}

GlobalFixInst* GlobalFixInst::create_global_fix(GlobalVariable* val, BasicBlock* bb)
{
	return create(val, bb);
}


FCmpInst* FCmpInst::create_fge(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fge, v1, v2, bb);
}

FCmpInst* FCmpInst::create_fgt(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fgt, v1, v2, bb);
}

FCmpInst* FCmpInst::create_fle(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fle, v1, v2, bb);
}

FCmpInst* FCmpInst::create_flt(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(flt, v1, v2, bb);
}

FCmpInst* FCmpInst::create_feq(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(feq, v1, v2, bb);
}

FCmpInst* FCmpInst::create_fne(Value* v1, Value* v2, BasicBlock* bb)
{
	return create(fne, v1, v2, bb);
}

CallInst::CallInst(Function* func, const std::vector<Value*>& args, BasicBlock* bb)
	: BaseInst<CallInst>(func->get_return_type(), call, bb)
{
	assert(func->get_type()->isFunctionType() && "Not a function");
	assert((func->get_num_of_args() == args.size()) && "Wrong number of args");
	add_operand(func);
	auto func_type = dynamic_cast<FuncType*>(func->get_type());
	for (unsigned i = 0; i < args.size(); i++)
	{
		assert(func_type->argumentType(i) == args[i]->get_type() &&
			"CallInst: Wrong arg type");
		add_operand(args[i]);
	}
}

CallInst* CallInst::create_call(Function* func, const std::vector<Value*>& args,
                                BasicBlock* bb)
{
	return create(func, args, bb);
}

FuncType* CallInst::get_function_type() const
{
	return dynamic_cast<FuncType*>(get_operand(0)->get_type());
}

BranchInst::BranchInst(Value* cond, BasicBlock* if_true, BasicBlock* if_false,
                       BasicBlock* bb)
	: BaseInst<BranchInst>(VOID, br, bb)
{
	if (cond == nullptr)
	{
		// conditionless jump
		assert(if_false == nullptr && "Given false-bb on conditionless jump");
		add_operand(if_true);
		// prev/succ
		if_true->add_pre_basic_block(bb);
		bb->add_succ_basic_block(if_true);
	}
	else
	{
		assert(cond->get_type()==BOOL &&
			"BranchInst condition is not i1");
		add_operand(cond);
		add_operand(if_true);
		add_operand(if_false);
		// prev/succ
		if_true->add_pre_basic_block(bb);
		if_false->add_pre_basic_block(bb);
		bb->add_succ_basic_block(if_true);
		bb->add_succ_basic_block(if_false);
	}
}

BranchInst::~BranchInst()
{
	std::list<BasicBlock*> succs;
	if (is_cond_br())
	{
		succs.push_back(dynamic_cast<BasicBlock*>(get_operand(1)));
		succs.push_back(dynamic_cast<BasicBlock*>(get_operand(2)));
	}
	else
	{
		succs.push_back(dynamic_cast<BasicBlock*>(get_operand(0)));
	}
	for (auto succ_bb : succs)
	{
		if (succ_bb)
		{
			succ_bb->remove_pre_basic_block(get_parent());
			get_parent()->remove_succ_basic_block(succ_bb);
		}
	}
}


BranchInst* BranchInst::create_cond_br(Value* cond, BasicBlock* if_true,
                                       BasicBlock* if_false, BasicBlock* bb)
{
	return create(cond, if_true, if_false, bb);
}

BranchInst* BranchInst::create_br(BasicBlock* if_true, BasicBlock* bb)
{
	return create(nullptr, if_true, nullptr, bb);
}

ReturnInst::ReturnInst(Value* val, BasicBlock* bb)
	: BaseInst<ReturnInst>(VOID, ret, bb)
{
	if (val == nullptr)
	{
		assert(bb->get_parent()->get_return_type()==VOID);
	}
	else
	{
		assert(bb->get_parent()->get_return_type()!=VOID &&
			"Void function returning a value");
		assert(bb->get_parent()->get_return_type() == val->get_type() &&
			"ReturnInst type is different from function return type");
		add_operand(val);
	}
}

ReturnInst* ReturnInst::create_ret(Value* val, BasicBlock* bb)
{
	return create(val, bb);
}

ReturnInst* ReturnInst::create_void_ret(BasicBlock* bb)
{
	return create(nullptr, bb);
}

bool ReturnInst::is_void_ret() const { return get_num_operand() == 0; }

GetElementPtrInst::GetElementPtrInst(Value* ptr, const std::vector<Value*>& idxs,
                                     BasicBlock* bb)
	: BaseInst<GetElementPtrInst>(pointerType(get_element_type(ptr, idxs)),
	                              getelementptr, bb)
{
	add_operand(ptr);
	for (auto idx : idxs)
	{
		assert(idx->get_type() == INT && "Index is not integer");
		add_operand(idx);
	}
}

Type* GetElementPtrInst::get_element_type(const Value* ptr,
                                          const std::vector<Value*>& idxs)
{
	assert(ptr->get_type()->isPointerType() &&
		"GetElementPtrInst ptr is not a pointer");

	Type* ty = dynamic_cast<PointerType*>(ptr->get_type())->typeContained();
	assert(
		"GetElementPtrInst ptr is wrong type" &&
		(ty->isArrayType() || ty == INT || ty == FLOAT));
	if (ty->isArrayType())
	{
		ArrayType* arr_ty = dynamic_cast<ArrayType*>(ty);
		for (unsigned i = 1; i < idxs.size(); i++)
		{
			ty = arr_ty->getSubType(1);
			if (i < idxs.size() - 1)
			{
				assert(ty->isArrayType() && "Index error!");
			}
			if (ty->isArrayType())
			{
				arr_ty = dynamic_cast<ArrayType*>(ty);
			}
		}
	}
	return ty;
}

Type* GetElementPtrInst::get_element_type() const
{
	return get_type()->toPointerType()->typeContained();
}

GetElementPtrInst* GetElementPtrInst::create_gep(Value* ptr,
                                                 const std::vector<Value*>& idxs,
                                                 BasicBlock* bb)
{
	return create(ptr, idxs, bb);
}

StoreInst::StoreInst(Value* val, Value* ptr, BasicBlock* bb)
	: BaseInst<StoreInst>(VOID, store, bb)
{
	assert((ptr->get_type()->toPointerType()->typeContained() == val->get_type()) &&
		"StoreInst ptr is not a pointer to val type");
	add_operand(val);
	add_operand(ptr);
}

StoreInst* StoreInst::create_store(Value* val, Value* ptr, BasicBlock* bb)
{
	return create(val, ptr, bb);
}

GlobalVariable* GlobalFixInst::get_var() const { return dynamic_cast<GlobalVariable*>(this->get_operand(0)); }


LoadInst::LoadInst(Value* ptr, BasicBlock* bb)
	: BaseInst<LoadInst>(ptr->get_type()->toPointerType()->typeContained(), load,
	                     bb)
{
	assert((get_type() == INT or get_type() == FLOAT or
			get_type()->isPointerType()) &&
		"Should not load value with type except int/float");
	add_operand(ptr);
}

LoadInst* LoadInst::create_load(Value* ptr, BasicBlock* bb)
{
	return create(ptr, bb);
}

AllocaInst::AllocaInst(Type* ty, BasicBlock* bb)
	: BaseInst<AllocaInst>(pointerType(ty), alloca_, bb)
{
	static constexpr std::array<TypeIDs, 4> allowed_alloc_type = {
		TypeIDs::Integer, TypeIDs::Float, TypeIDs::Array, TypeIDs::Pointer
	};
	assert(std::find(allowed_alloc_type.begin(), allowed_alloc_type.end(),
			ty->getTypeID()) != allowed_alloc_type.end() &&
		"Not allowed type for alloca");
}

AllocaInst* AllocaInst::create_alloca(Type* ty, BasicBlock* bb)
{
	return create(ty, bb);
}

Type* AllocaInst::get_alloca_type() const
{
	return get_type()->toPointerType()->typeContained();
}

ZextInst::ZextInst(Value* val, Type* ty, BasicBlock* bb)
	: BaseInst<ZextInst>(ty, zext, bb)
{
	assert(val->get_type() == BOOL &&
		"ZextInst operand is not bool");
	assert(ty == INT && "ZextInst destination type is not integer");
	add_operand(val);
}

ZextInst* ZextInst::create_zext_to_i32(Value* val, BasicBlock* bb)
{
	return create(val, INT, bb);
}

FpToSiInst::FpToSiInst(Value* val, Type* ty, BasicBlock* bb)
	: BaseInst<FpToSiInst>(ty, fptosi, bb)
{
	assert(val->get_type() == FLOAT &&
		"FpToSiInst operand is not float");
	assert(ty == INT &&
		"FpToSiInst destination type is not integer");
	add_operand(val);
}

FpToSiInst* FpToSiInst::create_fptosi(Value* val, Type* ty, BasicBlock* bb)
{
	return create(val, ty, bb);
}

FpToSiInst* FpToSiInst::create_fptosi_to_i32(Value* val, BasicBlock* bb)
{
	return create(val, INT, bb);
}

SiToFpInst::SiToFpInst(Value* val, Type* ty, BasicBlock* bb)
	: BaseInst<SiToFpInst>(ty, sitofp, bb)
{
	assert(val->get_type() == INT &&
		"SiToFpInst operand is not integer");
	assert(ty == FLOAT && "SiToFpInst destination type is not float");
	add_operand(val);
}

SiToFpInst* SiToFpInst::create_sitofp(Value* val, BasicBlock* bb)
{
	return create(val, FLOAT, bb);
}

PhiInst::PhiInst(Type* ty, const std::vector<Value*>& vals,
                 const std::vector<BasicBlock*>& val_bbs, BasicBlock* bb)
	: BaseInst<PhiInst>(ty, phi)
{
	assert(vals.size() == val_bbs.size() && "Unmatched vals and bbs");
	for (unsigned i = 0; i < vals.size(); i++)
	{
		assert(ty == vals[i]->get_type() && "Bad type for phi");
		add_operand(vals[i]);
		add_operand(val_bbs[i]);
	}
	this->set_parent(bb);
}

PhiInst* PhiInst::create_phi(Type* ty, BasicBlock* bb,
                             const std::vector<Value*>& vals,
                             const std::vector<BasicBlock*>& val_bbs)
{
	return create(ty, vals, val_bbs, bb);
}
