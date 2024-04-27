#ifndef __TIL_AST_RETURN_NODE_H__
#define __TIL_AST_RETURN_NODE_H__

#include <cdk/ast/basic_node.h>
#include <cdk/ast/expression_node.h>

namespace til {

  /**
   * Class for describing return nodes.
   */
  class return_node: public cdk::basic_node {
    cdk::expression_node *_retValue;

  public:
    return_node(int lineno, cdk::expression_node *revValue) :
        cdk::basic_node(lineno), _retValue(revValue) {
    }

    cdk::expression_node *retValue() { return _retValue; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_return_node(this, level);
    }

  };

} // til

#endif