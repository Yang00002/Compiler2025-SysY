#pragma once
#include <list>
#include <string>
#include <vector>

#include "Instruction.hpp"

class MMSUB;
class MMathInst;
class MCMP;
class FuncAddress;
class Instruction;
class FrameIndex;
class Immediate;
class MInstruction;
class MOperand;
class Register;
class DynamicBitset;
class MFunction;
class ConstantValue;
class GlobalAddress;
class MModule;

class CodeGen
{
	MModule* m_;
	std::list<std::string> txt_;
	MFunction* func_ = nullptr;
	MFunction* func2Call_ = nullptr;
	static std::string section(const std::string& secName);
	static std::string objectTypeDeclare(const std::string& name);
	static std::string functionTypeDeclare(const std::string& name);
	static std::string align(int dataSize, bool glob);
	static std::string align(long long dataSize, bool glob);
	static std::string global(const std::string& globName);
	static std::string label(const std::string& name);
	static std::string word(const std::vector<ConstantValue>* v);
	static std::string zero(int count);
	static std::string size(const std::string& globName, int bytes);
	static std::string size(const std::string& globName, long long bytes);
	void append(const std::string& txt);
	void append(const std::list<std::string>& txt);
	static std::list<std::string> makeGlobal(const GlobalAddress* address);
	std::list<std::string> makeFunction(MFunction* function);
	std::list<std::string> functionPrefix(const MFunction* function);
	std::list<std::string> functionSuffix(const MFunction* function);
	static std::list<std::string> add(const Register* to, const Register* l, const Register* r, int len);
	static std::list<std::string> fadd(const Register* to, const Register* l, const Register* r);
	static std::list<std::string> fmul(const Register* to, const Register* l, const Register* r);
	static std::list<std::string> fdiv(const Register* to, const Register* l, const Register* r);
	static std::list<std::string> stp(const Register* a, const Register* b, const Register* c, int offset, int len);
	std::list<std::string> str(const Register* a, const Register* c, long long offset, int len);
	std::list<std::string> str(const MOperand* regLike, const MOperand* stackLike, int len);
	// 将操作数转换为特定类型寄存器(如果类型不一样就进行复制), 可能会占用临时寄存器, 需要释放
	const Register* op2reg(const MOperand* op, int len, bool useIntReg, std::list<std::string>& appendSlot);
	// 将操作数转换为寄存器(忽略它的类型, 默认是放到整型寄存器, 但是如果是其它类型也不转换), 可能会占用临时寄存器, 需要释放
	const Register* op2reg(const MOperand* op, int len, std::list<std::string>& appendSlot);
	static std::list<std::string> ldp(const Register* a, const Register* b, const Register* c, int offset, int len);
	std::list<std::string> ldr(const Register* a, const Register* baseOffsetReg, long long offset, int len);
	std::list<std::string> ldr(const MOperand* a, const MOperand* stackLike, int len);
	std::list<std::string> ld1(const Register* stackLike, int count, int offset);
	std::list<std::string> ld1(const MOperand* stackLike, int count, int offset);
	static std::list<std::string> clearV(int count);
	std::list<std::string> mathInst(const MMathInst* inst,const MOperand* t, const MOperand* l, const MOperand* r, Instruction::OpID op,
	                                int len);
	std::list<std::string> st1(const Register* stackLike, int count, int offset);
	std::list<std::string> st1(const MOperand* stackLike, int count, int offset);
	std::list<std::string> add32(const Register* to, const Register* l, int imm);
	std::list<std::string> add64(const Register* to, const Register* l, long long imm);
	std::list<std::string> sub32(const Register* to, const Register* l, int imm);
	std::list<std::string> sub64(const Register* to, const Register* l, long long imm);
	std::list<std::string> call(const MOperand* address);
	std::list<std::string> ret();
	static std::list<std::string> bcc(const MOperand* to, Instruction::OpID cond);
	static std::list<std::string> branch(const MOperand* to);
	std::list<std::string> cset(const MOperand* to, Instruction::OpID cond);
	std::list<std::string> f2i(const MOperand* from, const MOperand* to);
	std::list<std::string> i2f(const MOperand* from, const MOperand* to);
	std::list<std::string> extend32To64(const MOperand* from, const MOperand* to);
	std::list<std::string> compare(const MCMP* inst, const MOperand* l, const MOperand* r, bool flt);
	static void reverseCmpOp(const MCMP* inst);
	static std::list<std::string> sub(const Register* to, const Register* l, const Register* r, int len);
	std::list<std::string> mathRRInst(const Register* to, const Register* l, const Register* r, Instruction::OpID op,
	                                  int len);
	static std::list<std::string> fsub(const Register* to, const Register* l, const Register* r);
	static std::list<std::string> copy(const Register* to, const Register* from, int len);
	std::list<std::string> copy(const Register* to, const Immediate* from, int len);
	std::list<std::string> copy(const MOperand* to, const MOperand* from, int len);
	static std::list<std::string> copy(const Register* to, const GlobalAddress* from, int len);
	std::list<std::string> copy(const Register* to, const FrameIndex* from, int len);
	std::list<std::string> makeInstruction(MInstruction* instruction);
	static std::string instruction(const std::string& name, const std::string& op1, const std::string& op2,
	                               const std::string& op3);
	static std::string instruction(const std::string& name, const std::string& op1, const std::string& op2,
	                               const std::string& op3, const std::string& op4);
	static std::string instruction(const std::string& name, const std::string& op1, const std::string& op2);
	static std::string instruction(const std::string& name, const std::string& op1);
	static std::string regName(const Register* reg, int len);
	static std::string immediate(int i);
	static std::string immediate(unsigned i);
	static std::string immediate(long long i);
	static std::string fimmediate(float i);
	static std::string immediate(float i);
	static std::string poolImmediate(int i);
	static std::string poolImmediate(unsigned i);
	[[nodiscard]] Register* floatRegister(int i) const;
	static void decideCond(MInstruction* instruction, int cond);
	static std::string poolImmediate(long long i);
	[[nodiscard]] Register* zeroRegister() const;
	std::list<std::string> makeI64Immediate(long long i, const Register* placeIn);
	std::list<std::string> makeI32Immediate(int i, const Register* placeIn);
	std::list<std::string> makeF32Immediate(float i, const Register* placeIn);
	std::list<std::string> makeImmediate(const Immediate* i, bool flt, int len, const Register* placeIn);
	static std::string leftShift12();
	static std::string leftShift(int count);
	static std::string functionSize(const std::string& name);
	static std::string regDataOffset(const Register* reg, int offset);
	static std::string regDataRegLSLOffset(const Register* reg, const Register* regofs, int offset);
	static std::string regData(const Register* reg);
	static const char* genMemcpy();
	static const char* genMemclr();
	static void merge(std::list<std::string>& l, const std::list<std::string>& r);
	static bool canLSInOneSPMove(const std::vector<std::pair<Register*, long long>>& offsets);
	long long  frameOffset(const FrameIndex* index, bool isStore, std::list<std::string>& appendSlot);
	[[nodiscard]] Register* sp() const;
	const MOperand* opbuffer[2] = {};
	bool ipcd_[2] = {};
	bool fipcd_[2] = {};
	bool ip16_ = false;
	bool fip16_ = false;
	Register* getIP();
	Register* getIPOfType(bool flt);
	Register* getFIP();
	void releaseIP(const Register* r);
	void setBuf(int id, const MOperand* op);
	[[nodiscard]] const MOperand* getBuf(int id) const;
	void releaseBuf(int id);

public:
	CodeGen(MModule* m);
	void run();
	[[nodiscard]] std::string print() const;
};
