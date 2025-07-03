#ifndef CODEGEN_HPP
#define CODEGEN_HPP
#include <cassert>
#include <string>
#include <iostream>
#include <map>
#include <cstdint>

#include "../util/Type.hpp"
#include "../util/Tensor.hpp"
#include "../ir/Constant.hpp"
#include "../ir/Instruction.hpp"
#include "../ir/Module.hpp"
#include "../ir/BasicBlock.hpp"

#define IMM8_MAX 0xFF
#define IMM8_MIN -0x100
inline bool IS_IMM8(int x){ return x <= IMM8_MAX && x >= IMM8_MIN; }

#define IMM9_MAX 0x1FF
#define IMM9_MIN -0x200
inline bool IS_IMM9(int x){ return x <= IMM9_MAX && x >= IMM9_MIN; }

#define IMM12_MAX 0x7FF
#define IMM12_MIN -0x800
inline bool IS_IMM12(int x) { return x <= IMM12_MAX && x >= IMM12_MIN; }

#define IMM32_MAX 0x7FFFFFFF
#define IMM32_MIN -0x80000000LL
inline bool IS_IMM32(int64_t x) { return x <= IMM32_MAX && x >= IMM32_MIN; }

#define NUMBYTES(x) (((x)+7)>>3)
#define PROLOGUE_OFFSET_BASE 16 
#define PROLOGUE_ALIGN 16
inline unsigned ALIGN(unsigned x, unsigned alignment) { return ((x+(alignment-1)) & ~(alignment-1)); }

const std::map<uint32_t, const std::string> iWidthPrefix = { {1,"W"},{8,"W"},{32,"W"},{64,"X"}};
const std::map<uint32_t, const std::string> iWidthSuffix = { {1,"B"},{8,"B"},{16,"H"},{32,"S"},{64,"D"}};
const std::map<uint32_t, const std::string> fvWidthPrefix = {{8,"B"},{16,"H"},{32,"S"},{64,"D"},{128,"Q"}};
const std::map<uint32_t, const std::string> ConstInitLen = { {1,".byte"}, {4,".word"}, {8, ".quad"}};

struct ASMInstruction {
    enum InstType { Instruction, Attribute, Label, Comment } type;
    std::string content;

    explicit ASMInstruction(std::string s, InstType ty = Instruction)
        : type(ty), content(s) {}

    std::string format() const {
        switch (type) {
        case ASMInstruction::Instruction:
        case ASMInstruction::Attribute:
            return "\t" + content + "\n";
        case ASMInstruction::Label:
            return content + ":\n";
        case ASMInstruction::Comment:
            return "# " + content + "\n";
        }
        assert(false && "unreachable");
    }
};

class ARMCodeGen{    
    Module* m = nullptr;
    std::string code_;
    std::list<ASMInstruction> output;
    
    struct {
        /* 随着ir遍历设置 */
        Function* func{nullptr};    // 当前函数
        BasicBlock* bb{nullptr};    // 当前基本块
        Instruction* inst{nullptr}; // 当前指令
        unsigned current_offset {0}; // 指令在当前栈帧中的位置，用于函数调用
        /* 在allocate()中设置 */
        unsigned frame_size {0};              // 当前函数的栈帧大小
        std::map<Value *, int> offset_map {}; // 指针相对 fp 的偏移
        unsigned fcmp_cnt{0};                 // fcmp 的计数器, 用于创建 fcmp 需要的 label
        void clear() {
            func = nullptr;
            bb = nullptr;
            inst = nullptr;
            frame_size = 0;
            fcmp_cnt = 0;
            offset_map.clear();
        }
    } context;

    static std::string label_name(BasicBlock *bb) { return "." + bb->get_parent()->get_name() + "_" + bb->get_name(); }
    static std::string func_exit_label_name(Function *func) { return func->get_name() + "_exit"; }
    static std::string fcmp_label_name(BasicBlock *bb, unsigned cnt) { return label_name(bb) + "_fcmp_" + std::to_string(cnt); }
    
    void Rload_to_GPreg(Value* v, unsigned Rid);
    void Rstore_from_GPreg(Value* v, unsigned Rid);
    void Rload_to_FPreg(Value* v, unsigned FRid);
    void Rstore_from_FPreg(Value* v, unsigned FRid);
    void Rload_imm_to_FPreg(float f, unsigned FRid);
    void Rload_stack_to_GPreg(Value* v, unsigned Rid);
    void Rload_stack_to_FPreg(Value* val, unsigned FRid);
    void Rload_imm_to_GPreg(int64_t imm, unsigned Rid, unsigned bits=0);
    void Rload_32_to_GPreg(int32_t i, unsigned Rid);
    void Rload_64_to_GPreg(int64_t i, unsigned Rid);

    void gen_memclr_procedure();
    void gen_memcpy_procedure();

    void allocate();
    void copy_stmt();
    void gen_prologue();
    void gen_ret();
    void gen_br();
    void gen_binary();
    void gen_fbinary();
    void gen_alloca();
    void gen_load();
    void gen_store();
    void gen_icmp();
    void gen_fcmp();
    void gen_zext();
    void gen_call();
    void gen_gep();
    void gen_sitofp();
    void gen_fptosi();
    void gen_epilogue();
    void gen_memop();
    void gen_nump2charp();
    void gen_cvt_global_type();
    
    struct MemAccess {
        std::string base;
        std::string offset = "#0";

        std::string format(const std::string& ofstr="") const {
            std::string result = "[" + base;
            std::string of = ofstr.empty() ? offset : ofstr;
            if (!of.empty()) {
                result += ", " + of;
            }
            result += "]";
            return result;
        }

        int get_offset() const { return std::stoi(offset.substr(1)); }
    };
    
    // TODO: 寄存器分配
    unsigned get_avail_tmp_reg( TypeIDs ty_=TypeIDs::Integer ) { return 8; }
    
    template <class... Args>
    void append_inst(const char *inst, std::initializer_list<std::string> args,
                    ASMInstruction::InstType ty = ASMInstruction::Instruction) {
        auto content = std::string(inst) + " ";
        for (const auto &arg : args)
            content += arg + ", ";
        if (!args.size()) content.pop_back(); else content.pop_back(), content.pop_back();
        output.emplace_back(content, ty);
    }

    // LDR/STR/...
    void append_inst(const char *inst, const std::string &reg,
                    const MemAccess& mem, ASMInstruction::InstType ty = ASMInstruction::Instruction) {
        bool indirect = !IS_IMM8(mem.get_offset());
        auto offset_reg = get_avail_tmp_reg();
        if(indirect)
            Rload_imm_to_GPreg(mem.get_offset(), offset_reg, 64);
        std::string content = std::string(inst) + " " + reg + ", " + mem.format(
            indirect ? "X"+std::to_string(offset_reg) : "" );
        output.emplace_back(content, ty);
    }

    // LDP/STP/...
    void append_inst(const char *inst, const std::vector<std::string> &regs,
                    const MemAccess& mem, ASMInstruction::InstType ty = ASMInstruction::Instruction) {
        bool indirect = !IS_IMM8(mem.get_offset());
        auto offset_reg = get_avail_tmp_reg();
        if(indirect)
            Rload_imm_to_GPreg(mem.get_offset(), offset_reg, 64);

        std::string reg_list;
        for (const auto &r : regs) {
            reg_list += r + ", ";
        }
        if (!regs.empty()) {
            reg_list.pop_back();
            reg_list.pop_back();
        }

        std::string content = std::string(inst) + " " + reg_list + ", " + mem.format(
            indirect ? "X"+std::to_string(offset_reg) : "" );
        output.emplace_back(content, ty);
    }

    template <class... Args>
    void append_inst(Args... args) {
        output.emplace_back(std::forward<Args>(args)...);
    }

public:
    ARMCodeGen(Module* m) : m(m) {}
    void run();
    std::string print() const;
};


#endif