#pragma once

#include "frontend/ast.h"
#include "midend/ast_visitor.h"

struct DryPass : AstVisitor {
  void do_pass(Ast* ast);
};
