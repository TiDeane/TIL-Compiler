#ifndef __TIL_AST_NEXT_NODE_H__
#define __TIL_AST_NEXT_NODE_H__

#include <cdk/ast/basic_node.h>

namespace til {

    /**
     * Class for describing next nodes.
     */
    class next_node: public cdk::basic_node {
        int _nIterations;

    public:
        inline next_node(int lineno, int nIterations):
            cdk::basic_node(lineno), _nIterations(nIterations) {
        }

    public:
        inline int nIterations() {
            return _nIterations;
        }    

        void accept(basic_ast_visitor *sp, int level) {
            sp->do_next_node(this, level);
        }

    };

} // til

#endif
