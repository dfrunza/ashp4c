#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal Hashmap concrete_type = {};
internal Hashmap* potential_types;

internal void visit_block_statement(Ast* block_stmt);
internal void visit_statement(Ast* decl);
internal void visit_expression(Ast* expr);
internal void visit_type_ref(Ast* type_ref);

internal void
visit_control(Ast* ast)
{

}

internal void
visit_control_proto(Ast* ast)
{

}

internal void
visit_extern(Ast* ast)
{

}

internal void
visit_struct(Ast* ast)
{

}

internal void
visit_header(Ast* ast)
{

}

internal void
visit_header_union(Ast* ast)
{

}

internal void
visit_package(Ast* ast)
{

}

internal void
visit_parser(Ast* ast)
{

}

internal void
visit_parser_proto(Ast* ast)
{

}

internal void
visit_instantiation(Ast* ast)
{

}

internal void
visit_type(Ast* ast)
{

}

internal void
visit_const(Ast* ast)
{

}

internal void
visit_function(Ast* ast)
{

}

internal void
visit_function_proto(Ast* ast)
{

}

internal void
visit_action(Ast* ast)
{

}

internal void
visit_enum(Ast* ast)
{

}

internal void
visit_match_kind(Ast* ast)
{

}

internal void
visit_error(Ast* ast)
{

}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_NodeList* decl_list = &program->decl_list;
  DList* li = decl_list->list.next;
  while (li) {
    Ast* decl = li->object;
    if (decl->kind == AST_CONTROL) {
      visit_control(decl);
    } else if (decl->kind == AST_CONTROL_PROTO) {
      visit_control_proto(decl);
    } else if (decl->kind == AST_EXTERN) {
      visit_extern(decl);
    } else if (decl->kind == AST_STRUCT) {
      visit_struct(decl);
    } else if (decl->kind == AST_HEADER) {
      visit_header(decl);
    } else if (decl->kind == AST_HEADER_UNION) {
      visit_header_union(decl);
    } else if (decl->kind == AST_PACKAGE) {
      visit_package(decl);
    } else if (decl->kind == AST_PARSER) {
      visit_parser(decl);
    } else if (decl->kind == AST_PARSER_PROTO) {
      visit_parser_proto(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      visit_instantiation(decl);
    } else if (decl->kind == AST_TYPE) {
      visit_type(decl);
    } else if (decl->kind == AST_CONST) {
      visit_const(decl);
    } else if (decl->kind == AST_FUNCTION) {
      visit_function(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      visit_function_proto(decl);
    } else if (decl->kind == AST_ACTION) {
      visit_action(decl);
    } else if (decl->kind == AST_ENUM) {
      visit_enum(decl);
    } else if (decl->kind == AST_MATCH_KIND) {
      visit_match_kind(decl);
    } else if (decl->kind == AST_ERROR_ENUM) {
      visit_error(decl);
    } else assert(0);
    li = li->next;
  }
}

void
select_type(Ast_P4Program* p4program, Hashmap* potential_types_)
{
  potential_types = potential_types_;
  visit_p4program((Ast*)p4program);
}
