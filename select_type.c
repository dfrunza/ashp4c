#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap selected_type = {};
internal Hashmap* potential_type;

internal void visit_expression(Ast* ast, Type* result_type);
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

void
type_select(uint32_t ast_id, Type* type)
{
  HashmapKey key = { .i_key = ast_id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, selected_type.capacity_log2);
  HashmapEntry* he = hashmap_create_entry(&selected_type, &key);
  he->object = type;
}

internal void
visit_binary_expr(Ast* ast, Type* result_type)
{
  assert(ast->kind == AST_binaryExpression);
  Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
  Type_TypeSet* ty_set = typeset_get(potential_type, expr->id);
  if (!typeset_contains_type(ty_set, result_type)) {
    error("At line %d, column %d: expression type mismatch.", expr->line_no, expr->column_no);
  }
  visit_expression(expr->left_operand, result_type);
  visit_expression(expr->right_operand, result_type);
  type_select(expr->id, result_type);
}

internal void
visit_unary_expr(Ast* ast)
{
  assert(ast->kind == AST_unaryExpression);
  Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
  visit_expression(expr->operand, 0);
}

internal void
visit_name_identifier(Ast* ast)
{
  assert(ast->kind == AST_nonTypeName);
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* expr = (Ast_FunctionCall*)ast;
  visit_expression(expr->callee_expr, 0);
  Ast_Expression* callee_expr = (Ast_Expression*)expr->callee_expr;
  DListItem* li;
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
    visit_expression(arg, 0);
    li = li->next;
  }
}

internal void
visit_member_select(Ast* ast)
{
  assert(ast->kind == AST_memberSelectExpression);
  Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
  visit_expression(expr->lhs_expr, 0);
  visit_expression(expr->member_name, 0);
}

internal void
visit_expression_list(Ast* ast)
{
  assert(ast->kind == AST_exprListExpression);
  Ast_ExprListExpression* expr = (Ast_ExprListExpression*)ast;
  Ast_NodeList* expr_list = &expr->expr_list;
  DListItem* li = expr_list->list.next;
  while (li) {
    Ast* expr_expr = li->object;
    visit_expression(expr_expr, 0);
    li = li->next;
  }
}

internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_Cast* expr = (Ast_Cast*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr, 0);
}

internal void
visit_subscript(Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_Subscript* expr = (Ast_Subscript*)ast;
  visit_expression(expr->index, 0);
  if (expr->end_index) {
    visit_expression(expr->end_index, 0);
  }
}

internal void
visit_kvpair(Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  Ast_KVPairExpr* expr = (Ast_KVPairExpr*)ast;
  visit_expression(expr->name, 0);
  visit_expression(expr->expr, 0);
}

internal void
visit_int_literal(Ast* ast, Type* result_type)
{
  assert(ast->kind == AST_integerLiteral);
  if (result_type->ctor != TYPE_INT) {
    error("At line %d, column %d: expected `int` type.", ast->line_no, ast->column_no);
  }
}

internal void
visit_bool_literal(Ast* ast, Type* result_type)
{
  assert(ast->kind == AST_booleanLiteral);
  if (result_type->ctor != TYPE_BOOL) {
    error("At line %d, column %d: expected `bool` type.", ast->line_no, ast->column_no);
  }
}

internal void
visit_string_literal(Ast* ast, Type* result_type)
{
  assert(ast->kind == AST_stringLiteral);
  if (result_type->ctor != TYPE_STRING) {
    error("At line %d, column %d: expected `string` type.", ast->line_no, ast->column_no);
  }
}

internal void
visit_expression(Ast* ast, Type* result_type)
{
  if (ast->kind == AST_binaryExpression) {
    visit_binary_expr(ast, result_type);
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
    visit_int_literal(ast, result_type);
  } else if (ast->kind == AST_booleanLiteral) {
    visit_bool_literal(ast, result_type);
  } else if (ast->kind == AST_stringLiteral) {
    visit_string_literal(ast, result_type);
  }
  else assert(0);
}

internal void
visit_param(Ast* ast)
{
  assert(ast->kind == AST_parameter);
  Ast_Param* param = (Ast_Param*)ast;
  /*
  Type_TypeSet* ty_set = typeset_get(potential_type, param->id);
  Type* param_ty = ty_set->members.next->object;
  type_select(param_ty, param->id);
  if (ty_set->member_count > 1) {
    Ast_Name* name = (Ast_Name*)param->name;
    error("At line %d, column %d: type of `%s` is ambiguous.",
          name->line_no, name->column_no, name->strname);
  }
  */
  visit_type_ref(param->type);
}

internal void
visit_type_param(Ast* ast)
{

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
  visit_expression(action->name, 0);
  Ast_NodeList* args = &action->args;
  DListItem* li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg, 0);
    li = li->next;
  }
}

internal void
visit_table_keyelem(Ast* ast)
{
  assert(ast->kind == AST_keyElement);
  Ast_KeyElement* keyelem = (Ast_KeyElement*)ast;
  visit_expression(keyelem->expr, 0);
  visit_expression(keyelem->name, 0);
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
  if (ast->kind == AST_defaultKeysetExpression) {
    visit_default_keyset(ast);
  } else if (ast->kind == AST_dontcareArgument) {
    visit_dontcare_keyset(ast);
  } else {
    visit_expression(ast, 0);
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
    visit_expression(prop->init_expr, 0);
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

}

internal void
visit_switch_label(Ast* ast)
{
  if (ast->kind == AST_defaultKeysetExpression) {
    visit_switch_default(ast);
  } else {
    visit_expression(ast, 0);
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
  visit_type_ref(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr, 0);
  }
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_tableDeclaration);
  Ast_Table* table_decl = (Ast_Table*)ast;
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
  Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
  Ast* if_stmt = stmt->stmt;
  visit_expression(stmt->cond_expr, 0);
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
  visit_expression(stmt->expr, 0);
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
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue, 0);
  Ast* assign_expr = stmt->expr;
  visit_expression(assign_expr, 0);
}

internal void
visit_return_stmt(Ast* ast)
{
  Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
  if (stmt->expr) {
    visit_expression(stmt->expr, 0);
  }
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
  visit_expression(select_case->name, 0);
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
    visit_expression(expr, 0);
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
    visit_expression(ast, 0);
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
  Ast_NodeList* stmt_list = &state->stmt_list;
  DListItem* li = stmt_list->list.next;
  while (li) {
    Ast* stmt = li->object;
    visit_statement(stmt);
    li = li->next;
  }
  visit_parser_transition(state->trans_stmt);
}

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_structField);
  Ast_StructField* field = (Ast_StructField*)ast;
  visit_type_ref(field->type);
}

internal void
visit_bool_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBool);
  visit_expression(((Ast_BoolType*)ast)->name, 0);
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeInt);
  visit_expression(((Ast_IntType*)ast)->name, 0);
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
  visit_expression(((Ast_BitType*)ast)->name, 0);
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
  visit_expression(((Ast_VarbitType*)ast)->name, 0);
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
  visit_expression(((Ast_StringType*)ast)->name, 0);
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
  visit_expression(((Ast_VoidType*)ast)->name, 0);
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
  visit_expression(((Ast_ErrorType*)ast)->name, 0);
}

internal void
visit_header_stack(Ast* ast)
{
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  visit_expression(type_ref->name, 0);
  Ast* stack_expr = type_ref->stack_expr;
  visit_expression(stack_expr, 0);
}

internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_nonTypeName);
}

internal void
visit_specialized_type(Ast* ast)
{
  Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
  visit_expression(speclzd_type->name, 0);
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
  Ast_Tuple* type_ref = (Ast_Tuple*)ast;
  Ast_NodeList* type_args = &type_ref->type_args;
  DListItem* li = type_args->list.next;
  while (li) {
    Ast* type_arg = li->object;
    visit_type_ref(type_arg);
    li = li->next;
  }
}

internal void
visit_dontcare_type(Ast* ast)
{

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
    visit_expression(init_expr, 0);
  }
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_controlDeclaration);
  Ast_Control* control_decl = (Ast_Control*)ast;
  Ast_ControlProto* proto = (Ast_ControlProto*)control_decl->proto;
  DListItem* li;
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
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlProto* ctrl_proto = (Ast_ControlProto*)ast;
  /*
  Type_TypeSet* ty_set = typeset_get(potential_type, ctrl_proto->id);
  Type_Function* proto_ty = ty_set->members.next->object;
  type_select((Type*)proto_ty, ctrl_proto->id);
  Ast* void_decl = scope_lookup_name(root_scope, "void")->ns_type->ast;
  proto_ty->return_ty = typeset_get(potential_type, void_decl->id)->members.next->object;
  */
  DListItem* li;
  Ast_NodeList* type_params = &ctrl_proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &ctrl_proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
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
  Ast_NodeList* fields = &struct_decl->fields;
  DListItem* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_NodeList* fields = &header_decl->fields;
  DListItem* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_NodeList* fields = &union_decl->fields;
  DListItem* li = fields->list.next;
  while (li) {
    Ast* field = li->object;
    visit_struct_field(field);
    li = li->next;
  }
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_Package* package_decl = (Ast_Package*)ast;
  /*
  Type_TypeSet* ty_set = typeset_get(potential_type, package_decl->id);
  Type_TypeName* package_ty = ty_set->members.next->object;
  type_select((Type*)package_ty, package_decl->id);
  */
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
  Ast_ParserProto* proto = (Ast_ParserProto*)parser_decl->proto;
  DListItem* li;
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
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto_decl = (Ast_ParserProto*)ast;
  DListItem* li;
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
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type);
  Ast_NodeList* args = &inst_decl->args;
  DListItem* li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg, 0);
    li = li->next;
  }
}

internal void
visit_typedef(Ast* ast)
{
  assert(ast->kind == AST_typedefDeclaration);
  Ast_TypeDef* type_decl = (Ast_TypeDef*)ast;
  Ast* type_ref = type_decl->type_ref;
  visit_type_ref(type_ref);
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_functionDeclaration);
  Ast_Function* func_decl = (Ast_Function*)ast;
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)func_decl->proto;
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  DListItem* li;
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
  Ast_FunctionProto* func_proto = (Ast_FunctionProto*)ast;
  /*
  Type_TypeSet* ty_set = typeset_get(potential_type, func_proto->id);
  Type_Function* proto_ty = ty_set->members.next->object;
  type_select((Type*)proto_ty, func_proto->id);
  */
  if (func_proto->return_type) {
    visit_type_ref(func_proto->return_type);
  }
  DListItem* li;
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
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_Const* const_decl = (Ast_Const*)ast;
  visit_type_ref(const_decl->type);
  Type_TypeSet* ty_set = typeset_get(potential_type, const_decl->type->id);
  Type* const_ty = ty_set->members.next->object;
  type_select(const_decl->type->id, const_ty);
  if (ty_set->member_count > 1) {
    Ast_Name* name = (Ast_Name*)const_decl->name;
    error("At line %d, column %d: type of `%s` is ambiguous.",
          name->line_no, name->column_no, name->strname);
  }
  visit_expression(const_decl->expr, const_ty);
  type_select(const_decl->id, const_ty);
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_NodeList* id_list = &enum_decl->id_list;
  DListItem* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_specifiedIdentifier) {
      visit_specified_identifier(id);
    }
    else assert(0);
    li = li->next;
  }
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_Action* action_decl = (Ast_Action*)ast;
  Ast_NodeList* params = &action_decl->params;
  DListItem* li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
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
  Ast_MatchKind* match_decl = (Ast_MatchKind*)ast;
  Ast_NodeList* id_list = &match_decl->id_list;
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
  Ast_ErrorEnum* error_decl = (Ast_ErrorEnum*)ast;
  Ast_NodeList* id_list = &error_decl->id_list;
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
select_type(Ast_P4Program* p4program, Scope* root_scope_, Hashmap* potential_type_, Arena* type_storage_)
{
  root_scope = root_scope_;
  potential_type = potential_type_;
  type_storage = type_storage_;
  hashmap_init(&selected_type, HASHMAP_KEY_UINT32, 8, type_storage);

  visit_p4program((Ast*)p4program);
  return &selected_type;
}
