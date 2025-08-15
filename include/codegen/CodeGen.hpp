#pragma once
#include <list>
#include <string>
#include <vector>

#include "Instruction.hpp"
#include "MachinePassManager.hpp"

class MMAddSUB;
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

// 生成实际的汇编指令
class CodeGen : public MachinePass
{
	MFunction* func_ = nullptr;
	MFunction* func2Call_ = nullptr;
	static std::string word(const std::vector<ConstantValue>* v);
	static std::string zero(int count);
	static std::string size(const std::string& globName, int bytes);
	static std::string size(const std::string& globName, long long bytes);
	void makeGlobal(const GlobalAddress* address) const;
	void makeFunction();
	void functionPrefix();
	void functionSuffix();
	static void add(const Register* to, const Register* l, const Register* r, int len, CodeString* toStr);
	static void fadd(const Register* to, const Register* l, const Register* r, CodeString* toStr);
	static void fmul(const Register* to, const Register* l, const Register* r, CodeString* toStr);
	static void fdiv(const Register* to, const Register* l, const Register* r, CodeString* toStr);
	static void stp(const Register* a, const Register* b, const Register* c, int offset, int len, CodeString* toStr);
	void str(const Register* a, const Register* c, long long offset, int len, CodeString* toStr);
	void str(const MOperand* regLike, const MOperand* stackLike, int len, CodeString* toStr, bool forFuncCall);
	// 将操作数转换为特定类型寄存器(如果类型不一样就进行复制), 可能会占用临时寄存器, 需要释放
	const Register* op2reg(const MOperand* op, int len, bool useIntReg, CodeString* toStr);
	// 将操作数转换为寄存器(忽略它的类型, 默认是放到整型寄存器, 但是如果是其它类型也不转换), 可能会占用临时寄存器, 需要释放
	const Register* op2reg(const MOperand* op, int len, CodeString* appendSlot);
	static void ldp(const Register* a, const Register* b, const Register* c, int offset, int len, CodeString* toStr);
	void ldr(const Register* a, const Register* baseOffsetReg, long long offset, int len, CodeString* toStr);
	void ldr(const MOperand* a, const MOperand* stackLike, int len, CodeString* toStr);
	void ld1(const Register* stackLike, int count, int offset, CodeString* toStr);
	void ld1(const MOperand* stackLike, int count, int offset, CodeString* toStr);
	static void clearV(int count, CodeString* toStr);
	void mathInst(const MOperand* t, const MOperand* l, const MOperand* r,
	              Instruction::OpID op,
	              int len, CodeString* toStr);
	void add32(const Register* to, const Register* l, int imm, CodeString* toStr);
	static void lsl32(const Register* to, const Register* l, int imm, CodeString* toStr);
	static void lsl64(const Register* to, const Register* l, long long imm, CodeString* toStr);
	static void asr32(const Register* to, const Register* l, int imm, CodeString* toStr);
	static void asr64(const Register* to, const Register* l, long long imm, CodeString* toStr);
	void and32(const Register* to, const Register* l, int imm, CodeString* toStr);
	void and64(const Register* to, const Register* l, long long imm, CodeString* toStr);
	void add64(const Register* to, const Register* l, long long imm, CodeString* toStr);
	void sub32(const Register* to, const Register* l, int imm, CodeString* toStr);
	void sub64(const Register* to, const Register* l, long long imm, CodeString* toStr);
	void call(const MOperand* address, CodeString* toStr);
	static void ret(CodeString* toStr);
	void cset(const MOperand* to, Instruction::OpID cond, CodeString* toStr);
	void f2i(const MOperand* from, const MOperand* to, CodeString* toStr);
	void i2f(const MOperand* from, const MOperand* to, CodeString* toStr);
	void extend32To64(const MOperand* from, const MOperand* to, CodeString* toStr);
	void compare(const MCMP* inst, const MOperand* l, const MOperand* r, bool flt, CodeString* toStr);
	static void reverseCmpOp(const MCMP* inst);
	static void sub(const Register* to, const Register* l, const Register* r, int len, CodeString* toStr);
	void mathRRInst(const Register* to, const Register* l, const Register* r, Instruction::OpID op,
	                int len, CodeString* toStr);
	void maddsub(MOperand* to, MOperand* l, MOperand* r, MOperand* s, bool isAdd, CodeString* toStr);
	void mneg(MOperand* to, MOperand* l, MOperand* r, CodeString* toStr);
	static void fsub(const Register* to, const Register* l, const Register* r, CodeString* toStr);
	static void copy(const Register* to, const Register* from, int len, CodeString* toStr);
	void copy(const Register* to, const Immediate* from, int len, CodeString* toStr);
	void copy(const MOperand* to, const MOperand* from, int len, CodeString* toStr);
	static void copy(const Register* to, const GlobalAddress* from, int len, CodeString* toStr);
	void copy(const Register* to, const FrameIndex* from, int len, CodeString* toStr);
	void makeInstruction(MInstruction* instruction);
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
	void makeI64Immediate(long long i, const Register* placeIn, CodeString* toStr);
	void makeI32Immediate(int i, const Register* placeIn, CodeString* toStr);
	void makeF32Immediate(float i, const Register* placeIn, CodeString* toStr);
	void makeImmediate(const Immediate* i, bool flt, int len, const Register* placeIn, CodeString* toStr);
	static std::string leftShift12();
	static std::string leftShift(int count);
	static std::string functionSize(const std::string& name);
	static std::string regDataOffset(const Register* reg, int offset);
	static std::string regDataRegLSLOffset(const Register* reg, const Register* regofs, int offset);
	static std::string regData(const Register* reg);
	static const char* genMemcpy();
	static const char* genMemclr();
	static bool canLSInOneSPMove(const std::vector<std::pair<Register*, long long>>& offsets);
	long long frameOffset(const FrameIndex* index, bool forCall, CodeString* appendSlot);
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
	void st1(const Register* stackLike, int count, int offset, CodeString* toStr);
	void st1(const MOperand* stackLike, int count, int offset, CodeString* toStr);
public:
	static bool mathInstImmediateCanInline(const MOperand* l, const MOperand* r, Instruction::OpID op, int len);
	static bool immCanInlineInAddSub(int imm);
	static bool immCanInlineInAddSub(long long imm);
	static int ldrNeedInstCount(long long offset, int len);
	static int copyFrameNeedInstCount(long long offset);
	static int makeI64ImmediateNeedInstCount(long long i);
	CodeGen(MModule* m);
	void run() override;
};
