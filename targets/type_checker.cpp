#include <string>
#include "targets/type_checker.h"
#include ".auto/all_nodes.h"  // automatically generated
#include <cdk/types/primitive_type.h>

#include "til_parser.tab.h"

#define ASSERT_UNSPEC { if (node->type() != nullptr && !node->is_typed(cdk::TYPE_UNSPEC)) return; }

//---------------------------------------------------------------------------

bool til::type_checker::deepTypeComparison(std::shared_ptr<cdk::basic_type> left, 
        std::shared_ptr<cdk::basic_type> right, bool allowCovariant) {
  if(left->name() == cdk::TYPE_UNSPEC || right->name() == cdk::TYPE_UNSPEC) {
    return false;
  } else if (left->name() == cdk::TYPE_FUNCTIONAL) {
    if (right->name() == cdk::TYPE_FUNCTIONAL) {
      return false;
    }

    auto left_function = cdk::functional_type::cast(left);
    auto right_function = cdk::functional_type::cast(right);

    if(left_function->input_length() != right_function->input_length()
     || left_function->output_length() != right_function->output_length()) {
     
     return false;
    }

    for(size_t i = 0; i < left_function->input_length(); i++) {
      if(!deepTypeComparison(right_function->input(i), left_function->input(i), allowCovariant)) {
        return false;
      }
    }

    for(size_t i = 0; i < left_function->output_length(); i++) {
      if(!deepTypeComparison(left_function->output(i), right_function->output(i), allowCovariant)) {
        return false;
      }
    }

    return true;
  } else if (right->name() == cdk::TYPE_FUNCTIONAL) {
    return false;
  } else if (left->name() == cdk::TYPE_POINTER) {
    if (right->name() != cdk::TYPE_POINTER) {
      return false;
    }

    auto left_pointer = cdk::reference_type::cast(left);
    auto right_pointer = cdk::reference_type::cast(right);

    return deepTypeComparison(left_pointer->referenced(), right_pointer->referenced(), false);
  } else if (right->name() == cdk::TYPE_POINTER) {
    return false;
  } else if (allowCovariant && left->name() == cdk::TYPE_DOUBLE) {
    return right->name() == cdk::TYPE_DOUBLE || right->name() == cdk::TYPE_INT;
  } else {
    return left == right;
  }
}

void til::type_checker::do_sequence_node(cdk::sequence_node *const node, int lvl) {
  for (size_t i = 0; i < node->size(); i++) {
    node->node(i)->accept(this, lvl);
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_nil_node(cdk::nil_node *const node, int lvl) {
  // EMPTY
}
void til::type_checker::do_data_node(cdk::data_node *const node, int lvl) {
  // EMPTY
}

//---------------------------------------------------------------------------

void til::type_checker::do_integer_node(cdk::integer_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_double_node(cdk::double_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
}

void til::type_checker::do_string_node(cdk::string_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(4, cdk::TYPE_STRING));
}

//---------------------------------------------------------------------------

void til::type_checker::processUnaryExpression(cdk::unary_operation_node *const node, int lvl, bool acceptDoubles) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl + 2);
  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->argument()->is_typed(cdk::TYPE_INT) && !(acceptDoubles && node->argument()->is_typed(cdk::TYPE_DOUBLE))) {
    throw std::string("wrong type in argument of unary expression");
  } 

  node->type(node->argument()->type());
}

void til::type_checker::do_unary_minus_node(cdk::unary_minus_node *const node, int lvl) {
  processUnaryExpression(node, lvl, true);
}

void til::type_checker::do_unary_plus_node(cdk::unary_plus_node *const node, int lvl) {
  processUnaryExpression(node, lvl, true);
}
void til::type_checker::do_not_node(cdk::not_node *const node, int lvl) {
  processUnaryExpression(node, lvl, false);
}

//---------------------------------------------------------------------------

/*
 * Type checks binary arithmetic expressions. The parent node will be typed
 * as follows (commutative):
 *    int, int -> int
 *    int, double -> double
 *    int, pointer -> pointer
 *    double, double -> double
 *    pointer, pointer -> int (both pointers must reference the same type)
 */
void til::type_checker::processBinaryArithmeticExpression(cdk::binary_operation_node *const node, int lvl,
      bool acceptDoubles, bool acceptOnePointer, bool acceptTwoPointers) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl + 2);

  if(node->left()->is_typed(cdk::TYPE_INT) || node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    node->right()->accept(this, lvl + 2);

    if(node->right()->is_typed(cdk::TYPE_INT) || (acceptDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      node->type(node->right()->type());
    } else if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (acceptOnePointer && node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->type(node->right()->type());

      if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
        node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      }
    } else {
      throw std::string("wrong type in right argument of arithmetic binary expression");
    }
    
    if(node->left()->is_typed(cdk::TYPE_UNSPEC)) {
      node->left()->type(node->type());
    }
  } else if (acceptDoubles && node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    node->right()->accept(this, lvl + 2);

    if (node->right()->is_typed(cdk::TYPE_INT) || node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      node->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
    } else if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      node->type(node->left()->type());
    } else {
      throw std::string("wrong type in right argument of arithmetic binary expression");
    }
  } else if (acceptOnePointer && node->left()->is_typed(cdk::TYPE_POINTER)) {
    node->right()->accept(this, lvl + 2);

    if (node->right()->is_typed(cdk::TYPE_INT)) {
      node->type(node->left()->type());
    } else if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->type(node->left()->type());
    } else if (acceptTwoPointers && deepTypeComparison(node->left()->type(), node->right()->type(), false)) {
      node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else {
      throw std::string("wrong type in right argument of arithmetic binary expression");
    } 
  } else {
      throw std::string("wrong type in left argument of arithmetic binary expression");
  }
}

void til::type_checker::do_add_node(cdk::add_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, true, false);
}
void til::type_checker::do_sub_node(cdk::sub_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, true, true);
}
void til::type_checker::do_mul_node(cdk::mul_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, false, false);
}
void til::type_checker::do_div_node(cdk::div_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, true, false, false);
}
void til::type_checker::do_mod_node(cdk::mod_node *const node, int lvl) {
  processBinaryArithmeticExpression(node, lvl, false, false, false);
}

/*
 * Type checks the components of each predicate expression (return is always int)
 * Note: pointers are never accepted
 */
void til::type_checker::processBinaryPredicateExpression(cdk::binary_operation_node *const node, int lvl, bool acceptDoubles) {
  ASSERT_UNSPEC;

  node->left()->accept(this, lvl + 2);

  if (node->left()->is_typed(cdk::TYPE_INT)) {
    node->right()->accept(this, lvl + 2);
    
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
    } else if (!node->right()->is_typed(cdk::TYPE_INT) 
        && !(acceptDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      throw std::string("wrong type in right argument of predicate binary expression");
    }  
  } else if (acceptDoubles && node->left()->is_typed(cdk::TYPE_DOUBLE)) {
    node->right()->accept(this, lvl + 2);
    
    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->right()->type(node->left()->type());
    } else if (!node->right()->is_typed(cdk::TYPE_INT) && !node->right()->is_typed(cdk::TYPE_DOUBLE)) {
      throw std::string("wrong type in right argument of predicate binary expression");
    }
  } else if (node->left()->is_typed(cdk::TYPE_UNSPEC)) {
    node->right()->accept(this, lvl + 2);

    if (node->right()->is_typed(cdk::TYPE_UNSPEC)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      node->right()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->right()->is_typed(cdk::TYPE_POINTER)) {
      node->left()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->left()->is_typed(cdk::TYPE_INT) || (acceptDoubles && node->right()->is_typed(cdk::TYPE_DOUBLE))) {
      node->left()->type(node->left()->type());
    } else {
      throw std::string("wrong type in right argument of predicate binary expression");
    }
  } else {
    throw std::string("wrong type in left argument of predicate binary expression");
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

void til::type_checker::do_lt_node(cdk::lt_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_le_node(cdk::le_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_ge_node(cdk::ge_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_gt_node(cdk::gt_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_ne_node(cdk::ne_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_eq_node(cdk::eq_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_and_node(cdk::and_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}
void til::type_checker::do_or_node(cdk::or_node *const node, int lvl) {
  processBinaryPredicateExpression(node, lvl, true);
}

//---------------------------------------------------------------------------

void til::type_checker::do_variable_node(cdk::variable_node *const node, int lvl) {
  ASSERT_UNSPEC;
  const std::string &id = node->name();
  std::shared_ptr<til::symbol> symbol = _symtab.find(id);

  if (symbol != nullptr) {
    node->type(symbol->type());
  } else {
    throw id;
  }
}

void til::type_checker::do_rvalue_node(cdk::rvalue_node *const node, int lvl) {
  ASSERT_UNSPEC;
  try {
    node->lvalue()->accept(this, lvl);
    node->type(node->lvalue()->type());
  } catch (const std::string &id) {
    throw "undeclared variable '" + id + "'";
  }
}

void til::type_checker::do_assignment_node(cdk::assignment_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl);
  node->rvalue()->accept(this, lvl);

  if (node->rvalue()->is_typed(cdk::TYPE_UNSPEC)) {
    node->rvalue()->type(node->lvalue()->type());
  } else if (node->rvalue()->is_typed(cdk::TYPE_POINTER) && node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    auto lref = cdk::reference_type::cast(node->lvalue()->type());
    auto rref = cdk::reference_type::cast(node->rvalue()->type());

    if (rref->referenced()->name() == cdk::TYPE_UNSPEC ||
        rref->referenced()->name() == cdk::TYPE_VOID ||
        lref->referenced()->name() == cdk::TYPE_VOID) {
      node->rvalue()->type(node->lvalue()->type());
    }
  }

 if (!deepTypeComparison(node->lvalue()->type(), node->rvalue()->type(), true)) {
    throw std::string("wrong type in right argument of assignment expression");
 }

  node->type(node->lvalue()->type());
}

//---------------------------------------------------------------------------

void til::type_checker::do_evaluation_node(til::evaluation_node *const node, int lvl) {
  node->argument()->accept(this, lvl + 2);
}

void til::type_checker::do_print_node(til::print_node *const node, int lvl) {
  node->arguments()->accept(this, lvl + 2);
}

//---------------------------------------------------------------------------

void til::type_checker::do_read_node(til::read_node *const node, int lvl) {
  ASSERT_UNSPEC;
  node->type(cdk::primitive_type::create(0, cdk::TYPE_UNSPEC));
}

//---------------------------------------------------------------------------

void til::type_checker::do_if_node(til::if_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void til::type_checker::do_if_else_node(til::if_else_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

//---------------------------------------------------------------------------

void til::type_checker::do_alloc_node(til::alloc_node *const node, int lvl) {
  ASSERT_UNSPEC;
  
  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->argument()->is_typed(cdk::TYPE_INT)) {
    throw std::string("wrong type in argument of unary expression");
  }

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

void til::type_checker::do_address_of_node(til::address_of_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->lvalue()->accept(this, lvl + 2);
  if (node->lvalue()->is_typed(cdk::TYPE_POINTER)) {
    auto ref = cdk::reference_type::cast(node->lvalue()->type());
    if (ref->referenced()->name() == cdk::TYPE_VOID) {
      node->type(node->lvalue()->type());
      return;
    }
  }
  node->type(cdk::reference_type::create(4, node->lvalue()->type()));
}

void til::type_checker::do_index_node(til::index_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->pointer()->accept(this, lvl + 2);
  if (node->index()->is_typed(cdk::TYPE_UNSPEC)) {
    node->index()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  } else if (!node->index()->is_typed(cdk::TYPE_INT)) {
    throw "wrong type in pointer index (expected integer)";
  }

  auto pointerType = cdk::reference_type::cast(node->pointer()->type());

  if (pointerType->referenced()->name() == cdk::TYPE_UNSPEC) {
    pointerType = cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT));
    node->pointer()->type(pointerType);
  }

  node->type(pointerType->referenced());
}

void til::type_checker::do_nullptr_node(til::nullptr_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->type(cdk::reference_type::create(4, cdk::primitive_type::create(0, cdk::TYPE_UNSPEC)));
}

void til::type_checker::do_sizeof_node(til::sizeof_node *const node, int lvl) {
  ASSERT_UNSPEC;

  node->argument()->accept(this, lvl + 2);

  if (node->argument()->is_typed(cdk::TYPE_UNSPEC)) {
    node->argument()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
  }

  node->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
}

//---------------------------------------------------------------------------

void til::type_checker::do_block_node(til::block_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_declaration_node(til::declaration_node *const node, int lvl) {
  if (node->type() == nullptr) { // var
    node->initializer()->accept(this, lvl + 2);

    if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) {
      node->initializer()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
    } else if (node->initializer()->is_typed(cdk::TYPE_POINTER)) {
      auto ref = cdk::reference_type::cast(node->initializer()->type());
      if (ref->referenced()->name() == cdk::TYPE_UNSPEC) {
        node->initializer()->type(cdk::reference_type::create(4, cdk::primitive_type::create(4, cdk::TYPE_INT)));
      }
    } else if (node->initializer()->is_typed(cdk::TYPE_VOID)) {
      throw std::string("cannot clare variable of type void");
    }

    node->type(node->initializer()->type());
  } else { // nott auto; node already has a type set
    if (node->initializer() != nullptr) {
      node->initializer()->accept(this, lvl + 2);

      if (node->initializer()->is_typed(cdk::TYPE_UNSPEC)) {
        if (node->is_typed(cdk::TYPE_DOUBLE)) {
          node->initializer()->type(node->type());
        } else {
          node->initializer()->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
        }
      } else if (node->initializer()->is_typed(cdk::TYPE_POINTER) && node->is_typed(cdk::TYPE_POINTER)) {
        auto node_ref = cdk::reference_type::cast(node->type());
        auto init_ref = cdk::reference_type::cast(node->initializer()->type());
        if (init_ref->referenced()->name() == cdk::TYPE_UNSPEC
              || init_ref->referenced()->name() == cdk::TYPE_VOID
              || node_ref->referenced()->name() == cdk::TYPE_VOID) {
          node->initializer()->type(node->type());
        }
      }

      if (!deepTypeComparison(node->type(), node->initializer()->type(), true)) {
        throw std::string("wrong type in initializer for variable '" + node->identifier() + "'");
      }
    }
  }
  
  if(node->qualifier() == tEXTERNAL && !node->is_typed(cdk::TYPE_FUNCTIONAL)) {
    throw std::string("external declaration of non-function '" + node->identifier() + "'");
  
  }
  
  auto symbol = make_symbol(node->type(), node->identifier(), node->qualifier());
  
  if(_symtab.insert(node->identifier(), symbol)) {
    _parent->set_new_symbol(symbol);
    return;
  }

  auto prev = _symtab.find(node->identifier());

  if(prev != nullptr && prev->qualifier() == tFORWARD) {
    if(deepTypeComparison(prev->type(), symbol->type(), false)) {
     _symtab.replace(node->identifier(), symbol);
     _parent->set_new_symbol(symbol);
     return;
    } 
  }  
  
  throw std::string("redeclaration of variable '" + node->identifier() + "'");
}

void til::type_checker::do_function_node(til::function_node *const node, int lvl) {
  auto function = til::make_symbol(node->type(), "@");
  function->is_main(node->is_main());

  if (!_symtab.insert(function->name(), function)) {
    _symtab.replace(function->name(), function);
  }
}

void til::type_checker::do_function_call_node(til::function_call_node *const node, int lvl) {
  ASSERT_UNSPEC;

  std::shared_ptr<cdk::functional_type> functype;

  if(node->func() == nullptr) {
    auto symbol = _symtab.find("@", 1);
    if(symbol == nullptr) {
      throw std::string("recursive call outside of function");
    } else if (symbol->is_main()) {
      throw std::string("recursive call inside begin end block");
    }
    
    functype = cdk::functional_type::cast(symbol->type());
  } else {
    node->func()->accept(this, lvl);
    
    if (!node->func()->is_typed(cdk::TYPE_FUNCTIONAL)) {
      throw std::string("wrong type in function call");
    }

    functype = cdk::functional_type::cast(node->func()->type());
  }
  
  
  if (functype->input()->length() != node->args()->size()) {
    throw std::string("wrong number of arguments in functionc call");
  }

  for (size_t i = 0; i < node->args()->size(); i++) {
    auto arg = dynamic_cast<cdk::expression_node*>(node->args()->node(i));
    arg->accept(this, lvl);

    auto paramtype = functype->input(i);

    if (arg->is_typed(cdk::TYPE_UNSPEC)) {
      if (paramtype->name() == cdk::TYPE_DOUBLE) {
        arg->type(cdk::primitive_type::create(8, cdk::TYPE_DOUBLE));
      } else {
        arg->type(cdk::primitive_type::create(4, cdk::TYPE_INT));
      }
    } else if (arg->is_typed(cdk::TYPE_POINTER) && paramtype->name() == cdk::TYPE_POINTER) {
      auto paramref = cdk::reference_type::cast(paramtype);
      auto argref = cdk::reference_type::cast(arg->type());

      if (argref->referenced()->name() == cdk::TYPE_UNSPEC
            || argref->referenced()->name() == cdk::TYPE_VOID
            || paramref->referenced()->name() == cdk::TYPE_VOID) {
          arg->type(paramtype);
      }
    }

    if (!deepTypeComparison(paramtype, arg->type(), true)) {
      throw "wrong type for argument " + std::to_string(i + 1) + " in function call";
    }
  }

  node->type(functype->output(0));
}

void til::type_checker::do_return_node(til::return_node *const node, int lvl) {
  auto symbol =_symtab.find("@", 1);
  if(symbol == nullptr) {
    throw std::string("return statement outside begin end block");
  }  

  std::shared_ptr<cdk::functional_type> function_type = cdk::functional_type::cast(symbol->type());

  auto rettype = function_type->output(0);
  auto rettype_name = rettype->name();
  
  if (node->retValue() == nullptr) {
    if (rettype_name != cdk::TYPE_VOID) {
      throw std::string("no return value specified for non-void function");
    }
    return;
  }

  if (rettype_name == cdk::TYPE_VOID) {
    throw std::string("return value specified for void function");
  }
  
  node->retValue()->accept(this, lvl + 2);

  if (!deepTypeComparison(rettype, node->retValue()->type(), true)) {
    throw std::string("wrong type for return expression");
  }
}

//---------------------------------------------------------------------------

void til::type_checker::do_loop_node(til::loop_node *const node, int lvl) {
  node->condition()->accept(this, lvl + 4);
}

void til::type_checker::do_next_node(til::next_node *const node, int lvl) {
  // EMPTY
}

void til::type_checker::do_stop_node(til::stop_node *const node, int lvl) {
  // EMPTY
}