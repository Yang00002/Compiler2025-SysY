#include "MachineOperand.hpp"

#include <cstring>
#include <iomanip>
#include <sstream>

#include "GlobalVariable.hpp"
#include "Type.hpp"
#include "MachineFunction.hpp"
#include "MachineBasicBlock.hpp"
#include "Constant.hpp"


using namespace std;


bool MOperand::isRegisterLike()
{
	return dynamic_cast<Register*>(this) || dynamic_cast<VirtualRegister*>(this);
}

bool MOperand::isCanAllocateRegister()
{
	auto r = dynamic_cast<RegisterLike*>(this);
	return r != nullptr && r->canAllocate();
}

Immediate::Immediate(unsigned long long value) : val_(value)
{
}

Immediate* Immediate::getImmediate(long long pImm, MModule* m)
{
	unsigned long long v = 0;
	memcpy(&v, &pImm, sizeof(int));
	auto f = m->imm_cache_.find(v);
	if (f != m->imm_cache_.end()) return f->second;
	auto ret = new Immediate{v};
	m->imm_cache_.emplace(v, ret);
	return ret;
}

Immediate* Immediate::getImmediate(int intImm, MModule* m)
{
	long long l = intImm;
	unsigned long long v = 0;
	memcpy(&v, &l, sizeof(long long));
	auto f = m->imm_cache_.find(v);
	if (f != m->imm_cache_.end()) return f->second;
	auto ret = new Immediate{v};
	m->imm_cache_.emplace(v, ret);
	return ret;
}

Immediate* Immediate::getImmediate(float floatImm, MModule* m)
{
	unsigned long long v = 0;
	memcpy(&v, &floatImm, sizeof(int));
	auto f = m->imm_cache_.find(v);
	if (f != m->imm_cache_.end()) return f->second;
	auto ret = new Immediate{v};
	m->imm_cache_.emplace(v, ret);
	return ret;
}

int Immediate::asInt() const
{
	long long l;
	memcpy(&l, &val_, sizeof(long long));
	return static_cast<int>(l);
}

float Immediate::asFloat() const
{
	float i;
	memcpy(&i, &val_, sizeof(int));
	return i;
}

long long Immediate::as64BitsInt() const
{
	long long i;
	memcpy(&i, &val_, sizeof(long long));
	return i;
}

bool Immediate::isZero(bool flt, int len) const
{
	if (flt) return asFloat() == 0;
	if (len == 64) return as64BitsInt() == 0;
	return asInt() == 0;
}

bool Immediate::isNotFloatOne(bool flt, int len) const
{
	if (flt)return false;
	if (len == 64) return as64BitsInt() == 1;
	return asInt() == 1;
}

std::string Immediate::print()
{
	std::stringstream ss;
	ss << std::hex << std::setfill('0') << std::setw(8) << val_;
	return "0x" + ss.str();
}

bool RegisterLike::isVirtualRegister() const
{
	return false;
}

bool RegisterLike::isPhysicalRegister() const
{
	return false;
}

int RegisterLike::uid() const
{
	return (id() << 1) + isVirtualRegister();
}

VirtualRegister::VirtualRegister(int id, bool ireg_t_freg_f, int size) : id_(id),
	size_(size), ireg_t_freg_f_(ireg_t_freg_f)
{
}

bool VirtualRegister::isVirtualRegister() const
{
	return true;
}

VirtualRegister* VirtualRegister::createVirtualIRegister(MFunction* f, int size)
{
	int id = u2iNegThrow(f->virtual_iregs_.size());
	auto reg = new VirtualRegister{id, true, size};
	f->virtual_iregs_.emplace_back(reg);
	return reg;
}

VirtualRegister* VirtualRegister::createVirtualFRegister(MFunction* f, int size)
{
	int id = u2iNegThrow(f->virtual_fregs_.size());
	auto reg = new VirtualRegister{id, false, size};
	f->virtual_fregs_.emplace_back(reg);
	return reg;
}

VirtualRegister* VirtualRegister::copy(MFunction* f, const VirtualRegister* v)
{
	if (v->ireg_t_freg_f_)
	{
		return createVirtualIRegister(f, v->size());
	}
	return createVirtualFRegister(f, v->size());
}

std::string VirtualRegister::print()
{
	return (ireg_t_freg_f_ ? "%vireg" : "%vfreg") + to_string(id_) + "<" + to_string(size_) + ">";
}

bool Register::isPhysicalRegister() const
{
	return true;
}

Register::Register(int id, bool ireg_t_freg_f, const std::string& name,
                   const std::string& sname) : id_(id), ireg_t_freg_f_(ireg_t_freg_f)
{
	name_ = name;
	shortName_ = sname;
}

Register* Register::getIParameterRegister(int id, const MModule* m)
{
	if (id >= 8) return nullptr;
	return m->iregs_[id];
}

Register* Register::getFParameterRegister(int id, const MModule* m)
{
	if (id >= 8) return nullptr;
	return m->fregs_[id];
}

Register* Register::getParameterRegisterWithType(int id, const Type* ty, const MModule* m)
{
	if (ty == Types::FLOAT) return getFParameterRegister(id, m);
	return getIParameterRegister(id, m);
}

Register* Register::getIReturnRegister(const MModule* m)
{
	return getIParameterRegister(0, m);
}

Register* Register::getFReturnRegister(const MModule* m)
{
	return getFParameterRegister(0, m);
}

Register* Register::getReturnRegisterWithType(const Type* ty, const MModule* m)
{
	return getParameterRegisterWithType(0, ty, m);
}

Register* Register::getLR(const MModule* m)
{
	return m->iregs_[28];
}

Register* Register::getNZCV(const MModule* m)
{
	return m->iregs_[31];
}

Register* Register::getZERO(const MModule* m)
{
	return m->iregs_[29];
}

Register* Register::getFP(const MModule* m)
{
	return m->iregs_[27];
}

Register* Register::getSP(const MModule* m)
{
	return m->iregs_[30];
}

Register* Register::getIIP0(const MModule* m)
{
	return m->iregs_[15];
}

Register* Register::getIIP1(const MModule* m)
{
	return m->iregs_[16];
}

Register* Register::getFIP0(const MModule* m)
{
	return m->fregs_[16];
}

Register* Register::getFIP1(const MModule* m)
{
	return m->fregs_[17];
}

std::string Register::print()
{
	return name_;
}

BlockAddress::BlockAddress(MBasicBlock* block) : block_(block)
{
}

BlockAddress* BlockAddress::get(MBasicBlock* bb)
{
	auto& m = bb->function()->ba_cache_;
	auto f = m.find(bb);
	if (f == m.end())
	{
		auto n = new BlockAddress{bb};
		m.emplace(bb, n);
		return n;
	}
	return f->second;
}

std::string BlockAddress::print()
{
	return "block:" + block_->name();
}

FuncAddress::FuncAddress(MFunction* block): func_(block)
{
}

FuncAddress* FuncAddress::get(MFunction* bb)
{
	auto& m = bb->module()->func_address_;
	auto f = m.find(bb);
	if (f == m.end())
	{
		auto n = new FuncAddress{bb};
		m.emplace(bb, n);
		return n;
	}
	return f->second;
}

std::string FuncAddress::print()
{
	return "func:" + func_->name();
}

GlobalAddress::GlobalAddress(MModule* module, GlobalVariable* var)
{
	index_ = u2iNegThrow(module->globals_.size());
	size_ = var->get_type()->toPointerType()->typeContained()->sizeInBitsInArm64();
	data_ = var->move_init();
	const_ = var->is_const();
	module->globals_.emplace_back(this);
	name_ = var->get_name();
}

GlobalAddress::~GlobalAddress()
{
	delete data_;
}

std::string GlobalAddress::print()
{
	return "global:" + name_;
}

FrameIndex::FrameIndex(MFunction* func, int idx, long long size, bool stack_t_fix_f) : func_(func), size_(size),
	index_(idx), stack_t_fix_f_(stack_t_fix_f), spilledFrame_(false)
{
}

std::string FrameIndex::print()
{
	return (stack_t_fix_f_ ? "%stackFrame" : "%fixFrame") + to_string(index_) + "<" + to_string(size_) + ">";
}
