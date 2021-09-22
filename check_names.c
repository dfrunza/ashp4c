#include "arena.h"
#include "ast.h"
#include "symtable.h"


#define DEBUG_ENABLED 1


internal void check_names_expression(struct Scope* scope, struct Ast* expr);
internal void check_names_type_ref(struct Scope* scope, struct Ast* type_ref);
internal void check_names_statement(struct Scope* scope, struct Ast* stmt);
  

internal void
check_names_instantiation(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_Instantiation);
  struct Ast* type_ref = ast_getattr(decl, "type_ref");
  check_names_type_ref(scope, type_ref);
  struct List* args = ast_getattr(decl, "args");
  if (args) {
    struct ListLink* link = list_first_link(args);
    while (link) {
      struct Ast* arg = link->object;
      check_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
check_names_function_call(struct Scope* scope, struct Ast* expr)
{
  assert(expr->kind == Ast_FunctionCallExpr);
  struct Ast* call_expr = ast_getattr(expr, "expr");
  check_names_expression(scope, call_expr);
  struct List* type_args = ast_getattr(call_expr, "type_args");
  if (type_args) {
    struct ListLink* link = list_first_link(type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      check_names_type_ref(scope, type_arg);
      link = link->next;
    }
  }
  struct List* args = ast_getattr(expr, "args");
  if (args) {
    struct ListLink* link = list_first_link(args);
    while (link) {
      struct Ast* arg = link->object;
      check_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
check_names_expression(struct Scope* scope, struct Ast* expr)
{
  if (expr->kind == Ast_BinaryExpr) {
    struct Ast* left_operand = ast_getattr(expr, "left_operand");
    check_names_expression(scope, left_operand);
    struct Ast* right_operand = ast_getattr(expr, "right_operand");
    check_names_expression(scope, right_operand);
  } else if (expr->kind == Ast_UnaryExpr) {
    struct Ast* operand = ast_getattr(expr, "operand");
    check_names_expression(scope, operand);
  } else if (expr->kind == Ast_Name) {
    char* strname = ast_getattr(expr, "name");
    struct SymtableEntry* entry = scope_resolve_name(scope, strname);
    if (!(entry->id_kw || entry->id_type || entry->id_ident)) {
      error("at line %d: unknown identifier `%s`.", expr->line_nr, strname);
    } else if (DEBUG_ENABLED) {
      printf("at line %d: identifier `%s` has been resolved.\n", expr->line_nr, strname);
    }
  } else if (expr->kind == Ast_Lvalue) {
    struct Ast* name = ast_getattr(expr, "name");
    check_names_expression(scope, name);
    struct List* lvalue_expr_list = ast_getattr(expr, "expr");
    if (lvalue_expr_list) {
      struct ListLink* link = list_first_link(lvalue_expr_list);
      while (link) {
        struct Ast* lvalue_expr = link->object;
        if (lvalue_expr->kind == Ast_Name) {
          bool* is_dotprefixed = ast_getattr(lvalue_expr, "is_dotprefixed");
          if (is_dotprefixed && *is_dotprefixed) {
            link = link->next;
            continue; // Member selection is checked in a later pass.
          }
        }
        check_names_expression(scope, lvalue_expr);
        link = link->next;
      }
    }
  } else if (expr->kind == Ast_FunctionCallExpr) {
    check_names_function_call(scope, expr);
  } else if (expr->kind == Ast_MemberSelectExpr) {
    struct Ast* struct_expr = ast_getattr(expr, "expr");
    if (struct_expr->kind == Ast_MemberSelectExpr) {
      ; // Member selection is checked in a later pass.
    } else {
      check_names_expression(scope, struct_expr);
    }
    // 'member_name' is checked in a later pass.
  } else if (expr->kind == Ast_SpecializedType) {
    struct Ast* name = ast_getattr(expr, "name");
    check_names_expression(scope, name);
  } else if (expr->kind == Ast_ExpressionListExpr) {
    struct List* expr_list = ast_getattr(expr, "expr_list");
    if (expr_list) {
      struct ListLink* link = list_first_link(expr_list);
      while (link) {
        struct Ast* expr_expr = link->object;
        check_names_expression(scope, expr_expr);
        link = link->next;
      }
    }
  } else if (expr->kind == Ast_CastExpr) {
    struct Ast* to_type = ast_getattr(expr, "to_type");
    check_names_type_ref(scope, to_type);
    struct Ast* cast_expr = ast_getattr(expr, "expr");
    check_names_expression(scope, cast_expr);
  } else if (expr->kind == Ast_IndexedArrayExpr) {
    struct Ast* array_index = ast_getattr(expr, "index");
    check_names_expression(scope, array_index);
    struct Ast* colon_index = ast_getattr(expr, "colon_index");
    if (colon_index) {
      check_names_expression(scope, colon_index);
    }
  } else if (expr->kind == Ast_KvPair) {
    struct Ast* kv_expr = ast_getattr(expr, "expr");
    check_names_expression(scope, kv_expr);
  } else if (expr->kind == Ast_Int || expr->kind == Ast_Bool || expr->kind == Ast_StringLiteral) {
    ; // pass
  }
  else assert(!"TODO");
}

internal void
check_names_function_param(struct Scope* scope, struct Ast* param)
{
  assert(param->kind == Ast_Parameter);
  struct Ast* type_name = ast_getattr(param, "type");
  check_names_type_ref(scope, type_name);
}

internal void
check_names_action_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ActionDecl);
  struct List* params = ast_getattr(decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(scope, param);
      link = link->next;
    }
  }
  struct Ast* action_body = ast_getattr(decl, "stmt");
  struct List* stmt_list = ast_getattr(action_body, "stmt_list");
  if (stmt_list) {
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
}

internal void
check_names_keyset_expr(struct Scope* scope, struct Ast* expr)
{
  if (expr->kind == Ast_Default || expr->kind == Ast_Dontcare) {
    ; // pass
  } else {
    check_names_expression(scope, expr);
  }
}

internal void
check_names_select_keyset(struct Scope* scope, struct Ast* keyset)
{
  if (keyset->kind == Ast_TupleKeyset) {
    struct List* expr_list = ast_getattr(keyset, "expr_list");
    struct ListLink* link = list_first_link(expr_list);
    while (link) {
      struct Ast* expr = link->object;
      check_names_keyset_expr(scope, expr);
      link = link->next;
    }
  } else {
    check_names_keyset_expr(scope, keyset);
  }
}

internal void
check_names_action_ref(struct Scope* scope, struct Ast* action)
{
  assert(action->kind == Ast_ActionRef);
  struct Ast* name = ast_getattr(action, "name");
  check_names_expression(scope, name);
  struct List* args = ast_getattr(action, "args");
  if (args) {
    struct ListLink* link = list_first_link(args);
    while (link) {
      struct Ast* arg = link->object;
      check_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
check_names_table_keyelem(struct Scope* scope, struct Ast* keyelem)
{
  assert(keyelem->kind == Ast_KeyElement);
  struct Ast* expr = ast_getattr(keyelem, "expr");
  check_names_expression(scope, expr);
  struct Ast* name = ast_getattr(keyelem, "name");
  check_names_expression(scope, name);
}

internal void
check_names_table_entry(struct Scope* scope, struct Ast* entry)
{
  assert(entry->kind == Ast_TableEntry);
  struct Ast* keyset = ast_getattr(entry, "keyset");
  check_names_select_keyset(scope, keyset);
  struct Ast* action = ast_getattr(entry, "action");
  check_names_action_ref(scope, action);
}

internal void
check_names_table_property(struct Scope* scope, struct Ast* prop)
{
  if (prop->kind == Ast_TableProp_Actions) {
    struct List* action_list = ast_getattr(prop, "action_list");
    if (action_list) {
      struct ListLink* link = list_first_link(action_list);
      while (link) {
        struct Ast* action = link->object;
        check_names_action_ref(scope, action);
        link = link->next;
      }
    }
  } else if (prop->kind == Ast_TableProp_SingleEntry) {
    struct Ast* init_expr = ast_getattr(prop, "init_expr");
    if (init_expr) {
      check_names_expression(scope, init_expr);
    }
  } else if (prop->kind == Ast_TableProp_Key) {
    struct List* keyelem_list = ast_getattr(prop, "keyelem_list");
    struct ListLink* link = list_first_link(keyelem_list);
    while (link) {
      struct Ast* keyelem = link->object;
      check_names_table_keyelem(scope, keyelem);
      link = link->next;
    }
  } else if (prop->kind == Ast_TableProp_Entries) {
    struct List* entries = ast_getattr(prop, "entries");
    struct ListLink* link = list_first_link(entries);
    while (link) {
      struct Ast* entry = link->object;
      check_names_table_entry(scope, entry);
      link = link->next;
    }
  }
  else assert(!"TODO");
}

internal void
check_names_table_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_TableDecl);
  struct List* prop_list = ast_getattr(decl, "prop_list");
  if (prop_list) {
    struct ListLink* link = list_first_link(prop_list);
    while (link) {
      struct Ast* prop = link->object;
      check_names_table_property(scope, prop);
      link = link->next;
    }
  }
}

internal void
check_names_method_call(struct Scope* scope, struct Ast* stmt)
{
  struct Ast* lvalue = ast_getattr(stmt, "lvalue");
  check_names_expression(scope, lvalue);
  struct List* type_args = ast_getattr(stmt, "type_args");
  if (type_args) {
    struct ListLink* link = list_first_link(type_args);
    while (link) {
      struct Ast* type_arg = link->object;
      check_names_type_ref(scope, type_arg);
      link = link->next;
    }
  }
  struct List* args = ast_getattr(stmt, "args");
  if (args) {
    struct ListLink* link = list_first_link(args);
    while (link) {
      struct Ast* arg = link->object;
      check_names_expression(scope, arg);
      link = link->next;
    }
  }
}

internal void
check_names_switch_case(struct Scope* scope, struct Ast* switch_case)
{
  assert(switch_case->kind == Ast_SwitchCase);
  struct Ast* case_stmt = ast_getattr(switch_case, "stmt");
  if (case_stmt) {
    check_names_statement(scope, case_stmt);
  }
}

internal void
check_names_statement(struct Scope* scope, struct Ast* stmt)
{
  if (stmt->kind == Ast_IfStmt) {
    struct Ast* cond_expr = ast_getattr(stmt, "cond_expr");
    check_names_expression(scope, cond_expr);
    struct Ast* if_stmt = ast_getattr(stmt, "stmt");
    check_names_statement(scope, if_stmt);
    struct Ast* else_stmt = ast_getattr(stmt, "else_stmt");
    if (else_stmt) {
      check_names_statement(scope, else_stmt);
    }
  } else if (stmt->kind == Ast_BlockStmt) {
    struct List* stmt_list = ast_getattr(stmt, "stmt_list");
    if (stmt_list) {
      struct ListLink* link = list_first_link(stmt_list);
      while (link) {
        struct Ast* block_stmt = link->object;
        check_names_statement(scope, block_stmt);
        link = link->next;
      }
    }
  } else if (stmt->kind == Ast_AssignmentStmt) {
    struct Ast* lvalue = ast_getattr(stmt, "lvalue");
    check_names_expression(scope, lvalue);
    struct Ast* assign_expr = ast_getattr(stmt, "expr");
    check_names_expression(scope, assign_expr);
  } else if (stmt->kind == Ast_MethodCallStmt) {
    check_names_method_call(scope, stmt);
  } else if (stmt->kind == Ast_DirectApplication) {
    struct Ast* name = ast_getattr(stmt, "name");
    check_names_expression(scope, name);
  } else if (stmt->kind == Ast_ReturnStmt) {
    struct Ast* return_expr = ast_getattr(stmt, "expr");
    if (return_expr) {
      check_names_expression(scope, return_expr);
    }
  } else if (stmt->kind == Ast_VarDecl) {
    struct Ast* var_type = ast_getattr(stmt, "type");
    check_names_type_ref(scope, var_type);
    struct Ast* init_expr = ast_getattr(stmt, "init_expr");
    if (init_expr) {
      check_names_expression(scope, init_expr);
    }
  } else if (stmt->kind == Ast_ActionDecl) {
    check_names_action_decl(scope, stmt);
  } else if (stmt->kind == Ast_Instantiation) {
    check_names_instantiation(scope, stmt);
  } else if (stmt->kind == Ast_TableDecl) {
    check_names_table_decl(scope, stmt);
  } else if (stmt->kind == Ast_SwitchStmt) {
    struct Ast* switch_expr = ast_getattr(stmt, "expr");
    check_names_expression(scope, switch_expr);
    struct List* switch_cases = ast_getattr(stmt, "switch_cases");
    if (switch_cases) {
      struct ListLink* link = list_first_link(switch_cases);
      while (link) {
        struct Ast* switch_case = link->object;
        check_names_switch_case(scope, switch_case);
        link = link->next;
      }
    }
  }
  else assert(!"TODO");
}

internal void
check_names_type_ref(struct Scope* scope, struct Ast* type_ref)
{
  if (type_ref->kind == Ast_BaseType) {
    struct Ast* name = ast_getattr(type_ref, "type_name");
    check_names_expression(scope, name);
  } else if (type_ref->kind == Ast_HeaderStack) {
    struct Ast* name = ast_getattr(type_ref, "name");
    check_names_expression(scope, name);
    struct Ast* stack_expr = ast_getattr(type_ref, "stack_expr");
    check_names_expression(scope, stack_expr);
  } else if (type_ref->kind == Ast_Name || type_ref->kind == Ast_SpecializedType) {
    check_names_expression(scope, type_ref);
  } else if (type_ref->kind == Ast_Tuple) {
    struct List* type_args = ast_getattr(type_ref, "type_args");
    if (type_args) {
      struct ListLink* link = list_first_link(type_args);
      while (link) {
        struct Ast* type_arg = link->object;
        check_names_type_ref(scope, type_arg);
        link = link->next;
      }
    }
  } else if (type_ref->kind == Ast_StructDecl || type_ref->kind == Ast_HeaderDecl) {
    ; // pass
  }
  else assert(0);
}

internal void
check_names_transition_select_case(struct Scope* scope, struct Ast* select_case)
{
  assert(select_case->kind == Ast_SelectCase);
  struct Ast* keyset = ast_getattr(select_case, "keyset");
  check_names_select_keyset(scope, keyset);
  struct Ast* name = ast_getattr(select_case, "name");
  check_names_expression(scope, name);
}

internal void
check_names_parser_transition(struct Scope* scope, struct Ast* trans_stmt)
{
  if (trans_stmt->kind == Ast_Name) {
    check_names_expression(scope, trans_stmt);
  } else if (trans_stmt->kind == Ast_SelectExpr) {
    struct List* expr_list = ast_getattr(trans_stmt, "expr_list");
    struct ListLink* link = list_first_link(expr_list);
    while (link) {
      struct Ast* expr = link->object;
      check_names_expression(scope, expr);
      link = link->next;
    }
    struct List* case_list = ast_getattr(trans_stmt, "case_list");
    link = list_first_link(case_list);
    while (link) {
      struct Ast* select_case = link->object;
      check_names_transition_select_case(scope, select_case);
      link = link->next;
    }
  }
  else assert(!"TODO");
}

internal void
check_names_parser_state(struct Scope* scope, struct Ast* state)
{
  assert(state->kind == Ast_ParserState);
  struct List* stmt_list = ast_getattr(state, "stmt_list");
  if (stmt_list) {
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(state->scope, stmt);
      link = link->next;
    }
  }
  struct Ast* trans_stmt = ast_getattr(state, "trans_stmt");
  check_names_parser_transition(state->scope, trans_stmt);
}

internal void
check_names_control_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ControlDecl);
  struct Ast* type_decl = ast_getattr(decl, "type_decl");
  struct List* params = ast_getattr(type_decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
  struct List* local_decls = ast_getattr(decl, "local_decls");
  if (local_decls) {
    struct ListLink* link = list_first_link(local_decls);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
  struct Ast* apply_stmt = ast_getattr(decl, "apply_stmt");
  if (apply_stmt) {
    struct List* stmt_list = ast_getattr(apply_stmt, "stmt_list");
    if (stmt_list) {
      struct ListLink* link = list_first_link(stmt_list);
      while (link) {
        struct Ast* stmt = link->object;
        check_names_statement(apply_stmt->scope, stmt);
        link = link->next;
      }
    }
  }
}

internal void
check_names_const_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ConstDecl);
  struct Ast* type_ref = ast_getattr(decl, "type_ref");
  check_names_type_ref(scope, type_ref);
  struct Ast* const_expr = ast_getattr(decl, "expr");
  check_names_expression(scope, const_expr);
}

internal void
check_names_function_proto(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_FunctionProto);
  struct Ast* return_type = ast_getattr(decl, "return_type");
  if (return_type) {
    check_names_type_ref(scope, return_type);
  }
  struct List* params = ast_getattr(decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
}

internal void
check_names_package(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_PackageDecl);
  struct List* params = ast_getattr(decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(scope, param);
      link = link->next;
    }
  }
}

internal void
check_names_parser_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ParserDecl);
  struct Ast* type_decl = ast_getattr(decl, "type_decl");
  struct List* params = ast_getattr(type_decl, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(decl->scope, param);
      link = link->next;
    }
  }
  struct List* states = ast_getattr(decl, "states");
  if (states) {
    struct ListLink* link = list_first_link(states);
    while (link) {
      struct Ast* state = link->object;
      check_names_parser_state(state->scope, state);
      link = link->next;
    }
  }
}

internal void
check_names_function_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_FunctionDecl);
  struct Ast* function_proto = ast_getattr(decl, "proto");
  struct List* params = ast_getattr(function_proto, "params");
  if (params) {
    struct ListLink* link = list_first_link(params);
    while (link) {
      struct Ast* param = link->object;
      check_names_function_param(scope, param);
      link = link->next;
    }
  }
  struct Ast* function_body = ast_getattr(decl, "stmt");
  struct List* stmt_list = ast_getattr(function_body, "stmt_list");
  if (stmt_list) {
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(decl->scope, stmt);
      link = link->next;
    }
  }
}

internal void
check_names_extern_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ExternDecl);
  struct List* method_protos = ast_getattr(decl, "method_protos");
  if (method_protos) {
    struct ListLink* link = list_first_link(method_protos);
    while (link) {
      struct Ast* method = link->object;
      check_names_function_proto(scope, method);
      link = link->next;
    }
  }
}

internal void
check_names_enum_specified_id(struct Scope* scope, struct Ast* id)
{
  assert(id->kind == Ast_SpecifiedIdent);
  struct Ast* init_expr = ast_getattr(id, "init_expr");
  if (init_expr) {
    check_names_expression(scope, init_expr);
  }
}

internal void
check_names_enum_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_EnumDecl);
  struct List* id_list = ast_getattr(decl, "id_list");
  if (id_list) {
    struct ListLink* link = list_first_link(id_list);
    while (link) {
      struct Ast* id = link->object;
      if (id->kind == Ast_SpecifiedIdent) {
        check_names_enum_specified_id(decl->scope, id);
      }
      link = link->next;
    }
  }
}

void
check_names_type_decl(struct Scope* scope, struct Ast* decl)
{
  assert (decl->kind == Ast_TypeDecl);
  struct Ast* type_ref = ast_getattr(decl, "type_ref");
  check_names_type_ref(scope, type_ref);
}

internal void
check_names_struct_decl(struct Scope* scope, struct Ast* decl)
{
  assert (decl->kind == Ast_StructDecl);
  struct List* fields = ast_getattr(decl, "fields");
  if (fields) {
    struct ListLink* link = list_first_link(fields);
    while (link) {
      struct Ast* field = link->object;
      struct Ast* field_type = ast_getattr(field, "type");
      check_names_type_ref(scope, field_type);
      link = link->next;
    }
  }
}

internal void
check_names_header_decl(struct Scope* scope, struct Ast* decl)
{
  assert (decl->kind == Ast_HeaderDecl);
  struct List* fields = ast_getattr(decl, "fields");
  if (fields) {
    struct ListLink* link = list_first_link(fields);
    while (link) {
      struct Ast* field = link->object;
      struct Ast* field_type = ast_getattr(field, "type");
      check_names_type_ref(scope, field_type);
      link = link->next;
    }
  }
}

internal void
check_names_header_union_decl(struct Scope* scope, struct Ast* decl)
{
  assert (decl->kind == Ast_HeaderUnionDecl);
  struct List* fields = ast_getattr(decl, "fields");
  if (fields) {
    struct ListLink* link = list_first_link(fields);
    while (link) {
      struct Ast* field = link->object;
      struct Ast* field_type = ast_getattr(field, "type");
      check_names_type_ref(scope, field_type);
      link = link->next;
    }
  }
}

void
check_names_program(struct Ast* program)
{
  assert(program->kind == Ast_P4Program);
  struct List* decl_list = ast_getattr(program, "decl_list");
  struct ListLink* link = list_first_link(decl_list);
  while (link) {
    struct Ast* decl = link->object;
    if (decl->kind == Ast_ControlDecl) {
      check_names_control_decl(program->scope, decl);
    } else if (decl->kind == Ast_ConstDecl) {
      check_names_const_decl(program->scope, decl);
    } else if (decl->kind == Ast_FunctionProto) {
      check_names_function_proto(program->scope, decl);
    } else if (decl->kind == Ast_PackageDecl) {
      check_names_package(program->scope, decl);
    } else if (decl->kind == Ast_Instantiation) {
      check_names_instantiation(program->scope, decl);
    } else if (decl->kind == Ast_ParserDecl) {
      check_names_parser_decl(program->scope, decl);
    } else if (decl->kind == Ast_FunctionDecl) {
      check_names_function_decl(program->scope, decl);
    } else if (decl->kind == Ast_ExternDecl) {
      check_names_extern_decl(program->scope, decl);
    } else if (decl->kind == Ast_ActionDecl) {
      check_names_action_decl(program->scope, decl);
    } else if (decl->kind == Ast_EnumDecl) {
      check_names_enum_decl(program->scope, decl);
    } else if (decl->kind == Ast_TypeDecl) {
      check_names_type_decl(program->scope, decl);
    } else if (decl->kind == Ast_StructDecl) {
      check_names_struct_decl(program->scope, decl);
    } else if (decl->kind == Ast_HeaderDecl) {
      check_names_header_decl(program->scope, decl);
    } else if (decl->kind == Ast_HeaderUnionDecl) {
      check_names_header_union_decl(program->scope, decl);
    } else if (decl->kind == Ast_MatchKind || decl->kind == Ast_Error) {
      ; // pass
    }
    else assert(!"TODO");
    link = link->next;
  }
}
