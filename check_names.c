#include "arena.h"
#include "ast.h"
#include "symtable.h"


#define DEBUG_ENABLED 1


internal void check_names_expression(struct Scope* scope, struct Ast* expr);
internal void check_names_type_ref(struct Scope* scope, struct Ast* type_ref);
  

internal void
check_names_expression(struct Scope* scope, struct Ast* expr)
{
  if (expr->kind == Ast_BinaryExpr) {
    struct Ast* left_operand = ast_getattr(expr, "left_operand");
    check_names_expression(scope, left_operand);
    struct Ast* right_operand = ast_getattr(expr, "right_operand");
    check_names_expression(scope, right_operand);
  } else if (expr->kind == Ast_Name) {
    char* strname = ast_getattr(expr, "name");
    struct SymtableEntry* entry = scope_resolve_name(scope, strname);
    if (!entry) {
      error("at line %d: unknown identifier `%s`.", expr->line_nr, strname);
    } else if (DEBUG_ENABLED) {
      printf("at line %d: identifier `%s` has been resolved.\n", expr->line_nr, strname);
    }
  } else if (expr->kind == Ast_Lvalue) {
    struct Ast* name = ast_getattr(expr, "name");
    check_names_expression(scope, name);
    struct Ast* lvalue_expr = ast_getattr(expr, "expr");
    if (lvalue_expr) {
      assert(!"TODO");
    }
  } else if (expr->kind == Ast_FunctionCallExpr) {
    struct Ast* call_expr = ast_getattr(expr, "expr");
    check_names_expression(scope, call_expr);
    struct List* args = ast_getattr(expr, "args");
    if (args) {
      struct ListLink* link = list_first_link(args);
      while (link) {
        struct Ast* arg = link->object;
        check_names_expression(scope, arg);
        link = link->next;
      }
    }
  } else if (expr->kind == Ast_Int || expr->kind == Ast_Bool || expr->kind == Ast_StringLiteral) {
    ; // pass
  }
  else assert(!"TODO");
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
    if (stmt->scope) {
      scope = stmt->scope;
    }
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
    struct Ast* lvalue = ast_getattr(stmt, "lvalue");
    check_names_expression(scope, lvalue);
    struct List* args = ast_getattr(stmt, "args");
    if (args) {
      struct ListLink* link = list_first_link(args);
      while (link) {
        struct Ast* arg = link->object;
        check_names_expression(scope, arg);
        link = link->next;
      }
    }
  } else if (stmt->kind == Ast_DirectApplication) {
    struct Ast* name = ast_getattr(stmt, "name");
    check_names_expression(scope, name);
  } else if (stmt->kind == Ast_ReturnStmt) {
    struct Ast* return_expr = ast_getattr(stmt, "expr");
    if (return_expr) {
      check_names_expression(scope, return_expr);
    }
  }
  else assert(!"TODO");
}

internal void
check_names_type_ref(struct Scope* scope, struct Ast* type_ref)
{
  assert(type_ref->kind == Ast_Name || type_ref->kind == Ast_BaseType);
  struct Ast* name = type_ref;
  if (type_ref->kind == Ast_BaseType) {
    name = ast_getattr(type_ref, "type_name");
  }
  check_names_expression(scope, name);
}

internal void
check_names_function_param(struct Scope* scope, struct Ast* param)
{
  assert(param->kind == Ast_Parameter);
  struct Ast* type_name = ast_getattr(param, "type");
  check_names_type_ref(scope, type_name);
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
}

internal void
check_names_control_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ControlDecl);
  // TODO: Check params.
  struct Ast* apply_stmt = ast_getattr(decl, "apply_stmt");
  if (apply_stmt) {
    struct List* stmt_list = ast_getattr(apply_stmt, "stmt_list");
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(apply_stmt->scope, stmt);
      link = link->next;
    }
  }
}

internal void
check_names_const_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ConstDecl);
  struct Ast* type_ref = ast_getattr(decl, "type_ref");
  check_names_type_ref(scope, type_ref);
}

internal void
check_names_function_proto(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_FunctionProto);
  struct Ast* return_type = ast_getattr(decl, "return_type");
  check_names_type_ref(scope, return_type);
  struct List* params = ast_getattr(decl, "params");
  struct ListLink* link = list_first_link(params);
  while (link) {
    struct Ast* param = link->object;
    check_names_function_param(decl->scope, param);
    link = link->next;
  }
}

internal void
check_names_package(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_PackageDecl);
  struct List* params = ast_getattr(decl, "params");
  struct ListLink* link = list_first_link(params);
  while (link) {
    struct Ast* param = link->object;
    check_names_function_param(scope, param);
    link = link->next;
  }
}

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
check_names_parser_decl(struct Scope* scope, struct Ast* decl)
{
  assert(decl->kind == Ast_ParserDecl);
  struct List* states = ast_getattr(decl, "states");
  struct ListLink* link = list_first_link(states);
  while (link) {
    struct Ast* state = link->object;
    check_names_parser_state(state->scope, state);
    link = link->next;
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
    struct Ast* stmt = link->object;
    check_names_statement(decl->scope, stmt);
    link = link->next;
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
    }
    else assert(!"TODO");
    link = link->next;
  }
}