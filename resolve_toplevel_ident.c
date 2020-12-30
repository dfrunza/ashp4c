#include "symtable.h"
#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;
external int scope_level;

internal void visit_expression(Ast_Expression* expr_ast);

internal void
visit_ident_expression(Ast_Ident* ident_ast)
{
  Ident* var_ident = sym_get_var(ident_ast->name);
  if (!var_ident)
    error("at line %d: unknown identifier '%s'", ident_ast->line_nr, ident_ast->name);
  ident_ast->var_ident = var_ident;
}

internal void
visit_type_ident_expression(Ast_TypeIdent* ident_ast)
{
  Ident* type_ident = sym_get_type(ident_ast->name);
  if (!type_ident)
    error("at line %d: unknown type '%s'", ident_ast->line_nr, ident_ast->name);
  ident_ast->type_ident = type_ident;
}

internal void
visit_binary_expression(Ast_BinaryExpr* expr_ast)
{
  Ast_Expression* l_operand = expr_ast->l_operand;
  Ast_Expression* r_operand = expr_ast->r_operand;

  if (expr_ast->op == AST_OP_MEMBER_SELECTOR)
    visit_expression(l_operand);
  else
  {
    visit_expression(l_operand);
    visit_expression(r_operand);
  }
}

internal void
visit_function_call(Ast_FunctionCall* call_ast)
{
  if (call_ast->function->kind == AST_IDENT)
    visit_ident_expression((Ast_Ident*)call_ast->function);
  else if (call_ast->function->kind == AST_TYPE_IDENT)
    visit_type_ident_expression((Ast_TypeIdent*)call_ast->function);
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

    case (AST_IDENT):
    {
      visit_ident_expression((Ast_Ident*)expr_ast);
    } break;

    case (AST_TYPE_IDENT):
    {
      visit_type_ident_expression((Ast_TypeIdent*)expr_ast);
    } break;

    case (AST_FUNCTION_CALL):
    {
      visit_function_call((Ast_FunctionCall*)expr_ast);
    } break;

    default: break;
  }
}

internal void
visit_parser_state(Ast_ParserState* state_ast)
{
  scope_push_level();
  Ast_Expression* expr_ast = state_ast->first_statement;
  while (expr_ast)
  {
    visit_expression(expr_ast);
    expr_ast = expr_ast->next_expression;
  }
  scope_pop_level(scope_level - 1);
}

internal void
visit_struct_decl(Ast_StructDecl* struct_ast)
{
  sym_import_type(struct_ast->type_ident);
}

internal void
visit_header_decl(Ast_HeaderDecl* header_ast)
{
  sym_import_type(header_ast->type_ident);
}

internal void
visit_error_type(Ast_ErrorType* error_ast)
{
  sym_import_type(error_ast->type_ident);
}

internal void
visit_typedef(Ast_Typedef* typedef_ast)
{
  sym_import_type(typedef_ast->type_ident);
}

internal void
visit_parser_decl(Ast_ParserDecl* parser_ast)
{
  int parser_scope_level = scope_level;
  sym_import_type(parser_ast->type_ident);

  scope_push_level();
  Ast_TypeParameter* type_parameter = parser_ast->first_type_parameter;
  while (type_parameter)
  {
    sym_import_type(type_parameter->type_ident);
    type_parameter = type_parameter->next_parameter;
  }

  Ast_Parameter* parameter = parser_ast->first_parameter;
  while (parameter)
  {
    sym_import_var(parameter->var_ident);
    parameter = parameter->next_parameter;
  }

  if (parser_ast->kind == AST_PARSER_DECL)
  {
    Ast_ParserState* state_ast = parser_ast->first_parser_state;
    while (state_ast)
    {
      visit_parser_state(state_ast);
      state_ast = state_ast->next_state;
    }
  }

  scope_pop_level(parser_scope_level);
}

internal void
visit_control_decl(Ast_ControlDecl* control_ast)
{
  sym_import_type(control_ast->type_ident);
}

internal void
visit_package_prototype(Ast_PackageDecl* package_ast)
{
  sym_import_type(package_ast->type_ident);
}

internal void
visit_extern_object_prototype(Ast_ExternObjectDecl* object_ast)
{
  sym_import_type(object_ast->type_ident);
}

internal void
visit_extern_function_prototype(Ast_FunctionDecl* function_ast)
{
  sym_import_type(function_ast->type_ident);
}

internal void
visit_package_instantiation(Ast_PackageInstantiation* instance_ast)
{
  visit_expression(instance_ast->package_ctor);
  sym_import_var(instance_ast->var_ident);
}

internal void
visit_p4declaration(Ast_Declaration* p4decl_ast)
{
  if (p4decl_ast->kind == AST_STRUCT_PROTOTYPE || p4decl_ast->kind == AST_STRUCT_DECL)
    visit_struct_decl((Ast_StructDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_HEADER_PROTOTYPE || p4decl_ast->kind == AST_HEADER_DECL)
    visit_header_decl((Ast_HeaderDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_ERROR_TYPE)
    visit_error_type((Ast_ErrorType*)p4decl_ast);
  else if (p4decl_ast->kind == AST_TYPEDEF)
    visit_typedef((Ast_Typedef*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PARSER_PROTOTYPE || p4decl_ast->kind == AST_PARSER_DECL)
    visit_parser_decl((Ast_ParserDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_CONTROL_PROTOTYPE || p4decl_ast->kind == AST_CONTROL_DECL)
    visit_control_decl((Ast_ControlDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_PROTOTYPE)
    visit_package_prototype((Ast_PackageDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_OBJECT_PROTOTYPE)
    visit_extern_object_prototype((Ast_ExternObjectDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_FUNCTION_PROTOTYPE)
    visit_extern_function_prototype((Ast_FunctionDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_INSTANCE)
    visit_package_instantiation((Ast_PackageInstantiation*)p4decl_ast);
  else
    assert (false);
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
resolve_toplevel_ident()
{
  assert (scope_level == 0);
  int top_scope_level = scope_push_level();
  visit_p4program(p4program);
  scope_pop_level(top_scope_level - 1);
  assert (scope_level == 0);
}
