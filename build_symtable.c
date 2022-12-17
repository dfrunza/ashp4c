#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal Arena* symtable_storage;
internal Scope* root_scope;
internal Scope* current_scope;
internal Hashmap nameref_map = {};

internal void visit_block_statement(Ast* block_stmt);
internal void visit_statement(Ast* decl);
internal void visit_expression(Ast* expr);
internal void visit_type_ref(Ast* type_ref);

NameRef*
nameref_get(Hashmap* map, uint32_t id)
{
  HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_get_entry(map, &key);
  NameRef* nameref = 0;
  if (he) {
    nameref = he->object;
  }
  return nameref;
}

void
nameref_add(Hashmap* map, NameRef* nameref, uint32_t id)
{
  HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  assert(!he->object);
  he->object = nameref;
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
visit_expression(Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    Ast_BinaryExpr* expr = (Ast_BinaryExpr*)ast;
    visit_expression(expr->left_operand);
    visit_expression(expr->right_operand);
  } else if (ast->kind == AST_UNARY_EXPR) {
    Ast_UnaryExpr* expr = (Ast_UnaryExpr*)ast;
    visit_expression(expr->operand);
  } else if (ast->kind == AST_NAME) {
    Ast_Name* name = (Ast_Name*)ast;
    NameRef* nameref = arena_push_struct(symtable_storage, NameRef);
    nameref->ast = (Ast*)name;
    nameref->strname = name->strname;
    nameref->line_no = name->line_no;
    nameref->column_no = name->column_no;
    nameref->scope = current_scope;
    nameref_add(&nameref_map, nameref, name->id);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT) {
    Ast_MemberSelect* expr = (Ast_MemberSelect*)ast;
    visit_expression(expr->lhs_expr);
    visit_expression(expr->member_name);
  } else if (ast->kind == AST_EXPRESSION_LIST) {
    Ast_ExpressionList* expr = (Ast_ExpressionList*)ast;
    Ast_NodeList* expr_list = &expr->expr_list;
    DList* li = expr_list->list.next;
    while (li) {
      Ast* expr_expr = li->object;
      visit_expression(expr_expr);
      li = li->next;
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    Ast_CastExpr* expr = (Ast_CastExpr*)ast;
    visit_type_ref(expr->to_type);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_SUBSCRIPT) {
    Ast_Subscript* expr = (Ast_Subscript*)ast;
    visit_expression(expr->index);
    if (expr->end_index) {
      visit_expression(expr->end_index);
    }
  } else if (ast->kind == AST_KVPAIR) {
    Ast_KVPair* expr = (Ast_KVPair*)ast;
    visit_expression(expr->name);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL || ast->kind == AST_STRING_LITERAL) {
    ; // pass
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
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
    declare_type_name(current_scope, name);
  } else {
    visit_type_ref((Ast*)name);
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
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
  Ast_Instantiation* ndecl = (Ast_Instantiation*)ast;
  Ast_Name* name = (Ast_Name*)ndecl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(ndecl->type);
  Ast_NodeList* args = &ndecl->args;
  DList* li = args->list.next;
  while (li) {
    Ast* arg = li->object;
    visit_expression(arg);
    li = li->next;
  }
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
visit_keyset_expr(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_select_keyset(Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    Ast_TupleKeyset* keyset = (Ast_TupleKeyset*)ast;
    Ast_NodeList* expr_list = &keyset->expr_list;
    DList* li = expr_list->list.next;
    while (li) {
      Ast* expr = li->object;
      visit_keyset_expr(expr);
      li = li->next;
    }
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
visit_table_property(Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    Ast_TableActions* prop = (Ast_TableActions*)ast;
    Ast_NodeList* action_list = &prop->action_list;
    DList* li = action_list->list.next;
    while (li) {
      Ast* action = li->object;
      visit_action_ref(action);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    Ast_TableSingleEntry* prop = (Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      visit_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    Ast_TableKey* prop = (Ast_TableKey*)ast;
    Ast_NodeList* keyelem_list = &prop->keyelem_list;
    DList* li = keyelem_list->list.next;
    while (li) {
      Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    Ast_TableEntries* prop = (Ast_TableEntries*)ast;
    Ast_NodeList* entries = &prop->entries;
    DList* li = entries->list.next;
    while (li) {
      Ast* entry = li->object;
      visit_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_table(Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  Ast_Table* table_decl = (Ast_Table*)ast;
  Ast_Name* name = (Ast_Name*)table_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
visit_switch_label(Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
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
visit_const(Ast* ast)
{
  assert(ast->kind == AST_CONST);
  Ast_Const* const_decl = (Ast_Const*)ast;
  Ast_Name* name = (Ast_Name*)const_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(const_decl->type);
  visit_expression(const_decl->expr);
}

internal void
visit_statement(Ast* ast)
{
  if (ast->kind == AST_VAR) {
    Ast_Var* var_decl = (Ast_Var*)ast;
    Ast_Name* name = (Ast_Name*)var_decl->name;
    NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
    if (!ne->ns_var) {
      declare_var_name(current_scope, name);
    } else error("at %d:%d redeclared name `%s`.",
                 name->line_no, name->column_no, name->strname);
    visit_type_ref(var_decl->type);
    if (var_decl->init_expr) {
      visit_expression(var_decl->init_expr);
    }
  } else if (ast->kind == AST_ACTION) {
    visit_action(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_TABLE) {
    visit_table(ast);
  } else if (ast->kind == AST_IF_STMT) {
    Ast_IfStmt* stmt = (Ast_IfStmt*)ast;
    Ast* if_stmt = stmt->stmt;
    visit_expression(stmt->cond_expr);
    visit_statement(if_stmt);
    Ast* else_stmt = stmt->else_stmt;
    if (else_stmt) {
      visit_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    Ast_SwitchStmt* stmt = (Ast_SwitchStmt*)ast;
    visit_expression(stmt->expr);
    Ast_NodeList* switch_cases = &stmt->switch_cases;
    DList* li = switch_cases->list.next;
    while (li) {
      Ast* switch_case = li->object;
      visit_switch_case(switch_case);
      li = li->next;
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    Ast_AssignmentStmt* stmt = (Ast_AssignmentStmt*)ast;
    visit_expression(stmt->lvalue);
    Ast* assign_expr = stmt->expr;
    visit_expression(assign_expr);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    Ast_ReturnStmt* stmt = (Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      visit_expression(stmt->expr);
    }
  } else if (ast->kind == AST_EXIT_STMT || ast->kind == AST_EMPTY_ELEMENT) {
    ; // pass
  }
  else assert(0);
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
visit_control_proto(Ast* ast)
{
  assert(ast->kind == AST_CONTROL_PROTO);
  Ast_ControlProto* type_decl = (Ast_ControlProto*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  declare_type_name(current_scope, name);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &type_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &type_decl->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_control(Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  Ast_Control* control_decl = (Ast_Control*)ast;
  Ast_ControlProto* proto = (Ast_ControlProto*)control_decl->proto;
  Ast_Name* name = (Ast_Name*)proto->name;
  declare_type_name(current_scope, name);
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
visit_local_parser_element(Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_VAR) {
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
visit_parser_transition(Ast* ast)
{
  if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
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
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
visit_parser_proto(Ast* ast)
{
  assert(ast->kind == AST_PARSER_PROTO);
  Ast_ParserProto* type_decl = (Ast_ParserProto*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  declare_type_name(current_scope, name);
  current_scope = push_scope();
  DList* li;
  Ast_NodeList* type_params = &type_decl->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &type_decl->params;
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
  declare_type_name(current_scope, name);
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
visit_function_proto(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  Ast_FunctionProto* function_proto = (Ast_FunctionProto*)ast;
  Ast_Name* name = (Ast_Name*)function_proto->name;
  declare_type_name(current_scope, name);
  current_scope = push_scope();
  if (function_proto->return_type) {
    visit_type_ref(function_proto->return_type);
  }
  DList* li;
  Ast_NodeList* type_params = &function_proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &function_proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  current_scope = pop_scope();
}
  
internal void
visit_function(Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  Ast_Function* function_decl = (Ast_Function*)ast;
  Ast_FunctionProto* function_proto = (Ast_FunctionProto*)function_decl->proto;
  Ast_Name* name = (Ast_Name*)function_proto->name;
  declare_type_name(current_scope, name);
  current_scope = push_scope();
  if (function_proto->return_type) {
    visit_type_ref(function_proto->return_type);
  }
  DList* li;
  Ast_NodeList* type_params = &function_proto->type_params;
  li = type_params->list.next;
  while (li) {
    Ast* type_param = li->object;
    visit_type_param(type_param);
    li = li->next;
  }
  Ast_NodeList* params = &function_proto->params;
  li = params->list.next;
  while (li) {
    Ast* param = li->object;
    visit_param(param);
    li = li->next;
  }
  Ast_BlockStmt* function_body = (Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    Ast_NodeList* stmt_list = &function_body->stmt_list;
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
visit_extern(Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  Ast_Extern* extern_decl = (Ast_Extern*)ast;
  Ast_Name* name = (Ast_Name*)extern_decl->name;
  declare_type_name(current_scope, name);
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

internal void
visit_struct_field(Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  Ast_StructField* field = (Ast_StructField*)ast;
  Ast_Name* name = (Ast_Name*)field->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
  visit_type_ref(field->type);
}

internal void
visit_struct(Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  Ast_Struct* struct_decl = (Ast_Struct*)ast;
  Ast_Name* name = (Ast_Name*)struct_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
visit_type_ref(Ast* ast)
{
  if (ast->kind == AST_BOOL_TYPE) {
    visit_expression(((Ast_BoolType*)ast)->name);
  } else if (ast->kind == AST_INT_TYPE) {
    visit_expression(((Ast_IntType*)ast)->name);
  } else if (ast->kind == AST_BIT_TYPE) {
    visit_expression(((Ast_BitType*)ast)->name);
  } else if (ast->kind == AST_VARBIT_TYPE) {
    visit_expression(((Ast_VarbitType*)ast)->name);
  } else if (ast->kind == AST_STRING_TYPE) {
    visit_expression(((Ast_StringType*)ast)->name);
  } else if (ast->kind == AST_VOID_TYPE) {
    visit_expression(((Ast_VoidType*)ast)->name);
  } else if (ast->kind == AST_ERROR_TYPE) {
    visit_expression(((Ast_ErrorType*)ast)->name);
  } else if (ast->kind == AST_HEADER_STACK) {
    Ast_HeaderStack* type_ref = (Ast_HeaderStack*)ast;
    visit_expression(type_ref->name);
    Ast* stack_expr = type_ref->stack_expr;
    visit_expression(stack_expr);
  } else if (ast->kind == AST_NAME) {
    Ast_Name* name = (Ast_Name*)ast;
    NameEntry* ne = scope_lookup_name(current_scope, name->strname);
    if (!ne->ns_type) {
      /* We got a type parameter. */
      declare_type_name(current_scope, name);
    } else {
      visit_expression(ast);
    }
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    Ast_SpecializedType* speclzd_type = (Ast_SpecializedType*)ast;
    visit_expression(speclzd_type->name);
    Ast_NodeList* type_args = &speclzd_type->type_args;
    DList* li = type_args->list.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    Ast_Tuple* type_ref = (Ast_Tuple*)ast;
    Ast_NodeList* type_args = &type_ref->type_args;
    DList* li = type_args->list.next;
    while (li) {
      Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_STRUCT) {
    visit_struct(ast);
  } else if (ast->kind == AST_HEADER) {
    visit_header(ast);
  } else if (ast->kind == AST_HEADER_UNION) {
    visit_header_union(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
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
    declare_var_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
}

internal void
visit_specified_id(Ast* ast)
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
visit_enum(Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  Ast_Enum* enum_decl = (Ast_Enum*)ast;
  Ast_Name* name = (Ast_Name*)enum_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
  current_scope = push_scope();
  Ast_NodeList* id_list = &enum_decl->id_list;
  DList* li = id_list->list.next;
  while (li) {
    Ast* id = li->object;
    if (id->kind == AST_SPECIFIED_IDENT) {
      visit_specified_id(id);
    }
    else assert(0);
    li = li->next;
  }
  current_scope = pop_scope();
}

internal void
visit_package(Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  Ast_Package* package_decl = (Ast_Package*)ast;
  Ast_Name* name = (Ast_Name*)package_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
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
visit_type(Ast* ast)
{
  assert(ast->kind == AST_TYPE);
  Ast_Type* type_decl = (Ast_Type*)ast;
  Ast_Name* name = (Ast_Name*)type_decl->name;
  NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    declare_type_name(current_scope, name);
  } else error("at %d:%d redeclared name `%s`.",
               name->line_no, name->column_no, name->strname);
  Ast* type_ref = type_decl->type_ref;
  visit_type_ref(type_ref);
}

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
      visit_specified_id(id);
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
      visit_const(decl);
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
build_symtable(Ast_P4Program* p4program, Arena* symtable_storage_,
               /*out*/Hashmap** nameref_map_)
{
  symtable_storage = symtable_storage_;
  scope_init(symtable_storage);
  hashmap_init(&nameref_map, HASHMAP_KEY_UINT32, 8, symtable_storage);
  root_scope = current_scope = push_scope();

  {
    Ast_Name* void_type = arena_push_struct(symtable_storage, Ast_Name);
    void_type->kind = AST_NAME;
    void_type->strname = "void";
    void_type->id = ++p4program->last_node_id;
    declare_type_name(root_scope, void_type);
  }
  {
    Ast_Name* bool_type = arena_push_struct(symtable_storage, Ast_Name);
    bool_type->kind = AST_NAME;
    bool_type->id = ++p4program->last_node_id;
    bool_type->strname = "bool";
    declare_type_name(root_scope, bool_type);
  }
  {
    Ast_Name* int_type = arena_push_struct(symtable_storage, Ast_Name);
    int_type->kind = AST_NAME;
    int_type->id = ++p4program->last_node_id;
    int_type->strname = "int";
    declare_type_name(root_scope, int_type);
  }
  {
    Ast_Name* bit_type = arena_push_struct(symtable_storage, Ast_Name);
    bit_type->kind = AST_NAME;
    bit_type->id = ++p4program->last_node_id;
    bit_type->strname = "bit";
    declare_type_name(root_scope, bit_type);
  }
  {
    Ast_Name* varbit_type = arena_push_struct(symtable_storage, Ast_Name);
    varbit_type->kind = AST_NAME;
    varbit_type->id = ++p4program->last_node_id;
    varbit_type->strname = "varbit";
    declare_type_name(root_scope, varbit_type);
  }
  {
    Ast_Name* string_type = arena_push_struct(symtable_storage, Ast_Name);
    string_type->kind = AST_NAME;
    string_type->id = ++p4program->last_node_id;
    string_type->strname = "string";
    declare_type_name(root_scope, string_type);
  }
  {
    Ast_Name* error_type = arena_push_struct(symtable_storage, Ast_Name);
    error_type->kind = AST_NAME;
    error_type->id = ++p4program->last_node_id;
    error_type->strname = "error";
    declare_type_name(root_scope, error_type);
  }
  {
    Ast_Name* match_type = arena_push_struct(symtable_storage, Ast_Name);
    match_type->kind = AST_NAME;
    match_type->id = ++p4program->last_node_id;
    match_type->strname = "match_kind";
    declare_type_name(root_scope, match_type);
  }
  {
    Ast_Name* accept_state = arena_push_struct(symtable_storage, Ast_Name);
    accept_state->kind = AST_NAME;
    accept_state->id = ++p4program->last_node_id;
    accept_state->strname = "accept";
    declare_var_name(root_scope, accept_state);
  }
  {
    Ast_Name* reject_state = arena_push_struct(symtable_storage, Ast_Name);
    reject_state->kind = AST_NAME;
    reject_state->id = ++p4program->last_node_id;
    reject_state->strname = "reject";
    declare_var_name(root_scope, reject_state);
  }

  visit_p4program((Ast*)p4program);
  current_scope = pop_scope();
  assert(current_scope == 0);
  *nameref_map_ = &nameref_map;

  return root_scope;
}
