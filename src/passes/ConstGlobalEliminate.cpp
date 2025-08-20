#include "ConstGlobalEliminate.hpp"

#include "Ast.hpp"
#include "Constant.hpp"
#include "FuncInfo.hpp"
#include "Instruction.hpp"
#include "Tensor.hpp"
#include "Type.hpp"

#define DEBUG 0
#include "Util.hpp"

void ConstGlobalEliminate::run() {
  manager_->flushGlobalInfo<FuncInfo>();
  std::unordered_set<GlobalVariable *> haveStore;
  for (auto glob : m_->get_global_variable()) {
    auto ty = glob->get_type()->toPointerType()->typeContained();
    if (ty->isBasicType()) {
      bool ok = true;
      for (auto use : glob->get_use_list()) {
        if (dynamic_cast<StoreInst *>(use.val_) != nullptr) {
          ok = false;
          haveStore.emplace(glob);
          break;
        }
      }
      if (!ok)
        continue;
      Constant *ct;
      if (ty == Types::FLOAT) {
        float val = 0.0f;
        auto seg = glob->get_init()->segment(0);
        if (seg.second == 0)
          val = seg.first->front().getFloatConstant();
        ct = Constant::create(m_, val);
      } else {
        int val = 0;
        auto seg = glob->get_init()->segment(0);
        if (seg.second == 0)
          val = seg.first->front().getIntConstant();
        ct = Constant::create(m_, val);
      }
      std::vector<LoadInst *> rm;
      for (auto use : glob->get_use_list()) {
        auto loadInst = dynamic_cast<LoadInst *>(use.val_);
        ASSERT(loadInst != nullptr);
        LOG(color::green("Replace ") + loadInst->print() +
            color::green(" with ") + ct->print());
        loadInst->replace_all_use_with(ct);
        rm.emplace_back(loadInst);
      }
      for (auto i : rm) {
        i->get_parent()->erase_instr(i);
        delete i;
      }
    }
  }
  if (localize_)
    for (auto glob :
         haveStore) // NOLINT(bugprone-nondeterministic-pointer-iteration-order)
    {
      Function *f = nullptr;
      bool ok = true;
      for (auto use : glob->get_use_list()) {
        auto inst = dynamic_cast<Instruction *>(use.val_);
        auto instF = inst->get_parent()->get_parent();
        if (f == nullptr)
          f = instF;
        else if (f != instF) {
          ok = false;
          break;
        }
      }
      if (!ok || f == nullptr)
        continue;
      auto stb = f->get_basic_blocks().front();
      LOG(color::yellow("Localize ") + glob->print());
      auto allc = AllocaInst::create_alloca(
          glob->get_type()->toPointerType()->typeContained(), stb);
      auto ld = LoadInst::create_load(glob, nullptr);
      auto st2allc = StoreInst::create_store(ld, allc, nullptr);
      ld->set_parent(stb);
      st2allc->set_parent(stb);
      stb->add_instr_begin(st2allc);
      stb->add_instr_begin(ld);
      glob->replace_all_use_with(allc);
      ld->set_operand(0, glob);
      PREPARE_PASS_MSG;
      LOG(color::green("create ") + allc->print());
      LOG(color::green("create ") + ld->print());
      LOG(color::green("create ") + st2allc->print());
      for (auto bb : f->get_basic_blocks()) {
        if (bb->get_terminator()->is_ret()) {
          auto ret = bb->get_instructions().pop_back();
          auto ldfallc = LoadInst::create_load(allc, bb);
          auto st = StoreInst::create_store(ldfallc, glob, bb);
          bb->add_instruction(ret);
          PREPARE_PASS_MSG;
          LOG(color::green("create ") + ldfallc->print());
          LOG(color::green("create ") + st->print());
        }
      }
    }

  PASS_SUFFIX;
}
