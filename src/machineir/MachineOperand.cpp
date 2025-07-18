#include "MachineOperand.hpp"
#include "Type.hpp"
#include "MachineIR.hpp"
#include "MachineBasicBlock.hpp"

using namespace std;


bool MOperand::isRegisterLike()
{
	return dynamic_cast<Register*>(this) || dynamic_cast<VirtualRegister*>(this);
}

Immediate::Immediate(unsigned int value) : val_(value)
{
}

Immediate* Immediate::getImmediate(int intImm, MModule* m)
{
	unsigned int v;
	memcpy(&v, &intImm, sizeof(unsigned));
	auto f = m->imm_cache_.find(v);
	if (f != m->imm_cache_.end()) return f->second;
	auto ret = new Immediate{v};
	m->imm_cache_.emplace(v, ret);
	return ret;
}

Immediate* Immediate::getImmediate(float floatImm, MModule* m)
{
	unsigned int v;
	memcpy(&v, &floatImm, sizeof(unsigned));
	auto f = m->imm_cache_.find(v);
	if (f != m->imm_cache_.end()) return f->second;
	auto ret = new Immediate{v};
	m->imm_cache_.emplace(v, ret);
	return ret;
}

int Immediate::asInt() const
{
	int i;
	memcpy(&i, &val_, sizeof(int));
	return i;
}

float Immediate::asFloat() const
{
	float i;
	memcpy(&i, &val_, sizeof(int));
	return i;
}

VirtualRegister::VirtualRegister(unsigned int id, bool ireg_t_freg_f) : id_(id), ireg_t_freg_f_(ireg_t_freg_f)
{
}

VirtualRegister* VirtualRegister::getVirtualIRegister(int id, const MModule* m)
{
	if (static_cast<int>(m->virtual_iregs_.size()) >= id) return nullptr;
	return m->virtual_iregs_[id];
}

VirtualRegister* VirtualRegister::createVirtualIRegister(MModule* m)
{
	unsigned int id = static_cast<unsigned int>(m->virtual_iregs_.size());
	auto reg = new VirtualRegister{id, true};
	m->virtual_iregs_.emplace_back(reg);
	return reg;
}

VirtualRegister* VirtualRegister::getVirtualFRegister(int id, const MModule* m)
{
	if (static_cast<int>(m->virtual_fregs_.size()) >= id) return nullptr;
	return m->virtual_fregs_[id];
}

VirtualRegister* VirtualRegister::createVirtualFRegister(MModule* m)
{
	unsigned int id = static_cast<unsigned int>(m->virtual_fregs_.size());
	auto reg = new VirtualRegister{id, false};
	m->virtual_fregs_.emplace_back(reg);
	return reg;
}

Register::Register(unsigned int id, bool ireg_t_freg_f) : id_(id), ireg_t_freg_f_(ireg_t_freg_f)
{
}

Register* Register::getIRegister(int id, const MModule* m)
{
	if (id >= 32) return nullptr;
	return m->iregs_[id];
}

Register* Register::getFRegister(int id, const MModule* m)
{
	if (id >= 32) return nullptr;
	return m->fregs_[id];
}

Register* Register::getRegisterWithType(int id, const Type* ty, const MModule* m)
{
	if (ty == Types::INT) return getIRegister(id, m);
	return getFRegister(id, m);
}

Register* Register::getLR(const MModule* m)
{
	return getIRegister(30, m);
}

Register* Register::getNZCV(const MModule* m)
{
	return getIRegister(32, m);
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

FrameIndex::FrameIndex(MFunction* func, unsigned int idx, size_t size, bool stack_t_fix_f) : func_(func), size_(size),
	index_(idx), stack_t_fix_f_(stack_t_fix_f)
{
}
