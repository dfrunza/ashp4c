#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

internal bool type_equal(Typexpr* type_A, Typexpr* type_B)
{
  bool is_equal = false;

  if (type_A == type_B)
    return true;

  if (type_A->kind == TYP_BASIC && type_B->kind == TYP_BASIC)
  {
    Typexpr_Basic* basic_A = (Typexpr_Basic*)type_A;
    Typexpr_Basic* basic_B = (Typexpr_Basic*)type_B;
    is_equal = (basic_A->basic_type == basic_B->basic_type);
  }

  return is_equal;
}

internal void
visit_binary_expression(Ast_BinaryExpr* expr_ast)
{
  Ast* l_operand = expr_ast->l_operand;
  Ast* r_operand = expr_ast->r_operand;
  Typexpr* l_type = l_operand->typexpr;
  Typexpr* r_type = r_operand->typexpr;

  switch (expr_ast->op)
  {
    case (AST_OP_ASSIGN):
    {
      if (type_equal(l_type, r_type))
        ; // ok
      else
        error("at line %d: incompatible types in assignment expression", expr_ast->line_nr);
    } break;

    default: ;
  }
}

internal void
visit_expression(Ast* expr_ast)
{
  switch (expr_ast->kind)
  {
    case (AST_BINARY_EXPR):
      visit_binary_expression((Ast_BinaryExpr*)expr_ast);
      break;

    default: break;
  }
}

internal void
visit_parser_state(Ast_ParserState* state_ast)
{
  Ast_Declaration* expr_ast = state_ast->first_statement;
  while (expr_ast)
  {
    visit_expression((Ast*)expr_ast);
    expr_ast = expr_ast->next_decl;
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
visit_local_declaration(Ast_Declaration* decl_ast)
{
}

internal void
visit_apply_block(Ast_BlockStmt* block_ast)
{
  Ast_Declaration* stmt = block_ast->first_statement;
  while (stmt)
  {
    visit_expression((Ast*)stmt);
    stmt = stmt->next_decl;
  }
}

internal void
visit_control_decl(Ast_ControlDecl* control_ast)
{
  Ast_Declaration* decl_ast = control_ast->first_local_decl;
  while (decl_ast)
  {
    visit_local_declaration(decl_ast);
    decl_ast = decl_ast->next_decl;
  }
  visit_apply_block(control_ast->apply_block);
}

internal void
visit_p4declaration(Ast_Declaration* p4decl_ast)
{
  switch (p4decl_ast->kind)
  {
    case (AST_PARSER_DECL):
      visit_parser_decl((Ast_ParserDecl*)p4decl_ast);
      break;
    case (AST_CONTROL_DECL):
      visit_control_decl((Ast_ControlDecl*)p4decl_ast);
      break;

    default: break;
  }
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
typecheck()
{
  visit_p4program(p4program);
}

