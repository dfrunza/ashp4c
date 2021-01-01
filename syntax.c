#include "dp4c.h"
#include "lex.h"
#include "symtable.h"

external Arena arena;
external Token* tokenized_input;
external int tokenized_input_len;
external int scope_level;
external Ast_P4Program* p4program;

internal Token* token_at = 0;
internal Token* prev_token_at = 0;

Ident_Keyword* error_kw = 0;
Ast_TypeIdent* error_type_ast = 0;
Ident* error_type_ident = 0;

Ast_TypeIdent* void_type_ast = 0;
Ident* void_type_ident = 0;

Ast_TypeIdent* bool_type_ast = 0;
Ident* bool_type_ident = 0;

Ast_TypeIdent* bit_type_ast = 0;
Ident* bit_type_ident = 0;

Ast_TypeIdent* varbit_type_ast = 0;
Ident* varbit_type_ident;

Ast_TypeIdent* int_type_ast = 0;
Ident* int_type_ident;

Ast_TypeIdent* string_type_ast = 0;
Ident* string_type_ident;

Ast_Integer* bool_true_ast = 0;
Ident* bool_true_ident = 0;
Ast_Integer* bool_false_ast = 0;
Ident* bool_false_ident = 0;

internal Ast_TypeExpression* build_type_expression();
internal Ast_Expression* build_expression(int priority_threshold);

internal void
next_token()
{
  assert(token_at < tokenized_input + tokenized_input_len);
  prev_token_at = token_at++;
  while (token_at->klass == TOK_COMMENT)
    token_at++;

  if (token_at->klass == TOK_IDENT)
  {
    Namespace_Entry* ns = sym_get_namespace(token_at->lexeme);
    if (ns->ns_global)
    {
      Ident* ident = ns->ns_global;
      if (ident->ident_kind == ID_KEYWORD)
      {
        token_at->klass = ((Ident_Keyword*)ident)->token_klass;
        return;
      }
    }

    if (ns->ns_type)
    {
      Ident* ident = ns->ns_type;
      if (ident->ident_kind == ID_TYPE || ident->ident_kind == ID_TYPEVAR)
      {
        token_at->klass = TOK_TYPE_IDENT;
        return;
      }
    }
  }
}

internal void
rewind_token()
{
  token_at = prev_token_at;
}

internal Ast_StructField*
build_struct_field()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_StructField* result = arena_push_struct(&arena, Ast_StructField);
  result->kind = AST_STRUCT_FIELD;
  result->member_type = build_type_expression();

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: member '%s' re-declared", result->line_nr, result->name);
    result->member_ident = sym_new_var(result->name, (Ast*)result);

    next_token();
    if (token_at->klass == TOK_SEMICOLON)
      next_token();
    else
      error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal bool
token_is_declaration(Token* token)
{
  bool result = token_at->klass == TOK_KW_STRUCT || token_at->klass == TOK_KW_HEADER ||
      token_at->klass == TOK_KW_ERROR || token_at->klass == TOK_KW_TYPEDEF ||
      token_at->klass == TOK_KW_PARSER || token_at->klass == TOK_KW_CONTROL ||
      token_at->klass == TOK_TYPE_IDENT || token_at->klass == TOK_KW_EXTERN || token_at->klass == TOK_KW_PACKAGE;
  return result;
}

internal Ast_HeaderDecl*
build_header_decl()
{
  Ast_StructField* field;

  assert(token_at->klass == TOK_KW_HEADER);
  next_token();
  Ast_HeaderDecl* result = arena_push_struct(&arena, Ast_HeaderDecl);
  result->kind = AST_HEADER_PROTOTYPE;
  int header_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    next_token();
    if (token_at->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_HEADER_DECL;
      scope_push_level();
      next_token();

      if (token_at->klass == TOK_TYPE_IDENT)
      {
        field = build_struct_field();
        result->first_field = field;
        while (token_at->klass == TOK_TYPE_IDENT)
        {
          Ast_StructField* next_field = build_struct_field();
          field->next_field = next_field;
          field = next_field;
        }
      }

      if (token_at->klass == TOK_BRACE_CLOSE)
      {
        scope_pop_level(header_scope_level);
        next_token(token_at);
      }
      else if (token_at->klass == TOK_IDENT)
        error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
      else
        error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      scope_pop_level(header_scope_level);
    }
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_StructDecl*
build_struct_decl()
{
  assert(token_at->klass == TOK_KW_STRUCT);
  next_token();
  Ast_StructDecl* result = arena_push_struct(&arena, Ast_StructDecl);
  result->kind = AST_STRUCT_PROTOTYPE;
  int struct_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    next_token();

    if (token_at->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_STRUCT_DECL;

      scope_push_level();
      next_token();

      if (token_at->klass == TOK_TYPE_IDENT)
      {
        Ast_StructField* field = build_struct_field();
        result->first_field = field;
        while (token_at->klass == TOK_TYPE_IDENT)
        {
          Ast_StructField* next_field = build_struct_field();
          field->next_field = next_field;
          field = next_field;
        }
      }

      if (token_at->klass == TOK_BRACE_CLOSE)
      {
        scope_pop_level(struct_scope_level);
        next_token();
      }
      else if (token_at->klass == TOK_IDENT)
        error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
      else
        error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      scope_pop_level(struct_scope_level);
    }
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal Ast_ErrorCode*
build_error_code()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_ErrorCode* code = arena_push_struct(&arena, Ast_ErrorCode);
  code->kind = AST_ERROR_CODE;
  code->name = token_at->lexeme;

  if (sym_ident_is_declared(sym_get_var(code->name)))
    error("at line %d: error '%s' re-declared", token_at->line_nr, code->name);
  code->var_ident = sym_new_var(code->name, (Ast*)code);

  next_token();
  return code;
}

internal Ast_ErrorType*
build_error_type_decl()
{
  assert(token_at->klass == TOK_KW_ERROR);
  Ast_ErrorType* result = arena_push_struct(&arena, Ast_ErrorType);
  result->kind = AST_ERROR_TYPE;
  result->line_nr = token_at->line_nr;
  result->type_ident = sym_get_error_type();
  result->type_ident->ast = (Ast*)result;
  next_token();
  int error_scope_level = scope_level;

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    scope_push_level();
    next_token();

    if (token_at->klass == TOK_IDENT)
    {
      Ast_ErrorCode* field = build_error_code();
      result->error_code = field;
      while (token_at->klass == TOK_COMMA)
      {
        next_token();
        if (token_at->klass == TOK_IDENT)
        {
          Ast_ErrorCode* next_code = build_error_code();
          field->next_code = next_code;
          field = next_code;
        }
        else if (token_at->klass == TOK_COMMA)
          error("at line %d: missing parameter", token_at->line_nr);
        else
          error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      scope_pop_level(error_scope_level);
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);

    scope_pop_level(error_scope_level);
  }
  else
    error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal bool
token_is_type_parameter(Token* token)
{
  bool result = (token->klass == TOK_IDENT) || (token->klass == TOK_TYPE_IDENT) || (token->klass == TOK_INTEGER);
  return result;
}

internal Ast_TypeParameter*
build_type_parameter()
{
  assert(token_is_type_parameter(token_at));
  Ast_TypeParameter* result = arena_push_struct(&arena, Ast_TypeParameter);
  result->kind = AST_TYPE_PARAMETER;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->parameter_kind = AST_TYPPARAM_VAR;
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_typevar(result->name, (Ast*)result);

    next_token();
  }
  else if (token_at->klass == TOK_INTEGER)
  {
    result->parameter_kind = AST_TYPPARAM_INT;
    // TODO
    next_token();
  }
  else assert(false);

  return result;
}

internal Ast_TypeParameter*
build_type_parameter_list()
{
  assert(token_is_type_parameter(token_at));
  Ast_TypeParameter* parameter = build_type_parameter();
  Ast_TypeParameter* result = parameter;
  while (token_at->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_type_parameter(token_at))
    {
      Ast_TypeParameter* next_parameter = build_type_parameter();
      parameter->next_parameter = next_parameter;
      parameter = next_parameter;
    }
    else if (token_at->klass == TOK_COMMA)
      error("at line %d: missing parameter", token_at->line_nr);
    else
      error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  return result;
}

internal void
build_type_expression_argument_list()
{
  assert(token_at->klass == TOK_ANGLE_OPEN);
  next_token();
  if (token_at->klass == TOK_TYPE_IDENT)
  {
    //TODO
    next_token();
    while (token_at->klass == TOK_COMMA)
    {
      next_token();
      if (token_at->klass == TOK_TYPE_IDENT)
      {
        //TODO:
        next_token();
      }
      else if (token_at->klass == TOK_INTEGER)
      {
        // TODO
        next_token();
      }
      else if (token_at->klass == TOK_COMMA)
        error("at line %d: missing parameter", token_at->line_nr);
      else
        error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
    }
  }
  else if (token_at->klass == TOK_INTEGER)
  {
    // TODO
    next_token();
  }
  else
    error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);

  if (token_at->klass == TOK_ANGLE_CLOSE)
    next_token();
  else
    error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
}

internal Ast_TypeExpression*
build_type_expression()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_TypeExpression* result = arena_push_struct(&arena, Ast_TypeExpression);
  result->kind = AST_TYPE_EXPRESSION;
  result->name = token_at->lexeme;

  result->type_ident = sym_get_type(token_at->lexeme);
  result->type_ast = result->type_ident->ast;

  next_token();
  if (token_at->klass == TOK_ANGLE_OPEN)
    build_type_expression_argument_list();
  return result;
}

internal Ast_Typedef*
build_typedef_decl()
{
  assert(token_at->klass == TOK_KW_TYPEDEF);
  next_token();
  Ast_Typedef* result = arena_push_struct(&arena, Ast_Typedef);
  result->kind = AST_TYPEDEF;

  if (token_at->klass == TOK_TYPE_IDENT)
  {
    result->type = build_type_expression();

    if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
    {
      result->name = token_at->lexeme;

      if (sym_ident_is_declared(sym_get_type(result->name)))
        error("at line %d: type '%s' re-declared", result->line_nr, result->name);
      result->type_ident = sym_new_type(result->name, (Ast*)result);

      next_token();
    }
    else
      error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (token_at->klass == TOK_IDENT)
    error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
  else
    error("at line %d : unexpected token '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal bool
token_is_direction(Token* token)
{
  bool result = token->klass == TOK_KW_IN || token->klass == TOK_KW_OUT || token->klass == TOK_KW_INOUT;
  return result;
}

internal bool
token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token->klass == TOK_TYPE_IDENT;
  return result;
}

internal enum Ast_ParameterDirection
build_direction()
{
  assert(token_is_direction(token_at));
  enum Ast_ParameterDirection result = 0;
  if (token_at->klass == TOK_KW_IN)
    result = AST_DIR_IN;
  else if (token_at->klass == TOK_KW_OUT)
    result = AST_DIR_OUT;
  else if (token_at->klass == TOK_KW_INOUT)
    result = AST_DIR_INOUT;
  next_token();
  return result;
}

internal Ast_Parameter*
build_parameter()
{
  assert(token_is_parameter(token_at));
  Ast_Parameter* result = arena_push_struct(&arena, Ast_Parameter);
  result->kind = AST_PARAMETER;

  if (token_is_direction(token_at))
    result->direction = build_direction();
  if (token_at->klass == TOK_TYPE_IDENT)
    result->param_type = build_type_expression();
  else
    error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: parameter '%s' re-declared", result->line_nr, result->name);
    result->var_ident = sym_new_var(result->name, (Ast*)result);

    next_token();
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Parameter*
build_parameter_list()
{
  assert(token_is_parameter(token_at));
  Ast_Parameter* parameter = build_parameter();
  Ast_Parameter* result = parameter;
  while (token_at->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_parameter(token_at))
    {
      Ast_Parameter* next_parameter = build_parameter();
      parameter->next_parameter = next_parameter;
      parameter = next_parameter;
    }
    else if (token_at->klass == TOK_COMMA)
      error("at line %d: missing parameter", token_at->line_nr);
    else
      error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
  }
  return result;
}

internal Ast_ParserDecl*
build_parser_prototype()
{
  assert(token_at->klass == TOK_KW_PARSER);
  Ast_ParserDecl* result = arena_push_struct(&arena, Ast_ParserDecl);
  result->kind = AST_PARSER_PROTOTYPE;

  next_token();
  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = build_type_parameter_list();
      else
        error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_ANGLE_CLOSE)
        next_token();
      else
        error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      next_token();

      if (token_is_parameter(token_at))
        result->first_parameter = build_parameter_list();

      if (token_at->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token_at->klass == TOK_IDENT)
          error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
      error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal bool
token_is_sinteger(Token* token)
{
  bool result = token->klass == TOK_SINTEGER || token->klass == TOK_SINTEGER_HEX
    || token->klass == TOK_SINTEGER_OCT || token->klass == TOK_SINTEGER_BIN;
  return result;
}

internal bool
token_is_winteger(Token* token)
{
  bool result = token->klass == TOK_WINTEGER || token->klass == TOK_WINTEGER_HEX
    || token->klass == TOK_WINTEGER_OCT || token->klass == TOK_WINTEGER_BIN;
  return result;
}

internal bool
token_is_integer(Token* token)
{
  bool result = token->klass == TOK_INTEGER || token->klass == TOK_INTEGER_HEX
    || token->klass == TOK_INTEGER_OCT || token->klass == TOK_INTEGER_BIN;
  return result;
}

internal bool
token_is_expression(Token* token)
{
  bool result = token->klass == TOK_IDENT || token->klass == TOK_TYPE_IDENT ||
    token_is_integer(token) || token_is_winteger(token) || token_is_sinteger(token) ||
    token->klass == TOK_STRING || token->klass == TOK_PARENTH_OPEN;
  return result;
}

internal bool
token_is_expression_operator(Token* token)
{
  bool result = token_at->klass == TOK_PERIOD || token_at->klass == TOK_EQUAL_EQUAL
    || token_at->klass == TOK_PARENTH_OPEN || token_at->klass == TOK_EQUAL
    || token_at->klass == TOK_MINUS || token_at->klass == TOK_PLUS;
  return result;
}

internal Ast_Expression*
build_expression_primary()
{
  assert(token_is_expression(token_at));
  Ast_Expression* result = 0;

  if (token_at->klass == TOK_IDENT)
  {
    Ast_Ident* ident_ast = arena_push_struct(&arena, Ast_Ident);
    ident_ast->kind = AST_IDENT;
    ident_ast->name = token_at->lexeme;

    ident_ast->var_ident = sym_get_var(ident_ast->name);
    if (ident_ast->var_ident)
      printf("resolved id '%s'\n", ident_ast->name);
    else
      printf("unknown id '%s'\n", ident_ast->name);

    result = (Ast_Expression*)ident_ast;
    next_token();
  }
  else if (token_at->klass == TOK_TYPE_IDENT)
  {
    Ast_TypeIdent* ident_ast = arena_push_struct(&arena, Ast_TypeIdent);
    ident_ast->kind = AST_TYPE_IDENT;
    ident_ast->name = token_at->lexeme;

    ident_ast->type_ident = sym_get_type(ident_ast->name);
    if (ident_ast->type_ident)
      printf("resolved id '%s'\n", ident_ast->name);
    else
      printf("unknown id '%s'\n", ident_ast->name);

    result = (Ast_Expression*)ident_ast;
    next_token();
  }
  else if (token_is_integer(token_at))
  {
    Ast_Integer* expression = arena_push_struct(&arena, Ast_Integer);
    expression->kind = AST_INTEGER;
    expression->lexeme = token_at->lexeme;
    expression->value = 0; //TODO

    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_is_winteger(token_at))
  {
    Ast_WInteger* expression = arena_push_struct(&arena, Ast_WInteger);
    expression->kind = AST_WINTEGER;
    expression->lexeme = token_at->lexeme;
    expression->value = 0; //TODO

    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_is_sinteger(token_at))
  {
    Ast_SInteger* expression = arena_push_struct(&arena, Ast_SInteger);
    expression->kind = AST_SINTEGER;
    expression->lexeme = token_at->lexeme;
    expression->value = 0; //TODO

    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();

    if (token_is_expression(token_at))
      result = build_expression(1);
    if (token_at->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    assert(false);
  return result;
}

internal enum Ast_ExprOperator
build_expression_operator()
{
  assert(token_is_expression_operator(token_at));
  enum Ast_ExprOperator result = 0;

  switch (token_at->klass)
  {
    case (TOK_PERIOD):
      result = AST_OP_MEMBER_SELECTOR;
      break;
    case (TOK_EQUAL):
      result = AST_OP_ASSIGN;
      break;
    case (TOK_EQUAL_EQUAL):
      result = AST_OP_LOGIC_EQUAL;
      break;
    case (TOK_PARENTH_OPEN):
      result = AST_OP_FUNCTION_CALL;
      break;
    case (TOK_MINUS):
      result = AST_OP_SUBTRACT;
      break;
    case (TOK_PLUS):
      result = AST_OP_ADDITION;
      break;

    default: assert(false);
  }

  return result;
}

internal int
op_get_priority(enum Ast_ExprOperator op)
{
  int result = 0;

  switch (op)
  {
    case (AST_OP_ASSIGN):
      result = 1;
      break;
    case (AST_OP_LOGIC_EQUAL):
      result = 2;
      break;
    case (AST_OP_ADDITION):
    case (AST_OP_SUBTRACT):
      result = 3;
      break;
    case (AST_OP_FUNCTION_CALL):
      result = 4;
      break;
    case (AST_OP_MEMBER_SELECTOR):
      result = 5;
      break;

    default: assert(false);
  }

  return result;
}

internal bool
op_is_binary(enum Ast_ExprOperator op)
{
  bool result = op == AST_OP_LOGIC_EQUAL || op == AST_OP_MEMBER_SELECTOR || op == AST_OP_ASSIGN
    || op == AST_OP_ADDITION || op == AST_OP_SUBTRACT;
  return result;
}

internal Ast_Expression*
build_expression(int priority_threshold)
{
  assert(token_is_expression(token_at));

  Ast_Expression* expr = build_expression_primary();

  while (token_is_expression_operator(token_at))
  {
    enum Ast_ExprOperator op = build_expression_operator();
    int priority = op_get_priority(op);

    if (priority >= priority_threshold)
    {
      next_token();

      if (op_is_binary(op))
      {
        Ast_BinaryExpr* binary_expr = arena_push_struct(&arena, Ast_BinaryExpr);
        binary_expr->kind = AST_BINARY_EXPR;
        binary_expr->l_operand = expr;
        binary_expr->op = op;

        if (token_is_expression(token_at))
          binary_expr->r_operand = build_expression(priority_threshold + 1);
        else
          error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
        expr = (Ast_Expression*)binary_expr;
      }
      else if (op == AST_OP_FUNCTION_CALL)
      {
        Ast_FunctionCall* function_call = arena_push_struct(&arena, Ast_FunctionCall);
        function_call->kind = AST_FUNCTION_CALL;
        function_call->function = expr;

        if (token_is_expression(token_at))
        {
          Ast_Expression* argument = build_expression(1);
          function_call->first_argument = argument;
          while (token_at->klass == TOK_COMMA)
          {
            next_token();
            if (token_is_expression(token_at))
            {
              Ast_Expression* next_argument = build_expression(1);
              argument->next_expression = next_argument;
              argument = next_argument;
            }
            else if (token_at->klass == TOK_COMMA)
              error("at line %d: missing argument", token_at->line_nr);
            else
              error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
          }
        }

        if (token_at->klass == TOK_PARENTH_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);

        expr = (Ast_Expression*)function_call;
      }
      else assert(false);
    }
    else break;
  }
  return expr;
}

internal Ast_IdentState*
build_ident_state()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_IdentState* result = arena_push_struct(&arena, Ast_IdentState);
  result->kind = AST_IDENT_STATE;

  result->name = token_at->lexeme;
  next_token();

  if (token_at->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal bool
token_is_select_case(Token* token)
{
  bool result = token_is_expression(token_at) || token_at->klass == TOK_KW_DEFAULT;
  return result;
}

internal Ast_SelectCase*
build_select_case()
{
  assert(token_is_select_case(token_at));
  Ast_SelectCase* result = 0;

  if (token_is_expression(token_at))
  {
    Ast_SelectCase_Expr* select_expr = arena_push_struct(&arena, Ast_SelectCase_Expr);
    select_expr->kind = AST_SELECT_CASE_EXPR;
    select_expr->key_expr = build_expression(1);
    result = (Ast_SelectCase*)select_expr;
  }
  else if (token_at->klass = TOK_KW_DEFAULT)
  {
    next_token();
    Ast_SelectCase_Default* default_select = arena_push_struct(&arena, Ast_SelectCase_Default);
    default_select->kind = AST_DEFAULT_SELECT_CASE;
    result = (Ast_SelectCase*)default_select;
  }
  else
    assert(false);

  if (token_at->klass == TOK_COLON)
  {
    next_token();

    if (token_at->klass == TOK_IDENT)
    {
      result->end_state = token_at->lexeme;
      next_token();
      if (token_at->klass == TOK_SEMICOLON)
        next_token();
      else
        error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: ':' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal Ast_SelectCase*
build_select_case_list()
{
  assert(token_is_select_case(token_at));
  Ast_SelectCase* select_case = build_select_case();
  Ast_SelectCase* result = select_case;
  while (token_is_select_case(token_at))
  {
    Ast_SelectCase* next_select_case = build_select_case();
    select_case->next = next_select_case;
    select_case = next_select_case;
  }
  return result;
}

internal Ast_SelectState*
build_select_state()
{
  assert(token_at->klass == TOK_KW_SELECT);
  next_token();
  Ast_SelectState* result = arena_push_struct(&arena, Ast_SelectState);
  result->kind = AST_SELECT_STATE;
  if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token_at))
      result->expression = build_expression(1);
    if (token_at->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  if (token_at->klass == TOK_BRACE_OPEN)
  {
    next_token();
    result->select_case = build_select_case_list();
    if (token_at->klass == TOK_BRACE_CLOSE)
      next_token();
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_TransitionStmt*
build_transition_stmt()
{
  assert(token_at->klass == TOK_KW_TRANSITION);
  Ast_TransitionStmt* result = arena_push_struct(&arena, Ast_TransitionStmt);
  result->kind = AST_TRANSITION_STMT;

  next_token();
  if (token_at->klass == TOK_IDENT)
    result->state_expr = (Ast_StateExpr*)build_ident_state();
  else if (token_at->klass == TOK_KW_SELECT)
    result->state_expr = (Ast_StateExpr*)build_select_state();
  else
    error("at line %d: transition stmt expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Expression*
build_statement_list()
{
  Ast_Expression* result = 0;

  if (token_is_expression(token_at))
  {
    Ast_Expression* expression = build_expression(1);
    result = expression;

    if (token_at->klass == TOK_SEMICOLON)
    {
      next_token();

      while (token_is_expression(token_at))
      {
        Ast_Expression* next_expression = build_expression(1);
        expression->next_expression = next_expression;
        expression = next_expression;

        if (token_at->klass == TOK_SEMICOLON)
          next_token();
        else
          error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }
    else
      error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }

  return result;
}

internal Ast_ParserState*
build_parser_state()
{
  assert(token_at->klass == TOK_KW_STATE);
  Ast_ParserState* result = arena_push_struct(&arena, Ast_ParserState);
  result->kind = AST_PARSER_STATE;

  int state_scope_level = scope_level;
  next_token();

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: state '%s' re-declared", result->line_nr, result->name);
    result->var_ident = sym_new_var(result->name, (Ast*)result);

    next_token();
    if (token_at->klass == TOK_BRACE_OPEN)
    {
      next_token();
      result->first_statement = build_statement_list();

      if (token_at->klass == TOK_KW_TRANSITION)
        result->transition_stmt = build_transition_stmt();
      else
        error("at line %d: 'transition' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_BRACE_CLOSE)
      {
        scope_pop_level(state_scope_level);
        next_token();
      }
      else
        error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }

  scope_pop_level(state_scope_level);
  return result;
}

internal Ast_ParserDecl*
build_parser_decl()
{
  assert(token_at->klass == TOK_KW_PARSER);

  int parser_scope_level = scope_level;
  Ast_ParserDecl* result = build_parser_prototype();

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_PARSER_DECL;
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token_at->klass == TOK_KW_STATE)
    {
      Ast_ParserState* state = build_parser_state();
      result->first_parser_state = state;
      while (token_at->klass == TOK_KW_STATE)
      {
        Ast_ParserState* next_state = build_parser_state();
        state->next_state = next_state;
        state = next_state;
      }
    }
    else
      error("at line %d: 'state' keyword expected, got '%s'", token_at->line_nr, token_at->lexeme);

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      scope_pop_level(parser_scope_level);
      sym_import_var((Ident*)error_kw);
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (token_at->klass == TOK_SEMICOLON)
    scope_pop_level(parser_scope_level);
  else
    error("at line %d: '{' or ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(parser_scope_level);

  return result;
}

internal Ast_ControlDecl*
build_control_prototype()
{
  assert(token_at->klass == TOK_KW_CONTROL);
  next_token();
  Ast_ControlDecl* result = arena_push_struct(&arena, Ast_ControlDecl);
  result->kind = AST_CONTROL_PROTOTYPE;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = build_type_parameter_list();
      else
        error("at line %d: identifier expected , got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_ANGLE_CLOSE)
        next_token();
      else
        error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      next_token();

      if (token_is_parameter(token_at))
        result->first_parameter = build_parameter_list();

      if (token_at->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token_at->klass == TOK_IDENT)
          error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_BlockStmt*
build_block_statement()
{
  assert(token_at->klass == TOK_BRACE_OPEN);
  Ast_BlockStmt* result = arena_push_struct(&arena, Ast_BlockStmt);
  result->kind = AST_BLOCK_STMT;

  next_token();

  if (token_is_expression(token_at))
    result->first_statement = build_statement_list();

  if (token_at->klass == TOK_BRACE_CLOSE)
    next_token();
  else
    error("at line %d: block stmt expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

bool
token_is_control_local_decl(Token* token)
{
  bool result = (token->klass == TOK_KW_ACTION) || (token->klass == TOK_KW_TABLE) \
                || (token->klass == TOK_TYPE_IDENT);
  return result;
}

internal Ast_ActionDecl*
build_action_decl()
{
  assert(token_at->klass == TOK_KW_ACTION);
  next_token();
  Ast_ActionDecl* result = arena_push_struct(&arena, Ast_ActionDecl);
  result->kind = AST_ACTION;

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;
    next_token();
    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      next_token();
      if (token_is_parameter(token_at))
        result->parameter = build_parameter_list();
      if (token_at->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
        error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    if (token_at->klass == TOK_BRACE_OPEN)
      result->action_body = build_block_statement();
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Key*
build_key_elem()
{
  assert(token_is_expression(token_at));
  Ast_Key* result = arena_push_struct(&arena, Ast_Key);
  result->kind = AST_TABLE_KEY;
  result->expression = build_expression(1);

  if (token_at->klass == TOK_COLON)
  {
    next_token();
    if (token_at->klass == TOK_IDENT)
    {
      result->name = build_expression(1);
      if (token_at->klass == TOK_SEMICOLON)
        next_token();
      else
        error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: ':' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal Ast_SimpleProp*
build_simple_prop()
{
  assert(token_is_expression(token_at));
  Ast_SimpleProp* result = arena_push_struct(&arena, Ast_SimpleProp);
  result->kind = AST_SIMPLE_PROP;
  result->expression = build_expression(1);

  if (token_at->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_ActionRef*
build_action_ref()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_ActionRef* result = arena_push_struct(&arena, Ast_ActionRef);
  result->kind = AST_ACTION_REF;
  result->name = token_at->lexeme;
  next_token();

  if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token_at))
    {
      Ast_Expression* argument = build_expression(1);
      result->argument = argument;
      while (token_at->klass == TOK_COMMA)
      {
        next_token();
        if (token_is_expression(token_at))
        {
          Ast_Expression* next_argument = build_expression(1);
          argument->next_expression = next_argument;
          argument = next_argument;
        }
        else if (token_at->klass == TOK_COMMA)
          error("at line %d: missing parameter", token_at->line_nr);
        else
          error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }
    if (token_at->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  if (token_at->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal Ast_TableProperty*
build_table_property()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_TableProperty* result = 0;
  // FIXME: WTF is this nonsense?
  if (cstr_match(token_at->lexeme, "key"))
  {
    next_token();
    if (token_at->klass == TOK_EQUAL)
    {
      next_token();
      if (token_at->klass == TOK_BRACE_OPEN)
      {
        next_token();
        if (token_is_expression(token_at))
        {
          Ast_Key* key_elem = build_key_elem();
          result = (Ast_TableProperty*)key_elem;
          while (token_is_expression(token_at))
          {
            Ast_Key* next_key_elem = build_key_elem();
            key_elem->next_key = next_key_elem;
            key_elem = next_key_elem;
          }
        }
        else
          error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
        if (token_at->klass == TOK_BRACE_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
      else
        error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '=' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (cstr_match(token_at->lexeme, "actions"))
  {
    next_token();
    if (token_at->klass == TOK_EQUAL)
    {
      next_token();
      if (token_at->klass == TOK_BRACE_OPEN)
      {
        next_token();
        if (token_at->klass == TOK_IDENT)
        {
          Ast_ActionRef* action_ref = build_action_ref();
          result = (Ast_TableProperty*)action_ref;
          while (token_at->klass == TOK_IDENT)
          {
            Ast_ActionRef* next_action_ref = build_action_ref();
            action_ref->next_action = next_action_ref;
            action_ref = next_action_ref;
          }
        }
        if (token_at->klass == TOK_BRACE_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
      else
        error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '=' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
  {
    Ast_SimpleProp* prop = build_simple_prop();
    result = (Ast_TableProperty*)prop;
    while (token_at->klass == TOK_IDENT)
    {
      Ast_SimpleProp* next_prop = build_simple_prop();
      prop->next = (Ast_TableProperty*)next_prop;
      prop = next_prop;
    }
  }
  return result;
}

internal Ast_TableDecl*
build_table_decl()
{
  assert(token_at->klass == TOK_KW_TABLE);
  next_token();
  Ast_TableDecl* result = arena_push_struct(&arena, Ast_TableDecl);
  result->kind = AST_TABLE;

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;
    next_token();
    if (token_at->klass == TOK_BRACE_OPEN)
    {
      next_token();
      if (token_at->klass == TOK_IDENT)
      {
        Ast_TableProperty* property = build_table_property();
        result->property = property;
        while (token_at->klass == TOK_IDENT)
        {
          Ast_TableProperty* next_property = build_table_property();
          property->next = next_property;
          property = next_property;
        }
      }
      else
        error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
      if (token_at->klass == TOK_BRACE_CLOSE)
        next_token();
      else
        error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_VarDecl*
build_var_decl()
{
  Ast_Ident* name_ast = 0;

  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_VarDecl* var_decl = arena_push_struct(&arena, Ast_VarDecl);
  var_decl->kind = AST_VAR_DECL;

  var_decl->var_type = build_type_expression();
  Ast_Expression* init_expr = build_expression(1);

  if (init_expr->kind == AST_BINARY_EXPR)
  {
    var_decl->init_expr = ((Ast_BinaryExpr*)init_expr)->r_operand;
    init_expr = ((Ast_BinaryExpr*)init_expr)->l_operand;
  }

  if (init_expr->kind == AST_IDENT)
  {
    name_ast = (Ast_Ident*)init_expr;
    var_decl->name = name_ast;
  }
  else
    error("at line %d: identifier expected", init_expr->line_nr);

  if (sym_ident_is_declared(sym_get_var(name_ast->name)))
    error("at line %d: variable '%s' re-declared", name_ast->line_nr, name_ast->name);
  name_ast->var_ident = sym_new_var(name_ast->name, (Ast*)var_decl);

  return var_decl;
}

internal Ast_Declaration*
build_control_local_decl()
{
  Ast_Declaration* result = 0;

  assert(token_is_control_local_decl(token_at));

  if (token_at->klass == TOK_KW_ACTION)
  {
    Ast_ActionDecl* action_decl = build_action_decl();
    result = (Ast_Declaration*)action_decl;
  }
  else if (token_at->klass == TOK_KW_TABLE)
  {
    Ast_TableDecl* table_decl = build_table_decl();
    result = (Ast_Declaration*)table_decl;
  }
  else if (token_at->klass == TOK_TYPE_IDENT)
  {
    Ast_VarDecl* var_decl = build_var_decl();
    result = (Ast_Declaration*)var_decl;

    if (token_at->klass == TOK_SEMICOLON)
      next_token();
    else
      error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    assert(false);
  return result;
}

internal Ast_ControlDecl*
build_control_decl()
{
  assert(token_at->klass == TOK_KW_CONTROL);

  int control_scope_level = scope_level;
  Ast_ControlDecl* result = build_control_prototype();

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_CONTROL_DECL;
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token_is_control_local_decl(token_at))
    {
      Ast_Declaration* local_decl = build_control_local_decl();
      result->first_local_decl = local_decl;
      while (token_is_control_local_decl(token_at))
      {
        Ast_Declaration* next_local_decl = build_control_local_decl(result);
        local_decl->next_decl = next_local_decl;
        local_decl = next_local_decl;
      }
    }

    if (token_at->klass == TOK_KW_APPLY)
    {
      next_token();
      if (token_at->klass == TOK_BRACE_OPEN)
        result->apply_block = build_block_statement();
      else
        error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      scope_pop_level(control_scope_level);
      sym_import_var((Ident*)error_kw);
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (token_at->klass == TOK_SEMICOLON)
    scope_pop_level(control_scope_level);
  else
    error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(control_scope_level);
  return result;
}

internal Ast_PackageDecl*
build_package_prototype()
{
  assert(token_at->klass == TOK_KW_PACKAGE);
  next_token();
  Ast_PackageDecl* result = arena_push_struct(&arena, Ast_PackageDecl);
  result->kind = AST_PACKAGE_PROTOTYPE;
  int package_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = build_type_parameter_list();
      else
        error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_ANGLE_CLOSE)
        next_token();
      else
        error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      next_token();

      if (token_is_parameter(token_at))
        result->first_parameter = build_parameter_list();

      if (token_at->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token_at->klass == TOK_IDENT)
          error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

  if (token_at->klass == TOK_SEMICOLON)
    scope_pop_level(package_scope_level);
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(package_scope_level);
  return result;
}

internal Ast_PackageInstantiation*
build_package_instantiation()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_PackageInstantiation* result = arena_push_struct(&arena, Ast_PackageInstantiation);
  zero_struct(result, Ast_PackageInstantiation);
  result->kind = AST_PACKAGE_INSTANTIATION;
  result->package_ctor = build_expression(1);

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: variable '%s' re-declared", result->line_nr, result->name);
    result->var_ident = sym_new_var(result->name, (Ast*)result);

    next_token();
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_FunctionDecl*
build_function_prototype()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_FunctionDecl* result = arena_push_struct(&arena, Ast_FunctionDecl);
  result->kind = AST_FUNCTION_PROTOTYPE;

  result->return_type_ident = sym_get_type(token_at->lexeme);
  result->return_type_ast = result->return_type_ident->ast;

  int function_scope_level = scope_level;
  next_token();

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
    result->type_ident = sym_new_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = build_type_parameter_list();
      else
        error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_ANGLE_CLOSE)
        next_token();
      else
        error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      sym_unimport_var((Ident*)error_kw);
      next_token();

      if (token_is_parameter(token_at))
        result->first_parameter = build_parameter_list();

      if (token_at->klass == TOK_PARENTH_CLOSE)
      {
        sym_import_var((Ident*)error_kw);
        next_token();
      }
      else
        error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_SEMICOLON)
        scope_pop_level(function_scope_level);
      else
        error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(function_scope_level);
  return result;
}

internal Ast_ExternObjectDecl*
build_extern_object_prototype()
{
  Ast_FunctionDecl* method;

  assert(token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT);
  Ast_ExternObjectDecl* result = arena_push_struct(&arena, Ast_ExternObjectDecl);
  result->kind = AST_EXTERN_OBJECT_PROTOTYPE;

  result->name = token_at->lexeme;

  if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' re-declared", result->line_nr, result->name);
  result->type_ident = sym_new_type(result->name, (Ast*)result);

  int object_scope_level = scope_level;
  scope_push_level();
  next_token();

  if (token_at->klass == TOK_ANGLE_OPEN)
  {
    next_token();

    if (token_is_type_parameter(token_at))
      result->first_type_parameter = build_type_parameter_list();
    else
      error("at line %d: identifier expected , got '%s'", token_at->line_nr, token_at->lexeme);

    if (token_at->klass == TOK_ANGLE_CLOSE)
      next_token();
    else
      error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token_at->klass == TOK_TYPE_IDENT)
    {
      method = build_function_prototype();
      result->first_method = method;

      if (token_at->klass == TOK_SEMICOLON)
        next_token();
      else
        error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      while (token_at->klass == TOK_TYPE_IDENT)
      {
        Ast_FunctionDecl* next_method = build_function_prototype();
        method->next_decl = (Ast_Declaration*)next_method;
        method = next_method;

        if (token_at->klass == TOK_SEMICOLON)
          next_token();
        else
          error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      }
    }

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      sym_import_var((Ident*)error_kw);
      scope_pop_level(object_scope_level);
      next_token();
    }
    else if (token_at->klass == TOK_IDENT)
      error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(object_scope_level);
  return result;
}

internal Ast_FunctionDecl*
build_extern_function_prototype()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_FunctionDecl* result = build_function_prototype();
  result->kind = AST_EXTERN_FUNCTION_PROTOTYPE;
  return result;
}

internal Ast_Declaration*
build_extern_decl()
{
  assert(token_at->klass == TOK_KW_EXTERN);
  Ast_Declaration* result = 0;
  next_token();

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    next_token();
    if (token_at->klass == TOK_ANGLE_OPEN || token_at->klass == TOK_BRACE_OPEN)
    {
      rewind_token();
      result = (Ast_Declaration*)build_extern_object_prototype();
    }
    else if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
    {
      rewind_token();
      result = (Ast_Declaration*)build_extern_function_prototype();
    }
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Declaration*
build_p4declaration()
{
  Ast_Declaration* result = 0;
  assert(token_is_declaration(token_at));

  switch (token_at->klass)
  {
    case (TOK_KW_STRUCT):
      result = (Ast_Declaration*)build_struct_decl();
      break;
    case (TOK_KW_HEADER):
      result = (Ast_Declaration*)build_header_decl();
      break;
    case (TOK_KW_ERROR):
      result = (Ast_Declaration*)build_error_type_decl();
      break;
    case (TOK_KW_TYPEDEF):
      result = (Ast_Declaration*)build_typedef_decl();
      break;
    case (TOK_KW_PARSER):
      result = (Ast_Declaration*)build_parser_decl();
      break;
    case (TOK_KW_CONTROL):
      result = (Ast_Declaration*)build_control_decl();
      break;
    case (TOK_KW_ACTION):
      result = (Ast_Declaration*)build_action_decl();
      break;
    case (TOK_KW_PACKAGE):
      result = (Ast_Declaration*)build_package_prototype();
      break;
    case (TOK_KW_EXTERN):
      result = (Ast_Declaration*)build_extern_decl();
      break;
    case (TOK_TYPE_IDENT):
      result = (Ast_Declaration*)build_package_instantiation();
      break;

    default: assert(false);
  }

  if (token_at->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  return result;
}

internal Ast_P4Program*
build_p4program()
{
  Ast_P4Program* result = 0;
  if (token_is_declaration(token_at))
  {
    result = arena_push_struct(&arena, Ast_P4Program);
    result->kind = AST_P4PROGRAM;
    Ast_Declaration* declaration = build_p4declaration();
    result->first_declaration = declaration;
    while (token_is_declaration(token_at))
    {
      Ast_Declaration* next_declaration = build_p4declaration();
      declaration->next_decl = next_declaration;
      declaration = next_declaration;
    }
    if (token_at->klass != TOK_EOI)
      error("at line %d: expected end of input, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: declaration expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

void
build_ast()
{
  error_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  error_type_ast->kind = AST_TYPE_IDENT;
  error_type_ast->name = "error";
  error_type_ast->is_builtin = true;

  void_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  void_type_ast->kind = AST_TYPE_IDENT;
  void_type_ast->name = "void";
  void_type_ast->is_builtin = true;

  bool_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  bool_type_ast->kind = AST_TYPE_IDENT;
  bool_type_ast->name = "bool";
  bool_type_ast->is_builtin = true;

  bit_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  bit_type_ast->kind = AST_TYPE_IDENT;
  bit_type_ast->name = "bit";
  bit_type_ast->is_builtin = true;

  varbit_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  varbit_type_ast->kind = AST_TYPE_IDENT;
  varbit_type_ast->name = "varbit";
  varbit_type_ast->is_builtin = true;

  int_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  int_type_ast->kind = AST_TYPE_IDENT;
  int_type_ast->name = "int";
  int_type_ast->is_builtin = true;

  string_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  string_type_ast->kind = AST_TYPE_IDENT;
  string_type_ast->name = "string";
  string_type_ast->is_builtin = true;

  bool_true_ast = arena_push_struct(&arena, Ast_Integer);
  bool_true_ast->kind = AST_INTEGER;
  bool_true_ast->lexeme = "true";
  bool_true_ast->value = 1;

  bool_false_ast = arena_push_struct(&arena, Ast_Integer);
  bool_false_ast->kind = AST_INTEGER;
  bool_false_ast->lexeme = "false";
  bool_false_ast->value = 0;

  add_keyword("action", TOK_KW_ACTION);
  add_keyword("const", TOK_KW_CONST);
  add_keyword("enum", TOK_KW_ENUM);
  add_keyword("in", TOK_KW_IN);
  add_keyword("package", TOK_KW_PACKAGE);
  add_keyword("select", TOK_KW_SELECT);
  add_keyword("switch", TOK_KW_SWITCH);
  add_keyword("tuple", TOK_KW_TUPLE);
  add_keyword("control", TOK_KW_CONTROL);
  error_kw = add_keyword("error", TOK_KW_ERROR);
  add_keyword("header", TOK_KW_HEADER);
  add_keyword("inout", TOK_KW_INOUT);
  add_keyword("parser", TOK_KW_PARSER);
  add_keyword("state", TOK_KW_STATE);
  add_keyword("table", TOK_KW_TABLE);
  add_keyword("typedef", TOK_KW_TYPEDEF);
  add_keyword("default", TOK_KW_DEFAULT);
  add_keyword("extern", TOK_KW_EXTERN);
  add_keyword("header_union", TOK_KW_HEADER_UNION);
  add_keyword("out", TOK_KW_OUT);
  add_keyword("transition", TOK_KW_TRANSITION);
  add_keyword("else", TOK_KW_ELSE);
  add_keyword("exit", TOK_KW_EXIT);
  add_keyword("if", TOK_KW_IF);
  add_keyword("match_kind", TOK_KW_MATCH_KIND);
  add_keyword("return", TOK_KW_RETURN);
  add_keyword("struct", TOK_KW_STRUCT);
  add_keyword("apply", TOK_KW_APPLY);
  //add_keyword("verify", TOK_KW_VERIFY);

  error_type_ident = sym_new_type(error_type_ast->name, (Ast*)error_type_ast);
  void_type_ident = sym_new_type(void_type_ast->name, (Ast*)void_type_ast);
  bool_type_ident = sym_new_type("bool", (Ast*)bool_type_ast);
  bit_type_ident = sym_new_type("bit", (Ast*)bit_type_ast);
  varbit_type_ident = sym_new_type("varbit", (Ast*)varbit_type_ast);
  int_type_ident = sym_new_type("int", (Ast*)int_type_ast);
  string_type_ident = sym_new_type("string", (Ast*)string_type_ast);

  bool_true_ident = sym_new_var("true", (Ast*)bool_true_ast);
  bool_false_ident = sym_new_var("false", (Ast*)bool_false_ast);

  assert(scope_level == 0);
  int top_scope_level = scope_push_level();
  token_at = tokenized_input;
  next_token();
  p4program = build_p4program();
  scope_pop_level(top_scope_level - 1);
  assert(scope_level == 0);
}
