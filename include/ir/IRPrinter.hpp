#pragma once
#include <string>
#include "Instruction.hpp"

class Value;


std::string print_as_op(Value *v, bool print_ty);
std::string print_instr_op_name(Instruction::OpID);
