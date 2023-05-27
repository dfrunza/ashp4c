#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Arena* decl_storage;
internal Scope* root_scope;
internal Scope* current_scope;

internal void visit_statement(Ast* ast);
internal void visit_expression(Ast* ast);
internal void visit_type_ref(Ast* ast);

internal void
visit_binary_expr(Ast* ast)
{
  assert(ast->kind == AST_BINARY_EXPR);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
}

internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  Ast_Name* name = (Ast_Name*)ast;
  name->scope = current_scope;
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_UNARY_EXPR);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL);
  Ast_FunctionCall* expr = (Ast_FunctionCall*)ast;
  visit_expression(expr->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)expr->callee_expr;
  DList* li;
  Ast_NodeList* type_args = &callee_expr->type_args;
  li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
  Ast_NodeList* args = &expr->args;
  li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
  }
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_MEMBER_SELECT);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
  visit_expression(expr->member_name);
}

internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_EXPRESSION_LIST);
  Ast_ExpressionList* expr = (Ast_ExpressionList*)ast;
  Ast_NodeList* expr_list = &expr->expr_list;
  DList* li = expr_list->list.next;
  while (li) {
    Ast* expr_expr = li->object;
    visit_expression(expr_expr);
    li = li->next;
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
  Ast_KVPair* expr = (Ast_KVPair*)ast;
  visit_expression(expr->name);
  visit_expression(expr->expr);
}

internal void
visit_int_literal(Ast* ast)
{
  // pass
}

internal void
visit_bool_literal(Ast* ast)
{
  // pass
}

internal void
visit_string_literal(Ast* ast)
{
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
  Ast_Name* name = (Ast_Name*)param->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)param);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(param->type);
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_NAME);
  Ast_Name* name = (Ast_Name*)ast;
  NameEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)name);
  } else {
    visit_type_ref((Ast*)name);
  }
}

internal void
visit_block_statement(Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  current_scope = push_scope();
  Ast_NodeList* stmt_list = &block_stmt->stmt_list;
  DList* li = stmt_list->list.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  visit_expression(action->name);
  Ast_NodeList* args = &action->args;
  DList* li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  visit_expression(keyelem->expr);
  visit_expression(keyelem->name);
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
  Ast_NodeList* expr_list = &keyset->expr_list;
  DList* li = expr_list->list.next;
  while (li) {
    Ast* expr = li->object;
    visit_keyset_expr(expr);
    li = li->next;
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
  Ast_NodeList* action_list = &prop->action_list;
  DList* li = action_list->list.next;
  while (li) {
    Ast* action = li->object;
    visit_action_ref(action);
    li = li->next;
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
  Ast_NodeList* keyelem_list = &prop->keyelem_list;
  DList* li = keyelem_list->list.next;
  while (li) {
    Ast* keyelem = li->object;
    visit_table_keyelem(keyelem);
    li = li->next;
  }
}

internal void
visit_table_entries(Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRIES);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_NodeList* entries = &prop->entries;
  DList* li = entries->list.next;
  while (li) {
    Ast* entry = li->object;
    visit_table_entry(entry);
    li = li->next;
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
visit_const_decl(Ast* ast)
{
  assert(ast->kind == AST_CONST);
  Ast_Const* const_decl = (Ast_Const*)ast;
  Ast_Name* name = (Ast_Name*)const_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)const_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
}

internal void
visit_var_decl(Ast* ast)
{
  Ast_Var* var_decl = (Ast_Var*)ast;
  Ast_Name* name = (Ast_Name*)var_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)var_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_ACTION);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_Name* name = (Ast_Name*)action_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_type_name(current_scope, name, (Ast*)action_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* params = &action_decl->params;
  DList* li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_NodeList* stmt_list = &action_body->stmt_list;
    DList* li = stmt_list->list.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  Ast_Name* name = (Ast_Name*)inst_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)inst_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(inst_decl->type);
  Ast_NodeList* args = &inst_decl->args;
  DList* li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
  }
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_Name* name = (Ast_Name*)table_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_type_name(current_scope, name, (Ast*)table_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast_NodeList* prop_list = &table_decl->prop_list;
  DList* li = prop_list->list.next;
  while (li) {
    Ast* prop = li->object;
    visit_table_property(prop);
    li = li->next;
  }
}

internal void
visit_if_stmt(Ast* ast)
{
  Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
  Ast* if_stmt = stmt->stmt;
  visit_expression(stmt->cond_expr);
  visit_statement(if_stmt);
  Ast* else_stmt = stmt->else_stmt;
  if (else_stmt) {
    visit_statement(else_stmt);
  }
}

internal void
visit_switch_stmt(Ast* ast)
{
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  Ast_NodeList* switch_cases = &stmt->switch_cases;
  DList* li = switch_cases->list.next;
  while (li) {
    Ast* switch_case = li->object;
    visit_switch_case(switch_case);
    li = li->next;
  }
}

internal void
visit_assignment_stmt(Ast* ast)
{
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  Ast* assign_expr = stmt->expr;
  visit_expression(assign_expr);
}

internal void
visit_return_stmt(Ast* ast)
{
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
  } else if(ast->kind == AST_EMPTY_STMT) {
    visit_empty_stmt(ast);
  }
  else assert(0);
}

internal void
visit_local_parser_element(Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const_decl(ast);
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
  visit_expression(select_case->name);
}

internal void
visit_select_expr(Ast* ast)
{
  assert(ast->kind == AST_SELECT_EXPR);
  Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
  DList* li;
  Ast_NodeList* expr_list = &trans_stmt->expr_list;
  li = expr_list->list.next;
  while (li) {
    Ast* expr = li->object;
    visit_expression(expr);
    li = li->next;
  }
  Ast_NodeList* case_list = &trans_stmt->case_list;
  li = case_list->list.next;
  while (li) {
    Ast* select_case = li->object;
    visit_transition_select_case(select_case);
    li = li->next;
  }
}

internal void
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    visit_select_expr(ast);
  }
  else assert(0);
}

internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  Ast_ParserState* state = (Ast_ParserState*)ast;
  Ast_Name* name = (Ast_Name*)state->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)state);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* stmt_list = &state->stmt_list;
  DList* li = stmt_list->list.next;
  while (li) {
    Ast* stmt = li->object;
    visit_statement(stmt);
    li = li->next;
  }
  visit_parser_transition(state->trans_stmt);
  current_scope = pop_scope();
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)ast;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name, (Ast*)func_proto);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  DList* li;
  Ast_NodeList* type_params = &func_proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &func_proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  Ast_StructField* field = (Ast_StructField*)ast;
  Ast_Name* name = (Ast_Name*)field->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)field);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(field->type);
}

internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_BOOL_TYPE);
  visit_expression(((Ast_BoolType*)ast)->name);
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_INT_TYPE);
  visit_expression(((Ast_IntType*)ast)->name);
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_BIT_TYPE);
  visit_expression(((Ast_BitType*)ast)->name);
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_VARBIT_TYPE);
  visit_expression(((Ast_VarbitType*)ast)->name);
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_STRING_TYPE);
  visit_expression(((Ast_StringType*)ast)->name);
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_VOID_TYPE);
  visit_expression(((Ast_VoidType*)ast)->name);
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_ERROR_TYPE);
  visit_expression(((Ast_ErrorType*)ast)->name);
}

internal void
visit_header_stack(Ast* ast)
{
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  visit_expression(type_ref->name);
  Ast* stack_expr = type_ref->stack_expr;
  visit_expression(stack_expr);
}

internal void
visit_name_type(Ast* ast)
{
  Ast_Name* name = (Ast_Name*)ast;
  NameEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    /* Declaration of a type parameter. */
    declare_type_name(current_scope, name, (Ast*)name);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_specialized_type(Ast* ast)
{
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  visit_expression(speclzd_type->name);
  Ast_NodeList* type_args = &speclzd_type->type_args;
  DList* li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
}

internal void
visit_tuple(Ast* ast)
{
  Ast_Tuple* type_ref = (Ast_Tuple*)ast;
  Ast_NodeList* type_args = &type_ref->type_args;
  DList* li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Ast_Name* name = (Ast_Name*)struct_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)struct_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* fields = &struct_decl->fields;
  DList* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_HEADER);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_Name* name = (Ast_Name*)header_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)header_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* fields = &header_decl->fields;
  DList* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_Name* name = (Ast_Name*)union_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)union_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* fields = &union_decl->fields;
  DList* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
  current_scope = pop_scope();
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
  Ast_Name* name = (Ast_Name*)ast;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)name);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
}

internal void
visit_specified_identifier(Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  Ast_SpecifiedIdent* id = (Ast_SpecifiedIdent*)ast;
  Ast_Name* name = (Ast_Name*)id->name;
  visit_enum_field((Ast*)name);
  Ast* init_expr = id->init_expr;
  if (init_expr) {
    visit_expression(init_expr);
  }
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  Ast_Control* control_decl = (Ast_Control*)ast;
  Ast_ControlProto* proto = (Ast_ControlProto*)control_decl->proto;
  Ast_Name* name = (Ast_Name*)proto->name;
  declare_type_name(current_scope, name, (Ast*)control_decl);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* ctor_params = &control_decl->ctor_params;
  li = ctor_params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* local_decls = &control_decl->local_decls;
  li = local_decls->list.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
  if (control_decl->apply_stmt) {
    visit_block_statement(control_decl->apply_stmt);
  }
  current_scope = pop_scope();
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_CONTROL_PROTO);
  Ast_ControlProto* proto_decl = (Ast_ControlProto*)ast;
  Ast_Name* name = (Ast_Name*)proto_decl->name;
  declare_type_name(current_scope, name, (Ast*)proto_decl);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &proto_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto_decl->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  declare_type_name(current_scope, name, (Ast*)extern_decl);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &extern_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* method_protos = &extern_decl->method_protos;
  li = method_protos->list.next;
  while (li) {
    Ast* proto = li->object;
    visit_function_proto(proto);
    li = li->next;
  }
  current_scope = pop_scope();
}

/* visit_struct() */

/* visit_header() */

/* visit_header_union() */

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)package_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &package_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &package_decl->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_PARSER);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
  Ast_Name* name = (Ast_Name*)proto->name;
  declare_type_name(current_scope, name, (Ast*)parser_decl);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* ctor_params = &parser_decl->ctor_params;
  li = ctor_params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* local_elements = &parser_decl->local_elements;
  li = local_elements->list.next;
  while (li) {
    Ast* element = li->object;
    visit_local_parser_element(element);
    li = li->next;
  }
  Ast_NodeList* states = &parser_decl->states;
  li = states->list.next;
  while (li) {
    Ast* state = li->object;
    visit_parser_state(state);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_PARSER_PROTO);
  Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
  Ast_Name* name = (Ast_Name*)proto_decl->name;
  declare_type_name(current_scope, name, (Ast*)proto_decl);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &proto_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto_decl->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}

/* visit_instantiation() */

internal void
visit_type(Ast* ast)
{
  assert(ast->kind == AST_TYPE);
  Ast_Type* type_decl = (Ast_Type*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)type_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast* type_ref = type_decl->type_ref;
  visit_type_ref(type_ref);
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  Ast_Function* func_decl = (Ast_Function*)ast;
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)func_decl->proto;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name, (Ast*)func_decl);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  DList* li;
  Ast_NodeList* type_params = &func_proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &func_proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_BlockStmt* func_body = (Ast_BlockStmt*)func_decl->stmt;
  if (func_body) {
    Ast_NodeList* stmt_list = &func_body->stmt_list;
    DList* li = stmt_list->list.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

/* visit_function_proto() */

/* visit_const_decl() */

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)enum_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* id_list = &enum_decl->id_list;
  DList* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_SPECIFIED_IDENT) {
      visit_specified_identifier(id);
    }
    else assert(0);
    li = li->next;
  }
  current_scope = pop_scope();
}

/* visit_action() */

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND);
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  assert(current_scope->scope_level == 1);
  Ast_NodeList* id_list = &match_decl->id_list;
  DList* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_NAME) {
      visit_enum_field(id);
    } else if (id->kind == AST_SPECIFIED_IDENT) {
      visit_specified_identifier(id);
    }
    else assert(0);
    li = li->next;
  }
}

internal void
visit_error(Ast* ast)
{
  assert (ast->kind == AST_ERROR_ENUM);
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  current_scope = push_scope();
  Ast_NodeList* id_list = &error_decl->id_list;
  DList* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_NAME) {
      visit_enum_field(id);
    }
    else assert(0);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  current_scope = push_scope();
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
    }
    else assert(0);
    li = li->next;
  }
  current_scope = pop_scope();
}

Scope*
build_name_decl(Ast_P4Program* p4program, Arena* decl_storage_)
{
  decl_storage = decl_storage_;
  symbol_table_init(decl_storage);
  root_scope = current_scope = push_scope();

  {
    Ast_Name* void_type = arena_push_struct(decl_storage, Ast_Name);
    void_type->kind = AST_NAME;
    void_type->strname = "void";
    void_type->id = ++p4program->last_node_id;
    declare_type_name(root_scope, void_type, (Ast*)void_type);
  }
  {
    Ast_Name* bool_type = arena_push_struct(decl_storage, Ast_Name);
    bool_type->kind = AST_NAME;
    bool_type->id = ++p4program->last_node_id;
    bool_type->strname = "bool";
    declare_type_name(root_scope, bool_type, (Ast*)bool_type);
  }
  {
    Ast_Name* int_type = arena_push_struct(decl_storage, Ast_Name);
    int_type->kind = AST_NAME;
    int_type->id = ++p4program->last_node_id;
    int_type->strname = "int";
    declare_type_name(root_scope, int_type, (Ast*)int_type);
  }
  {
    Ast_Name* bit_type = arena_push_struct(decl_storage, Ast_Name);
    bit_type->kind = AST_NAME;
    bit_type->id = ++p4program->last_node_id;
    bit_type->strname = "bit";
    declare_type_name(root_scope, bit_type, (Ast*)bit_type);
  }
  {
    Ast_Name* varbit_type = arena_push_struct(decl_storage, Ast_Name);
    varbit_type->kind = AST_NAME;
    varbit_type->id = ++p4program->last_node_id;
    varbit_type->strname = "varbit";
    declare_type_name(root_scope, varbit_type, (Ast*)varbit_type);
  }
  {
    Ast_Name* string_type = arena_push_struct(decl_storage, Ast_Name);
    string_type->kind = AST_NAME;
    string_type->id = ++p4program->last_node_id;
    string_type->strname = "string";
    declare_type_name(root_scope, string_type, (Ast*)string_type);
  }
  {
    Ast_Name* error_type = arena_push_struct(decl_storage, Ast_Name);
    error_type->kind = AST_NAME;
    error_type->id = ++p4program->last_node_id;
    error_type->strname = "error";
    declare_type_name(root_scope, error_type, (Ast*)error_type);
  }
  {
    Ast_Name* match_type = arena_push_struct(decl_storage, Ast_Name);
    match_type->kind = AST_NAME;
    match_type->id = ++p4program->last_node_id;
    match_type->strname = "match_kind";
    declare_type_name(root_scope, match_type, (Ast*)match_type);
  }
  {
    Ast_Name* accept_state = arena_push_struct(decl_storage, Ast_Name);
    accept_state->kind = AST_NAME;
    accept_state->id = ++p4program->last_node_id;
    accept_state->strname = "accept";
    declare_var_name(root_scope, accept_state, (Ast*)accept_state);
  }
  {
    Ast_Name* reject_state = arena_push_struct(decl_storage, Ast_Name);
    reject_state->kind = AST_NAME;
    reject_state->id = ++p4program->last_node_id;
    reject_state->strname = "reject";
    declare_var_name(root_scope, reject_state, (Ast*)reject_state);
  }
  {
    Ast_Name* add_op = arena_push_struct(decl_storage, Ast_Name);
    add_op->kind = AST_NAME;
    add_op->id = ++p4program->last_node_id;
    add_op->strname = "+";
    declare_type_name(root_scope, add_op, (Ast*)add_op);
  }

  visit_p4program((Ast*)p4program);
  current_scope = pop_scope();
  assert(current_scope == 0);
  return root_scope;
}
