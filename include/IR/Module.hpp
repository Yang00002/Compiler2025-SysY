#ifndef MODULE_HPP
#define MODULE_HPP

#include "Function.hpp"
#include "GlobalVariable.hpp"
#include "Instruction.hpp"
#include "Type.hpp"
#include "Value.hpp"

#include <list>
#include <map>
#include <memory>
#include <string>

class GlobalVariable;
class Function;
class Module {
  public:
    Module();
    ~Module() = default;

    Type *get_void_type();
    Type *get_label_type();
    IntegerType *get_int1_type();
    IntegerType *get_int32_type();
    PointerType *get_int32_ptr_type();
    FloatType *get_float_type();
    PointerType *get_float_ptr_type();

    PointerType *get_pointer_type(Type *contained);
    ArrayType *get_array_type(Type *contained, unsigned num_elements);
    FunctionType *get_function_type(Type *retty, std::vector<Type *> &args);

    void add_function(Function *f);
    std::list<Function*>& get_functions();
    void add_global_variable(GlobalVariable *g);
    std::list<GlobalVariable*>& get_global_variable();

    void set_print_name();
    std::string print();

  private:
    // The global variables in the module
    std::list<GlobalVariable*> global_list_;
    // The functions in the module
    std::list<Function*> function_list_;

    std::unique_ptr<IntegerType> int1_ty_;
    std::unique_ptr<IntegerType> int32_ty_;
    std::unique_ptr<Type> label_ty_;
    std::unique_ptr<Type> void_ty_;
    std::unique_ptr<FloatType> float32_ty_;
    std::map<Type *, std::unique_ptr<PointerType>> pointer_map_;
    std::map<std::pair<Type *, int>, std::unique_ptr<ArrayType>> array_map_;
    std::map<std::pair<Type *, std::vector<Type *>>,
             std::unique_ptr<FunctionType>>
        function_map_;
};

#endif