#ifndef GLOBALVAR_HPP
#define GLOBALVAR_HPP

#include "Constant.hpp"
#include "User.hpp"

class Module;
class GlobalVariable : public User {
private:
    bool is_const_;
    Constant *init_val_;
public:
    GlobalVariable(std::string name, Module *m, Type *ty, bool is_const,
                   Constant *init = nullptr);
    GlobalVariable(const GlobalVariable &) = delete;
    static GlobalVariable *create(std::string name, Module *m, Type *ty,
                                  bool is_const, Constant *init);
    virtual ~GlobalVariable() = default;
    Constant *get_init() { return init_val_; }
    bool is_const() { return is_const_; }
    std::string print();
};

#endif