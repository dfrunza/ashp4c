#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal int node_id = 0;

void
visit_typeRef()
{

}

void visit_name()
{

}

void visit_expression()
{

}

void
visit_variableDeclaration(Ast_VarDeclaration* var_decl)
{
  var_decl->id = ++node_id;
  visit_typeRef(var_decl->type);
  visit_name(var_decl->name);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
}

void
visit_declaration(Ast_Declaration* decl)
{
  decl->id = ++node_id;
  Ast* ast = decl->decl;
  if (ast->kind == AST_variableDeclaration) {
    visit_variableDeclaration((Ast_VarDeclaration*)ast);
  } else if (ast->kind == AST_externDeclaration) {

  } else if (ast->kind == AST_actionDeclaration) {

  } else if (ast->kind == AST_functionDeclaration) {

  } else if (ast->kind == AST_parserDeclaration) {

  } else if (ast->kind == AST_parserTypeDeclaration) {

  } else if (ast->kind == AST_controlDeclaration) {

  } else if (ast->kind == AST_controlTypeDeclaration) {

  } else if (ast->kind == AST_typeDeclaration) {

  } else if (ast->kind == AST_errorDeclaration) {

  } else if (ast->kind == AST_matchKindDeclaration) {

  } else if (ast->kind == AST_instantiation) {

  }
  else assert(0);
}

void
visit_declarationList(Ast_List* decl_list)
{
  decl_list->id = ++node_id;
  for (ListItem* li = decl_list->members.sentinel.next;
        li != 0; li = li->next) {
    visit_declaration(li->object);
  }
}

void
visit_p4program(Ast_P4Program* p4program)
{
  p4program->id = ++node_id;
  visit_declarationList((Ast_List*)p4program->decl_list);
}

void
node_id_pass(Ast_P4Program* p4program)
{
  visit_p4program(p4program);
}

