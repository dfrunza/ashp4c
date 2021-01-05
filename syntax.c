#include "dp4c.h"
#include "lex.h"
#include "syntax.h"

external Arena arena;

external Token* tokenized_input;
internal Token* token = 0;
internal Token* prev_token = 0;
external int tokenized_input_len;

external Namespace_Entry** symtable;
external int max_symtable_len;
external int scope_level;

external Ast_P4Program* p4program;

Ident_Keyword* error_kw = 0;
Ident_Keyword* apply_kw = 0;

Ast_TypeIdent* error_type_ast = 0;
Ast_TypeIdent* void_type_ast = 0;
Ast_TypeIdent* bool_type_ast = 0;
Ast_TypeIdent* bit_type_ast = 0;
Ast_TypeIdent* varbit_type_ast = 0;
Ast_TypeIdent* int_type_ast = 0;
Ast_TypeIdent* string_type_ast = 0;
Ast_Integer* bool_true_ast = 0;
Ast_Integer* bool_false_ast = 0;

/*
Ident* error_type_ident = 0;
Ident* void_type_ident = 0;
Ident* bool_type_ident = 0;
Ident* bit_type_ident = 0;
Ident* varbit_type_ident;
Ident* int_type_ident;
Ident* string_type_ident;
Ident* bool_true_ident = 0;
Ident* bool_false_ident = 0;
*/

internal Ast_TypeExpression* build_type_expression();
internal Ast* build_expression(int priority_threshold);
internal Ast_BlockStmt* build_block_statement();
internal Ast* build_typeRef();
internal Ast* build_typedefDeclaration();

internal uint32_t
name_hash(char* name)
{
  uint32_t result = 0;
  uint32_t sum = 0;
  char* c = name;
  while (*c)
    sum += (uint32_t)(*c++);
  result = sum % max_symtable_len;
  return result;
}

int
scope_push_level()
{
  int new_scope_level = ++scope_level;
  printf("push scope %d\n", new_scope_level);
  return new_scope_level;
}

int
scope_pop_level(int to_level)
{
  if (scope_level <= to_level)
    return scope_level;

  int i = 0;
  while (i < max_symtable_len)
  {
    Namespace_Entry* ns = symtable[i];
    while (ns)
    {
      Ident* ident = ns->ns_global;
      if (ident && ident->scope_level > to_level)
      {
        ns->ns_global = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= to_level);
        ident->next_in_scope = 0;
      }
      ident = ns->ns_type;
      if (ident && ident->scope_level > to_level)
      {
        ns->ns_type = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= to_level);
        ident->next_in_scope = 0;
      }
      ns = ns->next;
    }
    i++;
  }
  printf("pop scope %d\n", to_level);
  scope_level = to_level;
  return scope_level;
}

bool
sym_ident_is_declared(Ident* ident)
{
  bool is_declared = (ident && ident->scope_level >= scope_level);
  return is_declared;
}

internal Namespace_Entry*
sym_get_namespace(char* name)
{
  uint32_t h = name_hash(name);
  Namespace_Entry* name_info = symtable[h];
  while(name_info)
  {
    if (cstr_match(name_info->name, name))
      break;
    name_info = name_info->next;
  }
  if (!name_info)
  {
    name_info = arena_push_struct(&arena, Namespace_Entry);
    name_info->name = name;
    name_info->next = symtable[h];
    symtable[h] = name_info;
  }
  return name_info;
}

Ident*
sym_get_var(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident_var = (Ident*)ns->ns_global;
  if (ident_var)
    assert (ident_var->ident_kind == ID_VAR);
  return ident_var;
}

Ident*
sym_new_var(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_VAR;
  ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)ident;
  printf("new var '%s'\n", ident->name);
  return ident;
}

void
sym_import_var(Ident* var_ident)
{
  Namespace_Entry* ns = sym_get_namespace(var_ident->name);

  if (ns->ns_global)
  {
    assert (ns->ns_global == (Ident*)var_ident);
    return;
  }

  var_ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)var_ident;
}

void
sym_unimport_var(Ident* var_ident)
{
  Namespace_Entry* ns = sym_get_namespace(var_ident->name);

  if (!ns->ns_global)
    return;

  assert (ns->ns_global == (Ident*)var_ident);
  ns->ns_global = ns->ns_global->next_in_scope;
}

Ident*
sym_get_type(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* result = (Ident*)ns->ns_type;
  if (result)
    assert (result->ident_kind == ID_TYPE || result->ident_kind == ID_TYPEVAR);
  return result;
}

Ident*
sym_new_type(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPE;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("new type '%s'\n", ident->name);
  return ident;
}

Ident*
sym_new_typevar(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPEVAR;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("new typevar '%s'\n", ident->name);
  return ident;
}

void
sym_import_type(Ident* type_ident)
{
  Namespace_Entry* ns = sym_get_namespace(type_ident->name);

  if (ns->ns_type)
  {
    assert (ns->ns_type == (Ident*)type_ident);
    return;
  }

  type_ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)type_ident;
}

void
sym_unimport_type(Ident* type_ident)
{
  Namespace_Entry* ns = sym_get_namespace(type_ident->name);

  if (!ns->ns_type)
    return;

  assert (ns->ns_type == (Ident*)type_ident);
  ns->ns_type = ns->ns_type->next_in_scope;
}

internal Ident_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  Namespace_Entry* namespace = sym_get_namespace(name);
  assert (namespace->ns_global == 0);
  Ident_Keyword* ident = arena_push_struct(&arena, Ident_Keyword);
  ident->name = name;
  ident->scope_level = scope_level;
  ident->token_klass = token_klass;
  ident->ident_kind = ID_KEYWORD;
  namespace->ns_global = (Ident*)ident;
  return ident;
}

internal Token*
next_token()
{
  assert(token < tokenized_input + tokenized_input_len);
  prev_token = token++;
  while (token->klass == TOK_COMMENT)
    token++;

  if (token->klass == TOK_IDENTIFIER)
  {
    Namespace_Entry* ns = sym_get_namespace(token->lexeme);
    if (ns->ns_global)
    {
      Ident* ident = ns->ns_global;
      if (ident->ident_kind == ID_KEYWORD)
      {
        token->klass = ((Ident_Keyword*)ident)->token_klass;
        return token;
      }
    }

    if (ns->ns_type)
    {
      Ident* ident = ns->ns_type;
      if (ident->ident_kind == ID_TYPE || ident->ident_kind == ID_TYPEVAR)
      {
        token->klass = TOK_TYPE_IDENTIFIER;
        return token;
      }
    }
  }
  return token;
}

internal Token*
peek_token()
{
  prev_token = token;
  Token* peek_token = next_token();
  token = prev_token;
  return peek_token;
}

internal void
copy_tokenattr_to_ast(Token* token, Ast* ast)
{
  ast->line_nr = token->line_nr;
  ast->lexeme = token->lexeme;
}

internal void
expect_semicolon()
{
  if (token->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
}

internal bool
token_is_direction(Token* token)
{
  bool result = token->klass == TOK_IN || token->klass == TOK_OUT || token->klass == TOK_INOUT;
  return result;
}

internal bool
token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token->klass == TOK_TYPE_IDENTIFIER || token->klass == TOK_INTEGER;
  return result;
}

internal Ast_TypeExpression*
build_type_parameter_list()
{
  assert(token->klass == TOK_ANGLE_OPEN);
  Ast_TypeExpression* result = 0;
  next_token();

  if (token->klass == TOK_TYPE_IDENTIFIER)
  {
    Ast_TypeExpression* argument = build_type_expression();
    result = argument;

    while (token->klass == TOK_COMMA)
    {
      next_token();

      if (token->klass == TOK_TYPE_IDENTIFIER)
      {
        Ast_TypeExpression* next_argument = build_type_expression();
        argument->next_argument = next_argument;
        argument = next_argument;
      }
      else if (token->klass == TOK_COMMA)
        error("at line %d: missing type argument", token->line_nr);
      else
        error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
    }
  }

  if (token->klass == TOK_ANGLE_CLOSE)
    next_token();
  else
    error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);

  return result;
}

internal Ast_TypeExpression*
build_type_expression()
{
  assert(token->klass == TOK_TYPE_IDENTIFIER);
  Ast_TypeExpression* result = arena_push_struct(&arena, Ast_TypeExpression);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_TYPE_EXPRESSION;

  if (token->klass == TOK_TYPE_IDENTIFIER)
  {
    // TODO: Ast_Ident
    result->argument_kind = AST_TYPPARAM_VAR;
    result->name = token->lexeme;
    next_token();
  }
  else if (token->klass == TOK_INTEGER)
  {
    result->argument_kind = AST_TYPPARAM_INT;

    Ast* size_ast = build_expression(1);
    result->type_ast = size_ast;
    if (size_ast->kind != AST_INTEGER)
      error("at line %d: type width must be a integer literal, got '%s'", size_ast->line_nr, size_ast->lexeme);
  }
  else
    assert(false);

  if (token->klass == TOK_ANGLE_OPEN)
    result->first_type_argument = build_type_parameter_list();

  return result;
}

internal Ast_StructField*
build_struct_field()
{
  assert(token->klass == TOK_TYPE_IDENTIFIER);
  Ast_StructField* result = arena_push_struct(&arena, Ast_StructField);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_STRUCT_FIELD;
  result->member_type = build_type_expression();

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();
    expect_semicolon();
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

/*
internal Ast_HeaderDecl*
build_header_decl()
{
  Ast_StructField* field;

  assert(token->klass == TOK_HEADER);
  next_token();
  Ast_HeaderDecl* result = arena_push_struct(&arena, Ast_HeaderDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_HEADER_PROTOTYPE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_HEADER_DECL;
      next_token();

      if (token->klass == TOK_TYPE_IDENTIFIER)
      {
        field = build_struct_field();
        result->first_field = field;
        while (token->klass == TOK_TYPE_IDENTIFIER)
        {
          Ast_StructField* next_field = build_struct_field();
          field->next_field = next_field;
          field = next_field;
        }
      }

      if (token->klass == TOK_BRACE_CLOSE)
        next_token(token);
      else if (token->klass == TOK_IDENTIFIER)
        error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
      else
        error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}
*/

internal Ast_StructDecl*
build_struct_decl()
{
  assert(token->klass == TOK_STRUCT);
  next_token();
  Ast_StructDecl* result = arena_push_struct(&arena, Ast_StructDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_STRUCT_PROTOTYPE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_BRACE_OPEN)
    {
      result->kind = AST_STRUCT_DECL;
      next_token();

      if (token->klass == TOK_TYPE_IDENTIFIER)
      {
        Ast_StructField* field = build_struct_field();
        result->first_field = field;
        while (token->klass == TOK_TYPE_IDENTIFIER)
        {
          Ast_StructField* next_field = build_struct_field();
          field->next_field = next_field;
          field = next_field;
        }
      }

      if (token->klass == TOK_BRACE_CLOSE)
        next_token();
      else if (token->klass == TOK_IDENTIFIER)
        error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
      else
        error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);

  return result;
}

internal Ast_ErrorCode*
build_error_code()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_ErrorCode* code = arena_push_struct(&arena, Ast_ErrorCode);
  copy_tokenattr_to_ast(token, (Ast*)code);
  code->kind = AST_ERROR_CODE;
  code->name = token->lexeme;
  next_token();
  return code;
}

internal Ast_ErrorType*
build_errorDeclaration()
{
  assert(token->klass == TOK_ERROR);
  next_token();
  Ast_ErrorType* result = arena_push_struct(&arena, Ast_ErrorType);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_ERROR_TYPE;
  result->line_nr = token->line_nr;

  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    if (token->klass == TOK_IDENTIFIER)
    {
      Ast_ErrorCode* field = build_error_code();
      result->error_code = field;
      while (token->klass == TOK_COMMA)
      {
        next_token();
        if (token->klass == TOK_IDENTIFIER)
        {
          Ast_ErrorCode* next_code = build_error_code();
          field->next_code = next_code;
          field = next_code;
        }
        else if (token->klass == TOK_COMMA)
          error("at line %d: missing parameter", token->line_nr);
        else
          error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
      }
    }

    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_Typedef*
build_typedef_decl()
{
  assert(token->klass == TOK_TYPEDEF);
  next_token();
  Ast_Typedef* result = arena_push_struct(&arena, Ast_Typedef);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_TYPEDEF;

  if (token->klass == TOK_TYPE_IDENTIFIER)
  {
    result->type = build_type_expression();
    if (token->klass == TOK_TYPE_IDENTIFIER)
    {
      // TODO: Ast_Ident
      result->name = token->lexeme;
      next_token();
    }
    else
      error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d : unexpected token '%s'", token->line_nr, token->lexeme);

  expect_semicolon();
  return result;
}

internal enum Ast_ParameterDirection
build_direction()
{
  assert(token_is_direction(token));
  enum Ast_ParameterDirection result = 0;
  if (token->klass == TOK_IN)
    result = AST_DIR_IN;
  else if (token->klass == TOK_OUT)
    result = AST_DIR_OUT;
  else if (token->klass == TOK_INOUT)
    result = AST_DIR_INOUT;
  next_token();
  return result;
}

internal Ast_Parameter*
build_parameter()
{
  assert(token_is_parameter(token));
  Ast_Parameter* result = arena_push_struct(&arena, Ast_Parameter);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_PARAMETER;

  if (token_is_direction(token))
    result->direction = build_direction();

  if (token->klass == TOK_TYPE_IDENTIFIER)
    result->param_type = build_type_expression();
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);

  if (token->klass == TOK_IDENTIFIER)
  {
    // TODO: Ast_Ident
    result->name = token->lexeme;
    next_token();
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_Parameter*
build_parameter_list()
{
  assert(token_is_parameter(token));

  Ast_Parameter* parameter = build_parameter();
  Ast_Parameter* result = parameter;

  while (token->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_parameter(token))
    {
      Ast_Parameter* next_parameter = build_parameter();
      parameter->next_parameter = next_parameter;
      parameter = next_parameter;
    }
    else if (token->klass == TOK_COMMA)
      error("at line %d: missing parameter", token->line_nr);
    else
      error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
  }
  return result;
}

internal Ast_ParserDecl*
build_parser_prototype()
{
  assert(token->klass == TOK_PARSER);
  next_token();
  Ast_ParserDecl* result = arena_push_struct(&arena, Ast_ParserDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_PARSER_PROTOTYPE;

  if (token->klass == TOK_IDENTIFIER)
  {
    // TODO: Ast_Ident
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_PARENTH_OPEN)
    {
      next_token();
      if (token_is_parameter(token))
        result->first_parameter = build_parameter_list();

      if (token->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token->klass == TOK_IDENTIFIER)
          error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
      error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
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
  bool result = token->klass == TOK_IDENTIFIER || token->klass == TOK_TYPE_IDENTIFIER \
    || token_is_integer(token) || token_is_winteger(token) || token_is_sinteger(token) \
    || token->klass == TOK_STRING || token->klass == TOK_PARENTH_OPEN;
  return result;
}

internal bool
token_is_expression_operator(Token* token)
{
  bool result = token->klass == TOK_DOTPREFIX || token->klass == TOK_EQUAL_EQUAL
    || token->klass == TOK_PARENTH_OPEN || token->klass == TOK_EQUAL
    || token->klass == TOK_MINUS || token->klass == TOK_PLUS;
  return result;
}

internal Ast*
build_expression_primary()
{
  Ast* result = 0;

  assert(token_is_expression(token));

  if (token->klass == TOK_IDENTIFIER)
  {
    Ast_Ident* ident_ast = arena_push_struct(&arena, Ast_Ident);
    copy_tokenattr_to_ast(token, (Ast*)ident_ast);
    ident_ast->kind = AST_IDENT;
    ident_ast->name = token->lexeme;
    result = (Ast*)ident_ast;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      ident_ast->first_type_argument = build_type_parameter_list();
  }
  else if (token->klass == TOK_TYPE_IDENTIFIER)
  {
    Ast_TypeIdent* ident_ast = arena_push_struct(&arena, Ast_TypeIdent);
    copy_tokenattr_to_ast(token, (Ast*)ident_ast);
    ident_ast->kind = AST_TYPE_IDENT;
    ident_ast->name = token->lexeme;
    result = (Ast*)ident_ast;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      ident_ast->first_type_argument = build_type_parameter_list();
  }
  else if (token_is_integer(token))
  {
    Ast_Integer* expression = arena_push_struct(&arena, Ast_Integer);
    copy_tokenattr_to_ast(token, (Ast*)expression);
    expression->kind = AST_INTEGER;
    expression->lexeme = token->lexeme;
    expression->value = 0; //TODO

    result = (Ast*)expression;
    next_token();
  }
  else if (token_is_winteger(token))
  {
    Ast_WInteger* expression = arena_push_struct(&arena, Ast_WInteger);
    copy_tokenattr_to_ast(token, (Ast*)expression);
    expression->kind = AST_WINTEGER;
    expression->lexeme = token->lexeme;
    expression->value = 0; //TODO

    result = (Ast*)expression;
    next_token();
  }
  else if (token_is_sinteger(token))
  {
    Ast_SInteger* expression = arena_push_struct(&arena, Ast_SInteger);
    copy_tokenattr_to_ast(token, (Ast*)expression);
    expression->kind = AST_SINTEGER;
    expression->lexeme = token->lexeme;
    expression->value = 0; //TODO

    result = (Ast*)expression;
    next_token();
  }
  else if (token->klass == TOK_PARENTH_OPEN)
  {
    next_token();

    if (token_is_expression(token))
      result = build_expression(1);
    if (token->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    assert(false);
  return result;
}

internal enum Ast_ExprOperator
build_expression_operator()
{
  assert(token_is_expression_operator(token));
  enum Ast_ExprOperator result = 0;

  switch (token->klass)
  {
    case TOK_DOTPREFIX:
      result = AST_OP_MEMBER_SELECTOR;
      break;
    case TOK_EQUAL:
      result = AST_OP_ASSIGN;
      break;
    case TOK_EQUAL_EQUAL:
      result = AST_OP_LOGIC_EQUAL;
      break;
    case TOK_PARENTH_OPEN:
      result = AST_OP_FUNCTION_CALL;
      break;
    case TOK_MINUS:
      result = AST_OP_SUBTRACT;
      break;
    case TOK_PLUS:
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
    case AST_OP_ASSIGN:
      result = 1;
      break;
    case AST_OP_LOGIC_EQUAL:
      result = 2;
      break;
    case AST_OP_ADDITION:
    case AST_OP_SUBTRACT:
      result = 3;
      break;
    case AST_OP_FUNCTION_CALL:
      result = 4;
      break;
    case AST_OP_MEMBER_SELECTOR:
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

internal Ast*
build_expression(int priority_threshold)
{
  assert(token_is_expression(token));
  Ast* expr = build_expression_primary();

  while (token_is_expression_operator(token))
  {
    enum Ast_ExprOperator op = build_expression_operator();
    int priority = op_get_priority(op);

    if (priority >= priority_threshold)
    {
      next_token();

      if (op_is_binary(op))
      {
        Ast_BinaryExpr* binary_expr = arena_push_struct(&arena, Ast_BinaryExpr);
        copy_tokenattr_to_ast(token, (Ast*)binary_expr);
        binary_expr->kind = AST_BINARY_EXPR;
        binary_expr->l_operand = expr;
        binary_expr->op = op;

        if (token_is_expression(token))
          binary_expr->r_operand = build_expression(priority_threshold + 1);
        else
          error("at line %d: expression term expected, got '%s'", token->line_nr, token->lexeme);
        expr = (Ast*)binary_expr;
      }
      else if (op == AST_OP_FUNCTION_CALL)
      {
        Ast_FunctionCall* function_call = arena_push_struct(&arena, Ast_FunctionCall);
        copy_tokenattr_to_ast(token, (Ast*)function_call);
        function_call->kind = AST_FUNCTION_CALL;
        function_call->function = expr;

        if (token_is_expression(token))
        {
          Ast_Declaration* argument = (Ast_Declaration*)build_expression(1);
          function_call->first_argument = argument;
          while (token->klass == TOK_COMMA)
          {
            next_token();
            if (token_is_expression(token))
            {
              Ast_Declaration* next_argument = (Ast_Declaration*)build_expression(1);
              argument->next_decl = next_argument;
              argument = next_argument;
            }
            else if (token->klass == TOK_COMMA)
              error("at line %d: missing argument", token->line_nr);
            else
              error("at line %d: expression term expected, got '%s'", token->line_nr, token->lexeme);
          }
        }

        if (token->klass == TOK_PARENTH_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);

        expr = (Ast*)function_call;
      }
      else assert(false);
    }
    else break;
  }
  return expr;
}

internal Ast_VarDecl*
build_var_decl()
{
  Ast_Ident* name_ast = 0;
  Ast* init_expr = 0;

  assert(token->klass == TOK_VAR || token->klass == TOK_CONST);
  next_token();
  Ast_VarDecl* var_decl = arena_push_struct(&arena, Ast_VarDecl);
  copy_tokenattr_to_ast(token, (Ast*)var_decl);
  var_decl->kind = AST_VAR_DECL;
  var_decl->var_type = build_type_expression();
  Token* name_token = token;

  if (token_is_expression(token))
  {
    init_expr = build_expression(1);
    if (!init_expr)
      error("at line %d: identifier expected, got '%s'", name_token->line_nr, name_token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);

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
  expect_semicolon();
  return var_decl;
}

internal Ast_IdentState*
build_ident_state()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_IdentState* result = arena_push_struct(&arena, Ast_IdentState);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_IDENT_STATE;
  result->name = token->lexeme;
  next_token();
  expect_semicolon();
  return result;
}

internal bool
token_is_select_case(Token* token)
{
  bool result = token_is_expression(token) || token->klass == TOK_DEFAULT;
  return result;
}

internal Ast_SelectCase*
build_select_case()
{
  assert(token_is_select_case(token));
  Ast_SelectCase* result = 0;

  if (token_is_expression(token))
  {
    Ast_SelectCase_Expr* select_expr = arena_push_struct(&arena, Ast_SelectCase_Expr);
    copy_tokenattr_to_ast(token, (Ast*)select_expr);
    select_expr->kind = AST_SELECT_CASE_EXPR;
    select_expr->key_expr = build_expression(1);
    result = (Ast_SelectCase*)select_expr;
  }
  else if (token->klass = TOK_DEFAULT)
  {
    next_token();
    Ast_SelectCase_Default* default_select = arena_push_struct(&arena, Ast_SelectCase_Default);
    copy_tokenattr_to_ast(token, (Ast*)default_select);
    default_select->kind = AST_DEFAULT_SELECT_CASE;
    result = (Ast_SelectCase*)default_select;
  }
  else
    assert(false);

  if (token->klass == TOK_COLON)
  {
    next_token();
    if (token->klass == TOK_IDENTIFIER)
    {
      result->end_state = token->lexeme;
      next_token();
      expect_semicolon();
    }
    else
      error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: ':' expected, got '%s'", token->line_nr, token->lexeme);

  return result;
}

internal Ast_SelectCase*
build_select_case_list()
{
  assert(token_is_select_case(token));
  Ast_SelectCase* select_case = build_select_case();
  Ast_SelectCase* result = select_case;
  while (token_is_select_case(token))
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
  assert(token->klass == TOK_SELECT);
  next_token();
  Ast_SelectState* result = arena_push_struct(&arena, Ast_SelectState);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_SELECT_STATE;

  if (token->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token))
      result->expression = build_expression(1);
    if (token->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);

  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    result->select_case = build_select_case_list();
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_TransitionStmt*
build_transition_stmt()
{
  assert(token->klass == TOK_TRANSITION);
  Ast_TransitionStmt* result = arena_push_struct(&arena, Ast_TransitionStmt);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_TRANSITION_STMT;
  next_token();

  if (token->klass == TOK_IDENTIFIER)
    result->state_expr = (Ast*)build_ident_state();
  else if (token->klass == TOK_SELECT)
    result->state_expr = (Ast*)build_select_state();
  else
    error("at line %d: transition stmt expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal bool
token_is_statement(Token* token)
{
  bool result = token_is_expression(token) || token->klass == TOK_VAR \
                || token->klass == TOK_BRACE_OPEN;
  return result;
}

internal Ast_Declaration*
build_statement()
{
  Ast_Declaration* result = 0;
  assert (token_is_statement(token));

  if (token_is_expression(token))
  {
    result = (Ast_Declaration*)build_expression(1);
    expect_semicolon();
  }
  else if (token->klass == TOK_VAR)
    result = (Ast_Declaration*)build_var_decl();
  else if (token->klass == TOK_BRACE_OPEN)
    result = (Ast_Declaration*)build_block_statement();
  else
    assert(false);

  return result;
}

internal Ast_Declaration*
build_statement_list()
{
  Ast_Declaration* result = 0;

  if (token_is_statement(token))
  {
    Ast_Declaration* expression = (Ast_Declaration*)build_statement();
    result = expression;

    while (token_is_statement(token))
    {
      Ast_Declaration* next_expression = (Ast_Declaration*)build_statement();
      expression->next_decl = next_expression;
      expression = next_expression;
    }
  }

  return result;
}

internal Ast_ParserState*
build_parser_state()
{
  assert(token->klass == TOK_STATE);
  next_token();
  Ast_ParserState* result = arena_push_struct(&arena, Ast_ParserState);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_PARSER_STATE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    sym_unimport_var((Ident*)apply_kw);
    next_token();

    if (token->klass == TOK_BRACE_OPEN)
    {
      next_token();
      result->first_statement = build_statement_list();

      if (token->klass == TOK_TRANSITION)
        result->transition_stmt = build_transition_stmt();
      else
        error("at line %d: 'transition' expected, got '%s'", token->line_nr, token->lexeme);

      if (token->klass == TOK_BRACE_CLOSE)
        next_token();
      else
        error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);

    sym_import_var((Ident*)apply_kw);
  }
  return result;
}

internal bool
token_is_parser_local_decl(Token* token)
{
  bool result = token->klass == TOK_STATE || token->klass == TOK_VAR \
                || token->klass == TOK_IDENTIFIER || token->klass == TOK_TYPE_IDENTIFIER;
  return result;
}

internal Ast_Declaration*
build_parser_local_decl()
{
  Ast_Declaration* result = 0;
  assert(token_is_parser_local_decl(token));
  
  if (token->klass == TOK_STATE)
  {
    Ast_ParserState* state_decl = build_parser_state();
    result = (Ast_Declaration*)state_decl;
  }
  else if (token->klass == TOK_VAR)
  {
    Ast_VarDecl* var_decl = build_var_decl();
    result = (Ast_Declaration*)var_decl;
  }
  else if (token->klass == TOK_IDENTIFIER)
  {
    Ast_Declaration* expression = (Ast_Declaration*)build_expression(1);
    result = (Ast_Declaration*)expression;

    expect_semicolon();
  }
  else
    assert(false);

  return result;
}

internal Ast_ParserDecl*
build_parserDeclaration()
{
  assert(token->klass == TOK_PARSER);
  Ast_ParserDecl* result = build_parser_prototype();

  if (token->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_PARSER_DECL;
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token_is_parser_local_decl(token))
    {
      Ast_Declaration* local_decl = build_parser_local_decl();
      result->first_local_decl = local_decl;
      while (token_is_parser_local_decl(token))
      {
        Ast_Declaration* next_local_decl = build_parser_local_decl();
        local_decl->next_decl = next_local_decl;
        local_decl = next_local_decl;
      }
    }

    if (token->klass == TOK_BRACE_CLOSE)
    {
      sym_import_var((Ident*)error_kw);
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else if (token->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: '{' or ';' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_ControlDecl*
build_control_prototype()
{
  assert(token->klass == TOK_CONTROL);
  next_token();
  Ast_ControlDecl* result = arena_push_struct(&arena, Ast_ControlDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_CONTROL_PROTOTYPE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_PARENTH_OPEN)
    {
      next_token();

      if (token_is_parameter(token))
        result->first_parameter = build_parameter_list();

      if (token->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token->klass == TOK_IDENTIFIER)
          error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_BlockStmt*
build_block_statement()
{
  assert(token->klass == TOK_BRACE_OPEN);
  next_token();
  Ast_BlockStmt* result = arena_push_struct(&arena, Ast_BlockStmt);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_BLOCK_STMT;

  if (token_is_statement(token))
    result->first_statement = build_statement_list();

  if (token->klass == TOK_BRACE_CLOSE)
    next_token();
  else
    error("at line %d: statement expected, got '%s'", token->line_nr, token->lexeme);

  return result;
}

bool
token_is_control_local_decl(Token* token)
{
  bool result = token->klass == TOK_ACTION || token->klass == TOK_TABLE \
                || token->klass == TOK_IDENTIFIER || token->klass == TOK_VAR;
  return result;
}

/*
internal Ast_ActionDecl*
build_actionDeclaration()
{
  assert(token->klass == TOK_ACTION);

  next_token();

  Ast_ActionDecl* result = arena_push_struct(&arena, Ast_ActionDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_ACTION;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_PARENTH_OPEN)
    {
      next_token();
      if (token_is_parameter(token))
        result->parameter = build_parameter_list();
      if (token->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
        error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);

    if (token->klass == TOK_BRACE_OPEN)
      result->action_body = build_block_statement();
    else
      error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_Key*
build_key_elem()
{
  assert(token_is_expression(token));

  Ast_Key* result = arena_push_struct(&arena, Ast_Key);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_TABLE_KEY;
  result->expression = build_expression(1);

  if (token->klass == TOK_COLON)
  {
    next_token();
    if (token->klass == TOK_IDENTIFIER)
    {
      result->name = build_expression(1);
      expect_semicolon();
    }
    else
      error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: ':' expected, got '%s'", token->line_nr, token->lexeme);

  return result;
}

internal Ast_SimpleProp*
build_simple_prop()
{
  assert(token_is_expression(token));

  Ast_SimpleProp* result = arena_push_struct(&arena, Ast_SimpleProp);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_SIMPLE_PROP;
  result->expression = build_expression(1);
  expect_semicolon();

  return result;
}

internal Ast_ActionRef*
build_action_ref()
{
  assert(token->klass == TOK_IDENTIFIER);

  Ast_ActionRef* result = arena_push_struct(&arena, Ast_ActionRef);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_ACTION_REF;
  result->name = token->lexeme;

  next_token();

  if (token->klass == TOK_PARENTH_OPEN)
  {
    next_token();
    if (token_is_expression(token))
    {
      Ast_Declaration* argument = (Ast_Declaration*)build_expression(1);
      result->first_argument = argument;
      while (token->klass == TOK_COMMA)
      {
        next_token();
        if (token_is_expression(token))
        {
          Ast_Declaration* next_argument = (Ast_Declaration*)build_expression(1);
          argument->next_decl = next_argument;
          argument = next_argument;
        }
        else if (token->klass == TOK_COMMA)
          error("at line %d: missing parameter", token->line_nr);
        else
          error("at line %d: expression term expected, got '%s'", token->line_nr, token->lexeme);
      }
    }
    if (token->klass == TOK_PARENTH_CLOSE)
      next_token();
    else
      error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);

  expect_semicolon();

  return result;
}

internal Ast_TableProperty*
build_table_property()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_TableProperty* result = 0;
  // FIXME: WTF is this nonsense?
  if (cstr_match(token->lexeme, "key"))
  {
    next_token();
    if (token->klass == TOK_EQUAL)
    {
      next_token();
      if (token->klass == TOK_BRACE_OPEN)
      {
        next_token();
        if (token_is_expression(token))
        {
          Ast_Key* key_elem = build_key_elem();
          result = (Ast_TableProperty*)key_elem;
          while (token_is_expression(token))
          {
            Ast_Key* next_key_elem = build_key_elem();
            key_elem->next_key = next_key_elem;
            key_elem = next_key_elem;
          }
        }
        else
          error("at line %d: expression term expected, got '%s'", token->line_nr, token->lexeme);
        if (token->klass == TOK_BRACE_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
      }
      else
        error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '=' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else if (cstr_match(token->lexeme, "actions"))
  {
    next_token();
    if (token->klass == TOK_EQUAL)
    {
      next_token();
      if (token->klass == TOK_BRACE_OPEN)
      {
        next_token();
        if (token->klass == TOK_IDENTIFIER)
        {
          Ast_ActionRef* action_ref = build_action_ref();
          result = (Ast_TableProperty*)action_ref;
          while (token->klass == TOK_IDENTIFIER)
          {
            Ast_ActionRef* next_action_ref = build_action_ref();
            action_ref->next_action = next_action_ref;
            action_ref = next_action_ref;
          }
        }
        if (token->klass == TOK_BRACE_CLOSE)
          next_token();
        else
          error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
      }
      else
        error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '=' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
  {
    Ast_SimpleProp* prop = build_simple_prop();
    result = (Ast_TableProperty*)prop;
    while (token->klass == TOK_IDENTIFIER)
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
  assert(token->klass == TOK_TABLE);

  next_token();

  Ast_TableDecl* result = arena_push_struct(&arena, Ast_TableDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_TABLE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();
    if (token->klass == TOK_BRACE_OPEN)
    {
      next_token();
      if (token->klass == TOK_IDENTIFIER)
      {
        Ast_TableProperty* property = build_table_property();
        result->property = property;
        while (token->klass == TOK_IDENTIFIER)
        {
          Ast_TableProperty* next_property = build_table_property();
          property->next = next_property;
          property = next_property;
        }
      }
      else
        error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
      if (token->klass == TOK_BRACE_CLOSE)
        next_token();
      else
        error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}
*/

internal Ast_Declaration*
build_control_local_decl()
{
  Ast_Declaration* result = 0;
  assert(token_is_control_local_decl(token));

  switch (token->klass)
  {
    /*
    case TOK_ACTION:
    {
      Ast_ActionDecl* action_decl = build_action_decl();
      result = (Ast_Declaration*)action_decl;
    } break;
    case TOK_TABLE:
    {
      Ast_TableDecl* table_decl = build_table_decl();
      result = (Ast_Declaration*)table_decl;
    } break;
    */
    case TOK_VAR:
    {
      Ast_VarDecl* var_decl = build_var_decl();
      result = (Ast_Declaration*)var_decl;
    } break;
    case TOK_IDENTIFIER:
    case TOK_TYPE_IDENTIFIER:
    {
      Ast_Declaration* expression = (Ast_Declaration*)build_expression(1);
      result = (Ast_Declaration*)expression;
      expect_semicolon();
    } break;
    default:
      assert(false);
  }

  return result;
}

internal Ast_ControlDecl*
build_controlDeclaration()
{
  assert(token->klass == TOK_CONTROL);
  Ast_ControlDecl* result = build_control_prototype();

  if (token->klass == TOK_BRACE_OPEN)
  {
    result->kind = AST_CONTROL_DECL;
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token_is_control_local_decl(token))
    {
      Ast_Declaration* local_decl = build_control_local_decl();
      result->first_local_decl = local_decl;
      while (token_is_control_local_decl(token))
      {
        Ast_Declaration* next_local_decl = build_control_local_decl(result);
        local_decl->next_decl = next_local_decl;
        local_decl = next_local_decl;
      }
    }

    if (token->klass == TOK_APPLY)
    {
      sym_unimport_var((Ident*)apply_kw);
      next_token();

      if (token->klass == TOK_BRACE_OPEN)
        result->apply_block = build_block_statement();
      else
        error("at line %d: '{' expected, got '%s'", token->line_nr, token->lexeme);

      sym_import_var((Ident*)apply_kw);
    }

    if (token->klass == TOK_BRACE_CLOSE)
    {
      sym_import_var((Ident*)error_kw);
      next_token();
    }
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else if (token->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: '{' or ';' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_PackageDecl*
build_package_prototype()
{
  assert(token->klass == TOK_PACKAGE);
  next_token();
  Ast_PackageDecl* result = arena_push_struct(&arena, Ast_PackageDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_PACKAGE_PROTOTYPE;

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_PARENTH_OPEN)
    {
      next_token();

      if (token_is_parameter(token))
        result->first_parameter = build_parameter_list();

      if (token->klass == TOK_PARENTH_CLOSE)
        next_token();
      else
      {
        if (token->klass == TOK_IDENTIFIER)
          error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
        else
          error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);
      }
    }
    else
      error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);

  if (token->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_PackageInstantiation*
build_package_instantiation()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_PackageInstantiation* result = arena_push_struct(&arena, Ast_PackageInstantiation);
  copy_tokenattr_to_ast(token, (Ast*)result);
  zero_struct(result, Ast_PackageInstantiation);
  result->kind = AST_PACKAGE_INSTANTIATION;
  result->package_ctor = build_expression(1);

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();
  }
  else
    error("at line %d: non-type identifier expected, got '%s'", token->line_nr, token->lexeme);
  expect_semicolon();
  return result;
}

internal Ast_FunctionDecl*
build_function_prototype()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_FunctionDecl* result = arena_push_struct(&arena, Ast_FunctionDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_FUNCTION_PROTOTYPE;

  Token* lookahead_token = peek_token();
  if (lookahead_token->klass == TOK_IDENTIFIER)
    result->return_type = build_type_expression();

  if (token->klass == TOK_IDENTIFIER)
  {
    result->name = token->lexeme;
    next_token();

    if (token->klass == TOK_ANGLE_OPEN)
      result->first_type_parameter = build_type_parameter_list();

    if (token->klass == TOK_PARENTH_OPEN)
    {
      sym_unimport_var((Ident*)error_kw);
      next_token();

      if (token_is_parameter(token))
        result->first_parameter = build_parameter_list();

      if (token->klass == TOK_PARENTH_CLOSE)
      {
        sym_import_var((Ident*)error_kw);
        next_token();
      }
      else
        error("at line %d: ')' expected, got '%s'", token->line_nr, token->lexeme);

      if (token->klass == TOK_SEMICOLON)
        next_token();
      else
        error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
    }
    else
      error("at line %d: '(' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: identifier expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_ExternObjectDecl*
build_extern_object_prototype()
{
  Ast_FunctionDecl* method;
  assert(token->klass == TOK_IDENTIFIER);
  Ast_ExternObjectDecl* result = arena_push_struct(&arena, Ast_ExternObjectDecl);
  copy_tokenattr_to_ast(token, (Ast*)result);
  result->kind = AST_EXTERN_OBJECT_PROTOTYPE;
  result->name = token->lexeme;
  next_token();

  if (token->klass == TOK_ANGLE_OPEN)
    result->first_type_parameter = build_type_parameter_list();

  if (token->klass == TOK_BRACE_OPEN)
  {
    sym_unimport_var((Ident*)error_kw);
    next_token();

    if (token->klass == TOK_IDENTIFIER)
    {
      method = build_function_prototype();
      result->first_method = method;

      while (token->klass == TOK_IDENTIFIER)
      {
        Ast_FunctionDecl* next_method = build_function_prototype();
        method->next_decl = (Ast_Declaration*)next_method;
        method = next_method;
      }
    }

    sym_import_var((Ident*)error_kw);
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else if (token->klass == TOK_IDENTIFIER)
      error("at line %d: unknown type '%s'", token->line_nr, token->lexeme);
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else if (token->klass == TOK_SEMICOLON)
    next_token();
  else
    error("at line %d: '{' or ';' expected, got '%s'", token->line_nr, token->lexeme);
  return result;
}

internal Ast_FunctionDecl*
build_extern_function_prototype()
{
  assert(token->klass == TOK_IDENTIFIER);
  Ast_FunctionDecl* result = build_function_prototype();
  result->kind = AST_EXTERN_FUNCTION_PROTOTYPE;
  return result;
}

internal bool
token_is_baseType(Token* token)
{
  bool result = token->klass == TOK_BOOL || token->klass == TOK_ERROR || token->klass == TOK_INT
    || token->klass == TOK_BIT || token->klass == TOK_VARBIT;
  return result;
}

internal bool
token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token->klass == TOK_TYPE_IDENTIFIER || token->klass == TOK_TUPLE;
  return result;
}

internal bool
token_is_functionPrototype()
{
  bool result = token_is_typeRef(token) || token->klass == TOK_VOID || token->klass == TOK_IDENTIFIER;
  return result;
}

internal bool
token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TOK_HEADER || token->klass == TOK_HEADER_UNION || token->klass == TOK_STRUCT
    || token->klass == TOK_ENUM;
  return result;
}

internal bool
token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TOK_TYPEDEF || token->klass == TOK_TYPE
    || token->klass == TOK_PARSER || token->klass == TOK_CONTROL || token->klass == TOK_PACKAGE;
  return result;
}

internal bool
token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TOK_IDENTIFIER || token->klass == TOK_APPLY || token->klass == TOK_KEY
    || token->klass == TOK_ACTIONS || token->klass == TOK_STATE || token->klass == TOK_ENTRIES || token->klass == TOK_TYPE;
  return result;
}

internal bool
token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TOK_TYPE_IDENTIFIER;
  return result;
}

internal bool
token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TOK_IDENTIFIER || token->klass == TOK_TYPE_IDENTIFIER
    || token->klass == TOK_APPLY || token->klass == TOK_STATE || token->klass == TOK_TYPE;
  return result;
}

internal bool
token_is_typeArg(Token* token)
{
  bool result = token->klass == TOK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

internal bool
token_is_typeParameterList(Token* token)
{
  return token_is_name(token);
}

internal bool
token_is_typeOrVoid(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TOK_VOID || token->klass == TOK_IDENTIFIER;
  return result;
}

internal Ast*
build_nonTypeName()
{
  assert(token_is_nonTypeName(token));
  if (token->klass == TOK_IDENTIFIER)
    next_token();
  else if (token->klass == TOK_APPLY)
    next_token();
  else if (token->klass == TOK_KEY)
    next_token();
  else if (token->klass == TOK_ACTIONS)
    next_token();
  else if (token->klass == TOK_STATE)
    next_token();
  else if (token->klass == TOK_ENTRIES)
    next_token();
  else if (token->klass == TOK_TYPE)
    next_token();
  else assert(false);
  return 0;
}

internal Ast*
build_name()
{
  assert(token_is_name(token));
  if (token_is_nonTypeName(token))
    build_nonTypeName();
  else if (token->klass == TOK_TYPE_IDENTIFIER)
    next_token();
  else assert(false);
  return 0;
}

internal Ast*
build_typeParameterList()
{
  assert(token_is_typeParameterList(token));
  build_name();
  while (token->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_name(token))
      build_name();
    else error("");
  }
  return 0;
}

internal Ast*
build_optTypeParameters()
{
  if (token->klass == TOK_ANGLE_OPEN)
  {
    next_token();
    if (token_is_typeParameterList(token))
      build_typeParameterList();
    else error("");
    if (token->klass == TOK_ANGLE_CLOSE)
      next_token;
    else error("");
  }
  return 0;
}

internal Ast*
build_typeArg()
{
  assert(token_is_typeArg(token));
  if (token->klass == TOK_DONTCARE)
    next_token();
  else if (token_is_typeRef(token))
    build_typeRef();
  else if (token_is_nonTypeName(token))
    build_nonTypeName();
  else assert(false);
  return 0;
}

internal bool
token_is_methodPrototype(Token* token)
{
  return token_is_functionPrototype() | token->klass == TOK_TYPE_IDENTIFIER;
}

internal Ast*
build_parameterList()
{
  if (token_is_parameter(token))
  {
    build_parameter();
    while (token->klass == TOK_COMMA)
    {
      next_token();
      if (token_is_parameter(token))
        build_parameter();
      else {
        error("");
        break;
      }
    }
  }
  return 0;
}

internal Ast*
build_typeOrVoid()
{
  assert(token_is_typeOrVoid(token));
  if (token_is_typeRef(token))
    build_typeRef();
  else if (token->klass == TOK_VOID)
    next_token();
  else if (token->klass == TOK_IDENTIFIER)
    next_token();
  else error("");
  return 0;
}

internal Ast*
build_functionPrototype()
{
  if (token_is_typeOrVoid(token))
  {
    build_typeOrVoid();
    if (token_is_name(token))
    {
      build_name();
      build_optTypeParameters();
      if (token->klass == TOK_PARENTH_OPEN)
      {
        next_token();
        build_parameterList();
        if (token->klass == TOK_PARENTH_CLOSE)
          next_token();
        else error("");
      }
      else error("");
    }
    else error("");
  }
  else error("");
  return 0;
}

internal Ast*
build_methodPrototype()
{
  assert(token_is_methodPrototype(token));
  if (token->klass == TOK_TYPE_IDENTIFIER && peek_token()->klass == TOK_BRACE_OPEN)
  {
    next_token();
    if (token->klass == TOK_PARENTH_OPEN)
    {
      next_token();
      build_parameterList();
      if (token->klass == TOK_PARENTH_CLOSE)
        next_token();
      else error("");
    }
    else error("");
  }
  else if (token_is_functionPrototype(token))
  {
    build_functionPrototype();
    expect_semicolon();
  }
  else error("");
  return 0;
}

internal Ast*
build_methodPrototypes()
{
  while (token_is_methodPrototype(token))
    build_methodPrototype();
  return 0;
}

internal Ast_Declaration*
build_externDeclaration()
{
  assert(token->klass == TOK_EXTERN);
  next_token();
  if (token_is_nonTypeName(token))
    build_nonTypeName();
  else error("");
  build_optTypeParameters();
  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    build_methodPrototypes();
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else error("");
  }
  else if (token_is_functionPrototype(token))
    build_functionPrototype();
  else error("");
  return 0;
}

internal Ast*
build_baseType()
{
  assert(token_is_baseType(token));
  if (token->klass == TOK_BOOL)
    ; // TODO
  else if (token->klass == TOK_ERROR)
    ; // TODO
  else if (token->klass == TOK_INT)
    ; // TODO
  else if (token->klass == TOK_BIT)
    ; // TODO
  else if (token->klass == TOK_VARBIT)
    ; // TODO
  else assert(false);
  return 0;
}

internal Ast*
build_prefixedType()
{
  assert(token->klass == TOK_DOTPREFIX);
  next_token();
  if (token->klass == TOK_TYPE_IDENTIFIER)
    next_token();
  else error("");
  return 0;
}

internal Ast*
build_typeArgumentList()
{
  while (token_is_typeArg(token))
  {
    build_typeArg();
    if (token->klass == TOK_COMMA)
    {
      next_token();
      if (!token_is_typeArg(token))
        error("");
    }
  }
  return 0;
}

internal Ast*
build_tupleType()
{
  assert(token->klass == TOK_TUPLE);
  next_token();
  if (token->klass == TOK_ANGLE_OPEN)
  {
    next_token();
    build_typeArgumentList();
    if (token->klass == TOK_ANGLE_CLOSE)
      next_token();
    else
      error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: '<' expected, got '%s'", token->line_nr, token->lexeme);
  return 0;
}

internal Ast*
build_headerStackType()
{
  assert(token->klass == TOK_BRACKET_OPEN);
  next_token();
  if (token_is_expression(token))
    build_expression(1);
  else
    error("at line %d: expression expected, got '%s'", token->line_nr, token->lexeme);
  if (token->klass != TOK_BRACKET_CLOSE)
    error("at line %d: ']' expected, got '%s'", token->line_nr, token->lexeme);
  return 0;
}

internal Ast*
build_specializedType()
{
  assert(token->klass == TOK_ANGLE_OPEN);
  next_token();
  build_typeArgumentList();
  if (token->klass == TOK_ANGLE_CLOSE)
    next_token();
  else
    error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
  return 0;
}

internal Ast*
build_typeName()
{
  assert(token->klass == TOK_TYPE_IDENTIFIER);
  next_token();
  if (token->klass == TOK_DOTPREFIX)
    build_prefixedType();
  if (token->klass == TOK_ANGLE_OPEN)
    build_specializedType();
  if (token->klass == TOK_BRACKET_OPEN)
    build_headerStackType();
  return 0;
}

internal Ast*
build_typeRef()
{
  assert(token_is_typeRef(token));
  if (token_is_baseType(token))
    build_baseType();
  else if (token->klass == TOK_TYPE_IDENTIFIER)
    /* <typeName> | <specializedType> | <headerStackType> */
    build_typeName();
  else if (token->klass == TOK_TUPLE)
    build_tupleType();
  else assert(false);
  return 0;
}

internal bool
token_is_structField(Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

internal Ast*
build_structField()
{
  assert(token_is_structField(token));
  next_token();
  while (token->klass == TOK_COMMA)
  {
    next_token();
    if (!token_is_structField(token))
      error("");
  }
  return 0;
}

internal Ast*
build_structFieldList()
{
  while (token_is_structField(token))
  {
    build_structField();
    if (token->klass == TOK_COMMA)
    {
      next_token();
      if (!token_is_structField(token))
        error("");
    }
  }
  return 0;
}

internal Ast*
build_headerTypeDeclaration()
{
  assert(token->klass == TOK_HEADER);
  next_token();
  if (token_is_name(token))
    build_name();
  else error("");
  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    build_structFieldList();
    if (token->klass == TOK_BRACE_CLOSE)
      next_token(token);
    else
      error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
  }
  return 0;
}

internal Ast*
build_headerUnionDeclaration()
{
  assert(token->klass == TOK_HEADER_UNION);
  next_token();
  if (token_is_name(token))
    build_name();
  else error("");
  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    build_structFieldList();
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else error("");
  }
  else error("");
  return 0;
}

internal Ast*
build_structTypeDeclaration()
{
  assert(token->klass == TOK_STRUCT);
  next_token();
  if (token_is_name(token))
    build_name();
  else error("");
  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    build_structFieldList();
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else error("");
  }
  else error("");
  return 0;
}

internal bool
token_is_specifiedIdentifier(Token* token)
{
  return token_is_name(token);
}

internal Ast*
build_initializer()
{
  return build_expression(1);
}

internal Ast*
build_specifiedIdentifier()
{
  assert(token_is_specifiedIdentifier(token));
  build_name();
  if (token->klass == TOK_EQUAL)
  {
    next_token();
    if (token_is_expression(token))
      build_initializer();
    else error("");
  }
  else error("");
  return 0;
}

internal Ast*
build_specifiedIdentifierList()
{
  assert(token_is_specifiedIdentifier(token));
  build_specifiedIdentifier();
  while (token->klass == TOK_COMMA)
  {
    next_token();
    if (token_is_specifiedIdentifier(token))
      build_specifiedIdentifier();
    else error("");
  }
  return 0;
}

internal Ast*
build_enumDeclaration()
{
  assert(token->klass == TOK_ENUM);
  next_token();
  if (token->klass == TOK_BIT)
    ; // TODO
  if (token_is_name)
    build_name();
  else error("");
  if (token->klass == TOK_BRACE_OPEN)
  {
    next_token();
    if (token_is_specifiedIdentifier(token))
      build_specifiedIdentifierList();
    else error("");
    if (token->klass == TOK_BRACE_CLOSE)
      next_token();
    else error("");
  }
  return 0;
}

internal Ast*
build_derivedTypeDeclaration()
{
  assert(token_is_derivedTypeDeclaration(token));
  if (token->klass == TOK_HEADER)
    build_headerTypeDeclaration();
  else if (token->klass == TOK_HEADER_UNION)
    build_headerUnionDeclaration();
  else if (token->klass == TOK_STRUCT)
    build_structTypeDeclaration();
  else if (token->klass == TOK_ENUM)
    build_enumDeclaration();
  else assert(false);
  return 0;
}

internal Ast*
build_typeDeclaration()
{
  assert(token_is_typeDeclaration(token));
  if (token_is_derivedTypeDeclaration(token))
    build_derivedTypeDeclaration();
  if (token->klass == TOK_TYPEDEF)
    build_typedefDeclaration();
  else if (token->klass == TOK_PARSER)
    ;
  return 0;
}

internal Ast*
build_typedefDeclaration()
{
  assert(token->klass == TOK_TYPEDEF || token->klass == TOK_TYPE);
  if (token->klass == TOK_TYPEDEF)
    next_token();
  else if (token->klass == TOK_TYPE)
    next_token();
  else assert(false);
  if (token_is_typeRef(token))
    build_typeRef();
  else if (token_is_derivedTypeDeclaration(token))
    build_derivedTypeDeclaration();
  else error("");
  if (token_is_name(token))
    build_name();
  else error("");
  expect_semicolon();
  return 0;
}

internal bool
token_is_declaration(Token* token)
{
  bool result = token->klass == TOK_CONST || token->klass == TOK_EXTERN || token->klass == TOK_ACTION
    || token->klass == TOK_PARSER || token_is_typeDeclaration(token) || token->klass == TOK_CONTROL
    /* || token_is_instantiation(token) */ || token->klass == TOK_ERROR || token->klass == TOK_MATCH_KIND
    /* || token_is_functionDeclaration(token) */ ;
  return result;
}

internal Ast*
build_constantDeclaration()
{
  assert(token->klass == TOK_CONST);
  next_token();
  if (token_is_typeRef(token))
  {
    build_typeRef();
    if (token_is_name(token))
    {
      build_name();
      if (token->klass == TOK_EQUAL)
      {
        next_token();
        if (token_is_expression(token))
        {
          build_expression(1);
          expect_semicolon();
        }
        else error("");
      }
      else error("");
    }
    else error("");
  }
  else error("");
  return 0;
}

internal Ast*
build_declaration()
{
  assert(token_is_declaration(token));
  if (token->klass == TOK_CONST)
    build_constantDeclaration();
  else if (token->klass == TOK_EXTERN)
    build_externDeclaration();
  else if (token->klass == TOK_ACTION)
    ; /* TODO
    build_actionDeclaration(); */
  else if (token_is_typeDeclaration(token))
    /* <parserDeclaration> | <controlDeclaration> */
    build_typeDeclaration();
  /*
  else if (token_is_instantiation(token))
    ; TODO */
  else if (token->klass == TOK_ERROR)
    build_errorDeclaration();
  else if (token->klass == TOK_MATCH_KIND)
      ; // TODO
  /*
  else if (token_is_functionDeclaration(token))
    ; TODO */
  else assert(false);

    /*
    case TOK_STRUCT:
      result = (Ast_Declaration*)build_struct_decl();
      break;
    case TOK_HEADER:
      result = (Ast_Declaration*)build_header_decl();
      break;
    case TOK_TYPEDEF:
      result = (Ast_Declaration*)build_typedef_decl();
      break;
    case TOK_PACKAGE:
      result = (Ast_Declaration*)build_package_prototype();
      break;
    case TOK_IDENTIFIER:
    case TOK_TYPE_IDENTIFIER:
      result = (Ast_Declaration*)build_package_instantiation();
      break;
    */

  return 0;
}

internal Ast*
build_p4program()
{
  if (token->klass == TOK_SEMICOLON)
    return 0;
  if (token_is_declaration(token))
  {
    build_declaration();
    while (token_is_declaration(token))
    {
      build_declaration();
    }
    if (token->klass != TOK_EOI)
      error("at line %d: unexpected token '%s'", token->line_nr, token->lexeme);
  }
  else
    error("at line %d: declaration expected, got '%s'", token->line_nr, token->lexeme);
  return 0;
}

void
build_ast()
{
  error_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  error_type_ast->kind = AST_TYPE_IDENT;
  error_type_ast->name = "error";
  error_type_ast->lexeme = error_type_ast->name;
  error_type_ast->is_builtin = true;

  void_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  void_type_ast->kind = AST_TYPE_IDENT;
  void_type_ast->name = "void";
  void_type_ast->lexeme = void_type_ast->name;
  void_type_ast->is_builtin = true;

  bool_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  bool_type_ast->kind = AST_TYPE_IDENT;
  bool_type_ast->name = "bool";
  bool_type_ast->lexeme = bool_type_ast->name;
  bool_type_ast->is_builtin = true;

  bit_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  bit_type_ast->kind = AST_TYPE_IDENT;
  bit_type_ast->name = "bit";
  bit_type_ast->lexeme = bit_type_ast->name;
  bit_type_ast->is_builtin = true;

  varbit_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  varbit_type_ast->kind = AST_TYPE_IDENT;
  varbit_type_ast->name = "varbit";
  varbit_type_ast->lexeme = varbit_type_ast->name;
  varbit_type_ast->is_builtin = true;

  int_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  int_type_ast->kind = AST_TYPE_IDENT;
  int_type_ast->name = "int";
  int_type_ast->lexeme = int_type_ast->name;
  int_type_ast->is_builtin = true;

  string_type_ast = arena_push_struct(&arena, Ast_TypeIdent);
  string_type_ast->kind = AST_TYPE_IDENT;
  string_type_ast->name = "string";
  string_type_ast->lexeme = string_type_ast->name;
  string_type_ast->is_builtin = true;

  bool_true_ast = arena_push_struct(&arena, Ast_Integer);
  bool_true_ast->kind = AST_INTEGER;
  bool_true_ast->lexeme = "true";
  bool_true_ast->value = 1;

  bool_false_ast = arena_push_struct(&arena, Ast_Integer);
  bool_false_ast->kind = AST_INTEGER;
  bool_false_ast->lexeme = "false";
  bool_false_ast->value = 0;

  add_keyword("action", TOK_ACTION);
  add_keyword("enum", TOK_ENUM);
  add_keyword("in", TOK_IN);
  add_keyword("package", TOK_PACKAGE);
  add_keyword("select", TOK_SELECT);
  add_keyword("switch", TOK_SWITCH);
  add_keyword("tuple", TOK_TUPLE);
  add_keyword("control", TOK_CONTROL);
  error_kw = add_keyword("error", TOK_ERROR);
  add_keyword("header", TOK_HEADER);
  add_keyword("inout", TOK_INOUT);
  add_keyword("parser", TOK_PARSER);
  add_keyword("state", TOK_STATE);
  add_keyword("table", TOK_TABLE);
  add_keyword("key", TOK_KEY);
  add_keyword("typedef", TOK_TYPEDEF);
  add_keyword("default", TOK_DEFAULT);
  add_keyword("extern", TOK_EXTERN);
  add_keyword("header_union", TOK_HEADER_UNION);
  add_keyword("out", TOK_OUT);
  add_keyword("transition", TOK_TRANSITION);
  add_keyword("else", TOK_ELSE);
  add_keyword("exit", TOK_EXIT);
  add_keyword("if", TOK_IF);
  add_keyword("match_kind", TOK_MATCH_KIND);
  add_keyword("return", TOK_RETURN);
  add_keyword("struct", TOK_STRUCT);
  apply_kw = add_keyword("apply", TOK_APPLY);
  add_keyword("var", TOK_VAR);
  add_keyword("const", TOK_CONST);
  add_keyword("bool", TOK_BOOL);
  add_keyword("true", TOK_TRUE);
  add_keyword("false", TOK_FALSE);
  add_keyword("tuple", TOK_TUPLE);

  /*
  error_type_ident = sym_new_type(error_type_ast->name, (Ast*)error_type_ast);
  void_type_ident = sym_new_type(void_type_ast->name, (Ast*)void_type_ast);
  bool_type_ident = sym_new_type("bool", (Ast*)bool_type_ast);
  bit_type_ident = sym_new_type("bit", (Ast*)bit_type_ast);
  varbit_type_ident = sym_new_type("varbit", (Ast*)varbit_type_ast);
  int_type_ident = sym_new_type("int", (Ast*)int_type_ast);
  string_type_ident = sym_new_type("string", (Ast*)string_type_ast);

  bool_true_ident = sym_new_var("true", (Ast*)bool_true_ast);
  bool_false_ident = sym_new_var("false", (Ast*)bool_false_ast);
  */

  token = tokenized_input;
  next_token();
  build_p4program();
}
