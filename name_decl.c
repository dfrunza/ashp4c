#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Arena* name_storage;
internal Scope* root_scope;
internal Scope* current_scope;

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
  assert(ast->kind == AST_binaryExpression);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_unaryExpression);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
}

internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  name->scope = current_scope;
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* expr = (Ast_FunctionCall*)ast;
  visit_expression(expr->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)expr->callee_expr;
  Ast_List* type_args = (Ast_List*)callee_expr->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
  Ast_List* args = (Ast_List*)expr->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_memberSelectExpression);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
  visit_expression(expr->member_name);
}

internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_exprListExpression);
  Ast_ExprListExpression* expr = (Ast_ExprListExpression*)ast;
  Ast_List* expr_list = (Ast_List*)expr->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr_expr = li->object;
      visit_expression(expr_expr);
    }
  }
}

internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_Cast* expr = (Ast_Cast*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr);
}

internal void
visit_subscript(Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_Subscript* expr = (Ast_Subscript*)ast;
  visit_expression(expr->index);
  if (expr->end_index) {
    visit_expression(expr->end_index);
  }
}

internal void
visit_kvpair(Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  Ast_KVPairExpr* expr = (Ast_KVPairExpr*)ast;
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
  if (ast->kind == AST_binaryExpression) {
    visit_binary_expr(ast);
  } else if (ast->kind == AST_unaryExpression) {
    visit_unary_expr(ast);
  } else if (ast->kind == AST_name) {
    visit_name_identifier(ast);
  } else if (ast->kind == AST_functionCall) {
    visit_function_call(ast);
  } else if (ast->kind == AST_memberSelectExpression) {
    visit_member_select(ast);
  } else if (ast->kind == AST_exprListExpression) {
    visit_expression_list(ast);
  } else if (ast->kind == AST_castExpression) {
    visit_cast_expr(ast);
  } else if (ast->kind == AST_arraySubscript) {
    visit_subscript(ast);
  } else if (ast->kind == AST_kvPairExpression) {
    visit_kvpair(ast);
  } else if (ast->kind == AST_integerLiteral) {
    visit_int_literal(ast);
  } else if (ast->kind == AST_booleanLiteral) {
    visit_bool_literal(ast);
  } else if (ast->kind == AST_stringLiteral) {
    visit_string_literal(ast);
  }
  else assert(0);
}

internal void
visit_param(Ast* ast)
{
  assert(ast->kind == AST_parameter);
  Ast_Param* param = (Ast_Param*)ast;
  Ast_Name* name = (Ast_Name*)param->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)param);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(param->type);
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_name);
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
  assert(ast->kind == AST_blockStatement);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  current_scope = push_scope();
  Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
  if (stmt_list) {
    for (DListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      visit_statement(decl);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_actionRef);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  visit_expression(action->name);
  Ast_List* args = (Ast_List*)action->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_keyElement);
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
  if (ast->kind == AST_defaultKeysetExpression) {
    visit_default_keyset(ast);
  } else if (ast->kind == AST_dontcareArgument) {
    visit_dontcare_keyset(ast);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_tuple_keyset(Ast* ast)
{
  assert(ast->kind == AST_tupleKeysetExpression);
  Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
  Ast_List* expr_list = (Ast_List*)keyset->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      visit_keyset_expr(expr);
    }
  }
}

internal void
visit_select_keyset(Ast* ast)
{
  if (ast->kind == AST_tupleKeysetExpression) {
    visit_tuple_keyset(ast);
  } else {
    visit_keyset_expr(ast);
  }
}

internal void
visit_table_entry(Ast* ast)
{
  assert(ast->kind == AST_tableEntry);
  Ast_TableEntry* entry = (Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}

internal void
visit_table_actions(Ast *ast)
{
  assert(ast->kind == AST_tableActions);
  Ast_TableActions* prop = (Ast_TableActions*)ast;
  Ast_List* action_list = (Ast_List*)prop->action_list;
  if (action_list) {
    DListItem* li = action_list->members.sentinel.next;
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
  assert(ast->kind == AST_tableProperty);
  Ast_TableProperty* prop = (Ast_TableProperty*)ast;
  if (prop->init_expr) {
    visit_expression(prop->init_expr);
  }
}

internal void
visit_table_key(Ast* ast)
{
  assert(ast->kind == AST_tableKey);
  Ast_TableKey* prop = (Ast_TableKey*)ast;
  Ast_List* keyelem_list = (Ast_List*)prop->keyelem_list;
  if (keyelem_list) {
    for (DListItem* li = keyelem_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
    }
  }
}

internal void
visit_table_entries(Ast* ast)
{
  assert(ast->kind == AST_tableEntries);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_List* entries = (Ast_List*)prop->entries;
  if (entries) {
    for (DListItem* li = entries->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* entry = li->object;
      visit_table_entry(entry);
    }
  }
}

internal void
visit_table_property(Ast* ast)
{
  if (ast->kind == AST_tableActions) {
    visit_table_actions(ast);
  } else if (ast->kind == AST_tableProperty) {
    visit_table_single_entry(ast);
  } else if (ast->kind == AST_tableKey) {
    visit_table_key(ast);
  } else if (ast->kind == AST_tableEntries) {
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
  if (ast->kind == AST_defaultKeysetExpression) {
    visit_switch_default(ast);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_switch_case(Ast* ast)
{
  assert(ast->kind == AST_switchCase);
  Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_blockStatement) {
    visit_block_statement(case_stmt);
  }
}

internal void
visit_var_decl(Ast* ast)
{
  Ast_Var* var_decl = (Ast_Var*)ast;
  Ast_Name* name = (Ast_Name*)var_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
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
visit_table(Ast* ast)
{
  assert(ast->kind == AST_tableDeclaration);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_Name* name = (Ast_Name*)table_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_type_name(current_scope, name, (Ast*)table_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
  if (prop_list) {
    for (DListItem* li = prop_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* prop = li->object;
      visit_table_property(prop);
    }
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
  Ast_List* switch_cases = (Ast_List*)stmt->switch_cases;
  if (switch_cases) {
    for (DListItem* li = switch_cases->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* switch_case = li->object;
      visit_switch_case(switch_case);
    }
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
  if (ast->kind == AST_variableDeclaration) {
    visit_var_decl(ast);
  } else if (ast->kind == AST_actionDeclaration) {
    visit_action(ast);
  } else if (ast->kind == AST_blockStatement) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_instantiation) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_tableDeclaration) {
    visit_table(ast);
  } else if (ast->kind == AST_conditionalStatement) {
    visit_if_stmt(ast);
  } else if (ast->kind == AST_switchStatement) {
    visit_switch_stmt(ast);
  } else if (ast->kind == AST_assignmentStatement) {
    visit_assignment_stmt(ast);
  } else if (ast->kind == AST_functionCall) {
    visit_function_call(ast);
  } else if (ast->kind == AST_returnStatement) {
    visit_return_stmt(ast);
  } else if (ast->kind == AST_exitStatement) {
    visit_exit_stmt(ast);
  } else if(ast->kind == AST_emptyStatement) {
    visit_empty_stmt(ast);
  }
  else assert(0);
}

internal void
visit_local_parser_element(Ast* ast)
{
  if (ast->kind == AST_constantDeclaration) {
    visit_const(ast);
  } else if (ast->kind == AST_instantiation) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_variableDeclaration) {
    visit_statement(ast);
  } else assert(0);
}

internal void
visit_transition_select_case(Ast* ast)
{
  assert(ast->kind == AST_selectCase);
  Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
  visit_expression(select_case->name);
}

internal void
visit_select_expr(Ast* ast)
{
  assert(ast->kind == AST_selectExpression);
  Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
  Ast_List* expr_list = (Ast_List*)trans_stmt->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      visit_expression(expr);
    }
  }
  Ast_List* case_list = (Ast_List*)trans_stmt->case_list;
  if (case_list) {
    for (DListItem* li = case_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* select_case = li->object;
      visit_transition_select_case(select_case);
    }
  }
}

internal void
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_name) {
    visit_expression(ast);
  } else if (ast->kind == AST_selectExpression) {
    visit_select_expr(ast);
  }
  else assert(0);
}

internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_parserState);
  Ast_ParserState* state = (Ast_ParserState*)ast;
  Ast_Name* name = (Ast_Name*)state->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)state);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* stmt_list = (Ast_List*)state->stmt_list;
  if (stmt_list) {
    for (DListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* stmt = li->object;
      visit_statement(stmt);
    }
  }
  visit_parser_transition(state->trans_stmt);
  current_scope = pop_scope();
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_structField);
  Ast_StructField* field = (Ast_StructField*)ast;
  Ast_Name* name = (Ast_Name*)field->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)field);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(field->type);
}

internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBool);
  visit_expression(((Ast_BoolType*)ast)->name);
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeInt);
  visit_expression(((Ast_IntType*)ast)->name);
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
  visit_expression(((Ast_BitType*)ast)->name);
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
  visit_expression(((Ast_VarbitType*)ast)->name);
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
  visit_expression(((Ast_StringType*)ast)->name);
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
  visit_expression(((Ast_VoidType*)ast)->name);
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
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
  assert(ast->kind == AST_name);
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
  Ast_List* type_args = (Ast_List*)speclzd_type->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
}

internal void
visit_tuple(Ast* ast)
{
  Ast_Tuple* type_ref = (Ast_Tuple*)ast;
  Ast_List* type_args = (Ast_List*)type_ref->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
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
  if (ast->kind == AST_baseTypeBool) {
    visit_bool_type(ast);
  } else if (ast->kind == AST_baseTypeInt) {
    visit_int_type(ast);
  } else if (ast->kind == AST_baseTypeBit) {
    visit_bit_type(ast);
  } else if (ast->kind == AST_baseTypeVarbit) {
    visit_varbit_type(ast);
  } else if (ast->kind == AST_baseTypeString) {
    visit_string_type(ast);
  } else if (ast->kind == AST_baseTypeVoid) {
    visit_void_type(ast);
  } else if (ast->kind == AST_baseTypeError) {
    visit_error_type(ast);
  } else if (ast->kind == AST_headerStackType) {
    visit_header_stack(ast);
  } else if (ast->kind == AST_name) {
    visit_name_type(ast);
  } else if (ast->kind == AST_specializedType) {
    visit_specialized_type(ast);
  } else if (ast->kind == AST_tupleType) {
    visit_tuple(ast);
  } else if (ast->kind == AST_structTypeDeclaration) {
    visit_struct(ast);
  } else if (ast->kind == AST_headerTypeDeclaration) {
    visit_header(ast);
  } else if (ast->kind == AST_headerUnionDeclaration) {
    visit_header_union(ast);
  } else if (ast->kind == AST_dontcareArgument) {
    visit_dontcare_type(ast);
  }
  else assert(0);
}

internal void
visit_enum_field(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)name);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
}

internal void
visit_specified_identifier(Ast* ast)
{
  assert(ast->kind == AST_specifiedIdentifier);
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
  assert(ast->kind == AST_controlDeclaration);
  Ast_Control* ctrl_decl = (Ast_Control*)ast;
  Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ctrl_decl->proto;
  Ast_Name* name = (Ast_Name*)ctrl_proto->name;
  declare_type_name(current_scope, name, (Ast*)ctrl_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)ctrl_decl->ctor_params;
  if (ctor_params) {
    for (DListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* local_decls = (Ast_List*)ctrl_decl->local_decls;
  if (local_decls) {
    for (DListItem* li = local_decls->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      visit_statement(decl);
    }
  }
  if (ctrl_decl->apply_stmt) {
    visit_block_statement(ctrl_decl->apply_stmt);
  }
  current_scope = pop_scope();
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ast;
  Ast_Name* name = (Ast_Name*)ctrl_proto->name;
  declare_type_name(current_scope, name, (Ast*)ctrl_proto);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  declare_type_name(current_scope, name, (Ast*)extern_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)extern_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* method_protos = (Ast_List*)extern_decl->method_protos;
  if (method_protos) {
    for (DListItem* li = method_protos->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* proto = li->object;
      visit_function_proto(proto);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_structTypeDeclaration);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Ast_Name* name = (Ast_Name*)struct_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)struct_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)struct_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_Name* name = (Ast_Name*)header_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)header_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)header_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_Name* name = (Ast_Name*)union_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)union_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)union_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)package_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)package_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)package_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_parserDeclaration);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
  Ast_Name* name = (Ast_Name*)proto->name;
  declare_type_name(current_scope, name, (Ast*)parser_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)parser_decl->ctor_params;
  if (ctor_params) {
    for (DListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* local_elements = (Ast_List*)parser_decl->local_elements;
  if (local_elements) {
    for (DListItem* li = local_elements->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* element = li->object;
      visit_local_parser_element(element);
    }
  }
  Ast_List* states = (Ast_List*)parser_decl->states;
  if (states) {
    for (DListItem* li = states->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* state = li->object;
      visit_parser_state(state);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
  Ast_Name* name = (Ast_Name*)proto_decl->name;
  declare_type_name(current_scope, name, (Ast*)proto_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)proto_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  Ast_Name* name = (Ast_Name*)inst_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)inst_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(inst_decl->type);
  Ast_List* args = (Ast_List*)inst_decl->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}

internal void
visit_typedef(Ast* ast)
{
  assert(ast->kind == AST_typedefDeclaration);
  Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
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
  assert(ast->kind == AST_functionDeclaration);
  Ast_Function* func_decl = (Ast_Function*)ast;
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)func_decl->proto;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name, (Ast*)func_decl);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_BlockStmt* func_body = (Ast_BlockStmt*)func_decl->stmt;
  if (func_body) {
    Ast_List* stmt_list = (Ast_List*)func_body->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        visit_statement(stmt);
      }
    }
  }
  current_scope = pop_scope();
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_functionPrototype);
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)ast;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name, (Ast*)func_proto);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_Const* const_decl = (Ast_Const*)ast;
  Ast_Name* name = (Ast_Name*)const_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name, (Ast*)const_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name, (Ast*)enum_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)enum_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      visit_specified_identifier(id);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_Name* name = (Ast_Name*)action_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NameEntry* ne = arena_push_struct(name_storage, NameEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_type_name(current_scope, name, (Ast*)action_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* params = (Ast_List*)action_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_List* stmt_list = (Ast_List*)action_body->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        visit_statement(stmt);
      }
    }
  }
  current_scope = pop_scope();
}

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_matchKindDeclaration);
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  assert(current_scope->scope_level == 1);
  Ast_List* fields = (Ast_List*)match_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      if (id->kind == AST_name) {
        visit_enum_field(id);
      } else if (id->kind == AST_specifiedIdentifier) {
        visit_specified_identifier(id);
      }
      else assert(0);
    }
  }
}

internal void
visit_error_enum(Ast* ast)
{
  assert (ast->kind == AST_errorDeclaration);
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)error_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      visit_enum_field(id);
    }
  }
  current_scope = pop_scope();
}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_p4program);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  current_scope = push_scope();
  Ast_List* decl_list = (Ast_List*)program->decl_list;
  if (decl_list) {
    for (DListItem* li = decl_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      if (decl->kind == AST_controlDeclaration) {
        visit_control(decl);
      } else if (decl->kind == AST_controlTypeDeclaration) {
        visit_control_proto(decl);
      } else if (decl->kind == AST_externDeclaration) {
        visit_extern(decl);
      } else if (decl->kind == AST_structTypeDeclaration) {
        visit_struct(decl);
      } else if (decl->kind == AST_headerTypeDeclaration) {
        visit_header(decl);
      } else if (decl->kind == AST_headerUnionDeclaration) {
        visit_header_union(decl);
      } else if (decl->kind == AST_packageTypeDeclaration) {
        visit_package(decl);
      } else if (decl->kind == AST_parserDeclaration) {
        visit_parser(decl);
      } else if (decl->kind == AST_parserTypeDeclaration) {
        visit_parser_proto(decl);
      } else if (decl->kind == AST_instantiation) {
        visit_instantiation(decl);
      } else if (decl->kind == AST_typedefDeclaration) {
        visit_typedef(decl);
      } else if (decl->kind == AST_functionDeclaration) {
        visit_function(decl);
      } else if (decl->kind == AST_functionPrototype) {
        visit_function_proto(decl);
      } else if (decl->kind == AST_constantDeclaration) {
        visit_const(decl);
      } else if (decl->kind == AST_enumDeclaration) {
        visit_enum(decl);
      } else if (decl->kind == AST_actionDeclaration) {
        visit_action(decl);
      } else if (decl->kind == AST_matchKindDeclaration) {
        visit_match_kind(decl);
      } else if (decl->kind == AST_errorDeclaration) {
        visit_error_enum(decl);
      }
      else assert(0);
    }
  }
  current_scope = pop_scope();
}

Scope*
build_name_decl(Ast_P4Program* p4program, Arena* decl_storage_)
{
  name_storage = decl_storage_;
  symbol_table_init(name_storage);
  root_scope = current_scope = push_scope();

  {
    Ast_Name* void_type = arena_push_struct(name_storage, Ast_Name);
    void_type->kind = AST_name;
    void_type->strname = "void";
    void_type->id = ++p4program->last_node_id;
    declare_type_name(root_scope, void_type, (Ast*)void_type);
  }
  {
    Ast_Name* bool_type = arena_push_struct(name_storage, Ast_Name);
    bool_type->kind = AST_name;
    bool_type->id = ++p4program->last_node_id;
    bool_type->strname = "bool";
    declare_type_name(root_scope, bool_type, (Ast*)bool_type);
  }
  {
    Ast_Name* int_type = arena_push_struct(name_storage, Ast_Name);
    int_type->kind = AST_name;
    int_type->id = ++p4program->last_node_id;
    int_type->strname = "int";
    declare_type_name(root_scope, int_type, (Ast*)int_type);
  }
  {
    Ast_Name* bit_type = arena_push_struct(name_storage, Ast_Name);
    bit_type->kind = AST_name;
    bit_type->id = ++p4program->last_node_id;
    bit_type->strname = "bit";
    declare_type_name(root_scope, bit_type, (Ast*)bit_type);
  }
  {
    Ast_Name* varbit_type = arena_push_struct(name_storage, Ast_Name);
    varbit_type->kind = AST_name;
    varbit_type->id = ++p4program->last_node_id;
    varbit_type->strname = "varbit";
    declare_type_name(root_scope, varbit_type, (Ast*)varbit_type);
  }
  {
    Ast_Name* string_type = arena_push_struct(name_storage, Ast_Name);
    string_type->kind = AST_name;
    string_type->id = ++p4program->last_node_id;
    string_type->strname = "string";
    declare_type_name(root_scope, string_type, (Ast*)string_type);
  }
  {
    Ast_Name* error_type = arena_push_struct(name_storage, Ast_Name);
    error_type->kind = AST_name;
    error_type->id = ++p4program->last_node_id;
    error_type->strname = "error";
    declare_type_name(root_scope, error_type, (Ast*)error_type);
  }
  {
    Ast_Name* match_type = arena_push_struct(name_storage, Ast_Name);
    match_type->kind = AST_name;
    match_type->id = ++p4program->last_node_id;
    match_type->strname = "match_kind";
    declare_type_name(root_scope, match_type, (Ast*)match_type);
  }
  {
    Ast_Name* accept_state = arena_push_struct(name_storage, Ast_Name);
    accept_state->kind = AST_name;
    accept_state->id = ++p4program->last_node_id;
    accept_state->strname = "accept";
    declare_var_name(root_scope, accept_state, (Ast*)accept_state);
  }
  {
    Ast_Name* reject_state = arena_push_struct(name_storage, Ast_Name);
    reject_state->kind = AST_name;
    reject_state->id = ++p4program->last_node_id;
    reject_state->strname = "reject";
    declare_var_name(root_scope, reject_state, (Ast*)reject_state);
  }
  {
    Ast_Name* add_op = arena_push_struct(name_storage, Ast_Name);
    add_op->kind = AST_name;
    add_op->id = ++p4program->last_node_id;
    add_op->strname = "+";
    declare_type_name(root_scope, add_op, (Ast*)add_op);
  }

  visit_p4program((Ast*)p4program);
  current_scope = pop_scope();
  assert(current_scope == 0);
  return root_scope;
}
