#include "arena.h"
#include "ast.h"


internal void
nameref_context_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      //build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* link = list_first_link(control_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      //build_symtable_param(param);
      link = link->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* link = list_first_link(control_decl->local_decls);
    while (link) {
      struct Ast* decl = link->object;
      //build_symtable_statement(decl);
      link = link->next;
    }
  }
  if (control_decl->apply_stmt) {
    //build_symtable_block_statement(control_decl->apply_stmt);
  }
}

internal void
build_nameref_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == AST_CONTROL_DECL) {
      //link_parent_control_decl(decl);
    } else if (decl->kind == AST_EXTERN_DECL) {
      //link_parent_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT_DECL) {
      //link_parent_struct_decl(decl);
    } else if (decl->kind == AST_HEADER_DECL) {
      //link_parent_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION_DECL) {
      //link_parent_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE_DECL) {
      //link_parent_package(decl);
    } else if (decl->kind == AST_PARSER_DECL) {
      //link_parent_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      //link_parent_instantiation(decl);
    } else if (decl->kind == AST_TYPE_DECL) {
      //link_parent_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      //link_parent_function_proto(decl);
    } else if (decl->kind == AST_CONST_DECL) {
      //link_parent_const_decl(decl);
    } else if (decl->kind == AST_ENUM_DECL) {
      //link_parent_enum_decl(decl);
    } else if (decl->kind == AST_FUNCTION_DECL) {
      //link_parent_function_decl(decl);
    } else if (decl->kind == AST_ACTION_DECL) {
      //link_parent_action_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND_DECL) {
      //link_parent_match_kind(decl);
    } else if (decl->kind == AST_ERROR_DECL) {
      //link_parent_error_decl(decl);
    }
    else assert(0);
    link = link->next;
  }
}

void
build_nameref(struct Ast* p4program)
{
  build_nameref_p4program(p4program);
}
