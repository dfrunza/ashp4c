#include "arena.h"
#include "ast.h"
#include "symtable.h"


void
object_descriptor_name_resolve(struct Ast* ast)
{
  assert(ast->kind == Ast_P4Program);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;

    link = link->next;
  }
}
