#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal Scope* root_scope;
internal Arena *type_storage;
internal Hashmap type_table = {};

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
  // pass
}

internal void
visit_function_call(Ast* ast)
{
  assert(ast->kind == AST_functionCall);
  Ast_FunctionCall* function_call = (Ast_FunctionCall*)ast;
  visit_expression(function_call->callee_expr);
  Ast_Expression* callee_expr = (Ast_Expression*)function_call->callee_expr;
  Ast_List* type_args = (Ast_List*)callee_expr->type_args;
  if (type_args) {
    for (ListItem* li = type_args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
    }
  }
  Ast_List* args = (Ast_List*)function_call->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
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
}

internal void
visit_cast_expr(Ast* ast)
{
  assert(ast->kind == AST_castExpression);
  Ast_CastExpr* expr = (Ast_CastExpr*)ast;
  visit_type_ref(expr->to_type);
  visit_expression(expr->expr);
}

internal void
visit_subscript(Ast* ast)
{
  assert(ast->kind == AST_arraySubscript);
  Ast_ArraySubscript* expr = (Ast_ArraySubscript*)ast;
  visit_expression(expr->lhs_expr);
  visit_expression(expr->index_expr);
}

internal void
visit_kvpair(Ast* ast)
{
  assert(ast->kind == AST_kvPairExpression);
  // pass
}

internal void
visit_int_literal(Ast* ast)
{
  assert(ast->kind == AST_integerLiteral);
  // pass
}

internal void
visit_bool_literal(Ast* ast)
{
  assert(ast->kind == AST_booleanLiteral);
  // pass
}

internal void
visit_string_literal(Ast* ast)
{
  assert(ast->kind == AST_stringLiteral);
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
}

internal void
visit_type_param(Ast* ast)
{
  assert(ast->kind == AST_name);
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
  assert(ast->kind == AST_blockStatement);
  Ast_BlockStmt* block_stmt = (Ast_BlockStmt*)ast;
  Ast_List* stmt_list = (Ast_List*)block_stmt->stmt_list;
  if (stmt_list) {
    for (ListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* decl = li->object;
      visit_statement(decl);
    }
  }
}

internal void
visit_action_ref(Ast* ast)
{
  assert(ast->kind == AST_actionRef);
  Ast_ActionRef* action = (Ast_ActionRef*)ast;
  Ast_List* args = (Ast_List*)action->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
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
    for (ListItem* li = expr_list->members.sentinel.next;
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
    for (ListItem* li = action_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* action = li->object;
      visit_action_ref(action);
    }
  }
}

internal void
visit_table_single_entry(Ast* ast)
{
  assert(ast->kind == AST_tableProperty);
  Ast_SimpleTableProperty* prop = (Ast_SimpleTableProperty*)ast;
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
    for (ListItem* li = keyelem_list->members.sentinel.next;
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
    for (ListItem* li = entries->members.sentinel.next;
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
  assert(ast->kind == AST_variableDeclaration);
  Ast_Var* var_decl = (Ast_Var*)ast;
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
  Ast_List* prop_list = (Ast_List*)table_decl->prop_list;
  if (prop_list) {
    for (ListItem* li = prop_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* prop = li->object;
      visit_table_property(prop);
    }
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
  Ast_List* switch_cases = (Ast_List*)stmt->switch_cases;
  if (switch_cases) {
    for (ListItem* li = switch_cases->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* switch_case = li->object;
      visit_switch_case(switch_case);
    }
  }
}

internal void
visit_assignment_stmt(Ast* ast)
{
  assert(ast->kind == AST_assignmentStatement);
  Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
  visit_expression(stmt->lvalue);
  visit_expression(stmt->expr);
}

internal void
visit_return_stmt(Ast* ast)
{
  assert(ast->kind == AST_returnStatement);
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

internal void
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_name) {
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
  Ast_List* stmt_list = (Ast_List*)state_decl->stmt_list;
  if (stmt_list) {
    for (ListItem* li = stmt_list->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* stmt = li->object;
      visit_statement(stmt);
    }
  }
  visit_parser_transition(state_decl->transition_stmt);
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
}

internal void
visit_int_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeInt);
}

internal void
visit_bit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeBit);
}

internal void
visit_varbit_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVarbit);
}

internal void
visit_string_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeString);
}

internal void
visit_void_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeVoid);
}

internal void
visit_error_type(Ast* ast)
{
  assert(ast->kind == AST_baseTypeError);
}

internal void
visit_header_stack(Ast* ast)
{
  assert(ast->kind == AST_headerStackType);
  Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
  visit_expression(type_ref->name);
  visit_expression(type_ref->stack_expr);
}

internal void
visit_name_type(Ast* ast)
{
  assert(ast->kind == AST_name);
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
  assert(ast->kind == AST_specializedType);
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

internal void
visit_tuple(Ast* ast)
{
  assert(ast->kind == AST_tupleType);
  Ast_Tuple* tuple_decl = (Ast_Tuple*)ast;
  Ast_List* args = (Ast_List*)tuple_decl->type_args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* arg = li->object;
      visit_type_ref(arg);
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
    visit_block_statement(ctrl_decl->apply_stmt);
  }
}

internal void
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_controlTypeDeclaration);
  Ast_ControlProto* proto = (Ast_ControlProto*)ast;
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
}

internal void
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_externDeclaration);
  Ast_ExternType* extern_decl = (Ast_ExternType*)ast;
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
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_structTypeDeclaration);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Type_Type* struct_ty = arena_push_struct(type_storage, Type_Type);
  struct_ty->ctor = TYPE_TYPE;
  HashmapEntry* struct_he = hashmap_create_entry_uint32(&type_table, struct_decl->id);
  struct_he->object = (Type*)struct_ty;
  Ast_List* fields = (Ast_List*)struct_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
}

internal void
visit_header(Ast* ast)
{
  assert(ast->kind == AST_headerTypeDeclaration);
  Ast_Header* header_decl = (Ast_Header*)ast;
  Ast_List* fields = (Ast_List*)header_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
}

internal void
visit_header_union(Ast* ast)
{
  assert(ast->kind == AST_headerUnionDeclaration);
  Ast_HeaderUnion* union_decl = (Ast_HeaderUnion*)ast;
  Ast_List* fields = (Ast_List*)union_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      visit_struct_field(field);
    }
  }
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_packageTypeDeclaration);
  Ast_Package* package_decl = (Ast_Package*)ast;
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
}

internal void
visit_parser(Ast* ast)
{
  assert(ast->kind == AST_parserDeclaration);
  Ast_Parser* parser_decl = (Ast_Parser*)ast;
  visit_parser_proto(parser_decl->proto);
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
}

internal void
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_parserTypeDeclaration);
  Ast_ParserProto* proto = (Ast_ParserProto*)ast;
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
}

internal void
visit_instantiation(Ast* ast)
{
  assert(ast->kind == AST_instantiation);
  Ast_Instantiation* inst_decl = (Ast_Instantiation*)ast;
  visit_type_ref(inst_decl->type_ref);
  Ast_List* args = (Ast_List*)inst_decl->args;
  if (args) {
    for (ListItem* li = args->members.sentinel.next;
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
  visit_type_ref(type_decl->type_ref);
}

internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_functionDeclaration);
  Ast_Function* function_decl = (Ast_Function*)ast;
  visit_function_proto(function_decl->proto);
  Ast_BlockStmt* function_body = (Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    Ast_List* stmt_list = (Ast_List*)function_body->stmt_list;
    if (stmt_list) {
      for (ListItem* li = stmt_list->members.sentinel.next;
           li != 0; li = li->next) {
        Ast* stmt = li->object;
        visit_statement(stmt);
      }
    }
  }
}

internal void
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_functionPrototype);
  Ast_FunctionProto* proto = (Ast_FunctionProto*)ast;
  if (proto->return_type) {
    visit_type_ref(proto->return_type);
  }
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
}

internal void
visit_const(Ast* ast)
{
  assert(ast->kind == AST_constantDeclaration);
  Ast_Const* const_decl = (Ast_Const*)ast;
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->init_expr);
}

internal void
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_enumDeclaration);
  Ast_EnumDeclaration* enum_decl = (Ast_EnumDeclaration*)ast;
  Ast_List* fields = (Ast_List*)enum_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      if (field->kind == AST_name) {
        visit_enum_field(field);
      } else if (field->kind == AST_specifiedIdentifier) {
        visit_specified_identifier(field);
      }
      else assert(0);
    }
  }
}

internal void
visit_action(Ast* ast)
{
  assert(ast->kind == AST_actionDeclaration);
  Ast_Action* action_decl = (Ast_Action*)ast;
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
}

internal void
visit_match_kind(Ast* ast)
{
  assert(ast->kind == AST_matchKindDeclaration);
  Ast_MatchKindDeclaration* match_decl = (Ast_MatchKindDeclaration*)ast;
  Ast_List* fields = (Ast_List*)match_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      if (field->kind == AST_name) {
        visit_enum_field(field);
      } else if (field->kind == AST_specifiedIdentifier) {
        visit_specified_identifier(field);
      }
      else assert(0);
    }
  }
}

internal void
visit_error_enum(Ast* ast)
{
  assert (ast->kind == AST_errorDeclaration);
  Ast_ErrorDeclaration* error_decl = (Ast_ErrorDeclaration*)ast;
  Ast_List* fields = (Ast_List*)error_decl->fields;
  if (fields) {
    for (ListItem* li = fields->members.sentinel.next;
         li != 0; li = li->next) {
      Ast* field = li->object;
      if (field->kind == AST_name) {
        visit_enum_field(field);
      }
      else assert(0);
    }
  }
}

internal void
visit_p4program(Ast* ast)
{
  assert(ast->kind == AST_p4program);
  Ast_P4Program* program = (Ast_P4Program*)ast;
  Ast_List* decl_list = (Ast_List*)program->decl_list;
  if (decl_list) {
    for (ListItem* li = decl_list->members.sentinel.next;
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
      } else assert(0);
    }
  }
}

Hashmap*
build_type_decl(Ast_P4Program* p4program, Scope* root_scope_, Arena* type_storage_)
{
  root_scope = root_scope_;
  type_storage = type_storage_;
  hashmap_init(&type_table, HASHMAP_KEY_UINT32, 8, type_storage);

  {
    Ast* void_decl = scope_lookup_name(root_scope, "void")->ns_type->ast;
    Type* void_ty = arena_push_struct(type_storage, Type);
    void_ty->ctor = TYPE_VOID;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&type_table, void_decl->id);
    bit_he->object = void_ty;
  }
  {
    Ast* bool_decl = scope_lookup_name(root_scope, "bool")->ns_type->ast;
    Type* bool_ty = arena_push_struct(type_storage, Type);
    bool_ty->ctor = TYPE_BOOL;
    HashmapEntry* bool_he = hashmap_create_entry_uint32(&type_table, bool_decl->id);
    bool_he->object = bool_decl;
  }
  {
    Ast* int_decl = scope_lookup_name(root_scope, "int")->ns_type->ast;
    Type* int_ty = arena_push_struct(type_storage, Type);
    int_ty->ctor = TYPE_INT;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&type_table, int_decl->id);
    bit_he->object = int_ty;
  }
  {
    Ast* bit_decl = scope_lookup_name(root_scope, "bit")->ns_type->ast;
    Type* bit_ty = arena_push_struct(type_storage, Type);
    bit_ty->ctor = TYPE_BIT;
    HashmapEntry* bit_he = hashmap_create_entry_uint32(&type_table, bit_decl->id);
    bit_he->object = bit_ty;
  }
  {
    Ast* varbit_decl = scope_lookup_name(root_scope, "varbit")->ns_type->ast;
    Type* varbit_ty = arena_push_struct(type_storage, Type);
    varbit_ty->ctor = TYPE_VARBIT;
    HashmapEntry* varbit_he = hashmap_create_entry_uint32(&type_table, varbit_decl->id);
    varbit_he->object = varbit_ty;
  }
  {
    Ast* string_decl = scope_lookup_name(root_scope, "string")->ns_type->ast;
    Type* string_ty = arena_push_struct(type_storage, Type);
    string_ty->ctor = TYPE_STRING;
    HashmapEntry* string_he = hashmap_create_entry_uint32(&type_table, string_decl->id);
    string_he->object = string_ty;
  }

  visit_p4program((Ast*)p4program);
  return &type_table;
}

