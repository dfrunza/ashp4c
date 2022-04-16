#include "arena.h"
#include "ast.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* m_symtable_storage;


internal void build_symtable_block_statement(struct Ast* block_stmt);
internal void build_symtable_statement(struct Ast* decl);
internal void build_symtable_expression(struct Ast* expr);
internal void build_symtable_type_ref(struct Ast* type_ref);


#define new_name_decl(obj_type, obj_kind) ({ \
  obj_type* decl = arena_push(m_symtable_storage, sizeof(obj_type)); \
  memset(decl, 0, sizeof(obj_type)); \
  decl->kind = obj_kind; \
  decl; \
})

internal void
build_symtable_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_CALL_EXPR);
  struct Ast_FunctionCallExpr* expr = (struct Ast_FunctionCallExpr*)ast;
  build_symtable_expression(expr->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(expr->callee_expr);
  if (callee_expr->type_args) {
    struct ListLink* li = list_first_link(callee_expr->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      build_symtable_type_ref(type_arg);
      li = li->next;
    }
  }
  if (expr->args) {
    struct ListLink* li = list_first_link(expr->args);
    while (li) {
      struct Ast* arg = li->object;
      build_symtable_expression(arg);
      li = li->next;
    }
  }
}

internal void
build_symtable_expression(struct Ast* ast)
{
  if (ast->kind == AST_BINARY_EXPR) {
    struct Ast_BinaryExpr* expr = (struct Ast_BinaryExpr*)ast;
    build_symtable_expression(expr->left_operand);
    build_symtable_expression(expr->right_operand);
  } else if (ast->kind == AST_UNARY_EXPR) {
    struct Ast_UnaryExpr* expr = (struct Ast_UnaryExpr*)ast;
    build_symtable_expression(expr->operand);
  } else if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameRef* nameref = arena_push(m_symtable_storage, sizeof(struct NameRef));
    memset(nameref, 0, sizeof(*nameref));
    nameref->id = name->id;
    nameref->strname = name->strname;
    nameref->line_no = name->line_no;
    nameref->scope = get_current_scope();
    name->ref = nameref;
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    build_symtable_function_call(ast);
  } else if (ast->kind == AST_MEMBER_SELECT_EXPR) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    build_symtable_expression(expr->lhs_expr);
    build_symtable_expression(expr->member_name);
  } else if (ast->kind == AST_EXPRLIST_EXPR) {
    struct Ast_ExprListExpr* expr = (struct Ast_ExprListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* li = list_first_link(expr->expr_list);
      while (li) {
        struct Ast* expr_expr = li->object;
        build_symtable_expression(expr_expr);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    build_symtable_type_ref(expr->to_type);
    build_symtable_expression(expr->expr);
  } else if (ast->kind == AST_SUBSCRIPT_EXPR) {
    struct Ast_SubscriptExpr* expr = (struct Ast_SubscriptExpr*)ast;
    build_symtable_expression(expr->index);
    if (expr->colon_index) {
      build_symtable_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KVPAIR_EXPR) {
    struct Ast_KeyValuePairExpr* expr = (struct Ast_KeyValuePairExpr*)ast;
    build_symtable_expression(expr->name);
    build_symtable_expression(expr->expr);
  } else if (ast->kind == AST_INT_LITERAL || ast->kind == AST_BOOL_LITERAL || ast->kind == AST_STRING_LITERAL) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_param(struct Ast* ast)
{
  assert(ast->kind == AST_PARAM);
  struct Ast_Param* param = (struct Ast_Param*)ast;
  struct Ast_Name* name = (struct Ast_Name*)param->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PARAM);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  build_symtable_type_ref(param->type);
}

internal void
build_symtable_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameEntry* se = scope_lookup_name(get_current_scope(), name->strname);
  if (!se->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_TYPE_PARAM);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else {
    build_symtable_type_ref((struct Ast*)name);
  }
}

internal void
build_symtable_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_DECL);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_ACTION_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* li = list_first_link(params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    struct List* stmt_list = action_body->stmt_list;
    if (stmt_list) {
      struct ListLink* li = list_first_link(stmt_list);
      while (li) {
        struct Ast* stmt = li->object;
        build_symtable_statement(stmt);
        li = li->next;
      }
    }
  }
  pop_scope();
}

internal void
build_symtable_instantiation(struct Ast* ast)
{
  assert(ast->kind == AST_INSTANTIATION);
  struct Ast_Instantiation* decl = (struct Ast_Instantiation*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_INSTANTIATION);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  build_symtable_type_ref(decl->type_ref);
  if (decl->args) {
    struct ListLink* li = list_first_link(decl->args);
    while (li) {
      struct Ast* arg = li->object;
      build_symtable_expression(arg);
      li = li->next;
    }
  }
}

internal void
build_symtable_action_ref(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_REF);
  struct Ast_ActionRef* action = (struct Ast_ActionRef*)ast;
  build_symtable_expression(action->name);
  if (action->args) {
    struct ListLink* li = list_first_link(action->args);
    while (li) {
      struct Ast* arg = li->object;
      build_symtable_expression(arg);
      li = li->next;
    }
  }
}

internal void
build_symtable_table_keyelem(struct Ast* ast)
{
  assert(ast->kind == AST_KEY_ELEMENT);
  struct Ast_KeyElement* keyelem = (struct Ast_KeyElement*)ast;
  build_symtable_expression(keyelem->expr);
  build_symtable_expression(keyelem->name);
}

internal void
build_symtable_keyset_expr(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT || ast->kind == AST_DONTCARE) {
    ; // pass
  } else {
    build_symtable_expression(ast);
  }
}

internal void
build_symtable_select_keyset(struct Ast* ast)
{
  if (ast->kind == AST_TUPLE_KEYSET) {
    struct Ast_TupleKeyset* keyset = (struct Ast_TupleKeyset*)ast;
    struct ListLink* li = list_first_link(keyset->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      build_symtable_keyset_expr(expr);
      li = li->next;
    }
  } else {
    build_symtable_keyset_expr(ast);
  }
}

internal void
build_symtable_table_entry(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_ENTRY);
  struct Ast_TableEntry* entry = (struct Ast_TableEntry*)ast;
  build_symtable_select_keyset(entry->keyset);
  build_symtable_action_ref(entry->action);
}

internal void
build_symtable_table_property(struct Ast* ast)
{
  if (ast->kind == AST_TABLE_ACTIONS) {
    struct Ast_TableActions* prop = (struct Ast_TableActions*)ast;
    if (prop->action_list) {
      struct ListLink* li = list_first_link(prop->action_list);
      while (li) {
        struct Ast* action = li->object;
        build_symtable_action_ref(action);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      build_symtable_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct ListLink* li = list_first_link(prop->keyelem_list);
    while (li) {
      struct Ast* keyelem = li->object;
      build_symtable_table_keyelem(keyelem);
      li = li->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct ListLink* li = list_first_link(prop->entries);
    while (li) {
      struct Ast* entry = li->object;
      build_symtable_table_entry(entry);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
build_symtable_table_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TABLE_DECL);
  struct Ast_TableDecl* decl = (struct Ast_TableDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_TABLE_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  if (decl->prop_list) {
    struct ListLink* li = list_first_link(decl->prop_list);
    while (li) {
      struct Ast* prop = li->object;
      build_symtable_table_property(prop);
      li = li->next;
    }
  }
}

internal void
build_symtable_switch_label(struct Ast* ast)
{
  if (ast->kind == AST_DEFAULT_STMT) {
    ; // pass
  } else {
    build_symtable_expression(ast);
  }
}

internal void
build_symtable_switch_case(struct Ast* ast)
{
  assert(ast->kind == AST_SWITCH_CASE);
  struct Ast_SwitchCase* switch_case = (struct Ast_SwitchCase*)ast;
  build_symtable_switch_label(switch_case->label);
  struct Ast* case_stmt = switch_case->stmt;
  if (case_stmt && case_stmt->kind == AST_BLOCK_STMT) {
    build_symtable_block_statement(case_stmt);
  }
}

internal void
build_symtable_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST_DECL);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_CONST_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  build_symtable_type_ref(decl->type_ref);
  build_symtable_expression(decl->expr);
}

internal void
build_symtable_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    struct Ast_Name* name = (struct Ast_Name*)decl->name;
    struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
    if (!ne->ns_var) {
      struct NameDecl* decl = new_name_decl(struct NameDecl, AST_VAR_DECL);
      decl->id = name->id;
      decl->strname = name->strname;
      decl->line_no = name->line_no;
      declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
    } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
    build_symtable_type_ref(decl->type);
    if (decl->init_expr) {
      build_symtable_expression(decl->init_expr);
    }
  } else if (ast->kind == AST_ACTION_DECL) {
    build_symtable_action_decl(ast);
  } else if (ast->kind == AST_BLOCK_STMT) {
    build_symtable_block_statement(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    build_symtable_instantiation(ast);
  } else if (ast->kind == AST_TABLE_DECL) {
    build_symtable_table_decl(ast);
  } else if (ast->kind == AST_IF_STMT) {
    struct Ast_IfStmt* stmt = (struct Ast_IfStmt*)ast;
    struct Ast* if_stmt = stmt->stmt;
    build_symtable_expression(stmt->cond_expr);
    build_symtable_statement(if_stmt);
    struct Ast* else_stmt = stmt->else_stmt;
    if (else_stmt) {
      build_symtable_statement(else_stmt);
    }
  } else if (ast->kind == AST_SWITCH_STMT) {
    struct Ast_SwitchStmt* stmt = (struct Ast_SwitchStmt*)ast;
    build_symtable_expression(stmt->expr);
    if (stmt->switch_cases) {
      struct ListLink* li = list_first_link(stmt->switch_cases);
      while (li) {
        struct Ast* switch_case = li->object;
        build_symtable_switch_case(switch_case);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    build_symtable_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    build_symtable_expression(assign_expr);
  } else if (ast->kind == AST_FUNCTION_CALL_EXPR) {
    build_symtable_function_call(ast);
  } else if (ast->kind == AST_RETURN_STMT) {
    struct Ast_ReturnStmt* stmt = (struct Ast_ReturnStmt*)ast;
    if (stmt->expr) {
      build_symtable_expression(stmt->expr);
    }
  } else if (ast->kind == AST_EXIT_STMT) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_block_statement(struct Ast* ast)
{
  assert(ast->kind == AST_BLOCK_STMT);
  struct Ast_BlockStmt* block_stmt = (struct Ast_BlockStmt*)ast;
  push_scope();
  if (block_stmt->stmt_list) {
    struct ListLink* li = list_first_link(block_stmt->stmt_list);
    while (li) {
      struct Ast* decl = li->object;
      build_symtable_statement(decl);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_control_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONTROL_DECL);
  struct Ast_ControlDecl* control_decl = (struct Ast_ControlDecl*)ast;
  struct Ast_ControlProto* type_decl = (struct Ast_ControlProto*)control_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameDecl* decl = new_name_decl(struct NameDecl, AST_CONTROL_DECL);
  decl->id = name->id;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  push_scope();
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* li = list_first_link(control_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* li = list_first_link(control_decl->local_decls);
    while (li) {
      struct Ast* decl = li->object;
      build_symtable_statement(decl);
      li = li->next;
    }
  }
  if (control_decl->apply_stmt) {
    build_symtable_block_statement(control_decl->apply_stmt);
  }
  pop_scope();
}

internal void
build_symtable_local_parser_element(struct Ast* ast)
{
  if (ast->kind == AST_CONST_DECL) {
    build_symtable_const_decl(ast);
  } else if (ast->kind == AST_INSTANTIATION) {
    build_symtable_instantiation(ast);
  } else if (ast->kind == AST_VAR_DECL) {
    build_symtable_statement(ast);
  } else assert(0);
}

internal void
build_symtable_transition_select_case(struct Ast* ast)
{
  assert(ast->kind == AST_SELECT_CASE);
  struct Ast_SelectCase* select_case = (struct Ast_SelectCase*)ast;
  build_symtable_select_keyset(select_case->keyset);
  build_symtable_expression(select_case->name);
}

internal void
build_symtable_parser_transition(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    build_symtable_expression(ast);
  } else if (ast->kind == AST_SELECT_EXPR) {
    struct Ast_SelectExpr* trans_stmt = (struct Ast_SelectExpr*)ast;
    struct ListLink* li = list_first_link(trans_stmt->expr_list);
    while (li) {
      struct Ast* expr = li->object;
      build_symtable_expression(expr);
      li = li->next;
    }
    li = list_first_link(trans_stmt->case_list);
    while (li) {
      struct Ast* select_case = li->object;
      build_symtable_transition_select_case(select_case);
      li = li->next;
    }
  }
  else assert(0);
}

internal void
build_symtable_parser_state(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_STATE);
  struct Ast_ParserState* state = (struct Ast_ParserState*)ast;
  struct Ast_Name* name = (struct Ast_Name*)state->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PARSER_STATE);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (state->stmt_list) {
    struct ListLink* li = list_first_link(state->stmt_list);
    while (li) {
      struct Ast* stmt = li->object;
      build_symtable_statement(stmt);
      li = li->next;
    }
  }
  build_symtable_parser_transition(state->trans_stmt);
  pop_scope();
}

internal void
build_symtable_parser_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PARSER_DECL);
  struct Ast_ParserDecl* parser_decl = (struct Ast_ParserDecl*)ast;
  struct Ast_ParserProto* type_decl = (struct Ast_ParserProto*)parser_decl->type_decl;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PARSER_DECL);
  decl->id = name->id;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  push_scope();
  if (type_decl->type_params) {
    struct ListLink* li = list_first_link(type_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* li = list_first_link(type_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* li = list_first_link(parser_decl->ctor_params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  if (parser_decl->local_elements) {
    struct ListLink* li = list_first_link(parser_decl->local_elements);
    while (li) {
      struct Ast* element = li->object;
      build_symtable_local_parser_element(element);
      li = li->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* li = list_first_link(parser_decl->states);
    while (li) {
      struct Ast* state = li->object;
      build_symtable_parser_state(state);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* name = (struct Ast_Name*)ast;
    struct NameEntry* se = scope_lookup_name(get_current_scope(), name->strname);
    if (!se->ns_type) {
      struct NameDecl* decl = new_name_decl(struct NameDecl, AST_TYPE_PARAM);
      decl->id = name->id;
      decl->strname = name->strname;
      decl->line_no = name->line_no;
      declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
    } else {
      build_symtable_type_ref(ast);
    }
  } else {
    build_symtable_type_ref(ast);
  }
}

void
build_symtable_function_proto(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_PROTO);
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)ast;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct NameDecl* decl = new_name_decl(struct NameDecl, AST_FUNCTION_PROTO);
  decl->id = name->id;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  push_scope();
  if (function_proto->return_type) {
    build_symtable_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_extern_decl(struct Ast* ast)
{
  assert(ast->kind == AST_EXTERN_DECL);
  struct Ast_ExternDecl* extern_decl = (struct Ast_ExternDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)extern_decl->name;
  struct NameDecl* decl = new_name_decl(struct NameDecl, AST_EXTERN_DECL);
  decl->id = name->id;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  push_scope();
  if (extern_decl->type_params) {
    struct ListLink* li = list_first_link(extern_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* li = list_first_link(extern_decl->method_protos);
    while (li) {
      struct Ast* proto = li->object;
      build_symtable_function_proto(proto);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_struct_field(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_FIELD);
  struct Ast_StructField* field = (struct Ast_StructField*)ast;
  struct Ast_Name* name = (struct Ast_Name*)field->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_STRUCT_FIELD);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  build_symtable_type_ref(field->type);
}

internal void
build_symtable_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_DECL);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_STRUCT_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (struct_decl->fields) {
    struct ListLink* li = list_first_link(struct_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_symtable_struct_field(field);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_header_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_DECL);
  struct Ast_HeaderDecl* header_decl = (struct Ast_HeaderDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_HEADER_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (header_decl->fields) {
    struct ListLink* li = list_first_link(header_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_symtable_struct_field(field);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_header_union_decl(struct Ast* ast)
{
  assert(ast->kind == AST_HEADER_UNION_DECL);
  struct Ast_HeaderUnionDecl* header_union_decl = (struct Ast_HeaderUnionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)header_union_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_HEADER_UNION_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (header_union_decl->fields) {
    struct ListLink* li = list_first_link(header_union_decl->fields);
    while (li) {
      struct Ast* field = li->object;
      build_symtable_struct_field(field);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_type_ref(struct Ast* ast)
{
  if (ast->kind == AST_BASETYPE_BOOL || ast->kind == AST_BASETYPE_ERROR
      || ast->kind == AST_BASETYPE_INT || ast->kind == AST_BASETYPE_BIT
      || ast->kind == AST_BASETYPE_VARBIT || ast->kind == AST_BASETYPE_STRING
      || ast->kind == AST_BASETYPE_VOID) {
    build_symtable_expression(((struct Ast_BaseType*)ast)->name);
  } else if (ast->kind == AST_HEADER_STACK) {
    struct Ast_HeaderStack* type_ref = (struct Ast_HeaderStack*)ast;
    build_symtable_expression(type_ref->name);
    struct Ast* stack_expr = type_ref->stack_expr;
    build_symtable_expression(stack_expr);
  } else if (ast->kind == AST_NAME) {
    build_symtable_expression(ast);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* speclzd_type = (struct Ast_SpecializedType*)ast;
    build_symtable_expression(speclzd_type->name);
    struct ListLink* li = list_first_link(speclzd_type->type_args);
    while (li) {
      struct Ast* type_arg = li->object;
      build_symtable_type_ref(type_arg);
      li = li->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* li = list_first_link(type_ref->type_args);
      while (li) {
        struct Ast* type_arg = li->object;
        build_symtable_type_ref(type_arg);
        li = li->next;
      }
    }
  } else if (ast->kind == AST_STRUCT_DECL) {
    build_symtable_struct_decl(ast);
  } else if (ast->kind == AST_HEADER_DECL) {
    build_symtable_header_decl(ast);
  } else if (ast->kind == AST_HEADER_UNION_DECL) {
    build_symtable_header_union_decl(ast);
  } else if (ast->kind == AST_DONTCARE) {
    ; // pass
  }
  else assert(0);
}

internal void
build_symtable_enum_field(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* name = (struct Ast_Name*)ast;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_var) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_ENUM_FIELD);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_VAR, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
}

internal void
build_symtable_specified_id(struct Ast* ast)
{
  assert(ast->kind == AST_SPECIFIED_IDENT);
  struct Ast_SpecifiedIdent* id = (struct Ast_SpecifiedIdent*)ast;
  struct Ast_Name* name = (struct Ast_Name*)id->name;
  build_symtable_enum_field((struct Ast*)name);
  struct Ast* init_expr = id->init_expr;
  if (init_expr) {
    build_symtable_expression(init_expr);
  }
}

internal void
build_symtable_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM_DECL);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_ENUM_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (enum_decl->id_list) {
    struct ListLink* li = list_first_link(enum_decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_SPECIFIED_IDENT) {
        build_symtable_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_package_decl(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE_DECL);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PACKAGE_DECL);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  push_scope();
  if (package_decl->type_params) {
    struct ListLink* li = list_first_link(package_decl->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (package_decl->params) {
    struct ListLink* li = list_first_link(package_decl->params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_type_decl(struct Ast* ast)
{
  assert(ast->kind == AST_TYPE_DECL);
  struct Ast_TypeDecl* type_decl = (struct Ast_TypeDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)type_decl->name;
  struct NameEntry* ne = name_get_or_create_entry(&get_current_scope()->declarations, name->strname);
  if (!ne->ns_type) {
    struct NameDecl* decl = new_name_decl(struct NameDecl, 0);
    decl->id = name->id;
    decl->strname = name->strname;
    decl->kind = type_decl->is_typedef ? AST_TYPEDEF : AST_TYPE_DECL;
    decl->line_no = name->line_no;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  } else error("at line %d: name `%s` redeclared.", name->line_no, name->strname);
  struct Ast* type_ref = type_decl->type_ref;
  build_symtable_type_ref(type_ref);
}

internal void
build_symtable_function_decl(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTION_DECL);
  struct Ast_FunctionDecl* function_decl = (struct Ast_FunctionDecl*)ast;
  struct Ast_FunctionProto* function_proto = (struct Ast_FunctionProto*)function_decl->proto;
  struct Ast_Name* name = (struct Ast_Name*)function_proto->name;
  struct NameDecl* decl = new_name_decl(struct NameDecl, AST_FUNCTION_DECL);
  decl->id = name->id;
  decl->strname = name->strname;
  decl->line_no = name->line_no;
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
  push_scope();
  if (function_proto->return_type) {
    build_symtable_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* li = list_first_link(function_proto->type_params);
    while (li) {
      struct Ast* type_param = li->object;
      build_symtable_type_param(type_param);
      li = li->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* li = list_first_link(function_proto->params);
    while (li) {
      struct Ast* param = li->object;
      build_symtable_param(param);
      li = li->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* li = list_first_link(function_body->stmt_list);
      while (li) {
        struct Ast* stmt = li->object;
        build_symtable_statement(stmt);
        li = li->next;
      }
    }
  }
  pop_scope();
}

internal void
build_symtable_match_kind(struct Ast* ast)
{
  assert(ast->kind == AST_MATCH_KIND_DECL);
  struct Ast_MatchKindDecl* decl = (struct Ast_MatchKindDecl*)ast;
  assert(get_current_scope()->scope_level == 1);
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        build_symtable_enum_field(id);
      } else if (id->kind == AST_SPECIFIED_IDENT) {
        build_symtable_specified_id(id);
      }
      else assert(0);
      li = li->next;
    }
  }
}

internal void
build_symtable_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR_DECL);
  struct Ast_ErrorDecl* decl = (struct Ast_ErrorDecl*)ast;
  push_scope();
  if (decl->id_list) {
    struct ListLink* li = list_first_link(decl->id_list);
    while (li) {
      struct Ast* id = li->object;
      if (id->kind == AST_NAME) {
        build_symtable_enum_field(id);
      }
      else assert(0);
      li = li->next;
    }
  }
  pop_scope();
}

internal void
build_symtable_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  push_scope();
  struct ListLink* li = list_first_link(program->decl_list);
  while (li) {
    struct Ast* decl = li->object;
    if (decl->kind == AST_CONTROL_DECL) {
      build_symtable_control_decl(decl);
    } else if (decl->kind == AST_EXTERN_DECL) {
      build_symtable_extern_decl(decl);
    } else if (decl->kind == AST_STRUCT_DECL) {
      build_symtable_struct_decl(decl);
    } else if (decl->kind == AST_HEADER_DECL) {
      build_symtable_header_decl(decl);
    } else if (decl->kind == AST_HEADER_UNION_DECL) {
      build_symtable_header_union_decl(decl);
    } else if (decl->kind == AST_PACKAGE_DECL) {
      build_symtable_package_decl(decl);
    } else if (decl->kind == AST_PARSER_DECL) {
      build_symtable_parser_decl(decl);
    } else if (decl->kind == AST_INSTANTIATION) {
      build_symtable_instantiation(decl);
    } else if (decl->kind == AST_TYPE_DECL) {
      build_symtable_type_decl(decl);
    } else if (decl->kind == AST_FUNCTION_PROTO) {
      build_symtable_function_proto(decl);
    } else if (decl->kind == AST_CONST_DECL) {
      build_symtable_const_decl(decl);
    } else if (decl->kind == AST_ENUM_DECL) {
      build_symtable_enum_decl(decl);
    } else if (decl->kind == AST_FUNCTION_DECL) {
      build_symtable_function_decl(decl);
    } else if (decl->kind == AST_ACTION_DECL) {
      build_symtable_action_decl(decl);
    } else if (decl->kind == AST_MATCH_KIND_DECL) {
      build_symtable_match_kind(decl);
    } else if (decl->kind == AST_ERROR_DECL) {
      build_symtable_error_decl(decl);
    }
    else assert(0);
    li = li->next;
  }
  pop_scope();
}

void
build_symtable(struct Ast* p4program, struct Arena* symtable_storage)
{
  m_symtable_storage = symtable_storage;
  symtable_init(m_symtable_storage);

  int node_id = 1;

  /* Builtin Types */

  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_VOID);
    decl->id = node_id++;
    decl->strname = "void";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_BOOL);
    decl->id = node_id++;
    decl->strname = "bool";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_INT);
    decl->id = node_id++;
    decl->strname = "int";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_BIT);
    decl->id = node_id++;
    decl->strname = "bit";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_VARBIT);
    decl->id = node_id++;
    decl->strname = "varbit";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_BASETYPE_STRING);
    decl->id = node_id++;
    decl->strname = "string";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_ERROR_DECL);
    decl->id = node_id++;
    decl->strname = "error";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_ERROR_DECL);
    decl->id = node_id++;
    decl->strname = "error";
    declare_object_in_scope(get_root_scope(), NAMESPACE_VAR, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_MATCH_KIND_DECL);
    decl->id = node_id++;
    decl->strname = "match_kind";
    declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, decl);
  }

  /* Parser States */

  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PARSER_STATE);
    decl->id = node_id++;
    decl->strname = "accept";
    declare_object_in_scope(get_root_scope(), NAMESPACE_VAR, decl);
  }
  {
    struct NameDecl* decl = new_name_decl(struct NameDecl, AST_PARSER_STATE);
    decl->id = node_id++;
    decl->strname = "reject";
    declare_object_in_scope(get_root_scope(), NAMESPACE_VAR, decl);
  }

  build_symtable_p4program(p4program);
}
