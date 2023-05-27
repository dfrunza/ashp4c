#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap selected_type = {};
internal Hashmap* potential_types;

internal void visit_statement(Ast* ast);
internal void visit_expression(Ast* ast);
internal void visit_type_ref(Ast* ast);

void
type_select(Type* type, uint32_t ast_id)
{
  HashmapKey key = { .i_key = ast_id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, selected_type.capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(&selected_type, &key);
  he->object = type;
}

internal void
visit_binary_expr(Ast* ast)
{

}

internal void
visit_name_identifier(Ast* ast)
{

}

internal void
visit_unary_expr(Ast* ast)
{

}

internal void
visit_function_call(Ast* ast)
{

}

internal void
visit_member_select(Ast* ast)
{

}

internal void
visit_expression_list(Ast* ast)
{

}

internal void
visit_cast_expr(Ast* ast)
{

}

internal void
visit_subscript(Ast* ast)
{

}

internal void
visit_kvpair(Ast* ast)
{

}

internal void
visit_int_literal(Ast* ast)
{

}

internal void
visit_bool_literal(Ast* ast)
{

}

internal void
visit_string_literal(Ast* ast)
{

}

internal void
visit_expression(Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    visit_binary_expr(ast);
  } else if (ast->kind == AST_UNARY_EXPR) {
    visit_unary_expr(ast);
  } else if (ast->kind == AST_NAME) {
    visit_name_identifier(ast);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT) {
    visit_member_select(ast);
  } else if (ast->kind == AST_EXPRESSION_LIST) {
    visit_expression_list(ast);
  } else if (ast->kind == AST_CAST_EXPR) {
    visit_cast_expr(ast);
  } else if (ast->kind == AST_SUBSCRIPT) {
    visit_subscript(ast);
  } else if (ast->kind == AST_KVPAIR) {
    visit_kvpair(ast);
  } else if (ast->kind == AST_INT_LITERAL) {
    visit_int_literal(ast);
  } else if (ast->kind == AST_BOOL_LITERAL) {
    visit_bool_literal(ast);
  } else if (ast->kind == AST_STRING_LITERAL) {
    visit_string_literal(ast);
  }
  else assert(0);
}

internal void
visit_param(Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  Ast_Param* param = (Ast_Param*)ast;
  Type_TypeSet* ty_set = typeset_get(potential_types, param->id);
  Type* param_ty = ty_set->members.next->object;
  type_select(param_ty, param->id);
  if (ty_set->member_count > 1) {
    Ast_Name* name = (Ast_Name*)param->name;
    error("At line %d, column %d: type of `%s` is ambiguous.",
          name->line_no, name->column_no, name->strname);
  }
}

internal void
visit_type_param(Ast* ast)
{

}

internal void
visit_block_statement(Ast* ast)
{

}

internal void
visit_action_ref(Ast* ast)
{

}

internal void
visit_table_keyelem(Ast* ast)
{

}

internal void
visit_default_keyset(Ast *ast)
{

}

internal void
visit_dontcare_keyset(Ast* ast)
{

}

internal void
visit_keyset_expr(Ast* ast)
{

}

internal void
visit_tuple_keyset(Ast* ast)
{

}

internal void
visit_select_keyset(Ast* ast)
{

}

internal void
visit_table_entry(Ast* ast)
{

}

internal void
visit_table_actions(Ast *ast)
{

}

internal void
visit_table_single_entry(Ast* ast)
{

}

internal void
visit_table_key(Ast* ast)
{

}

internal void
visit_table_entries(Ast* ast)
{

}

internal void
visit_table_property(Ast* ast)
{

}

internal void
visit_switch_default(Ast* ast)
{

}

internal void
visit_switch_label(Ast* ast)
{

}

internal void
visit_switch_case(Ast* ast)
{

}

internal void
visit_const_decl(Ast* ast)
{

}

internal void
visit_var_decl(Ast* ast)
{

}

internal void
visit_action(Ast* ast)
{

}

internal void
visit_instantiation(Ast* ast)
{

}

internal void
visit_table(Ast* ast)
{

}

internal void
visit_if_stmt(Ast* ast)
{

}

internal void
visit_switch_stmt(Ast* ast)
{

}

internal void
visit_assignment_stmt(Ast* ast)
{

}

internal void
visit_return_stmt(Ast* ast)
{

}

internal void
visit_exit_stmt(Ast* ast)
{

}

internal void
visit_empty_stmt(Ast* ast)
{

}

internal void
visit_statement(Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    visit_var_decl(ast);
  } else if (ast->kind == AST_ACTION) {
    visit_action(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_TABLE) {
    visit_table(ast);
  } else if (ast->kind == AST_IF_STMT) {
    visit_if_stmt(ast);
  } else if (ast->kind == AST_SWITCH_STMT) {
    visit_switch_stmt(ast);
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    visit_assignment_stmt(ast);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    visit_return_stmt(ast);
  } else if (ast->kind == AST_EXIT_STMT) {
    visit_exit_stmt(ast);
  } else if(ast->kind == AST_EMPTY_STMT) {
    visit_empty_stmt(ast);
  }
  else assert(0);
}

internal void
visit_local_parser_element(Ast* ast)
{

}

internal void
visit_transition_select_case(Ast* ast)
{

}

internal void
visit_select_expr(Ast* ast)
{

}

internal void
visit_parser_transition(Ast* ast)
{

}

internal void
visit_parser_state(Ast* ast)
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
  Ast_NodeList* params = &proto->params;
  DList* li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
}

internal void
visit_struct_field(Ast* ast)
{

}

internal void
visit_bool_type(Ast* ast)
{

}

internal void
visit_int_type(Ast* ast)
{

}

internal void
visit_bit_type(Ast* ast)
{

}

internal void
visit_varbit_type(Ast* ast)
{

}

internal void
visit_string_type(Ast* ast)
{

}

internal void
visit_void_type(Ast* ast)
{

}

internal void
visit_error_type(Ast* ast)
{

}

internal void
visit_header_stack(Ast* ast)
{

}

internal void
visit_name_type(Ast* ast)
{

}

internal void
visit_specialized_type(Ast* ast)
{

}

internal void
visit_tuple(Ast* ast)
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
visit_dontcare_type(Ast* ast)
{

}

internal void
visit_type_ref(Ast* ast)
{
  if (ast->kind == AST_BOOL_TYPE) {
    visit_bool_type(ast);
  } else if (ast->kind == AST_INT_TYPE) {
    visit_int_type(ast);
  } else if (ast->kind == AST_BIT_TYPE) {
    visit_bit_type(ast);
  } else if (ast->kind == AST_VARBIT_TYPE) {
    visit_varbit_type(ast);
  } else if (ast->kind == AST_STRING_TYPE) {
    visit_string_type(ast);
  } else if (ast->kind == AST_VOID_TYPE) {
    visit_void_type(ast);
  } else if (ast->kind == AST_ERROR_TYPE) {
    visit_error_type(ast);
  } else if (ast->kind == AST_HEADER_STACK) {
    visit_header_stack(ast);
  } else if (ast->kind == AST_NAME) {
    visit_name_type(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    visit_specialized_type(ast);
  } else if (ast->kind == AST_TUPLE) {
    visit_tuple(ast);
  } else if (ast->kind == AST_STRUCT) {
    visit_struct(ast);
  } else if (ast->kind == AST_HEADER) {
    visit_header(ast);
  } else if (ast->kind == AST_HEADER_UNION) {
    visit_header_union(ast);
  } else if (ast->kind == AST_DONTCARE) {
    visit_dontcare_type(ast);
  }
  else assert(0);
}

internal void
visit_enum_field(Ast* ast)
{

}

internal void
visit_specified_identifier(Ast* ast)
{

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
  Ast_NodeList* params = &proto->params;
  DList* li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
}

internal void
visit_extern(Ast* ast)
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
    visit_param(param);
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
visit_type(Ast* ast)
{

}

internal void
visit_function(Ast* ast)
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
    } else if (decl->kind == AST_FUNCTION) {
      visit_function(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      visit_function_proto(decl);
    } else if (decl->kind == AST_CONST) {
      visit_const_decl(decl);
    } else if (decl->kind == AST_ENUM) {
      visit_enum(decl);
    } else if (decl->kind == AST_ACTION) {
      visit_action(decl);
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
