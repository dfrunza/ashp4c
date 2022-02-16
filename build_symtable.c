#include "arena.h"
#include "ast.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* symtable_storage;
internal struct Hashmap nameref_table = {};


internal void build_symtable_block_statement(struct Ast* block_stmt);
internal void build_symtable_statement(struct Ast* decl);
internal void build_symtable_expression(struct Ast* expr);
internal void build_symtable_type_ref(struct Ast* type_ref);


#define new_object_descriptor(obj_type, obj_kind, obj_name) ({ \
  obj_type* descriptor = arena_push(symtable_storage, sizeof(obj_type)); \
  memset(descriptor, 0, sizeof(obj_type)); \
  descriptor->strname = obj_name; \
  descriptor->kind = obj_kind; \
  descriptor; \
})

internal void
build_symtable_function_call(struct Ast* ast)
{
  assert(ast->kind == AST_FUNCTIONCALL_EXPR);
  struct Ast_FunctionCallExpr* expr = (struct Ast_FunctionCallExpr*)ast;
  build_symtable_expression(expr->callee_expr);
  struct Ast_Expression* callee_expr = (struct Ast_Expression*)(expr->callee_expr);
  if (callee_expr->type_args) {
    struct ListLink* link = list_first_link(callee_expr->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      build_symtable_type_ref(type_arg);
      link = link->next;
    }
  }
  if (expr->args) {
    struct ListLink* link = list_first_link(expr->args);
    while (link) {
      struct Ast* arg = link->object;
      build_symtable_expression(arg);
      link = link->next;
    }
  }
}

internal void
build_symtable_method_call(struct Ast* ast)
{
  assert(ast->kind == AST_METHODCALL_STMT);
  struct Ast_MethodCallStmt* stmt = (struct Ast_MethodCallStmt*)ast;
  build_symtable_expression(stmt->lvalue);
  if (stmt->type_args) {
    struct ListLink* link = list_first_link(stmt->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      build_symtable_type_ref(type_arg);
      link = link->next;
    }
  }
  if (stmt->args) {
    struct ListLink* link = list_first_link(stmt->args);
    while (link) {
      struct Ast* arg = link->object;
      build_symtable_expression(arg);
      link = link->next;
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
    struct Object_NameRef* descriptor = new_object_descriptor(struct Object_NameRef, OBJECT_NAMEREF, name->strname);
    descriptor->name = name;
    descriptor->scope = get_current_scope();
    struct HashmapEntry* entry = hashmap_get_or_create_entry(&nameref_table, (struct HashmapKey){ .i_key=name->id });
    entry->object = descriptor;
  } else if (ast->kind == AST_LVALUE) {
    struct Ast_Lvalue* expr = (struct Ast_Lvalue*)ast;
    build_symtable_expression(expr->name);
    if (expr->expr) {
      struct ListLink* link = list_first_link(expr->expr);
      while (link) {
        struct Ast* lvalue_expr = link->object;
        build_symtable_expression(lvalue_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_FUNCTIONCALL_EXPR) {
    build_symtable_function_call(ast);
  } else if (ast->kind == AST_MEMBERSELECT_EXPR) {
    struct Ast_MemberSelectExpr* expr = (struct Ast_MemberSelectExpr*)ast;
    build_symtable_expression(expr->expr);
    build_symtable_expression(expr->member_name);
  } else if (ast->kind == AST_SPECIALIZED_TYPE) {
    struct Ast_SpecializedType* expr = (struct Ast_SpecializedType*)ast;
    build_symtable_expression(expr->name);
  } else if (ast->kind == AST_EXPRLIST_EXPR) {
    struct Ast_ExprListExpr* expr = (struct Ast_ExprListExpr*)ast;
    if (expr->expr_list) {
      struct ListLink* link = list_first_link(expr->expr_list);
      while (link) {
        struct Ast* expr_expr = link->object;
        build_symtable_expression(expr_expr);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_CAST_EXPR) {
    struct Ast_CastExpr* expr = (struct Ast_CastExpr*)ast;
    build_symtable_type_ref(expr->to_type);
    build_symtable_expression(expr->expr);
  } else if (ast->kind == AST_INDEXEDARRAY_EXPR) {
    struct Ast_IndexedArrayExpr* expr = (struct Ast_IndexedArrayExpr*)ast;
    build_symtable_expression(expr->index);
    if (expr->colon_index) {
      build_symtable_expression(expr->colon_index);
    }
  } else if (ast->kind == AST_KEYVALUE_PAIR_EXPR) {
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_PARAM, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  build_symtable_type_ref(param->type);
}

internal void
build_symtable_type_param(struct Ast* ast)
{
  assert(ast->kind == AST_NAME);
  struct Ast_Name* type_param = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), type_param->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_TYPE_PARAM, type_param->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, type_param->line_nr);
  } else {
    build_symtable_type_ref(ast);
  }
}

internal void
build_symtable_action_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ACTION_DECL);
  struct Ast_ActionDecl* action_decl = (struct Ast_ActionDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)action_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_ACTION, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  struct List* params = action_decl->params;
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* action_body = (struct Ast_BlockStmt*)action_decl->stmt;
  if (action_body) {
    struct List* stmt_list = action_body->stmt_list;
    if (stmt_list) {
      struct ListLink* link = list_first_link(stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        build_symtable_statement(stmt);
        link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_INSTANTIATION, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  build_symtable_type_ref(decl->type_ref);
  if (decl->args) {
    struct ListLink* link = list_first_link(decl->args);
    while (link) {
      struct Ast* arg = link->object;
      build_symtable_expression(arg);
      link = link->next;
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
    struct ListLink* link = list_first_link(action->args);
    while (link) {
      struct Ast* arg = link->object;
      build_symtable_expression(arg);
      link = link->next;
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
    struct ListLink* link = list_first_link(keyset->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      build_symtable_keyset_expr(expr);
      link = link->next;
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
      struct ListLink* link = list_first_link(prop->action_list);
      while (link) {
        struct Ast* action = link->object;
        build_symtable_action_ref(action);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_TABLE_SINGLE_ENTRY) {
    struct Ast_TableSingleEntry* prop = (struct Ast_TableSingleEntry*)ast;
    if (prop->init_expr) {
      build_symtable_expression(prop->init_expr);
    }
  } else if (ast->kind == AST_TABLE_KEY) {
    struct Ast_TableKey* prop = (struct Ast_TableKey*)ast;
    struct ListLink* link = list_first_link(prop->keyelem_list);
    while (link) {
      struct Ast* keyelem = link->object;
      build_symtable_table_keyelem(keyelem);
      link = link->next;
    }
  } else if (ast->kind == AST_TABLE_ENTRIES) {
    struct Ast_TableEntries* prop = (struct Ast_TableEntries*)ast;
    struct ListLink* link = list_first_link(prop->entries);
    while (link) {
      struct Ast* entry = link->object;
      build_symtable_table_entry(entry);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_TABLE, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  if (decl->prop_list) {
    struct ListLink* link = list_first_link(decl->prop_list);
    while (link) {
      struct Ast* prop = link->object;
      build_symtable_table_property(prop);
      link = link->next;
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

void
build_symtable_const_decl(struct Ast* ast)
{
  assert(ast->kind == AST_CONST_DECL);
  struct Ast_ConstDecl* decl = (struct Ast_ConstDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_CONST, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  build_symtable_type_ref(decl->type_ref);
  build_symtable_expression(decl->expr);
}

internal void
build_symtable_statement(struct Ast* ast)
{
  if (ast->kind == AST_VAR_DECL) {
    struct Ast_VarDecl* decl = (struct Ast_VarDecl*)ast;
    struct Ast_Name* name = (struct Ast_Name*)decl->name;
    struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
    if (!entry->ns_general) {
      struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_VAR, name->strname);
      declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
    } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
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
      struct ListLink* link = list_first_link(stmt->switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        build_symtable_switch_case(switch_case);
        link = link->next;
      }
    }
  } else if (ast->kind == AST_ASSIGNMENT_STMT) {
    struct Ast_AssignmentStmt* stmt = (struct Ast_AssignmentStmt*)ast;
    build_symtable_expression(stmt->lvalue);
    struct Ast* assign_expr = stmt->expr;
    build_symtable_expression(assign_expr);
  } else if (ast->kind == AST_METHODCALL_STMT) {
    build_symtable_method_call(ast);
  } else if (ast->kind == AST_DIRECT_APPLICATION) {
    struct Ast_DirectApplication* stmt = (struct Ast_DirectApplication*)ast;
    build_symtable_expression(stmt->name);
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
    struct ListLink* link = list_first_link(block_stmt->stmt_list);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_CONTROL_PROTO, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (control_decl->ctor_params) {
    struct ListLink* link = list_first_link(control_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (control_decl->local_decls) {
    struct ListLink* link = list_first_link(control_decl->local_decls);
    while (link) {
      struct Ast* decl = link->object;
      build_symtable_statement(decl);
      link = link->next;
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
    struct ListLink* link = list_first_link(trans_stmt->expr_list);
    while (link) {
      struct Ast* expr = link->object;
      build_symtable_expression(expr);
      link = link->next;
    }
    link = list_first_link(trans_stmt->case_list);
    while (link) {
      struct Ast* select_case = link->object;
      build_symtable_transition_select_case(select_case);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_PARSER_STATE, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (state->stmt_list) {
    struct ListLink* link = list_first_link(state->stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      build_symtable_statement(stmt);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_PARSER_PROTO, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (type_decl->type_params) {
    struct ListLink* link = list_first_link(type_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (type_decl->params) {
    struct ListLink* link = list_first_link(type_decl->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (parser_decl->ctor_params) {
    struct ListLink* link = list_first_link(parser_decl->ctor_params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  if (parser_decl->local_elements) {
    struct ListLink* link = list_first_link(parser_decl->local_elements);
    while (link) {
      struct Ast* element = link->object;
      build_symtable_local_parser_element(element);
      link = link->next;
    }
  }
  if (parser_decl->states) {
    struct ListLink* link = list_first_link(parser_decl->states);
    while (link) {
      struct Ast* state = link->object;
      build_symtable_parser_state(state);
      link = link->next;
    }
  }
  pop_scope();
}

void
build_symtable_function_return_type(struct Ast* ast)
{
  if (ast->kind == AST_NAME) {
    struct Ast_Name* return_type = (struct Ast_Name*)ast;
    struct SymtableEntry* entry = scope_resolve_name(get_current_scope(), return_type->strname);
    if (!entry->ns_type) {
      build_symtable_type_param(ast);
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
  struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_FUNCTION_PROTO, name->strname);
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  push_scope();
  if (function_proto->return_type) {
    build_symtable_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* link = list_first_link(function_proto->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_EXTERN, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (extern_decl->type_params) {
    struct ListLink* link = list_first_link(extern_decl->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (extern_decl->method_protos) {
    struct ListLink* link = list_first_link(extern_decl->method_protos);
    while (link) {
      struct Ast* proto = link->object;
      build_symtable_function_proto(proto);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_STRUCT_FIELD, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  build_symtable_type_ref(field->type);
}

internal void
build_symtable_struct_decl(struct Ast* ast)
{
  assert(ast->kind == AST_STRUCT_DECL);
  struct Ast_StructDecl* struct_decl = (struct Ast_StructDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)struct_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_STRUCT, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (struct_decl->fields) {
    struct ListLink* link = list_first_link(struct_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(field);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_HEADER, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (header_decl->fields) {
    struct ListLink* link = list_first_link(header_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(field);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_HEADER_UNION, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (header_union_decl->fields) {
    struct ListLink* link = list_first_link(header_union_decl->fields);
    while (link) {
      struct Ast* field = link->object;
      build_symtable_struct_field(field);
      link = link->next;
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
    struct ListLink* link = list_first_link(speclzd_type->type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      build_symtable_type_ref(type_arg);
      link = link->next;
    }
  } else if (ast->kind == AST_TUPLE) {
    struct Ast_Tuple* type_ref = (struct Ast_Tuple*)ast;
    if (type_ref->type_args) {
      struct ListLink* link = list_first_link(type_ref->type_args);
      while (link) {
        struct Ast* type_arg = link->object;
        build_symtable_type_ref(type_arg);
        link = link->next;
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
  struct Ast_Name* field = (struct Ast_Name*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), field->strname);
  if (!entry->ns_general) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_ENUM_FIELD, field->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_GENERAL, descriptor, field->line_nr);
  } else error("at line %d: name `%s` redeclared.", field->line_nr, field->strname);
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
build_symtable_enum_id_list(struct List* id_list)
{
  struct ListLink* link = list_first_link(id_list);
  while (link) {
    struct Ast* id = link->object;
    if (id->kind == AST_NAME) {
      build_symtable_enum_field(id);
    } else if (id->kind == AST_SPECIFIED_IDENT) {
      build_symtable_specified_id(id);
    }
    else assert(0);
    link = link->next;
  }
}

internal void
build_symtable_enum_decl(struct Ast* ast)
{
  assert(ast->kind == AST_ENUM_DECL);
  struct Ast_EnumDecl* enum_decl = (struct Ast_EnumDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)enum_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_ENUM, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (enum_decl->id_list) {
    build_symtable_enum_id_list(enum_decl->id_list);
  }
  pop_scope();
}

internal void
build_symtable_package(struct Ast* ast)
{
  assert(ast->kind == AST_PACKAGE_DECL);
  struct Ast_PackageDecl* package_decl = (struct Ast_PackageDecl*)ast;
  struct Ast_Name* name = (struct Ast_Name*)package_decl->name;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_PACKAGE, name->strname);
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
  push_scope();
  if (package_decl->params) {
    struct ListLink* link = list_first_link(package_decl->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_current_scope(), name->strname);
  if (!entry->ns_type) {
    struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_NONE, name->strname);
    descriptor->kind = type_decl->is_typedef ? OBJECT_TYPEDEF : OBJECT_TYPE;
    declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  } else error("at line %d: name `%s` redeclared.", name->line_nr, name->strname);
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
  struct NamedObject* descriptor = new_object_descriptor(struct NamedObject, OBJECT_FUNCTION, name->strname);
  declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, name->line_nr);
  push_scope();
  if (function_proto->return_type) {
    build_symtable_function_return_type(function_proto->return_type);
  }
  if (function_proto->type_params) {
    struct ListLink* link = list_first_link(function_proto->type_params);
    while (link) {
      struct Ast* type_param = link->object;
      build_symtable_type_param(type_param);
      link = link->next;
    }
  }
  if (function_proto->params) {
    struct ListLink* link = list_first_link(function_proto->params);
    while (link) {
      struct Ast* param = link->object;
      build_symtable_param(param);
      link = link->next;
    }
  }
  struct Ast_BlockStmt* function_body = (struct Ast_BlockStmt*)function_decl->stmt;
  if (function_body) {
    if (function_body->stmt_list) {
      struct ListLink* link = list_first_link(function_body->stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        build_symtable_statement(stmt);
        link = link->next;
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
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_root_scope(), "match_kind");
  assert(entry->ns_type);
  push_scope();
  if (decl->id_list) {
    build_symtable_enum_id_list(decl->id_list);
  }
  pop_scope();
}

internal void
build_symtable_error_decl(struct Ast* ast)
{
  assert (ast->kind == AST_ERROR_DECL);
  struct Ast_ErrorDecl* decl = (struct Ast_ErrorDecl*)ast;
  struct SymtableEntry* entry = symtable_get_or_create_entry(get_root_scope(), "error");
  assert(entry->ns_type);
  push_scope();
  if (decl->id_list) {
    build_symtable_enum_id_list(decl->id_list);
  }
  pop_scope();
}

internal void
build_symtable_p4program(struct Ast* ast)
{
  assert(ast->kind == AST_P4PROGRAM);
  struct Ast_P4Program* program = (struct Ast_P4Program*)ast;
  push_scope();
  struct ListLink* link = list_first_link(program->decl_list);
  while (link) {
    struct Ast* decl = link->object;
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
      build_symtable_package(decl);
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
    link = link->next;
  }
  pop_scope();
}

struct Hashmap*
build_symtable(struct Ast* p4program, struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;

  symtable_begin_build(symtable_storage);
  hashmap_init(&nameref_table, HASHMAP_KEY_INT, sizeof(struct Object_NameRef**), symtable_storage);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_VOID, "void"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_BOOL, "bool"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_INT, "int"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_BIT, "bit"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_VARBIT, "varbit"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_STRING, "string"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_ERROR, "error"), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_TYPE, new_object_descriptor(struct NamedObject, OBJECT_MATCH_KIND, "match_kind"), 0);
  build_symtable_p4program(p4program);
  symtable_end_build();
  return &nameref_table;
}
