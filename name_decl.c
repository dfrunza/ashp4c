#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal Arena* name_storage;
internal Scope* root_scope;
internal Scope* current_scope;

#if 0
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
#endif

#if 0
internal void
visit_binary_expr(Ast* ast)
{
  assert(ast->kind == AST_binaryExpression);
  Ast_BinaryExpression* expr = (Ast_BinaryExpression*)ast;
  visit_expression(expr->left_operand);
  visit_expression(expr->right_operand);
}
#endif

#if 0
internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_unaryExpression);
  Ast_UnaryExpression* expr = (Ast_UnaryExpression*)ast;
  visit_expression(expr->operand);
}
#endif

#if 0
internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  name->scope = current_scope;
}
#endif

#if 0
internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* expr = (Ast_FunctionCall*)ast;
  visit_expression(expr->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)expr->callee_expr;
  Ast_List* type_args = (Ast_List*)callee_expr->type_args;
  if (type_args) {
    for (ListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
  Ast_List* args = (Ast_List*)expr->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}
#endif

#if 0
internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_memberSelector);
  Ast_MemberSelector* expr = (Ast_MemberSelector*)ast;
  visit_expression(expr->lhs_expr);
  visit_expression(expr->member_name);
}
#endif

#if 0
internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_exprListExpression);
  Ast_ExprListExpression* expr = (Ast_ExprListExpression*)ast;
  Ast_List* expr_list = (Ast_List*)expr->expr_list;
  if (expr_list) {
    for (ListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr_expr = li->object;
      visit_expression(expr_expr);
    }
  }
}
#endif

#if 0
internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_CastExpression* expr = (Ast_CastExpression*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr);
}
#endif

#if 0
internal void
visit_array_subscript(Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_ArraySubscript* expr = (Ast_ArraySubscript*)ast;
  visit_expression(expr->index);
  if (expr->end_index) {
    visit_expression(expr->end_index);
  }
}
#endif

#if 0
internal void
visit_kvpair_expr(Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  Ast_KVPair* expr = (Ast_KVPair*)ast;
  visit_expression(expr->name);
  visit_expression(expr->expr);
}
#endif

#if 0
internal void
visit_int_literal(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_bool_literal(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_string_literal(Ast* ast)
{
  // pass
}
#endif

#if 0
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
  } else if (ast->kind == AST_memberSelector) {
    visit_member_select(ast);
  } else if (ast->kind == AST_exprListExpression) {
    visit_expression_list(ast);
  } else if (ast->kind == AST_castExpression) {
    visit_cast_expr(ast);
  } else if (ast->kind == AST_arraySubscript) {
    visit_array_subscript(ast);
  } else if (ast->kind == AST_kvPairExpression) {
    visit_kvpair_expr(ast);
  } else if (ast->kind == AST_integerLiteral) {
    visit_int_literal(ast);
  } else if (ast->kind == AST_booleanLiteral) {
    visit_bool_literal(ast);
  } else if (ast->kind == AST_stringLiteral) {
    visit_string_literal(ast);
  }
  else assert(0);
}
#endif

#if 0
internal void
visit_param(Ast* ast)
{
  assert(ast->kind == AST_parameter);
  Ast_Param* param = (Ast_Param*)ast;
  Ast_Name* name = (Ast_Name*)param->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)param);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(param->type);
}
#endif

#if 0
internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  NamespaceEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else {
    visit_type_ref((Ast*)name);
  }
}
#endif

#if 0
internal void
visit_block_stmt(Ast* ast)
{
  assert(ast->kind == AST_blockStatement);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  current_scope = push_scope();
  Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
  if (stmt_list) {
    for (ListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      visit_statement(decl);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_actionRef);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  visit_expression(action->name);
  Ast_List* args = (Ast_List*)action->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}
#endif

#if 0
internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_keyElement);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  visit_expression(keyelem->expr);
  visit_expression(keyelem->name);
}
#endif

#if 0
internal void
visit_default_keyset(Ast *ast)
{
  // pass
}
#endif

#if 0
internal void
visit_dontcare_keyset(Ast* ast)
{
  // pass
}
#endif

#if 0
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
#endif

#if 0
internal void
visit_tuple_keyset(Ast* ast)
{
  assert(ast->kind == AST_tupleKeysetExpression);
  Ast_TupleKeysetExpression* keyset = (Ast_TupleKeysetExpression*)ast;
  Ast_List* expr_list = (Ast_List*)keyset->expr_list;
  if (expr_list) {
    for (ListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      visit_keyset_expr(expr);
    }
  }
}
#endif

#if 0
internal void
visit_select_keyset(Ast* ast)
{
  if (ast->kind == AST_tupleKeysetExpression) {
    visit_tuple_keyset(ast);
  } else {
    visit_keyset_expr(ast);
  }
}
#endif

#if 0
internal void
visit_table_entry(Ast* ast)
{
  assert(ast->kind == AST_tableEntry);
  Ast_TableEntry* entry = (Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}
#endif

#if 0
internal void
visit_table_actions(Ast *ast)
{
  assert(ast->kind == AST_tableActions);
  Ast_TableActions* prop = (Ast_TableActions*)ast;
  Ast_List* action_list = (Ast_List*)prop->action_list;
  if (action_list) {
    ListItem* li = action_list->members.sentinel.next;
    while (li) {
      Ast* action = li->object;
      visit_action_ref(action);
      li = li->next;
    }
  }
}
#endif

#if 0
internal void
visit_table_single_entry(Ast* ast)
{
  assert(ast->kind == AST_tableProperty);
  Ast_TableProperty* prop = (Ast_TableProperty*)ast;
  if (prop->init_expr) {
    visit_expression(prop->init_expr);
  }
}
#endif

#if 0
internal void
visit_table_key(Ast* ast)
{
  assert(ast->kind == AST_tableKey);
  Ast_TableKey* prop = (Ast_TableKey*)ast;
  Ast_List* keyelem_list = (Ast_List*)prop->keyelem_list;
  if (keyelem_list) {
    for (ListItem* li = keyelem_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
    }
  }
}
#endif

#if 0
internal void
visit_table_entries(Ast* ast)
{
  assert(ast->kind == AST_tableEntries);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_List* entries = (Ast_List*)prop->entries;
  if (entries) {
    for (ListItem* li = entries->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* entry = li->object;
      visit_table_entry(entry);
    }
  }
}
#endif

#if 0
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
#endif

#if 0
internal void
visit_switch_default(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_switch_label(Ast* ast)
{
  if (ast->kind == AST_defaultKeysetExpression) {
    visit_switch_default(ast);
  } else {
    visit_expression(ast);
  }
}
#endif

#if 0
internal void
visit_switch_case(Ast* ast)
{
  assert(ast->kind == AST_switchCase);
  Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_blockStatement) {
    visit_block_stmt(case_stmt);
  }
}
#endif

#if 0
internal void
visit_var_decl(Ast* ast)
{
  Ast_VarDeclaration* var_decl = (Ast_VarDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)var_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)var_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
                name->line_no, name->column_no, name->strname);
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
}
#endif

#if 0
internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_tableDeclaration);
  Ast_TableDeclaration* table_decl = (Ast_TableDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)table_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)table_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
  if (prop_list) {
    for (ListItem* li = prop_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* prop = li->object;
      visit_table_property(prop);
    }
  }
}
#endif

#if 0
internal void
visit_if_stmt(Ast* ast)
{
  Ast_ConditionalStmt* stmt = (Ast_ConditionalStmt*)ast;
  Ast* if_stmt = stmt->stmt;
  visit_expression(stmt->cond_expr);
  visit_statement(if_stmt);
  Ast* else_stmt = stmt->else_stmt;
  if (else_stmt) {
    visit_statement(else_stmt);
  }
}
#endif

#if 0
internal void
visit_switch_stmt(Ast* ast)
{
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  Ast_List* switch_cases = (Ast_List*)stmt->switch_cases;
  if (switch_cases) {
    for (ListItem* li = switch_cases->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* switch_case = li->object;
      visit_switch_case(switch_case);
    }
  }
}
#endif

#if 0
internal void
visit_assignment_stmt(Ast* ast)
{
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  Ast* assign_expr = stmt->expr;
  visit_expression(assign_expr);
}
#endif

#if 0
internal void
visit_return_stmt(Ast* ast)
{
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    visit_expression(stmt->expr);
  }
}
#endif

#if 0
internal void
visit_exit_stmt(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_empty_stmt(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_statement(Ast* ast)
{
  if (ast->kind == AST_variableDeclaration) {
    visit_var_decl(ast);
  } else if (ast->kind == AST_actionDeclaration) {
    visit_action(ast);
  } else if (ast->kind == AST_blockStatement) {
    visit_block_stmt(ast);
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
#endif

#if 0
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
#endif

#if 0
internal void
visit_transition_select_case(Ast* ast)
{
  assert(ast->kind == AST_selectCase);
  Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
  visit_expression(select_case->name);
}
#endif

#if 0
internal void
visit_select_expr(Ast* ast)
{
  assert(ast->kind == AST_selectExpression);
  Ast_SelectExpression* trans_stmt = (Ast_SelectExpression*)ast;
  Ast_List* expr_list = (Ast_List*)trans_stmt->expr_list;
  if (expr_list) {
    for (ListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      visit_expression(expr);
    }
  }
  Ast_List* case_list = (Ast_List*)trans_stmt->case_list;
  if (case_list) {
    for (ListItem* li = case_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* select_case = li->object;
      visit_transition_select_case(select_case);
    }
  }
}
#endif

#if 0
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
#endif

#if 0
internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_parserState);
  Ast_ParserState* state = (Ast_ParserState*)ast;
  Ast_Name* name = (Ast_Name*)state->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)state);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* stmt_list = (Ast_List*)state->stmt_list;
  if (stmt_list) {
    for (ListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* stmt = li->object;
      visit_statement(stmt);
    }
  }
  visit_parser_transition(state->trans_stmt);
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_structField);
  Ast_StructField* field = (Ast_StructField*)ast;
  Ast_Name* name = (Ast_Name*)field->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)field);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(field->type);
}
#endif

#if 0
internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBool);
  visit_expression(((Ast_BoolType*)ast)->name);
}
#endif

#if 0
internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeInteger);
  visit_expression(((Ast_IntegerType*)ast)->name);
}
#endif

#if 0
internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
  visit_expression(((Ast_BitType*)ast)->name);
}
#endif

#if 0
internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
  visit_expression(((Ast_VarbitType*)ast)->name);
}
#endif

#if 0
internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
  visit_expression(((Ast_StringType*)ast)->name);
}
#endif

#if 0
internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
  visit_expression(((Ast_VoidType*)ast)->name);
}
#endif

#if 0
internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
  visit_expression(((Ast_ErrorType*)ast)->name);
}
#endif

#if 0
internal void
visit_header_stack(Ast* ast)
{
  Ast_HeaderStackType* type_ref = (Ast_HeaderStackType*)ast;
  visit_expression(type_ref->name);
  Ast* stack_expr = type_ref->stack_expr;
  visit_expression(stack_expr);
}
#endif

#if 0
internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  NamespaceEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    /* Declaration of a type parameter. */
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else {
    visit_expression(ast);
  }
}
#endif

#if 0
internal void
visit_specialized_type(Ast* ast)
{
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  visit_expression(speclzd_type->name);
  Ast_List* type_args = (Ast_List*)speclzd_type->type_args;
  if (type_args) {
    for (ListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
}
#endif

#if 0
internal void
visit_tuple(Ast* ast)
{
  Ast_TupleType* type_ref = (Ast_TupleType*)ast;
  Ast_List* type_args = (Ast_List*)type_ref->type_args;
  if (type_args) {
    for (ListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
}
#endif

#if 0
internal void
visit_dontcare_type(Ast* ast)
{
  // pass
}
#endif

#if 0
internal void
visit_type_ref(Ast* ast)
{
  if (ast->kind == AST_baseTypeBool) {
    visit_bool_type(ast);
  } else if (ast->kind == AST_baseTypeInteger) {
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
#endif

#if 0
internal void
visit_enum_field(Ast* ast)
{
  assert(ast->kind == AST_name);
  Ast_Name* name = (Ast_Name*)ast;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)name);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
}
#endif

#if 0
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
#endif

#if 0
internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_controlDeclaration);
  Ast_ControlDeclaration* ctrl_decl = (Ast_ControlDeclaration*)ast;
  Ast_ControlTypeDeclaration* ctrl_proto = (Ast_ControlTypeDeclaration*)ctrl_decl->proto;
  Ast_Name* name = (Ast_Name*)ctrl_proto->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)ctrl_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)ctrl_decl->ctor_params;
  if (ctor_params) {
    for (ListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* local_decls = (Ast_List*)ctrl_decl->local_decls;
  if (local_decls) {
    for (ListItem* li = local_decls->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      visit_statement(decl);
    }
  }
  if (ctrl_decl->apply_stmt) {
    visit_block_stmt(ctrl_decl->apply_stmt);
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlTypeDeclaration* ctrl_proto = (Ast_ControlTypeDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)ctrl_proto->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)ctrl_proto);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_ExternType* extern_decl = (Ast_ExternType*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)extern_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)extern_decl->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* method_protos = (Ast_List*)extern_decl->method_protos;
  if (method_protos) {
    for (ListItem* li = method_protos->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* proto = li->object;
      visit_function_proto(proto);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_structTypeDeclaration);
  Ast_StructTypeDeclaration* struct_decl = (Ast_StructTypeDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)struct_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)struct_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)struct_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_HeaderTypeDeclaration* header_decl = (Ast_HeaderTypeDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)header_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)header_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)header_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnionDeclaration* union_decl = (Ast_HeaderUnionDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)union_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)union_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)union_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_PackageDeclaration* package_decl = (Ast_PackageDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)package_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)package_decl->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)package_decl->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_parserDeclaration);
  Ast_ParserDeclaration* parser_decl = (Ast_ParserDeclaration*)ast;
  Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
  Ast_Name* name = (Ast_Name*)proto->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)parser_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)parser_decl->ctor_params;
  if (ctor_params) {
    for (ListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_List* local_elements = (Ast_List*)parser_decl->local_elements;
  if (local_elements) {
    for (ListItem* li = local_elements->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* element = li->object;
      visit_local_parser_element(element);
    }
  }
  Ast_List* states = (Ast_List*)parser_decl->states;
  if (states) {
    for (ListItem* li = states->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* state = li->object;
      visit_parser_state(state);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
  Ast_Name* name = (Ast_Name*)proto_decl->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)proto_decl);
  current_scope = push_scope();
  Ast_List* type_params = (Ast_List*)proto_decl->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto_decl->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  Ast_Name* name = (Ast_Name*)inst_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)inst_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(inst_decl->type);
  Ast_List* args = (Ast_List*)inst_decl->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_expression(arg);
    }
  }
}
#endif

#if 0
internal void
visit_typedef(Ast* ast)
{
  assert(ast->kind == AST_typedefDeclaration);
  Ast_TypedefDeclaration* type_decl = (Ast_TypedefDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)type_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast* type_ref = type_decl->type_ref;
  visit_type_ref(type_ref);
}
#endif

#if 0
internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_functionDeclaration);
  Ast_FunctionDeclaration* func_decl = (Ast_FunctionDeclaration*)ast;
  Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)func_decl->proto;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_decl);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_BlockStmt* func_body = (Ast_BlockStmt*)func_decl->stmt;
  if (func_body) {
    Ast_List* stmt_list = (Ast_List*)func_body->stmt_list;
    if (stmt_list) {
      for (ListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        visit_statement(stmt);
      }
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_functionPrototype);
  Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)ast;
  Ast_Name* name = (Ast_Name*)func_proto->name;
  declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_proto);
  current_scope = push_scope();
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (ListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      visit_type_param(type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_ConstDeclaration* const_decl = (Ast_ConstDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)const_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)const_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
}
#endif

#if 0
internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_type) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)enum_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)enum_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      visit_specified_identifier(id);
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_ActionDeclaration* action_decl = (Ast_ActionDeclaration*)ast;
  Ast_Name* name = (Ast_Name*)action_decl->name;
  HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
  NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
  ne->strname = name->strname;
  name_he->object = ne;
  if (!ne->ns_var) {
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)action_decl);
  } else error("At line %d, column %d: redeclaration of name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_List* params = (Ast_List*)action_decl->params;
  if (params) {
    for (ListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      visit_param(param);
    }
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_List* stmt_list = (Ast_List*)action_body->stmt_list;
    if (stmt_list) {
      for (ListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        visit_statement(stmt);
      }
    }
  }
  current_scope = pop_scope();
}
#endif

#if 0
internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_matchKindDeclaration);
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  assert(current_scope->scope_level == 1);
  Ast_List* fields = (Ast_List*)match_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
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
#endif

#if 0
internal void
visit_error_enum(AstWalkContext* context, Ast* ast)
{
  assert (ast->kind == AST_errorDeclaration);
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  current_scope = push_scope();
  Ast_List* fields = (Ast_List*)error_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      visit_enum_field(id);
    }
  }
  current_scope = pop_scope();
}
#endif

internal void
visit(Ast* ast, enum AstWalkDirection direction)
{
  if (ast->kind == AST_p4program) {
    if (direction == WALK_IN) {
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_errorDeclaration) {
    if (direction == WALK_IN) {
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_matchKindDeclaration) {
    assert(current_scope->scope_level == 1);
  } else if (ast->kind == AST_actionDeclaration) {
    if (direction == WALK_IN) {
      Ast_ActionDeclaration* action_decl = (Ast_ActionDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)action_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)action_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_enumDeclaration) {
    if (direction == WALK_IN) {
      Ast_EnumDeclaration* enum_decl = (Ast_EnumDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)enum_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)enum_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_constantDeclaration) {
    if (direction == WALK_IN) {
      Ast_ConstDeclaration* const_decl = (Ast_ConstDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)const_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)const_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_functionPrototype) {
    if (direction == WALK_IN) {
      Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)ast;
      Ast_Name* name = (Ast_Name*)func_proto->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_proto);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_functionDeclaration) {
    if (direction == WALK_IN) {
      Ast_FunctionDeclaration* func_decl = (Ast_FunctionDeclaration*)ast;
      Ast_FunctionPrototype* func_proto = (Ast_FunctionPrototype*)func_decl->proto;
      Ast_Name* name = (Ast_Name*)func_proto->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)func_decl);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_typedefDeclaration) {
    if (direction == WALK_IN) {
      Ast_TypedefDeclaration* type_decl = (Ast_TypedefDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)type_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)type_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_instantiation) {
    if (direction == WALK_IN) {
      Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
      Ast_Name* name = (Ast_Name*)inst_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)inst_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_parserTypeDeclaration) {
    if (direction == WALK_IN) {
      Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
      Ast_Name* name = (Ast_Name*)proto_decl->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)proto_decl);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_parserDeclaration) {
    if (direction == WALK_IN) {
      Ast_ParserDeclaration* parser_decl = (Ast_ParserDeclaration*)ast;
      Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
      Ast_Name* name = (Ast_Name*)proto->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)parser_decl);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_packageTypeDeclaration) {
    if (direction == WALK_IN) {
      Ast_PackageDeclaration* package_decl = (Ast_PackageDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)package_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)package_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_headerUnionDeclaration) {
    if (direction == WALK_IN) {
      Ast_HeaderUnionDeclaration* union_decl = (Ast_HeaderUnionDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)union_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)union_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_headerTypeDeclaration) {
    if (direction == WALK_IN) {
      Ast_HeaderTypeDeclaration* header_decl = (Ast_HeaderTypeDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)header_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)header_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_structTypeDeclaration) {
    if (direction == WALK_IN) {
      Ast_StructTypeDeclaration* struct_decl = (Ast_StructTypeDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)struct_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_type) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)struct_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_externDeclaration) {
    if (direction == WALK_IN) {
      Ast_ExternType* extern_decl = (Ast_ExternType*)ast;
      Ast_Name* name = (Ast_Name*)extern_decl->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)extern_decl);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_controlTypeDeclaration) {
    if (direction == WALK_IN) {
      Ast_ControlTypeDeclaration* ctrl_proto = (Ast_ControlTypeDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)ctrl_proto->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)ctrl_proto);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_specializedType) {
  } else if (ast->kind == AST_headerStackType) {
  } else if (ast->kind == AST_baseTypeError) {
  } else if (ast->kind == AST_baseTypeVoid) {
  } else if (ast->kind == AST_baseTypeString) {
  } else if (ast->kind == AST_baseTypeVarbit) {
  } else if (ast->kind == AST_baseTypeBit) {
  } else if (ast->kind == AST_baseTypeBool) {
  } else if (ast->kind == AST_baseTypeInteger) {
  } else if (ast->kind == AST_structField) {
    if (direction == WALK_IN) {
      Ast_StructField* field = (Ast_StructField*)ast;
      Ast_Name* name = (Ast_Name*)field->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)field);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_parserState) {
    if (direction == WALK_IN) {
      Ast_ParserState* state = (Ast_ParserState*)ast;
      Ast_Name* name = (Ast_Name*)state->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)state);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_selectExpression) {
  } else if (ast->kind == AST_selectCase) {
  } else if (ast->kind == AST_returnStatement) {
  } else if (ast->kind == AST_assignmentStatement) {
  } else if (ast->kind == AST_switchStatement) {
  } else if (ast->kind == AST_conditionalStatement) {
  } else if (ast->kind == AST_switchCase) {
  } else if (ast->kind == AST_defaultKeysetExpression) {
  } else if (ast->kind == AST_tableDeclaration) {
    if (direction == WALK_IN) {
      Ast_TableDeclaration* table_decl = (Ast_TableDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)table_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)table_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_variableDeclaration) {
    if (direction == WALK_IN) {
      Ast_VarDeclaration* var_decl = (Ast_VarDeclaration*)ast;
      Ast_Name* name = (Ast_Name*)var_decl->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)var_decl);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                    name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_tableEntries) {
  } else if (ast->kind == AST_tableKey) {
  } else if (ast->kind == AST_tableProperty) {
  } else if (ast->kind == AST_tableActions) {
  } else if (ast->kind == AST_tableEntry) {
  } else if (ast->kind == AST_tupleKeysetExpression) {
  } else if (ast->kind == AST_defaultKeysetExpression) {
  } else if (ast->kind == AST_keyElement) {
  } else if (ast->kind == AST_actionRef) {
  } else if (ast->kind == AST_blockStatement) {
    if (direction == WALK_IN) {
      current_scope = push_scope();
    } else if (direction == WALK_OUT) {
      current_scope = pop_scope();
    } else assert(0);
  } else if (ast->kind == AST_parameter) {
    if (direction == WALK_IN) {
      Ast_Parameter* param = (Ast_Parameter*)ast;
      Ast_Name* name = (Ast_Name*)param->name;
      HashmapEntry* name_he = hashmap_create_entry_string(&current_scope->sym_table, name->strname);
      NamespaceEntry* ne = arena_push_struct(name_storage, NamespaceEntry);
      ne->strname = name->strname;
      name_he->object = ne;
      if (!ne->ns_var) {
        declare_var_name(current_scope, name->strname, name->line_no, name->column_no, (Ast*)param);
      } else error("At line %d, column %d: redeclaration of name `%s`.",
                  name->line_no, name->column_no, name->strname);
    } else if (direction == WALK_OUT) {
    } else assert(0);
  } else if (ast->kind == AST_name) {
  } else if (ast->kind == AST_specifiedIdentifier) {
  } else if (AST_integerLiteral) {
  } else if (AST_booleanLiteral) {
  } else if (AST_stringLiteral) {
  } else if (ast->kind == AST_kvPair) {
  } else if (ast->kind == AST_arraySubscript) {
  } else if (ast->kind == AST_castExpression) {
  } else if (ast->kind == AST_memberSelector) {
  } else if (ast->kind == AST_functionCall) {
  } else if (ast->kind == AST_unaryExpression) {
  } else if (ast->kind == AST_binaryExpression) {
  }
  else assert(0);
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
    declare_type_name(root_scope, void_type->strname, 0, 0, (Ast*)void_type);
  }
  {
    Ast_Name* bool_type = arena_push_struct(name_storage, Ast_Name);
    bool_type->kind = AST_name;
    bool_type->id = ++p4program->last_node_id;
    bool_type->strname = "bool";
    declare_type_name(root_scope, bool_type->strname, 0, 0, (Ast*)bool_type);
  }
  {
    Ast_Name* int_type = arena_push_struct(name_storage, Ast_Name);
    int_type->kind = AST_name;
    int_type->id = ++p4program->last_node_id;
    int_type->strname = "int";
    declare_type_name(root_scope, int_type->strname, 0, 0, (Ast*)int_type);
  }
  {
    Ast_Name* bit_type = arena_push_struct(name_storage, Ast_Name);
    bit_type->kind = AST_name;
    bit_type->id = ++p4program->last_node_id;
    bit_type->strname = "bit";
    declare_type_name(root_scope, bit_type->strname, 0, 0, (Ast*)bit_type);
  }
  {
    Ast_Name* varbit_type = arena_push_struct(name_storage, Ast_Name);
    varbit_type->kind = AST_name;
    varbit_type->id = ++p4program->last_node_id;
    varbit_type->strname = "varbit";
    declare_type_name(root_scope, varbit_type->strname, 0, 0, (Ast*)varbit_type);
  }
  {
    Ast_Name* string_type = arena_push_struct(name_storage, Ast_Name);
    string_type->kind = AST_name;
    string_type->id = ++p4program->last_node_id;
    string_type->strname = "string";
    declare_type_name(root_scope, string_type->strname, 0, 0, (Ast*)string_type);
  }
  {
    Ast_Name* error_type = arena_push_struct(name_storage, Ast_Name);
    error_type->kind = AST_name;
    error_type->id = ++p4program->last_node_id;
    error_type->strname = "error";
    declare_type_name(root_scope, error_type->strname, 0, 0, (Ast*)error_type);
  }
  {
    Ast_Name* match_type = arena_push_struct(name_storage, Ast_Name);
    match_type->kind = AST_name;
    match_type->id = ++p4program->last_node_id;
    match_type->strname = "match_kind";
    declare_type_name(root_scope, match_type->strname, 0, 0, (Ast*)match_type);
  }
  {
    Ast_Name* accept_state = arena_push_struct(name_storage, Ast_Name);
    accept_state->kind = AST_name;
    accept_state->id = ++p4program->last_node_id;
    accept_state->strname = "accept";
    declare_var_name(root_scope, accept_state->strname, 0, 0, (Ast*)accept_state);
  }
  {
    Ast_Name* reject_state = arena_push_struct(name_storage, Ast_Name);
    reject_state->kind = AST_name;
    reject_state->id = ++p4program->last_node_id;
    reject_state->strname = "reject";
    declare_var_name(root_scope, reject_state->strname, 0, 0, (Ast*)reject_state);
  }

  traverse_p4program(p4program, visit);

  current_scope = pop_scope();
  assert(current_scope == 0);
  return root_scope;
}
