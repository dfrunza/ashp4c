#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "ast.h"

internal struct Arena *ast_storage;
internal struct UnboundedArray* tokens_array;
internal int token_at = 0;
internal struct Token* token = 0;
internal int prev_token_at = 0;
internal struct Token* prev_token = 0;
internal int node_id = 0;
internal struct Scope* root_scope;
internal struct Scope* current_scope;

internal struct Ast* build_expression(int priority_threshold);
internal struct Ast* build_typeRef();
internal struct Ast* build_blockStatement();
internal struct Ast* build_statement(struct Ast* type_name);
internal struct Ast* build_parserStatement();

internal struct Token*
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
    struct NameEntry* ne = scope_lookup_name(current_scope, token->lexeme);
    if (ne->ns_keyword) {
      struct NameDecl* nd = ne->ns_keyword;
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

internal struct Token*
peek_token()
{
  prev_token = token;
  prev_token_at = token_at;
  struct Token* peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

internal bool
token_is_typeName(struct Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOTPREFIX;
}

internal bool
token_is_prefixedType(struct Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOTPREFIX;
}

internal bool
token_is_baseType(struct Token* token)
{
  bool result = token->klass == TK_BOOL || token->klass == TK_ERROR || token->klass == TK_INT
    || token->klass == TK_BIT || token->klass == TK_VARBIT || token->klass == TK_STRING
    || token->klass == TK_VOID;
  return result;
}

internal bool
token_is_typeRef(struct Token* token)
{
  bool result = token_is_baseType(token) || token_is_prefixedType(token) || token->klass == TK_TUPLE;
  return result;
}

internal bool
token_is_direction(struct Token* token)
{
  bool result = token->klass == TK_IN || token->klass == TK_OUT || token->klass == TK_INOUT;
  return result;
}

internal bool
token_is_parameter(struct Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

internal bool
token_is_derivedTypeDeclaration(struct Token* token)
{
  bool result = token->klass == TK_HEADER || token->klass == TK_HEADER_UNION || token->klass == TK_STRUCT
    || token->klass == TK_ENUM;
  return result;
}

internal bool
token_is_typeDeclaration(struct Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF || token->klass == TK_TYPE
    || token->klass == TK_PARSER || token->klass == TK_CONTROL || token->klass == TK_PACKAGE;
  return result;
}

internal bool
token_is_nonTypeName(struct Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_name(struct Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

internal bool
token_is_nonTableKwName(struct Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_typeArg(struct Token* token)
{
  bool result = token->klass == TK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

internal bool
token_is_typeParameterList(struct Token* token)
{
  return token_is_name(token);
}

internal bool
token_is_typeOrVoid(struct Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_VOID || token->klass == TK_IDENTIFIER;
  return result;
}

internal bool
token_is_actionRef(struct Token* token)
{
  bool result = token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)
    || token->klass == TK_PARENTH_OPEN;
  return result;
}

internal bool
token_is_tableProperty(struct Token* token)
{
  bool result = token->klass == TK_KEY || token->klass == TK_ACTIONS
    || token->klass == TK_CONST || token->klass == TK_ENTRIES
    || token_is_nonTableKwName(token);
  return result;
}

internal bool
token_is_switchLabel(struct Token* token)
{
  bool result = token_is_name(token) || token->klass == TK_DEFAULT;
  return result;
}

internal bool
token_is_expressionPrimary(struct Token* token)
{
  bool result = token->klass == TK_INT_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token_is_prefixedType(token);
  return result;
}

internal bool
token_is_expression(struct Token* token)
{
  return token_is_expressionPrimary(token);
}

internal bool
token_is_methodPrototype(struct Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

internal struct Ast*
build_nonTypeName(bool is_type)
{
  struct Ast_Name* name = 0;
  if (token_is_nonTypeName(token)) {
    name = arena_push_struct(ast_storage, struct Ast_Name);
    name->kind = AST_NAME;
    name->id = node_id++;
    name->line_no = token->line_no;
    name->strname = token->lexeme;
    if (is_type) {
      struct NameDecl* decl = arena_push_struct(ast_storage, struct NameDecl);
      decl->ast = (struct Ast*)name;
      decl->strname = name->strname;
      decl->line_no = token->line_no;
      declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
    }
    next_token();
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_name(bool is_type)
{
  struct Ast_Name* name = 0;
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      name = (struct Ast_Name*)build_nonTypeName(is_type);
    } else if (token->klass == TK_TYPE_IDENTIFIER) {
      struct Ast_Name* type_name = arena_push_struct(ast_storage, struct Ast_Name);
      type_name->kind = AST_NAME;
      type_name->id = node_id++;
      type_name->line_no = token->line_no;
      type_name->strname = token->lexeme;
      name = type_name;
      next_token();
    } else assert(0);
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)name;
}

internal struct List*
build_typeParameterList()
{
  struct List* params = 0;
  if (token_is_typeParameterList(token)) {
    params = arena_push_struct(ast_storage, struct List);
    list_init(params);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_name(true);
    list_append_link(params, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_name(true);
      list_append_link(params, li);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  return params;
}

internal struct List*
build_optTypeParameters()
{
  struct List* params = 0;
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    if (token_is_typeParameterList(token)) {
      params = build_typeParameterList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  }
  return params;
}

internal struct Ast*
build_typeArg()
{
  struct Ast* arg = 0;
  if (token_is_typeArg(token))
  {
    if (token->klass == TK_DONTCARE) {
      struct Ast_Dontcare* dontcare = arena_push_struct(ast_storage, struct Ast_Dontcare);
      dontcare->kind = AST_DONTCARE;
      dontcare->id = node_id++;
      dontcare->line_no = token->line_no;
      arg = (struct Ast*)dontcare;
      next_token();
    } else if (token_is_typeRef(token)) {
      arg = build_typeRef();
    } else if (token_is_nonTypeName(token)) {
      arg = build_nonTypeName(false);
    } else assert(0);
  } else error("at line %d: type argument was expected, got `%s`.", token->line_no, token->lexeme);
  return arg;
}

internal enum AstParamDirection
build_direction()
{
  enum AstParamDirection dir = 0;
  if (token_is_direction(token)) {
    if (token->klass == TK_IN) {
      dir = PARAMDIR_IN;
    } else if (token->klass == TK_OUT) {
      dir = PARAMDIR_OUT;
    } else if (token->klass == TK_INOUT) {
      dir = PARAMDIR_INOUT;
    } else assert(0);
    next_token();
  }
  return dir;
}

internal struct Ast*
build_parameter()
{
  struct Ast_Param* param = arena_push_struct(ast_storage, struct Ast_Param);
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
  return (struct Ast*)param;
}

internal struct List*
build_parameterList()
{
  struct List* params = 0;
  if (token_is_parameter(token)) {
    params = arena_push_struct(ast_storage, struct List);
    list_init(params);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_parameter();
    list_append_link(params, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_parameter();
      list_append_link(params, li);
    }
  }
  return params;
}

internal struct Ast*
build_typeOrVoid(bool is_type)
{
  struct Ast* type = 0;
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      type = (struct Ast*)build_typeRef();
    } else if (token->klass == TK_VOID) {
      struct Ast_Name* void_name = arena_push_struct(ast_storage, struct Ast_Name);
      void_name->kind = AST_NAME;
      void_name->id = node_id++;
      void_name->line_no = token->line_no;
      void_name->strname = token->lexeme;
      type = (struct Ast*)void_name;
      next_token();
    } else if (token->klass == TK_IDENTIFIER) {
      struct Ast_Name* name = arena_push_struct(ast_storage, struct Ast_Name);
      name->kind = AST_NAME;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->strname = token->lexeme;
      type = (struct Ast*)name;
      if (is_type) {
        struct NameDecl* decl = arena_push_struct(ast_storage, struct NameDecl);
        decl->ast = (struct Ast*)name;
        decl->strname = name->strname;
        decl->line_no = token->line_no;
        declare_name_in_scope(current_scope, NAMESPACE_TYPE, decl);
      }
      next_token();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_functionPrototype(struct Ast* return_type)
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_typeOrVoid(token) || return_type) {
    proto = arena_push_struct(ast_storage, struct Ast_FunctionProto);
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
      proto->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: function name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)proto;
}

internal struct Ast*
build_methodPrototype()
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      proto = arena_push_struct(ast_storage, struct Ast_FunctionProto);
      proto->kind = AST_FUNCTION_PROTO;
      proto->id = node_id++;
      proto->line_no = token->line_no;
      proto->name = build_name(false);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      proto = (struct Ast_FunctionProto*)build_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)proto;
}

internal struct List*
build_methodPrototypes()
{
  struct List* protos = 0;
  if (token_is_methodPrototype(token)) {
    protos = arena_push_struct(ast_storage, struct List);
    list_init(protos);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_methodPrototype();
    list_append_link(protos, li);
    while (token_is_methodPrototype(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_methodPrototype();
      list_append_link(protos, li);
    }
  }
  return protos;
}

internal struct Ast*
build_externDeclaration()
{
  struct Ast* decl = 0;
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
      struct Ast_FunctionProto* proto = (struct Ast_FunctionProto*)build_functionPrototype(0);
      decl = (struct Ast*)proto;
      proto->is_extern = true;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else {
      struct Ast_Extern* extern_decl = arena_push_struct(ast_storage, struct Ast_Extern);
      extern_decl->kind = AST_EXTERN;
      extern_decl->id = node_id++;
      extern_decl->line_no = token->line_no;
      decl = (struct Ast*)extern_decl;
      extern_decl->name = build_nonTypeName(true);
      extern_decl->type_params = build_optTypeParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_decl->method_protos = build_methodPrototypes();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  }
  return decl;
}

internal struct Ast*
build_integer()
{
  struct Ast_IntLiteral* int_node = 0;
  if (token->klass == TK_INT_LITERAL) {
    int_node = arena_push_struct(ast_storage, struct Ast_IntLiteral);
    int_node->kind = AST_INT_LITERAL;
    int_node->id = node_id++;
    int_node->line_no = token->line_no;
    int_node->is_signed = token->i.is_signed;
    int_node->width = token->i.width;
    int_node->value = token->i.value;
    next_token();
  }
  return (struct Ast*)int_node;
}

internal struct Ast*
build_boolean()
{
  struct Ast_BoolLiteral* bool_node = 0;
  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    bool_node = arena_push_struct(ast_storage, struct Ast_BoolLiteral);
    bool_node->kind = AST_BOOL_LITERAL;
    bool_node->id = node_id++;
    bool_node->line_no = token->line_no;
    bool_node->value = (token->klass == TK_TRUE);
    next_token();
  }
  return (struct Ast*)bool_node;
}

internal struct Ast*
build_stringLiteral()
{
  struct Ast_StringLiteral* string = 0;
  if (token->klass == TK_STRING_LITERAL) {
    string = arena_push_struct(ast_storage, struct Ast_StringLiteral);
    string->kind = AST_STRING_LITERAL;
    string->id = node_id++;
    string->line_no = token->line_no;
    string->value = token->lexeme;
    next_token();
  }
  return (struct Ast*)string;
}

internal struct Ast*
build_integerTypeSize()
{
  struct Ast_IntTypeSize* type_size = arena_push_struct(ast_storage, struct Ast_IntTypeSize);
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
  return (struct Ast*)type_size;
}

internal struct Ast*
build_baseType()
{
  struct Ast* base_type = 0;
  if (token_is_baseType(token)) {
    struct Ast_Name* type_name = arena_push_struct(ast_storage, struct Ast_Name);
    type_name->kind = AST_NAME;
    type_name->id = node_id++;
    type_name->line_no = token->line_no;
    if (token->klass == TK_BOOL) {
      struct Ast_BoolType* bool_type = arena_push_struct(ast_storage, struct Ast_BoolType);
      bool_type->kind = AST_BOOL_TYPE;
      bool_type->id = node_id++;
      bool_type->line_no = token->line_no;
      type_name->strname = "bool";
      bool_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bool_type;
      next_token();
    } else if (token->klass == TK_ERROR) {
      struct Ast_ErrorType* error_type = arena_push_struct(ast_storage, struct Ast_ErrorType);
      error_type->kind = AST_ERROR_TYPE;
      error_type->id = node_id++;
      error_type->line_no = token->line_no;
      type_name->strname = "error";
      error_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)error_type;
      next_token();
    } else if (token->klass == TK_INT) {
      struct Ast_IntType* int_type = arena_push_struct(ast_storage, struct Ast_IntType);
      int_type->kind = AST_INT_TYPE;
      int_type->id = node_id++;
      int_type->line_no = node_id++;
      type_name->strname = "int";
      int_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)int_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_BIT) {
      struct Ast_BitType* bit_type = arena_push_struct(ast_storage, struct Ast_BitType);
      bit_type->kind = AST_BIT_TYPE;
      bit_type->id = node_id++;
      bit_type->line_no = token->line_no;
      type_name->strname = "bit";
      bit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_VARBIT) {
      struct Ast_VarbitType* varbit_type = arena_push_struct(ast_storage, struct Ast_VarbitType);
      varbit_type->kind = AST_VARBIT_TYPE;
      varbit_type->id = node_id++;
      varbit_type->line_no = node_id++;
      type_name->strname = "varbit";
      varbit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)varbit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_STRING) {
      struct Ast_StringType* string_type = arena_push_struct(ast_storage, struct Ast_StringType);
      string_type->kind = AST_STRING_TYPE;
      string_type->id = node_id++;
      string_type->line_no = token->line_no;
      type_name->strname = "string";
      string_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)string_type;
      next_token();
    } else if (token->klass == TK_VOID) {
      struct Ast_VoidType* void_type = arena_push_struct(ast_storage, struct Ast_VoidType);
      void_type->kind = AST_VOID_TYPE;
      void_type->id = node_id++;
      void_type->line_no = token->line_no;
      type_name->strname = "void";
      void_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)void_type;
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", token->line_no, token->lexeme);
  return base_type;
}

internal struct List*
build_typeArgumentList()
{
  struct List* args = 0;
  if (token_is_typeArg(token)) {
    args = arena_push_struct(ast_storage, struct List);
    list_init(args);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_typeArg();
    list_append_link(args, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_typeArg();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_tupleType()
{
  struct Ast_Tuple* tuple = 0;
  if (token->klass == TK_TUPLE) {
    next_token();
    tuple = arena_push_struct(ast_storage, struct Ast_Tuple);
    tuple->kind = AST_TUPLE;
    tuple->id = node_id++;
    tuple->line_no = token->line_no;
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      tuple->type_args = build_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `tuple` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)tuple;
}

internal struct Ast*
build_headerStackType()
{
  struct Ast_HeaderStack* stack = 0;
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    stack = arena_push_struct(ast_storage, struct Ast_HeaderStack);
    stack->kind = AST_HEADER_STACK;
    stack->id = node_id++;
    stack->line_no = token->line_no;
    if (token_is_expression(token)) {
      stack->stack_expr = build_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: an expression expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `[` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)stack;
}

internal struct Ast*
build_specializedType()
{
  struct Ast_SpecializedType* type = 0;
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    type = arena_push_struct(ast_storage, struct Ast_SpecializedType);
    type->kind = AST_SPECIALIZED_TYPE;
    type->id = node_id++;
    type->line_no = token->line_no;
    type->type_args = build_typeArgumentList();
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_prefixedType()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    name = arena_push_struct(ast_storage, struct Ast_Name);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
    name->id = node_id++;
    name->line_no = token->line_no;
    name->strname = token->lexeme;
    next_token();
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_typeName()
{
  struct Ast* name = 0;
  if (token_is_typeName(token)) {
    name = build_prefixedType();
    if (token->klass == TK_ANGLE_OPEN) {
      struct Ast* speclzd_type = build_specializedType();
      assert (speclzd_type->kind == AST_SPECIALIZED_TYPE);
      ((struct Ast_SpecializedType*)speclzd_type)->name = name;
      name = speclzd_type;
    } if (token->klass == TK_BRACKET_OPEN) {
      struct Ast* stack_type = build_headerStackType();
      assert (stack_type->kind == AST_HEADER_STACK);
      ((struct Ast_HeaderStack*)stack_type)->name = name;
      name = stack_type;
    }
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return name;
}

internal struct Ast*
build_typeRef()
{
  struct Ast* ref = 0;
  if (token_is_typeRef(token)) {
    if (token_is_baseType(token)) {
      ref = build_baseType();
    } else if (token_is_typeName(token)) {
      /* <typeName> | <specializedType> | <headerStackType> */
      ref = build_typeName();
    } else if (token->klass == TK_TUPLE) {
      ref = build_tupleType();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return ref;
}

internal bool
token_is_structField(struct Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

internal struct Ast*
build_structField()
{
  struct Ast_StructField* field = arena_push_struct(ast_storage, struct Ast_StructField);
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
  return (struct Ast*)field;
}

internal struct List*
build_structFieldList()
{
  struct List* fields = 0;
  if (token_is_structField(token)) {
    fields = arena_push_struct(ast_storage, struct List);
    list_init(fields);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_structField();
    list_append_link(fields, li);
    while (token_is_structField(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_structField();
      list_append_link(fields, li);
    }
  }
  return fields;
}

internal struct Ast*
build_headerTypeDeclaration()
{
  struct Ast_Header* decl = 0;
  if (token->klass == TK_HEADER) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Header);
    decl->kind = AST_HEADER;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `header` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_headerUnionDeclaration()
{
  struct Ast_HeaderUnion* decl = 0;
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_HeaderUnion);
    decl->kind = AST_HEADER_UNION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `header_union` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_structTypeDeclaration()
{
  struct Ast_Struct* decl = 0;
  if (token->klass == TK_STRUCT) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Struct);
    decl->kind = AST_STRUCT;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `struct` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal bool
token_is_specifiedIdentifier(struct Token* token)
{
  return token_is_name(token);
}

internal struct Ast*
build_initializer()
{
  return build_expression(1);
}

internal struct Ast*
build_optInitializer()
{
  struct Ast* init = 0;
  if (token->klass == TK_EQUAL) {
    next_token();
    init = build_initializer();
  }
  return init;
}

internal struct Ast*
build_specifiedIdentifier()
{
  struct Ast_SpecifiedIdent* id = 0;
  if (token_is_specifiedIdentifier(token)) {
    id = arena_push_struct(ast_storage, struct Ast_SpecifiedIdent);
    id->kind = AST_SPECIFIED_IDENT;
    id->id = node_id++;
    id->line_no = token->line_no;
    id->name = build_name(false);
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->init_expr = build_initializer();
      } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)id;
}

internal struct List*
build_specifiedIdentifierList()
{
  struct List* ids = 0;
  if (token_is_specifiedIdentifier(token)) {
    ids = arena_push_struct(ast_storage, struct List);
    list_init(ids);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_specifiedIdentifier();
    list_append_link(ids, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_specifiedIdentifier();
      list_append_link(ids, li);
    }
  }
  return ids;
}

internal struct Ast*
build_enumDeclaration()
{
  struct Ast_Enum* decl = 0;
  if (token->klass == TK_ENUM) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Enum);
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
          decl->id_list = build_specifiedIdentifierList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `enum` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_derivedTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token_is_derivedTypeDeclaration(token)) {
    if (token->klass == TK_HEADER) {
      decl = build_headerTypeDeclaration();
    } else if (token->klass == TK_HEADER_UNION) {
      decl = build_headerUnionDeclaration();
    } else if (token->klass == TK_STRUCT) {
      decl = build_structTypeDeclaration();
    } else if (token->klass == TK_ENUM) {
      decl = build_enumDeclaration();
    } else assert(0);
  } else error("at line %d: structure declaration was expected, got `%s`.", token->line_no, token->lexeme);
  return decl;
}

internal struct Ast*
build_parserTypeDeclaration()
{
  struct Ast_ParserProto* type = 0;
  if (token->klass == TK_PARSER) {
    next_token();
    type = arena_push_struct(ast_storage, struct Ast_ParserProto);
    type->kind = AST_PARSER_PROTO;
    type->id = node_id++;
    type->line_no = token->line_no; 
    if (token_is_name(token)) {
      type->name = build_name(true);
      type->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        type->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)type;
}

internal struct List*
build_optConstructorParameters()
{
  struct List* ctor_params = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    ctor_params = build_parameterList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
  }
  return ctor_params;
}

internal struct Ast*
build_constantDeclaration()
{
  struct Ast_Const* decl = 0;
  if (token->klass == TK_CONST) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Const);
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
          } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `const` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal bool
token_is_declaration(struct Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token) || token->klass == TK_DOTPREFIX;
  return result;
}

internal bool
token_is_lvalue(struct Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == TK_DOTPREFIX;
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(struct Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TK_PARENTH_OPEN || token->klass == TK_ANGLE_OPEN
    || token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_statement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TK_IF
    || token->klass == TK_SEMICOLON || token->klass == TK_BRACE_OPEN || token->klass == TK_EXIT
    || token->klass == TK_RETURN || token->klass == TK_SWITCH;
  return result;
}

internal bool
token_is_statementOrDeclaration(struct Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_CONST || token_is_statement(token);
  return result;
}

internal bool
token_is_argument(struct Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_parserLocalElement(struct Token* token)
{
  bool result = token->klass == TK_CONST || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_CONST || token_is_typeRef(token)
    || token->klass == TK_SEMICOLON;
  return result;
}

internal bool
token_is_simpleKeysetExpression(struct Token* token) {
  bool result = token_is_expression(token) || token->klass == TK_DEFAULT || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_keysetExpression(struct Token* token)
{
  bool result = token->klass == TK_TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

internal bool
token_is_selectCase(struct Token* token)
{
  return token_is_keysetExpression(token);
}

internal bool
token_is_controlLocalDeclaration(struct Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_ACTION
    || token->klass == TK_TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

internal struct Ast*
build_argument()
{
  struct Ast* arg = 0;
  if (token_is_argument(token)) {
    if (token_is_expression(token)) {
      arg = build_expression(1);
    } else if (token_is_name(token)) {
      struct Ast_Argument* name_arg = arena_push_struct(ast_storage, struct Ast_Argument);
      name_arg->kind = AST_ARGUMENT;
      name_arg->id = node_id++;
      name_arg->line_no = token->line_no;
      arg = (struct Ast*)name_arg;
      name_arg->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          name_arg->init_expr = build_expression(1);
        } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_DONTCARE) {
      struct Ast_Dontcare* dontcare_arg = arena_push_struct(ast_storage, struct Ast_Dontcare);
      dontcare_arg->kind = AST_DONTCARE;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      arg = (struct Ast*)dontcare_arg;
      next_token();
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", token->line_no, token->lexeme);
  return arg;
}

internal struct List*
build_argumentList()
{
  struct List* args = 0;
  if (token_is_argument(token)) {
    args = arena_push_struct(ast_storage, struct List);
    list_init(args);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_argument();
    list_append_link(args, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_argument();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_variableDeclaration(struct Ast* type_ref)
{
  struct Ast_Var* decl = 0;
  if (token_is_typeRef(token) || type_ref) {
    decl = arena_push_struct(ast_storage, struct Ast_Var);
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
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_instantiation(struct Ast* type_ref)
{
  struct Ast_Instantiation* inst = 0;
  if (token_is_typeRef(token) || type_ref) {
    inst = arena_push_struct(ast_storage, struct Ast_Instantiation);
    inst->kind = AST_INSTANTIATION;
    inst->id = node_id++;
    inst->line_no = token->line_no;
    inst->type_ref = type_ref ? type_ref : build_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      inst->args = build_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          inst->name = build_name(false);
          if (token->klass == TK_SEMICOLON) {
            next_token();
          } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: instance name was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)inst;
}

internal struct Ast*
build_parserLocalElement()
{
  struct Ast* elem = 0;
  if (token_is_parserLocalElement(token)) {
    if (token->klass == TK_CONST) {
      elem = build_constantDeclaration();
    } else if (token_is_typeRef(token)) {
      struct Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        elem = build_instantiation(type_ref);
      } else if (token_is_name(token)) {
        elem = build_variableDeclaration(type_ref);
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else assert(0);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  return elem;
}

internal struct List*
build_parserLocalElements()
{
  struct List* elems = 0;
  if (token_is_parserLocalElement(token)) {
    elems = arena_push_struct(ast_storage, struct List);
    list_init(elems);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_parserLocalElement();
    list_append_link(elems, li);
    while (token_is_parserLocalElement(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_parserLocalElement();
      list_append_link(elems, li);
    }
  }
  return elems;
}

internal struct Ast*
build_directApplication(struct Ast* type_name)
{
  struct Ast_FunctionCallExpr* apply_expr = 0;
  if (token_is_typeName(token) || type_name) {
    apply_expr = arena_push_struct(ast_storage, struct Ast_FunctionCallExpr);
    apply_expr->kind = AST_FUNCTION_CALL_EXPR;
    apply_expr->id = node_id++;
    apply_expr->line_no = token->line_no;
    struct Ast_MemberSelectExpr* apply_select = arena_push_struct(ast_storage, struct Ast_MemberSelectExpr);
    apply_select->kind = AST_MEMBER_SELECT_EXPR;
    apply_select->id = node_id++;
    apply_select->line_no = token->line_no;
    apply_select->lhs_expr = type_name ? type_name : build_typeName();
    struct Ast_Name* apply_name = arena_push_struct(ast_storage, struct Ast_Name);
    apply_name->kind = AST_NAME;
    apply_name->id = node_id++;
    apply_name->line_no = token->line_no;
    apply_name->strname = "apply";
    apply_select->member_name = (struct Ast*)apply_name;
    apply_expr->callee_expr = (struct Ast*)apply_select;
    if (token->klass == TK_DOTPREFIX) {
      next_token();
      if (token->klass == TK_APPLY) {
        next_token();
        if (token->klass == TK_PARENTH_OPEN) {
          next_token();
          apply_expr->args = build_argumentList();
          if (token->klass == TK_PARENTH_CLOSE) {
            next_token();
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
          } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `apply` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `.` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type name was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)apply_expr;
}

internal struct Ast*
build_prefixedNonTypeName()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    name = (struct Ast_Name*)build_nonTypeName(false);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
  } else error("at line %d: non-type name was expected, ", token->line_no, token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_arraySubscript()
{
  struct Ast_SubscriptExpr* subscript_expr = arena_push_struct(ast_storage, struct Ast_SubscriptExpr);
  subscript_expr->kind = AST_SUBSCRIPT_EXPR;
  subscript_expr->id = node_id++;
  subscript_expr->line_no = token->line_no;
  if (token_is_expression(token)) {
    subscript_expr->index = build_expression(1);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  if (token->klass == TK_COLON) {
    next_token();
    if (token_is_expression(token)) {
      subscript_expr->colon_index = build_expression(1);
    } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  }
  return (struct Ast*)subscript_expr;
}

internal struct Ast*
build_lvalueExpr()
{
  struct Ast* expr = 0;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    struct Ast_Name* dot_member = (struct Ast_Name*)build_name(false);
    dot_member->kind = AST_DOTNAME;
    expr = (struct Ast*)dot_member;
  } else if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    expr = build_arraySubscript();
    if (token->klass == TK_BRACKET_CLOSE) {
      next_token();
    } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

internal struct Ast*
build_lvalue()
{
  struct Ast* lvalue = 0;
  if (token_is_lvalue(token)) {
    struct Ast* name = build_prefixedNonTypeName();
    lvalue = name;
    while(token->klass == TK_DOTPREFIX || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOTPREFIX) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = arena_push_struct(ast_storage, struct Ast_MemberSelectExpr);
        select_expr->kind = AST_MEMBER_SELECT_EXPR;
        select_expr->id = node_id++;
        select_expr->line_no = token->line_no;
        select_expr->lhs_expr = lvalue;
        lvalue = (struct Ast*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        struct Ast_SubscriptExpr* subscript_expr = arena_push_struct(ast_storage, struct Ast_SubscriptExpr);
        subscript_expr->kind = AST_SUBSCRIPT_EXPR;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->index = lvalue;
        subscript_expr->colon_index = build_arraySubscript();
        lvalue = (struct Ast*)subscript_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    }
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)lvalue;
}

internal struct Ast*
build_assignmentOrMethodCallStatement()
{
  struct Ast* stmt = 0;
  if (token_is_lvalue(token)) {
    struct Ast* lvalue = build_lvalue();
    struct List* type_args = 0;
    stmt = lvalue;
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      type_args = build_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
    }
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      struct Ast_FunctionCallExpr* call_stmt = arena_push_struct(ast_storage, struct Ast_FunctionCallExpr);
      call_stmt->kind = AST_FUNCTION_CALL_EXPR;
      call_stmt->id = node_id++;
      call_stmt->line_no = token->line_no;
      call_stmt->callee_expr = lvalue;
      call_stmt->type_args = type_args;
      call_stmt->args = build_argumentList();
      stmt = (struct Ast*)call_stmt;
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_EQUAL) {
      next_token();
      struct Ast_AssignmentStmt* assign_stmt = arena_push_struct(ast_storage, struct Ast_AssignmentStmt);
      assign_stmt->kind = AST_ASSIGNMENT_STMT;
      assign_stmt->id = node_id++;
      assign_stmt->line_no = token->line_no;
      assign_stmt->lvalue = lvalue;
      assign_stmt->expr = build_expression(1);
      stmt = (struct Ast*)assign_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_no, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
}

internal struct List*
build_parserStatements()
{
  struct List* stmts = 0;
  if (token_is_parserStatement(token)) {
    stmts = arena_push_struct(ast_storage, struct List);
    memset(stmts, 0, sizeof(*stmts));
    list_init(stmts);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_parserStatement();
    list_append_link(stmts, li);
    while (token_is_parserStatement(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_parserStatement();
      list_append_link(stmts, li);
    }
  }
  return stmts;
}

internal struct Ast*
build_parserBlockStatements()
{
  struct Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = arena_push_struct(ast_storage, struct Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    stmt->stmt_list = build_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)stmt;
}

internal struct Ast*
build_parserStatement()
{
  struct Ast* stmt = 0;
  if (token_is_typeRef(token)) {
    struct Ast* type_ref = build_typeRef();
    if (token_is_name(token)) {
      stmt = build_variableDeclaration(type_ref);
    } else {
      stmt = build_directApplication(type_ref);
    }
  } else if (token_is_assignmentOrMethodCallStatement(token)) {
    stmt = build_assignmentOrMethodCallStatement();
  } else if (token->klass == TK_BRACE_OPEN) {
    stmt = build_parserBlockStatements();
  } else if (token->klass == TK_CONST) {
    stmt = build_constantDeclaration();
  } else if (token->klass == TK_SEMICOLON) {
    stmt = (struct Ast*)arena_push_struct(ast_storage, struct Ast_EmptyStmt);
    stmt->kind = AST_EMPTY_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
  } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
}

internal struct List*
build_expressionList()
{
  struct List* exprs = 0;
  if (token_is_expression(token)) {
    exprs = arena_push_struct(ast_storage, struct List);
    list_init(exprs);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_expression(1);
    list_append_link(exprs, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_expression(1);
      list_append_link(exprs, li);
    }
  }
  return exprs;
}

internal struct Ast*
build_simpleKeysetExpression()
{
  struct Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expression(1);
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    expr = (struct Ast*)arena_push_struct(ast_storage, struct Ast_DefaultStmt);
    expr->kind = AST_DEFAULT_STMT;
    expr->id = node_id++;
    expr->line_no = token->line_no;
  } else if (token->klass == TK_DONTCARE) {
    next_token();
    expr = (struct Ast*)arena_push_struct(ast_storage, struct Ast_Dontcare);
    expr->kind = AST_DONTCARE;
    expr->id = node_id++;
    expr->line_no = token->line_no;
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

internal struct Ast*
build_tupleKeysetExpression()
{
  struct Ast_TupleKeyset* tuple_keyset = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    tuple_keyset = arena_push_struct(ast_storage, struct Ast_TupleKeyset);
    tuple_keyset->kind = AST_TUPLE_KEYSET;
    tuple_keyset->id = node_id++;
    tuple_keyset->line_no = token->line_no;
    next_token();
    struct List* exprs = arena_push_struct(ast_storage, struct List);
    list_init(exprs);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_simpleKeysetExpression();
    list_append_link(exprs, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_simpleKeysetExpression();
      list_append_link(exprs, li);
    }
    tuple_keyset->expr_list = exprs;
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)tuple_keyset;
}

internal struct Ast*
build_keysetExpression()
{
  struct Ast* expr = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    expr = build_tupleKeysetExpression();
  } else if (token_is_simpleKeysetExpression(token)) {
    expr = build_simpleKeysetExpression();
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

internal struct Ast*
build_selectCase()
{
  struct Ast_SelectCase* select_case = 0;
  if (token_is_keysetExpression(token)) {
    select_case = arena_push_struct(ast_storage, struct Ast_SelectCase);
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
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)select_case;
}

internal struct List*
build_selectCaseList()
{
  struct List* cases = 0;
  if (token_is_selectCase(token)) {
    cases = arena_push_struct(ast_storage, struct List);
    list_init(cases);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_selectCase();
    list_append_link(cases, li);
    while (token_is_selectCase(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_selectCase();
      list_append_link(cases, li);
    }
  }
  return cases;
}

internal struct Ast*
build_selectExpression()
{
  struct Ast_SelectExpr* select_expr = 0;
  if (token->klass == TK_SELECT) {
    next_token();
    select_expr = arena_push_struct(ast_storage, struct Ast_SelectExpr);
    select_expr->kind = AST_SELECT_EXPR;
    select_expr->id = node_id++;
    select_expr->line_no = token->line_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      select_expr->expr_list = build_expressionList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          select_expr->case_list = build_selectCaseList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `select` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)select_expr;
}

internal struct Ast*
build_stateExpression()
{
  struct Ast* state_expr = 0;
  if (token_is_name(token)) {
    state_expr = build_name(false);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
  } else if (token->klass == TK_SELECT) {
    state_expr = build_selectExpression();
  } else error("at line %d: state expression was expected, got `%s`.", token->line_no, token->lexeme);
  return state_expr;
}

internal struct Ast*
build_transitionStatement()
{
  struct Ast* stmt = 0;
  if (token->klass == TK_TRANSITION) {
    next_token();
    stmt = build_stateExpression();
  } else error("at line %d: `transition` was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
}

internal struct Ast*
build_parserState()
{
  struct Ast_ParserState* state = 0;
  if (token->klass == TK_STATE) {
    next_token();
    state = arena_push_struct(ast_storage, struct Ast_ParserState);
    state->kind = AST_PARSER_STATE;
    state->id = node_id++;
    state->line_no = token->line_no;
    state->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      state->stmt_list = build_parserStatements();
      state->trans_stmt = build_transitionStatement();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `state` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)state;
}

internal struct List*
build_parserStates()
{
  struct List* states = 0;
  if (token->klass == TK_STATE) {
    states = arena_push_struct(ast_storage, struct List);
    list_init(states);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_parserState();
    list_append_link(states, li);
    while (token->klass == TK_STATE) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_parserState();
      list_append_link(states, li);
    }
  } else error("at line %d: `state` was expected, got `%s`.", token->line_no, token->lexeme);
  return states;
}

internal struct Ast*
build_parserDeclaration()
{
  struct Ast_Parser* decl = 0;
  if (token->klass == TK_PARSER) {
    decl = arena_push_struct(ast_storage, struct Ast_Parser);
    decl->kind = AST_PARSER;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->type_decl = build_parserTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <parserTypeDeclaration> */
    } else {
      decl->ctor_params = build_optConstructorParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->local_elements = build_parserLocalElements();
        decl->states = build_parserStates();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_controlTypeDeclaration()
{
  struct Ast_ControlProto* decl = 0;
  if (token->klass == TK_CONTROL) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_ControlProto);
    decl->kind = AST_CONTROL_PROTO;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `control` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_actionDeclaration()
{
  struct Ast_Action* decl = 0;
  if (token->klass == TK_ACTION) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Action);
    decl->kind = AST_ACTION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(false);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TK_BRACE_OPEN) {
            decl->stmt = build_blockStatement();
          } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `action` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_keyElement()
{
  struct Ast_KeyElement* key_elem = 0;
  if (token_is_expression(token)) {
    key_elem = arena_push_struct(ast_storage, struct Ast_KeyElement);
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
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)key_elem;
}

internal struct List*
build_keyElementList()
{
  struct List* elems = 0;
  if (token_is_expression(token)) {
    elems = arena_push_struct(ast_storage, struct List);
    list_init(elems);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_keyElement();
    list_append_link(elems, li);
    while (token_is_expression(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_keyElement();
      list_append_link(elems, li);
    }
  }
  return elems;
}

internal struct Ast*
build_actionRef()
{
  struct Ast_ActionRef* ref = 0;
  if (token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)) {
    ref = arena_push_struct(ast_storage, struct Ast_ActionRef);
    ref->kind = AST_ACTION_REF;
    ref->id = node_id++;
    ref->line_no = token->line_no;
    ref->name = build_prefixedNonTypeName();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      ref->args = build_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)ref;
}

internal struct List*
build_actionList()
{
  struct List* actions = 0;
  if (token_is_actionRef(token)) {
    actions = arena_push_struct(ast_storage, struct List);
    list_init(actions);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_actionRef();
    list_append_link(actions, li);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    while (token_is_actionRef(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_actionRef();
      list_append_link(actions, li);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  }
  return actions;
}

internal struct Ast*
build_entry()
{
  struct Ast_TableEntry* entry = 0;
  if (token_is_keysetExpression(token)) {
    entry = arena_push_struct(ast_storage, struct Ast_TableEntry);
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
  } else error("at line %d: keyset was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)entry;
}

internal struct List*
build_entriesList()
{
  struct List* entries = 0;
  if (token_is_keysetExpression(token)) {
    entries = arena_push_struct(ast_storage, struct List);
    list_init(entries);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_entry();
    list_append_link(entries, li);
    while (token_is_keysetExpression(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_entry();
      list_append_link(entries, li);
    }
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return entries;
}

internal struct Ast*
build_tableProperty()
{
  struct Ast* prop = 0;
  if (token_is_tableProperty(token)) {
    bool is_const = false;
    if (token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    if (token->klass == TK_KEY) {
      next_token();
      struct Ast_TableKey* key_prop = arena_push_struct(ast_storage, struct Ast_TableKey);
      key_prop->kind = AST_TABLE_KEY;
      key_prop->id = node_id++;
      key_prop->line_no = token->line_no;
      prop = (struct Ast*)key_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          key_prop->keyelem_list = build_keyElementList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      struct Ast_TableActions* actions_prop = arena_push_struct(ast_storage, struct Ast_TableActions);
      actions_prop->kind = AST_TABLE_ACTIONS;
      actions_prop->id = node_id++;
      actions_prop->line_no = token->line_no;
      prop = (struct Ast*)actions_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          actions_prop->action_list = build_actionList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      struct Ast_TableEntries* entries_prop = arena_push_struct(ast_storage, struct Ast_TableEntries);
      entries_prop->kind = AST_TABLE_ENTRIES;
      entries_prop->id = node_id++;
      entries_prop->line_no = token->line_no;
      entries_prop->is_const = is_const;
      prop = (struct Ast*)entries_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          entries_prop->entries = build_entriesList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token_is_nonTableKwName(token)) {
      struct Ast_TableSingleEntry* entry_prop = arena_push_struct(ast_storage, struct Ast_TableSingleEntry);
      entry_prop->kind = AST_TABLE_SINGLE_ENTRY;
      entry_prop->id = node_id++;
      entry_prop->line_no = token->line_no;
      entry_prop->name = build_name(false);
      prop = (struct Ast*)entry_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        entry_prop->init_expr = build_initializer();
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else assert(0);
  } else error("at line %d: table property was expected, got `%s`.", token->line_no, token->lexeme);
  return prop;
}

internal struct List*
build_tablePropertyList()
{
  struct List* props = 0;
  if (token_is_tableProperty(token)) {
    props = arena_push_struct(ast_storage, struct List);
    list_init(props);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_tableProperty();
    list_append_link(props, li);
    while (token_is_tableProperty(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_tableProperty();
      list_append_link(props, li);
    }
  } else error("at line %d: table property was expected, got `%s`.", token->line_no, token->lexeme);
  return props;
}

internal struct Ast*
build_tableDeclaration()
{
  struct Ast_Table* table = 0;
  if (token->klass == TK_TABLE) {
    next_token();
    table = arena_push_struct(ast_storage, struct Ast_Table);
    table->kind = AST_TABLE;
    table->id = node_id++;
    table->line_no = token->line_no;
    table->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      table->prop_list = build_tablePropertyList();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `table` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)table;
}

internal struct Ast*
build_controlLocalDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == TK_CONST) {
    decl = build_constantDeclaration();
  } else if (token->klass == TK_ACTION) {
    decl = build_actionDeclaration();
  } else if (token->klass == TK_TABLE) {
    decl = build_tableDeclaration();
  } else if (token_is_typeRef(token)) {
    struct Ast* type_ref = build_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      decl = build_instantiation(type_ref);
    } else if (token_is_name(token)) {
      decl = build_variableDeclaration(type_ref);
    } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  return decl;
}

internal struct List*
build_controlLocalDeclarations()
{
  struct List* decls = 0;
  if (token_is_controlLocalDeclaration(token)) {
    decls = arena_push_struct(ast_storage, struct List);
    list_init(decls);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_controlLocalDeclaration();
    list_append_link(decls, li);
    while (token_is_controlLocalDeclaration(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_controlLocalDeclaration();
      list_append_link(decls, li);
    }
  }
  return decls;
}

internal struct Ast*
build_controlDeclaration()
{
  struct Ast_Control* decl = 0;
  if (token->klass == TK_CONTROL) {
    decl = arena_push_struct(ast_storage, struct Ast_Control);
    decl->kind = AST_CONTROL;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->type_decl = build_controlTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      decl->ctor_params = build_optConstructorParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->local_decls = build_controlLocalDeclarations();
        if (token->klass == TK_APPLY) {
          next_token();
          decl->apply_stmt = build_blockStatement();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `apply` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  } else error("at line %d: `control` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_packageTypeDeclaration()
{
  struct Ast_Package* decl = 0;
  if (token->klass == TK_PACKAGE) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Package);
    decl->kind = AST_PACKAGE;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `package` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_typedefDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
    bool is_typedef = false;
    if (token->klass == TK_TYPEDEF) {
      is_typedef = true;
      next_token();
    } else if (token->klass == TK_TYPE) {
      next_token();
    } else assert(0);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      struct Ast_Type* type_decl = arena_push_struct(ast_storage, struct Ast_Type);
      type_decl->kind = AST_TYPE;
      type_decl->id = node_id++;
      type_decl->line_no = token->line_no;
      type_decl->is_typedef = is_typedef;
      decl = (struct Ast*)type_decl;
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
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type definition was expected, got `%s`.", token->line_no, token->lexeme);
  return decl;
}

internal struct Ast*
build_typeDeclaration()
{
  struct Ast* decl = 0;
  if (token_is_typeDeclaration(token)) {
    if (token_is_derivedTypeDeclaration(token)) {
      decl = build_derivedTypeDeclaration();
    } else if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
      decl = build_typedefDeclaration();
    } else if (token->klass == TK_PARSER) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      decl = build_parserDeclaration();
    } else if (token->klass == TK_CONTROL) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      decl = build_controlDeclaration();
    } else if (token->klass == TK_PACKAGE) {
      decl = build_packageTypeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
    } else assert(0);
  } else error("at line %d: type declaration was expected, got `%s`.", token->line_no, token->lexeme); 
  return decl;
}

internal struct Ast*
build_conditionalStatement()
{
  struct Ast_IfStmt* if_stmt = 0;
  if (token->klass == TK_IF) {
    next_token();
    if_stmt = arena_push_struct(ast_storage, struct Ast_IfStmt);
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
      } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `if` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)if_stmt;
}

internal struct Ast*
build_exitStatement()
{
  struct Ast_ExitStmt* exit_stmt = 0;
  if (token->klass == TK_EXIT) {
    next_token();
    exit_stmt = arena_push_struct(ast_storage, struct Ast_ExitStmt);
    exit_stmt->kind = AST_EXIT_STMT;
    exit_stmt->id = node_id++;
    exit_stmt->line_no = token->line_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)exit_stmt;
}

internal struct Ast*
build_returnStatement()
{
  struct Ast_ReturnStmt* ret_stmt = 0;
  if (token->klass == TK_RETURN) {
    next_token();
    ret_stmt = arena_push_struct(ast_storage, struct Ast_ReturnStmt);
    ret_stmt->kind = AST_RETURN_STMT;
    ret_stmt->id = node_id++;
    ret_stmt->line_no = token->line_no;
    if (token_is_expression(token))
      ret_stmt->expr = build_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `return` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)ret_stmt;
}

internal struct Ast*
build_switchLabel()
{
  struct Ast* label = 0;
  if (token_is_name(token)) {
    label = build_name(false);
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    label = (struct Ast*)arena_push_struct(ast_storage, struct Ast_DefaultStmt);
    label->kind = AST_DEFAULT_STMT;
    label->id = node_id++;
    label->line_no = token->line_no;
  } else error("at line %d: switch label was expected, got `%s`.", token->line_no, token->lexeme);
  return label;
}

internal struct Ast*
build_switchCase()
{
  struct Ast_SwitchCase* switch_case = 0;
  if (token_is_switchLabel(token)) {
    switch_case = arena_push_struct(ast_storage, struct Ast_SwitchCase);
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
  } else error("at line %d: switch label was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)switch_case;
}

internal struct List*
build_switchCases()
{
  struct List* cases = 0;
  if (token_is_switchLabel(token)) {
    cases = arena_push_struct(ast_storage, struct List);
    list_init(cases);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_switchCase();
    list_append_link(cases, li);
    while (token_is_switchLabel(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_switchCase();
      list_append_link(cases, li);
    }
  }
  return cases;
}

internal struct Ast*
build_switchStatement()
{
  struct Ast_SwitchStmt* stmt = 0;
  if (token->klass == TK_SWITCH) {
    next_token();
    stmt = arena_push_struct(ast_storage, struct Ast_SwitchStmt);
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
          stmt->switch_cases = build_switchCases();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `switch` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)stmt;
}

internal struct Ast*
build_statement(struct Ast* type_name)
{
  struct Ast* stmt = 0;
  if (token_is_typeName(token) || type_name) {
    stmt = build_directApplication(type_name);
  } else if (token_is_assignmentOrMethodCallStatement(token)) {
    stmt = build_assignmentOrMethodCallStatement();
  } else if (token->klass == TK_IF) {
    stmt = build_conditionalStatement();
  } else if (token->klass == TK_SEMICOLON) {
    next_token();
    stmt = (struct Ast*)arena_push_struct(ast_storage, struct Ast_EmptyStmt);
    stmt->kind = AST_EMPTY_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
  } else if (token->klass == TK_BRACE_OPEN) {
    stmt = build_blockStatement();
  } else if (token->klass == TK_EXIT) {
    stmt = build_exitStatement();
  } else if (token->klass == TK_RETURN) {
    stmt = build_returnStatement();
  } else if (token->klass == TK_SWITCH) {
    stmt = build_switchStatement();
  } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
}

internal struct Ast*
build_statementOrDecl()
{
  struct Ast* stmt = 0;
  if (token_is_statementOrDeclaration(token)) {
    if (token_is_typeRef(token)) {
      struct Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        stmt = build_instantiation(type_ref);
      } else if (token_is_name(token)) {
        stmt = build_variableDeclaration(type_ref);
      } else {
        stmt = build_statement(type_ref);
      }
    } else if (token_is_statement(token)) {
      stmt = build_statement(0);
    } else if (token->klass == TK_CONST) {
      stmt = build_constantDeclaration();
    } else assert(0);
  }
  return stmt;
}

internal struct List*
build_statementOrDeclList()
{
  struct List* stmts = 0;
  if (token_is_statementOrDeclaration(token)) {
    stmts = arena_push_struct(ast_storage, struct List);
    list_init(stmts);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_statementOrDecl();
    list_append_link(stmts, li);
    while (token_is_statementOrDeclaration(token)) {
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_statementOrDecl();
      list_append_link(stmts, li);
    }
  }
  return stmts;
}

internal struct Ast*
build_blockStatement()
{
  struct Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = arena_push_struct(ast_storage, struct Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    stmt->stmt_list = build_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)stmt;
}

internal struct List*
build_identifierList()
{
  struct List* ids = 0;
  if (token_is_name(token)) {
    ids = arena_push_struct(ast_storage, struct List);
    list_init(ids);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_name(false);
    list_append_link(ids, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_name(false);
      list_append_link(ids, li);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
  return ids;
}

internal struct Ast*
build_errorDeclaration()
{
  struct Ast_Error* decl = 0;
  if (token->klass == TK_ERROR) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_Error);
    decl->kind = AST_ERROR;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `error` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_matchKindDeclaration()
{
  struct Ast_MatchKind* decl = 0;
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    decl = arena_push_struct(ast_storage, struct Ast_MatchKind);
    decl->kind = AST_MATCH_KIND;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `match_kind` was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_functionDeclaration(struct Ast* type_ref)
{
  struct Ast_Function* decl = 0;
  if (token_is_typeOrVoid(token)) {
    decl = arena_push_struct(ast_storage, struct Ast_Function);
    decl->kind = AST_FUNCTION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->proto = build_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      decl->stmt = build_blockStatement();
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_declaration()
{
  struct Ast* decl = 0;
  if (token_is_declaration(token)) {
    if (token->klass == TK_CONST) {
      decl = build_constantDeclaration();
    } else if (token->klass == TK_EXTERN) {
      decl = build_externDeclaration();
    } else if (token->klass == TK_ACTION) {
      decl = build_actionDeclaration();
    } else if (token_is_typeDeclaration(token)) {
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      decl = build_typeDeclaration();
    } else if (token->klass == TK_ERROR) {
      decl = build_errorDeclaration();
    } else if (token->klass == TK_MATCH_KIND) {
      decl = build_matchKindDeclaration();
    } else if (token_is_typeRef(token)) {
      struct Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        decl = build_instantiation(type_ref);
      } else if (token_is_name(token)) {
        decl = build_functionDeclaration(type_ref);
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      decl = build_functionDeclaration(build_typeRef());
    } else assert(0);
  } else error("at line %d: top-level declaration as expected, got `%s`.", token->line_no, token->lexeme);
  return decl;
}

internal struct Ast*
build_p4program()
{
  struct Ast_P4Program* program = arena_push_struct(ast_storage, struct Ast_P4Program);
  program->kind = AST_P4PROGRAM;
  program->id = node_id++;
  program->line_no = token->line_no;
  struct List* decls = arena_push_struct(ast_storage, struct List);
  list_init(decls);
  while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
    if (token_is_declaration(token)) {
      struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_declaration();
      list_append_link(decls, li);
    } else if (token->klass == TK_SEMICOLON) {
      next_token(); /* empty declaration */
    }
  }
  program->decl_list = decls;
  if (token->klass != TK_END_OF_INPUT) {
    error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
  }
  return (struct Ast*)program;
}

internal bool
token_is_realTypeArg(struct Token* token)
{
  bool result = token->klass == TK_DONTCARE|| token_is_typeRef(token);
  return result;
}

internal bool
token_is_binaryOperator(struct Token* token)
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
token_is_exprOperator(struct Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOTPREFIX
    || token->klass == TK_BRACKET_OPEN || token->klass == TK_PARENTH_OPEN
    || token->klass == TK_ANGLE_OPEN;
  return result;
}

internal struct Ast*
build_realTypeArg()
{
  struct Ast* arg = 0;
  if (token->klass == TK_DONTCARE) {
    next_token();
    arg = (struct Ast*)arena_push_struct(ast_storage, struct Ast_Dontcare);
    arg->kind = AST_DONTCARE;
    arg->id = node_id++;
    arg->line_no = token->line_no;
  } else if (token_is_typeRef(token)) {
    arg = build_typeRef();
  } else error("at line %d: type argument was expected, got `%s`.", token->line_no, token->lexeme);
  return arg;
}

internal struct List*
build_realTypeArgumentList()
{
  struct List* args = 0;
  if (token_is_realTypeArg(token)) {
    args = arena_push_struct(ast_storage, struct List);
    list_init(args);
    struct ListLink* li = arena_push_struct(ast_storage, struct ListLink);
    li->object = build_realTypeArg();
    list_append_link(args, li);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, struct ListLink);
      li->object = build_realTypeArg();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_expressionPrimary()
{
  struct Ast* primary = 0;
  if (token_is_expression(token)) {
    if (token->klass == TK_INT_LITERAL) {
      primary = build_integer();
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      primary = build_boolean();
    } else if (token->klass == TK_STRING_LITERAL) {
      primary = build_stringLiteral();
    } else if (token->klass == TK_DOTPREFIX) {
      next_token();
      if (token->klass == TK_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_nonTypeName(false);
        name->kind = AST_DOTNAME;
        primary = (struct Ast*)name;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_typeName(false);
        name->kind = AST_DOTNAME;
        primary = (struct Ast*)name;
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else if (token_is_nonTypeName(token)) {
      primary = build_nonTypeName(false);
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      struct Ast_ExprListExpr* expr_list = arena_push_struct(ast_storage, struct Ast_ExprListExpr);
      expr_list->kind = AST_EXPRLIST_EXPR;
      expr_list->id = node_id++;
      expr_list->line_no = token->line_no;
      expr_list->expr_list = build_expressionList();
      primary = (struct Ast*)expr_list;
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        struct Ast_CastExpr* cast_expr = arena_push_struct(ast_storage, struct Ast_CastExpr);
        cast_expr->kind = AST_CAST_EXPR;
        cast_expr->id = node_id++;
        cast_expr->line_no = token->line_no;
        cast_expr->to_type = build_typeRef();
        primary = (struct Ast*)cast_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->expr = build_expression(1);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else if (token_is_expression(token)) {
        primary = build_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, struct Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NOT;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == TK_TILDA) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, struct Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_BITWISE_NOT;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, struct Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NEG;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token_is_typeName(token)) {
      primary = build_typeName();
    } else if (token->klass == TK_ERROR) {
      struct Ast_Name* name = arena_push_struct(ast_storage, struct Ast_Name);
      name->kind = AST_NAME;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->strname = token->lexeme;
      primary = (struct Ast*)name;
      next_token();
    } else assert(0);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  return primary;
}

internal int
get_operator_priority(struct Token* token)
{
  int prio = 0;
  if (token->klass == TK_DOUBLE_AMPERSAND || token->klass == TK_DOUBLE_PIPE) {
    /* Logical AND, OR */
    prio = 1;
  } else if (token->klass == TK_DOUBLE_EQUAL || token->klass == TK_EXCLAMATION_EQUAL
      || token->klass == TK_ANGLE_OPEN /* Less */ || token->klass == TK_ANGLE_CLOSE /* Greater */
      || token->klass == TK_ANGLE_OPEN_EQUAL /* Less-equal */ || token->klass == TK_ANGLE_CLOSE_EQUAL /* Greater-equal */) {
    /* Relational ops  */
    prio = 2;
  }
  else if (token->klass == TK_PLUS || token->klass == TK_MINUS
           || token->klass == TK_AMPERSAND || token->klass == TK_PIPE
           || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN /* BitshiftLeft */
           || token->klass == TK_DOUBLE_ANGLE_CLOSE /* BitshiftRight */) {
    /* Addition and subtraction; bitwise ops */
    prio = 3;
  }
  else if (token->klass == TK_STAR || token->klass == TK_SLASH) {
    /* Multiplication and division */
    prio = 4;
  }
  else if (token->klass == TK_TRIPLE_AMPERSAND) {
    /* Masking */
    prio = 5;
  }
  else assert(0);
  return prio;
}

internal enum AstOperator
token_to_binop(struct Token* token)
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

internal struct Ast*
build_expression(int priority_threshold)
{
  struct Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOTPREFIX) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = arena_push_struct(ast_storage, struct Ast_MemberSelectExpr);
        select_expr->kind = AST_MEMBER_SELECT_EXPR;
        select_expr->id = node_id++;
        select_expr->line_no = token->line_no;
        select_expr->lhs_expr = expr;
        expr = (struct Ast*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        struct Ast_SubscriptExpr* subscript_expr = arena_push_struct(ast_storage, struct Ast_SubscriptExpr);
        subscript_expr->kind = AST_SUBSCRIPT_EXPR;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->index = expr;
        subscript_expr->colon_index = build_arraySubscript();
        expr = (struct Ast*)subscript_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        struct Ast_FunctionCallExpr* call_expr = arena_push_struct(ast_storage, struct Ast_FunctionCallExpr);
        call_expr->kind = AST_FUNCTION_CALL_EXPR;
        call_expr->id = node_id++;
        call_expr->line_no = token->line_no;
        call_expr->callee_expr = expr;
        call_expr->args = build_argumentList();
        expr = (struct Ast*)call_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        ((struct Ast_Expression*)expr)->type_args = build_realTypeArgumentList();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        struct Ast_KVPairExpr* kv_pair = arena_push_struct(ast_storage, struct Ast_KVPairExpr);
        kv_pair->kind = AST_KVPAIR_EXPR;
        kv_pair->id = node_id++;
        kv_pair->line_no = token->line_no;
        kv_pair->name = expr;
        kv_pair->expr = build_expression(1);
        expr = (struct Ast*)kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          struct Ast_BinaryExpr* bin_expr = arena_push_struct(ast_storage, struct Ast_BinaryExpr);
          bin_expr->kind = AST_BINARY_EXPR;
          bin_expr->id = node_id++;
          bin_expr->line_no = token->line_no;
          bin_expr->left_operand = expr;
          bin_expr->op = token_to_binop(token);
          next_token();
          bin_expr->right_operand = build_expression(priority + 1);
          expr = (struct Ast*)bin_expr;
        } else break;
      } else assert(0);
    }
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

struct Ast_P4Program*
build_ast(struct UnboundedArray* tokens_array_, struct Arena* ast_storage_)
{
  struct NameDecl*
  declare_keyword(char* strname, enum TokenClass token_class)
  {
    struct NameDecl* decl = arena_push_struct(ast_storage, struct NameDecl);
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
  struct Ast_P4Program* p4program = (struct Ast_P4Program*)build_p4program();
  p4program->last_node_id = node_id;
  current_scope = pop_scope();
  assert(current_scope == 0);

  return p4program;
}
