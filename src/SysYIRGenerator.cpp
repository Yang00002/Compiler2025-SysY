#include "../include/SysYIRGenerator.hpp"
#include <Token.h>
#include <any>
#include <memory>
#include <vector>

#define ty_void (module->get_void_type())
#define ty_bool (module->get_int1_type())
#define ty_i32  (module->get_int32_type())
#define ty_f32  (module->get_float_type())
#define ty_i32p (module->get_int32_ptr_type())
#define ty_f32p (module->get_float_ptr_type())
#define const_int(num) (ConstantInt::get((num), module.get()))
#define const_fp(num)  (ConstantFP::get((float)(num), module.get()))

std::any SysYIRGenerator::visitBType(SysYParser::BTypeContext *ctx) 
{
    if(ctx->INT())
        return ty_i32;
    else if(ctx->FLOAT())
        return ty_f32;
    return nullptr;
}

std::any SysYIRGenerator::visitFuncType(SysYParser::FuncTypeContext *ctx)
{
    if(ctx->VOID())
        return ty_void;
    else if(ctx->FLOAT())
        return ty_f32;
    else if(ctx->INT())
        return ty_i32;
    return nullptr;
}

std::any SysYIRGenerator::visitCompUnit(SysYParser::CompUnitContext* ctx)
{
    auto ThisCompUnit = new Module();
    module.reset(ThisCompUnit);
    scope.enter();
 
    // TODO: sysy库函数
    auto _sylib_getint = Function::create(
        FunctionType::get(ty_i32,std::vector<Type*>()), 
        "getint", module.get());
    auto _sylib_getch = Function::create(
        FunctionType::get(ty_i32,std::vector<Type*>()), 
        "getch", module.get());
    auto _sylib_getarray = Function::create(
        FunctionType::get(ty_i32,std::vector<Type*>({ty_i32p})), 
        "getarray", module.get());
    auto _sylib_getfloat = Function::create(
        FunctionType::get(ty_f32,std::vector<Type*>()), 
        "getfloat", module.get());
    auto _sylib_getfarray = Function::create(
        FunctionType::get(ty_i32,std::vector<Type*>({ty_f32p})), 
        "getfarray", module.get());
        
    auto _sylib_putint = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32})), 
        "putint", module.get());
    auto _sylib_putch = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32})), 
        "putch", module.get());
    auto _sylib_putarray = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32,ty_i32p})), 
        "putarray", module.get());
    auto _sylib_putfloat = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_f32})), 
        "putfloat", module.get());
    auto _sylib_putfarray = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32,ty_f32p})), 
        "putfarray", module.get());

    auto _sylib_sysy_starttime = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32})),
        "_sysy_starttime", module.get()
    );
    auto _sylib_sysy_stoptime = Function::create(
        FunctionType::get(ty_void,std::vector<Type*>({ty_i32})),
        "_sysy_stoptime", module.get()
    );
    
    scope.add_symbol("getint",_sylib_getint);
    scope.add_symbol("getch",_sylib_getch);
    scope.add_symbol("getarray",_sylib_getarray);
    scope.add_symbol("getfloat",_sylib_getfloat);
    scope.add_symbol("getfarray",_sylib_getfarray);
    scope.add_symbol("putint",_sylib_putint);
    scope.add_symbol("putch",_sylib_putch);
    scope.add_symbol("putarray",_sylib_putarray);
    scope.add_symbol("putfloat",_sylib_putfloat);
    scope.add_symbol("putfarray",_sylib_putfarray);
    scope.add_symbol("_sysy_starttime",_sylib_sysy_starttime);
    scope.add_symbol("_sysy_stoptime",_sylib_sysy_stoptime);

    for(auto decl:ctx->decl())
        visitDecl(decl);
    for(auto def:ctx->funcDef())
        visitFuncDef(def);
    
    return ThisCompUnit;
}

std::any SysYIRGenerator::visitInitVal(SysYParser::InitValContext* ctx) 
{
    if(ctx->exp())
        visitExp(ctx->exp());
    return nullptr;
}

std::any SysYIRGenerator::visitBlock(SysYParser::BlockContext* ctx) 
{
    scope.enter();
    std::vector<Value*> block_values;
    int num_blockitems = ctx->blockItem().size();
    for(int i=0;i<num_blockitems;++i)
        block_values.emplace_back(std::any_cast<Value*>(visitBlockItem(ctx->blockItem(i))));    
    scope.exit();
    return block_values;
}

std::any SysYIRGenerator::visitBlockItem(SysYParser::BlockItemContext* ctx) 
{
    if(ctx->decl())
        return visitDecl(ctx->decl());
    else
        return visitStmt(ctx->stmt());
}

std::any SysYIRGenerator::visitCond(SysYParser::CondContext* ctx) 
{
    auto orval = std::any_cast<Value*>(visitLOrExp(ctx->lOrExp()));
    Value* expval = nullptr;
    if(!orval)
        return expval;
    if(orval->get_type()==ty_i32||orval->get_type()==ty_bool){
        if(auto c_val = dynamic_cast<ConstantInt*>(orval)){
            bool cond_exp_val = c_val->get_value();
            if(cond_exp_val)
                expval = builder->create_br(true_targets.back());
            else
                expval = builder->create_br(false_targets.back());
        }else{
            Value* cmp_val = builder->create_icmp_ne(orval,const_int(0));
            expval = builder->create_cond_br(cmp_val,true_targets.back(),false_targets.back());
        }
    }else if(orval->get_type()==ty_f32){
        if(auto c_val = dynamic_cast<ConstantFP*>(orval)){
            bool cond_exp_val = c_val->get_value();
            if(cond_exp_val)
                expval = builder->create_br(true_targets.back());
            else
                expval = builder->create_br(false_targets.back());
        }else{
            Value* cmp_val = builder->create_fcmp_ne(orval,const_fp(0.0));
            expval = builder->create_cond_br(cmp_val,true_targets.back(),false_targets.back());
        }
    }
	return nullptr;
}

std::any SysYIRGenerator::visitPrimaryExp(SysYParser::PrimaryExpContext* ctx) 
{
    if(ctx->LPAREN())
        return std::any_cast<Value*>(visitExp(ctx->exp()));
    else if(ctx->lVal())
        return std::any_cast<Value*>(visitLVal(ctx->lVal()));
    else if(ctx->number())
        return std::any_cast<Value*>(visitNumber(ctx->number()));
    return nullptr;
}

std::any SysYIRGenerator::visitNumber(SysYParser::NumberContext* ctx) 
{
    if(auto iptr = ctx->IntConst()){
        int value = 0;
        const auto& literal = iptr->getText();
        if( literal[0]=='0' && literal.size()>1 ){
            if ( literal[1]=='x' || literal[1]=='X' )
                value = std::stoi(literal,0,16);
            else
                value = std::stoi(literal,0,8);
        }else 
            value = std::stoi(literal);
        return const_int(value);
    }else if(auto fptr = ctx->FloatConst()){
        float value = std::stof(fptr->getText());
        return const_fp(value);
    }
    return nullptr;
}

std::any SysYIRGenerator::visitFuncRParams(SysYParser::FuncRParamsContext* ctx) 
{
    std::vector<Value*> args{};
    for(auto p:ctx->exp())
        args.emplace_back(std::any_cast<Value*>(visitExp(p)));
    return args;
}

std::any SysYIRGenerator::visitConstExp(SysYParser::ConstExpContext* ctx) 
{
    return std::any_cast<Value*>(visitAddExp(ctx->addExp()));
}

std::any SysYIRGenerator::visitExp(SysYParser::ExpContext* ctx) 
{
    return std::any_cast<Value*>(visitAddExp(ctx->addExp()));
}

std::any SysYIRGenerator::visitUnaryOp(SysYParser::UnaryOpContext* ctx) 
{
    return nullptr; // 没有什么可以访问的
}

std::any SysYIRGenerator::visitLAndExp(SysYParser::LAndExpContext* ctx) 
{
    int exp_num = ctx->eqExp().size();
    Value* result = nullptr;
    if(exp_num==1)
        return visitEqExp(ctx->eqExp(0));
    
    auto F = builder->get_insert_block()->get_parent();
    for(int i=0;i<exp_num;++i){
        Value* lhs = std::any_cast<Value*>(visitEqExp(ctx->eqExp(i)));
        Value* lhs_val = nullptr;
        if ( auto p = dynamic_cast<ConstantInt*>(lhs) ) {
            bool l_res = p->get_value();
            lhs_val = const_int(l_res);
        } else if(lhs->get_type()->is_int1_type()){
            lhs_val = lhs;
        } else if(lhs->get_type()->is_int32_type()){ 
            lhs_val = builder->create_icmp_ne(lhs,const_int(0));
        } else {    // 其他所有类型
            lhs_val = builder->create_fcmp_ne(lhs,const_fp(0.0));
        }
        if (i != exp_num-1) { // 如果lhs_val已经为假，直接去假出口
            BasicBlock* check_rhs = BasicBlock::create(module.get(),"",F);
            builder->create_cond_br(lhs_val,check_rhs,false_targets.back());
            builder->set_insert_point(check_rhs);
        } else {
            builder->create_cond_br(lhs_val, true_targets.back(), false_targets.back());
        }
    }
    return result;
}

std::any SysYIRGenerator::visitLOrExp(SysYParser::LOrExpContext* ctx) 
{
    int exp_num = ctx->lAndExp().size();
    Value* result = nullptr;
    if(exp_num==1)
        return visitLAndExp(ctx->lAndExp(0));
    
    auto F = builder->get_insert_block()->get_parent();
    for(int i=0;i<exp_num;++i){
        if (i != exp_num-1) {
            BasicBlock* check_rhs = BasicBlock::create(module.get(),"",F);
            false_targets.emplace_back(check_rhs);
            Value* lhs = std::any_cast<Value*>(visitLAndExp(ctx->lAndExp(i)));
            false_targets.pop_back();
            if(lhs){ 
                Value* lhs_val = nullptr;
                if ( auto p = dynamic_cast<ConstantInt*>(lhs) ) {
                    bool l_res = p->get_value();
                    lhs_val = const_int(l_res);
                } else if(lhs->get_type()->is_int1_type()){
                    lhs_val = lhs;
                } else if(lhs->get_type()->is_int32_type()){ 
                    lhs_val = builder->create_icmp_ne(lhs,const_int(0));
                } else {    // 其他所有类型
                    lhs_val = builder->create_fcmp_ne(lhs,const_fp(0.0));
                }
                // 如果lhs_val已经为真，直接去真出口
                builder->create_cond_br(lhs_val, true_targets.back(), check_rhs);
            }
            builder->set_insert_point(check_rhs);
        } else {
            Value* lhs = std::any_cast<Value*>(visitLAndExp(ctx->lAndExp(i)));
            if(lhs){
                if ( auto p = dynamic_cast<ConstantInt*>(lhs) ) {
                    bool l_res = p->get_value(); 
                    builder->create_br(l_res ? true_targets.back() : false_targets.back());
                } else {
                    Value* lhs_val = nullptr;
                    if(lhs->get_type()->is_int1_type())
                        lhs_val = lhs;
                    else if(lhs->get_type()->is_int32_type())
                        lhs_val = builder->create_icmp_ne(lhs,const_int(0));
                    else
                        lhs_val = builder->create_fcmp_ne(lhs,const_fp(0.0));
                    builder->create_cond_br(lhs_val, true_targets.back(), false_targets.back());
                } 
            }
        }
    }
    return result;
}

std::any SysYIRGenerator::visitMulExp(SysYParser::MulExpContext* ctx) 
{
    Value* result = nullptr;
    if(!ctx->mulExp())
        return visitUnaryExp(ctx->unaryExp());
    
    Value* lhs_res = std::any_cast<Value*>(visitMulExp(ctx->mulExp()));
    Value* rhs_res = std::any_cast<Value*>(visitUnaryExp(ctx->unaryExp()));

    // TODO: Int1需要扩展 | 确认所有的 create_sitofp 使用是否正确
    if( lhs_res->get_type()->is_integer_type() && rhs_res->get_type()->is_integer_type() ) {
        if (ctx->MUL()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                int val = lhs * rhs;
                result = const_int(val);
            } else{
                result = builder->create_imul(lhs_res, rhs_res);
            }
        } else if(ctx->DIV()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                float val = (float)lhs / rhs;
                if(lhs%rhs==0) 
                    result = const_int((int)val);
                else
                    result = const_fp(val);
            } else{
                result = builder->create_isdiv(lhs_res, rhs_res);
            }
        } else if(ctx->MOD()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                int val = lhs % rhs;
                result = const_int(val);
            } else{
                result = builder->create_srem(lhs_res, rhs_res);   // IR Builder中没有，需要实现，不一定对(考虑取余的语义)
            }
        }
    } else if( lhs_res->get_type()->is_float_type() && rhs_res->get_type()->is_integer_type() ) {
        if(ctx->MUL()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                float val = lhs * rhs;
                result = const_fp(val);
            } else {
                Value* float_r = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fmul(lhs_res,float_r);
            }
        } else if(ctx->DIV()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                float val = lhs / rhs;
                result = const_fp(val);
            } else {
                Value* float_r = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fdiv(lhs_res,float_r);
            }
        }else if(ctx->MOD()) {  // 显然不该出现
            std::cerr << "Invaild operand type for modulo.\n";
        }
    } else if( lhs_res->get_type()->is_integer_type() && rhs_res->get_type()->is_float_type() ) {	
        if(ctx->MUL()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs * rhs;
                result = const_fp(val);
            } else {
                Value* float_l = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fmul(float_l,rhs_res);
            }
        } else if(ctx->DIV()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs / rhs;
                result = const_fp(val);
            } else {
                Value* float_l = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fdiv(float_l,rhs_res);
            }
        } else if(ctx->MOD()) {
            std::cerr << "Invaild operand type for modulo.\n";
        }
    } else if( lhs_res->get_type()->is_float_type() && rhs_res->get_type()->is_float_type() ) {	
        if(ctx->MUL()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs * rhs;
                result = const_fp(val);
            } else {
                result = builder->create_fmul(lhs_res, rhs_res);
            }
        } else if(ctx->DIV()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs / rhs;
                result = const_fp(val);
            } else {
                result = builder->create_fdiv(lhs_res, rhs_res);
            }
        } else if(ctx->MOD()) {
            std::cerr << "Invaild operand type for modulo.\n";
        }
    }
    return result;
}

std::any SysYIRGenerator::visitAddExp(SysYParser::AddExpContext* ctx) 
{
    Value* result = nullptr;
    if(!ctx->addExp())
        return visitMulExp(ctx->mulExp());

    Value* lhs_res = std::any_cast<Value*>(visitAddExp(ctx->addExp()));
    Value* rhs_res = std::any_cast<Value*>(visitMulExp(ctx->mulExp()));

    // TODO: Int1需要扩展 | 确认所有的 create_sitofp 使用是否正确
    if( lhs_res->get_type()->is_integer_type() && rhs_res->get_type()->is_integer_type() ) {
        if (ctx->ADD()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                int val = lhs + rhs;
                result = const_int(val);
            } else{
                result = builder->create_iadd(lhs_res, rhs_res);
            }
        } else if(ctx->SUB()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                int val = lhs - rhs;
                result = const_int(val);
            } else{
                result = builder->create_isub(lhs_res, rhs_res);
            }
        } 
    } else if( lhs_res->get_type()->is_float_type() && rhs_res->get_type()->is_integer_type() ) {
        if(ctx->ADD()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                float val = lhs + rhs;
                result = const_fp(val);
            } else {
                Value* float_r = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fadd(lhs_res,float_r);
            }
        } else if(ctx->SUB()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantInt*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                int rhs = dynamic_cast<ConstantInt*>(rhs_res)->get_value();
                float val = lhs - rhs;
                result = const_fp(val);
            } else {
                Value* float_r = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fsub(lhs_res,float_r);
            }
        }
    } else if( lhs_res->get_type()->is_integer_type() && rhs_res->get_type()->is_float_type() ) {	
        if(ctx->ADD()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs + rhs;
                result = const_fp(val);
            } else {
                Value* float_l = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fadd(float_l,rhs_res);
            }
        } else if(ctx->SUB()) {
            if ( dynamic_cast<ConstantInt*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                int lhs = dynamic_cast<ConstantInt*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs - rhs;
                result = const_fp(val);
            } else {
                Value* float_l = builder->create_sitofp(rhs_res, ty_f32);
                result = builder->create_fsub(float_l,rhs_res);
            }
        }
    } else if( lhs_res->get_type()->is_float_type() && rhs_res->get_type()->is_float_type() ) {	
        if(ctx->ADD()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs + rhs;
                result = const_fp(val);
            } else {
                result = builder->create_fadd(lhs_res, rhs_res);
            }
        } else if(ctx->SUB()) {
            if ( dynamic_cast<ConstantFP*>(lhs_res) && dynamic_cast<ConstantFP*>(rhs_res) ){
                float lhs = dynamic_cast<ConstantFP*>(lhs_res)->get_value();
                float rhs = dynamic_cast<ConstantFP*>(rhs_res)->get_value();
                float val = lhs - rhs;
                result = const_fp(val);
            } else {
                result = builder->create_fsub(lhs_res, rhs_res);
            }
        }
    }
	return result;
}

std::any SysYIRGenerator::visitEqExp(SysYParser::EqExpContext* ctx) 
{
    auto first_expr_eq = dynamic_cast<SysYParser::EqExpContext*>(ctx->children[0]);
    auto first_expr_rel = dynamic_cast<SysYParser::RelExpContext*>(ctx->children[0]);
    if (first_expr_rel)
        return std::any_cast<Value*>(visitRelExp(first_expr_rel));
       
    Value* eq = std::any_cast<Value*>(visitEqExp(ctx->eqExp()));
    Value* rel = std::any_cast<Value*>(visitRelExp(ctx->relExp()));

    auto const_eq_p = dynamic_cast<Constant*>(eq);
    auto const_rel_p = dynamic_cast<Constant*>(rel);

    if ( const_eq_p && const_rel_p ) {
        float eq_val = 0.0f;
        float rel_val = 0.0f;
        if (eq->get_type()->is_integer_type())
            eq_val = dynamic_cast<ConstantInt*>(eq)->get_value();
        else 
            eq_val = dynamic_cast<ConstantFP*>(eq)->get_value();
        if (rel->get_type()->is_integer_type())
            rel_val = dynamic_cast<ConstantInt*>(rel)->get_value();
        else 
            rel_val = dynamic_cast<ConstantFP*>(rel)->get_value();
        if (ctx->EQ())
            return const_int(eq_val==rel_val);
        else if (ctx->NE())
            return const_int(eq_val!=rel_val);
        else
            std::cerr << "EqExp error: Invalid operator.\n";
    } else if(eq->get_type()->is_integer_type() && rel->get_type()->is_integer_type()) {
        return ctx->EQ() ? builder->create_icmp_eq(eq,rel) : builder->create_icmp_ne(eq,rel);
    } else if(eq->get_type()->is_float_type() && rel->get_type()->is_float_type()) {
        return ctx->EQ() ? builder->create_fcmp_eq(eq,rel) : builder->create_fcmp_ne(eq,rel);
    } else if(eq->get_type()->is_integer_type() && rel->get_type()->is_float_type()) {
        Value* eq_f = builder->create_sitofp(eq,ty_f32);
        return ctx->EQ() ? builder->create_fcmp_eq(eq_f,rel) : builder->create_fcmp_ne(eq_f,rel);
    } else if(eq->get_type()->is_float_type() && rel->get_type()->is_integer_type()) {
        Value* rel_f = builder->create_sitofp(rel,ty_f32);
        return ctx->EQ() ? builder->create_fcmp_eq(eq,rel_f) : builder->create_fcmp_ne(eq,rel_f);
    }
    return nullptr;
}

/*
 * TODO: 正在实现以下的部分。
 */

std::any SysYIRGenerator::visitRelExp(SysYParser::RelExpContext* ctx) 
{
    if(!ctx->relExp())
        return visitAddExp(ctx->addExp());
    // 类型转换和visitEqExp类似
    Value* result = nullptr;
    if(ctx->GE()){

    }else if(ctx->GT()){

    }else if(ctx->LT()){

    }else if(ctx->LE()){

    }
    return result;
}

std::any SysYIRGenerator::visitUnaryExp(SysYParser::UnaryExpContext* ctx) 
{
    if(auto pexp = ctx->primaryExp())
        return visitPrimaryExp(pexp);

    if(ctx->ID()){
        // 处理函数调用
        return nullptr;
    }
    
    if(auto uop=ctx->unaryOp()){
        return nullptr;
    }


    return nullptr;
}

std::any SysYIRGenerator::visitDecl(SysYParser::DeclContext* ctx)
{
    if(ctx->constDecl())
        return visitConstDecl(ctx->constDecl());
    else if(ctx->varDecl())
        return visitVarDecl(ctx->varDecl());
    return nullptr;
}

std::any SysYIRGenerator::visitLVal(SysYParser::LValContext* ctx) 
{
    auto id = ctx->ID()->getText();
    auto val = scope.find(id);
    int num_exps = ctx->exp().size();

    if(num_exps!=0){
        std::vector<Value*> indices{};
        auto entry_val = scope.find(val->get_name());
        auto ty_ = entry_val->get_type();
        if(ty_->is_pointer_type()){

        }else{

        }
    }else{
        auto entry_val = scope.find(val->get_name());
        
    }
}

std::any SysYIRGenerator::visitStmt(SysYParser::StmtContext* ctx) 
{
    Value* ret_value = nullptr;
    if(ctx->lVal()){
        auto name = ctx->lVal()->ID()->getText();

    }
    return nullptr;
}

std::any SysYIRGenerator::visitConstDecl(SysYParser::ConstDeclContext* ctx) 
{
    std::vector<Value*> values;
    Type* ty_ = std::any_cast<Type*>(visitBType(ctx->bType()));
    for(auto def:ctx->constDef()){
        std::vector<int> array_dims;
        int dims = def->constExp().size();
        Type* base = ty_;
        for(int i=dims-1;i>=0;--i){
            int n_elems = dynamic_cast<ConstantInt*>(
                std::any_cast<Value*>(visitConstExp(def->constExp(i)))
            )->get_value();
            array_dims.insert(array_dims.begin(),n_elems);
            ty_ = module->get_array_type(ty_,n_elems);
        }
        PointerType* ptr_ty_ = module->get_pointer_type(ty_);
        
        // alloca
        auto cur_block = builder->get_insert_block();
        auto entry_block = cur_block->get_parent()->get_entry_block();
        builder->set_insert_point(entry_block);
        auto alloca_inst = builder->create_alloca(ptr_ty_);
        builder->set_insert_point(cur_block);

        // store
    }
}

std::any SysYIRGenerator::visitConstDef(SysYParser::ConstDefContext* ctx) 
{
    
}

std::any SysYIRGenerator::visitConstInitVal(SysYParser::ConstInitValContext* ctx) 
{
    if(ctx->constExp())
        visitConstExp(ctx->constExp());
    return nullptr;
}

std::any SysYIRGenerator::visitVarDecl(SysYParser::VarDeclContext* ctx) 
{
    
}

std::any SysYIRGenerator::visitFuncFParam(SysYParser::FuncFParamContext* ctx) 
{
    return nullptr;
}

std::any SysYIRGenerator::visitVarDef(SysYParser::VarDefContext* ctx) 
{
    // TODO:
    return nullptr;
}

std::any SysYIRGenerator::visitFuncDef(SysYParser::FuncDefContext* ctx) 
{
    
}

std::any SysYIRGenerator::visitFuncFParams(SysYParser::FuncFParamsContext* ctx) 
{
    for(auto p:ctx->funcFParam())
        visitFuncFParam(p);
    return nullptr;
}