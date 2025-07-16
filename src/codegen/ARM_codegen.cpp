#include "ARM_codegen.hpp"
#include "Constant.hpp"
#include <cstdint>
#include <ios>
#include <string>
#include <sstream>
#include <vector>


const std::map<uint32_t, const std::string> iWidthPrefix = {{1, "W"}, {8, "W"}, {32, "W"}, {64, "X"}};
const std::map<uint32_t, const std::string> iWidthSuffix = {{1, "B"}, {8, "B"}, {16, "H"}, {32, "S"}, {64, "D"}};
const std::map<uint32_t, const std::string> fvWidthPrefix = {{8, "B"}, {16, "H"}, {32, "S"}, {64, "D"}, {128, "Q"}};
const std::map<uint32_t, const std::string> ConstInitLen = {{1, ".byte"}, {4, ".word"}, {8, ".quad"}};


/*
 * TODO: 
 * 1. 寄存器分配
 * 2. 指令选择与合并，可能需要单独使用若干个peephole optimization pass
 */

void ARMCodeGen::run()
{
	bool gen_mclr = false, gen_mcpy = false;

	// 确保每个函数中基本块的名字都被设置好
	m->set_print_name();

	if (!m->get_global_variable().empty())
	{
		append_inst(".section", {".data"}, ASMInstruction::Attribute);

		for (auto global : m->get_global_variable())
		{
			append_inst(".globl", {global->get_name()}, ASMInstruction::Attribute);
			append_inst(".type", {global->get_name(), "@object"}, ASMInstruction::Attribute);
			auto var_ty = dynamic_cast<PointerType*>(global->get_type())->typeContained();
			if (var_ty->isBasicValueType())
			{
				append_inst(".align", {"2"}, ASMInstruction::Attribute);
				append_inst(global->get_name(), ASMInstruction::Label);
				auto size = NUMBYTES(var_ty->sizeInBitsInArm64());
				if (var_ty == Types::FLOAT)
				{
					std::stringstream ss(global->printInitValue());
					uint64_t val;
					ss >> std::hex >> val;
					auto f = (float)(*(double*)&val);
					ss.str("");
					ss.clear();
					ss << "0x" << std::hex << *(int*)&f;
					append_inst(
						ConstInitLen.at(size).c_str(),
						{ss.str()},
						ASMInstruction::Attribute);
				}
				else
				{
					append_inst(
						ConstInitLen.at(size).c_str(),
						{global->printInitValue()},
						ASMInstruction::Attribute);
				}
			}
			else
			{
				append_inst(".align", {"7"}, ASMInstruction::Attribute);
				append_inst(global->get_name(), ASMInstruction::Label);
				auto arr_init_val = global->get_init();
				auto n_segs = arr_init_val->segmentCount();
				for (int i = 0; i < n_segs; ++i)
				{
					auto s = arr_init_val->segment(i);
					if (s.second == 0)
					{
						auto v = s.first;
						for (auto& e : *v)
						{
							if (e.isIntConstant())
								append_inst(ConstInitLen.at(NUMBYTES(e.getType()->sizeInBitsInArm64())).c_str(),
								            {std::to_string(e.getIntConstant())}, ASMInstruction::Attribute);
							else if (e.isBoolConstant()) // Bool的长度视同Int
								append_inst(ConstInitLen.at(NUMBYTES(Types::INT->sizeInBitsInArm64())).c_str(),
								            {std::to_string(e.getBoolConstant())}, ASMInstruction::Attribute);
							else if (e.isFloatConstant())
							{
								double dv = e.getFloatConstant();
								float v = dv;
								std::stringstream ss;
								ss << std::hex << v;
								append_inst(ConstInitLen.at(NUMBYTES(Types::FLOAT->sizeInBitsInArm64())).c_str(),
								            {ss.str()}, ASMInstruction::Attribute);
							}
							else
								assert(false);
						}
					}
					else
					{
						auto z = s.second << 2;
						if (z > 0)
							append_inst(".skip", {std::to_string(z)}, ASMInstruction::Attribute);
					}
				}
			}
			append_inst(".size", {global->get_name(), ".-" + global->get_name()}, ASMInstruction::Attribute);
		}
	}

	output.emplace_back(".text", ASMInstruction::Attribute);
	for (auto func : m->get_functions())
	{
		if (!func->is_declaration())
		{
			context.clear();
			context.func = func;

			// 函数信息
			append_inst(".globl", {func->get_name()}, ASMInstruction::Attribute);
			append_inst(".type", {func->get_name(), "@function"},
			            ASMInstruction::Attribute);
			append_inst(func->get_name(), ASMInstruction::Label);

			// 分配函数栈帧
			allocate();
			// 生成 prologue
			gen_prologue();

			for (auto bb : func->get_basic_blocks())
			{
				context.bb = bb;
				append_inst(label_name(context.bb), ASMInstruction::Label);
				for (auto instr : bb->get_instructions())
				{
					// For debug
					append_inst(instr->print(), ASMInstruction::Comment);
					context.inst = instr; // 更新 context
					switch (instr->get_instr_type())
					{
						case Instruction::global_fix:
							gen_cvt_global_type();
							break;
						case Instruction::nump2charp:
							gen_nump2charp();
							break;
						case Instruction::memclear_:
							gen_mclr = true;
							gen_memop();
							break;
						case Instruction::memcpy_:
							gen_mcpy = true;
							gen_memop();
							break;
						case Instruction::ret:
							gen_ret();
							break;
						case Instruction::br:
							gen_br();
							break;
						case Instruction::add:
						case Instruction::sub:
						case Instruction::mul:
						case Instruction::sdiv:
						case Instruction::srem:
							gen_binary();
							break;
						case Instruction::fadd:
						case Instruction::fsub:
						case Instruction::fmul:
						case Instruction::fdiv:
							gen_fbinary();
							break;
						case Instruction::alloca_:
							// 已经为 alloca 的内容分配空间，还需保存 alloca 指令的定值
							gen_alloca();
							break;
						case Instruction::load:
							gen_load();
							break;
						case Instruction::store:
							gen_store();
							break;
						case Instruction::ge:
						case Instruction::gt:
						case Instruction::le:
						case Instruction::lt:
						case Instruction::eq:
						case Instruction::ne:
							gen_icmp();
							break;
						case Instruction::fge:
						case Instruction::fgt:
						case Instruction::fle:
						case Instruction::flt:
						case Instruction::feq:
						case Instruction::fne:
							gen_fcmp();
							break;
						case Instruction::phi: // copy_stmt会处理phi指令在分支中的定值
							break;
						case Instruction::call:
							gen_call();
							break;
						case Instruction::getelementptr:
							gen_gep();
							break;
						case Instruction::zext:
							gen_zext();
							break;
						case Instruction::fptosi:
							gen_fptosi();
							break;
						case Instruction::sitofp:
							gen_sitofp();
							break;
					}
				}
			}
			gen_epilogue();
		}
	}
	if (gen_mcpy)
		gen_memcpy_procedure();
	if (gen_mclr)
		gen_memclr_procedure();
}

std::string ARMCodeGen::print() const
{
	std::string result;
	for (const auto& inst : output)
		result += inst.format();
	return result;
}

void ARMCodeGen::Rload_to_GPreg(Value* val, unsigned Rid)
{
	auto width = val->get_type()->sizeInBitsInArm64();
	if (auto constant = dynamic_cast<ConstantValue*>(val))
	{
		if (constant->isIntConstant())
		{
			auto val = constant->getIntConstant();
			if (IS_IMM12(val))
				append_inst("MOV", {iWidthPrefix.at(width) + std::to_string(Rid), "#" + std::to_string(val)});
			else
				Rload_imm_to_GPreg(val, Rid);
		}
		else if (constant->isBoolConstant())
		{
			auto val = constant->getBoolConstant();
			append_inst("MOV", {iWidthPrefix.at(width) + std::to_string(Rid), "#" + std::to_string(val)});
		}
		else if (constant->isFloatConstant())
		{
			double dval = constant->getFloatConstant();
			float val = dval;
			int ival = *(int*)&val;
			Rload_imm_to_GPreg(ival, Rid);
		}
	}
	else if (auto gvar = dynamic_cast<GlobalVariable*>(val))
	{
		append_inst("LDR", {iWidthPrefix.at(width) + std::to_string(Rid), "=" + gvar->get_name()});
	}
	else if (!val->get_type()->isFunctionType())
	{
		Rload_stack_to_GPreg(val, Rid);
	}
}

void ARMCodeGen::Rstore_from_GPreg(Value* val, unsigned Rid)
{
	auto offset = context.offset_map.at(val);
	auto width = val->get_type()->sizeInBitsInArm64();
	MemAccess m = {"X29", "#" + std::to_string(offset)};
	append_inst("STR", iWidthPrefix.at(width) + std::to_string(Rid), m);
}

void ARMCodeGen::Rload_stack_to_GPreg(Value* val, unsigned Rid)
{
	int stack_offset = context.offset_map.at(val);
	auto width = val->get_type()->sizeInBitsInArm64();
	MemAccess m = {"X29", "#" + std::to_string(stack_offset)};
	append_inst("LDR", iWidthPrefix.at(width) + std::to_string(Rid), m);
}

void ARMCodeGen::Rload_imm_to_GPreg(int64_t imm, unsigned Rid, unsigned bits)
{
	if ((bits == 0 && !IS_IMM32(imm)) || bits == 64)
	{
		Rload_64_to_GPreg(imm, Rid);
	}
	else if ((bits == 0 && IS_IMM32(imm)) || bits == 32)
	{
		Rload_32_to_GPreg(imm, Rid);
	}
	else
		assert(false && "Invalid int width!\n");
}

void ARMCodeGen::Rload_32_to_GPreg(int32_t imm, unsigned Rid)
{
	uint32_t value = static_cast<uint32_t>(imm);
	uint32_t part0 = value & 0xFFFF;
	uint32_t part1 = (value >> 16) & 0xFFFF;
	if (part1 == 0)
	{
		if (part0 != 0)
			append_inst("MOVZ", {"W" + std::to_string(Rid), "#" + std::to_string(part0), "LSL #0"});
		else
			append_inst("MOV", {"W" + std::to_string(Rid), "WZR"});
	}
	else
	{
		append_inst("MOVZ", {"W" + std::to_string(Rid), "#" + std::to_string(part0), "LSL #0"});
		append_inst("MOVK", {"W" + std::to_string(Rid), "#" + std::to_string(part1), "LSL #16"});
	}
}

void ARMCodeGen::Rload_64_to_GPreg(int64_t imm, unsigned Rid)
{
	uint64_t value = static_cast<uint64_t>(imm);
	uint64_t parts[4] = {value & 0xFFFF, (value >> 16) & 0xFFFF, (value >> 32) & 0xFFFF, (value >> 48) & 0xFFFF};

	bool first = true;
	for (int i = 0; i < 4; ++i)
	{
		if (parts[i])
		{
			if (first)
			{
				append_inst("MOVZ", {
					            "X" + std::to_string(Rid), "#" + std::to_string(parts[i]),
					            "LSL #" + std::to_string(i << 4)
				            });
				first = false;
			}
			else
			{
				append_inst("MOVK", {
					            "X" + std::to_string(Rid), "#" + std::to_string(parts[i]),
					            "LSL #" + std::to_string(i << 4)
				            });
			}
		}
	}

	if (first)
		append_inst("MOV", {"X" + std::to_string(Rid), "XZR"});
}

void ARMCodeGen::Rload_to_FPreg(Value* val, unsigned FRid)
{
	auto width = val->get_type()->sizeInBitsInArm64();
	if (auto constant = dynamic_cast<ConstantValue*>(val))
	{
		double dv = constant->getFloatConstant();
		float fv = dv;
		Rload_imm_to_FPreg(fv, FRid);
	}
	else if (auto gvar = dynamic_cast<GlobalVariable*>(val))
	{
		append_inst("LDR", {fvWidthPrefix.at(width) + std::to_string(FRid), gvar->get_name()});
	}
	else if (!val->get_type()->isFunctionType())
	{
		Rload_stack_to_FPreg(val, FRid);
	}
}

void ARMCodeGen::Rstore_from_FPreg(Value* val, unsigned FRid)
{
	auto offset = context.offset_map.at(val);
	auto width = Types::FLOAT->sizeInBitsInArm64();
	MemAccess m = {"X29", "#" + std::to_string(offset)};
	append_inst("STR", fvWidthPrefix.at(width) + std::to_string(FRid), m);
}

void ARMCodeGen::Rload_imm_to_FPreg(float imm, unsigned FRid)
{
	auto t = get_avail_tmp_reg();
	auto i = *(int*)&imm;
	auto width = Types::FLOAT->sizeInBitsInArm64();
	Rload_imm_to_GPreg(i, t);
	append_inst("FMOV", {
		            fvWidthPrefix.at(width) + std::to_string(FRid),
		            iWidthPrefix.at(width) + std::to_string(t)
	            });
}

void ARMCodeGen::Rload_stack_to_FPreg(Value* val, unsigned FRid)
{
	int stack_offset = context.offset_map.at(val);
	auto width = Types::FLOAT->sizeInBitsInArm64();
	MemAccess m = {"X29", "#" + std::to_string(stack_offset)};
	append_inst("LDR", fvWidthPrefix.at(width) + std::to_string(FRid), m);
}

/* ================================ 代码翻译 ================================ */
void ARMCodeGen::allocate()
{
	int offset = PROLOGUE_OFFSET_BASE;
	int iarg_count = 0, farg_count = 0;
	int arg_offset = 0;

	// 为参数分配栈空间
	for (auto& arg : context.func->get_args())
	{
		if (arg.get_type() == Types::FLOAT && farg_count < 8)
		{
			// 从寄存器读参数到栈上
			offset += NUMBYTES(arg.get_type()->sizeInBitsInArm64());
			context.offset_map[&arg] = -offset;
			++farg_count;
		}
		else if (iarg_count < 8)
		{
			// 从寄存器读参数到栈上
			offset += arg.get_type() == Types::BOOL
				          ? NUMBYTES(Types::INT->sizeInBitsInArm64())
				          : NUMBYTES(arg.get_type()->sizeInBitsInArm64());
			context.offset_map[&arg] = -offset;
			++iarg_count;
		}
		else
		{
			// 从 caller 栈帧读参数
			context.offset_map[&arg] = PROLOGUE_OFFSET_BASE + arg_offset;
			arg_offset += arg.get_type() == Types::BOOL
				              ? NUMBYTES(Types::INT->sizeInBitsInArm64())
				              : NUMBYTES(arg.get_type()->sizeInBitsInArm64());
		}
	}

	// 局部变量
	for (auto bb : context.func->get_basic_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (inst->is_alloca())
			{
				auto alloca_inst = static_cast<AllocaInst*>(inst);
				int size = NUMBYTES(alloca_inst->get_alloca_type()->sizeInBitsInArm64());
				size = ALIGN(size, 4);
				offset += size + 8;
				context.offset_map[alloca_inst] = -offset;
			}
		}
	}

	for (auto bb : context.func->get_basic_blocks())
	{
		for (auto inst : bb->get_instructions())
		{
			if (!context.offset_map.count(inst) && !inst->is_void())
			{
				offset += 8;
				context.offset_map[inst] = -offset;
			}
		}
	}

	// 对所有成为Phi来源的定值，保存其在前驱块出口时的值
	for (auto bb : context.func->get_basic_blocks())
	{
		for (auto succ : bb->get_succ_basic_blocks())
		{
			for (auto inst : succ->get_instructions())
			{
				if (inst->is_phi())
				{
					for (unsigned i = 1; i < inst->get_num_operand(); i += 2)
					{
						if (inst->get_operand(i) == bb)
						{
							auto lvalue = inst->get_operand(i - 1);
							offset += 8;
							context.phi_offset_map[lvalue] = -offset;
						}
					}
				}
				else break;
			}
		}
	}
	context.frame_size = ALIGN(offset, PROLOGUE_ALIGN);
}

void ARMCodeGen::copy_stmt(BasicBlock* succ)
{
	for (auto inst : succ->get_instructions())
	{
		if (inst->is_phi())
		{
			// 拷贝旧值到临时位置，防止被其他phi操作破坏
			for (unsigned i = 1; i < inst->get_operands().size(); i += 2)
			{
				if (inst->get_operand(i) == context.bb)
				{
					auto lvalue = inst->get_operand(i - 1);
					auto offset = context.phi_offset_map.at(lvalue);
					auto width = lvalue->get_type()->sizeInBitsInArm64();
					MemAccess m = {"X29", "#" + std::to_string(offset)};

					auto tmp_reg_id = 9;
					Rload_to_GPreg(lvalue, tmp_reg_id);
					append_inst("STR", iWidthPrefix.at(width)
					                   + std::to_string(tmp_reg_id), m);

					break;
				}
			}
		}
		else break;
	}
	for (auto inst : succ->get_instructions())
	{
		if (inst->is_phi())
		{
			// 用旧值定值
			for (unsigned i = 1; i < inst->get_operands().size(); i += 2)
			{
				if (inst->get_operand(i) == context.bb)
				{
					auto lvalue = inst->get_operand(i - 1);
					auto offset = context.phi_offset_map.at(lvalue);
					auto width = lvalue->get_type()->sizeInBitsInArm64();
					MemAccess m = {"X29", "#" + std::to_string(offset)};

					auto tmp_reg_id = 9;
					append_inst("LDR", iWidthPrefix.at(width)
					                   + std::to_string(tmp_reg_id), m);
					Rstore_from_GPreg(inst, tmp_reg_id);

					break;
				}
			}
		}
		else break;
	}
}

void ARMCodeGen::gen_ret()
{
	auto ret_inst = static_cast<ReturnInst*>(context.inst);
	if (ret_inst->get_num_operand() > 0)
	{
		auto ret_val = ret_inst->get_operand(0);
		if (ret_val->get_type() == Types::FLOAT)
			Rload_to_FPreg(ret_val, 0);
		else if (ret_val->get_type() == Types::INT)
			Rload_to_GPreg(ret_val, 0);
		else
			append_inst("MOV", {"X0", "#0"});
	}
	auto exit = context.func->get_name() + "_exit";
	append_inst("B", {exit});
}

void ARMCodeGen::gen_br()
{
	auto br_inst = static_cast<BranchInst*>(context.inst);

	if (br_inst->is_cond_br())
	{
		auto cond_reg_id = 9;
		auto cond = br_inst->get_operand(0);
		auto true_bb = dynamic_cast<BasicBlock*>(br_inst->get_operand(1));
		auto false_bb = dynamic_cast<BasicBlock*>(br_inst->get_operand(2));
		// 分别为真假出口对应的phi节点定值
		Rload_to_GPreg(cond, cond_reg_id);
		auto false_exit_name = "." + context.func->get_name() + "_" + context.bb->get_name()
		                       + "_" + false_bb->get_name();
		append_inst("CMP", {
			            iWidthPrefix.at(cond->get_type()->sizeInBitsInArm64())
			            + std::to_string(cond_reg_id),
			            "#0"
		            });
		append_inst("B.EQ", {false_exit_name});
		copy_stmt(true_bb);
		append_inst("B", {label_name(true_bb)});
		append_inst(false_exit_name, ASMInstruction::Label);
		copy_stmt(false_bb);
		append_inst("B", {label_name(false_bb)});
	}
	else
	{
		auto bb = dynamic_cast<BasicBlock*>(br_inst->get_operand(0));
		copy_stmt(bb);
		append_inst("B", {label_name(bb)});
	}
}

void ARMCodeGen::gen_binary()
{
	auto bin_inst = static_cast<IBinaryInst*>(context.inst);
	auto op1 = bin_inst->get_operand(0);
	auto op2 = bin_inst->get_operand(1);
	auto p = iWidthPrefix.at(op1->get_type()->sizeInBitsInArm64());

	auto op1_reg_id = 9;
	auto op2_reg_id = 10;
	auto res_reg_id = 11;

	Rload_to_GPreg(op1, op1_reg_id);
	Rload_to_GPreg(op2, op2_reg_id);

	switch (bin_inst->get_instr_type())
	{
		case Instruction::add:
			append_inst("ADD", {
				            p + std::to_string(res_reg_id),
				            p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)
			            });
			break;
		case Instruction::sub:
			append_inst("SUB", {
				            p + std::to_string(res_reg_id),
				            p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)
			            });
			break;
		case Instruction::mul:
			append_inst("MUL", {
				            p + std::to_string(res_reg_id),
				            p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)
			            });
			break;
		case Instruction::sdiv:
			append_inst("SDIV", {
				            p + std::to_string(res_reg_id),
				            p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)
			            });
			break;
		case Instruction::srem:
			append_inst("SDIV", {
				            p + std::to_string(res_reg_id),
				            p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)
			            });
			append_inst("MSUB", {
				            p + std::to_string(res_reg_id), p + std::to_string(op2_reg_id),
				            p + std::to_string(res_reg_id), p + std::to_string(op1_reg_id)
			            });
			break;
		default: assert(false && "Unknown binary operator");
	}
	Rstore_from_GPreg(bin_inst, res_reg_id);
}

void ARMCodeGen::gen_fbinary()
{
	auto bin_inst = static_cast<FBinaryInst*>(context.inst);
	auto op1 = bin_inst->get_operand(0);
	auto op2 = bin_inst->get_operand(1);
	auto p = fvWidthPrefix.at(op1->get_type()->sizeInBitsInArm64());

	auto op1_freg_id = 0;
	auto op2_freg_id = 1;
	auto res_freg_id = 2;

	Rload_to_FPreg(op1, op1_freg_id);
	Rload_to_FPreg(op2, op2_freg_id);

	switch (bin_inst->get_instr_type())
	{
		case Instruction::fadd:
			append_inst("FADD", {
				            p + std::to_string(res_freg_id),
				            p + std::to_string(op1_freg_id), p + std::to_string(op2_freg_id)
			            });
			break;
		case Instruction::fsub:
			append_inst("FSUB", {
				            p + std::to_string(res_freg_id),
				            p + std::to_string(op1_freg_id), p + std::to_string(op2_freg_id)
			            });
			break;
		case Instruction::fmul:
			append_inst("FMUL", {
				            p + std::to_string(res_freg_id),
				            p + std::to_string(op1_freg_id), p + std::to_string(op2_freg_id)
			            });
			break;
		case Instruction::fdiv:
			append_inst("FDIV", {
				            p + std::to_string(res_freg_id),
				            p + std::to_string(op1_freg_id), p + std::to_string(op2_freg_id)
			            });
			break;
		default: assert(false && "Unknown fbinary operator");
	}
	Rstore_from_FPreg(bin_inst, res_freg_id);
}

void ARMCodeGen::gen_alloca()
{
	auto alloca = static_cast<AllocaInst*>(context.inst);
	auto size = NUMBYTES(alloca->get_alloca_type()->sizeInBitsInArm64());
	auto offset = context.offset_map.at(alloca) + 8;

	auto tmp_reg_id = 9;

	if (IS_IMM12(offset))
	{
		append_inst("ADD", {"X" + std::to_string(tmp_reg_id), "X29", "#" + std::to_string(offset)});
	}
	else
	{
		Rload_imm_to_GPreg(offset, tmp_reg_id, 64);
		append_inst("ADD", {"X" + std::to_string(tmp_reg_id), "X29", "X" + std::to_string(tmp_reg_id)});
	}
	Rstore_from_GPreg(alloca, tmp_reg_id);
}

void ARMCodeGen::gen_load()
{
	auto load_inst = static_cast<LoadInst*>(context.inst);
	auto ptr = load_inst->get_operand(0);

	auto ld_reg_id = 9;

	Rload_to_GPreg(ptr, ld_reg_id);
	if (load_inst->get_type() == Types::FLOAT)
	{
		auto ld_freg_id = 9;
		auto fp = fvWidthPrefix.at(Types::FLOAT->sizeInBitsInArm64());
		auto p = iWidthPrefix.at(load_inst->get_type()->sizeInBitsInArm64());
		append_inst("LDR", fp + std::to_string(ld_freg_id), MemAccess{"X" + std::to_string(ld_reg_id)});
		Rstore_from_FPreg(load_inst, ld_freg_id);
	}
	else
	{
		auto p = iWidthPrefix.at(load_inst->get_type()->sizeInBitsInArm64());
		append_inst("LDR", p + std::to_string(ld_reg_id), MemAccess{"X" + std::to_string(ld_reg_id)});
		Rstore_from_GPreg(load_inst, ld_reg_id);
	}
}

void ARMCodeGen::gen_store()
{
	auto store_inst = static_cast<StoreInst*>(context.inst);
	auto val = store_inst->get_operand(0);
	auto ptr = store_inst->get_operand(1);

	auto st_adr_reg_id = 10;

	Rload_to_GPreg(ptr, st_adr_reg_id);
	if (val->get_type() == Types::FLOAT)
	{
		auto st_freg_id = 9;
		Rload_to_FPreg(val, st_freg_id);
		auto p = fvWidthPrefix.at(val->get_type()->sizeInBitsInArm64());
		append_inst("STR", p + std::to_string(st_freg_id), MemAccess{"X" + std::to_string(st_adr_reg_id)});
	}
	else
	{
		auto st_reg_id = 9;
		Rload_to_GPreg(val, st_reg_id);
		auto p = iWidthPrefix.at(val->get_type()->sizeInBitsInArm64());
		append_inst("STR", p + std::to_string(st_reg_id), MemAccess{"X" + std::to_string(st_adr_reg_id)});
	}
}

void ARMCodeGen::gen_icmp()
{
	auto icmp_inst = static_cast<ICmpInst*>(context.inst);
	auto op1 = icmp_inst->get_operand(0);
	auto op2 = icmp_inst->get_operand(1);
	auto p = iWidthPrefix.at(op1->get_type()->sizeInBitsInArm64());

	auto op1_reg_id = 9;
	auto op2_reg_id = 10;
	auto cond_reg_id = 11;

	Rload_to_GPreg(op1, op1_reg_id);
	Rload_to_GPreg(op2, op2_reg_id);
	append_inst("CMP", {p + std::to_string(op1_reg_id), p + std::to_string(op2_reg_id)});

	std::string cond;
	switch (icmp_inst->get_instr_type())
	{
		case Instruction::eq: cond = "EQ";
			break;
		case Instruction::ne: cond = "NE";
			break;
		case Instruction::gt: cond = "GT";
			break;
		case Instruction::ge: cond = "GE";
			break;
		case Instruction::lt: cond = "LT";
			break;
		case Instruction::le: cond = "LE";
			break;
		default: assert(false && "Unknown icmp operator");
	}

	append_inst("CSET", {p + std::to_string(cond_reg_id), cond});
	Rstore_from_GPreg(icmp_inst, cond_reg_id);
}

void ARMCodeGen::gen_fcmp()
{
	auto fcmp_inst = static_cast<FCmpInst*>(context.inst);
	auto op1 = fcmp_inst->get_operand(0);
	auto op2 = fcmp_inst->get_operand(1);
	auto p = fvWidthPrefix.at(op1->get_type()->sizeInBitsInArm64());

	auto op1_freg_id = 9;
	auto op2_freg_id = 10;
	auto cond_reg_id = 9;

	Rload_to_FPreg(op1, op1_freg_id);
	Rload_to_FPreg(op2, op2_freg_id);
	append_inst("FCMP", {p + std::to_string(op1_freg_id), p + std::to_string(op2_freg_id)});

	std::string cond;
	switch (fcmp_inst->get_instr_type())
	{
		case Instruction::feq: cond = "EQ";
			break;
		case Instruction::fne: cond = "NE";
			break;
		case Instruction::fgt: cond = "GT";
			break;
		case Instruction::fge: cond = "GE";
			break;
		case Instruction::flt: cond = "MI";
			break;
		case Instruction::fle: cond = "LS";
			break;
		default: assert(false && "Unknown fcmp operator");
	}

	append_inst("CSET", {"X" + std::to_string(cond_reg_id), cond});
	Rstore_from_GPreg(fcmp_inst, cond_reg_id);
}

void ARMCodeGen::gen_zext()
{
	auto zext_inst = static_cast<ZextInst*>(context.inst);
	auto src = zext_inst->get_operand(0);

	auto ext_reg_id = 9;

	Rload_to_GPreg(src, ext_reg_id);
	Rstore_from_GPreg(zext_inst, ext_reg_id);
}

void ARMCodeGen::gen_gep()
{
	auto gep = static_cast<GetElementPtrInst*>(context.inst);
	auto n_opnd = gep->get_num_operand();

	Value* base_ptr = gep->get_operand(0);
	auto base_reg_id = 9;
	Rload_to_GPreg(base_ptr, base_reg_id);
	if (n_opnd == 1)
	{
		Rstore_from_GPreg(gep, base_reg_id);
		return;
	}

	auto offset_reg_id = 10;
	auto idx_reg_id = 11;
	auto tmp_reg_id = 12;

	append_inst("MOV", {"X" + std::to_string(offset_reg_id), "#0"});
	std::vector<Value*> idxs{};
	Type* current_type = nullptr;
	for (unsigned i = 1; i < n_opnd; ++i)
	{
		Value* index_val = gep->get_operand(i);
		idxs.emplace_back(index_val);
		current_type = gep->get_element_type(base_ptr, idxs);
		if (auto const_int = dynamic_cast<ConstantValue*>(index_val))
		{
			int64_t offset = const_int->getIntConstant();
			uint64_t type_size = type_size = NUMBYTES(current_type->sizeInBitsInArm64());
			if (offset != 0)
			{
				int64_t total_offset = offset * type_size;
				if (total_offset != 0)
				{
					Rload_imm_to_GPreg(total_offset, tmp_reg_id);
					append_inst("ADD", {
						            "X" + std::to_string(offset_reg_id),
						            "X" + std::to_string(offset_reg_id), "X" + std::to_string(tmp_reg_id)
					            });
				}
			}
		}
		else
		{
			uint64_t type_size = NUMBYTES(current_type->sizeInBitsInArm64());
			if (type_size != 0)
			{
				Rload_to_GPreg(index_val, idx_reg_id);
				Rload_imm_to_GPreg(type_size, tmp_reg_id);
				append_inst("MUL", {
					            "X" + std::to_string(tmp_reg_id),
					            "X" + std::to_string(tmp_reg_id), "X" + std::to_string(idx_reg_id)
				            });
				append_inst("ADD", {
					            "X" + std::to_string(offset_reg_id),
					            "X" + std::to_string(tmp_reg_id), "X" + std::to_string(offset_reg_id)
				            });
			}
		}
	}
	append_inst("ADD", {
		            "X" + std::to_string(base_reg_id), "X" + std::to_string(base_reg_id),
		            "X" + std::to_string(offset_reg_id)
	            });
	Rstore_from_GPreg(gep, base_reg_id);
}

void ARMCodeGen::gen_sitofp()
{
	auto inst = static_cast<SiToFpInst*>(context.inst);

	auto reg_id = 9;
	auto freg_id = 9;

	Rload_to_GPreg(inst->get_operand(0), reg_id);
	append_inst("SCVTF", {"S" + std::to_string(freg_id), "W" + std::to_string(reg_id)});
	Rstore_from_FPreg(inst, freg_id);
}

void ARMCodeGen::gen_fptosi()
{
	auto inst = static_cast<FpToSiInst*>(context.inst);

	auto reg_id = 9;
	auto freg_id = 9;

	Rload_to_FPreg(inst->get_operand(0), freg_id);
	append_inst("FCVTZS", {"W" + std::to_string(reg_id), "S" + std::to_string(freg_id)});
	Rstore_from_GPreg(inst, reg_id);
}

void ARMCodeGen::gen_call()
{
	auto call_inst = static_cast<CallInst*>(context.inst);

	int Narg = 0, Nfarg = 0;

	std::vector<Value*> stack_params;
	unsigned total_param_size = 0;

	for (int i = 1; i < call_inst->get_num_operand(); ++i)
	{
		auto arg = call_inst->get_operand(i);
		if (arg->get_type() == Types::FLOAT && Nfarg < 8)
			Rload_to_FPreg(arg, Nfarg++);
		else if (Narg < 8)
			Rload_to_GPreg(arg, Narg++);
		else
		{
			stack_params.emplace_back(arg);
			total_param_size += arg->get_type() == Types::BOOL
				                    ? NUMBYTES(Types::INT->sizeInBitsInArm64())
				                    : NUMBYTES(arg->get_type()->sizeInBitsInArm64());
		}
	}

	auto total_param_size_a = ALIGN(total_param_size, 16);

	// 栈上传参，参数从后往前压栈
	auto tmp_reg_id = 9;
	if (total_param_size > 0)
	{
		if (IS_IMM12(total_param_size))
			append_inst("SUB", {"SP", "SP", std::to_string(total_param_size_a)});
		else
		{
			Rload_imm_to_GPreg(total_param_size_a, tmp_reg_id, 64);
			append_inst("SUB", {"SP", "SP", "X" + std::to_string(tmp_reg_id)});
		}

		auto arg_offset = total_param_size;
		for (auto p = stack_params.rbegin(); p != stack_params.rend(); ++p)
		{
			auto arg = *p;
			auto sz = arg->get_type() == Types::BOOL
				          ? Types::INT->sizeInBitsInArm64()
				          : arg->get_type()->sizeInBitsInArm64();
			arg_offset -= NUMBYTES(sz);
			if (arg->get_type() == Types::FLOAT)
			{
				Rload_to_FPreg(arg, tmp_reg_id);
				append_inst("STR", std::string(fvWidthPrefix.at(sz) + std::to_string(tmp_reg_id)),
				            MemAccess{"SP", "#" + std::to_string(arg_offset)});
			}
			else
			{
				Rload_to_GPreg(arg, tmp_reg_id);
				append_inst("STR", std::string(iWidthPrefix.at(sz) + std::to_string(tmp_reg_id)),
				            MemAccess{"SP", "#" + std::to_string(arg_offset)});
			}
		}
	}

	append_inst("BL", {call_inst->get_operand(0)->get_name()});

	if (!call_inst->is_void())
	{
		if (call_inst->get_type() == Types::FLOAT)
			Rstore_from_FPreg(call_inst, 0);
		else
			Rstore_from_GPreg(call_inst, 0);
	}
}

void ARMCodeGen::gen_prologue()
{
	append_inst("STP X29, X30, [SP, #-" + std::to_string(PROLOGUE_OFFSET_BASE) + "]!");
	append_inst("MOV X29, SP");
	if (IS_IMM12(context.frame_size))
	{
		append_inst("SUB", {"SP", "SP", "#" + std::to_string(context.frame_size)});
	}
	else
	{
		auto offset_reg_id = 9;
		Rload_imm_to_GPreg(context.frame_size, offset_reg_id);
		append_inst("SUB", {"SP", "SP", "X" + std::to_string(offset_reg_id)});
	}

	int int_id = 0, flt_id = 0;
	for (auto& arg : context.func->get_args())
	{
		// 加载不在 caller 栈帧上的参数
		if (arg.get_type() == Types::FLOAT && flt_id < 8)
		{
			MemAccess m = {"X29", "#" + std::to_string(context.offset_map.at(&arg))};
			auto sz = arg.get_type()->sizeInBitsInArm64();
			append_inst("STR", fvWidthPrefix.at(sz) + std::to_string(flt_id++), m);
		}
		else if (int_id < 8)
		{
			MemAccess m = {"X29", "#" + std::to_string(context.offset_map.at(&arg))};
			auto sz = arg.get_type() == Types::BOOL
				          ? Types::INT->sizeInBitsInArm64()
				          : arg.get_type()->sizeInBitsInArm64();
			append_inst("STR", iWidthPrefix.at(sz) + std::to_string(int_id++), m);
		}
	}
}

void ARMCodeGen::gen_epilogue()
{
	append_inst(func_exit_label_name(context.func), ASMInstruction::Label);
	append_inst("MOV SP, X29");
	append_inst("LDP X29, X30, [SP], #" + std::to_string(PROLOGUE_OFFSET_BASE));
	append_inst("RET");
}

void ARMCodeGen::gen_memop()
{
	auto inst = context.inst;
	if (inst->get_instr_type() == Instruction::memcpy_)
	{
		auto mcpyinst = static_cast<MemCpyInst*>(inst);
		Rload_to_GPreg(mcpyinst->get_to(), 0); // dst
		Rload_to_GPreg(mcpyinst->get_from(), 1); // src
		Rload_imm_to_GPreg(mcpyinst->get_copy_bytes(), 2); // size

		append_inst("BL", {"__memcpy__"});
	}
	else if (inst->get_instr_type() == Instruction::memclear_)
	{
		auto mclrinst = static_cast<MemClearInst*>(inst);
		Rload_to_GPreg(mclrinst->get_target(), 0); // dst
		Rload_imm_to_GPreg(mclrinst->get_clear_bytes(), 1); // size

		append_inst("BL", {"__memclr__"});
	}
}

void ARMCodeGen::gen_memclr_procedure()
{
	append_inst(".globl", {"__memclr__"}, ASMInstruction::Attribute);
	append_inst(".type", {"__memclr__", "@function"},
	            ASMInstruction::Attribute);
	append_inst("__memclr__", ASMInstruction::Label);
	append_inst("STP X29, X30, [SP, #-16]!");
	append_inst("MOV X29, SP");

	// 额外需要的寄存器: X2, Q0~Q3
	append_inst("SUB", {"SP", "SP", "#64"});
	append_inst("ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [SP]");
	append_inst("EOR", {"V0.16B", "V0.16B", "V0.16B"});
	append_inst("EOR", {"V1.16B", "V1.16B", "V1.16B"});
	append_inst("EOR", {"V2.16B", "V2.16B", "V2.16B"});
	append_inst("EOR", {"V3.16B", "V3.16B", "V3.16B"});
	append_inst("LSR", {"X2", "X1", "#7"});

	// 每次 128 bytes
	append_inst("1", ASMInstruction::Label);
	append_inst("CBZ", {"X2", "2f"});
	append_inst("ST1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X0]", "#64"});
	append_inst("ST1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X0]", "#64"});
	append_inst("SUBS", {"X2", "X2", "#1"});
	append_inst("B.NE", {"1b"});

	// 剩余部分
	append_inst("2", ASMInstruction::Label);
	append_inst("AND", {"X1", "X1", "#0x7F"});
	append_inst("LSR", {"X2", "X1", "#4"});

	// 每次 16 bytes，总字节数一定是 16 的倍数
	append_inst("3", ASMInstruction::Label);
	append_inst("CBZ", {"X2", "4f"});
	append_inst("STR", {"Q0", "[X0]", "#16"});
	append_inst("SUBS", {"X2", "X2", "#1"});
	append_inst("B.NE", {"3b"});

	// 返回
	append_inst("4", ASMInstruction::Label);
	append_inst("LD1 {V0.16B,V1.16B,V2.16B,V3.16B}, [SP], #64");

	append_inst("MOV SP, X29");
	append_inst("LDP X29, X30, [SP], #16");
	append_inst("RET");
	append_inst(".size", {"__memclr__", ".-__memclr__"}, ASMInstruction::Attribute);
}

void ARMCodeGen::gen_memcpy_procedure()
{
	append_inst(".globl", {"__memcpy__"}, ASMInstruction::Attribute);
	append_inst(".type", {"__memcpy__", "@function"},
	            ASMInstruction::Attribute);
	append_inst("__memcpy__", ASMInstruction::Label);
	append_inst("STP X29, X30, [SP, #-16]!");
	append_inst("MOV X29, SP");

	// 额外需要的寄存器: X3, Q0~Q3
	append_inst("SUB", {"SP", "SP", "#64"});
	append_inst("ST1 {V0.16B,V1.16B,V2.16B,V3.16B}, [SP]");
	append_inst("LSR", {"X3", "X2", "#7"});

	// 每次 128 bytes
	append_inst("1", ASMInstruction::Label);
	append_inst("CBZ", {"X3", "2f"});
	append_inst("LD1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X1]", "#64"});
	append_inst("ST1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X0]", "#64"});
	append_inst("LD1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X1]", "#64"});
	append_inst("ST1", {"{V0.16B,V1.16B,V2.16B,V3.16B}", "[X0]", "#64"});
	append_inst("SUBS", {"X3", "X3", "#1"});
	append_inst("B.NE", {"1b"});

	// 剩余部分
	append_inst("2", ASMInstruction::Label);
	append_inst("AND", {"X2", "X2", "#0x7F"});
	append_inst("LSR", {"X3", "X2", "#4"});

	// 每次 16 bytes，总字节数一定是 16 的倍数
	append_inst("3", ASMInstruction::Label);
	append_inst("CBZ", {"X3", "4f"});
	append_inst("LDR", {"Q0", "[X1]", "#16"});
	append_inst("STR", {"Q0", "[X0]", "#16"});
	append_inst("SUBS", {"X3", "X3", "#1"});
	append_inst("B.NE", {"3b"});

	// 返回
	append_inst("4", ASMInstruction::Label);
	append_inst("LD1 {V0.16B,V1.16B,V2.16B,V3.16B}, [SP], #64");

	append_inst("MOV SP, X29");
	append_inst("LDP X29, X30, [SP], #16");
	append_inst("RET");
	append_inst(".size", {"__memcpy__", ".-__memcpy__"}, ASMInstruction::Attribute);
}

// bitcast指令: 只需给指针定值
void ARMCodeGen::gen_nump2charp()
{
	auto i = static_cast<Nump2CharpInst*>(context.inst);
	auto tmp_reg_id = 9;
	Rload_to_GPreg(i->get_val(), tmp_reg_id);
	Rstore_from_GPreg(i, tmp_reg_id);
}

void ARMCodeGen::gen_cvt_global_type()
{
	auto i = static_cast<GlobalFixInst*>(context.inst);
	auto tmp_reg_id = 9;
	Rload_to_GPreg(i->get_var(), tmp_reg_id);
	Rstore_from_GPreg(i, tmp_reg_id);
}
