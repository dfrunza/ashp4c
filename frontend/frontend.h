#pragma once

#include "frontend/ast.h"
#include "frontend/lexer.h"
#include "frontend/scope.h"

struct Frontend {
  Ast* p4program;
  Scope* root_scope;

  void do_analysis(Arena *storage, Arena *scratch, SourceText *source_text);
};