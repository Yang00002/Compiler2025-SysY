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

#define OPEN_ASSERT 0
#include <unordered_set>

#include "Util.hpp"

using namespace Types;

Instruction::Instruction(Type* ty, OpID id, BasicBlock* parent)
	: User(ty, ""), parent_(parent), op_id_(id)
{
	if (parent)
	{
		parent->add_instruction(this);
	}
}

namespace
{
	Value* getOrDefault(std::unordered_map<Value*, Value*>& m, Value* t)
	{
		auto fd = m.find(t);
		if (fd == m.end()) return t;
		return fd->second;
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
	int size = u2iNegThrow(get_operands().size());
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

Instruction* IBinaryInst::copy(BasicBlock* parent)
{
	return new IBinaryInst{get_instr_type(), get_operand(0), get_operand(1), parent};
}

Instruction* IBinaryInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new IBinaryInst{
		get_instr_type(), getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), nullptr
	};
	valMap[this] = ret;
	return ret;
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

Instruction* FBinaryInst::copy(BasicBlock* parent)
{
	return new FBinaryInst{get_instr_type(), get_operand(0), get_operand(1), parent};
}

Instruction* FBinaryInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new FBinaryInst{
		get_instr_type(), getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), nullptr
	};
	valMap[this] = ret;
	return ret;
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

Instruction* ICmpInst::copy(BasicBlock* parent)
{
	return new ICmpInst{get_instr_type(), get_operand(0), get_operand(1), parent};
}

Instruction* ICmpInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new ICmpInst{
		get_instr_type(), getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), nullptr
	};
	valMap[this] = ret;
	return ret;
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

Instruction* FCmpInst::copy(BasicBlock* parent)
{
	return new FCmpInst{get_instr_type(), get_operand(0), get_operand(1), parent};
}

Instruction* FCmpInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new FCmpInst{
		get_instr_type(), getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), nullptr
	};
	valMap[this] = ret;
	return ret;
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

Instruction* MemCpyInst::copy(BasicBlock* parent)
{
	return new MemCpyInst{get_operand(0), get_operand(1), length_, parent};
}

Instruction* MemCpyInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto inst = new MemCpyInst{
		getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), length_, nullptr
	};
	return inst;
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

Instruction* MemClearInst::copy(BasicBlock* parent)
{
	return new MemClearInst{get_operand(0), length_, parent};
}

Instruction* MemClearInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new MemClearInst{getOrDefault(valMap, get_operand(0)), length_, nullptr};
	return ret;
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

Instruction* Nump2CharpInst::copy(BasicBlock* parent)
{
	return new Nump2CharpInst{get_operand(0), parent};
}

Instruction* Nump2CharpInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new Nump2CharpInst{getOrDefault(valMap, get_operand(0)), nullptr};
	valMap[this] = ret;
	return ret;
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

Instruction* GlobalFixInst::copy(BasicBlock* parent)
{
	return new GlobalFixInst{dynamic_cast<GlobalVariable*>(get_operand(0)), parent};
}

Instruction* GlobalFixInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new GlobalFixInst{dynamic_cast<GlobalVariable*>(get_operand(0)), nullptr};
	valMap[this] = ret;
	return ret;
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
	int size = u2iNegThrow(args.size());
	assert((func->get_num_of_args() == size) && "Wrong number of args");
	add_operand(func);
	auto func_type = dynamic_cast<FuncType*>(func->get_type());
	for (int i = 0; i < size; i++)
	{
		assert(func_type->argumentType(i) == args[i]->get_type() &&
			"CallInst: Wrong arg type");
		add_operand(args[i]);
	}
}

Instruction* CallInst::copy(BasicBlock* parent)
{
	std::vector<Value*> args;
	args.resize(get_operands().size() - 1);
	for (int i = 0, size = u2iNegThrow(get_operands().size()); i < size; i++) args[i] = get_operand(i + 1);
	return new CallInst{dynamic_cast<Function*>(get_operand(0)), args, parent};
}

Instruction* CallInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	std::vector<Value*> args;
	args.resize(get_operands().size() - 1);
	for (int i = 0, size = u2iNegThrow(get_operands().size()) - 1; i < size; i++)
		args[i] = getOrDefault(valMap, get_operand(i + 1));
	auto ret = new CallInst{dynamic_cast<Function*>(get_operand(0)), args, nullptr};
	valMap[this] = ret;
	return ret;
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

Instruction* BranchInst::copy(BasicBlock* parent)
{
	return nullptr;
}

Instruction* BranchInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	return nullptr;
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

Instruction* ReturnInst::copy(BasicBlock* parent)
{
	if (get_operands().size() == 1)
		return new ReturnInst{get_operand(0), parent};
	return new ReturnInst{nullptr, parent};
}

Instruction* ReturnInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	return nullptr;
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

Instruction* GetElementPtrInst::copy(BasicBlock* parent)
{
	std::vector<Value*> args;
	args.resize(get_operands().size() - 1);
	for (int i = 0, size = u2iNegThrow(get_operands().size()); i < size; i++) args[i] = get_operand(i + 1);
	return new GetElementPtrInst{get_operand(0), args, parent};
}

Instruction* GetElementPtrInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	std::vector<Value*> args;
	args.resize(get_operands().size() - 1);
	for (int i = 0, size = u2iNegThrow(get_operands().size()); i < size; i++)
		args[i] = getOrDefault(valMap, get_operand(i + 1));
	auto ret = new GetElementPtrInst{getOrDefault(valMap, get_operand(0)), args, nullptr};
	valMap[this] = ret;
	return ret;
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
		int size = u2iNegThrow(idxs.size());
		for (int i = 1; i < size; i++)
		{
			ty = arr_ty->getSubType(1);
			if (i < size - 1)
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

Instruction* StoreInst::copy(BasicBlock* parent)
{
	return new StoreInst{get_operand(0), get_operand(1), parent};
}

Instruction* StoreInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new StoreInst{getOrDefault(valMap, get_operand(0)), getOrDefault(valMap, get_operand(1)), nullptr};
	return ret;
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

Instruction* LoadInst::copy(BasicBlock* parent)
{
	return new LoadInst{get_operand(0), parent};
}

Instruction* LoadInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new LoadInst{getOrDefault(valMap, get_operand(0)), nullptr};
	valMap[this] = ret;
	return ret;
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

Instruction* AllocaInst::copy(BasicBlock* parent)
{
	return new AllocaInst{get_alloca_type(), parent};
}

Instruction* AllocaInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	return nullptr;
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

Instruction* ZextInst::copy(BasicBlock* parent)
{
	return new ZextInst{get_operand(0), get_type(), parent};
}

Instruction* ZextInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new ZextInst{getOrDefault(valMap, get_operand(0)), get_type(), nullptr};
	valMap[this] = ret;
	return ret;
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

Instruction* FpToSiInst::copy(BasicBlock* parent)
{
	return new FpToSiInst{get_operand(0), get_type(), parent};
}

Instruction* FpToSiInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new FpToSiInst{getOrDefault(valMap, get_operand(0)), get_type(), nullptr};
	valMap[this] = ret;
	return ret;
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

Instruction* SiToFpInst::copy(BasicBlock* parent)
{
	return new SiToFpInst{get_operand(0), get_type(), parent};
}

Instruction* SiToFpInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	auto ret = new SiToFpInst{getOrDefault(valMap, get_operand(0)), get_type(), nullptr};
	valMap[this] = ret;
	return ret;
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
	int size = u2iNegThrow(vals.size());
	for (int i = 0; i < size; i++)
	{
		ASSERT(val_bbs[i]);
		assert(ty == vals[i]->get_type() && "Bad type for phi");
		add_operand(vals[i]);
		add_operand(val_bbs[i]);
	}
	this->set_parent(bb);
}

Instruction* PhiInst::copy(BasicBlock* parent)
{
	std::vector<Value*> vals;
	std::vector<BasicBlock*> val_bbs;
	for (auto [v,b] : get_phi_pairs())
	{
		vals.emplace_back(v);
		val_bbs.emplace_back(b);
	}
	return new PhiInst{get_type(), vals, val_bbs, parent};
}

Instruction* PhiInst::copy(std::unordered_map<Value*, Value*>& valMap)
{
	return nullptr;
}

PhiInst* PhiInst::create_phi(Type* ty, BasicBlock* bb,
                             const std::vector<Value*>& vals,
                             const std::vector<BasicBlock*>& val_bbs)
{
	return create(ty, vals, val_bbs, bb);
}

void PhiInst::add_phi_pair_operand(Value* val, BasicBlock* pre_bb)
{
	ASSERT(val != nullptr);
	ASSERT(pre_bb != nullptr);
	this->add_operand(val);
	this->add_operand(pre_bb);
}


void PhiInst::remove_phi_operand(const Value* pre_bb)
{
	ASSERT(allOpNotNull());
	for (int i = 0; i < this->get_num_operand(); i += 2)
	{
		if (this->get_operand(i + 1) == pre_bb)
		{
			this->remove_operand(i);
			this->remove_operand(i);
			return;
		}
	}
	ASSERT(allOpNotNull());
}

void PhiInst::remove_phi_operandIfIn(const std::unordered_set<BasicBlock*>& in)
{
	int opc = get_num_operand();
	for (int i = opc - 1; i >= 0; i -= 2)
	{
		auto bb = dynamic_cast<BasicBlock*>(get_operand(i));
		if (in.count(bb))
		{
			remove_operand(i);
			remove_operand(i - 1);
		}
	}
}

void PhiInst::remove_phi_operand(const Value* pre_bb, int opId)
{
	ASSERT(allOpNotNull());
	if (get_operand(opId) == pre_bb)
	{
		this->remove_operand(opId);
		this->remove_operand(opId - 1);
	}
	ASSERT(allOpNotNull());
}
