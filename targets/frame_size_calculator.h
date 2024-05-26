#ifndef __TIL_TARGETS_FRAME_SIZE_CALCULATOR_H__
#define __TIL_TARGETS_FRAME_SIZE_CALCULATOR_H__

#include "targets/basic_ast_visitor.h"

namespace til {

    class frame_size_calculator: public basic_ast_visitor {
        cdk::symbol_table<til::symbol> &_symtab;
        size_t _localsize;
    
    public:
        frame_size_calculator(std::shared_ptr<cdk::compiler> compiler, cdk::symbol_table<til::symbol> &symtab) :
            basic_ast_visitor(compiler), _symtab(symtab), _localsize(0) {
        }
    
    public:
        ~frame_size_calculator() {
            os().flush();
        }
    
    public:
        inline size_t localsize() {
            return _localsize;
        }
    
    public:
    // do not edit these lines
#define __IN_VISITOR_HEADER__
#include ".auto/visitor_decls.h"      // automatically generated
#undef __IN_VISITOR_HEADER__
    // do not edit these lines: end

    };

} // til

#endif

