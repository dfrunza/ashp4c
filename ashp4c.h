#pragma once
#include "foundation.h"
#include "frontend.h"
#include "ast_visitor.h"

struct DryPass : AstVisitor {
  void do_pass(Ast* ast);
};
