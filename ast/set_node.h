#ifndef __TIL_AST_SET_NODE_H__
#define __TIL_AST_SET_NODE_H__

#include <cdk/ast/assignment_node.h>
#include <cdk/ast/expression_node.h>
#include <cdk/ast/lvalue_node.h>

namespace til {

  /**
   * Class for describing set nodes.
   */
  class set_node: public cdk::assignment_node {
    cdk::lvalue_node *_lvalue;
    cdk::expression_node *_rvalue;

  public:
    set_node(int lineno, cdk::lvalue_node *lvalue, cdk::expression_node *rvalue) :
        cdk::assignment_node(lineno, lvalue, rvalue) {
    }

    cdk::lvalue_node *lvalue() { return _lvalue; }

    cdk::expression_node *rvalue() { return _rvalue; }

    void accept(basic_ast_visitor *sp, int level) {
        sp->do_set_node(this, level);
    }

  };

} // til

#endif