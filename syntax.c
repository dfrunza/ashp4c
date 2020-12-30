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

internal Ast_Typeref* syn_typeref();
internal Ast_Expression* syn_expression(int priority_threshold);

Ast_Ident* error_type_ast = 0;
Ast_VarDecl* error_var_ast = 0;
Ast_Ident* void_type_ast = 0;
Ast_Ident* bool_type_ast = 0;
Ast_Ident* bit_type_ast = 0;
Ast_Ident* varbit_type_ast = 0;
Ast_Ident* int_type_ast = 0;
Ast_Ident* string_type_ast = 0;

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
syn_struct_field()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_StructField* result = arena_push_struct(&arena, Ast_StructField);
  zero_struct(result, Ast_StructField);
  result->kind = AST_STRUCT_FIELD;
  result->typeref = syn_typeref();

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: member '%s' has been previously declared", token_at->line_nr, result->name);
    result->var_ident = sym_add_var(result->name, (Ast*)result);

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
syn_header_decl()
{
  Ast_StructField* field;

  assert(token_at->klass == TOK_KW_HEADER);
  next_token();
  Ast_HeaderDecl* result = arena_push_struct(&arena, Ast_HeaderDecl);
  zero_struct(result, Ast_HeaderDecl);
  result->kind = AST_HEADER_PROTOTYPE;
  int header_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    next_token();
    if (token_at->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_HEADER_DECL;
      scope_push_level();
      next_token();

      if (token_at->klass == TOK_TYPE_IDENT)
      {
        field = syn_struct_field();
        result->first_field = field;
        while (token_at->klass == TOK_TYPE_IDENT)
        {
          Ast_StructField* next_field = syn_struct_field();
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
syn_struct_decl()
{
  assert(token_at->klass == TOK_KW_STRUCT);
  next_token();
  Ast_StructDecl* result = arena_push_struct(&arena, Ast_StructDecl);
  zero_struct(result, Ast_StructDecl);
  result->kind = AST_STRUCT_PROTOTYPE;
  int struct_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    next_token();

    if (token_at->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_STRUCT_DECL;

      scope_push_level();
      next_token();

      if (token_at->klass == TOK_TYPE_IDENT)
      {
        Ast_StructField* field = syn_struct_field();
        result->first_field = field;
        while (token_at->klass == TOK_TYPE_IDENT)
        {
          Ast_StructField* next_field = syn_struct_field();
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
syn_error_code()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_ErrorCode* code = arena_push_struct(&arena, Ast_ErrorCode);
  zero_struct(code, Ast_ErrorCode);
  code->kind = AST_ERROR_CODE;
  code->name = token_at->lexeme;

  if (sym_ident_is_declared(sym_get_var(code->name)))
    error("at line %d: member '%s' has been previously declared", token_at->line_nr, code->name);
  code->var_ident = sym_add_var(code->name, (Ast*)code);

  next_token();
  return code;
}

internal Ast_ErrorType*
syn_error_type_decl()
{
  assert(token_at->klass == TOK_KW_ERROR);
  Ast_ErrorType* result = arena_push_struct(&arena, Ast_ErrorType);
  zero_struct(result, Ast_ErrorType);
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
      Ast_ErrorCode* field = syn_error_code();
      result->error_code = field;
      while (token_at->klass == TOK_COMMA)
      {
        next_token();
        if (token_at->klass == TOK_IDENT)
        {
          Ast_ErrorCode* next_code = syn_error_code();
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
syn_type_parameter()
{
  assert(token_is_type_parameter(token_at));
  Ast_TypeParameter* result = arena_push_struct(&arena, Ast_TypeParameter);
  zero_struct(result, Ast_TypeParameter);
  result->kind = AST_TYPE_PARAMETER;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->parameter_kind = AST_TYPPARAM_VAR;
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_typevar(result->name, (Ast*)result);

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
syn_type_parameter_list()
{
  assert(token_is_type_parameter(token_at));
  Ast_TypeParameter* parameter = syn_type_parameter();
  Ast_TypeParameter* result = parameter;
  while (token_at->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_type_parameter(token_at))
    {
      Ast_TypeParameter* next_parameter = syn_type_parameter();
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
syn_typeref_argument_list()
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

internal Ast_Typeref*
syn_typeref()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_Typeref* result = arena_push_struct(&arena, Ast_Typeref);
  zero_struct(result, Ast_Typeref);
  result->kind = AST_TYPEREF;
  result->name = token_at->lexeme;
  result->type_ident = sym_get_type(token_at->lexeme);
  result->type_ast = result->type_ident->ast;
  next_token();
  if (token_at->klass == TOK_ANGLE_OPEN)
    syn_typeref_argument_list();
  return result;
}

internal Ast_Typedef*
syn_typedef_decl()
{
  assert(token_at->klass == TOK_KW_TYPEDEF);
  next_token();
  Ast_Typedef* result = arena_push_struct(&arena, Ast_Typedef);
  zero_struct(result, Ast_Typedef);
  result->kind = AST_TYPEDEF;

  if (token_at->klass == TOK_TYPE_IDENT)
  {
    result->typeref = syn_typeref();

    if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
    {
      result->name = token_at->lexeme;

      if (sym_ident_is_declared(sym_get_type(result->name)))
        error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
      result->type_ident = sym_add_type(result->name, (Ast*)result);

      next_token();
      if (token_at->klass == TOK_SEMICOLON)
        next_token();
      else
        error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
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

internal enum AstParameterDirection
syn_direction()
{
  assert(token_is_direction(token_at));
  enum AstParameterDirection result = 0;
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
syn_parameter()
{
  assert(token_is_parameter(token_at));
  Ast_Parameter* result = arena_push_struct(&arena, Ast_Parameter);
  zero_struct(result, Ast_Parameter);
  result->kind = AST_PARAMETER;

  if (token_is_direction(token_at))
    result->direction = syn_direction();
  if (token_at->klass == TOK_TYPE_IDENT)
    result->typeref = syn_typeref();
  else
    error("at line %d: unknown type '%s'", token_at->line_nr, token_at->lexeme);

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: parameter '%s' has been previously declared", token_at->line_nr, result->name);
    result->var_ident = sym_add_var(result->name, (Ast*)result);

    next_token();
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Parameter*
syn_parameter_list()
{
  assert(token_is_parameter(token_at));
  Ast_Parameter* parameter = syn_parameter();
  Ast_Parameter* result = parameter;
  while (token_at->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_parameter(token_at))
    {
      Ast_Parameter* next_parameter = syn_parameter();
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
syn_parser_prototype()
{
  assert(token_at->klass == TOK_KW_PARSER);
  next_token();
  Ast_ParserDecl* result = arena_push_struct(&arena, Ast_ParserDecl);
  zero_struct(result, Ast_ParserDecl);
  result->kind = AST_PARSER_PROTOTYPE;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = syn_type_parameter_list();
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
        result->first_parameter = syn_parameter_list();

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
    //token->klass == TOK_KW_TRUE || token->klass == TOK_KW_FALSE ||
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
syn_expression_primary()
{
  assert(token_is_expression(token_at));
  Ast_Expression* result = 0;

  if (token_at->klass == TOK_IDENT)
  {
    Ast_IdentExpr* expression = arena_push_struct(&arena, Ast_IdentExpr);
    zero_struct(expression, Ast_IdentExpr);
    expression->kind = AST_IDENT_EXPR;
    expression->name = token_at->lexeme;
    expression->var_ident = sym_get_var(expression->name);
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_at->klass == TOK_TYPE_IDENT)
  {
    Ast_TypeIdentExpr* expression = arena_push_struct(&arena, Ast_TypeIdentExpr);
    zero_struct(expression, Ast_TypeIdentExpr);
    expression->kind = AST_TYPE_IDENT_EXPR;
    expression->name = token_at->lexeme;
    expression->type_ident = sym_get_type(expression->name);
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_is_integer(token_at))
  {
    Ast_IntegerExpr* expression = arena_push_struct(&arena, Ast_IntegerExpr);
    zero_struct(expression, Ast_IntegerExpr);
    expression->kind = AST_INTEGER_EXPR;
    expression->value = 0; //TODO
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_is_winteger(token_at))
  {
    Ast_WIntegerExpr* expression = arena_push_struct(&arena, Ast_WIntegerExpr);
    zero_struct(expression, Ast_WIntegerExpr);
    expression->kind = AST_WINTEGER_EXPR;
    expression->value = 0; //TODO
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_is_sinteger(token_at))
  {
    Ast_SIntegerExpr* expression = arena_push_struct(&arena, Ast_SIntegerExpr);
    zero_struct(expression, Ast_SIntegerExpr);
    expression->kind = AST_SINTEGER_EXPR;
    expression->value = 0; //TODO
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_at->klass == TOK_KW_ERROR)
  {
    Ast_ErrorExpr* expression = arena_push_struct(&arena, Ast_ErrorExpr);
    zero_struct(expression, Ast_ErrorExpr);
    expression->kind = AST_ERROR_EXPR;
    result = (Ast_Expression*)expression;
    next_token();
  }
  else if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token_at))
      result = syn_expression(1);
    if (token_at->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
//  else if (token_at->klass == TOK_KW_TRUE || token_at->klass == TOK_KW_FALSE)
//  {
//    Ast_Bool* boolean = arena_push_struct(&arena, Ast_Bool);
//    *boolean = (Ast_Bool){};
//    boolean->kind = AST_BOOL;
//    if (token_at->klass == TOK_KW_TRUE)
//      boolean->value = true;
//    token_at++;
//  }
  else if (token_at->klass == TOK_KW_APPLY)
  {
    error("TODO");
    //Ast_IdentExpr* expression = arena_push_struct(&arena, Ast_IdentExpr);
    //zero_struct(expression, Ast_IdentExpr);
    //expression->kind = AST_IDENT;
    //if (token_at->klass == TOK_KW_APPLY)
    //  expression->name = "apply";
    //else if (token_at->klass == TOK_KW_VERIFY)
    //  expression->name = "verify";
    //result = (Ast_Expression*)expression;
    //next_token();
  }
  else
    assert(false);
  return result;
}

internal enum AstExprOperator
syn_expression_operator()
{
  assert(token_is_expression_operator(token_at));
  enum AstExprOperator result = 0;
  if (token_at->klass == TOK_PERIOD)
    result = OP_MEMBER_SELECTOR;
  else if (token_at->klass == TOK_EQUAL)
    result = OP_ASSIGN;
  else if (token_at->klass == TOK_EQUAL_EQUAL)
    result = OP_LOGIC_EQUAL;
  else if (token_at->klass == TOK_PARENTH_OPEN)
    result = OP_FUNCTION_CALL;
  else if (token_at->klass == TOK_MINUS)
    result = OP_SUBTRACT;
  else if (token_at->klass == TOK_PLUS)
    result = OP_ADDITION;
  else
    assert(false);
  return result;
}

internal int
op_get_priority(enum AstExprOperator op)
{
  int result = 0;
  if (op == OP_ASSIGN)
    result = 1;
  else if (op == OP_LOGIC_EQUAL)
    result = 2;
  else if (op == OP_ADDITION || op == OP_SUBTRACT)
    result = 3;
  else if (op == OP_FUNCTION_CALL)
    result = 4;
  else if (op == OP_MEMBER_SELECTOR)
    result = 5;
  else
    assert(false);
  return result;
}

internal bool
op_is_binary(enum AstExprOperator op)
{
  bool result = op == OP_LOGIC_EQUAL || op == OP_MEMBER_SELECTOR || op == OP_ASSIGN
    || op == OP_ADDITION || op == OP_SUBTRACT;
  return result;
}

internal Ast_Expression*
syn_expression(int priority_threshold)
{
  assert(token_is_expression(token_at));
  Ast_Expression* primary = syn_expression_primary();
  Ast_Expression* result = primary;
  while (token_is_expression_operator(token_at))
  {
    enum AstExprOperator op = syn_expression_operator();
    int priority = op_get_priority(op);
    if (priority >= priority_threshold)
    {
      next_token();
      if (op_is_binary(op))
      {
        Ast_BinaryExpr* binary_expr = arena_push_struct(&arena, Ast_BinaryExpr);
        zero_struct(binary_expr, Ast_BinaryExpr)
        binary_expr->kind = AST_BINARY_EXPR;
        binary_expr->l_operand = result;
        binary_expr->op = op;

        if (token_is_expression(token_at))
          binary_expr->r_operand = syn_expression(priority_threshold + 1);
        else
          error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
        result = (Ast_Expression*)binary_expr;
      }
      else if (op == OP_FUNCTION_CALL)
      {
        Ast_FunctionCallExpr* function_call = arena_push_struct(&arena, Ast_FunctionCallExpr);
        zero_struct(function_call, Ast_FunctionCallExpr);
        function_call->kind = AST_FUNCTION_CALL;
        if (token_is_expression(token_at))
        {
          Ast_Expression* argument = syn_expression(1);
          function_call->argument = argument;
          while (token_at->klass == TOK_COMMA)
          {
            next_token();
            if (token_is_expression(token_at))
            {
              Ast_Expression* next_argument = syn_expression(1);
              argument->next = next_argument;
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
          error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
        result = (Ast_Expression*)function_call;
      }
      else
        assert(false);
    }
    else
      break;
  }
  return result;
}

internal Ast_IdentState*
syn_ident_state()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_IdentState* result = arena_push_struct(&arena, Ast_IdentState);
  zero_struct(result, Ast_IdentState);
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
syn_select_case()
{
  assert(token_is_select_case(token_at));
  Ast_SelectCase* result = 0;
  if (token_is_expression(token_at))
  {
    Ast_ExprSelectCase* expr_select = arena_push_struct(&arena, Ast_ExprSelectCase);
    zero_struct(expr_select, Ast_ExprSelectCase);
    expr_select->kind = AST_EXPR_SELECT_CASE;
    expr_select->key_expr = syn_expression(1);
    result = (Ast_SelectCase*)expr_select;
  }
  else if (token_at->klass = TOK_KW_DEFAULT)
  {
    next_token();
    Ast_DefaultSelectCase* default_select = arena_push_struct(&arena, Ast_DefaultSelectCase);
    zero_struct(default_select, Ast_DefaultSelectCase);
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
syn_select_case_list()
{
  assert(token_is_select_case(token_at));
  Ast_SelectCase* select_case = syn_select_case();
  Ast_SelectCase* result = select_case;
  while (token_is_select_case(token_at))
  {
    Ast_SelectCase* next_select_case = syn_select_case();
    select_case->next = next_select_case;
    select_case = next_select_case;
  }
  return result;
}

internal Ast_SelectState*
syn_select_state()
{
  assert(token_at->klass == TOK_KW_SELECT);
  next_token();
  Ast_SelectState* result = arena_push_struct(&arena, Ast_SelectState);
  zero_struct(result, Ast_SelectState);
  result->kind = AST_SELECT_STATE;
  if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token_at))
      result->expression = syn_expression(1);
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
    result->select_case = syn_select_case_list();
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
syn_transition_stmt()
{
  assert(token_at->klass == TOK_KW_TRANSITION);
  next_token();
  Ast_TransitionStmt* result = arena_push_struct(&arena, Ast_TransitionStmt);
  zero_struct(result, Ast_TransitionStmt);
  result->kind = AST_TRANSITION_STMT;

  if (token_at->klass == TOK_IDENT)
    result->state_expr = (Ast_StateExpr*)syn_ident_state();
  else if (token_at->klass == TOK_KW_SELECT)
    result->state_expr = (Ast_StateExpr*)syn_select_state();
  else
    error("at line %d: transition stmt expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Expression*
syn_statement_list()
{
  Ast_Expression* result = 0;
  if (token_is_expression(token_at))
  {
    Ast_Expression* expression = syn_expression(1);
    result = expression;
    if (token_at->klass == TOK_SEMICOLON)
    {
      next_token();
      while (token_is_expression(token_at))
      {
        Ast_Expression* next_expression = syn_expression(1);
        expression->next = next_expression;
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
syn_parser_state()
{
  assert(token_at->klass == TOK_KW_STATE);
  next_token();
  Ast_ParserState* result = arena_push_struct(&arena, Ast_ParserState);
  zero_struct(result, Ast_ParserState);
  result->kind = AST_PARSER_STATE;

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;
    next_token();
    if (token_at->klass == TOK_BRACE_OPEN)
    {
      next_token();
      result->statement = syn_statement_list();
      if (token_at->klass == TOK_KW_TRANSITION)
        result->transition_stmt = syn_transition_stmt();
      else
        error("at line %d: 'transition' expected, got '%s'", token_at->line_nr, token_at->lexeme);
      if (token_at->klass == TOK_BRACE_CLOSE)
        next_token();
      else
        error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  return result;
}

internal Ast_ParserDecl*
syn_parser_decl()
{
  assert(token_at->klass == TOK_KW_PARSER);

  int parser_scope_level = scope_level;
  Ast_ParserDecl* result = syn_parser_prototype();

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_PARSER_DECL;
    sym_remove_error_kw();
    sym_add_error_var();
    next_token();

    if (token_at->klass == TOK_KW_STATE)
    {
      Ast_ParserState* state = syn_parser_state();
      result->parser_state = state;
      while (token_at->klass == TOK_KW_STATE)
      {
        Ast_ParserState* next_state = syn_parser_state();
        state->next = next_state;
        state = next_state;
      }
    }
    else
      error("at line %d: 'state' keyword expected, got '%s'", token_at->line_nr, token_at->lexeme);

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      scope_pop_level(parser_scope_level);
      sym_remove_error_var();
      sym_add_error_kw();
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (token_at->klass == TOK_SEMICOLON)
  {
    scope_pop_level(parser_scope_level);
    next_token();
  }
  else
    error("at line %d: '{' or ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(parser_scope_level);
  return result;
}

internal Ast_ControlDecl*
syn_control_prototype()
{
  assert(token_at->klass == TOK_KW_CONTROL);
  next_token();
  Ast_ControlDecl* result = arena_push_struct(&arena, Ast_ControlDecl);
  zero_struct(result, Ast_ControlDecl);
  result->kind = AST_CONTROL_PROTOTYPE;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = syn_type_parameter_list();
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
        result->first_parameter = syn_parameter_list();

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
syn_block_statement()
{
  assert(token_at->klass == TOK_BRACE_OPEN);
  next_token();
  Ast_BlockStmt* result = arena_push_struct(&arena, Ast_BlockStmt);
  zero_struct(result, Ast_BlockStmt);
  result->kind = AST_BLOCK_STMT;

  if (token_is_expression(token_at))
    result->statement = syn_statement_list();

  if (token_at->klass == TOK_BRACE_CLOSE)
    next_token();
  else
    error("at line %d: block stmt expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

bool
token_is_control_local_decl(Token* token)
{
  bool result = token->klass == TOK_KW_ACTION || token->klass == TOK_KW_TABLE ||
    token->klass == TOK_TYPE_IDENT;
  return result;
}

internal Ast_ActionDecl*
syn_action_decl()
{
  assert(token_at->klass == TOK_KW_ACTION);
  next_token();
  Ast_ActionDecl* result = arena_push_struct(&arena, Ast_ActionDecl);
  zero_struct(result, Ast_ActionDecl);
  result->kind = AST_ACTION;

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;
    next_token();
    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      next_token();
      if (token_is_parameter(token_at))
        result->parameter = syn_parameter_list();
      if (token_at->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
        error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }
    else
      error("at line %d: '(' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    if (token_at->klass == TOK_BRACE_OPEN)
      result->action_body = syn_block_statement();
    else
      error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Key*
syn_key_elem()
{
  assert(token_is_expression(token_at));
  Ast_Key* result = arena_push_struct(&arena, Ast_Key);
  zero_struct(result, Ast_Key);
  result->kind = AST_TABLE_KEY;
  result->expression = syn_expression(1);

  if (token_at->klass == TOK_COLON)
  {
    next_token();
    if (token_at->klass == TOK_IDENT)
    {
      result->name = syn_expression(1);
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
syn_simple_prop()
{
  assert(token_is_expression(token_at));
  Ast_SimpleProp* result = arena_push_struct(&arena, Ast_SimpleProp);
  zero_struct(result, Ast_SimpleProp);
  result->kind = AST_SIMPLE_PROP;
  result->expression = syn_expression(1);
  if (token_at->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_ActionRef*
syn_action_ref()
{
  assert(token_at->klass == TOK_IDENT);
  Ast_ActionRef* result = arena_push_struct(&arena, Ast_ActionRef);
  zero_struct(result, Ast_ActionRef);
  result->kind = AST_ACTION_REF;
  result->name = token_at->lexeme;
  next_token();

  if (token_at->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token_at))
    {
      Ast_Expression* argument = syn_expression(1);
      result->argument = argument;
      while (token_at->klass == TOK_COMMA)
      {
        next_token();
        if (token_is_expression(token_at))
        {
          Ast_Expression* next_argument = syn_expression(1);
          argument->next = next_argument;
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
syn_table_property()
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
          Ast_Key* key_elem = syn_key_elem();
          result = (Ast_TableProperty*)key_elem;
          while (token_is_expression(token_at))
          {
            Ast_Key* next_key_elem = syn_key_elem();
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
          Ast_ActionRef* action_ref = syn_action_ref();
          result = (Ast_TableProperty*)action_ref;
          while (token_at->klass == TOK_IDENT)
          {
            Ast_ActionRef* next_action_ref = syn_action_ref();
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
    Ast_SimpleProp* prop = syn_simple_prop();
    result = (Ast_TableProperty*)prop;
    while (token_at->klass == TOK_IDENT)
    {
      Ast_SimpleProp* next_prop = syn_simple_prop();
      prop->next = (Ast_TableProperty*)next_prop;
      prop = next_prop;
    }
  }
  return result;
}

internal Ast_TableDecl*
syn_table_decl()
{
  assert(token_at->klass == TOK_KW_TABLE);
  next_token();
  Ast_TableDecl* result = arena_push_struct(&arena, Ast_TableDecl);
  zero_struct(result, Ast_TableDecl);
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
        Ast_TableProperty* property = syn_table_property();
        result->property = property;
        while (token_at->klass == TOK_IDENT)
        {
          Ast_TableProperty* next_property = syn_table_property();
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
syn_var_decl()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_VarDecl* result = arena_push_struct(&arena, Ast_VarDecl);
  zero_struct(result, Ast_VarDecl);
  result->kind = AST_VAR_DECL;
  syn_typeref();

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: variable '%s' has been previously declared", token_at->line_nr, result->name);
    result->var_ident = sym_add_var(result->name, (Ast*)result);

    next_token();
    if (token_at->klass == TOK_EQUAL)
    {
      next_token();
      if (token_is_expression(token_at))
        result->initializer = syn_expression(1);
      else
        error("at line %d: expression term expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_SEMICOLON)
      next_token();
    else
      error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Declaration*
syn_control_local_decl()
{
  assert(token_is_control_local_decl(token_at));
  Ast_Declaration* result = 0;
  if (token_at->klass == TOK_KW_ACTION)
  {
    Ast_ActionDecl* action_decl = syn_action_decl();
    result = (Ast_Declaration*)action_decl;
  }
  else if (token_at->klass == TOK_KW_TABLE)
  {
    Ast_TableDecl* table_decl = syn_table_decl();
    result = (Ast_Declaration*)table_decl;
  }
  else if (token_at->klass == TOK_TYPE_IDENT)
  {
    Ast_VarDecl* var_decl = syn_var_decl();
    result = (Ast_Declaration*)var_decl;
  }
  else
    assert(false);
  return result;
}

internal Ast_ControlDecl*
syn_control_decl()
{
  assert(token_at->klass == TOK_KW_CONTROL);

  int control_scope_level = scope_level;
  Ast_ControlDecl* result = syn_control_prototype();

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_CONTROL_DECL;
    sym_remove_error_kw();
    sym_add_error_var();
    next_token();

    if (token_is_control_local_decl(token_at))
    {
      Ast_Declaration* local_decl = syn_control_local_decl();
      result->local_decl = local_decl;
      while (token_is_control_local_decl(token_at))
      {
        Ast_Declaration* next_local_decl = syn_control_local_decl(result);
        local_decl->next_decl = local_decl;
        local_decl = next_local_decl;
      }
    }

    if (token_at->klass == TOK_KW_APPLY)
    {
      next_token();
      if (token_at->klass == TOK_BRACE_OPEN)
        result->control_body = syn_block_statement();
      else
        error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      scope_pop_level(control_scope_level);
      sym_remove_error_var();
      sym_add_error_kw();
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else if (token_at->klass == TOK_SEMICOLON)
  {
    scope_pop_level(control_scope_level);
    next_token();
  }
  else
    error("at line %d: '{' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(control_scope_level);
  return result;
}

internal Ast_PackageDecl*
syn_package_prototype()
{
  assert(token_at->klass == TOK_KW_PACKAGE);
  next_token();
  Ast_PackageDecl* result = arena_push_struct(&arena, Ast_PackageDecl);
  zero_struct(result, Ast_PackageDecl);
  result->kind = AST_PACKAGE_PROTOTYPE;
  int package_scope_level = scope_level;

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = syn_type_parameter_list();
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
        result->first_parameter = syn_parameter_list();

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
  {
    scope_pop_level(package_scope_level);
    next_token();
  }
  else
    error("at line %d: ';' expected, got '%s'", token_at->line_nr, token_at->lexeme);

  scope_pop_level(package_scope_level);
  return result;
}

internal Ast_PackageInstance*
syn_package_instance()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_PackageInstance* result = arena_push_struct(&arena, Ast_PackageInstance);
  zero_struct(result, Ast_PackageInstance);
  result->kind = AST_PACKAGE_INSTANCE;
  result->package = syn_expression(1);

  if (token_at->klass == TOK_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_var(result->name)))
      error("at line %d: variable '%s' has been previously declared", token_at->line_nr, result->name);
    result->var_ident = sym_add_var(result->name, (Ast*)result);

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

internal Ast_FunctionDecl*
syn_function_prototype()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_FunctionDecl* result = arena_push_struct(&arena, Ast_FunctionDecl);
  zero_struct(result, Ast_FunctionDecl);
  result->kind = AST_FUNCTION_PROTOTYPE;

  result->return_type_ident = sym_get_type(token_at->lexeme);
  result->return_type_ast = result->return_type_ident->ast;

  int function_scope_level = scope_level;
  next_token();

  if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
  {
    result->name = token_at->lexeme;

    if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
    result->type_ident = sym_add_type(result->name, (Ast*)result);

    scope_push_level();
    next_token();

    if (token_at->klass == TOK_ANGLE_OPEN)
    {
      next_token();

      if (token_is_type_parameter(token_at))
        result->first_type_parameter = syn_type_parameter_list();
      else
        error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_ANGLE_CLOSE)
        next_token();
      else
        error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
    }

    if (token_at->klass == TOK_PARENTH_OPEN)
    {
      sym_remove_error_kw();
      next_token();

      if (token_is_parameter(token_at))
        result->first_parameter = syn_parameter_list();

      if (token_at->klass == TOK_PARENTH_CLOSE)
      {
        sym_add_error_kw();
        next_token();
      }
      else
        error("at line %d: ')' expected, got '%s'", token_at->line_nr, token_at->lexeme);

      if (token_at->klass == TOK_SEMICOLON)
      {
        scope_pop_level(function_scope_level);
        next_token();
      }
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
syn_extern_object_prototype()
{
  Ast_FunctionDecl* method;

  assert(token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT);
  Ast_ExternObjectDecl* result = arena_push_struct(&arena, Ast_ExternObjectDecl);
  zero_struct(result, Ast_ExternObjectDecl);
  result->kind = AST_EXTERN_OBJECT_PROTOTYPE;

  result->name = token_at->lexeme;

  if (sym_ident_is_declared(sym_get_type(result->name)))
      error("at line %d: type '%s' has been previously declared", token_at->line_nr, result->name);
  result->type_ident = sym_add_type(result->name, (Ast*)result);

  int object_scope_level = scope_level;
  scope_push_level();
  next_token();

  if (token_at->klass == TOK_ANGLE_OPEN)
  {
    next_token();

    if (token_is_type_parameter(token_at))
      result->first_type_parameter = syn_type_parameter_list();
    else
      error("at line %d: identifier expected , got '%s'", token_at->line_nr, token_at->lexeme);

    if (token_at->klass == TOK_ANGLE_CLOSE)
      next_token();
    else
      error("at line %d: '>' expected, got '%s'", token_at->line_nr, token_at->lexeme);
  }

  if (token_at->klass == TOK_BRACE_OPEN)
  {
    sym_remove_error_kw();
    next_token();

    if (token_at->klass == TOK_TYPE_IDENT)
    {
      method = syn_function_prototype();
      result->first_method = method;
      while (token_at->klass == TOK_TYPE_IDENT)
      {
        Ast_FunctionDecl* next_method = syn_function_prototype();
        method->next_decl = (Ast_Declaration*)next_method;
        method = next_method;
      }
    }

    if (token_at->klass == TOK_BRACE_CLOSE)
    {
      sym_add_error_kw();
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
syn_extern_function_prototype()
{
  assert(token_at->klass == TOK_TYPE_IDENT);
  Ast_FunctionDecl* result = syn_function_prototype();
  result->kind = AST_EXTERN_FUNCTION_PROTOTYPE;
  return result;
}

internal Ast_Declaration*
syn_extern_decl()
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
      result = (Ast_Declaration*)syn_extern_object_prototype();
    }
    else if (token_at->klass == TOK_IDENT || token_at->klass == TOK_TYPE_IDENT)
    {
      rewind_token();
      result = (Ast_Declaration*)syn_extern_function_prototype();
    }
  }
  else
    error("at line %d: identifier expected, got '%s'", token_at->line_nr, token_at->lexeme);
  return result;
}

internal Ast_Declaration*
syn_p4declaration()
{
  Ast_Declaration* result = 0;
  assert(token_is_declaration(token_at));
  if (token_at->klass == TOK_KW_STRUCT)
    result = (Ast_Declaration*)syn_struct_decl();
  else if (token_at->klass == TOK_KW_HEADER)
    result = (Ast_Declaration*)syn_header_decl();
  else if (token_at->klass == TOK_KW_ERROR)
    result = (Ast_Declaration*)syn_error_type_decl();
  else if (token_at->klass == TOK_KW_TYPEDEF)
    result = (Ast_Declaration*)syn_typedef_decl();
  else if (token_at->klass == TOK_KW_PARSER)
    result = (Ast_Declaration*)syn_parser_decl();
  else if (token_at->klass == TOK_KW_CONTROL)
    result = (Ast_Declaration*)syn_control_decl();
  else if (token_at->klass == TOK_KW_ACTION)
    result = (Ast_Declaration*)syn_action_decl();
  else if (token_at->klass == TOK_KW_PACKAGE)
    result = (Ast_Declaration*)syn_package_prototype();
  else if (token_at->klass == TOK_KW_EXTERN)
    result = (Ast_Declaration*)syn_extern_decl();
  else if (token_at->klass == TOK_TYPE_IDENT)
    result = (Ast_Declaration*)syn_package_instance();
  else
    assert(false);
  return result;
}

internal Ast_P4Program*
syn_p4program()
{
  Ast_P4Program* result = 0;
  if (token_is_declaration(token_at))
  {
    result = arena_push_struct(&arena, Ast_P4Program);
    zero_struct(result, Ast_P4Program);
    result->kind = AST_P4PROGRAM;
    Ast_Declaration* declaration = syn_p4declaration();
    result->first_declaration = declaration;
    while (token_is_declaration(token_at))
    {
      Ast_Declaration* next_declaration = syn_p4declaration();
      declaration->next_decl = next_declaration;
      declaration = next_declaration;
    }
    if (token_at->klass != TOK_EOI)
      error("at line %d: expected end of input, got '%s'", token_at->line_nr, token_at->lexeme);
  }
  else
    error("at line %d: declaration expected, got '%s'", token_at->line_nr, token_at->lexeme);
  arena_print_usage(&arena, "Memory (syn_p4program): ");
  return result;
}

void
build_ast()
{
  error_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(error_type_ast, Ast_Ident);
  error_type_ast->kind = AST_TYPE_IDENT_EXPR;
  error_type_ast->name = "error";
  error_type_ast->is_builtin = true;

  error_var_ast = arena_push_struct(&arena, Ast_VarDecl);
  zero_struct(error_var_ast, Ast_VarDecl);
  error_var_ast->kind = AST_VAR_DECL;
  error_var_ast->name = "error";
  error_var_ast->is_builtin = true;

  void_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(void_type_ast, Ast_Ident);
  void_type_ast->kind = AST_TYPE_IDENT_EXPR;
  void_type_ast->name = "void";
  void_type_ast->is_builtin = true;

  bool_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(bool_type_ast, Ast_Ident);
  bool_type_ast->kind = AST_TYPE_IDENT_EXPR;
  bool_type_ast->name = "bool";
  bool_type_ast->is_builtin = true;

  bit_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(bit_type_ast, Ast_Ident);
  bit_type_ast->kind = AST_TYPE_IDENT_EXPR;
  bit_type_ast->name = "bit";
  bit_type_ast->is_builtin = true;

  varbit_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(varbit_type_ast, Ast_Ident);
  varbit_type_ast->kind = AST_TYPE_IDENT_EXPR;
  varbit_type_ast->name = "varbit";
  varbit_type_ast->is_builtin = true;

  int_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(int_type_ast, Ast_Ident);
  int_type_ast->kind = AST_TYPE_IDENT_EXPR;
  int_type_ast->name = "int";
  int_type_ast->is_builtin = true;

  string_type_ast = arena_push_struct(&arena, Ast_Ident);
  zero_struct(string_type_ast, Ast_Ident);
  string_type_ast->kind = AST_TYPE_IDENT_EXPR;
  string_type_ast->name = "string";
  string_type_ast->is_builtin = true;

  sym_init();

  int top_scope_level = scope_push_level();
  token_at = tokenized_input;
  next_token();
  p4program = syn_p4program();
  assert (scope_level == top_scope_level);
  scope_pop_level(top_scope_level - 1);
}
