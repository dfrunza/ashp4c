#include "arena.h"
#include "ast.h"
#include "symtable.h"


void
objdesc_ast_program(struct Ast* ast)
{
  if (ast->kind == Ast_P4Program) {
    push_scope();
    pop_scope();
  }
}
