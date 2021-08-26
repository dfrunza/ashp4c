#include "arena.h"
#include "ast.h"
#include "symtable.h"


#define DEBUG_ENABLED 1


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
    if (!entry->id_ident) {
      error("at line %d: unknown name `%s`.");
    } else if (DEBUG_ENABLED) {
      printf("at line %d: name `%s` has been resolved.\n", expr->line_nr, strname);
    }
  } else if (expr->kind == Ast_Lvalue) {
    struct Ast* name = ast_getattr(expr, "name");
    check_names_expression(scope, name);
    struct Ast* lvalue_expr = ast_getattr(expr, "expr");
    if (lvalue_expr) {
      assert(!"TODO");
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
    struct Scope* stmt_scope = ast_getattr(stmt, "stmt_scope");
    if (stmt_scope) {
      scope = stmt_scope;
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
  }
  else assert(!"TODO");
}

internal void
check_names_control(struct Ast* control)
{
  struct Ast* apply_stmt = ast_getattr(control, "apply_stmt");
  if (apply_stmt) {
    struct Scope* stmt_scope = ast_getattr(apply_stmt, "stmt_scope");
    struct List* stmt_list = (struct List*)ast_getattr(apply_stmt, "stmt_list");
    struct ListLink* link = list_first_link(stmt_list);
    while (link) {
      struct Ast* stmt = link->object;
      check_names_statement(stmt_scope, stmt);
      link = link->next;
    }
  }
}

void
check_names_program(struct Ast* program)
{
  if (program->kind == Ast_P4Program) {
    struct List* decl_list = (struct List*)ast_getattr(program, "decl_list");
    struct ListLink* link = list_first_link(decl_list);
    while (link) {
      struct Ast* decl = link->object;
      if (decl->kind == Ast_Control) {
        check_names_control(decl);
      }
      link = link->next;
    }
  }
  else assert(0);
}
