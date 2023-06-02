#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal void visit_expression(AstTraversalHooks* hooks, Ast* ast);
internal void visit_type_ref(AstTraversalHooks* hooks, Ast* ast);
internal void visit_statement(AstTraversalHooks* hooks, Ast* ast);
internal void visit_control_proto(AstTraversalHooks* hooks, Ast* ast);
internal void visit_struct(AstTraversalHooks* hooks, Ast* ast);
internal void visit_header(AstTraversalHooks* hooks, Ast* ast);
internal void visit_header_union(AstTraversalHooks* hooks, Ast* ast);
internal void visit_instantiation(AstTraversalHooks* hooks, Ast* ast);
internal void visit_function_proto(AstTraversalHooks* hooks, Ast* ast);
internal void visit_const(AstTraversalHooks* hooks, Ast* ast);
internal void visit_action(AstTraversalHooks* hooks, Ast* ast);
internal void visit_parser_proto(AstTraversalHooks* hooks, Ast* ast);

internal void
visit_binary_expr(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_binaryExpression);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  hooks->visit_expression(AST_binaryExpression, HOOK_ENTER_AST, expr->left_operand);
  visit_expression(expr->left_operand);
  hooks->visit_expression(AST_binaryExpression, HOOK_EXIT_AST, expr->left_operand);
  hooks->visit_expression(AST_binaryExpression, HOOK_ENTER_AST, expr->right_operand);
  visit_expression(expr->right_operand);
  hooks->visit_expression(AST_binaryExpression, HOOK_EXIT_AST, expr->right_operand);
}

internal void
visit_unary_expr(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_unaryExpression);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  hooks->visit_expression(AST_unaryExpression, HOOK_ENTER_AST, expr->operand);
  visit_expression(expr->operand);
  hooks->visit_expression(AST_unaryExpression, HOOK_EXIT_AST, expr->operand);
}

internal void
visit_name_identifier(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_name);
}

internal void
visit_function_call(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* expr = (Ast_FunctionCall*)ast;
  hooks->visit_expression(AST_functionCall, HOOK_ENTER_AST, expr->callee_expr);
  visit_expression(expr->callee_expr);
  hooks->visit_expression(AST_functionCall, HOOK_EXIT_AST, expr->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)expr->callee_expr;
  Ast_List* type_args = (Ast_List*)callee_expr->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      hooks->visit_type_ref(AST_functionCall, HOOK_ENTER_AST, type_arg);
      visit_type_ref(type_arg);
      hooks->visit_type_ref(AST_functionCall, HOOK_EXIT_AST, type_arg);
    }
  }
  Ast_List* args = (Ast_List*)expr->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      hooks->visit_expression(AST_functionCall, HOOK_ENTER_AST, arg);
      visit_expression(arg);
      hooks->visit_expression(AST_functionCall, HOOK_EXIT_AST, arg);
    }
  }
}

internal void
visit_member_select(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_memberSelectExpression);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  hooks->visit_expression(AST_memberSelectExpression, HOOK_ENTER_AST, expr->lhs_expr);
  visit_expression(expr->lhs_expr);
  hooks->visit_expression(AST_memberSelectExpression, HOOK_EXIT_AST, expr->lhs_expr);
  hooks->visit_expression(AST_memberSelectExpression, HOOK_ENTER_AST, expr->member_name);
  visit_expression(expr->member_name);
  hooks->visit_expression(AST_memberSelectExpression, HOOK_EXIT_AST, expr->member_name);
}

internal void
visit_expression_list(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_exprListExpression);
  Ast_ExprListExpression* expr = (Ast_ExprListExpression*)ast;
  Ast_List* expr_list = (Ast_List*)expr->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr_expr = li->object;
      hooks->visit_expression(AST_exprListExpression, HOOK_ENTER_AST, expr_expr);
      visit_expression(expr_expr);
      hooks->visit_expression(AST_exprListExpression, HOOK_EXIT_AST, expr_expr);
    }
  }
}

internal void
visit_cast_expr(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_CastExpr* expr = (Ast_CastExpr*)ast;
  hooks->visit_type_ref(AST_castExpression, HOOK_ENTER_AST, expr->to_type);
  visit_type_ref(expr->to_type);
  hooks->visit_type_ref(AST_castExpression, HOOK_EXIT_AST, expr->to_type);
  hooks->visit_expression(AST_castExpression, HOOK_ENTER_AST, expr->expr);
  visit_expression(expr->expr);
  hooks->visit_expression(AST_castExpression, HOOK_EXIT_AST, expr->expr);
}

internal void
visit_array_subscript(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_ArraySubscript* expr = (Ast_ArraySubscript*)ast;
  hooks->visit_expression(AST_arraySubscript, HOOK_ENTER_AST, expr->index);
  visit_expression(expr->index);
  hooks->visit_expression(AST_arraySubscript, HOOK_EXIT_AST, expr->index);
  if (expr->end_index) {
    hooks->visit_expression(AST_arraySubscript, HOOK_ENTER_AST, expr->end_index);
    visit_expression(expr->end_index);
    hooks->visit_expression(AST_arraySubscript, HOOK_EXIT_AST, expr->end_index);
  }
}

internal void
visit_kvpair_expr(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  Ast_KVPairExpr* expr = (Ast_KVPairExpr*)ast;
  hooks->visit_expression(AST_kvPairExpression, HOOK_ENTER_AST, expr->name);
  visit_expression(expr->name);
  hooks->visit_expression(AST_kvPairExpression, HOOK_EXIT_AST, expr->name);
  hooks->visit_expression(AST_kvPairExpression, HOOK_ENTER_AST, expr->expr);
  visit_expression(expr->expr);
  hooks->visit_expression(AST_kvPairExpression, HOOK_EXIT_AST, expr->expr);
}

internal void
visit_int_literal(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_bool_literal(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_string_literal(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_expression(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_binaryExpression) {
    hooks->visit_binary_expr(AST_expression, HOOK_ENTER_AST, ast);
    visit_binary_expr(ast);
    hooks->visit_binary_expr(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_unaryExpression) {
    hooks->visit_unary_expr(AST_expression, HOOK_ENTER_AST, ast);
    visit_unary_expr(ast);
    hooks->visit_unary_expr(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_name) {
    hooks->visit_name_identifier(AST_expression, HOOK_ENTER_AST, ast);
    visit_name_identifier(ast);
    hooks->visit_name_identifier(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_functionCall) {
    hooks->visit_function_call(AST_expression, HOOK_ENTER_AST, ast);
    visit_function_call(ast);
    hooks->visit_function_call(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_memberSelectExpression) {
    hooks->visit_member_select(AST_expression, HOOK_ENTER_AST, ast);
    visit_member_select(ast);
    hooks->visit_member_select(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_exprListExpression) {
    hooks->visit_expression_list(AST_expression, HOOK_ENTER_AST, ast);
    visit_expression_list(ast);
    hooks->visit_expression_list(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_castExpression) {
    hooks->visit_cast_expr(AST_expression, HOOK_ENTER_AST, ast);
    visit_cast_expr(ast);
    hooks->visit_cast_expr(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_arraySubscript) {
    hooks->visit_array_subscript(AST_expression, HOOK_ENTER_AST, ast);
    visit_array_subscript(ast);
    hooks->visit_array_subscript(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_kvPairExpression) {
    hooks->visit_kvpair_expr(AST_expression, HOOK_ENTER_AST, ast);
    visit_kvpair_expr(ast);
    hooks->visit_kvpair_expr(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_integerLiteral) {
    hooks->visit_int_literal(AST_expression, HOOK_ENTER_AST, ast);
    visit_int_literal(ast);
    hooks->visit_int_literal(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_booleanLiteral) {
    hooks->visit_bool_literal(AST_expression, HOOK_ENTER_AST, ast);
    visit_bool_literal(ast);
    hooks->visit_bool_literal(AST_expression, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_stringLiteral) {
    hooks->visit_string_literal(AST_expression, HOOK_ENTER_AST, ast);
    visit_string_literal(ast);
    hooks->visit_string_literal(AST_expression, HOOK_EXIT_AST, ast);
  }
  else assert(0);
}

internal void
visit_param(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_parameter);
  Ast_Param* param = (Ast_Param*)ast;
  hooks->visit_type_ref(AST_parameter, HOOK_ENTER_AST, ast);
  visit_type_ref(param->type);
  hooks->visit_type_ref(AST_parameter, HOOK_EXIT_AST, ast);
}

internal void
visit_type_param(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_name);
#if 0
  if (!ne->ns_type) {
    /* Declaration of a type parameter. */
  }
#endif
  hooks->visit_type_ref(AST_typeParameter, HOOK_ENTER_AST, ast);
  visit_type_ref(ast);
  hooks->visit_type_ref(AST_typeParameter, HOOK_EXIT_AST, ast);
}

internal void
visit_block_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_blockStatement);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
  if (stmt_list) {
    for (DListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      hooks->visit_statement(AST_blockStatement, HOOK_ENTER_AST, decl);
      visit_statement(decl);
      hooks->visit_statement(AST_blockStatement, HOOK_EXIT_AST, decl);
    }
  }
}

internal void
visit_action_ref(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_actionRef);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  hooks->visit_expression(AST_actionRef, HOOK_ENTER_AST, action->name);
  visit_expression(action->name);
  hooks->visit_expression(AST_actionRef, HOOK_EXIT_AST, action->name);
  Ast_List* args = (Ast_List*)action->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      hooks->visit_expression(AST_actionRef, HOOK_ENTER_AST, arg);
      visit_expression(arg);
      hooks->visit_expression(AST_actionRef, HOOK_EXIT_AST, arg);
    }
  }
}

internal void
visit_table_keyelem(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_keyElement);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  hooks->visit_expression(AST_keyElement, HOOK_ENTER_AST, keyelem->expr);
  visit_expression(keyelem->expr);
  hooks->visit_expression(AST_keyElement, HOOK_EXIT_AST, keyelem->expr);
  hooks->visit_expression(AST_keyElement, HOOK_ENTER_AST, keyelem->name);
  visit_expression(keyelem->name);
  hooks->visit_expression(AST_keyElement, HOOK_EXIT_AST, keyelem->name);
}

internal void
visit_default_keyset(AstTraversalHooks* hooks, Ast *ast)
{
  // pass
}

internal void
visit_dontcare_keyset(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_keyset_expr(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_defaultKeysetExpression) {
    hooks->visit_default_keyset(AST_keysetExpr, HOOK_ENTER_AST, ast);
    visit_default_keyset(ast);
    hooks->visit_default_keyset(AST_keysetExpr, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_dontcareArgument) {
    hooks->visit_dontcare_keyset(AST_keysetExpr, HOOK_ENTER_AST, ast);
    visit_dontcare_keyset(ast);
    hooks->visit_dontcare_keyset(AST_keysetExpr, HOOK_EXIT_AST, ast);
  } else {
    hooks->visit_expression(AST_keysetExpr, HOOK_ENTER_AST, ast);
    visit_expression(ast);
    hooks->visit_expression(AST_keysetExpr, HOOK_EXIT_AST, ast);
  }
}

internal void
visit_tuple_keyset(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tupleKeysetExpression);
  Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
  Ast_List* expr_list = (Ast_List*)keyset->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      hooks->visit_keyset_expr(AST_tupleKeysetExpression, HOOK_ENTER_AST, expr);
      visit_keyset_expr(expr);
      hooks->visit_keyset_expr(AST_tupleKeysetExpression, HOOK_EXIT_AST, expr);
    }
  }
}

internal void
visit_select_keyset(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_tupleKeysetExpression) {
    hooks->visit_tuple_keyset(AST_selectKeyset, HOOK_ENTER_AST, ast);
    visit_tuple_keyset(ast);
    hooks->visit_tuple_keyset(AST_selectKeyset, HOOK_EXIT_AST, ast);
  } else {
    hooks->visit_keyset_expr(AST_selectKeyset, HOOK_ENTER_AST, ast);
    visit_keyset_expr(ast);
    hooks->visit_keyset_expr(AST_selectKeyset, HOOK_EXIT_AST, ast);
  }
}

internal void
visit_table_entry(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableEntry);
  Ast_TableEntry* entry = (Ast_TableEntry*)ast;
  hooks->visit_select_keyset(AST_tableEntry, HOOK_ENTER_AST, entry->keyset);
  visit_select_keyset(entry->keyset);
  hooks->visit_select_keyset(AST_tableEntry, HOOK_EXIT_AST, entry->keyset);
  hooks->visit_action_ref(AST_tableEntry, HOOK_ENTER_AST, entry->action);
  visit_action_ref(entry->action);
  hooks->visit_action_ref(AST_tableEntry, HOOK_EXIT_AST, entry->action);
}

internal void
visit_table_actions(AstTraversalHooks* hooks, Ast *ast)
{
  assert(ast->kind == AST_tableActions);
  Ast_TableActions* prop = (Ast_TableActions*)ast;
  Ast_List* action_list = (Ast_List*)prop->action_list;
  if (action_list) {
    DListItem* li = action_list->members.sentinel.next;
    while (li) {
      Ast* action = li->object;
      hooks->visit_action_ref(AST_tableActions, HOOK_ENTER_AST, action);
      visit_action_ref(action);
      hooks->visit_action_ref(AST_tableActions, HOOK_EXIT_AST, action);
      li = li->next;
    }
  }
}

internal void
visit_table_single_entry(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableProperty);
  Ast_TableProperty* prop = (Ast_TableProperty*)ast;
  if (prop->init_expr) {
    hooks->visit_expression(AST_tableProperty, HOOK_ENTER_AST, prop->init_expr);
    visit_expression(prop->init_expr);
    hooks->visit_expression(AST_tableProperty, HOOK_EXIT_AST, prop->init_expr);
  }
}

internal void
visit_table_key(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableKey);
  Ast_TableKey* prop = (Ast_TableKey*)ast;
  Ast_List* keyelem_list = (Ast_List*)prop->keyelem_list;
  if (keyelem_list) {
    for (DListItem* li = keyelem_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* keyelem = li->object;
      hooks->visit_table_keyelem(AST_tableKey, HOOK_ENTER_AST, keyelem);
      visit_table_keyelem(keyelem);
      hooks->visit_table_keyelem(AST_tableKey, HOOK_EXIT_AST, keyelem);
    }
  }
}

internal void
visit_table_entries(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableEntries);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_List* entries = (Ast_List*)prop->entries;
  if (entries) {
    for (DListItem* li = entries->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* entry = li->object;
      hooks->visit_table_entry(AST_tableEntries, HOOK_ENTER_AST, entry);
      visit_table_entry(entry);
      hooks->visit_table_entry(AST_tableEntries, HOOK_EXIT_AST, entry);
    }
  }
}

internal void
visit_table_property(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableProperty);
  if (ast->kind == AST_tableActions) {
    hooks->visit_table_actions(AST_tableProperty, HOOK_ENTER_AST, ast);
    visit_table_actions(ast);
    hooks->visit_table_actions(AST_tableProperty, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_tableProperty) {
    hooks->visit_table_single_entry(AST_tableProperty, HOOK_ENTER_AST, ast);
    visit_table_single_entry(ast);
    hooks->visit_table_single_entry(AST_tableProperty, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_tableKey) {
    hooks->visit_table_key(AST_tableProperty, HOOK_ENTER_AST, ast);
    visit_table_key(ast);
    hooks->visit_table_key(AST_tableProperty, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_tableEntries) {
    hooks->visit_table_entries(AST_tableProperty, HOOK_ENTER_AST, ast);
    visit_table_entries(ast);
    hooks->visit_table_entries(AST_tableProperty, HOOK_EXIT_AST, ast);
  }
  else assert(0);
}

internal void
visit_switch_default(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_switch_label(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_defaultKeysetExpression) {
    hooks->visit_switch_default(AST_switchLabel, HOOK_ENTER_AST, ast);
    visit_switch_default(ast);
    hooks->visit_switch_default(AST_switchLabel, HOOK_EXIT_AST, ast);
  } else {
    hooks->visit_expression(AST_switchLabel, HOOK_ENTER_AST, ast);
    visit_expression(ast);
    hooks->visit_expression(AST_switchLabel, HOOK_EXIT_AST, ast);
  }
}

internal void
visit_switch_case(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_switchCase);
  Ast_SwitchCase* switch_case = (Ast_SwitchCase*)ast;
  hooks->visit_switch_label(AST_switchCase, HOOK_ENTER_AST, switch_case->label);
  visit_switch_label(switch_case->label);
  hooks->visit_switch_label(AST_switchCase, HOOK_EXIT_AST, switch_case->label);
  hooks->visit_block_stmt(AST_switchCase, HOOK_ENTER_AST, switch_case->stmt);
  visit_block_stmt(switch_case->stmt);
  hooks->visit_block_stmt(AST_switchCase, HOOK_EXIT_AST, switch_case->stmt);
}

internal void
visit_var_decl(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_variableDeclaration);
  Ast_Var* var_decl = (Ast_Var*)ast;
  hooks->visit_type_ref(AST_variableDeclaration, HOOK_ENTER_AST, var_decl->type);
  visit_type_ref(var_decl->type);
  hooks->visit_type_ref(AST_variableDeclaration, HOOK_EXIT_AST, var_decl->type);
  if (var_decl->init_expr) {
    hooks->visit_expression(AST_variableDeclaration, HOOK_ENTER_AST, var_decl->init_expr);
    visit_expression(var_decl->init_expr);
    hooks->visit_expression(AST_variableDeclaration, HOOK_EXIT_AST, var_decl->init_expr);
  }
}

internal void
visit_table(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tableDeclaration);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
  if (prop_list) {
    for (DListItem* li = prop_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* prop = li->object;
      hooks->visit_table_property(AST_tableDeclaration, HOOK_ENTER_AST, prop);
      visit_table_property(prop);
      hooks->visit_table_property(AST_tableDeclaration, HOOK_EXIT_AST, prop);
    }
  }
}

internal void
visit_if_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_conditionalStatement);
  Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
  hooks->visit_expression(AST_conditionalStatement, HOOK_ENTER_AST, stmt->cond_expr);
  visit_expression(stmt->cond_expr);
  hooks->visit_expression(AST_conditionalStatement, HOOK_EXIT_AST, stmt->cond_expr);
  hooks->visit_statement(AST_conditionalStatement, HOOK_ENTER_AST, stmt->stmt);
  if (stmt->else_stmt) {
    hooks->visit_statement(AST_conditionalStatement, HOOK_ENTER_AST, stmt->else_stmt);
    visit_statement(stmt->else_stmt);
    hooks->visit_statement(AST_conditionalStatement, HOOK_EXIT_AST, stmt->else_stmt);
  }
}

internal void
visit_switch_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_switchStatement);
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  hooks->visit_expression(AST_switchStatement, HOOK_ENTER_AST, stmt->expr);
  visit_expression(stmt->expr);
  hooks->visit_expression(AST_switchStatement, HOOK_EXIT_AST, stmt->expr);
  Ast_List* switch_cases = (Ast_List*)stmt->switch_cases;
  if (switch_cases) {
    for (DListItem* li = switch_cases->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* switch_case = li->object;
      hooks->visit_expression(AST_switchStatement, HOOK_ENTER_AST, switch_case);
      visit_switch_case(switch_case);
      hooks->visit_expression(AST_switchStatement, HOOK_EXIT_AST, switch_case);
    }
  }
}

internal void
visit_assignment_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_assignmentStatement);
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  hooks->visit_expression(AST_assignmentStatement, HOOK_ENTER_AST, stmt->lvalue);
  visit_expression(stmt->lvalue);
  hooks->visit_expression(AST_assignmentStatement, HOOK_EXIT_AST, stmt->lvalue);
  hooks->visit_expression(AST_assignmentStatement, HOOK_ENTER_AST, stmt->expr);
  visit_expression(assign_expr);
  hooks->visit_expression(AST_assignmentStatement, HOOK_EXIT_AST, stmt->expr);
}

internal void
visit_return_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_returnStatement);
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    hooks->visit_expression(AST_returnStatement, HOOK_ENTER_AST, stmt->expr);
    visit_expression(stmt->expr);
    hooks->visit_expression(AST_returnStatement, HOOK_EXIT_AST, stmt->expr);
  }
}

internal void
visit_exit_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_empty_stmt(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_statement(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_variableDeclaration) {
    hooks->visit_var_decl(AST_statement, HOOK_ENTER_AST, ast);
    visit_var_decl(ast);
    hooks->visit_var_decl(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_actionDeclaration) {
    hooks->visit_action(AST_statement, HOOK_ENTER_AST, ast);
    visit_action(ast);
    hooks->visit_action(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_blockStatement) {
    hooks->visit_block_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_block_stmt(ast);
    hooks->visit_block_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_instantiation) {
    hooks->visit_instantiation(AST_statement, HOOK_ENTER_AST, ast);
    visit_instantiation(ast);
    hooks->visit_instantiation(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_tableDeclaration) {
    hooks->visit_table(AST_statement, HOOK_ENTER_AST, ast);
    visit_table(ast);
    hooks->visit_table(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_conditionalStatement) {
    hooks->visit_if_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_if_stmt(ast);
    hooks->visit_if_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_switchStatement) {
    hooks->visit_switch_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_switch_stmt(ast);
    hooks->visit_switch_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_assignmentStatement) {
    hooks->visit_assignment_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_assignment_stmt(ast);
    hooks->visit_assignment_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_functionCall) {
    hooks->visit_function_call(AST_statement, HOOK_ENTER_AST, ast);
    visit_function_call(ast);
    hooks->visit_function_call(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_returnStatement) {
    hooks->visit_return_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_return_stmt(ast);
    hooks->visit_return_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_exitStatement) {
    hooks->visit_exit_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_exit_stmt(ast);
    hooks->visit_exit_stmt(AST_statement, HOOK_EXIT_AST, ast);
  } else if(ast->kind == AST_emptyStatement) {
    hooks->visit_empty_stmt(AST_statement, HOOK_ENTER_AST, ast);
    visit_empty_stmt(ast);
    hooks->visit_empty_stmt(AST_statement, HOOK_EXIT_AST, ast);
  }
  else assert(0);
}

internal void
visit_local_parser_element(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_constantDeclaration) {
    hooks->visit_const(AST_parserLocalElement, HOOK_ENTER_AST, ast);
    visit_const(ast);
    hooks->visit_const(AST_parserLocalElement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_instantiation) {
    hooks->visit_instantiation(AST_parserLocalElement, HOOK_ENTER_AST, ast);
    visit_instantiation(ast);
    hooks->visit_instantiation(AST_parserLocalElement, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_variableDeclaration) {
    hooks->visit_statement(AST_parserLocalElement, HOOK_ENTER_AST, ast);
    visit_statement(ast);
    hooks->visit_statement(AST_parserLocalElement, HOOK_EXIT_AST, ast);
  } else assert(0);
}

internal void
visit_transition_select_case(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_selectCase);
  Ast_SelectCase* select_case = (Ast_SelectCase*)ast;
  hooks->visit_select_keyset(AST_selectCase, HOOK_ENTER_AST, select_case->keyset);
  visit_select_keyset(select_case->keyset);
  hooks->visit_select_keyset(AST_selectCase, HOOK_EXIT_AST, select_case->keyset);
  hooks->visit_expression(AST_selectCase, HOOK_ENTER_AST, select_case->name)
  visit_expression(select_case->name);
  hooks->visit_expression(AST_selectCase, HOOK_EXIT_AST, select_case->name)
}

internal void
visit_select_expr(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_selectExpression);
  Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
  Ast_List* expr_list = (Ast_List*)trans_stmt->expr_list;
  if (expr_list) {
    for (DListItem* li = expr_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* expr = li->object;
      hooks->visit_expression(AST_selectExpression, HOOK_ENTER_AST, expr);
      visit_expression(expr);
      hooks->visit_expression(AST_selectExpression, HOOK_EXIT_AST, expr);
    }
  }
  Ast_List* case_list = (Ast_List*)trans_stmt->case_list;
  if (case_list) {
    for (DListItem* li = case_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* select_case = li->object;
      hooks->visit_transition_select_case(AST_selectExpression, HOOK_ENTER_AST, select_case);
      visit_transition_select_case(select_case);
      hooks->visit_transition_select_case(AST_selectExpression, HOOK_EXIT_AST, select_case);
    }
  }
}

internal void
visit_parser_transition(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_name) {
    hooks->visit_expression(AST_parserTransition, HOOK_ENTER_AST, ast);
    visit_expression(ast);
    hooks->visit_expression(AST_parserTransition, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_selectExpression) {
    hooks->visit_select_expr(AST_parserTransition, HOOK_ENTER_AST, ast);
    visit_select_expr(ast);
    hooks->visit_select_expr(AST_parserTransition, HOOK_EXIT_AST, ast);
  }
  else assert(0);
}

internal void
visit_parser_state(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_parserState);
  Ast_ParserState* state = (Ast_ParserState*)ast;
  Ast_List* stmt_list = (Ast_List*)state->stmt_list;
  if (stmt_list) {
    for (DListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* stmt = li->object;
      hooks->visit_statement(AST_parserState, HOOK_ENTER_AST, stmt);
      visit_statement(stmt);
      hooks->visit_statement(AST_parserState, HOOK_EXIT_AST, stmt);
    }
  }
  hooks->visit_transition(AST_parserState, HOOK_ENTER_AST, state->trans_stmt);
  visit_parser_transition(state->trans_stmt);
  hooks->visit_transition(AST_parserState, HOOK_EXIT_AST, state->trans_stmt);
}

internal void
visit_struct_field(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_structField);
  Ast_StructField* field = (Ast_StructField*)ast;
  hooks->visit_type_ref(AST_structField, HOOK_ENTER_AST, field->type);
  visit_type_ref(field->type);
  hooks->visit_type_ref(AST_structField, HOOK_EXIT_AST, field->type);
}

internal void
visit_bool_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeBool);
  Ast_BoolType* bool_type = (Ast_BoolType*)ast;
  hooks->visit_expression(AST_baseTypeBool, HOOK_ENTER_AST, bool_type->name);
  visit_expression(bool_type->name);
  hooks->visit_expression(AST_baseTypeBool, HOOK_EXIT_AST, bool_type->name);
}

internal void
visit_int_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeInt);
  Ast_IntType* int_type = (Ast_IntType*)ast;
  hooks->visit_expression(AST_baseTypeInt, HOOK_ENTER_AST, int_type->name);
  visit_expression(int_type->name);
  hooks->visit_expression(AST_baseTypeInt, HOOK_EXIT_AST, int_type->name);
}

internal void
visit_bit_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
  Ast_BitType* bit_type = (Ast_BitType*)ast;
  hooks->visit_expression(AST_baseTypeBit, HOOK_ENTER_AST, bit_type->name);
  visit_expression(bit_type->name);
  hooks->visit_expression(AST_baseTypeBit, HOOK_EXIT_AST, bit_type->name);
}

internal void
visit_varbit_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
  Ast_VarbitType* varbit_type = (Ast_VarbitType*)ast;
  hooks->visit_expression(AST_baseTypeVarbit, HOOK_ENTER_AST, varbit_type->name);
  visit_expression(varbit_type->name);
  hooks->visit_expression(AST_baseTypeVarbit, HOOK_EXIT_AST, varbit_type->name);
}

internal void
visit_string_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
  Ast_StringType* string_type = (Ast_StringType*)ast;
  hooks->visit_expression(AST_baseTypeString, HOOK_ENTER_AST, string_type->name);
  visit_expression(string_type->name);
  hooks->visit_expression(AST_baseTypeString, HOOK_EXIT_AST, string_type->name);
}

internal void
visit_void_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
  Ast_VoidType* void_type = (Ast_VoidType*)ast;
  hooks->visit_expression(AST_baseTypeVoid, HOOK_ENTER_AST, void_type->name);
  visit_expression(void_type->name);
  hooks->visit_expression(AST_baseTypeVoid, HOOK_EXIT_AST, void_type->name);
}

internal void
visit_error_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
  Ast_ErrorType* error_type = (Ast_ErrorType*)ast; 
  hooks->visit_expression(AST_baseTypeError, HOOK_ENTER_AST, error_type->name);
  visit_expression(error_type->name);
  hooks->visit_expression(AST_baseTypeError, HOOK_EXIT_AST, error_type->name);
}

internal void
visit_header_stack(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_headerStackType);
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  hooks->visit_expression(AST_headerStackType, HOOK_ENTER_AST, type_ref->name);
  visit_expression(type_ref->name);
  hooks->visit_expression(AST_headerStackType, HOOK_EXIT_AST, type_ref->name);
  Ast* stack_expr = type_ref->stack_expr;
  hooks->visit_expression(AST_headerStackType, HOOK_ENTER_AST, stack_expr);
  visit_expression(stack_expr);
  hooks->visit_expression(AST_headerStackType, HOOK_EXIT_AST, stack_expr);
}

internal void
visit_name_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_name);
#if 0
  if (!ne->ns_type) {
    /* Declaration of a type parameter. */
  }
#endif
  hooks->visit_expression(AST_name, HOOK_ENTER_AST, ast);
  visit_expression(ast);
  hooks->visit_expression(AST_name, HOOK_EXIT_AST, ast);
}

internal void
visit_specialized_type(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_specializedType);
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  hooks->visit_expression(AST_specializedType, HOOK_ENTER_AST, speclzd_type->name);
  visit_expression(speclzd_type->name);
  hooks->visit_expression(AST_specializedType, HOOK_EXIT_AST, speclzd_type->name);
  Ast_List* type_args = (Ast_List*)speclzd_type->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      hooks->visit_type_ref(AST_specializedType, HOOK_ENTER_AST, type_arg);
      visit_type_ref(type_arg);
      hooks->visit_type_ref(AST_specializedType, HOOK_EXIT_AST, type_arg);
    }
  }
}

internal void
visit_tuple(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_tupleType);
  Ast_Tuple* type_ref = (Ast_Tuple*)ast;
  Ast_List* type_args = (Ast_List*)type_ref->type_args;
  if (type_args) {
    for (DListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      hooks->visit_type_ref(AST_tupleType, HOOK_ENTER_AST, type_arg);
      visit_type_ref(type_arg);
      hooks->visit_type_ref(AST_tupleType, HOOK_EXIT_AST, type_arg);
    }
  }
}

internal void
visit_dontcare_type(AstTraversalHooks* hooks, Ast* ast)
{
  // pass
}

internal void
visit_type_ref(AstTraversalHooks* hooks, Ast* ast)
{
  if (ast->kind == AST_baseTypeBool) {
    hooks->visit_bool_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_bool_type(ast);
    hooks->visit_bool_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeInt) {
    hooks->visit_int_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_int_type(ast);
    hooks->visit_int_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeBit) {
    hooks->visit_bit_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_bit_type(ast);
    hooks->visit_bit_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeVarbit) {
    hooks->visit_varbit_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_varbit_type(ast);
    hooks->visit_varbit_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeString) {
    hooks->visit_string_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_string_type(ast);
    hooks->visit_string_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeVoid) {
    hooks->visit_void_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_void_type(ast);
    hooks->visit_void_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_baseTypeError) {
    hooks->visit_error_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_error_type(ast);
    hooks->visit_error_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_headerStackType) {
    hooks->visit_header_stack(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_header_stack(ast);
    hooks->visit_header_stack(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_name) {
    hooks->visit_name_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_name_type(ast);
    hooks->visit_name_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_specializedType) {
    hooks->visit_specialized_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_specialized_type(ast);
    hooks->visit_specialized_type(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_tupleType) {
    hooks->visit_tuple(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_tuple(ast);
    hooks->visit_tuple(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_structTypeDeclaration) {
    hooks->visit_struct(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_struct(ast);
    hooks->visit_struct(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_headerTypeDeclaration) {
    hooks->visit_header(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_header(ast);
    hooks->visit_header(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_headerUnionDeclaration) {
    hooks->visit_header_union(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_header_union(ast);
    hooks->visit_header_union(AST_typeRef, HOOK_EXIT_AST, ast);
  } else if (ast->kind == AST_dontcareArgument) {
    hooks->visit_dontcare_type(AST_typeRef, HOOK_ENTER_AST, ast);
    visit_dontcare_type(ast);
    hooks->visit_dontcare_type(AST_typeRef, HOOK_EXIT_AST, ast);
  }
  else assert(0);
}

internal void
visit_enum_field(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_name);
  // pass
}

internal void
visit_specified_identifier(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_specifiedIdentifier);
  Ast_SpecifiedIdent* id = (Ast_SpecifiedIdent*)ast;
  hooks->visit_enum_field(AST_specifiedIdentifier, HOOK_ENTER_AST, id->name);
  visit_enum_field(id->name);
  hooks->visit_enum_field(AST_specifiedIdentifier, HOOK_EXIT_AST, id->name);
  Ast* init_expr = id->init_expr;
  if (init_expr) {
    hooks->visit_expression(AST_specifiedIdentifier, HOOK_ENTER_AST, init_expr);
    visit_expression(init_expr);
    hooks->visit_expression(AST_specifiedIdentifier, HOOK_EXIT_AST, init_expr);
  }
}

internal void
visit_control(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_controlDeclaration);
  Ast_Control* ctrl_decl = (Ast_Control*)ast;
  Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ctrl_decl->proto;
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_controlDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_controlDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_controlDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_controlDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)ctrl_decl->ctor_params;
  if (ctor_params) {
    for (DListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_controlDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_controlDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_List* local_decls = (Ast_List*)ctrl_decl->local_decls;
  if (local_decls) {
    for (DListItem* li = local_decls->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      hooks->visit_statement(AST_controlDeclaration, HOOK_ENTER_AST, decl);
      visit_statement(decl);
      hooks->visit_statement(AST_controlDeclaration, HOOK_EXIT_AST, decl);
    }
  }
  if (ctrl_decl->apply_stmt) {
    hooks->visit_block_stmt(AST_controlDeclaration, HOOK_ENTER_AST, ctrl_decl->apply_stmt);
    visit_block_stmt(ctrl_decl->apply_stmt);
    hooks->visit_block_stmt(AST_controlDeclaration, HOOK_EXIT_AST, ctrl_decl->apply_stmt);
  }
}

internal void
visit_control_proto(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ast;
  Ast_List* type_params = (Ast_List*)ctrl_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_controlTypeDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_controlTypeDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)ctrl_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_controlTypeDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_controlTypeDeclaration, HOOK_EXIT_AST, param);
    }
  }
}

internal void
visit_extern(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_List* type_params = (Ast_List*)extern_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_externDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_externDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* method_protos = (Ast_List*)extern_decl->method_protos;
  if (method_protos) {
    for (DListItem* li = method_protos->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* proto = li->object;
      hooks->visit_function_proto(AST_externDeclaration, HOOK_ENTER_AST, proto);
      visit_function_proto(proto);
      hooks->visit_function_proto(AST_externDeclaration, HOOK_EXIT_AST, proto);
    }
  }
}

internal void
visit_struct(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_structTypeDeclaration);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Ast_List* fields = (Ast_List*)struct_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      hooks->visit_struct_field(AST_structTypeDeclaration, HOOK_ENTER_AST, field);
      visit_struct_field(field);
      hooks->visit_struct_field(AST_structTypeDeclaration, HOOK_EXIT_AST, field);
    }
  }
}

internal void
visit_header(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_List* fields = (Ast_List*)header_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      hooks->visit_struct_field(AST_headerTypeDeclaration, HOOK_ENTER_AST, field);
      visit_struct_field(field);
      hooks->visit_struct_field(AST_headerTypeDeclaration, HOOK_EXIT_AST, field);
    }
  }
}

internal void
visit_header_union(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_List* fields = (Ast_List*)union_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      hooks->visit_struct_field(AST_headerUnionDeclaration, HOOK_ENTER_AST, field);
      visit_struct_field(field);
      hooks->visit_struct_field(AST_headerUnionDeclaration, HOOK_EXIT_AST, field);
    }
  }
}

internal void
visit_package(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_List* type_params = (Ast_List*)package_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_packageTypeDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_packageTypeDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)package_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_packageTypeDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_packageTypeDeclaration, HOOK_EXIT_AST, param);
    }
  }
}

internal void
visit_parser(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_parserDeclaration);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
  Ast_List* type_params = (Ast_List*)proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_parserDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_parserDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_parserDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_parserDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_List* ctor_params = (Ast_List*)parser_decl->ctor_params;
  if (ctor_params) {
    for (DListItem* li = ctor_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_parserDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_parserDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_List* local_elements = (Ast_List*)parser_decl->local_elements;
  if (local_elements) {
    for (DListItem* li = local_elements->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* element = li->object;
      hooks->visit_local_parser_element(AST_parserDeclaration, HOOK_ENTER_AST, element);
      visit_local_parser_element(element);
      hooks->visit_local_parser_element(AST_parserDeclaration, HOOK_EXIT_AST, element);
    }
  }
  Ast_List* states = (Ast_List*)parser_decl->states;
  if (states) {
    for (DListItem* li = states->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* state = li->object;
      hooks->visit_parser_state(AST_parserDeclaration, HOOK_ENTER_AST, state);
      visit_parser_state(state);
      hooks->visit_parser_state(AST_parserDeclaration, HOOK_EXIT_AST, state);
    }
  }
}

internal void
visit_parser_proto(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
  Ast_List* type_params = (Ast_List*)proto_decl->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_parserTypeDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_parserTypeDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)proto_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_parserTypeDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_parserTypeDeclaration, HOOK_EXIT_AST, param);
    }
  }
}

internal void
visit_instantiation(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  hooks->visit_instantiation(AST_instantiation, HOOK_ENTER_AST, inst_decl->type);
  visit_type_ref(inst_decl->type);
  hooks->visit_instantiation(AST_instantiation, HOOK_EXIT_AST, inst_decl->type);
  Ast_List* args = (Ast_List*)inst_decl->args;
  if (args) {
    for (DListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      hooks->visit_expression(AST_instantiation, HOOK_ENTER_AST, arg);
      visit_expression(arg);
      hooks->visit_expression(AST_instantiation, HOOK_EXIT_AST, arg);
    }
  }
}

internal void
visit_typedef(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_typedefDeclaration);
  Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
  hooks->visit_type_ref(AST_typedefDeclaration, HOOK_ENTER_AST, type_decl->type_ref);
  visit_type_ref(type_decl->type_ref);
  hooks->visit_type_ref(AST_typedefDeclaration, HOOK_EXIT_AST, type_decl->type_ref);
}

internal void
visit_function(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_functionDeclaration);
  Ast_Function* func_decl = (Ast_Function*)ast;
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)func_decl->proto;
  if (func_proto->return_type) {
    hooks->visit_type_ref(AST_functionDeclaration, HOOK_ENTER_AST, func_proto->return_type);
    visit_type_ref(func_proto->return_type);
    hooks->visit_type_ref(AST_functionDeclaration, HOOK_EXIT_AST, func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_functionDeclaration, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_functionDeclaration, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_functionDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_functionDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_BlockStmt* func_body = (Ast_BlockStmt*)func_decl->stmt;
  if (func_body) {
    Ast_List* stmt_list = (Ast_List*)func_body->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        hooks->visit_statement(AST_functionDeclaration, HOOK_ENTER_AST, stmt);
        visit_statement(stmt);
        hooks->visit_statement(AST_functionDeclaration, HOOK_EXIT_AST, stmt);
      }
    }
  }
}

internal void
visit_function_proto(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_functionPrototype);
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)ast;
  if (func_proto->return_type) {
    hooks->visit_type_ref(AST_functionPrototype, HOOK_ENTER_AST, func_proto->return_type);
    visit_type_ref(func_proto->return_type);
    hooks->visit_type_ref(AST_functionPrototype, HOOK_EXIT_AST, func_proto->return_type);
  }
  Ast_List* type_params = (Ast_List*)func_proto->type_params;
  if (type_params) {
    for (DListItem* li = type_params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_param = li->object;
      hooks->visit_type_param(AST_functionPrototype, HOOK_ENTER_AST, type_param);
      visit_type_param(type_param);
      hooks->visit_type_param(AST_functionPrototype, HOOK_EXIT_AST, type_param);
    }
  }
  Ast_List* params = (Ast_List*)func_proto->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_functionPrototype, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_functionPrototype, HOOK_EXIT_AST, param);
    }
  }
}

internal void
visit_const(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_Const* const_decl = (Ast_Const*)ast;
  hooks->visit_type_ref(AST_constantDeclaration, HOOK_ENTER_AST, const_decl->type);
  visit_type_ref(const_decl->type);
  hooks->visit_type_ref(AST_constantDeclaration, HOOK_EXIT_AST, const_decl->type);
  hooks->visit_expression(AST_constantDeclaration, HOOK_ENTER_AST, const_decl->expr);
  visit_expression(const_decl->expr);
  hooks->visit_expression(AST_constantDeclaration, HOOK_EXIT_AST, const_decl->expr);
}

internal void
visit_enum(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_List* fields = (Ast_List*)enum_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      hooks->visit_specified_identifier(AST_enumDeclaration, HOOK_ENTER_AST, id);
      visit_specified_identifier(id);
      hooks->visit_specified_identifier(AST_enumDeclaration, HOOK_EXIT_AST, id);
    }
  }
}

internal void
visit_action(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_List* params = (Ast_List*)action_decl->params;
  if (params) {
    for (DListItem* li = params->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* param = li->object;
      hooks->visit_param(AST_actionDeclaration, HOOK_ENTER_AST, param);
      visit_param(param);
      hooks->visit_param(AST_actionDeclaration, HOOK_EXIT_AST, param);
    }
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_List* stmt_list = (Ast_List*)action_body->stmt_list;
    if (stmt_list) {
      for (DListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        hooks->visit_statement(AST_actionDeclaration, HOOK_ENTER_AST, stmt);
        visit_statement(stmt);
        hooks->visit_statement(AST_actionDeclaration, HOOK_EXIT_AST, stmt);
      }
    }
  }
}

internal void
visit_match_kind(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_matchKindDeclaration);
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  Ast_List* fields = (Ast_List*)match_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      if (id->kind == AST_name) {
        hooks->visit_enum_field(AST_matchKindDeclaration, HOOK_ENTER_AST, id);
        visit_enum_field(id);
        hooks->visit_enum_field(AST_matchKindDeclaration, HOOK_EXIT_AST, id);
      } else if (id->kind == AST_specifiedIdentifier) {
        hooks->visit_specified_identifier(AST_matchKindDeclaration, HOOK_ENTER_AST, id);
        visit_specified_identifier(id);
        hooks->visit_specified_identifier(AST_matchKindDeclaration, HOOK_EXIT_AST, id);
      }
      else assert(0);
    }
  }
}

internal void
visit_error_enum(AstTraversalHooks* hooks, Ast* ast)
{
  assert (ast->kind == AST_errorDeclaration);
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  Ast_List* fields = (Ast_List*)error_decl->fields;
  if (fields) {
    for (DListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* id = li->object;
      hooks->visit_enum_field(AST_errorDeclaration, HOOK_ENTER_AST, id);
      visit_enum_field(id);
      hooks->visit_enum_field(AST_errorDeclaration, HOOK_EXIT_AST, id);
    }
  }
}

internal void
visit_p4program(AstTraversalHooks* hooks, Ast* ast)
{
  assert(ast->kind == AST_p4program);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_List* decl_list = (Ast_List*)program->decl_list;
  if (decl_list) {
    for (DListItem* li = decl_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      if (decl->kind == AST_controlDeclaration) {
        hooks->visit_control(AST_p4program, HOOK_ENTER_AST, decl);
        visit_control(decl);
        hooks->visit_control(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_controlTypeDeclaration) {
        hooks->visit_control_proto(AST_p4program, HOOK_ENTER_AST, decl);
        visit_control_proto(decl);
        hooks->visit_control_proto(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_externDeclaration) {
        hooks->visit_extern(AST_p4program, HOOK_ENTER_AST, decl);
        visit_extern(decl);
        hooks->visit_extern(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_structTypeDeclaration) {
        hooks->visit_struct(AST_p4program, HOOK_ENTER_AST, decl);
        visit_struct(decl);
        hooks->visit_struct(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_headerTypeDeclaration) {
        hooks->visit_header(AST_p4program, HOOK_ENTER_AST, decl);
        visit_header(decl);
        hooks->visit_header(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_headerUnionDeclaration) {
        hooks->visit_header_union(AST_p4program, HOOK_ENTER_AST, decl);
        visit_header_union(decl);
        hooks->visit_header_union(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_packageTypeDeclaration) {
        hooks->visit_package(AST_p4program, HOOK_ENTER_AST, decl);
        visit_package(decl);
        hooks->visit_package(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_parserDeclaration) {
        hooks->visit_parser(AST_p4program, HOOK_ENTER_AST, decl);
        visit_parser(decl);
        hooks->visit_parser(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_parserTypeDeclaration) {
        hooks->visit_parser_proto(AST_p4program, HOOK_ENTER_AST, decl);
        visit_parser_proto(decl);
        hooks->visit_parser_proto(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_instantiation) {
        hooks->visit_instantiation(AST_p4program, HOOK_ENTER_AST, decl);
        visit_instantiation(decl);
        hooks->visit_instantiation(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_typedefDeclaration) {
        hooks->visit_typedef(AST_p4program, HOOK_ENTER_AST, decl);
        visit_typedef(decl);
        hooks->visit_typedef(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_functionDeclaration) {
        hooks->visit_function(AST_p4program, HOOK_ENTER_AST, decl);
        visit_function(decl);
        hooks->visit_function(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_functionPrototype) {
        hooks->visit_function_proto(AST_p4program, HOOK_ENTER_AST, decl);
        visit_function_proto(decl);
        hooks->visit_function_proto(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_constantDeclaration) {
        hooks->visit_const(AST_p4program, HOOK_ENTER_AST, decl);
        visit_const(decl);
        hooks->visit_const(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_enumDeclaration) {
        hooks->visit_enum(AST_p4program, HOOK_ENTER_AST, decl);
        visit_enum(decl);
        hooks->visit_enum(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_actionDeclaration) {
        hooks->visit_action(AST_p4program, HOOK_ENTER_AST, decl);
        visit_action(decl);
        hooks->visit_action(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_matchKindDeclaration) {
        hooks->visit_match_kind(AST_p4program, HOOK_ENTER_AST, decl);
        visit_match_kind(decl);
        hooks->visit_match_kind(AST_p4program, HOOK_EXIT_AST, decl);
      } else if (decl->kind == AST_errorDeclaration) {
        hooks->visit_error_enum(AST_p4program, HOOK_ENTER_AST, decl);
        visit_error_enum(decl);
        hooks->visit_error_enum(AST_p4program, HOOK_EXIT_AST, decl);
      }
      else assert(0);
    }
  }
}

internal void
default_hook(enum AstEnum context, enum AstHookPoint point, Ast* ast)
{
  // pass
}

void
init_traversal_hooks(AstTraversalHooks* hooks)
{
  hooks->visit_p4program = default_hook;
  hooks->visit_error_enum = default_hook;
  hooks->visit_match_kind = default_hook;
  hooks->visit_action = default_hook;
  hooks->visit_enum = default_hook;
  hooks->visit_const = default_hook;
  hooks->visit_function_proto = default_hook;
  hooks->visit_function = default_hook;
  hooks->visit_typedef = default_hook;
  hooks->visit_instantiation = default_hook;
  hooks->visit_parser_proto = default_hook;
  hooks->visit_parser = default_hook;
  hooks->visit_package = default_hook;
  hooks->visit_header_union = default_hook;
  hooks->visit_header = default_hook;
  hooks->visit_struct = default_hook;
  hooks->visit_extern = default_hook;
  hooks->visit_control_proto = default_hook;
  hooks->visit_control = default_hook;
  hooks->visit_specified_identifier = default_hook;
  hooks->visit_enum_field = default_hook;
  hooks->visit_type_ref = default_hook;
  hooks->visit_dontcare_type = default_hook;
  hooks->visit_tuple = default_hook;
  hooks->visit_specialized_type = default_hook;
  hooks->visit_name_type = default_hook;
  hooks->visit_header_stack = default_hook;
  hooks->visit_error_type = default_hook;
  hooks->visit_void_type = default_hook;
  hooks->visit_string_type = default_hook;
  hooks->visit_varbit_type = default_hook;
  hooks->visit_bit_type = default_hook;
  hooks->visit_int_type = default_hook;
  hooks->visit_bool_type = default_hook;
  hooks->visit_struct_field = default_hook;
  hooks->visit_parser_state = default_hook;
  hooks->visit_parser_transition = default_hook;
  hooks->visit_select_expr = default_hook;
  hooks->visit_transition_select_case = default_hook;
  hooks->visit_local_parser_element = default_hook;
  hooks->visit_statement = default_hook;
  hooks->visit_empty_stmt = default_hook;
  hooks->visit_exit_stmt = default_hook;
  hooks->visit_return_stmt = default_hook;
  hooks->visit_assignment_stmt = default_hook;
  hooks->visit_switch_stmt = default_hook;
  hooks->visit_if_stmt = default_hook;
  hooks->visit_table = default_hook;
  hooks->visit_var_decl = default_hook;
  hooks->visit_switch_case = default_hook;
  hooks->visit_switch_default = default_hook;
  hooks->visit_table_property = default_hook;
  hooks->visit_table_entries = default_hook;
  hooks->visit_table_key = default_hook;
  hooks->visit_table_single_entry = default_hook;
  hooks->visit_table_actions = default_hook;
  hooks->visit_table_entry = default_hook;
  hooks->visit_select_keyset = default_hook;
  hooks->visit_tuple_keyset = default_hook;
  hooks->visit_keyset_expr = default_hook;
  hooks->visit_dontcare_keyset = default_hook;
  hooks->visit_default_keyset = default_hook;
  hooks->visit_table_keyelem = default_hook;
  hooks->visit_action_ref = default_hook;
  hooks->visit_block_stmt = default_hook;
  hooks->visit_type_param = default_hook;
  hooks->visit_param = default_hook;
  hooks->visit_expression = default_hook;
  hooks->visit_string_literal = default_hook;
  hooks->visit_bool_literal = default_hook;
  hooks->visit_int_literal = default_hook;
  hooks->visit_kvpair_expr = default_hook;
  hooks->visit_array_subscript = default_hook;
  hooks->visit_cast_expr = default_hook;
  hooks->visit_expression_list = default_hook;
  hooks->visit_member_select = default_hook;
  hooks->visit_function_call = default_hook;
  hooks->visit_name_identifier = default_hook;
  hooks->visit_unary_expr = default_hook;
  hooks->visit_binary_expr = default_hook;
}

void
traverse_ast_preorder(AstTraversalHooks* hooks, Ast_P4Program* p4program)
{
  hooks->visit_p4program(0, HOOK_ENTER_AST, ast);
  visit_p4program(hooks, (Ast*)p4program);
  hooks->visit_p4program(0, HOOK_EXIT_AST, ast);
}
