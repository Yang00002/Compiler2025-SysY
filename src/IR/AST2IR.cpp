#include "../../include/ir/AST2IR.hpp"
#include "../../include/ast/Type.hpp"
#include "../../include/ast/Ast.hpp"
#include "../../include/ast/Tensor.hpp"
#include "../../include/ir/Module.hpp"
#include "../../include/ir/IRBuilder.hpp"
#include "../../include/ir/BasicBlock.hpp"

using namespace std;
using namespace Types;


AST2IRVisitor::AST2IRVisitor()
{
	_module = new Module{};
	_builder = new IRBuilder(nullptr, _module);
}

Module* AST2IRVisitor::getModule() const { return _module; }

Value* AST2IRVisitor::visit(ASTCompUnit* comp_unit)
{
	_var_scope.enter();
	_func_scope.enter();

	for (auto& i : comp_unit->lib())
	{
		std::vector<Type*> inputTypes;
		auto& input = i->args();
		for (auto& j : input)
		{
			auto t = j->getType();
			if (t->isArrayType()) t = t->getPointerTypeContainSelf();
			inputTypes.emplace_back(t);
		}
		auto* func = Function::create(Types::functionType(i->returnType(), inputTypes), i->id(), _module, true);
		_func_scope.push(i->id(), func);
	}

	for (auto& i : comp_unit->var_declarations())
	{
		// 常数非数组全局变量的值已经被内联, 不会被使用
		if (i->isConst() && i->getType()->isBasicType()) continue;
		const auto glob = GlobalVariable::create(i->id(), _builder->get_module(), i->getType(), i->isConst(),
		                                         i->getInitList());
		_var_scope.push(i->id(), glob);
	}

	for (auto& i : comp_unit->func_declarations()) i->accept(this);

	return nullptr;
}

Value* AST2IRVisitor::visit(ASTVarDecl* decl)
{
	Type* type = decl->getType();
	auto scope = AllocaInst::create_alloca(type, _builder->get_insert_block());
	_var_scope.push(decl->id(), scope);
	auto ini = decl->getInitList();
	if (ini != nullptr)
	{
		if (decl->getType()->isBasicType())
		{
			auto v = ini->visitData()->getElement(0);
			Value* val = nullptr;
			if (v.isConstant())
				val = _builder->create_constant(v.toConstant());
			else
				val = v.getExpression()->accept(this);
			_builder->create_store(val, scope);
			return nullptr;
		}
		// 数组初始化调度策略
		// 将数组分为 4 个一段
		// 初始化为全 expression	-> 忽略该段
		// 初始化为全 default		-> 使用 memclear
		// 非上述情况				-> 使用 memcpy
		// 剩下不能分段的			-> 使用 store
		// 然后我们将 expression store 到对应位置
		vector<int> segmentOpTypes;
		vector<int> segmentLength;
		vector<InitializeValue> prefill;
		InitializeValue prefill_cache[4];
		// 0b01 非全 default, 0b10 非全 exp
		int seg_type = 0;
		int fill = 0;
		for (auto i = ini->getIterator(); !i.isEnd(); ++i)
		{
			switch (i.getCurrentIterateType())
			{
				case TensorIterateType::VALUE:
					{
						auto v = i.getValue();
						if (v.isExpression())
						{
							seg_type |= 0b01;
							prefill_cache[fill] = ini->defaultValue();
						}
						else
						{
							seg_type |= 0b10;
							if (v != ini->defaultValue())
							{
								seg_type |= 0b01;
								prefill_cache[fill] = ini->defaultValue();
							}
							else prefill_cache[fill] = v;
						}
						fill++;
						if (fill == 4)
						{
							fill = 0;
							if (segmentOpTypes.empty() || segmentOpTypes.back() != seg_type)
							{
								segmentOpTypes.emplace_back(seg_type);
								segmentLength.emplace_back(1);
							}
							else
								segmentLength.back()++;
							if (seg_type == 0b11)
							{
								for (auto& p : prefill_cache) prefill.emplace_back(p);
							}
							seg_type = 0;
						}
						break;
					}
				case TensorIterateType::DEFAULT_VALUES:
					{
						auto [d, l] = i.getDefaultValues();
						int lim = min(fill + l, 4);
						for (int p = fill; p < lim; p++) prefill_cache[p] = ini->defaultValue();
						fill += l;
						seg_type |= 0b10;
						if (fill >= 4)
						{
							if (segmentOpTypes.empty() || segmentOpTypes.back() != seg_type)
							{
								segmentOpTypes.emplace_back(seg_type);
								segmentLength.emplace_back(1);
							}
							else
								segmentLength.back()++;
							fill -= 4;
							if (seg_type == 0b11)
							{
								for (auto& p : prefill_cache) prefill.emplace_back(p);
							}
							seg_type = 0;
						}
						int len = fill >> 2;
						fill &= 0b11;
						if (len > 0)
						{
							if (segmentOpTypes.empty() || segmentOpTypes.back() != seg_type)
							{
								segmentOpTypes.emplace_back(seg_type);
								segmentLength.emplace_back(len);
							}
							else
								segmentLength.back() += len;
						}
						if (fill > 0)
						{
							seg_type |= 0b10;
							for (int p = 0; p < fill; p++) prefill_cache[p] = ini->defaultValue();
						}
						break;
					}
				default:
					break;
			}
		}
		int ps = static_cast<int>(prefill.size());
		// 为 memcpy 初始化分配常量全局变量
		GlobalVariable* glob = nullptr;
		if (ps > 0)
		{
			Tensor<InitializeValue>* tensor = new Tensor<InitializeValue>{vector<int>{ps}, ini->defaultValue()};
			for (auto& i : prefill) tensor->getData()->append(i);
			auto id = createPrivateGlobalVarID();
			glob = GlobalVariable::create(id, _builder->get_module(),
			                              Types::arrayType(ini->defaultValue().getExpressionType(), false,
			                                               {static_cast<unsigned>(ps)}),
			                              true, tensor);
			_var_scope.pushFront(id, glob);
			delete tensor;
		}

		int ed = static_cast<int>(segmentOpTypes.size());
		int selfIdx = 0;
		int cpIdx = 0;
		for (int i = 0; i < ed; i++)
		{
			int ty = segmentOpTypes[i];
			int len = segmentLength[i];
			// memclear
			if (ty == 0b10)
			{
				auto dest = _builder->create_nump2charp(
					_builder->create_gep(scope, index(ini->getShape(), selfIdx, 1)));
				_builder->create_memclear(dest, len << 4);
			}
			// memcpy
			else if (ty == 0b11)
			{
				auto dest = _builder->create_nump2charp(
					_builder->create_gep(scope, index(ini->getShape(), selfIdx, 1)));
				auto t_dest = _builder->create_nump2charp(_builder->create_gep(glob,
				                                                               {
					                                                               _builder->create_constant(0),
					                                                               _builder->create_constant(cpIdx)
				                                                               }));
				_builder->create_memcpy(t_dest, dest, len << 4);
				cpIdx += len << 2;
			}

			selfIdx += len << 2;
		}
		int top = selfIdx;
		selfIdx = 0;
		for (int i = 0; i < ed; i++)
		{
			int ty = segmentOpTypes[i];
			int len = segmentLength[i];
			if (ty == 0b01)
			{
				for (int ii = 0; ii < 4; ii++)
				{
					auto dest =
						_builder->create_gep(scope, index(ini->getShape(), selfIdx + ii, 1));
					auto v = ini->visitData()->getElement(selfIdx + ii);
					auto exp = v.getExpression();
					auto expv = exp->accept(this);
					_builder->create_store(expv, dest);
				}
			}
			else if (ty == 0b11)
			{
				for (int ii = 0; ii < 4; ii++)
				{
					auto v = ini->visitData()->getElement(selfIdx + ii);
					if (v.isExpression())
					{
						auto dest =
							_builder->create_gep(scope, index(ini->getShape(), selfIdx + ii, 1));
						auto exp = v.getExpression();
						auto expv = exp->accept(this);
						_builder->create_store(expv, dest);
					}
				}
			}
			selfIdx += len << 2;
		}
		int cap = ini->getDimCapacity(-1);
		for (int i = top; i < cap; i++)
		{
			auto v = ini->visitData()->getElement(i);
			auto dest =
				_builder->create_gep(scope, index(ini->getShape(), i, 1));
			if (v.isExpression())
			{
				auto exp = v.getExpression();
				auto expv = exp->accept(this);
				_builder->create_store(expv, dest);
			}
			else
			{
				_builder->create_store(_builder->create_constant(v.toConstant()), dest);
			}
		}
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTFuncDecl* func_decl)
{
	std::vector<Type*> inputTypes;
	auto& input = func_decl->args();
	for (auto& j : input)
	{
		auto t = j->getType();
		if (t->isArrayType()) t = t->getPointerTypeContainSelf();
		inputTypes.emplace_back(t);
	}
	const auto func = Function::create(Types::functionType(func_decl->returnType(), inputTypes), func_decl->id(),
	                                   _module);
	_func_scope.push(func_decl->id(), func);
	_functionBelong = func;
	const auto funBB = BasicBlock::create(_module, "entry", func);
	_builder->set_insert_point(funBB);
	_var_scope.enter();
	const int argCount = static_cast<int>(func_decl->args().size());
	auto it = func->get_args().begin();
	for (int i = 0; i < argCount; ++i, ++it)
	{
		auto& ast_arg = func_decl->args()[i];
		auto& arg = *it;
		const auto inter = _builder->create_alloca(inputTypes[i]);
		_builder->create_store(&arg, inter);
		_var_scope.push(ast_arg->id(), inter);
	}
	std::vector<Value*> args;
	for (auto& arg : func->get_args())
	{
		args.push_back(&arg);
	}
	func_decl->block()->accept(this);
	if (not _builder->get_insert_block()->is_terminated())
	{
		if (func->get_return_type() == Types::VOID)
			_builder->create_void_ret();
		else if (func->get_return_type() == Types::FLOAT)
			_builder->create_ret(_builder->create_constant(0.0f));
		else
			_builder->create_ret(_builder->create_constant(0));
	}
	_var_scope.exit();
	return nullptr;
}

// getelement 第一维对指针操作, 其它对数组操作, 输出为指针
// lval 对于输出的 i32* load 得到 i32*; 对于 []* 保留 *
// call 对于输入的 []* 转换为 *
Value* AST2IRVisitor::visit(ASTLVal* l_val)
{
	auto var = _var_scope.find(l_val->getDeclaration()->id());
	if (auto glob = dynamic_cast<GlobalVariable*>(var);
		glob != nullptr && glob->get_type()->toPointerType()->typeContained()->isArrayType() && glob->get_type()->
		print() != glob->printInitType() + "*")
	{
		var = _builder->create_global_fix(glob);
	}
	auto var_type = var->get_type()->toPointerType()->typeContained();
	if (var_type->isBasicType())
		return var;
	auto& idx = l_val->index();
	// ArrayInParameter
	if (var_type->isPointerType())
	{
		// ? ** -> ? *
		var = _builder->create_load(var);
		vector<Value*> indexes;
		indexes.reserve(idx.size());
		for (auto& i : idx)
			indexes.emplace_back(i->accept(this));
		// [a x [b x ?]]* / i32*
		if (!indexes.empty())
		{
			// [a x [b x ?]]* -> ?* / i32* -> i32*
			var = _builder->create_gep(var, indexes);
			return var;
		}
		// []* -> []*
		return var;
	}
	// [a x [b x i32]]*
	if (idx.empty())
		// [a x [b x i32]]* -> [a x [b x i32]]*
		return var;
	vector<Value*> indexes;
	indexes.reserve(idx.size() + 1);
	indexes.emplace_back(_builder->create_constant(0));
	for (auto& i : idx)
		indexes.emplace_back(i->accept(this));
	var = _builder->create_gep(var, indexes);
	return var;
}

// getelement 第一维对指针操作, 其它对数组操作, 输出为指针
// lval 对于输出的 i32* load 得到 i32; 对于 []* 保留 *
// call 对于输入的 []* 转换为 *
Value* AST2IRVisitor::visit(ASTRVal* r_val)
{
	auto var = _var_scope.find(r_val->getDeclaration()->id());
	if (auto glob = dynamic_cast<GlobalVariable*>(var);
		glob != nullptr && glob->get_type()->toPointerType()->typeContained()->isArrayType() && glob->get_type()->
		print() != glob->printInitType())
	{
		var = _builder->create_global_fix(glob);
	}
	auto var_type = var->get_type()->toPointerType()->typeContained();
	if (var_type->isBasicType())
		return _builder->create_load(var);
	auto& idx = r_val->index();
	// ArrayInParameter
	if (var_type->isPointerType())
	{
		var_type = var_type->toPointerType()->typeContained();
		// ? ** -> ? *
		var = _builder->create_load(var);
		int dimAll = static_cast<int>(var_type->toArrayType()->dimensions().size()) + 1;
		int dimGet = static_cast<int>(idx.size());
		vector<Value*> indexes;
		indexes.reserve(idx.size());
		for (auto& i : idx)
			indexes.emplace_back(i->accept(this));
		// [a x [b x ?]]* / i32*
		if (!indexes.empty())
		{
			// [a x [b x ?]]* -> ?* / i32* -> i32*
			var = _builder->create_gep(var, indexes);
			// hit: [a x [b x i32]]* -> i32* -> i32
			// mis: [a x [b x i32]]* -> [b x i32]*
			if (dimAll == dimGet)
				var = _builder->create_load(var);
			return var;
		}
		// []* -> []*
		return var;
	}
	// [a x [b x i32]]*
	if (idx.empty())
		// [a x [b x i32]]* -> [a x [b x i32]]*
		return var;
	vector<Value*> indexes;
	indexes.reserve(idx.size() + 1);
	indexes.emplace_back(_builder->create_constant(0));
	for (auto& i : idx)
		indexes.emplace_back(i->accept(this));
	int dimAll = static_cast<int>(var_type->toArrayType()->dimensions().size());
	int dimGet = static_cast<int>(idx.size());
	var = _builder->create_gep(var, indexes);
	// hit: [a x [b x i32]]* -> i32* -> i32
	// mis: [a x [b x i32]]* -> [b x i32]*
	if (dimAll == dimGet)
		var = _builder->create_load(var);
	return var;
}

Value* AST2IRVisitor::visit(ASTNumber* number)
{
	return _builder->create_constant(number->toInitializeValue().toConstant());
}

Value* AST2IRVisitor::visit(ASTCast* cast)
{
	auto v = cast->source();
	auto t = cast->cast_to();
	auto type = v->getExpressionType();
	if (t == Types::BOOL)
	{
		if (type == Types::INT)
		{
			auto cond = _builder->create_icmp_ne(v->accept(this), _builder->create_constant(0));
			_builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
			return nullptr;
		}
		auto cond = _builder->create_fcmp_ne(v->accept(this), _builder->create_constant(0.0f));
		_builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
		return nullptr;
	}
	if (t == Types::INT)
	{
		if (type == Types::BOOL)
		{
			// BOOL -> INT
			auto bbt = BasicBlock::create(_module, "", _functionBelong);
			auto bbf = BasicBlock::create(_module, "", _functionBelong);
			_true_targets.emplace_back(bbt);
			_false_targets.emplace_back(bbf);
			v->accept(this);
			_true_targets.pop_back();
			_false_targets.pop_back();
			vector<BasicBlock*> labels;
			labels.reserve(bbf->get_pre_basic_blocks().size() + 1);
			vector<Value*> gets;
			gets.reserve(bbf->get_pre_basic_blocks().size() + 1);
			auto tv = _builder->create_constant(1);
			auto fv = _builder->create_constant(0);
			for (auto& i : bbf->get_pre_basic_blocks())
			{
				labels.emplace_back(i);
				gets.emplace_back(fv);
			}
			labels.emplace_back(bbt);
			gets.emplace_back(tv);
			_builder->set_insert_point(bbt);
			_builder->create_br(bbf);
			_builder->set_insert_point(bbf);
			return _builder->create_phi(INT, gets, labels);
		}
		return _builder->create_fptosi_to_i32(v->accept(this));
	}
	if (type == Types::BOOL)
	{
		// BOOL -> FLOAT
		auto bbt = BasicBlock::create(_module, "", _functionBelong);
		auto bbf = BasicBlock::create(_module, "", _functionBelong);
		_true_targets.emplace_back(bbt);
		_false_targets.emplace_back(bbf);
		v->accept(this);
		_true_targets.pop_back();
		_false_targets.pop_back();
		vector<BasicBlock*> labels;
		labels.reserve(bbf->get_pre_basic_blocks().size() + 1);
		vector<Value*> gets;
		gets.reserve(bbf->get_pre_basic_blocks().size() + 1);
		auto tv = _builder->create_constant(1.0f);
		auto fv = _builder->create_constant(0.0f);
		for (auto& i : bbf->get_pre_basic_blocks())
		{
			labels.emplace_back(i);
			gets.emplace_back(fv);
		}
		labels.emplace_back(bbt);
		gets.emplace_back(tv);
		_builder->set_insert_point(bbt);
		_builder->create_br(bbf);
		_builder->set_insert_point(bbf);
		return _builder->create_phi(FLOAT, gets, labels);
	}
	return _builder->create_sitofp(v->accept(this));
}

Value* AST2IRVisitor::visit(ASTMathExp* math_exp)
{
	auto l = math_exp->l()->accept(this);
	auto r = math_exp->r()->accept(this);
	bool flt = l->get_type() == Types::FLOAT;
	switch (math_exp->op())
	{
		case MathOP::ADD:
			if (flt) return _builder->create_fadd(l, r);
			return _builder->create_iadd(l, r);
		case MathOP::SUB:
			if (flt) return _builder->create_fsub(l, r);
			return _builder->create_isub(l, r);
		case MathOP::MUL:
			if (flt) return _builder->create_fmul(l, r);
			return _builder->create_imul(l, r);
		case MathOP::DIV:
			if (flt) return _builder->create_fdiv(l, r);
			return _builder->create_isdiv(l, r);
		case MathOP::MOD:
			return _builder->create_isrem(l, r);
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTLogicExp* logic_exp)
{
	int exp_num = static_cast<int>(logic_exp->exps().size());
	if (logic_exp->op() == LogicOP::OR)
	{
		for (int i = 0; i < exp_num; ++i)
		{
			if (i != exp_num - 1)
			{
				BasicBlock* check_rhs = BasicBlock::create(_module, "", _functionBelong);
				_false_targets.emplace_back(check_rhs);
				logic_exp->exps()[i]->accept(this);
				_false_targets.pop_back();
				_builder->set_insert_point(check_rhs);
			}
			else
				logic_exp->exps()[i]->accept(this);
		}
	}
	else
	{
		for (int i = 0; i < exp_num; ++i)
		{
			if (i != exp_num - 1)
			{
				BasicBlock* check_rhs = BasicBlock::create(_module, "", _functionBelong);
				_true_targets.emplace_back(check_rhs);
				logic_exp->exps()[i]->accept(this);
				_true_targets.pop_back();
				_builder->set_insert_point(check_rhs);
			}
			else
				logic_exp->exps()[i]->accept(this);
		}
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTEqual* equal)
{
	auto l = equal->l()->accept(this);
	auto r = equal->r()->accept(this);
	bool flt = equal->getExpressionType() == FLOAT;
	if (equal->op_equal())
	{
		Value* cond;
		if (flt) cond = _builder->create_fcmp_eq(l, r);
		else cond = _builder->create_icmp_eq(l, r);
		_builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
	}
	else
	{
		Value* cond;
		if (flt) cond = _builder->create_fcmp_ne(l, r);
		else cond = _builder->create_icmp_ne(l, r);
		_builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTRelation* relation)
{
	auto l = relation->l()->accept(this);
	auto r = relation->r()->accept(this);
	bool flt = relation->getExpressionType() == FLOAT;
	Value* cond;
	switch (relation->op())
	{
		case RelationOP::LT:
			if (flt) cond = _builder->create_fcmp_lt(l, r);
			else cond = _builder->create_icmp_lt(l, r);
			return _builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
		case RelationOP::GT:
			if (flt) cond = _builder->create_fcmp_gt(l, r);
			else cond = _builder->create_icmp_gt(l, r);
			return _builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
		case RelationOP::LE:
			if (flt) cond = _builder->create_fcmp_le(l, r);
			else cond = _builder->create_icmp_le(l, r);
			return _builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
		case RelationOP::GE:
			if (flt) cond = _builder->create_fcmp_ge(l, r);
			else cond = _builder->create_icmp_ge(l, r);
			return _builder->create_cond_br(cond, _true_targets.back(), _false_targets.back());
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTCall* call)
{
	auto ast_func = call->function();
	auto func = _func_scope.find(ast_func->id());
	if (ast_func->isLibFunc())
	{
		auto& ast_args = call->parameters();
		vector<Value*> args;
		for (auto& ast_arg : ast_args)
		{
			auto arg = ast_arg->accept(this);
			if (arg->get_type()->isBasicType())
				args.emplace_back(arg);
			else
			{
				auto dims = arg->get_type()->toPointerType()->typeContained();
				if (dims->isBasicType())
					args.emplace_back(arg);
				else // 库函数的任意数组维度都解码
				{
					auto ar = dims->toArrayType();
					int ds = static_cast<int>(ar->dimensions().size());
					vector<Value*> d;
					d.reserve(1 + ds);
					auto zero = _builder->create_constant(0);
					for (int i = -1; i < ds; i++) d.emplace_back(zero);
					args.emplace_back(_builder->create_gep(arg, d));
				}
			}
		}
		return _builder->create_call(func, args);
	}
	auto& ast_args = call->parameters();
	vector<Value*> args;
	int idx = 0;
	for (auto& ast_arg : ast_args)
	{
		auto arg = ast_arg->accept(this);
		if (arg->get_type()->isBasicType())
			args.emplace_back(arg);
		else
		{
			auto dims = arg->get_type()->toPointerType()->typeContained();
			if (dims->isBasicType() || arg->get_type() == func->get_type()->toFuncType()->argumentType(idx))
				args.emplace_back(arg);
			else
			{
				auto zero = _builder->create_constant(0);
				vector<Value*> d{zero, zero};
				args.emplace_back(_builder->create_gep(arg, d));
			}
		}
		idx++;
	}
	return _builder->create_call(func, args);
}

Value* AST2IRVisitor::visit(ASTNeg* neg)
{
	auto v = neg->hold()->accept(this);
	if (neg->hold()->getExpressionType() == FLOAT) return _builder->create_fsub(_builder->create_constant(0.0f), v);
	return _builder->create_isub(_builder->create_constant(0), v);
}

Value* AST2IRVisitor::visit(ASTNot* not_node)
{
	auto bbt = _false_targets.back();
	auto bbf = _true_targets.back();
	_true_targets.emplace_back(bbt);
	_false_targets.emplace_back(bbf);
	not_node->hold()->accept(this);
	_true_targets.pop_back();
	_false_targets.pop_back();
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTBlock* block)
{
	return visitStmts(block->stmts());
}

Value* AST2IRVisitor::visit(ASTAssign* assign)
{
	auto l = assign->assign_to()->accept(this);
	auto r = assign->assign_value()->accept(this);
	_builder->create_store(r, l);
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTIf* if_node)
{
	Value* ret = nullptr;
	if (if_node->else_stmt().empty())
	{
		auto bbt = BasicBlock::create(_module, "", _functionBelong);
		auto bbf = BasicBlock::create(_module, "", _functionBelong);
		_true_targets.emplace_back(bbt);
		_false_targets.emplace_back(bbf);
		if_node->cond()->accept(this);
		_true_targets.pop_back();
		_false_targets.pop_back();
		_builder->set_insert_point(bbt);
		auto res = visitStmts(if_node->if_stmt());
		if (res == nullptr)
			_builder->create_br(bbf);
		else ret = res;
		_builder->set_insert_point(bbf);
	}
	else
	{
		auto bbt = BasicBlock::create(_module, "", _functionBelong);
		auto bbf = BasicBlock::create(_module, "", _functionBelong);
		auto bb = BasicBlock::create(_module, "", _functionBelong);
		_true_targets.emplace_back(bbt);
		_false_targets.emplace_back(bbf);
		if_node->cond()->accept(this);
		_true_targets.pop_back();
		_false_targets.pop_back();
		_builder->set_insert_point(bbt);
		auto res = visitStmts(if_node->if_stmt());
		if (res == nullptr)
			_builder->create_br(bb);
		else ret = res;
		_builder->set_insert_point(bbf);
		res = visitStmts(if_node->else_stmt());
		if (res == nullptr)
		{
			_builder->create_br(bb);
			if (ret != nullptr) ret = nullptr;
		}
		_builder->set_insert_point(bb);
	}
	return ret;
}

Value* AST2IRVisitor::visit(ASTWhile* while_node)
{
	if (while_node->stmt().empty())
	{
		auto bb_cond = BasicBlock::create(_module, "", _functionBelong);
		auto bb_next = BasicBlock::create(_module, "", _functionBelong);
		_builder->create_br(bb_cond);
		_builder->set_insert_point(bb_cond);
		_true_targets.emplace_back(bb_cond);
		_false_targets.emplace_back(bb_next);
		while_node->cond()->accept(this);
		_true_targets.pop_back();
		_false_targets.pop_back();
		_builder->set_insert_point(bb_next);
	}
	else
	{
		auto bb_cond = BasicBlock::create(_module, "", _functionBelong);
		auto bb_next = BasicBlock::create(_module, "", _functionBelong);
		auto bb_while = BasicBlock::create(_module, "", _functionBelong);
		_builder->create_br(bb_cond);
		_builder->set_insert_point(bb_cond);
		_true_targets.emplace_back(bb_while);
		_false_targets.emplace_back(bb_next);
		while_node->cond()->accept(this);
		_true_targets.pop_back();
		_false_targets.pop_back();
		_builder->set_insert_point(bb_while);
		_while_nexts.emplace_back(bb_while);
		auto res = visitStmts(while_node->stmt());
		_while_nexts.pop_back();
		if (res == nullptr)
			_builder->create_br(bb_cond);
		_builder->set_insert_point(bb_next);
		return res;
	}
	return nullptr;
}

Value* AST2IRVisitor::visit(ASTBreak* break_node)
{
	return _while_nexts.back();
}

Value* AST2IRVisitor::visit(ASTReturn* return_node)
{
	if (_functionBelong->get_return_type() == Types::VOID)
		return _builder->create_void_ret();
	return _builder->create_ret(return_node->return_value()->accept(this));
}

Value* AST2IRVisitor::visitStmts(const std::vector<ASTStmt*>& vec)
{
	Value* ret = nullptr;
	for (auto& stmt : vec)
	{
		auto block2 = dynamic_cast<ASTBlock*>(stmt);
		if (block2 != nullptr)
		{
			_var_scope.enter();
			ret = block2->accept(this);
			_var_scope.exit();
		}
		else
		{
			auto exp = dynamic_cast<ASTExpression*>(stmt);
			if (exp != nullptr && exp->getExpressionType() == BOOL)
			{
				auto bb = BasicBlock::create(_module, "", _functionBelong);
				_true_targets.emplace_back(bb);
				_false_targets.emplace_back(bb);
				ret = stmt->accept(this);
				_true_targets.pop_back();
				_false_targets.pop_back();
				_builder->set_insert_point(bb);
			}
			else
				ret = stmt->accept(this);
		}
		if (_builder->get_insert_block()->is_terminated())
			break;
	}
	return ret;
}

Value* AST2IRVisitor::visitStmts(const std::vector<ASTNode*>& vec)
{
	Value* ret = nullptr;
	for (auto& stmt : vec)
	{
		auto block2 = dynamic_cast<ASTBlock*>(stmt);
		if (block2 != nullptr)
		{
			_var_scope.enter();
			ret = block2->accept(this);
			_var_scope.exit();
		}
		else
		{
			auto exp = dynamic_cast<ASTExpression*>(stmt);
			if (exp != nullptr && exp->getExpressionType() == BOOL)
			{
				auto bb = BasicBlock::create(_module, "", _functionBelong);
				_true_targets.emplace_back(bb);
				_false_targets.emplace_back(bb);
				ret = stmt->accept(this);
				_true_targets.pop_back();
				_false_targets.pop_back();
				_builder->set_insert_point(bb);
			}
			else
				ret = stmt->accept(this);
		}
		if (_builder->get_insert_block()->is_terminated())
			break;
	}
	return ret;
}

std::string AST2IRVisitor::createPrivateGlobalVarID()
{
	return "constinit." + std::to_string(_globalInitIdx++);
}

std::vector<Value*> AST2IRVisitor::index(const std::vector<int>& shape, int idx, int paddings) const
{
	std::vector<Value*> ret;
	int ed = static_cast<int>(shape.size());
	ret.reserve(ed + paddings);
	for (int i = 0; i < paddings; i++) ret.emplace_back(_builder->create_constant(0));
	for (int i = 0; i < ed; i++)
	{
		int x = idx / shape[i];
		idx -= x * shape[i];
		ret.emplace_back(_builder->create_constant(x));
	}
	return ret;
}
