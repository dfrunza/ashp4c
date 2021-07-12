#include "arena.h"
#include "ast.h"
#include "symtable.h"


internal void
build_symtable_control(struct Ast* control)
{
  struct Ast* type_decl = (struct Ast*)ast_getattr(control, "type_decl");
  struct Ast* name = (struct Ast*)ast_getattr(type_decl, "name");
  char* strname = (char*)ast_getattr(name, "name");
  new_type(strname, name->line_nr);
}

void
build_symtable_program(struct Ast* ast)
{
  if (ast->kind == Ast_P4Program) {
    push_scope();
    struct AstList* decl_list = (struct AstList*)ast_getattr(ast, "decl_list");
    struct AstListLink* decl_link = ast_list_first_link(decl_list);
    while (decl_link) {
      if (decl_link->ast->kind == Ast_Control) {
        build_symtable_control(decl_link->ast);
      }
      decl_link = decl_link->next;
    }
    pop_scope();
  }
}
