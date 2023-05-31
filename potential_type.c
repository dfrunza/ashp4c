#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap potential_type = {};

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
  Type_TypeSet* lhs_ty = typeset_get(&potential_type, expr->left_operand->id);
  Type_TypeSet* rhs_ty = typeset_get(&potential_type, expr->right_operand->id);
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  if (expr->op == OP_ADD || expr->op == OP_SUB ||
      expr->op == OP_MUL || expr->op == OP_DIV) {
    NamespaceEntry* ne = scope_lookup_name(root_scope, "int");
    Ast* int_decl = ne->ns_type->ast;
    Type_TypeSet* int_ty = typeset_get(&potential_type, int_decl->id);
    assert(int_ty->member_count == 1);
    if (typeset_contains_type(lhs_ty, (Type*)int_ty->members.next->object) && 
        typeset_contains_type(rhs_ty, (Type*)int_ty->members.next->object)) {
      typeset_add_type(ty_set, (Type*)int_ty->members.next->object);
    }
  }
  if (ty_set->member_count == 0) {
    error("At line %d, column %d: could not infer type of expression.",
          expr->line_no, expr->column_no);
  }
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_unaryExpression);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand);
  Type_FunctionCall* expr_ty = arena_push_struct(type_storage, Type_FunctionCall);
  expr_ty->ctor = TYPE_FUNCTION_CALL;
  expr_ty->args_ty = (Type*)typeset_get(&potential_type, expr->operand->id);
  expr_ty->ast = (Ast*)expr;
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  typeset_add_type(ty_set, (Type*)expr_ty);
}

internal void
visit_name_identifier(Ast* ast)
{
  Ast_Name* name = (Ast_Name*)ast;
  NamespaceEntry* ne = scope_lookup_name(name->scope, name->strname);
  Type_TypeSet* ty_set = typeset_create(&potential_type, name->id);
  ty_set->ast = (Ast*)name;
  if (ne->ns_type) {
    NameDecl* ndecl = ne->ns_type;
    while (ndecl) {
      Type_TypeSet* decl_ty = typeset_get(&potential_type, ndecl->ast->id);
      if (!decl_ty) {
        error("At line %d, column %d: forward reference to `%s`.",
              name->line_no, name->column_no, name->strname);
        continue;
      }
      typeset_import_set(ty_set, decl_ty);
      ndecl = ndecl->next_decl;
    }
  } else if (ne->ns_var) {
    NameDecl* ndecl = ne->ns_var;
    typeset_import_set(ty_set, typeset_get(&potential_type, ndecl->ast->id));
  } else error("At line %d, column %d: unresolved name `%s`.",
                name->line_no, name->column_no, name->strname);
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* function_call = (Ast_FunctionCall*)ast;
  visit_expression(function_call->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)function_call->callee_expr;
  Type_FunctionCall* fcall_ty = arena_push_struct(type_storage, Type_FunctionCall);
  fcall_ty->ctor = TYPE_FUNCTION_CALL;
  fcall_ty->ast = (Ast*)function_call;
  Type_TypeSet* ty_set = typeset_create(&potential_type, function_call->id);
  ty_set->ast = (Ast*)function_call;
  typeset_add_type(ty_set, (Type*)fcall_ty);
  Ast_NodeList* type_args = &callee_expr->type_args;
  DListItem* li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
  Ast_NodeList* args = &function_call->args;
  if (args->list.next) {
    DListItem* li = args->list.next;
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
    if (li) {
      Type* args_ty = (Type*)typeset_get(&potential_type, arg->id);
      while (li) {
        Ast* arg = li->object;
        visit_expression(arg);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = args_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, arg->id);
        args_ty = (Type*)product_ty;
        li = li->next;
      }
      fcall_ty->args_ty = args_ty;
    } else {
      fcall_ty->args_ty = (Type*)typeset_get(&potential_type, arg->id);
    }
  } else {
    Type_TypeSet* args_ty = typeset_create(&potential_type, args->id);
    args_ty->ast = (Ast*)args;
    fcall_ty->args_ty = (Type*)args_ty;
  }
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_memberSelectExpression);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr);
  Type_FunctionCall* expr_ty = arena_push_struct(type_storage, Type_FunctionCall);
  expr_ty->ctor = TYPE_FUNCTION_CALL;
  expr_ty->args_ty = (Type*)typeset_get(&potential_type, expr->lhs_expr->id);
  expr_ty->ast = (Ast*)expr;
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  typeset_add_type(ty_set, (Type*)expr_ty);
}

internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_exprListExpression);
  Ast_ExprListExpression* expr = (Ast_ExprListExpression*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  Ast_NodeList* expr_list = &expr->expr_list;
  if (expr_list->list.next) {
    DListItem* li = expr_list->list.next;
    Ast* item = li->object;
    visit_expression(item);
    li = li->next;
    if (li) {
      Type* items_ty = (Type*)typeset_get(&potential_type, item->id);
      while (li) {
        Ast* item = li->object;
        visit_expression(item);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = items_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, item->id);
        items_ty = (Type*)product_ty;
        li = li->next;
      }
      typeset_add_type(ty_set, items_ty);
    } else {
      typeset_import_set(ty_set, typeset_get(&potential_type, item->id));
    }
  }
}

internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_CastExpr* expr = (Ast_CastExpr*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr);
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  typeset_import_set(ty_set, typeset_get(&potential_type, expr->to_type->id));
}

internal void
visit_subscript(Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_ArraySubscript* expr = (Ast_ArraySubscript*)ast;
  visit_expression(expr->index);
  if (expr->end_index) {
    visit_expression(expr->end_index);
  }
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  typeset_import_set(ty_set, typeset_get(&potential_type, expr->index->id));
}

internal void
visit_kvpair(Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  Ast_KVPairExpr* expr = (Ast_KVPairExpr*)ast;
  visit_expression(expr->expr);
  Type_TypeSet* ty_set = typeset_create(&potential_type, expr->id);
  ty_set->ast = (Ast*)expr;
  typeset_import_set(ty_set, typeset_get(&potential_type, expr->expr->id));
}

internal void
visit_int_literal(Ast* ast)
{
  assert(ast->kind == AST_integerLiteral);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "int");
  Ast* int_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, int_decl->id));
}

internal void
visit_bool_literal(Ast* ast)
{
  assert(ast->kind == AST_booleanLiteral);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "bool");
  Ast* bool_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, bool_decl->id));
}

internal void
visit_string_literal(Ast* ast)
{
  assert(ast->kind == AST_stringLiteral);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "string");
  Ast* string_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, string_decl->id));
}

internal void
visit_expression(Ast* ast)
{
  if (ast->kind == AST_binaryExpression) {
    visit_binary_expr(ast);
  } else if (ast->kind == AST_unaryExpression) {
    visit_unary_expr(ast);
  } else if (ast->kind == AST_nonTypeName) {
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
  visit_type_ref(param->type);
  Type_TypeSet* ty_set = typeset_create(&potential_type, param->id);
  ty_set->ast = (Ast*)param;
  typeset_import_set(ty_set, typeset_get(&potential_type, param->type->id));
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_nonTypeName);
  Ast_Name* name = (Ast_Name*)ast;
  if (!name->scope) {
    /* Declaration of a type parameter. */
    Type_TypeParam* param_ty = arena_push_struct(type_storage, Type_TypeParam);
    param_ty->ctor = TYPE_TYPEPARAM;
    param_ty->strname = name->strname;
    param_ty->ast = (Ast*)name;
    Type_TypeSet* ty_set = typeset_create(&potential_type, name->id);
    ty_set->ast = (Ast*)name;
    typeset_add_type(ty_set, (Type*)param_ty);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_block_statement(Ast* ast)
{
  assert(ast->kind == AST_blockStatement);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  Ast_NodeList* stmt_list = &block_stmt->stmt_list;
  DListItem* li = stmt_list->list.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_actionRef);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  Type_FunctionCall* fcall_ty = arena_push_struct(type_storage, Type_FunctionCall);
  fcall_ty->ctor = TYPE_FUNCTION_CALL;
  fcall_ty->ast = (Ast*)action;
  Type_TypeSet* ty_set = typeset_create(&potential_type, action->id);
  ty_set->ast = (Ast*)action;
  typeset_add_type(ty_set, (Type*)fcall_ty);
  Ast_NodeList* args = &action->args;
  if (args->list.next) {
    DListItem* li = args->list.next;
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
    if (li) {
      Type* args_ty = (Type*)typeset_get(&potential_type, arg->id);
      while (li) {
        Ast* arg = li->object;
        visit_expression(arg);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = args_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, arg->id);
        args_ty = (Type*)product_ty;
        li = li->next;
      }
      fcall_ty->args_ty = args_ty;
    } else {
      fcall_ty->args_ty = (Type*)typeset_get(&potential_type, arg->id);
    }
  } else {
    Type_TypeSet* args_ty = typeset_create(&potential_type, args->id);
    args_ty->ast = (Ast*)args;
    fcall_ty->args_ty = (Type*)args_ty;
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_keyElement);
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
  Ast_NodeList* expr_list = &keyset->expr_list;
  DListItem* li = expr_list->list.next;
  while (li) {
    Ast* expr = li->object;
    visit_keyset_expr(expr);
    li = li->next;
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
  Ast_NodeList* action_list = &prop->action_list;
  DListItem* li = action_list->list.next;
  while (li) {
    Ast* action = li->object;
    visit_action_ref(action);
    li = li->next;
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
  Ast_NodeList* keyelem_list = &prop->keyelem_list;
  DListItem* li = keyelem_list->list.next;
  while (li) {
    Ast* keyelem = li->object;
    visit_table_keyelem(keyelem);
    li = li->next;
  }
}

internal void
visit_table_entries(Ast* ast)
{
  assert(ast->kind == AST_tableEntries);
  Ast_TableEntries* prop = (Ast_TableEntries*)ast;
  Ast_NodeList* entries = &prop->entries;
  DListItem* li = entries->list.next;
  while (li) {
    Ast* entry = li->object;
    visit_table_entry(entry);
    li = li->next;
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
  assert(ast->kind == AST_variableDeclaration);
  Ast_Var* var_decl = (Ast_Var*)ast;
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
  Type_TypeSet* ty_set = typeset_create(&potential_type, var_decl->id);
  ty_set->ast = (Ast*)var_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, var_decl->type->id));
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_tableDeclaration);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_Name* name = (Ast_Name*)table_decl->name;
  Type_TypeName* table_ty = arena_push_struct(type_storage, Type_TypeName);
  table_ty->ctor = TYPE_TYPENAME;
  table_ty->ast = (Ast*)table_decl;
  table_ty->strname = name->strname;
  Type_TypeSet* ty_set = typeset_create(&potential_type, table_decl->id);
  ty_set->ast = (Ast*)table_decl;
  typeset_add_type(ty_set, (Type*)table_ty);
  Ast_NodeList* prop_list = &table_decl->prop_list;
  DListItem* li = prop_list->list.next;
  while (li) {
    Ast* prop = li->object;
    visit_table_property(prop);
    li = li->next;
  }
}

internal void
visit_if_stmt(Ast* ast)
{
  assert(ast->kind == AST_conditionalStatement);
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
  assert(ast->kind == AST_switchStatement);
  Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
  visit_expression(stmt->expr);
  Ast_NodeList* switch_cases = &stmt->switch_cases;
  DListItem* li = switch_cases->list.next;
  while (li) {
    Ast* switch_case = li->object;
    visit_switch_case(switch_case);
    li = li->next;
  }
}

internal void
visit_assignment_stmt(Ast* ast)
{
  assert(ast->kind == AST_assignmentStatement);
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  visit_expression(stmt->expr);
  Type_Product* operands_ty = arena_push_struct(type_storage, Type_Product);
  operands_ty->ctor = TYPE_PRODUCT;
  operands_ty->lhs_ty = (Type*)typeset_get(&potential_type, stmt->lvalue->id);
  operands_ty->rhs_ty = (Type*)typeset_get(&potential_type, stmt->expr->id);
  Type_FunctionCall* assgn_ty = arena_push_struct(type_storage, Type_FunctionCall);
  assgn_ty->ctor = TYPE_FUNCTION_CALL;
  assgn_ty->args_ty = (Type*)operands_ty;
  assgn_ty->ast = (Ast*)stmt;
  Type_TypeSet* ty_set = typeset_create(&potential_type, stmt->id);
  ty_set->ast = (Ast*)stmt;
  typeset_add_type(ty_set, (Type*)assgn_ty);
}

internal void
visit_return_stmt(Ast* ast)
{
  assert(ast->kind == AST_returnStatement);
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, stmt->id);
  ty_set->ast = (Ast*)stmt;
  if (stmt->expr) {
    visit_expression(stmt->expr);
    typeset_import_set(ty_set, typeset_get(&potential_type, stmt->expr->id));
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
  } else if (ast->kind == AST_emptyStatement) {
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
}

internal void
visit_select_expr(Ast* ast)
{
  assert(ast->kind == AST_selectExpression);
  Ast_SelectExpr* trans_stmt = (Ast_SelectExpr*)ast;
  DListItem* li;
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
  if (ast->kind == AST_nonTypeName) {
    // pass
  } else if (ast->kind == AST_selectExpression) {
    visit_select_expr(ast);
  }
  else assert(0);
}

internal void
visit_parser_state(Ast* ast)
{
  assert(ast->kind == AST_parserState);
  Ast_ParserState* state_decl = (Ast_ParserState*)ast;
  Ast_NodeList* stmt_list = &state_decl->stmt_list;
  DListItem* li = stmt_list->list.next;
  while (li) {
    Ast* stmt = li->object;
    visit_statement(stmt);
    li = li->next;
  }
  visit_parser_transition(state_decl->trans_stmt);
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_structField);
  Ast_StructField* field = (Ast_StructField*)ast;
  visit_type_ref(field->type);
  Type_TypeSet* ty_set = typeset_create(&potential_type, field->id);
  ty_set->ast = (Ast*)field;
  typeset_import_set(ty_set, typeset_get(&potential_type, field->type->id));
}

internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBool);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "bool");
  Ast* bool_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, bool_decl->id));
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeInt);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "int");
  Ast* int_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, int_decl->id));
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "bit");
  Ast* bit_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, bit_decl->id));
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "varbit");
  Ast* varbit_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, varbit_decl->id));
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "string");
  Ast* string_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, string_decl->id));
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "void");
  Ast* void_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, void_decl->id));
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
  NamespaceEntry* ne = scope_lookup_name(root_scope, "error");
  Ast* error_decl = ne->ns_type->ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, ast->id);
  ty_set->ast = ast;
  typeset_import_set(ty_set, typeset_get(&potential_type, error_decl->id));
}

internal void
visit_header_stack(Ast* ast)
{
  assert(ast->kind == AST_headerStackType);
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  visit_expression(type_ref->name);
  visit_expression(type_ref->stack_expr);
  Type_TypeSet* ty_set = typeset_create(&potential_type, type_ref->id);
  ty_set->ast = (Ast*)type_ref;
  typeset_import_set(ty_set, typeset_get(&potential_type, type_ref->name->id));
}

internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_nonTypeName);
  Ast_Name* name = (Ast_Name*)ast;
  if (!name->scope) {
    /* Declaration of a type parameter. */
    Type_TypeParam* param_ty = arena_push_struct(type_storage, Type_TypeParam);
    param_ty->ctor = TYPE_TYPEPARAM;
    param_ty->strname = name->strname;
    param_ty->ast = (Ast*)name;
    Type_TypeSet* ty_set = typeset_create(&potential_type, name->id);
    ty_set->ast = (Ast*)name;
    typeset_add_type(ty_set, (Type*)param_ty);
  } else {
    visit_expression(ast);
  }
}

internal void
visit_specialized_type(Ast* ast)
{
  assert(ast->kind == AST_specializedType);
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  visit_expression(speclzd_type->name);
  Type_TypeSet* ty_set = typeset_create(&potential_type, speclzd_type->id);
  ty_set->ast = (Ast*)speclzd_type;
  typeset_import_set(ty_set, typeset_get(&potential_type, speclzd_type->name->id));
  Ast_NodeList* type_args = &speclzd_type->type_args;
  DListItem* li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
}

internal void
visit_tuple(Ast* ast)
{
  assert(ast->kind == AST_tupleType);
  Ast_Tuple* tuple_decl = (Ast_Tuple*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, tuple_decl->id);
  ty_set->ast = (Ast*)tuple_decl;
  Ast_NodeList* args = &tuple_decl->type_args;
  if (args->list.next) {
    DListItem* li = args->list.next;
    Ast* arg = li->object;
    visit_type_ref(arg);
    li = li->next;
    if (li) {
      Type* args_ty = (Type*)typeset_get(&potential_type, arg->id);
      while (li) {
        Ast* arg = li->object;
        visit_type_ref(arg);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = args_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, arg->id);
        args_ty = (Type*)product_ty;
        li = li->next;
      }
      typeset_add_type(ty_set, args_ty);
    } else {
      typeset_import_set(ty_set, typeset_get(&potential_type, arg->id));
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
  } else if (ast->kind == AST_nonTypeName) {
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
  assert(ast->kind == AST_nonTypeName);
  // pass
}

internal void
visit_specified_identifier(Ast* ast)
{
  assert(ast->kind == AST_specifiedIdentifier);
  // pass
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_controlDeclaration);
  Ast_Control* ctrl_decl = (Ast_Control*)ast;
  visit_control_proto(ctrl_decl->proto);
  Type_TypeSet* ty_set = typeset_create(&potential_type, ctrl_decl->id);
  ty_set->ast = (Ast*)ctrl_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, ctrl_decl->proto->id));
  DListItem* li;
  Ast_NodeList* ctor_params = &ctrl_decl->ctor_params;
  li = ctor_params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_NodeList* local_decls = &ctrl_decl->local_decls;
  li = local_decls->list.next;
  while (li) {
    Ast* decl = li->object;
    visit_statement(decl);
    li = li->next;
  }
  if (ctrl_decl->apply_stmt) {
    visit_block_statement(ctrl_decl->apply_stmt);
  }
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlProto* proto = (Ast_ControlProto*)ast;
  Type_Function* control_ty = arena_push_struct(type_storage, Type_Function);
  control_ty->ctor = TYPE_FUNCTION;
  control_ty->ast = (Ast*)proto;
  Type_TypeSet* ty_set = typeset_create(&potential_type, proto->id);
  ty_set->ast = (Ast*)proto;
  typeset_add_type(ty_set, (Type*)control_ty);
  Ast_NodeList* type_params = &proto->type_params;
  DListItem* li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->list.next) {
    DListItem* li = params->list.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = (Type*)typeset_get(&potential_type, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      control_ty->params_ty = params_ty;
    } else {
      control_ty->params_ty = (Type*)typeset_get(&potential_type, param->id);
    }
  } else {
    Type_TypeSet* params_ty = typeset_create(&potential_type, params->id);
    params_ty->ast = (Ast*)params;
    control_ty->params_ty = (Type*)params_ty;
  }
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  Type_TypeName* extern_ty = arena_push_struct(type_storage, Type_TypeName);
  extern_ty->ctor = TYPE_TYPENAME;
  extern_ty->ast = (Ast*)extern_decl;
  extern_ty->strname = name->strname;
  Type_TypeSet* ty_set = typeset_create(&potential_type, extern_decl->id);
  ty_set->ast = (Ast*)extern_decl;
  typeset_add_type(ty_set, (Type*)extern_ty);
  DListItem* li;
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
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_structTypeDeclaration);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, struct_decl->id);
  ty_set->ast = (Ast*)struct_decl;
  Ast_NodeList* fields = &struct_decl->fields;
  if (fields->list.next) {
    DListItem* li = fields->list.next;
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
    if (li) {
      Type* fields_ty = (Type*)typeset_get(&potential_type, field->id);
      while (li) {
        Ast* field = li->object;
        visit_struct_field(field);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = fields_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, field->id);
        fields_ty = (Type*)product_ty;
        li = li->next;
      }
      typeset_add_type(ty_set, fields_ty);
    } else {
      typeset_import_set(ty_set, typeset_get(&potential_type, field->id));
    }
  }
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, header_decl->id);
  ty_set->ast = (Ast*)header_decl;
  Ast_NodeList* fields = &header_decl->fields;
  if (fields->list.next) {
    DListItem* li = fields->list.next;
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
    if (li) {
      Type* fields_ty = (Type*)typeset_get(&potential_type, field->id);
      while (li) {
        Ast* field = li->object;
        visit_struct_field(field);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = fields_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, field->id);
        fields_ty = (Type*)product_ty;
        li = li->next;
      }
      typeset_add_type(ty_set, fields_ty);
    } else {
      typeset_import_set(ty_set, typeset_get(&potential_type, field->id));
    }
  }
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Type_TypeSet* ty_set = typeset_create(&potential_type, union_decl->id);
  ty_set->ast = (Ast*)union_decl;
  Ast_NodeList* fields = &union_decl->fields;
  if (fields->list.next) {
    DListItem* li = fields->list.next;
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
    if (li) {
      Type* fields_ty = (Type*)typeset_get(&potential_type, field->id);
      while (li) {
        Ast* field = li->object;
        visit_struct_field(field);
        Type_Union* union_ty = arena_push_struct(type_storage, Type_Union);
        union_ty->ctor = TYPE_UNION;
        union_ty->lhs_ty = fields_ty;
        union_ty->rhs_ty = (Type*)typeset_get(&potential_type, field->id);
        fields_ty = (Type*)union_ty;
        li = li->next;
      }
      typeset_add_type(ty_set, fields_ty);
    } else {
      typeset_import_set(ty_set, typeset_get(&potential_type, field->id));
    }
  }
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  Type_TypeName* package_ty = arena_push_struct(type_storage, Type_TypeName);
  package_ty->ctor = TYPE_TYPENAME;
  package_ty->ast = (Ast*)package_decl;
  package_ty->strname = name->strname;
  Type_TypeSet* ty_set = typeset_create(&potential_type, package_decl->id);
  ty_set->ast = (Ast*)package_decl;
  typeset_add_type(ty_set, (Type*)package_ty);
  DListItem* li;
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
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_parserDeclaration);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  visit_parser_proto(parser_decl->proto);
  Type_TypeSet* ty_set = typeset_create(&potential_type, parser_decl->id);
  ty_set->ast = (Ast*)parser_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, parser_decl->id));
  DListItem* li;
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
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto = (Ast_ParserProto*)ast;
  Type_Function* parser_ty = arena_push_struct(type_storage, Type_Function);
  parser_ty->ctor = TYPE_FUNCTION;
  parser_ty->ast = (Ast*)proto;
  Type_TypeSet* ty_set = typeset_create(&potential_type, proto->id);
  ty_set->ast = (Ast*)proto;
  typeset_add_type(ty_set, (Type*)parser_ty);
  Ast_NodeList* type_params = &proto->type_params;
  DListItem* li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->list.next) {
    DListItem* li = params->list.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = (Type*)typeset_get(&potential_type, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      parser_ty->params_ty = params_ty;
    } else {
      parser_ty->params_ty = (Type*)typeset_get(&potential_type, param->id);
    }
  } else {
    Type_TypeSet* params_ty = typeset_create(&potential_type, params->id);
    params_ty->ast = (Ast*)params;
    parser_ty->params_ty = (Type*)params_ty;
  }
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type);
  Type_FunctionCall* inst_ty = arena_push_struct(type_storage, Type_FunctionCall);
  inst_ty->ctor = TYPE_FUNCTION_CALL;
  inst_ty->ast = (Ast*)inst_decl;
  Type_TypeSet* ty_set = typeset_create(&potential_type, inst_decl->id);
  ty_set->ast = (Ast*)inst_decl;
  typeset_add_type(ty_set, (Type*)inst_ty);
  Ast_NodeList* args = &inst_decl->args;
  if (args->list.next) {
    DListItem* li = args->list.next;
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
    if (li) {
      Type* args_ty = (Type*)typeset_get(&potential_type, arg->id);
      while (li) {
        Ast* arg = li->object;
        visit_expression(arg);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = args_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, arg->id);
        args_ty = (Type*)product_ty;
        li = li->next;
      }
      inst_ty->args_ty = args_ty;
    } else {
      inst_ty->args_ty = (Type*)typeset_get(&potential_type, arg->id);
    }
  } else {
    Type_TypeSet* args_ty = typeset_create(&potential_type, args->id);
    args_ty->ast = (Ast*)args;
    inst_ty->args_ty = (Type*)args_ty;
  }
}

internal void
visit_typedef(Ast* ast)
{
  assert(ast->kind == AST_typedefDeclaration);
  Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
  visit_type_ref(type_decl->type_ref);
  Type_TypeSet* ty_set = typeset_create(&potential_type, type_decl->id);
  ty_set->ast = (Ast*)type_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, type_decl->type_ref->id));
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_functionDeclaration);
  Ast_Function* function_decl = (Ast_Function*)ast;
  visit_function_proto(function_decl->proto);
  Type_TypeSet* ty_set = typeset_create(&potential_type, function_decl->id);
  ty_set->ast = (Ast*)function_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, function_decl->proto->id));
  Ast_BlockStmt* function_body = (Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    Ast_NodeList* stmt_list = &function_body->stmt_list;
    DListItem* li = stmt_list->list.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_functionPrototype);
  Ast_FunctionProto* proto = (Ast_FunctionProto*)ast;
  Type_Function* function_ty = arena_push_struct(type_storage, Type_Function);
  function_ty->ctor = TYPE_FUNCTION;
  function_ty->ast = (Ast*)proto;
  Type_TypeSet* ty_set = typeset_create(&potential_type, proto->id);
  ty_set->ast = (Ast*)proto;
  typeset_add_type(ty_set, (Type*)function_ty);
  if (proto->return_type) {
    visit_type_ref(proto->return_type);
    function_ty->return_ty = (Type*)typeset_get(&potential_type, proto->return_type->id);
  }
  Ast_NodeList* type_params = &proto->type_params;
  DListItem* li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &proto->params;
  if (params->list.next) {
    DListItem* li = params->list.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = (Type*)typeset_get(&potential_type, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      function_ty->params_ty = params_ty;
    } else {
      function_ty->params_ty = (Type*)typeset_get(&potential_type, param->id);
    }
  } else {
    Type_TypeSet* params_ty = typeset_create(&potential_type, params->id);
    params_ty->ast = (Ast*)params;
    function_ty->params_ty = (Type*)params_ty;
  }
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_Const* const_decl = (Ast_Const*)ast;
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
  Type_TypeSet* ty_set = typeset_create(&potential_type, const_decl->id);
  ty_set->ast = (Ast*)const_decl;
  typeset_import_set(ty_set, typeset_get(&potential_type, const_decl->type->id));
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  Type_TypeName* enum_ty = arena_push_struct(type_storage, Type_TypeName);
  enum_ty->ctor = TYPE_TYPENAME;
  enum_ty->strname = name->strname;
  enum_ty->ast = (Ast*)enum_decl;
  Type_TypeSet* ty_set = typeset_create(&potential_type, enum_decl->id);
  ty_set->ast = (Ast*)enum_decl;
  typeset_add_type(ty_set, (Type*)enum_ty);
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Type_Function* function_ty = arena_push_struct(type_storage, Type_Function);
  function_ty->ctor = TYPE_FUNCTION;
  function_ty->ast = (Ast*)action_decl;
  Type_TypeSet* ty_set = typeset_create(&potential_type, action_decl->id);
  ty_set->ast = (Ast*)action_decl;
  typeset_add_type(ty_set, (Type*)function_ty);
  Ast_NodeList* params = &action_decl->params;
  if (params->list.next) {
    DListItem* li = params->list.next;
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
    if (li) {
      Type* params_ty = (Type*)typeset_get(&potential_type, param->id);
      while (li) {
        Ast* param = li->object;
        visit_param(param);
        Type_Product* product_ty = arena_push_struct(type_storage, Type_Product);
        product_ty->ctor = TYPE_PRODUCT;
        product_ty->lhs_ty = params_ty;
        product_ty->rhs_ty = (Type*)typeset_get(&potential_type, param->id);
        params_ty = (Type*)product_ty;
        li = li->next;
      }
      function_ty->params_ty = params_ty;
    } else {
      function_ty->params_ty = (Type*)typeset_get(&potential_type, param->id);
    }
  } else {
    Type_TypeSet* params_ty = typeset_create(&potential_type, params->id);
    params_ty->ast = (Ast*)params;
    function_ty->params_ty = (Type*)params_ty;
  }
  Ast_BlockStmt* action_body = (Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    Ast_NodeList* stmt_list = &action_body->stmt_list;
    DListItem* li = stmt_list->list.next;
    while (li) {
      Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
}

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_matchKindDeclaration);
  Ast_MatchKind* decl = (Ast_MatchKind*)ast;
  Ast_NodeList* id_list = &decl->id_list;
  DListItem* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_nonTypeName) {
      visit_enum_field(id);
    } else if (id->kind == AST_specifiedIdentifier) {
      visit_specified_identifier(id);
    }
    else assert(0);
    li = li->next;
  }
}

internal void
visit_error(Ast* ast)
{
  assert (ast->kind == AST_ERROR);
  Ast_ErrorEnum* decl = (Ast_ErrorEnum*)ast;
  Ast_NodeList* id_list = &decl->id_list;
  DListItem* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_nonTypeName) {
      visit_enum_field(id);
    }
    else assert(0);
    li = li->next;
  }
}


internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_p4program);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_NodeList* decl_list = &program->decl_list;
  DListItem* li = decl_list->list.next;
  while (li) {
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
    } else if (decl->kind == AST_ERROR) {
      visit_error(decl);
    } else assert(0);
    li = li->next;
  }
}

Hashmap*
build_potential_type(Ast_P4Program* p4program, Scope* root_scope_, Arena* type_storage_)
{
  root_scope = root_scope_;
  type_storage = type_storage_;
  hashmap_init(&potential_type, HASHMAP_KEY_UINT32, 8, type_storage);
  typeset_init(type_storage);

  {
    Ast* void_decl = scope_lookup_name(root_scope, "void")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, void_decl->id);
    ty_set->ast = void_decl;
    Type* void_ty = arena_push_struct(type_storage, Type);
    void_ty->ctor = TYPE_VOID;
    void_ty->ast = void_decl;
    typeset_add_type(ty_set, void_ty);
  }
  {
    Ast* bool_decl = scope_lookup_name(root_scope, "bool")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, bool_decl->id);
    ty_set->ast = bool_decl;
    Type* bool_ty = arena_push_struct(type_storage, Type);
    bool_ty->ctor = TYPE_BOOL;
    bool_ty->ast = bool_decl;
    typeset_add_type(ty_set, bool_ty);
  }
  {
    Ast* int_decl = scope_lookup_name(root_scope, "int")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, int_decl->id);
    ty_set->ast = int_decl;
    Type* int_ty = arena_push_struct(type_storage, Type);
    int_ty->ctor = TYPE_INT;
    int_ty->ast = int_decl;
    typeset_add_type(ty_set, int_ty);
  }
  {
    Ast* bit_decl = scope_lookup_name(root_scope, "bit")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, bit_decl->id);
    ty_set->ast = bit_decl;
    Type* bit_ty = arena_push_struct(type_storage, Type);
    bit_ty->ctor = TYPE_BIT;
    bit_ty->ast = bit_decl;
    typeset_add_type(ty_set, bit_ty);
  }
  {
    Ast* varbit_decl = scope_lookup_name(root_scope, "varbit")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, varbit_decl->id);
    ty_set->ast = varbit_decl;
    Type* varbit_ty = arena_push_struct(type_storage, Type);
    varbit_ty->ctor = TYPE_VARBIT;
    varbit_ty->ast = varbit_decl;
    typeset_add_type(ty_set, varbit_ty);
  }
  {
    Ast* string_decl = scope_lookup_name(root_scope, "string")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, string_decl->id);
    ty_set->ast = string_decl;
    Type* string_ty = arena_push_struct(type_storage, Type);
    string_ty->ctor = TYPE_STRING;
    string_ty->ast = string_decl;
    typeset_add_type(ty_set, string_ty);
  }
  {
    Ast* error_decl = scope_lookup_name(root_scope, "error")->ns_type->ast;
    Type_TypeSet* ty_set = typeset_create(&potential_type, error_decl->id);
    ty_set->ast = error_decl;
    Type* error_ty = arena_push_struct(type_storage, Type);
    error_ty->ctor = TYPE_ERROR;
    error_ty->ast = error_decl;
    typeset_add_type(ty_set, error_ty);
  }

  visit_p4program((Ast*)p4program);
  return &potential_type;
}
