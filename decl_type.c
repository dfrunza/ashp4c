#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap decl_typemap = {};

internal void visit_expression(Ast* ast);
internal void visit_type_ref(Ast* ast);
internal void visit_statement(Ast* ast);
internal void visit_control_proto(Ast* ast);
internal void visit_struct(Ast* ast);
internal void visit_header(Ast* ast);
internal void visit_header_union(Ast* ast);
internal void visit_instantiation(Ast* ast);
internal void visit_function_proto(Ast* ast);
internal void visit_const(Ast* ast);
internal void visit_action(Ast* ast);
internal void visit_parser_proto(Ast* ast);

internal void
visit_binary_expr(Ast* ast)
{
  assert(ast->kind == AST_BINARY_EXPR);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_UNARY_EXPR);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
}

internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  // pass
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL);
  Ast_FunctionCall* function_call = (Ast_FunctionCall*)ast;
  visit_expression(function_call->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)function_call->callee_expr;
  Ast_List* type_args = (Ast_List*)callee_expr->type_args;
  if (type_args) {
    DList* li = type_args->members.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  }
  Ast_List* args = (Ast_List*)function_call->args;
  if (args) {
    DList* li = args->members.next;
    while (li) {
      Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_MEMBER_SELECT);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
}

internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_EXPRESSION_LIST);
  Ast_ExpressionList* expr = (Ast_ExpressionList*)ast;
  Ast_List* expr_list = (Ast_List*)expr->expr_list;
  if (expr_list) {
    DList* li = expr_list->members.next;
    while (li) {
      Ast* item = li->object;
      visit_expression(item);
      li = li->next;
    }
  }
}

internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_CAST_EXPR);
  Ast_Cast* expr = (Ast_Cast*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr);
}

internal void
visit_subscript(Ast* ast)
{
  assert(ast->kind == AST_SUBSCRIPT);
  Ast_Subscript* expr = (Ast_Subscript*)ast;
  visit_expression(expr->index);
  if (expr->end_index) {
    visit_expression(expr->end_index);
  }
}

internal void
visit_kvpair(Ast* ast)
{
  assert(ast->kind == AST_KVPAIR);
  // pass
}

internal void
visit_int_literal(Ast* ast)
{
  assert(ast->kind == AST_INT_LITERAL);
  // pass
}

internal void
visit_bool_literal(Ast* ast)
{
  assert(ast->kind == AST_BOOL_LITERAL);
  // pass
}

internal void
visit_string_literal(Ast* ast)
{
  assert(ast->kind == AST_STRING_LITERAL);
  // pass
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
  visit_type_ref(param->type);
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  Ast_Name* name = (Ast_Name*)ast;
  if (!name->scope) {
    /* Declaration of a type parameter. */
  } else {
    visit_expression(ast);
  }
}

internal void
visit_block_statement(Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
  if (stmt_list) {
    DList* li = stmt_list->members.next;
    while (li) {
      Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  Ast_List* args = (Ast_List*)action->args;
  if (args) {
    DList* li = args->members.next;
    while (li) {
      Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  visit_expression(keyelem->expr);
}

internal void
visit_default_keyset(Ast *ast)
{
  // pass
}

internal void
visit_dontcare_keyset(Ast* ast)
{
  // pass
}

internal void
visit_keyset_expr(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    visit_default_keyset(ast);
  } else if (ast->kind == AST_DONTCARE) {
    visit_dontcare_keyset(ast);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_tuple_keyset(Ast* ast)
{
  assert(ast->kind == AST_TUPLE_KEYSET);
  Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
  Ast_List* expr_list = (Ast_List*)keyset->expr_list;
  if (expr_list) {
    DList* li = expr_list->members.next;
    while (li) {
      Ast* expr = li->object;
      visit_keyset_expr(expr);
      li = li->next;
    }
  }
}

internal void
visit_select_keyset(Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    visit_tuple_keyset(ast);
  } else {
    visit_keyset_expr(ast);
  }
}

internal void
visit_table_entry(Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  Ast_TableEntry* entry = (Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}

internal void
visit_table_actions(Ast *ast)
{
  assert(ast->kind == AST_TABLE_ACTIONS);
  Ast_TableActions* prop = (Ast_TableActions*)ast;
  Ast_List* action_list = (Ast_List*)prop->action_list;
  if (action_list) {
    DList* li = action_list->members.next;
    while (li) {
      Ast* action = li->object;
      visit_action_ref(action);
      li = li->next;
    }
  }
}

internal void
visit_table_single_entry(Ast* ast)
{
  assert(ast->kind == AST_TABLE_SINGLE_ENTRY);
  Ast_TableSingleEntry* prop = (Ast_TableSingleEntry*)ast;
  if (prop->init_expr) {
    visit_expression(prop->init_expr);
  }
}

internal void
visit_table_key(Ast* ast)
{
  assert(ast->kind == AST_TABLE_KEY);
  Ast_TableKey* prop = (Ast_TableKey*)ast;
  Ast_List* keyelem_list = (Ast_List*)prop->keyelem_list;
  if (keyelem_list) {
    DList* li = keyelem_list->members.next;
    while (li) {
      Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
      li = li->next;
    }
  }
}

internal void
visit_table_entries(Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRIES);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_List* entries = (Ast_List*)prop->entries;
  if (entries) {
    DList* li = entries->members.next;
    while (li) {
      Ast* entry = li->object;
      visit_table_entry(entry);
      li = li->next;
    }
  }
}

internal void
visit_table_property(Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    visit_table_actions(ast);
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    visit_table_single_entry(ast);
  } else if (ast->kind == AST_TABLE_KEY) {
    visit_table_key(ast);
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    visit_table_entries(ast);
  }
  else assert(0);
}

internal void
visit_switch_default(Ast* ast)
{
  // pass
}

internal void
visit_switch_label(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    visit_switch_default(ast);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_switch_case(Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    visit_block_statement(case_stmt);
  }
}

internal void
visit_var_decl(Ast* ast)
{
  assert(ast->kind == AST_VAR_DECL);
  Ast_Var* var_decl = (Ast_Var*)ast;
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
  if (prop_list) {
    DList* li = prop_list->members.next;
    while (li) {
      Ast* prop = li->object;
      visit_table_property(prop);
      li = li->next;
    }
  }
}

internal void
visit_if_stmt(Ast* ast)
{
  assert(ast->kind == AST_IF_STMT);
  Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
  visit_expression(stmt->cond_expr);
  visit_statement(stmt->stmt);
  if (stmt->else_stmt) {
    visit_statement(stmt->else_stmt);
  }
}

internal void
visit_switch_stmt(Ast* ast)
{
  assert(ast->kind == AST_SWITCH_STMT);
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  Ast_List* switch_cases = (Ast_List*)stmt->switch_cases;
  if (switch_cases) {
    DList* li = switch_cases->members.next;
    while (li) {
      Ast* switch_case = li->object;
      visit_switch_case(switch_case);
      li = li->next;
    }
  }
}

internal void
visit_assignment_stmt(Ast* ast)
{
  assert(ast->kind == AST_ASSIGNMENT_STMT);
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  visit_expression(stmt->expr);
}

internal void
visit_return_stmt(Ast* ast)
{
  assert(ast->kind == AST_RETURN_STMT);
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    visit_expression(stmt->expr);
  }
}

internal void
visit_exit_stmt(Ast* ast)
{
  // pass
}

internal void
visit_empty_stmt(Ast* ast)
{
  // pass
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
  } else if (ast->kind == AST_EMPTY_STMT) {
    visit_empty_stmt(ast);
  }
  else assert(0);
}

internal void
visit_local_parser_element(Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_VAR_DECL) {
    visit_statement(ast);
  } else assert(0);
}

internal void
visit_transition_select_case(Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
}

internal void
visit_select_expr(Ast* ast)
{
  assert(ast->kind == AST_SELECT_EXPR);
  Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
  Ast_List* expr_list = (Ast_List*)trans_stmt->expr_list;
  if (expr_list) {
    DList* li = expr_list->members.next;
    while (li) {
      Ast* expr = li->object;
      visit_expression(expr);
      li = li->next;
    }
  }
  Ast_List* case_list = (Ast_List*)trans_stmt->case_list;
  if (case_list) {
    DList* li = case_list->members.next;
    while (li) {
      Ast* select_case = li->object;
      visit_transition_select_case(select_case);
      li = li->next;
    }
  }
}

internal void
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_NAME) {
    // pass
  } else if (ast->kind == AST_SELECT_EXPR) {
    visit_select_expr(ast);
  }
  else assert(0);
}

internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  Ast_ParserState* state_decl = (Ast_ParserState*)ast;
  Ast_List* stmt_list = (Ast_List*)state_decl->stmt_list;
  if (stmt_list) {
    DList* li = stmt_list->members.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
  visit_parser_transition(state_decl->trans_stmt);
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  Ast_StructField* field = (Ast_StructField*)ast;
  visit_type_ref(field->type);
}

internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_BOOL_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "bool");
  Ast* bool_decl = ne->ns_type->ast;
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_INT_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "int");
  Ast* int_decl = ne->ns_type->ast;
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_BIT_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "bit");
  Ast* bit_decl = ne->ns_type->ast;
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_VARBIT_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "varbit");
  Ast* varbit_decl = ne->ns_type->ast;
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_STRING_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "string");
  Ast* string_decl = ne->ns_type->ast;
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_VOID_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "void");
  Ast* void_decl = ne->ns_type->ast;
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_ERROR_TYPE);
  NameEntry* ne = scope_lookup_name(root_scope, "error");
  Ast* error_decl = ne->ns_type->ast;
}

internal void
visit_header_stack(Ast* ast)
{
  assert(ast->kind == AST_HEADER_STACK);
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  visit_expression(type_ref->name);
  visit_expression(type_ref->stack_expr);
}

internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  Ast_Name* name = (Ast_Name*)ast;
  if (!name->scope) {
    /* Declaration of a type parameter. */
  } else {
    visit_expression(ast);
  }
}

internal void
visit_specialized_type(Ast* ast)
{
  assert(ast->kind == AST_SPECIALIZED_TYPE);
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  visit_expression(speclzd_type->name);
  Ast_List* type_args = (Ast_List*)speclzd_type->type_args;
  if (type_args) {
    DList* li = type_args->members.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  }
}

internal void
visit_tuple(Ast* ast)
{
  assert(ast->kind == AST_TUPLE);
  Ast_Tuple* tuple_decl = (Ast_Tuple*)ast;
  Ast_List* args = (Ast_List*)tuple_decl->type_args;
  if (args) {
    DList* li = args->members.next;
    while (li) {
      Ast* arg = li->object;
      visit_type_ref(arg);
      li = li->next;
    }
  }
}

internal void
visit_dontcare_type(Ast* ast)
{
  // pass
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
  assert(ast->kind == AST_NAME);
  // pass
}

internal void
visit_specified_identifier(Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  // pass
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  Ast_Control* ctrl_decl = (Ast_Control*)ast;
  visit_control_proto(ctrl_decl->proto);
  Ast_List* ctor_params = (Ast_List*)ctrl_decl->ctor_params;
  if (ctor_params) {
    DList* li = ctor_params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  Ast_List* local_decls = (Ast_List*)ctrl_decl->local_decls;
  if (local_decls) {
    DList* li = local_decls->members.next;
    while (li) {
      Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
  if (ctrl_decl->apply_stmt) {
    visit_block_statement(ctrl_decl->apply_stmt);
  }
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_CONTROL_PROTO);
  Ast_ControlProto* proto = (Ast_ControlProto*)ast;
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    DList* li = type_params->members.next;
    while (li) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    DList* li = params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_List* type_params = (Ast_List*)extern_decl->type_params;
  if (type_params) {
    DList* li = type_params->members.next;
    while (li) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  Ast_List* method_protos = (Ast_List*)extern_decl->method_protos;
  if (method_protos) {
    DList* li = method_protos->members.next;
    while (li) {
      Ast* proto = li->object;
      visit_function_proto(proto);
      li = li->next;
    }
  }
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Type_Type* struct_ty = arena_push_struct(type_storage, Type_Type);
  struct_ty->ctor = TYPE_TYPE;
  HashmapEntry* struct_he = hashmap_create_entry_uint32(&decl_typemap, struct_decl->id);
  struct_he->object = (Type*)struct_ty;
  Ast_List* fields = (Ast_List*)struct_decl->fields;
  if (fields) {
    DList* field_li = fields->members.next;
    while (field_li) {
      Ast* field = field_li->object;
      visit_struct_field(field);
      field_li = field_li->next;
    }
  }
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_HEADER);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_List* fields = (Ast_List*)header_decl->fields;
  if (fields) {
    DList* li = fields->members.next;
    while (li) {
      Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_List* fields = (Ast_List*)union_decl->fields;
  if (fields) {
    DList* li = fields->members.next;
    while (li) {
      Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_List* type_params = (Ast_List*)package_decl->type_params;
  if (type_params) {
    DList* li = type_params->members.next;
    while (li) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  Ast_List* params = (Ast_List*)package_decl->params;
  if (params) {
    DList* li = params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_PARSER);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  visit_parser_proto(parser_decl->proto);
  Ast_List* ctor_params = (Ast_List*)parser_decl->ctor_params;
  if (ctor_params) {
    DList* li = ctor_params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  Ast_List* local_elements = (Ast_List*)parser_decl->local_elements;
  if (local_elements) {
    DList* li = local_elements->members.next;
    while (li) {
      Ast* element = li->object;
      visit_local_parser_element(element);
      li = li->next;
    }
  }
  Ast_List* states = (Ast_List*)parser_decl->states;
  if (states) {
    DList* li = states->members.next;
    while (li) {
      Ast* state = li->object;
      visit_parser_state(state);
      li = li->next;
    }
  }
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_PARSER_PROTO);
  Ast_ParserProto* proto = (Ast_ParserProto*)ast;
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    DList* li = type_params->members.next;
    while (li) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    DList* li = params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type);
  Ast_List* args = (Ast_List*)inst_decl->args;
  if (args) {
    DList* li = args->members.next;
    while (li) {
      Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_typedef(Ast* ast)
{
  assert(ast->kind == AST_TYPEDEF);
  Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
  visit_type_ref(type_decl->type_ref);
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  Ast_Function* function_decl = (Ast_Function*)ast;
  visit_function_proto(function_decl->proto);
  Ast_BlockStmt* function_body = (Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    Ast_List* stmt_list = (Ast_List*)function_body->stmt_list;
    if (stmt_list) {
      DList* li = stmt_list->members.next;
      while (li) {
        Ast* stmt = li->object;
        visit_statement(stmt);
        li = li->next;
      }
    }
  }
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  Ast_FunctionProto* proto = (Ast_FunctionProto*)ast;
  if (proto->return_type) {
    visit_type_ref(proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    DList* li = type_params->members.next;
    while (li) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    DList* li = params->members.next;
    while (li) {
      Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_CONST);
  Ast_Const* const_decl = (Ast_Const*)ast;
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_List* fields = (Ast_List*)enum_decl->fields;
  if (fields) {
    DList* field_li = fields->members.next;
    while (field_li) {
      Ast* field = field_li->object;
      if (field->kind == AST_NAME) {
        visit_enum_field(field);
      } else if (field->kind == AST_SPECIFIED_IDENT) {
        visit_specified_identifier(field);
      }
      else assert(0);
      field_li = field_li->next;
    }
  }
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_ACTION);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_List* params = (Ast_List*)action_decl->params;
  if (params) {
    DList* param_li = params->members.next;
    while (param_li) {
      Ast* param = param_li->object;
      visit_param(param);
      param_li = param_li->next;
    }
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_List* stmt_list = (Ast_List*)action_body->stmt_list;
    if (stmt_list) {
      DList* stmt_li = stmt_list->members.next;
      while (stmt_li) {
        Ast* stmt = stmt_li->object;
        visit_statement(stmt);
        stmt_li = stmt_li->next;
      }
    }
  }
}

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND);
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  Ast_List* fields = (Ast_List*)match_decl->fields;
  if (fields) {
    DList* field_li = fields->members.next;
    while (field_li) {
      Ast* field = field_li->object;
      if (field->kind == AST_NAME) {
        visit_enum_field(field);
      } else if (field->kind == AST_SPECIFIED_IDENT) {
        visit_specified_identifier(field);
      }
      else assert(0);
      field_li = field_li->next;
    }
  }
}

internal void
visit_error_enum(Ast* ast)
{
  assert (ast->kind == AST_ERROR_ENUM);
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  Ast_List* fields = (Ast_List*)error_decl->fields;
  if (fields) {
    DList* field_li = fields->members.next;
    while (field_li) {
      Ast* field = field_li->object;
      if (field->kind == AST_NAME) {
        visit_enum_field(field);
      }
      else assert(0);
      field_li = field_li->next;
    }
  }
}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_List* decl_list = (Ast_List*)program->decl_list;
  if (decl_list) {
    DList* decl_li = decl_list->members.next;
    while (decl_li) {
      Ast* decl = decl_li->object;
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
      } else if (decl->kind == AST_TYPEDEF) {
        visit_typedef(decl);
      } else if (decl->kind == AST_FUNCTION) {
        visit_function(decl);
      } else if (decl->kind == AST_FUNCTION_PROTO) {
        visit_function_proto(decl);
      } else if (decl->kind == AST_CONST) {
        visit_const(decl);
      } else if (decl->kind == AST_ENUM) {
        visit_enum(decl);
      } else if (decl->kind == AST_ACTION) {
        visit_action(decl);
      } else if (decl->kind == AST_MATCH_KIND) {
        visit_match_kind(decl);
      } else if (decl->kind == AST_ERROR_ENUM) {
        visit_error_enum(decl);
      } else assert(0);
      decl_li = decl_li->next;
    }
  }
}

Hashmap*
build_decl_type(Ast_P4Program* p4program, Scope* root_scope_, Arena* type_storage_)
{
  root_scope = root_scope_;
  type_storage = type_storage_;
  hashmap_init(&decl_typemap, HASHMAP_KEY_UINT32, 8, type_storage);

  {
    Ast* void_decl = scope_lookup_name(root_scope, "void")->ns_type->ast;
    Type* void_ty = arena_push_struct(type_storage, Type);
    void_ty->ctor = TYPE_VOID;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&decl_typemap, void_decl->id);
    bit_he->object = void_ty;
  }
  {
    Ast* bool_decl = scope_lookup_name(root_scope, "bool")->ns_type->ast;
    Type* bool_ty = arena_push_struct(type_storage, Type);
    bool_ty->ctor = TYPE_BOOL;
    HashmapEntry* bool_he = hashmap_create_entry_uint32(&decl_typemap, bool_decl->id);
    bool_he->object = bool_decl;
  }
  {
    Ast* int_decl = scope_lookup_name(root_scope, "int")->ns_type->ast;
    Type* int_ty = arena_push_struct(type_storage, Type);
    int_ty->ctor = TYPE_INT;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&decl_typemap, int_decl->id);
    bit_he->object = int_ty;
  }
  {
    Ast* bit_decl = scope_lookup_name(root_scope, "bit")->ns_type->ast;
    Type* bit_ty = arena_push_struct(type_storage, Type);
    bit_ty->ctor = TYPE_BIT;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&decl_typemap, bit_decl->id);
    bit_he->object = bit_ty;
  }
  {
    Ast* varbit_decl = scope_lookup_name(root_scope, "varbit")->ns_type->ast;
    Type* varbit_ty = arena_push_struct(type_storage, Type);
    varbit_ty->ctor = TYPE_VARBIT;
    HashmapEntry* varbit_he = hashmap_create_entry_uint32(&decl_typemap, varbit_decl->id);
    varbit_he->object = varbit_ty;
  }
  {
    Ast* string_decl = scope_lookup_name(root_scope, "string")->ns_type->ast;
    Type* string_ty = arena_push_struct(type_storage, Type);
    string_ty->ctor = TYPE_STRING;
    HashmapEntry* string_he = hashmap_create_entry_uint32(&decl_typemap, string_decl->id);
    string_he->object = string_ty;
  }

  visit_p4program((Ast*)p4program);
  return &decl_typemap;
}

