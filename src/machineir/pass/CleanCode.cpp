#include "CleanCode.hpp"

#include "MachineBasicBlock.hpp"
#include "MachineFunction.hpp"
#include "MachineInstruction.hpp"

#define DEBUG 0
#include "Util.hpp"

void CleanCode::removeInst(MInstruction* inst)
{
	LOG(color::red("remove ") + inst->print());
	auto f = inst->block()->function();
	for (auto i : inst->operands())
	{
		f->removeUse(i, inst);
	}
	delete inst;
}

void CleanCode::run()
{
	for (auto f : m_->functions())
	{
		for (auto bb : f->blocks())
		{
			std::vector<MInstruction*> ok;
			MInstruction* pre = nullptr;
			auto& insts = bb->instructions();
			for (auto i : insts)
			{
				if (pre == nullptr)
				{
					pre = i;
					continue;
				}
				{
					// ST a -> b + ST a -> b
					auto st = dynamic_cast<MSTR*>(i);
					if (st != nullptr)
					{
						auto st2 = dynamic_cast<MSTR*>(pre);
						if (st2 != nullptr)
						{
							if (st2->operand(0) == st->operand(0) && st2->operand(1) == st->operand(1))
							{
								removeInst(st2);
								pre = st;
							}
							else
							{
								ok.emplace_back(st2);
								pre = st;
							}
						}
						else
						{
							ok.emplace_back(pre);
							pre = st;
						}
						continue;
					}
				}
				{
					// LD a <- b + ST a -> b
					auto c = dynamic_cast<MSTR*>(i);
					if (c != nullptr)
					{
						auto p = dynamic_cast<MLDR*>(pre);
						if (p != nullptr)
						{
							// ST a -> b + LD a <- b
							if (c->operand(0) == p->operand(0) && c->operand(1) == p->operand(1))
								removeInst(c);
							else
							{
								ok.emplace_back(p);
								pre = c;
							}
						}
						else
						{
							ok.emplace_back(pre);
							pre = c;
						}
						continue;
					}
				}
				{
					auto c = dynamic_cast<MLDR*>(i);
					if (c != nullptr)
					{
						auto p = dynamic_cast<MSTR*>(pre);
						if (p != nullptr)
						{
							// ST a -> b + LD a <- b
							if (c->operand(0) == p->operand(0) && c->operand(1) == p->operand(1))
								removeInst(c);
								// ST a -> b + LD c <- b
							else if (c->operand(1) == p->operand(1))
							{
								auto mv = new MCopy{bb, p->operand(0), c->operand(0), c->width()};
								removeInst(c);
								ok.emplace_back(p);
								pre = mv;
							}
							else
							{
								ok.emplace_back(p);
								pre = c;
							}
						}
						else
						{
							ok.emplace_back(pre);
							pre = c;
						}
						continue;
					}
				}
				{
					auto c = dynamic_cast<MLDR*>(i);
					if (c != nullptr)
					{
						auto p = dynamic_cast<MLDR*>(pre);
						if (p != nullptr)
						{
							// LD a <- b + LD a <- b
							if (c->operand(0) == p->operand(0) && c->operand(1) == p->operand(1))
								removeInst(c);
								// LD a <- b + LD c <- b
							else if (c->operand(1) == p->operand(1))
							{
								auto mv = new MCopy{bb, p->operand(0), c->operand(0), c->width()};
								removeInst(c);
								ok.emplace_back(p);
								pre = mv;
							}
							else
							{
								ok.emplace_back(p);
								pre = c;
							}
						}
						else
						{
							ok.emplace_back(pre);
							pre = c;
						}
						continue;
					}
				}
				{
					auto c = dynamic_cast<MCopy*>(i);
					if (c != nullptr)
					{
						auto p = dynamic_cast<MCopy*>(pre);
						if (p != nullptr)
						{
							// CP a <- b + CP b <- a
							if (c->operand(0) == p->operand(1) && c->operand(1) == p->operand(0))
								removeInst(c);
							else
							{
								ok.emplace_back(p);
								pre = c;
							}
						}
						else
						{
							ok.emplace_back(pre);
							pre = c;
						}
						continue;
					}
				}
				if (pre != nullptr)
				{
					ok.emplace_back(pre);
					pre = i;
				}
			}
			if (pre != nullptr)
			{
				ok.emplace_back(pre);
			}
			LOG(color::yellow("pre ") + std::to_string(insts.size()) + color::yellow(" current ") + std::to_string(ok.
				size()));
			insts = ok;
		}
	}
}
