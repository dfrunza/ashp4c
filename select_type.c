#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap selected_type = {};
internal Hashmap* potential_types;

internal void visit_block_statement(Ast* block_stmt);
internal void visit_statement(Ast* decl);
internal void visit_expression(Ast* expr);
internal void visit_type_ref(Ast* type_ref);

void
type_select(Type* type, uint32_t ast_id)
{
  HashmapKey key = { .i_key = ast_id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, selected_type.capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(&selected_type, &key);
  he->object = type;
}

internal void
visit_control(Ast* ast)
{

}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_CONTROL_PROTO);
  Ast_ControlProto* proto = (Ast_ControlProto*)ast;
  Type_TypeSet* ty_set = typeset_get(potential_types, proto->id);
  Type_Function* proto_ty = ty_set->members.next->object;
  type_select((Type*)proto_ty, proto->id);
  Ast* void_decl = scope_lookup_name(root_scope, "void")->ns_type->ast;
  proto_ty->return_ty = typeset_get(potential_types, void_decl->id)->members.next->object;
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
  assert(ast->kind == AST_PACKAGE);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Type_TypeSet* ty_set = typeset_get(potential_types, package_decl->id);
  Type_TypeName* package_ty = ty_set->members.next->object;
  type_select((Type*)package_ty, package_decl->id);
  Ast_NodeList* params = &package_decl->params;
  DList* li = params->list.next;
  while (li) {
    Ast* param = li->object;
    li = li->next;
  }
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
  assert(ast->kind == AST_FUNCTION_PROTO);
  Ast_FunctionProto* proto = (Ast_FunctionProto*)ast;
  Type_TypeSet* ty_set = typeset_get(potential_types, proto->id);
  Type_Function* proto_ty = ty_set->members.next->object;
  type_select((Type*)proto_ty, proto->id);
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

Hashmap*
select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_types_, Arena* type_storage_)
{
  root_scope = root_scope_;
  potential_types = potential_types_;
  type_storage = type_storage_;
  hashmap_init(&selected_type, HASHMAP_KEY_UINT32, 8, type_storage);

  visit_p4program((Ast*)p4program);
  return &selected_type;
}
