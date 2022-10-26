#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal struct Arena* symtable_storage;
internal struct Scope* root_scope;
internal struct Scope* current_scope;
internal struct Hashmap nameref_map = {};

internal void visit_block_statement(struct Ast* block_stmt);
internal void visit_statement(struct Ast* decl);
internal void visit_expression(struct Ast* expr);
internal void visit_type_ref(struct Ast* type_ref);

struct NameRef*
nameref_get(struct Hashmap* map, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_entry(map, &key);
  struct NameRef* nameref = 0;
  if (he) {
    nameref = he->object;
  }
  return nameref;
}

void
nameref_add(struct Hashmap* map, struct NameRef* nameref, uint32_t id)
{
  struct HashmapKey key = { .i_key = id };
  hashmap_hash_key(HASHMAP_KEY_UINT32, &key, map->capacity_log2);
  struct HashmapEntry* he = hashmap_get_or_create_entry(map, &key);
  assert(!he->object);
  he->object = nameref;
}

internal void
visit_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL);
  struct Ast_FunctionCall* expr = (struct Ast_FunctionCall*)ast;
  visit_expression(expr->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(expr->callee_expr);
  if (callee_expr->type_args) {
    struct DList* li = callee_expr->type_args;
    while (li) {
      struct Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  }
  if (expr->args) {
    struct DList* li = expr->args;
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_expression(struct Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    visit_expression(expr->left_operand);
    visit_expression(expr->right_operand);
  } else if (ast->kind == AST_UNARY_EXPR) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    visit_expression(expr->operand);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameRef* nameref = arena_push_struct(symtable_storage, struct NameRef);
    nameref->strname = name->strname;
    nameref->line_no = name->line_no;
    nameref->scope = current_scope;
    nameref_add(&nameref_map, nameref, name->id);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT) {
    struct Ast_MemberSelect* expr = (struct Ast_MemberSelect*)ast;
    visit_expression(expr->lhs_expr);
    visit_expression(expr->member_name);
  } else if (ast->kind == AST_EXPRLIST) {
    struct Ast_ExprList* expr = (struct Ast_ExprList*)ast;
    if (expr->expr_list) {
      struct DList* li = expr->expr_list;
      while (li) {
        struct Ast* expr_expr = li->object;
        visit_expression(expr_expr);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    visit_type_ref(expr->to_type);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_SUBSCRIPT) {
    struct Ast_Subscript* expr = (struct Ast_Subscript*)ast;
    visit_expression(expr->index);
    if (expr->colon_index) {
      visit_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KVPAIR) {
    struct Ast_KVPair* expr = (struct Ast_KVPair*)ast;
    visit_expression(expr->name);
    visit_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL || ast->kind == AST_STRING_LITERAL) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  visit_type_ref(param->type);
}

internal void
visit_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameEntry* ne = scope_lookup_name(current_scope, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else {
    visit_type_ref((struct Ast*)name);
  }
}

internal void
visit_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION);
  struct Ast_Action* action_decl = (struct Ast_Action*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (action_decl->params) {
    struct DList* li = action_decl->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    if (action_body->stmt_list) {
      struct DList* li = action_body->stmt_list;
      while (li) {
        struct Ast* stmt = li->object;
        visit_statement(stmt);
        li = li->next;
      }
    }
  }
  current_scope = pop_scope();
}

internal void
visit_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  visit_type_ref(decl->type_ref);
  if (decl->args) {
    struct DList* li = decl->args;
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_action_ref(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  visit_expression(action->name);
  if (action->args) {
    struct DList* li = action->args;
    while (li) {
      struct Ast* arg = li->object;
      visit_expression(arg);
      li = li->next;
    }
  }
}

internal void
visit_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  visit_expression(keyelem->expr);
  visit_expression(keyelem->name);
}

internal void
visit_keyset_expr(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_select_keyset(struct Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct DList* li = keyset->expr_list;
    while (li) {
      struct Ast* expr = li->object;
      visit_keyset_expr(expr);
      li = li->next;
    }
  } else {
    visit_keyset_expr(ast);
  }
}

internal void
visit_table_entry(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  visit_select_keyset(entry->keyset);
  visit_action_ref(entry->action);
}

internal void
visit_table_property(struct Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    struct Ast_TableActions* prop = (struct Ast_TableActions*)ast;
    if (prop->action_list) {
      struct DList* li = prop->action_list;
      while (li) {
        struct Ast* action = li->object;
        visit_action_ref(action);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      visit_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct DList* li = prop->keyelem_list;
    while (li) {
      struct Ast* keyelem = li->object;
      visit_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct DList* li = prop->entries;
    while (li) {
      struct Ast* entry = li->object;
      visit_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE);
  struct Ast_Table* decl = (struct Ast_Table*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  if (decl->prop_list) {
    struct DList* li = decl->prop_list;
    while (li) {
      struct Ast* prop = li->object;
      visit_table_property(prop);
      li = li->next;
    }
  }
}

internal void
visit_switch_label(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    visit_expression(ast);
  }
}

internal void
visit_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  visit_switch_label(switch_case->label);
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    visit_block_statement(case_stmt);
  }
}

internal void
visit_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST);
  struct Ast_Const* decl = (struct Ast_Const*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  visit_type_ref(decl->type_ref);
  visit_expression(decl->expr);
}

internal void
visit_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR) {
    struct Ast_Var* decl = (struct Ast_Var*)ast;
    struct Ast_Name* name = (struct Ast_Name*)decl->name;
    struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
    if (!ne->ns_var) {
      struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
      decl->ast = ast;
      decl->strname = name->strname;
      decl->line_no = name->line_no;
      declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
    } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
    visit_type_ref(decl->type);
    if (decl->init_expr) {
      visit_expression(decl->init_expr);
    }
  } else if (ast->kind == AST_ACTION) {
    visit_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    visit_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_TABLE) {
    visit_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = stmt->stmt;
    visit_expression(stmt->cond_expr);
    visit_statement(if_stmt);
    struct Ast* else_stmt = stmt->else_stmt;
    if (else_stmt) {
      visit_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    visit_expression(stmt->expr);
    if (stmt->switch_cases) {
      struct DList* li = stmt->switch_cases;
      while (li) {
        struct Ast* switch_case = li->object;
        visit_switch_case(switch_case);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    visit_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    visit_expression(assign_expr);
  } else if (ast->kind == AST_FUNCTION_CALL) {
    visit_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      visit_expression(stmt->expr);
    }
  } else if (ast->kind == AST_EXIT_STMT || ast->kind == AST_EMPTY_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  current_scope = push_scope();
  if (block_stmt->stmt_list) {
    struct DList* li = block_stmt->stmt_list;
    while (li) {
      struct Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL);
  struct Ast_Control* control_decl = (struct Ast_Control*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  current_scope = push_scope();
  if (type_decl->type_params) {
    struct DList* li = type_decl->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct DList* li = type_decl->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  if (control_decl->ctor_params) {
    struct DList* li = control_decl->ctor_params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  if (control_decl->local_decls) {
    struct DList* li = control_decl->local_decls;
    while (li) {
      struct Ast* decl = li->object;
      visit_statement(decl);
      li = li->next;
    }
  }
  if (control_decl->apply_stmt) {
    visit_block_statement(control_decl->apply_stmt);
  }
  current_scope = pop_scope();
}

internal void
visit_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST) {
    visit_const_decl(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    visit_instantiation(ast);
  } else if (ast->kind == AST_VAR) {
    visit_statement(ast);
  } else assert(0);
}

internal void
visit_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  visit_select_keyset(select_case->keyset);
  visit_expression(select_case->name);
}

internal void
visit_parser_transition(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct DList* li = trans_stmt->expr_list;
    while (li) {
      struct Ast* expr = li->object;
      visit_expression(expr);
      li = li->next;
    }
    li = trans_stmt->case_list;
    while (li) {
      struct Ast* select_case = li->object;
      visit_transition_select_case(select_case);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
visit_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  struct Ast_Name* name = (struct Ast_Name*)state->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (state->stmt_list) {
    struct DList* li = state->stmt_list;
    while (li) {
      struct Ast* stmt = li->object;
      visit_statement(stmt);
      li = li->next;
    }
  }
  visit_parser_transition(state->trans_stmt);
  current_scope = pop_scope();
}

internal void
visit_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER);
  struct Ast_Parser* parser_decl = (struct Ast_Parser*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  current_scope = push_scope();
  if (type_decl->type_params) {
    struct DList* li = type_decl->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct DList* li = type_decl->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct DList* li = parser_decl->ctor_params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  if (parser_decl->local_elements) {
    struct DList* li = parser_decl->local_elements;
    while (li) {
      struct Ast* element = li->object;
      visit_local_parser_element(element);
      li = li->next;
    }
  }
  if (parser_decl->states) {
    struct DList* li = parser_decl->states;
    while (li) {
      struct Ast* state = li->object;
      visit_parser_state(state);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameEntry* ne = scope_lookup_name(current_scope, name->strname);
    if (!ne->ns_type) {
      struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
      decl->ast = ast;
      decl->strname = name->strname;
      decl->line_no = name->line_no;
      declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
    } else {
      visit_type_ref(ast);
    }
  } else {
    visit_type_ref(ast);
  }
}

internal void
visit_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  current_scope = push_scope();
  if (function_proto->return_type) {
    visit_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct DList* li = function_proto->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (function_proto->params) {
    struct DList* li = function_proto->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN);
  struct Ast_Extern* extern_decl = (struct Ast_Extern*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  current_scope = push_scope();
  if (extern_decl->type_params) {
    struct DList* li = extern_decl->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (extern_decl->method_protos) {
    struct DList* li = extern_decl->method_protos;
    while (li) {
      struct Ast* proto = li->object;
      visit_function_proto(proto);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_struct_field(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  struct Ast_Name* name = (struct Ast_Name*)field->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  visit_type_ref(field->type);
}

internal void
visit_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT);
  struct Ast_Struct* struct_decl = (struct Ast_Struct*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (struct_decl->fields) {
    struct DList* li = struct_decl->fields;
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER);
  struct Ast_Header* header_decl = (struct Ast_Header*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (header_decl->fields) {
    struct DList* li = header_decl->fields;
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_header_union_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION);
  struct Ast_HeaderUnion* header_union_decl = (struct Ast_HeaderUnion*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_union_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (header_union_decl->fields) {
    struct DList* li = header_union_decl->fields;
    while (li) {
      struct Ast* field = li->object;
      visit_struct_field(field);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_type_ref(struct Ast* ast)
{
  if (ast->kind == AST_BOOL_TYPE || ast->kind == AST_ERROR_TYPE
      || ast->kind == AST_INT_TYPE || ast->kind == AST_BIT_TYPE
      || ast->kind == AST_VARBIT_TYPE || ast->kind == AST_STRING_TYPE
      || ast->kind == AST_VOID_TYPE) {
    visit_expression(((struct Ast_BasicType*)ast)->name);
  } else if (ast->kind == AST_HEADER_STACK) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    visit_expression(type_ref->name);
    struct Ast* stack_expr = type_ref->stack_expr;
    visit_expression(stack_expr);
  } else if (ast->kind == AST_NAME) {
    visit_expression(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* speclzd_type = (struct Ast_SpecializedType*)ast;
    visit_expression(speclzd_type->name);
    struct DList* li = speclzd_type->type_args;
    while (li) {
      struct Ast* type_arg = li->object;
      visit_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct DList* li = type_ref->type_args;
      while (li) {
        struct Ast* type_arg = li->object;
        visit_type_ref(type_arg);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_STRUCT) {
    visit_struct_decl(ast);
  } else if (ast->kind == AST_HEADER) {
    visit_header_decl(ast);
  } else if (ast->kind == AST_HEADER_UNION) {
    visit_header_union_decl(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
visit_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
}

internal void
visit_specified_id(struct Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  struct Ast_Name* name = (struct Ast_Name*)id->name;
  visit_enum_field((struct Ast*)name);
  struct Ast* init_expr = id->init_expr;
  if (init_expr) {
    visit_expression(init_expr);
  }
}

internal void
visit_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM);
  struct Ast_Enum* enum_decl = (struct Ast_Enum*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (enum_decl->id_list) {
    struct DList* li = enum_decl->id_list;
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_SPECIFIED_IDENT) {
        visit_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_package_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE);
  struct Ast_Package* package_decl = (struct Ast_Package*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  current_scope = push_scope();
  if (package_decl->type_params) {
    struct DList* li = package_decl->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (package_decl->params) {
    struct DList* li = package_decl->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE);
  struct Ast_Type* type_decl = (struct Ast_Type*)ast;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameEntry* ne = namedecl_get_or_create(&current_scope->decls, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->ast = ast;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  struct Ast* type_ref = type_decl->type_ref;
  visit_type_ref(type_ref);
}

internal void
visit_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION);
  struct Ast_Function* function_decl = (struct Ast_Function*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
  decl->ast = ast;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
  current_scope = push_scope();
  if (function_proto->return_type) {
    visit_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct DList* li = function_proto->type_params;
    while (li) {
      struct Ast* type_param = li->object;
      visit_type_param(type_param);
      li = li->next;
    }
  }
  if (function_proto->params) {
    struct DList* li = function_proto->params;
    while (li) {
      struct Ast* param = li->object;
      visit_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct DList* li = function_body->stmt_list;
      while (li) {
        struct Ast* stmt = li->object;
        visit_statement(stmt);
        li = li->next;
      }
    }
  }
  current_scope = pop_scope();
}

internal void
visit_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND);
  struct Ast_MatchKind* decl = (struct Ast_MatchKind*)ast;
  assert(current_scope->scope_level == 1);
  if (decl->id_list) {
    struct DList* li = decl->id_list;
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        visit_enum_field(id);
      } else if (id->kind == AST_SPECIFIED_IDENT) {
        visit_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
visit_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR);
  struct Ast_Error* decl = (struct Ast_Error*)ast;
  current_scope = push_scope();
  if (decl->id_list) {
    struct DList* li = decl->id_list;
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        visit_enum_field(id);
      }
      else assert(0);
      li = li->next;
    }
  }
  current_scope = pop_scope();
}

internal void
visit_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  current_scope = push_scope();
  struct DList* li = program->decl_list;
  while (li) {
    struct Ast* decl = li->object;
    if (decl->kind == AST_CONTROL) {
      visit_control_decl(decl);
    } else if (decl->kind == AST_EXTERN) {
      visit_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT) {
      visit_struct_decl(decl);
    } else if (decl->kind == AST_HEADER) {
      visit_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION) {
      visit_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE) {
      visit_package_decl(decl);
    } else if (decl->kind == AST_PARSER) {
      visit_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      visit_instantiation(decl);
    } else if (decl->kind == AST_TYPE) {
      visit_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      visit_function_proto(decl);
    } else if (decl->kind == AST_CONST) {
      visit_const_decl(decl);
    } else if (decl->kind == AST_ENUM) {
      visit_enum_decl(decl);
    } else if (decl->kind == AST_FUNCTION) {
      visit_function_decl(decl);
    } else if (decl->kind == AST_ACTION) {
      visit_action_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND) {
      visit_match_kind(decl);
    } else if (decl->kind == AST_ERROR) {
      visit_error_decl(decl);
    }
    else assert(0);
    li = li->next;
  }
  current_scope = pop_scope();
}

struct Scope*
build_symtable(struct Ast_P4Program* p4program, struct Arena* symtable_storage_,
               /*out*/struct Hashmap** nameref_map_)
{
  struct NameDecl*
  declare_builtin_ident(struct Ast* ast, char* strname, enum Namespace ns)
  {
    struct NameDecl* decl = arena_push_struct(symtable_storage, struct NameDecl);
    decl->strname = strname;
    decl->ast = ast;
    declare_name_in_scope(root_scope, ns, decl);
    return decl;
  }

  symtable_storage = symtable_storage_;
  scope_init(symtable_storage);
  hashmap_init(&nameref_map, HASHMAP_KEY_UINT32, 8, symtable_storage);
  root_scope = current_scope = push_scope();

  struct Ast_VoidType* void_type = arena_push_struct(symtable_storage, struct Ast_VoidType);
  void_type->kind = AST_VOID_TYPE;
  void_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)void_type, "void", NAMESPACE_TYPE);

  struct Ast_BoolType* bool_type = arena_push_struct(symtable_storage, struct Ast_BoolType);
  bool_type->kind = AST_BOOL_TYPE;
  bool_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)bool_type, "bool", NAMESPACE_TYPE);

  struct Ast_IntType* int_type = arena_push_struct(symtable_storage, struct Ast_IntType);
  int_type->kind = AST_INT_TYPE;
  int_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)int_type, "int", NAMESPACE_TYPE);

  struct Ast_BitType* bit_type = arena_push_struct(symtable_storage, struct Ast_BitType);
  bit_type->kind = AST_BIT_TYPE;
  bit_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)bit_type, "bit", NAMESPACE_TYPE);

  struct Ast_VarbitType* varbit_type = arena_push_struct(symtable_storage, struct Ast_VarbitType);
  varbit_type->kind = AST_VARBIT_TYPE;
  varbit_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)varbit_type, "varbit", NAMESPACE_TYPE);

  struct Ast_StringType* string_type = arena_push_struct(symtable_storage, struct Ast_StringType);
  string_type->kind = AST_STRING_TYPE;
  string_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)string_type, "string", NAMESPACE_TYPE);

  struct Ast_ErrorType* error_type = arena_push_struct(symtable_storage, struct Ast_ErrorType);
  error_type->kind = AST_ERROR_TYPE;
  error_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)error_type, "error", NAMESPACE_TYPE);

  struct Ast_MatchKind* mk_type = arena_push_struct(symtable_storage, struct Ast_MatchKind);
  mk_type->kind = AST_MATCH_KIND;
  mk_type->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)mk_type, "match_kind", NAMESPACE_TYPE);

  struct Ast_ParserState* accept_state = arena_push_struct(symtable_storage, struct Ast_ParserState);
  accept_state->kind = AST_PARSER_STATE;
  accept_state->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)accept_state, "accept", NAMESPACE_VAR);

  struct Ast_ParserState* reject_state = arena_push_struct(symtable_storage, struct Ast_ParserState);
  reject_state->kind = AST_PARSER_STATE;
  reject_state->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)reject_state, "reject", NAMESPACE_VAR);

  struct Ast_Error* error_var = arena_push_struct(symtable_storage, struct Ast_Error);
  error_var->kind = AST_ERROR;
  error_var->id = ++p4program->last_node_id;
  declare_builtin_ident((struct Ast*)error_var, "error", NAMESPACE_VAR);

  visit_p4program((struct Ast*)p4program);
  current_scope = pop_scope();
  assert(current_scope == 0);
  *nameref_map_ = &nameref_map;

  return root_scope;
}
