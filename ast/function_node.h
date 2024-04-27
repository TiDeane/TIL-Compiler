#ifndef __TIL_AST_FUNCTION_NODE_H__
#define __TIL_AST_FUNCTION_NODE_H__


#include <memory>
#include <vector>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/sequence_node.h>
#include <cdk/ast/typed_node.h>
#include <cdk/types/basic_type.h>
#include <cdk/types/functional_type.h>
#include <cdk/types/primitive_type.h>
#include <cdk/types/typename_type.h>
//#include "block_node.h"
//* TODO: Should we use the block node instead of declarations, instructions ?

namespace til {

    /**
     * Class for describing function nodes.
     */
    class function_node: public cdk::expression_node {
        cdk::sequence_node *_args;
        cdk::sequence_node *_declarations, *_instructions;
        bool _is_main;

    public:
        inline function_node(int lineno,
            cdk::sequence_node *args,
            std::shared_ptr<cdk::basic_type> return_type,
            cdk::sequence_node *declarations,
            cdk::sequence_node *instructions,
            bool is_main = false) : 

            cdk::expression_node(lineno), _args(args), _declarations(declarations), 
            _instructions(instructions), _is_main(is_main) {
                std::vector<std::shared_ptr<cdk::basic_type>> arg_types;
                for (size_t i = 0; i < args->size(); i++) {
                    arg_types.push_back(dynamic_cast<cdk::typed_node*>(args->node(i))->type());
                }

                this->type(cdk::functional_type::create(arg_types, return_type));

        }
        
        /** Constructor for the main function. */
        inline function_node(int lineno, cdk::sequence_node *declarations,
            cdk::sequence_node *instructions) :
            cdk::expression_node(lineno), _args(new cdk::sequence_node(lineno)), _declarations(declarations), _instructions(instructions), _is_main(true) {
            this->type(cdk::functional_type::create(cdk::primitive_type::create(4, cdk::TYPE_INT)));
        }        

    public:
        inline cdk::sequence_node *args() {
            return _args;
        }        
        inline cdk::sequence_node *declarations() {
            return _declarations;
        }
        inline cdk::sequence_node *instructions() {
            return _instructions;
        }
        inline bool is_main() {
            return _is_main;
        }
        void accept(basic_ast_visitor *sp, int level) {
            sp->do_function_node(this, level);
        }

    }; 
} // til

#endif