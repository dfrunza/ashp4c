#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal Arena *ast_storage;
internal UnboundedArray* tokens_array;
internal int token_at = 0;
internal Token* token = 0;
internal int prev_token_at = 0;
internal Token* prev_token = 0;
internal int node_id = 0;
internal Scope* root_scope;
internal Scope* current_scope;

internal Ast* build_expression(int priority_threshold);
internal Ast* build_typeRef();
internal Ast* build_blockStatement();
internal Ast* build_statement(Ast* type_name);
internal Ast* build_parserStatement();

internal Token*
next_token()
{
  assert (token_at < tokens_array->elem_count);
  prev_token = token;
  prev_token_at = token_at;
  token = array_get(tokens_array, ++token_at);
  while (token->klass == TK_COMMENT) {
    token = array_get(tokens_array, ++token_at);
  }
  if (token->klass == TK_IDENTIFIER) {
    NameEntry* ne = scope_lookup_name(current_scope, token->lexeme);
    if (ne->ns_keyword) {
      NameDecl* nd = ne->ns_keyword;
      token->klass = nd->token_class;
      return token;
    }
    if (ne->ns_type) {
      token->klass = TK_TYPE_IDENTIFIER;
      return token;
    }
  }
  return token;
}

internal Token*
peek_token()
{
  prev_token = token;
  prev_token_at = token_at;
  Token* peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

internal bool
token_is_typeName(Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOTPREFIX;
}

internal bool
token_is_prefixedType(Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOTPREFIX;
}

internal bool
token_is_baseType(Token* token)
{
  bool result = token->klass == TK_BOOL || token->klass == TK_ERROR || token->klass == TK_INT
    || token->klass == TK_BIT || token->klass == TK_VARBIT || token->klass == TK_STRING
    || token->klass == TK_VOID;
  return result;
}

internal bool
token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token_is_prefixedType(token) || token->klass == TK_TUPLE;
  return result;
}

internal bool
token_is_direction(Token* token)
{
  bool result = token->klass == TK_IN || token->klass == TK_OUT || token->klass == TK_INOUT;
  return result;
}

internal bool
token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

internal bool
token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TK_HEADER || token->klass == TK_HEADER_UNION || token->klass == TK_STRUCT
    || token->klass == TK_ENUM;
  return result;
}

internal bool
token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF || token->klass == TK_TYPE
    || token->klass == TK_PARSER || token->klass == TK_CONTROL || token->klass == TK_PACKAGE;
  return result;
}

internal bool
token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

internal bool
token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_typeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
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
  bool result = token_is_typeRef(token) || token->klass == TK_VOID || token->klass == TK_IDENTIFIER;
  return result;
}

internal bool
token_is_actionRef(Token* token)
{
  bool result = token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)
    || token->klass == TK_PARENTH_OPEN;
  return result;
}

internal bool
token_is_tableProperty(Token* token)
{
  bool result = token->klass == TK_KEY || token->klass == TK_ACTIONS
    || token->klass == TK_CONST || token->klass == TK_ENTRIES
    || token_is_nonTableKwName(token);
  return result;
}

internal bool
token_is_switchLabel(Token* token)
{
  bool result = token_is_name(token) || token->klass == TK_DEFAULT;
  return result;
}

internal bool
token_is_expressionPrimary(Token* token)
{
  bool result = token->klass == TK_INT_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token_is_prefixedType(token);
  return result;
}

internal bool
token_is_expression(Token* token)
{
  return token_is_expressionPrimary(token);
}

internal bool
token_is_methodPrototype(Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

internal Ast*
build_nonTypeName(bool is_type)
{
  if (token_is_nonTypeName(token)) {
    Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
    name->kind = AST_NAME;
    name->id = node_id++;
    name->line_no = token->line_no;
    name->strname = token->lexeme;
    if (is_type) {
      NameDecl* decl = arena_push_struct(ast_storage, NameDecl);
      decl->ast = (Ast*)name;
      decl->strname = name->strname;
      decl->line_no = token->line_no;
      declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
    }
    next_token();
    return (Ast*)name;
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_name(bool is_type)
{
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      Ast_Name* name = (Ast_Name*)build_nonTypeName(is_type);
      return (Ast*)name;
    } else if (token->klass == TK_TYPE_IDENTIFIER) {
      Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
      type_name->kind = AST_NAME;
      type_name->id = node_id++;
      type_name->line_no = token->line_no;
      type_name->strname = token->lexeme;
      next_token();
      return (Ast*)type_name;
    } else assert(0);
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_typeParameterList()
{
  Ast_ElementList* params = arena_push_struct(ast_storage, Ast_ElementList);
  params->kind = AST_ELEM_LIST;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->head.next = 0;
  if (token_is_typeParameterList(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&params->head, li);
    li->object = build_name(true);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_name(true);
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)params;
}

internal Ast*
build_optTypeParameters()
{
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    if (token_is_typeParameterList(token)) {
      Ast* params = build_typeParameterList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      return params;
    } else if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
  }
  Ast_ElementList* params = arena_push_struct(ast_storage, Ast_ElementList);
  params->kind = AST_ELEM_LIST;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->head.next = 0;
  return (Ast*)params;
}

internal Ast*
build_typeArg()
{
  if (token_is_typeArg(token)) {
    if (token->klass == TK_DONTCARE) {
      Ast* dontcare = arena_push_struct(ast_storage, Ast);
      dontcare->kind = AST_DONTCARE;
      dontcare->id = node_id++;
      dontcare->line_no = token->line_no;
      next_token();
      return dontcare;
    } else if (token_is_typeRef(token)) {
      Ast* arg = build_typeRef();
      return arg;
    } else if (token_is_nonTypeName(token)) {
      Ast* arg = build_nonTypeName(false);
      return arg;
    } else assert(0);
  } else error("at line %d: type argument was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal enum AstParamDirection
build_direction()
{
  if (token_is_direction(token)) {
    if (token->klass == TK_IN) {
      next_token();
      return PARAMDIR_IN;
    } else if (token->klass == TK_OUT) {
      next_token();
      return PARAMDIR_OUT;
    } else if (token->klass == TK_INOUT) {
      next_token();
      return PARAMDIR_INOUT;
    } else assert(0);
  }
  return 0;
}

internal Ast*
build_parameter()
{
  Ast_Param* param = arena_push_struct(ast_storage, Ast_Param);
  param->kind = AST_PARAM;
  param->id = node_id++;
  param->line_no = token->line_no;
  param->direction = build_direction();
  if (token_is_typeRef(token)) {
    param->type = build_typeRef();
    if (token_is_name(token)) {
      param->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          param->init_expr = build_expression(1);
        } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)param;
}

internal void
build_parameterList(Ast_ElementList* params)
{
  params->kind = AST_ELEM_LIST;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->head.next = 0;
  if (token_is_parameter(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&params->head, li);
    li->object = build_parameter();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_parameter();
      dlist_concat(last, li);
      last = li;
    }
  }
}

internal Ast*
build_typeOrVoid(bool is_type)
{
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      Ast* type = build_typeRef();
      return type;
    } else if (token->klass == TK_VOID) {
      Ast_Name* void_name = arena_push_struct(ast_storage, Ast_Name);
      void_name->kind = AST_NAME;
      void_name->id = node_id++;
      void_name->line_no = token->line_no;
      void_name->strname = token->lexeme;
      next_token();
      return (Ast*)void_name;
    } else if (token->klass == TK_IDENTIFIER) {
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_NAME;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->strname = token->lexeme;
      if (is_type) {
        NameDecl* decl = arena_push_struct(ast_storage, NameDecl);
        decl->ast = (Ast*)name;
        decl->strname = name->strname;
        decl->line_no = token->line_no;
        declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
      }
      next_token();
      return (Ast*)name;
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_functionPrototype(Ast* return_type)
{
  if (token_is_typeOrVoid(token) || return_type) {
    Ast_FunctionProto* proto = arena_push_struct(ast_storage, Ast_FunctionProto);
    proto->kind = AST_FUNCTION_PROTO;
    proto->id = node_id++;
    proto->line_no = token->line_no;
    if (return_type) {
      proto->return_type = return_type;
    } else {
      proto->return_type = build_typeOrVoid(true);
    }
    if (token_is_name(token)) {
      proto->name = build_name(false);
      build_optTypeParameters(&proto->type_params);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&proto->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: function name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)proto;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_methodPrototype()
{
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      Ast_FunctionProto* proto = arena_push_struct(ast_storage, Ast_FunctionProto);
      proto->kind = AST_FUNCTION_PROTO;
      proto->id = node_id++;
      proto->line_no = token->line_no;
      proto->name = build_name(false);
      build_optTypeParameters(&proto->type_params);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&proto->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", token->line_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)proto;
    } else if (token_is_typeOrVoid(token)) {
      Ast_FunctionProto* proto = (Ast_FunctionProto*)build_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)proto;
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_methodPrototypes()
{
  Ast_ElementList* protos = arena_push_struct(ast_storage, Ast_ElementList);
  protos->kind = AST_ELEM_LIST;
  protos->id = node_id++;
  protos->line_no = token->line_no;
  protos->head.next = 0;
  if (token_is_methodPrototype(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&protos->head, li);
    li->object = build_methodPrototype();
    while (token_is_methodPrototype(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_methodPrototype();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)protos;
}

internal Ast*
build_externDeclaration()
{
  if (token->klass == TK_EXTERN) {
    next_token();
    bool is_function_proto = false;
    if (token_is_typeOrVoid(token) && token_is_nonTypeName(token)) {
      is_function_proto = token_is_typeOrVoid(token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(token)) {
      is_function_proto = true;
    } else if (token_is_nonTypeName(token)) {
      is_function_proto = false;
    } else error("at line %d: extern declaration was expected, got `%s`.", token->line_no, token->lexeme);

    if (is_function_proto) {
      Ast_FunctionProto* proto = (Ast_FunctionProto*)build_functionPrototype(0);
      proto->is_extern = true;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)proto;
    } else {
      Ast_Extern* extern_decl = arena_push_struct(ast_storage, Ast_Extern);
      extern_decl->kind = AST_EXTERN;
      extern_decl->id = node_id++;
      extern_decl->line_no = token->line_no;
      extern_decl->name = build_nonTypeName(true);
      build_optTypeParameters(&extern_decl->type_params);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_methodPrototypes(&extern_decl->method_protos);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)extern_decl;
    }
  } else error("at line %d: `extern` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_integer()
{
  if (token->klass == TK_INT_LITERAL) {
    Ast_IntLiteral* int_literal = arena_push_struct(ast_storage, Ast_IntLiteral);
    int_literal->kind = AST_INT_LITERAL;
    int_literal->id = node_id++;
    int_literal->line_no = token->line_no;
    int_literal->is_signed = token->i.is_signed;
    int_literal->width = token->i.width;
    int_literal->value = token->i.value;
    next_token();
    return (Ast*)int_literal;
  } else error("at line %d: integer was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_boolean()
{
  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    Ast_BoolLiteral* bool_literal = arena_push_struct(ast_storage, Ast_BoolLiteral);
    bool_literal->kind = AST_BOOL_LITERAL;
    bool_literal->id = node_id++;
    bool_literal->line_no = token->line_no;
    bool_literal->value = (token->klass == TK_TRUE);
    next_token();
    return (Ast*)bool_literal;
  } else error("at line %d: boolean was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_stringLiteral()
{
  if (token->klass == TK_STRING_LITERAL) {
    Ast_StringLiteral* string_literal = arena_push_struct(ast_storage, Ast_StringLiteral);
    string_literal->kind = AST_STRING_LITERAL;
    string_literal->id = node_id++;
    string_literal->line_no = token->line_no;
    string_literal->value = token->lexeme;
    next_token();
    return (Ast*)string_literal;
  } else error("at line %d: string was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_integerTypeSize()
{
  Ast_IntTypeSize* type_size = arena_push_struct(ast_storage, Ast_IntTypeSize);
  type_size->kind = AST_INT_TYPESIZE;
  type_size->id = node_id++;
  type_size->line_no = token->line_no;
  if (token->klass == TK_INT_LITERAL) {
    type_size->size = build_integer();
  } else if (token->klass == TK_PARENTH_OPEN) {
    /* FIXME
    type_size->size = build_expression(1); */
    error("at line %d: integer was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)type_size;
}

internal Ast*
build_baseType()
{
  if (token_is_baseType(token)) {
    Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
    type_name->kind = AST_NAME;
    type_name->id = node_id++;
    type_name->line_no = token->line_no;
    if (token->klass == TK_BOOL) {
      Ast_BoolType* bool_type = arena_push_struct(ast_storage, Ast_BoolType);
      bool_type->kind = AST_BOOL_TYPE;
      bool_type->id = node_id++;
      bool_type->line_no = token->line_no;
      type_name->strname = "bool";
      bool_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)bool_type;
    } else if (token->klass == TK_ERROR) {
      Ast_ErrorType* error_type = arena_push_struct(ast_storage, Ast_ErrorType);
      error_type->kind = AST_ERROR_TYPE;
      error_type->id = node_id++;
      error_type->line_no = token->line_no;
      type_name->strname = "error";
      error_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)error_type;
    } else if (token->klass == TK_INT) {
      Ast_IntType* int_type = arena_push_struct(ast_storage, Ast_IntType);
      int_type->kind = AST_INT_TYPE;
      int_type->id = node_id++;
      int_type->line_no = node_id++;
      type_name->strname = "int";
      int_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      return (Ast*)int_type;
    } else if (token->klass == TK_BIT) {
      Ast_BitType* bit_type = arena_push_struct(ast_storage, Ast_BitType);
      bit_type->kind = AST_BIT_TYPE;
      bit_type->id = node_id++;
      bit_type->line_no = token->line_no;
      type_name->strname = "bit";
      bit_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      return (Ast*)bit_type;
    } else if (token->klass == TK_VARBIT) {
      Ast_VarbitType* varbit_type = arena_push_struct(ast_storage, Ast_VarbitType);
      varbit_type->kind = AST_VARBIT_TYPE;
      varbit_type->id = node_id++;
      varbit_type->line_no = node_id++;
      type_name->strname = "varbit";
      varbit_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      return (Ast*)varbit_type;
    } else if (token->klass == TK_STRING) {
      Ast_StringType* string_type = arena_push_struct(ast_storage, Ast_StringType);
      string_type->kind = AST_STRING_TYPE;
      string_type->id = node_id++;
      string_type->line_no = token->line_no;
      type_name->strname = "string";
      string_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)string_type;
    } else if (token->klass == TK_VOID) {
      Ast_VoidType* void_type = arena_push_struct(ast_storage, Ast_VoidType);
      void_type->kind = AST_VOID_TYPE;
      void_type->id = node_id++;
      void_type->line_no = token->line_no;
      type_name->strname = "void";
      void_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)void_type;
    } else assert(0);
  } else error("at line %d: base type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal void
build_typeArgumentList(Ast_ElementList* args)
{
  args->kind = AST_ELEM_LIST;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->head.next = 0;
  if (token_is_typeArg(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&args->head, li);
    li->object = build_typeArg();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_typeArg();
      dlist_concat(last, li);
      last = li;
    }
  }
}

internal Ast*
build_tupleType()
{
  if (token->klass == TK_TUPLE) {
    next_token();
    Ast_Tuple* tuple = arena_push_struct(ast_storage, Ast_Tuple);
    tuple->kind = AST_TUPLE;
    tuple->id = node_id++;
    tuple->line_no = token->line_no;
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      build_typeArgumentList(&tuple->type_args);
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)tuple;
  } else error("at line %d: `tuple` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_headerStackType()
{
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    Ast_HeaderStack* stack = arena_push_struct(ast_storage, Ast_HeaderStack);
    stack->kind = AST_HEADER_STACK;
    stack->id = node_id++;
    stack->line_no = token->line_no;
    if (token_is_expression(token)) {
      stack->stack_expr = build_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: expression expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)stack;
  } else error("at line %d: `[` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_specializedType()
{
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    Ast_SpecializedType* type = arena_push_struct(ast_storage, Ast_SpecializedType);
    type->kind = AST_SPECIALIZED_TYPE;
    type->id = node_id++;
    type->line_no = token->line_no;
    build_typeArgumentList(&type->type_args);
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)type;
  } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_prefixedType()
{
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
    name->id = node_id++;
    name->line_no = token->line_no;
    name->strname = token->lexeme;
    next_token();
    return (Ast*)name;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_typeName()
{
  if (token_is_typeName(token)) {
    Ast* name = build_prefixedType();
    if (token->klass == TK_ANGLE_OPEN) {
      Ast* speclzd_type = build_specializedType();
      assert (speclzd_type->kind == AST_SPECIALIZED_TYPE);
      ((Ast_SpecializedType*)speclzd_type)->name = name;
      name = speclzd_type;
    } if (token->klass == TK_BRACKET_OPEN) {
      Ast* stack_type = build_headerStackType();
      assert (stack_type->kind == AST_HEADER_STACK);
      ((Ast_HeaderStack*)stack_type)->name = name;
      name = stack_type;
    }
    return name;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_typeRef()
{
  if (token_is_typeRef(token)) {
    if (token_is_baseType(token)) {
      Ast* ref = build_baseType();
      return ref;
    } else if (token_is_typeName(token)) {
      /* <typeName> | <specializedType> | <headerStackType> */
      Ast* ref = build_typeName();
      return ref;
    } else if (token->klass == TK_TUPLE) {
      Ast* ref = build_tupleType();
      return ref;
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
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
  Ast_StructField* field = arena_push_struct(ast_storage, Ast_StructField);
  field->kind = AST_STRUCT_FIELD;
  field->id = node_id++;
  field->line_no = token->line_no;
  if (token_is_typeRef(token)) {
    field->type = build_typeRef();
    if (token_is_name(token)) {
      field->name = build_name(false);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: struct field was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)field;
}

internal Ast*
build_structFieldList()
{
  Ast_ElementList* fields = arena_push_struct(ast_storage, Ast_ElementList);
  fields->kind = AST_ELEM_LIST;
  fields->id = node_id++;
  fields->line_no = token->line_no;
  fields->head.next = 0;
  if (token_is_structField(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&fields->head, li);
    li->object = build_structField();
    while (token_is_structField(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_structField();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)fields;
}

internal Ast*
build_headerTypeDeclaration()
{
  if (token->klass == TK_HEADER) {
    next_token();
    Ast_Header* decl = arena_push_struct(ast_storage, Ast_Header);
    decl->kind = AST_HEADER;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_structFieldList(&decl->fields);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `header` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_headerUnionDeclaration()
{
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    Ast_HeaderUnion* decl = arena_push_struct(ast_storage, Ast_HeaderUnion);
    decl->kind = AST_HEADER_UNION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_structFieldList(decl->fields);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `header_union` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_structTypeDeclaration()
{
  if (token->klass == TK_STRUCT) {
    next_token();
    Ast_Struct* decl = arena_push_struct(ast_storage, Ast_Struct);
    decl->kind = AST_STRUCT;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_structFieldList(&decl->fields);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `struct` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
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
build_optInitializer()
{
  if (token->klass == TK_EQUAL) {
    next_token();
    Ast* init_stmt = build_initializer();
    return init_stmt;
  }
  return 0;
}

internal Ast*
build_specifiedIdentifier()
{
  if (token_is_specifiedIdentifier(token)) {
    Ast_SpecifiedIdent* id = arena_push_struct(ast_storage, Ast_SpecifiedIdent);
    id->kind = AST_SPECIFIED_IDENT;
    id->id = node_id++;
    id->line_no = token->line_no;
    id->name = build_name(false);
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->init_expr = build_initializer();
      } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
    }
    return (Ast*)id;
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_specifiedIdentifierList()
{
  Ast_ElementList* ids = arena_push_struct(ast_storage, Ast_ElementList);
  ids->kind = AST_ELEM_LIST;
  ids->id = node_id++;
  ids->line_no = token->line_no;
  ids->head.next = 0;
  if (token_is_specifiedIdentifier(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&ids->head, li);
    li->object = build_specifiedIdentifier();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_specifiedIdentifier();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)ids;
}

internal Ast*
build_enumDeclaration()
{
  if (token->klass == TK_ENUM) {
    next_token();
    Ast_Enum* decl = arena_push_struct(ast_storage, Ast_Enum);
    decl->kind = AST_ENUM;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BIT) {
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        if (token->klass == TK_INT_LITERAL) {
          decl->type_size = build_integer();
          if (token->klass == TK_ANGLE_CLOSE) {
            next_token();
          } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: an integer was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
    }
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          build_specifiedIdentifierList(&decl->id_list);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `enum` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    if (token->klass == TK_HEADER) {
      Ast* decl = build_headerTypeDeclaration();
      return decl;
    } else if (token->klass == TK_HEADER_UNION) {
      Ast* decl = build_headerUnionDeclaration();
      return decl;
    } else if (token->klass == TK_STRUCT) {
      Ast* decl = build_structTypeDeclaration();
      return decl;
    } else if (token->klass == TK_ENUM) {
      Ast* decl = build_enumDeclaration();
      return decl;
    } else assert(0);
  } else error("at line %d: structure declaration was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserTypeDeclaration()
{
  if (token->klass == TK_PARSER) {
    next_token();
    Ast_ParserProto* type = arena_push_struct(ast_storage, Ast_ParserProto);
    type->kind = AST_PARSER_PROTO;
    type->id = node_id++;
    type->line_no = token->line_no; 
    if (token_is_name(token)) {
      type->name = build_name(true);
      build_optTypeParameters(&type->type_params);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&type->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)type;
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal void
build_optConstructorParameters(Ast_ElementList* params)
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    if (token_is_parameter(token)) {
      build_parameterList(params);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
  }
}

internal Ast*
build_constantDeclaration()
{
  if (token->klass == TK_CONST) {
    next_token();
    Ast_Const* decl = arena_push_struct(ast_storage, Ast_Const);
    decl->kind = AST_CONST;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_typeRef(token)) {
      decl->type_ref = build_typeRef();
      if (token_is_name(token)) {
        decl->name = build_name(false);
        if (token->klass == TK_EQUAL) {
          next_token();
          if (token_is_expression(token)) {
            decl->expr = build_expression(1);
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
          } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `const` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal bool
token_is_declaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token) || token->klass == TK_DOTPREFIX;
  return result;
}

internal bool
token_is_lvalue(Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == TK_DOTPREFIX;
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TK_PARENTH_OPEN || token->klass == TK_ANGLE_OPEN
    || token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_statement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TK_IF
    || token->klass == TK_SEMICOLON || token->klass == TK_BRACE_OPEN || token->klass == TK_EXIT
    || token->klass == TK_RETURN || token->klass == TK_SWITCH;
  return result;
}

internal bool
token_is_statementOrDeclaration(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_CONST || token_is_statement(token);
  return result;
}

internal bool
token_is_argument(Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_parserLocalElement(Token* token)
{
  bool result = token->klass == TK_CONST || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_CONST || token_is_typeRef(token)
    || token->klass == TK_SEMICOLON;
  return result;
}

internal bool
token_is_simpleKeysetExpression(Token* token) {
  bool result = token_is_expression(token) || token->klass == TK_DEFAULT || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_keysetExpression(Token* token)
{
  bool result = token->klass == TK_TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

internal bool
token_is_selectCase(Token* token)
{
  return token_is_keysetExpression(token);
}

internal bool
token_is_controlLocalDeclaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_ACTION
    || token->klass == TK_TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

internal Ast*
build_argument()
{
  if (token_is_argument(token)) {
    if (token_is_expression(token)) {
      Ast* arg = build_expression(1);
      return arg;
    } else if (token_is_name(token)) {
      Ast_Argument* name_arg = arena_push_struct(ast_storage, Ast_Argument);
      name_arg->kind = AST_ARGUMENT;
      name_arg->id = node_id++;
      name_arg->line_no = token->line_no;
      name_arg->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          name_arg->init_expr = build_expression(1);
        } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)name_arg;
    } else if (token->klass == TK_DONTCARE) {
      Ast* dontcare_arg = arena_push_struct(ast_storage, Ast);
      dontcare_arg->kind = AST_DONTCARE;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      next_token();
      return dontcare_arg;
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_argumentList()
{
  Ast_ElementList* args = arena_push_struct(ast_storage, Ast_ElementList);
  args->kind = AST_ELEM_LIST;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->head.next = 0;
  if (token_is_argument(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&args->head, li);
    li->object = build_argument();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_argument();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)args;
}

internal Ast*
build_optArguments()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    if (token_is_argument(token)) {
      Ast* args = build_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      return args;
    } else if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
  }
  Ast_ElementList* args = arena_push_struct(ast_storage, Ast_ElementList);
  args->kind = AST_ELEM_LIST;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->head.next = 0;
  return (Ast*)args;
}

internal Ast*
build_variableDeclaration(Ast* type_ref)
{
  if (token_is_typeRef(token) || type_ref) {
    Ast_Var* decl = arena_push_struct(ast_storage, Ast_Var);
    decl->kind = AST_VAR;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->type = type_ref ? type_ref : build_typeRef();
    if (token_is_name(token)) {
      decl->name = build_name(false);
      decl->init_expr = build_optInitializer();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_instantiation(Ast* type_ref)
{
  if (token_is_typeRef(token) || type_ref) {
    Ast_Instantiation* inst_stmt = arena_push_struct(ast_storage, Ast_Instantiation);
    inst_stmt->kind = AST_INSTANTIATION;
    inst_stmt->id = node_id++;
    inst_stmt->line_no = token->line_no;
    inst_stmt->type_ref = type_ref ? type_ref : build_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      build_argumentList(&inst_stmt->args);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          inst_stmt->name = build_name(false);
          if (token->klass == TK_SEMICOLON) {
            next_token();
          } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: instance name was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)inst_stmt;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserLocalElement()
{
  if (token_is_parserLocalElement(token)) {
    if (token->klass == TK_CONST) {
      Ast* elem = build_constantDeclaration();
      return elem;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        Ast* elem = build_instantiation(type_ref);
        return elem;
      } else if (token_is_name(token)) {
        Ast* elem = build_variableDeclaration(type_ref);
        return elem;
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else assert(0);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserLocalElements()
{
  Ast_ElementList* elems = arena_push_struct(ast_storage, Ast_ElementList);
  elems->kind = AST_ELEM_LIST;
  elems->id = node_id++;
  elems->line_no = token->line_no;
  elems->head.next = 0;
  if (token_is_parserLocalElement(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&elems->head, li);
    li->object = build_parserLocalElement();
    while (token_is_parserLocalElement(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_parserLocalElement();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)elems;
}

internal Ast*
build_directApplication(Ast* type_name)
{
  if (token_is_typeName(token) || type_name) {
    Ast_FunctionCall* apply_expr = arena_push_struct(ast_storage, Ast_FunctionCall);
    apply_expr->kind = AST_FUNCTION_CALL;
    apply_expr->id = node_id++;
    apply_expr->line_no = token->line_no;
    Ast_MemberSelect* apply_select = arena_push_struct(ast_storage, Ast_MemberSelect);
    apply_select->kind = AST_MEMBER_SELECT;
    apply_select->id = node_id++;
    apply_select->line_no = token->line_no;
    apply_select->lhs_expr = type_name ? type_name : build_typeName();
    Ast_Name* apply_name = arena_push_struct(ast_storage, Ast_Name);
    apply_name->kind = AST_NAME;
    apply_name->id = node_id++;
    apply_name->line_no = token->line_no;
    apply_name->strname = "apply";
    apply_select->member_name = (Ast*)apply_name;
    apply_expr->callee_expr = (Ast*)apply_select;
    if (token->klass == TK_DOTPREFIX) {
      next_token();
      if (token->klass == TK_APPLY) {
        next_token();
        if (token->klass == TK_PARENTH_OPEN) {
          next_token();
          build_argumentList(&apply_expr->args);
          if (token->klass == TK_PARENTH_CLOSE) {
            next_token();
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
          } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `apply` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `.` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)apply_expr;
  } else error("at line %d: type name was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_prefixedNonTypeName()
{
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    Ast_Name* name = (Ast_Name*)build_nonTypeName(false);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
    return (Ast*)name;
  } else error("at line %d: non-type name was expected, ", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_arraySubscript()
{
  Ast_Subscript* subscript_expr = arena_push_struct(ast_storage, Ast_Subscript);
  subscript_expr->kind = AST_SUBSCRIPT;
  subscript_expr->id = node_id++;
  subscript_expr->line_no = token->line_no;
  if (token_is_expression(token)) {
    subscript_expr->index = build_expression(1);
  } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
  if (token->klass == TK_COLON) {
    next_token();
    if (token_is_expression(token)) {
      subscript_expr->end_index = build_expression(1);
    } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
  }
  return (Ast*)subscript_expr;
}

internal Ast*
build_lvalueExpr()
{
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    Ast_Name* dot_member = (Ast_Name*)build_name(false);
    dot_member->kind = AST_DOTNAME;
    return (Ast*)dot_member;
  } else if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    Ast* subscript_expr = build_arraySubscript();
    if (token->klass == TK_BRACKET_CLOSE) {
      next_token();
    } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
    return subscript_expr;
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_lvalue()
{
  if (token_is_lvalue(token)) {
    Ast* name = build_prefixedNonTypeName();
    Ast* lvalue = name;
    while(token->klass == TK_DOTPREFIX || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOTPREFIX) {
        next_token();
        Ast_MemberSelect* select_expr = arena_push_struct(ast_storage, Ast_MemberSelect);
        select_expr->kind = AST_MEMBER_SELECT;
        select_expr->id = node_id++;
        select_expr->line_no = token->line_no;
        select_expr->lhs_expr = lvalue;
        lvalue = (Ast*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_Subscript* subscript_expr = arena_push_struct(ast_storage, Ast_Subscript);
        subscript_expr->kind = AST_SUBSCRIPT;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->index = lvalue;
        subscript_expr->end_index = build_arraySubscript();
        lvalue = (Ast*)subscript_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    }
    return lvalue;
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    Ast_Expression* lvalue = (Ast_Expression*)build_lvalue();
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      build_typeArgumentList(&lvalue->type_args);
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    }
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      Ast_FunctionCall* call_stmt = arena_push_struct(ast_storage, Ast_FunctionCall);
      call_stmt->kind = AST_FUNCTION_CALL;
      call_stmt->id = node_id++;
      call_stmt->line_no = token->line_no;
      call_stmt->callee_expr = (Ast*)lvalue;
      build_argumentList(&call_stmt->args);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)call_stmt;
    } else if (token->klass == TK_EQUAL) {
      next_token();
      Ast_AssignmentStmt* assign_stmt = arena_push_struct(ast_storage, Ast_AssignmentStmt);
      assign_stmt->kind = AST_ASSIGNMENT_STMT;
      assign_stmt->id = node_id++;
      assign_stmt->line_no = token->line_no;
      assign_stmt->lvalue = (Ast*)lvalue;
      assign_stmt->expr = build_expression(1);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)assign_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserStatements()
{
  Ast_ElementList* stmts = arena_push_struct(ast_storage, Ast_ElementList);
  stmts->kind = AST_ELEM_LIST;
  stmts->id = node_id++;
  stmts->line_no = token->line_no;
  stmts->head.next = 0;
  if (token_is_parserStatement(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&stmts->head, li);
    li->object = build_parserStatement();
    while (token_is_parserStatement(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_parserStatement();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)stmts;
}

internal Ast*
build_parserBlockStatements()
{
  if (token->klass == TK_BRACE_OPEN) {
    Ast_BlockStmt* stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    build_parserStatements(&stmt->stmt_list);
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)stmt;
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserStatement()
{
  if (token_is_typeRef(token)) {
    Ast* type_ref = build_typeRef();
    if (token_is_name(token)) {
      Ast* stmt = build_variableDeclaration(type_ref);
      return stmt;
    } else {
      Ast* stmt = build_directApplication(type_ref);
      return stmt;
    }
  } else if (token_is_assignmentOrMethodCallStatement(token)) {
    Ast* stmt = build_assignmentOrMethodCallStatement();
    return stmt;
  } else if (token->klass == TK_BRACE_OPEN) {
    Ast* stmt = build_parserBlockStatements();
    return stmt;
  } else if (token->klass == TK_CONST) {
    Ast* stmt = build_constantDeclaration();
    return stmt;
  } else if (token->klass == TK_SEMICOLON) {
    Ast* stmt = arena_push_struct(ast_storage, Ast);
    stmt->kind = AST_EMPTY_ELEMENT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    return stmt;
  } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_expressionList()
{
  Ast_ElementList* exprs = arena_push_struct(ast_storage, Ast_ElementList);
  exprs->kind = AST_ELEM_LIST;
  exprs->id = node_id++;
  exprs->line_no = token->line_no;
  exprs->head.next = 0;
  if (token_is_expression(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&exprs->head, li);
    li->object = build_expression(1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_expression(1);
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)exprs;
}

internal Ast*
build_simpleKeysetExpression()
{
  if (token_is_expression(token)) {
    Ast* expr = build_expression(1);
    return expr;
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    Ast* expr = arena_push_struct(ast_storage, Ast);
    expr->kind = AST_DEFAULT_STMT;
    expr->id = node_id++;
    expr->line_no = token->line_no;
    return expr;
  } else if (token->klass == TK_DONTCARE) {
    next_token();
    Ast* expr = arena_push_struct(ast_storage, Ast);
    expr->kind = AST_DONTCARE;
    expr->id = node_id++;
    expr->line_no = token->line_no;
    return expr;
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_keysetExpressionList()
{
  Ast_ElementList* exprs = arena_push_struct(ast_storage, Ast_ElementList);
  exprs->kind = AST_ELEM_LIST;
  exprs->id = node_id++;
  exprs->line_no = token->line_no;
  exprs->head.next = 0;
  if (token_is_expression(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&exprs->head, li);
    li->object = build_simpleKeysetExpression();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_simpleKeysetExpression();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)exprs;
}

internal Ast*
build_tupleKeysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN) {
    Ast_TupleKeyset* tuple_keyset = arena_push_struct(ast_storage, Ast_TupleKeyset);
    tuple_keyset->kind = AST_TUPLE_KEYSET;
    tuple_keyset->id = node_id++;
    tuple_keyset->line_no = token->line_no;
    next_token();
    build_keysetExpressionList(&tuple_keyset->expr_list);
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)tuple_keyset;
  } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_keysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN) {
    Ast* expr = build_tupleKeysetExpression();
    return expr;
  } else if (token_is_simpleKeysetExpression(token)) {
    Ast* expr = build_simpleKeysetExpression();
    return expr;
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_selectCase()
{
  if (token_is_keysetExpression(token)) {
    Ast_SelectCase* select_case = arena_push_struct(ast_storage, Ast_SelectCase);
    select_case->kind = AST_SELECT_CASE;
    select_case->id = node_id++;
    select_case->line_no = token->line_no;
    select_case->keyset = build_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_name(token)) {
        select_case->name = build_name(false);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)select_case;
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_selectCaseList()
{
  Ast_ElementList* cases = arena_push_struct(ast_storage, Ast_ElementList);
  cases->kind = AST_ELEM_LIST;
  cases->id = node_id++;
  cases->line_no = token->line_no;
  cases->head.next = 0;
  if (token_is_selectCase(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&cases->head, li);
    li->object = build_selectCase();
    while (token_is_selectCase(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_selectCase();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)cases;
}

internal Ast*
build_selectExpression()
{
  if (token->klass == TK_SELECT) {
    next_token();
    Ast_SelectExpr* select_expr = arena_push_struct(ast_storage, Ast_SelectExpr);
    select_expr->kind = AST_SELECT_EXPR;
    select_expr->id = node_id++;
    select_expr->line_no = token->line_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      build_expressionList(&select_expr->expr_list);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          build_selectCaseList(&select_expr->case_list);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)select_expr;
  } else error("at line %d: `select` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_stateExpression()
{
  if (token_is_name(token)) {
    Ast* state_expr = build_name(false);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    return state_expr;
  } else if (token->klass == TK_SELECT) {
    Ast* state_expr = build_selectExpression();
    return state_expr;
  } else error("at line %d: state expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_transitionStatement()
{
  if (token->klass == TK_TRANSITION) {
    next_token();
    Ast* stmt = build_stateExpression();
    return stmt;
  } else error("at line %d: `transition` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserState()
{
  if (token->klass == TK_STATE) {
    next_token();
    Ast_ParserState* state = arena_push_struct(ast_storage, Ast_ParserState);
    state->kind = AST_PARSER_STATE;
    state->id = node_id++;
    state->line_no = token->line_no;
    state->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      build_parserStatements(&state->stmt_list);
      state->trans_stmt = build_transitionStatement();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)state;
  } else error("at line %d: `state` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_parserStates()
{
  Ast_ElementList* states = arena_push_struct(ast_storage, Ast_ElementList);
  states->kind = AST_ELEM_LIST;
  states->id = node_id++;
  states->line_no = token->line_no;
  states->head.next = 0;
  if (token->klass == TK_STATE) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&states->head, li);
    li->object = build_parserState();
    while (token->klass == TK_STATE) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_parserState();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)states;
}

internal Ast*
build_parserDeclaration()
{
  if (token->klass == TK_PARSER) {
    Ast_Parser* decl = arena_push_struct(ast_storage, Ast_Parser);
    decl->kind = AST_PARSER;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->type_decl = build_parserTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <parserTypeDeclaration> */
      return decl->type_decl;
    } else {
      build_optConstructorParameters(&decl->ctor_params);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_parserLocalElements(&decl->local_elements);
        if (token->klass == TK_STATE) {
          build_parserStates(&decl->states);
        } else error("at line %d: `state` was expected, got `%s`.", token->line_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
    return (Ast*)decl;
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_controlTypeDeclaration()
{
  if (token->klass == TK_CONTROL) {
    next_token();
    Ast_ControlProto* decl = arena_push_struct(ast_storage, Ast_ControlProto);
    decl->kind = AST_CONTROL_PROTO;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      build_optTypeParameters(&decl->type_params);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&decl->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `control` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_actionDeclaration()
{
  if (token->klass == TK_ACTION) {
    next_token();
    Ast_Action* decl = arena_push_struct(ast_storage, Ast_Action);
    decl->kind = AST_ACTION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(false);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&decl->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TK_BRACE_OPEN) {
            decl->stmt = build_blockStatement();
          } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `action` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_keyElement()
{
  if (token_is_expression(token)) {
    Ast_KeyElement* key_elem = arena_push_struct(ast_storage, Ast_KeyElement);
    key_elem->kind = AST_KEY_ELEMENT;
    key_elem->id = node_id++;
    key_elem->line_no = token->line_no;
    key_elem->expr = build_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->name = build_name(false);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)key_elem;
  } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_keyElementList()
{
  Ast_ElementList* elems = arena_push_struct(ast_storage, Ast_ElementList);
  elems->kind = AST_ELEM_LIST;
  elems->id = node_id++;
  elems->line_no = token->line_no;
  elems->head.next = 0;
  if (token_is_expression(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&elems->head, li);
    li->object = build_keyElement();
    while (token_is_expression(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_keyElement();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)elems;
}

internal Ast*
build_actionRef()
{
  if (token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)) {
    Ast_ActionRef* ref = arena_push_struct(ast_storage, Ast_ActionRef);
    ref->kind = AST_ACTION_REF;
    ref->id = node_id++;
    ref->line_no = token->line_no;
    ref->name = build_prefixedNonTypeName();
    build_optArguments(&ref->args);
    return (Ast*)ref;
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_actionList()
{
  Ast_ElementList* actions = arena_push_struct(ast_storage, Ast_ElementList);
  actions->kind = AST_ELEM_LIST;
  actions->id = node_id++;
  actions->line_no = token->line_no;
  actions->head.next = 0;
  if (token_is_actionRef(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&actions->head, li);
    li->object = build_actionRef();
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    while (token_is_actionRef(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_actionRef();
      dlist_concat(last, li);
      last = li;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  }
  return (Ast*)actions;
}

internal Ast*
build_entry()
{
  if (token_is_keysetExpression(token)) {
    Ast_TableEntry* entry = arena_push_struct(ast_storage, Ast_TableEntry);
    entry->kind = AST_TABLE_ENTRY;
    entry->id = node_id++;
    entry->line_no = token->line_no;
    entry->keyset = build_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      entry->action = build_actionRef();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)entry;
  } else error("at line %d: keyset was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_entriesList()
{
  Ast_ElementList* entries = arena_push_struct(ast_storage, Ast_ElementList);
  entries->kind = AST_ELEM_LIST;
  entries->id = node_id++;
  entries->line_no = token->line_no;
  entries->head.next = 0;
  if (token_is_keysetExpression(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&entries->head, li);
    li->object = build_entry();
    while (token_is_keysetExpression(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_entry();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)entries;
}

internal Ast*
build_tableProperty()
{
  if (token_is_tableProperty(token)) {
    bool is_const = false;
    if (token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    if (token->klass == TK_KEY) {
      next_token();
      Ast_TableKey* key_prop = arena_push_struct(ast_storage, Ast_TableKey);
      key_prop->kind = AST_TABLE_KEY;
      key_prop->id = node_id++;
      key_prop->line_no = token->line_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          build_keyElementList(&key_prop->keyelem_list);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)key_prop;
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      Ast_TableActions* actions_prop = arena_push_struct(ast_storage, Ast_TableActions);
      actions_prop->kind = AST_TABLE_ACTIONS;
      actions_prop->id = node_id++;
      actions_prop->line_no = token->line_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          build_actionList(&actions_prop->action_list);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)actions_prop;
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      Ast_TableEntries* entries_prop = arena_push_struct(ast_storage, Ast_TableEntries);
      entries_prop->kind = AST_TABLE_ENTRIES;
      entries_prop->id = node_id++;
      entries_prop->line_no = token->line_no;
      entries_prop->is_const = is_const;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            build_entriesList(&entries_prop->entries);
          } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)entries_prop;
    } else if (token_is_nonTableKwName(token)) {
      Ast_TableSingleEntry* entry_prop = arena_push_struct(ast_storage, Ast_TableSingleEntry);
      entry_prop->kind = AST_TABLE_SINGLE_ENTRY;
      entry_prop->id = node_id++;
      entry_prop->line_no = token->line_no;
      entry_prop->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        entry_prop->init_expr = build_initializer();
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)entry_prop;
    } else assert(0);
  } else error("at line %d: table property was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_tablePropertyList()
{
  Ast_ElementList* props = arena_push_struct(ast_storage, Ast_ElementList);
  props->kind = AST_ELEM_LIST;
  props->id = node_id++;
  props->line_no = token->line_no;
  props->head.next = 0;
  if (token_is_tableProperty(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&props->head, li);
    li->object = build_tableProperty();
    while (token_is_tableProperty(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_tableProperty();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)props;
}

internal Ast*
build_tableDeclaration()
{
  if (token->klass == TK_TABLE) {
    next_token();
    Ast_Table* table = arena_push_struct(ast_storage, Ast_Table);
    table->kind = AST_TABLE;
    table->id = node_id++;
    table->line_no = token->line_no;
    table->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_tableProperty(token)) {
        build_tablePropertyList(&table->prop_list);
      } else error("at line %d: table property was expected, got `%s`.", token->line_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)table;
  } else error("at line %d: `table` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_controlLocalDeclaration()
{
  if (token->klass == TK_CONST) {
    Ast* decl = build_constantDeclaration();
    return decl;
  } else if (token->klass == TK_ACTION) {
    Ast* decl = build_actionDeclaration();
    return decl;
  } else if (token->klass == TK_TABLE) {
    Ast* decl = build_tableDeclaration();
    return decl;
  } else if (token_is_typeRef(token)) {
    Ast* type_ref = build_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      Ast* decl = build_instantiation(type_ref);
      return decl;
    } else if (token_is_name(token)) {
      Ast* decl = build_variableDeclaration(type_ref);
      return decl;
    } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_controlLocalDeclarations()
{
  Ast_ElementList* decls = arena_push_struct(ast_storage, Ast_ElementList);
  decls->kind = AST_ELEM_LIST;
  decls->id = node_id++;
  decls->line_no = token->line_no;
  decls->head.next = 0;
  if (token_is_controlLocalDeclaration(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&decls->head, li);
    li->object = build_controlLocalDeclaration();
    while (token_is_controlLocalDeclaration(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_controlLocalDeclaration();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)decls;
}

internal Ast*
build_controlDeclaration()
{
  if (token->klass == TK_CONTROL) {
    Ast_Control* decl = arena_push_struct(ast_storage, Ast_Control);
    decl->kind = AST_CONTROL;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->type_decl = build_controlTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <controlTypeDeclaration> */
      return decl->type_decl;
    } else {
      build_optConstructorParameters(&decl->ctor_params);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        build_controlLocalDeclarations(&decl->local_decls);
        if (token->klass == TK_APPLY) {
          next_token();
          decl->apply_stmt = build_blockStatement();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `apply` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
    return (Ast*)decl;
  } else error("at line %d: `control` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_packageTypeDeclaration()
{
  if (token->klass == TK_PACKAGE) {
    next_token();
    Ast_Package* decl = arena_push_struct(ast_storage, Ast_Package);
    decl->kind = AST_PACKAGE;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      build_optTypeParameters(&decl->type_params);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        build_parameterList(&decl->params);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `package` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_typedefDeclaration()
{
  if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
    bool is_typedef = false;
    if (token->klass == TK_TYPEDEF) {
      is_typedef = true;
      next_token();
    } else if (token->klass == TK_TYPE) {
      next_token();
    } else assert(0);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      Ast_Type* type_decl = arena_push_struct(ast_storage, Ast_Type);
      type_decl->kind = AST_TYPE;
      type_decl->id = node_id++;
      type_decl->line_no = token->line_no;
      type_decl->is_typedef = is_typedef;
      if (token_is_typeRef(token)) {
        type_decl->type_ref = build_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->type_ref = build_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        type_decl->name = build_name(true);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)type_decl;
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type definition was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_typeDeclaration()
{
  if (token_is_typeDeclaration(token)) {
    if (token_is_derivedTypeDeclaration(token)) {
      Ast* decl = build_derivedTypeDeclaration();
      return decl;
    } else if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
      Ast* decl = build_typedefDeclaration();
      return decl;
    } else if (token->klass == TK_PARSER) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      Ast* decl = build_parserDeclaration();
      return decl;
    } else if (token->klass == TK_CONTROL) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      Ast* decl = build_controlDeclaration();
      return decl;
    } else if (token->klass == TK_PACKAGE) {
      Ast* decl = build_packageTypeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
      return decl;
    } else assert(0);
  } else error("at line %d: type declaration was expected, got `%s`.", token->line_no, token->lexeme); 
  assert(0);
  return 0;
}

internal Ast*
build_conditionalStatement()
{
  if (token->klass == TK_IF) {
    next_token();
    Ast_IfStmt* if_stmt = arena_push_struct(ast_storage, Ast_IfStmt);
    if_stmt->kind = AST_IF_STMT;
    if_stmt->id = node_id++;
    if_stmt->line_no = token->line_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(token)) {
        if_stmt->cond_expr = build_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token_is_statement(token)) {
            if_stmt->stmt = build_statement(0);
            if (token->klass == TK_ELSE) {
              next_token();
              if (token_is_statement(token)) {
                if_stmt->else_stmt = build_statement(0);
              } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
            }
          } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)if_stmt;
  } else error("at line %d: `if` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_exitStatement()
{
  if (token->klass == TK_EXIT) {
    next_token();
    Ast* exit_stmt = arena_push_struct(ast_storage, Ast);
    exit_stmt->kind = AST_EXIT_STMT;
    exit_stmt->id = node_id++;
    exit_stmt->line_no = token->line_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
    return exit_stmt;
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_returnStatement()
{
  if (token->klass == TK_RETURN) {
    next_token();
    Ast_ReturnStmt* ret_stmt = arena_push_struct(ast_storage, Ast_ReturnStmt);
    ret_stmt->kind = AST_RETURN_STMT;
    ret_stmt->id = node_id++;
    ret_stmt->line_no = token->line_no;
    if (token_is_expression(token))
      ret_stmt->expr = build_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)ret_stmt;
  } else error("at line %d: `return` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_switchLabel()
{
  if (token_is_name(token)) {
    Ast* label = build_name(false);
    return label;
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    Ast* label = arena_push_struct(ast_storage, Ast);
    label->kind = AST_DEFAULT_STMT;
    label->id = node_id++;
    label->line_no = token->line_no;
    return label;
  } else error("at line %d: switch label was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_switchCase()
{
  if (token_is_switchLabel(token)) {
    Ast_SwitchCase* switch_case = arena_push_struct(ast_storage, Ast_SwitchCase);
    switch_case->kind = AST_SWITCH_CASE;
    switch_case->id = node_id++;
    switch_case->line_no = token->line_no;
    switch_case->label = build_switchLabel();
    if (token->klass == TK_COLON) {
      next_token();
      if (token->klass == TK_BRACE_OPEN) {
        switch_case->stmt = build_blockStatement();
      }
    } else error("at line %d: `:` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)switch_case;
  } else error("at line %d: switch label was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_switchCases()
{
  Ast_ElementList* cases = arena_push_struct(ast_storage, Ast_ElementList);
  cases->kind = AST_ELEM_LIST;
  cases->id = node_id++;
  cases->line_no = token->line_no;
  cases->head.next = 0;
  if (token_is_switchLabel(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&cases->head, li);
    li->object = build_switchCase();
    while (token_is_switchLabel(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_switchCase();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)cases;
}

internal Ast*
build_switchStatement()
{
  if (token->klass == TK_SWITCH) {
    next_token();
    Ast_SwitchStmt* stmt = arena_push_struct(ast_storage, Ast_SwitchStmt);
    stmt->kind = AST_SWITCH_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      stmt->expr = build_expression(1);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          build_switchCases(&stmt->switch_cases);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)stmt;
  } else error("at line %d: `switch` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_statement(Ast* type_name)
{
  if (token_is_typeName(token) || type_name) {
    Ast* stmt = build_directApplication(type_name);
    return stmt;
  } else if (token_is_assignmentOrMethodCallStatement(token)) {
    Ast* stmt = build_assignmentOrMethodCallStatement();
    return stmt;
  } else if (token->klass == TK_IF) {
    Ast* stmt = build_conditionalStatement();
    return stmt;
  } else if (token->klass == TK_SEMICOLON) {
    next_token();
    Ast* stmt = arena_push_struct(ast_storage, Ast);
    stmt->kind = AST_EMPTY_ELEMENT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    return stmt;
  } else if (token->klass == TK_BRACE_OPEN) {
    Ast* stmt = build_blockStatement();
    return stmt;
  } else if (token->klass == TK_EXIT) {
    Ast* stmt = build_exitStatement();
    return stmt;
  } else if (token->klass == TK_RETURN) {
    Ast* stmt = build_returnStatement();
    return stmt;
  } else if (token->klass == TK_SWITCH) {
    Ast* stmt = build_switchStatement();
    return stmt;
  } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_statementOrDecl()
{
  if (token_is_statementOrDeclaration(token)) {
    if (token_is_typeRef(token)) {
      Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        Ast* stmt = build_instantiation(type_ref);
        return stmt;
      } else if (token_is_name(token)) {
        Ast* stmt = build_variableDeclaration(type_ref);
        return stmt;
      } else {
        Ast* stmt = build_statement(type_ref);
        return stmt;
      }
    } else if (token_is_statement(token)) {
      Ast* stmt = build_statement(0);
      return stmt;
    } else if (token->klass == TK_CONST) {
      Ast* stmt = build_constantDeclaration();
      return stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

internal Ast*
build_statementOrDeclList()
{
  Ast_ElementList* stmts = arena_push_struct(ast_storage, Ast_ElementList);
  stmts->kind = AST_ELEM_LIST;
  stmts->id = node_id++;
  stmts->line_no = token->line_no;
  stmts->head.next = 0;
  if (token_is_statementOrDeclaration(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&stmts->head, li);
    li->object = build_statementOrDecl();
    while (token_is_statementOrDeclaration(token)) {
      li = arena_push_struct(ast_storage, DList);
      li->object = build_statementOrDecl();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)stmts;
}

internal Ast*
build_blockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    Ast_BlockStmt* stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    build_statementOrDeclList(&stmt->stmt_list);
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)stmt;
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_identifierList()
{
  Ast_ElementList* ids = arena_push_struct(ast_storage, Ast_ElementList);
  ids->kind = AST_ELEM_LIST;
  ids->id = node_id++;
  ids->line_no = token->line_no;
  ids->head.next = 0;
  if (token_is_name(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&ids->head, li);
    li->object = build_name(false);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_name(false);
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)ids;
}

internal Ast*
build_errorDeclaration()
{
  if (token->klass == TK_ERROR) {
    next_token();
    Ast_Error* decl = arena_push_struct(ast_storage, Ast_Error);
    decl->kind = AST_ERROR;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        if (token_is_name(token)) {
          build_identifierList(&decl->id_list);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `error` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_matchKindDeclaration()
{
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    Ast_MatchKind* decl = arena_push_struct(ast_storage, Ast_MatchKind);
    decl->kind = AST_MATCH_KIND;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        build_identifierList(&decl->id_list);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: `match_kind` was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_functionDeclaration(Ast* type_ref)
{
  if (token_is_typeOrVoid(token)) {
    Ast_Function* decl = arena_push_struct(ast_storage, Ast_Function);
    decl->kind = AST_FUNCTION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->proto = build_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      decl->stmt = build_blockStatement();
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    return (Ast*)decl;
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_declaration()
{
  if (token_is_declaration(token)) {
    if (token->klass == TK_CONST) {
      Ast* decl = build_constantDeclaration();
      return decl;
    } else if (token->klass == TK_EXTERN) {
      Ast* decl = build_externDeclaration();
      return decl;
    } else if (token->klass == TK_ACTION) {
      Ast* decl = build_actionDeclaration();
      return decl;
    } else if (token_is_typeDeclaration(token)) {
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      Ast* decl = build_typeDeclaration();
      return decl;
    } else if (token->klass == TK_ERROR) {
      Ast* decl = build_errorDeclaration();
      return decl;
    } else if (token->klass == TK_MATCH_KIND) {
      Ast* decl = build_matchKindDeclaration();
      return decl;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        Ast* decl = build_instantiation(type_ref);
        return decl;
      } else if (token_is_name(token)) {
        Ast* decl = build_functionDeclaration(type_ref);
        return decl;
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(token)) {
      Ast* decl = build_functionDeclaration(build_typeRef());
      return decl;
    } else assert(0);
  } else error("at line %d: top-level declaration as expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_declarationList()
{
  Ast_ElementList* decls = arena_push_struct(ast_storage, Ast_ElementList);
  decls->kind = AST_ELEM_LIST;
  decls->id = node_id++;
  decls->line_no = token->line_no;
  decls->head.next = 0;
  if (token_is_declaration(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&decls->head, li);
    li->object = build_declaration();
    while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
      if (token_is_declaration(token)) {
        li = arena_push_struct(ast_storage, DList);
        li->object = build_declaration();
        dlist_concat(last, li);
        last = li;
      } else if (token->klass == TK_SEMICOLON) {
        next_token(); /* empty declaration */
      }
    }
  }
  return (Ast*)decls;
}

internal Ast*
build_p4program()
{
  Ast_P4Program* program = arena_push_struct(ast_storage, Ast_P4Program);
  program->kind = AST_P4PROGRAM;
  program->id = node_id++;
  program->line_no = token->line_no;
  while (token->klass == TK_SEMICOLON) {
    next_token(); /* empty declaration */
  }
  build_declarationList(&program->decl_list);
  if (token->klass != TK_END_OF_INPUT) {
    error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
  }
  return (Ast*)program;
}

internal bool
token_is_realTypeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE|| token_is_typeRef(token);
  return result;
}

internal bool
token_is_binaryOperator(Token* token)
{
  bool result = token->klass == TK_STAR || token->klass == TK_SLASH
    || token->klass == TK_PLUS || token->klass == TK_MINUS
    || token->klass == TK_ANGLE_OPEN_EQUAL || token->klass == TK_ANGLE_CLOSE_EQUAL
    || token->klass == TK_ANGLE_OPEN || token->klass == TK_ANGLE_CLOSE
    || token->klass == TK_EXCLAMATION_EQUAL || token->klass == TK_DOUBLE_EQUAL
    || token->klass == TK_DOUBLE_PIPE || token->klass == TK_DOUBLE_AMPERSAND
    || token->klass == TK_PIPE || token->klass == TK_AMPERSAND
    || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN
    || token->klass == TK_DOUBLE_ANGLE_CLOSE || token->klass == TK_TRIPLE_AMPERSAND
    || token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_exprOperator(Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOTPREFIX
    || token->klass == TK_BRACKET_OPEN || token->klass == TK_PARENTH_OPEN
    || token->klass == TK_ANGLE_OPEN;
  return result;
}

internal Ast*
build_realTypeArg()
{
  if (token->klass == TK_DONTCARE) {
    next_token();
    Ast* arg = arena_push_struct(ast_storage, Ast);
    arg->kind = AST_DONTCARE;
    arg->id = node_id++;
    arg->line_no = token->line_no;
    return arg;
  } else if (token_is_typeRef(token)) {
    Ast* arg = build_typeRef();
    return arg;
  } else error("at line %d: type argument was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
build_realTypeArgumentList()
{
  Ast_ElementList* args = arena_push_struct(ast_storage, Ast_ElementList);
  args->kind = AST_ELEM_LIST;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->head.next = 0;
  if (token_is_realTypeArg(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    dlist_concat(&args->head, li);
    li->object = build_realTypeArg();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_realTypeArg();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)args;
}

internal Ast*
build_expressionPrimary()
{
  if (token_is_expression(token)) {
    if (token->klass == TK_INT_LITERAL) {
      Ast* primary = build_integer();
      return primary;
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      Ast* primary = build_boolean();
      return primary;
    } else if (token->klass == TK_STRING_LITERAL) {
      Ast* primary = build_stringLiteral();
      return primary;
    } else if (token->klass == TK_DOTPREFIX) {
      next_token();
      if (token->klass == TK_IDENTIFIER) {
        Ast_Name* name = (Ast_Name*)build_nonTypeName(false);
        name->kind = AST_DOTNAME;
        return (Ast*)name;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        Ast_Name* name = (Ast_Name*)build_typeName(false);
        name->kind = AST_DOTNAME;
        return (Ast*)name;
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(token)) {
      Ast* primary = build_nonTypeName(false);
      return primary;
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      Ast_ExprList* expr_list = arena_push_struct(ast_storage, Ast_ExprList);
      expr_list->kind = AST_EXPRLIST;
      expr_list->id = node_id++;
      expr_list->line_no = token->line_no;
      build_expressionList(&expr_list->expr_list);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      return (Ast*)expr_list;
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        Ast_CastExpr* cast_expr = arena_push_struct(ast_storage, Ast_CastExpr);
        cast_expr->kind = AST_CAST_EXPR;
        cast_expr->id = node_id++;
        cast_expr->line_no = token->line_no;
        cast_expr->to_type = build_typeRef();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->expr = build_expression(1);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
        return (Ast*)cast_expr;
      } else if (token_is_expression(token)) {
        Ast* primary = build_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
        return primary;
      } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
      assert(0);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NOT;
      unary_expr->operand = build_expression(1);
      return (Ast*)unary_expr;
    } else if (token->klass == TK_TILDA) {
      next_token();
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_BITWISE_NOT;
      unary_expr->operand = build_expression(1);
      return (Ast*)unary_expr;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NEG;
      unary_expr->operand = build_expression(1);
      return (Ast*)unary_expr;
    } else if (token_is_typeName(token)) {
      Ast* primary = build_typeName();
      return primary;
    } else if (token->klass == TK_ERROR) {
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_NAME;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->strname = token->lexeme;
      next_token();
      return (Ast*)name;
    } else assert(0);
    assert(0);
  } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

internal int
get_operator_priority(Token* token)
{
  if (token->klass == TK_DOUBLE_AMPERSAND || token->klass == TK_DOUBLE_PIPE) {
    /* Logical AND, OR */
    return 1;
  } else if (token->klass == TK_DOUBLE_EQUAL || token->klass == TK_EXCLAMATION_EQUAL
      || token->klass == TK_ANGLE_OPEN /* Less */ || token->klass == TK_ANGLE_CLOSE /* Greater */
      || token->klass == TK_ANGLE_OPEN_EQUAL /* Less-equal */ || token->klass == TK_ANGLE_CLOSE_EQUAL /* Greater-equal */) {
    /* Relational ops  */
    return 2;
  }
  else if (token->klass == TK_PLUS || token->klass == TK_MINUS
           || token->klass == TK_AMPERSAND || token->klass == TK_PIPE
           || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN /* BitshiftLeft */
           || token->klass == TK_DOUBLE_ANGLE_CLOSE /* BitshiftRight */) {
    /* Addition and subtraction; bitwise ops */
    return 3;
  }
  else if (token->klass == TK_STAR || token->klass == TK_SLASH) {
    /* Multiplication and division */
    return 4;
  }
  else if (token->klass == TK_TRIPLE_AMPERSAND) {
    /* Masking */
    return 5;
  }
  else assert(0);
  return 0;
}

internal enum AstOperator
token_to_binop(Token* token)
{
  switch (token->klass) {
    case TK_DOUBLE_AMPERSAND:
      return OP_AND;
    case TK_DOUBLE_PIPE:
      return OP_OR;
    case TK_DOUBLE_EQUAL:
      return OP_EQUAL;
    case TK_EXCLAMATION_EQUAL:
      return OP_NOT_EQUAL;
    case TK_ANGLE_OPEN:
      return OP_LESS;
    case TK_ANGLE_CLOSE:
      return OP_GREATER;
    case TK_ANGLE_OPEN_EQUAL:
      return OP_LESS_EQUAL;
    case TK_ANGLE_CLOSE_EQUAL:
      return OP_GREATER_EQUAL;
    case TK_PLUS:
      return OP_ADD;
    case TK_MINUS:
      return OP_SUB;
    case TK_STAR:
      return OP_MUL;
    case TK_SLASH:
      return OP_DIV;
    case TK_AMPERSAND:
      return OP_BITWISE_AND;
    case TK_PIPE:
      return OP_BITWISE_OR;
    case TK_CIRCUMFLEX:
      return OP_BITWISE_XOR;
    case TK_DOUBLE_ANGLE_OPEN:
      return OP_BITWISE_SHIFT_LEFT;
    case TK_DOUBLE_ANGLE_CLOSE:
      return OP_BITWISE_SHIFT_RIGHT;
    case TK_TRIPLE_AMPERSAND:
      return OP_MASK;
    default: return 0;
  }
}

internal Ast*
build_expression(int priority_threshold)
{
  if (token_is_expression(token)) {
    Ast_Expression* expr = (Ast_Expression*)build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOTPREFIX) {
        next_token();
        Ast_MemberSelect* select_expr = arena_push_struct(ast_storage, Ast_MemberSelect);
        select_expr->kind = AST_MEMBER_SELECT;
        select_expr->id = node_id++;
        select_expr->line_no = token->line_no;
        select_expr->lhs_expr = (Ast*)expr;
        expr = (Ast_Expression*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_Subscript* subscript_expr = arena_push_struct(ast_storage, Ast_Subscript);
        subscript_expr->kind = AST_SUBSCRIPT;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->index = (Ast*)expr;
        subscript_expr->end_index = build_arraySubscript();
        expr = (Ast_Expression*)subscript_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        Ast_FunctionCall* call_expr = arena_push_struct(ast_storage, Ast_FunctionCall);
        call_expr->kind = AST_FUNCTION_CALL;
        call_expr->id = node_id++;
        call_expr->line_no = token->line_no;
        call_expr->callee_expr = (Ast*)expr;
        build_argumentList(&call_expr->args);
        expr = (Ast_Expression*)call_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        build_realTypeArgumentList(&expr->type_args);
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        Ast_KVPair* kv_pair = arena_push_struct(ast_storage, Ast_KVPair);
        kv_pair->kind = AST_KVPAIR;
        kv_pair->id = node_id++;
        kv_pair->line_no = token->line_no;
        kv_pair->name = (Ast*)expr;
        kv_pair->expr = build_expression(1);
        expr = (Ast_Expression*)kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          Ast_BinaryExpr* bin_expr = arena_push_struct(ast_storage, Ast_BinaryExpr);
          bin_expr->kind = AST_BINARY_EXPR;
          bin_expr->id = node_id++;
          bin_expr->line_no = token->line_no;
          bin_expr->left_operand = (Ast*)expr;
          bin_expr->op = token_to_binop(token);
          next_token();
          bin_expr->right_operand = build_expression(priority + 1);
          expr = (Ast_Expression*)bin_expr;
        } else break;
      } else assert(0);
    }
    return (Ast*)expr;
  } else error("at line %d: expression was expected, got `%s`.", token->line_no, token->lexeme);
  assert(0);
  return 0;
}

Ast_P4Program*
build_ast(UnboundedArray* tokens_array_, Arena* ast_storage_)
{
  NameDecl*
  declare_keyword(char* strname, enum TokenClass token_class)
  {
    NameDecl* decl = arena_push_struct(ast_storage, NameDecl);
    decl->strname = strname;
    decl->token_class = token_class;
    declare_name_in_scope(root_scope, NAMESPACE_KEYWORD, decl);
    return decl;
  }

  tokens_array = tokens_array_;
  ast_storage = ast_storage_;
  scope_init(ast_storage);
  root_scope = current_scope = push_scope();

  declare_keyword("action", TK_ACTION);
  declare_keyword("actions", TK_ACTIONS);
  declare_keyword("entries", TK_ENTRIES);
  declare_keyword("enum", TK_ENUM);
  declare_keyword("in", TK_IN);
  declare_keyword("package", TK_PACKAGE);
  declare_keyword("select", TK_SELECT);
  declare_keyword("switch", TK_SWITCH);
  declare_keyword("tuple", TK_TUPLE);
  declare_keyword("control", TK_CONTROL);
  declare_keyword("error", TK_ERROR);
  declare_keyword("header", TK_HEADER);
  declare_keyword("inout", TK_INOUT);
  declare_keyword("parser", TK_PARSER);
  declare_keyword("state", TK_STATE);
  declare_keyword("table", TK_TABLE);
  declare_keyword("key", TK_KEY);
  declare_keyword("typedef", TK_TYPEDEF);
  declare_keyword("type", TK_TYPE);
  declare_keyword("default", TK_DEFAULT);
  declare_keyword("extern", TK_EXTERN);
  declare_keyword("header_union", TK_HEADER_UNION);
  declare_keyword("out", TK_OUT);
  declare_keyword("transition", TK_TRANSITION);
  declare_keyword("else", TK_ELSE);
  declare_keyword("exit", TK_EXIT);
  declare_keyword("if", TK_IF);
  declare_keyword("match_kind", TK_MATCH_KIND);
  declare_keyword("return", TK_RETURN);
  declare_keyword("struct", TK_STRUCT);
  declare_keyword("apply", TK_APPLY);
  declare_keyword("const", TK_CONST);
  declare_keyword("bool", TK_BOOL);
  declare_keyword("true", TK_TRUE);
  declare_keyword("false", TK_FALSE);
  declare_keyword("void", TK_VOID);
  declare_keyword("int", TK_INT);
  declare_keyword("bit", TK_BIT);
  declare_keyword("varbit", TK_VARBIT);
  declare_keyword("string", TK_STRING);

  token_at = 0;
  token = array_get(tokens_array, token_at);
  next_token();
  Ast_P4Program* p4program = (Ast_P4Program*)build_p4program();
  p4program->last_node_id = node_id;
  current_scope = pop_scope();
  assert(current_scope == 0);

  return p4program;
}
