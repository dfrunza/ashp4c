#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

internal void
visit_binary_expression(Ast_BinaryExpr* expr_ast)
{
  if (expr_ast->op == AST_OP_MEMBER_SELECTOR)
  {
    Ast_Expression* l_operand = expr_ast->l_operand;
    Ast_Expression* r_operand = expr_ast->r_operand;

    if (l_operand->kind == AST_IDENT_EXPR)
    {
      Ast_IdentExpr* l_ident_ast = (Ast_IdentExpr*)l_operand;
      int x = 0;
    }
  }
}

internal void
visit_expression(Ast_Expression* expr_ast)
{
  switch (expr_ast->kind)
  {
    case (AST_BINARY_EXPR):
    {
      visit_binary_expression((Ast_BinaryExpr*)expr_ast);
    } break;

    default: break;
  }
}

internal void
visit_parser_state(Ast_ParserState* state_ast)
{
  Ast_Expression* expr_ast = state_ast->first_statement;
  while (expr_ast)
  {
    visit_expression(expr_ast);
    expr_ast = expr_ast->next_expression;
  }
}

internal void
visit_parser_decl(Ast_ParserDecl* parser_ast)
{
  Ast_ParserState* state_ast = parser_ast->first_parser_state;
  while (state_ast)
  {
    visit_parser_state(state_ast);
    state_ast = state_ast->next_state;
  }
}

internal void
visit_p4declaration(Ast_Declaration* p4decl_ast)
{
  if (p4decl_ast->kind == AST_PARSER_DECL)
    visit_parser_decl((Ast_ParserDecl*)p4decl_ast);
}

internal void
visit_p4program(Ast_P4Program* program)
{
  Ast_Declaration* p4decl_ast = p4program->first_declaration;
  while (p4decl_ast)
  {
    visit_p4declaration(p4decl_ast);
    p4decl_ast = p4decl_ast->next_decl;
  }
}

void
resolve_member_ident()
{
  visit_p4program(p4program);
}

