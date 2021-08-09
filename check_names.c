#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void
check_names_top_level_instantiation(struct Scope* scope, struct Ast* instantiation)
{
  struct Ast* type_ref = ast_getattr(instantiation, "type_ref");
}

void
check_names_program(struct Ast* program)
{
  if (program->kind == Ast_P4Program) {
    struct Scope* scope = ast_getattr(program, "scope");
    struct List* decl_list = (struct List*)ast_getattr(program, "decl_list");
    struct ListLink* link = list_first_link(decl_list);
    while (link) {
      struct Ast* decl = link->object;
      if (decl->kind == Ast_Instantiation) {
        check_names_top_level_instantiation(scope, decl);
      }
      link = link->next;
    }
  }
  else assert (0);
}
