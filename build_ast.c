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
  Ast* name = arena_push_struct(ast_storage, Ast);
  name->kind = AST_EMPTY_ELEMENT;
  name->id = node_id++;
  name->line_no = token->line_no;
  return name;
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
  Ast* name = arena_push_struct(ast_storage, Ast);
  name->kind = AST_EMPTY_ELEMENT;
  name->id = node_id++;
  name->line_no = token->line_no;
  return name;
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
    params->head.next = li;
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
    } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
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
  Ast* arg = arena_push_struct(ast_storage, Ast);
  arg->kind = AST_EMPTY_ELEMENT;
  arg->id = node_id++;
  arg->line_no = token->line_no;
  return arg;
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

internal Ast*
build_parameterList()
{
  Ast_ElementList* params = arena_push_struct(ast_storage, Ast_ElementList);
  params->kind = AST_ELEM_LIST;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->head.next = 0;
  if (token_is_parameter(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    params->head.next = li;
    li->object = build_parameter();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_parameter();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)params;
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
  Ast* type = arena_push_struct(ast_storage, Ast);
  type->kind = AST_EMPTY_ELEMENT;
  type->id = node_id++;
  type->line_no = token->line_no;
  return type;
}

internal Ast*
build_functionPrototype(Ast* return_type)
{
  Ast_FunctionProto* proto = 0;
  if (token_is_typeOrVoid(token) || return_type) {
    proto = arena_push_struct(ast_storage, Ast_FunctionProto);
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
  return (Ast*)proto;
}

internal Ast*
build_methodPrototype()
{
  Ast_FunctionProto* proto = 0;
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      proto = arena_push_struct(ast_storage, Ast_FunctionProto);
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
      proto = (Ast_FunctionProto*)build_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)proto;
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
    protos->head.next = li;
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
  Ast* decl = 0;
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
      decl = (Ast*)proto;
      proto->is_extern = true;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_no, token->lexeme);
    } else {
      Ast_Extern* extern_decl = arena_push_struct(ast_storage, Ast_Extern);
      extern_decl->kind = AST_EXTERN;
      extern_decl->id = node_id++;
      extern_decl->line_no = token->line_no;
      decl = (Ast*)extern_decl;
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

internal Ast*
build_integer()
{
  Ast_IntLiteral* int_node = 0;
  if (token->klass == TK_INT_LITERAL) {
    int_node = arena_push_struct(ast_storage, Ast_IntLiteral);
    int_node->kind = AST_INT_LITERAL;
    int_node->id = node_id++;
    int_node->line_no = token->line_no;
    int_node->is_signed = token->i.is_signed;
    int_node->width = token->i.width;
    int_node->value = token->i.value;
    next_token();
  }
  return (Ast*)int_node;
}

internal Ast*
build_boolean()
{
  Ast_BoolLiteral* bool_node = 0;
  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    bool_node = arena_push_struct(ast_storage, Ast_BoolLiteral);
    bool_node->kind = AST_BOOL_LITERAL;
    bool_node->id = node_id++;
    bool_node->line_no = token->line_no;
    bool_node->value = (token->klass == TK_TRUE);
    next_token();
  }
  return (Ast*)bool_node;
}

internal Ast*
build_stringLiteral()
{
  Ast_StringLiteral* string = 0;
  if (token->klass == TK_STRING_LITERAL) {
    string = arena_push_struct(ast_storage, Ast_StringLiteral);
    string->kind = AST_STRING_LITERAL;
    string->id = node_id++;
    string->line_no = token->line_no;
    string->value = token->lexeme;
    next_token();
  }
  return (Ast*)string;
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
  Ast* base_type = 0;
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
      base_type = (Ast*)bool_type;
      next_token();
    } else if (token->klass == TK_ERROR) {
      Ast_ErrorType* error_type = arena_push_struct(ast_storage, Ast_ErrorType);
      error_type->kind = AST_ERROR_TYPE;
      error_type->id = node_id++;
      error_type->line_no = token->line_no;
      type_name->strname = "error";
      error_type->name = (Ast*)type_name;
      base_type = (Ast*)error_type;
      next_token();
    } else if (token->klass == TK_INT) {
      Ast_IntType* int_type = arena_push_struct(ast_storage, Ast_IntType);
      int_type->kind = AST_INT_TYPE;
      int_type->id = node_id++;
      int_type->line_no = node_id++;
      type_name->strname = "int";
      int_type->name = (Ast*)type_name;
      base_type = (Ast*)int_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_BIT) {
      Ast_BitType* bit_type = arena_push_struct(ast_storage, Ast_BitType);
      bit_type->kind = AST_BIT_TYPE;
      bit_type->id = node_id++;
      bit_type->line_no = token->line_no;
      type_name->strname = "bit";
      bit_type->name = (Ast*)type_name;
      base_type = (Ast*)bit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_VARBIT) {
      Ast_VarbitType* varbit_type = arena_push_struct(ast_storage, Ast_VarbitType);
      varbit_type->kind = AST_VARBIT_TYPE;
      varbit_type->id = node_id++;
      varbit_type->line_no = node_id++;
      type_name->strname = "varbit";
      varbit_type->name = (Ast*)type_name;
      base_type = (Ast*)varbit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    } else if (token->klass == TK_STRING) {
      Ast_StringType* string_type = arena_push_struct(ast_storage, Ast_StringType);
      string_type->kind = AST_STRING_TYPE;
      string_type->id = node_id++;
      string_type->line_no = token->line_no;
      type_name->strname = "string";
      string_type->name = (Ast*)type_name;
      base_type = (Ast*)string_type;
      next_token();
    } else if (token->klass == TK_VOID) {
      Ast_VoidType* void_type = arena_push_struct(ast_storage, Ast_VoidType);
      void_type->kind = AST_VOID_TYPE;
      void_type->id = node_id++;
      void_type->line_no = token->line_no;
      type_name->strname = "void";
      void_type->name = (Ast*)type_name;
      base_type = (Ast*)void_type;
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", token->line_no, token->lexeme);
  return base_type;
}

internal Ast*
build_typeArgumentList()
{
  Ast_ElementList* args = arena_push_struct(ast_storage, Ast_ElementList);
  args->kind = AST_ELEM_LIST;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->head.next = 0;
  if (token_is_typeArg(token)) {
    DList* li = arena_push_struct(ast_storage, DList);
    DList* last = li;
    args->head.next = li;
    li->object = build_typeArg();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, DList);
      li->object = build_typeArg();
      dlist_concat(last, li);
      last = li;
    }
  }
  return (Ast*)args;
}

internal Ast*
build_tupleType()
{
  Ast_Tuple* tuple = 0;
  if (token->klass == TK_TUPLE) {
    next_token();
    tuple = arena_push_struct(ast_storage, Ast_Tuple);
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
  return (Ast*)tuple;
}

internal Ast*
build_headerStackType()
{
  Ast_HeaderStack* stack = 0;
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    stack = arena_push_struct(ast_storage, Ast_HeaderStack);
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
  return (Ast*)stack;
}

internal Ast*
build_specializedType()
{
  Ast_SpecializedType* type = 0;
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    type = arena_push_struct(ast_storage, Ast_SpecializedType);
    type->kind = AST_SPECIALIZED_TYPE;
    type->id = node_id++;
    type->line_no = token->line_no;
    type->type_args = build_typeArgumentList();
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `<` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)type;
}

internal Ast*
build_prefixedType()
{
  Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    name = arena_push_struct(ast_storage, Ast_Name);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
    name->id = node_id++;
    name->line_no = token->line_no;
    name->strname = token->lexeme;
    next_token();
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)name;
}

internal Ast*
build_typeName()
{
  Ast* name = 0;
  if (token_is_typeName(token)) {
    name = build_prefixedType();
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
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return name;
}

internal Ast*
build_typeRef()
{
  Ast* ref = 0;
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
    fields->head.next = li;
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
  Ast_Header* decl = 0;
  if (token->klass == TK_HEADER) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Header);
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
  return (Ast*)decl;
}

internal Ast*
build_headerUnionDeclaration()
{
  Ast_HeaderUnion* decl = 0;
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_HeaderUnion);
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
  return (Ast*)decl;
}

internal Ast*
build_structTypeDeclaration()
{
  Ast_Struct* decl = 0;
  if (token->klass == TK_STRUCT) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Struct);
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
  return (Ast*)decl;
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
  Ast* init = 0;
  if (token->klass == TK_EQUAL) {
    next_token();
    init = build_initializer();
  }
  return init;
}

internal Ast*
build_specifiedIdentifier()
{
  Ast_SpecifiedIdent* id = 0;
  if (token_is_specifiedIdentifier(token)) {
    id = arena_push_struct(ast_storage, Ast_SpecifiedIdent);
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
  return (Ast*)id;
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
    ids->head.next = li;
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
  Ast_Enum* decl = 0;
  if (token->klass == TK_ENUM) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Enum);
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
  return (Ast*)decl;
}

internal Ast*
build_derivedTypeDeclaration()
{
  Ast* decl = 0;
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

internal Ast*
build_parserTypeDeclaration()
{
  Ast_ParserProto* type = 0;
  if (token->klass == TK_PARSER) {
    next_token();
    type = arena_push_struct(ast_storage, Ast_ParserProto);
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
  return (Ast*)type;
}

internal Ast*
build_optConstructorParameters()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    Ast* params = build_parameterList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    return params;
  }
  Ast_ElementList* params = arena_push_struct(ast_storage, Ast_ElementList);
  params->kind = AST_ELEM_LIST;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->head.next = 0;
  return (Ast*)params;
}

internal Ast*
build_constantDeclaration()
{
  Ast_Const* decl = 0;
  if (token->klass == TK_CONST) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Const);
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
  return (Ast*)decl;
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
  Ast* arg = 0;
  if (token_is_argument(token)) {
    if (token_is_expression(token)) {
      arg = build_expression(1);
    } else if (token_is_name(token)) {
      Ast_Argument* name_arg = arena_push_struct(ast_storage, Ast_Argument);
      name_arg->kind = AST_ARGUMENT;
      name_arg->id = node_id++;
      name_arg->line_no = token->line_no;
      arg = (Ast*)name_arg;
      name_arg->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          name_arg->init_expr = build_expression(1);
        } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_DONTCARE) {
      Ast* dontcare_arg = arena_push_struct(ast_storage, Ast);
      dontcare_arg->kind = AST_DONTCARE;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      arg = (Ast*)dontcare_arg;
      next_token();
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", token->line_no, token->lexeme);
  return arg;
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
    args->head.next = li;
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
build_variableDeclaration(Ast* type_ref)
{
  Ast_Var* decl = 0;
  if (token_is_typeRef(token) || type_ref) {
    decl = arena_push_struct(ast_storage, Ast_Var);
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
  return (Ast*)decl;
}

internal Ast*
build_instantiation(Ast* type_ref)
{
  Ast_Instantiation* inst = 0;
  if (token_is_typeRef(token) || type_ref) {
    inst = arena_push_struct(ast_storage, Ast_Instantiation);
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
  return (Ast*)inst;
}

internal Ast*
build_parserLocalElement()
{
  Ast* elem = 0;
  if (token_is_parserLocalElement(token)) {
    if (token->klass == TK_CONST) {
      elem = build_constantDeclaration();
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = build_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        elem = build_instantiation(type_ref);
      } else if (token_is_name(token)) {
        elem = build_variableDeclaration(type_ref);
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else assert(0);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  return elem;
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
    elems->head.next = li;
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
  Ast_FunctionCall* apply_expr = 0;
  if (token_is_typeName(token) || type_name) {
    apply_expr = arena_push_struct(ast_storage, Ast_FunctionCall);
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
  return (Ast*)apply_expr;
}

internal Ast*
build_prefixedNonTypeName()
{
  Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    name = (Ast_Name*)build_nonTypeName(false);
    name->kind = is_dotprefixed ? AST_DOTNAME : AST_NAME;
  } else error("at line %d: non-type name was expected, ", token->line_no, token->lexeme);
  return (Ast*)name;
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
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  if (token->klass == TK_COLON) {
    next_token();
    if (token_is_expression(token)) {
      subscript_expr->after_colon = build_expression(1);
    } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  }
  return (Ast*)subscript_expr;
}

internal Ast*
build_lvalueExpr()
{
  Ast* expr = 0;
  if (token->klass == TK_DOTPREFIX) {
    next_token();
    Ast_Name* dot_member = (Ast_Name*)build_name(false);
    dot_member->kind = AST_DOTNAME;
    expr = (Ast*)dot_member;
  } else if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    expr = build_arraySubscript();
    if (token->klass == TK_BRACKET_CLOSE) {
      next_token();
    } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

internal Ast*
build_lvalue()
{
  Ast* lvalue = 0;
  if (token_is_lvalue(token)) {
    Ast* name = build_prefixedNonTypeName();
    lvalue = name;
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
        subscript_expr->after_colon = build_arraySubscript();
        lvalue = (Ast*)subscript_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_no, token->lexeme);
      }
    }
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)lvalue;
}

internal Ast*
build_assignmentOrMethodCallStatement()
{
  Ast* stmt = 0;
  if (token_is_lvalue(token)) {
    Ast* lvalue = build_lvalue();
    Ast* type_args = 0;
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
      Ast_FunctionCall* call_stmt = arena_push_struct(ast_storage, Ast_FunctionCall);
      call_stmt->kind = AST_FUNCTION_CALL;
      call_stmt->id = node_id++;
      call_stmt->line_no = token->line_no;
      call_stmt->callee_expr = lvalue;
      call_stmt->type_args = type_args;
      call_stmt->args = build_argumentList();
      stmt = (Ast*)call_stmt;
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_EQUAL) {
      next_token();
      Ast_AssignmentStmt* assign_stmt = arena_push_struct(ast_storage, Ast_AssignmentStmt);
      assign_stmt->kind = AST_ASSIGNMENT_STMT;
      assign_stmt->id = node_id++;
      assign_stmt->line_no = token->line_no;
      assign_stmt->lvalue = lvalue;
      assign_stmt->expr = build_expression(1);
      stmt = (Ast*)assign_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_no, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
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
    stmts->head.next = li;
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
  Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    stmt->stmt_list = build_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)stmt;
}

internal Ast*
build_parserStatement()
{
  Ast* stmt = 0;
  if (token_is_typeRef(token)) {
    Ast* type_ref = build_typeRef();
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
    stmt = arena_push_struct(ast_storage, Ast);
    stmt->kind = AST_EMPTY_ELEMENT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
  } else error("at line %d: statement was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
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
    exprs->head.next = li;
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
  Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expression(1);
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    expr = arena_push_struct(ast_storage, Ast);
    expr->kind = AST_DEFAULT_STMT;
    expr->id = node_id++;
    expr->line_no = token->line_no;
  } else if (token->klass == TK_DONTCARE) {
    next_token();
    expr = arena_push_struct(ast_storage, Ast);
    expr->kind = AST_DONTCARE;
    expr->id = node_id++;
    expr->line_no = token->line_no;
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
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
    exprs->head.next = li;
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
  Ast_TupleKeyset* tuple_keyset = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    tuple_keyset = arena_push_struct(ast_storage, Ast_TupleKeyset);
    tuple_keyset->kind = AST_TUPLE_KEYSET;
    tuple_keyset->id = node_id++;
    tuple_keyset->line_no = token->line_no;
    next_token();
    tuple_keyset->expr_list = build_keysetExpressionList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)tuple_keyset;
}

internal Ast*
build_keysetExpression()
{
  Ast* expr = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    expr = build_tupleKeysetExpression();
  } else if (token_is_simpleKeysetExpression(token)) {
    expr = build_simpleKeysetExpression();
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
}

internal Ast*
build_selectCase()
{
  Ast_SelectCase* select_case = 0;
  if (token_is_keysetExpression(token)) {
    select_case = arena_push_struct(ast_storage, Ast_SelectCase);
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
  return (Ast*)select_case;
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
    cases->head.next = li;
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
  Ast_SelectExpr* select_expr = 0;
  if (token->klass == TK_SELECT) {
    next_token();
    select_expr = arena_push_struct(ast_storage, Ast_SelectExpr);
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
  return (Ast*)select_expr;
}

internal Ast*
build_stateExpression()
{
  Ast* state_expr = 0;
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

internal Ast*
build_transitionStatement()
{
  Ast* stmt = 0;
  if (token->klass == TK_TRANSITION) {
    next_token();
    stmt = build_stateExpression();
  } else error("at line %d: `transition` was expected, got `%s`.", token->line_no, token->lexeme);
  return stmt;
}

internal Ast*
build_parserState()
{
  Ast_ParserState* state = 0;
  if (token->klass == TK_STATE) {
    next_token();
    state = arena_push_struct(ast_storage, Ast_ParserState);
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
  return (Ast*)state;
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
    states->head.next = li;
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
  Ast_Parser* decl = 0;
  if (token->klass == TK_PARSER) {
    decl = arena_push_struct(ast_storage, Ast_Parser);
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
        if (token->klass == TK_STATE) {
          decl->states = build_parserStates();
        } else error("at line %d: `state` was expected, got `%s`.", token->line_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
    }
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)decl;
}

internal Ast*
build_controlTypeDeclaration()
{
  Ast_ControlProto* decl = 0;
  if (token->klass == TK_CONTROL) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_ControlProto);
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
  return (Ast*)decl;
}

internal Ast*
build_actionDeclaration()
{
  Ast_Action* decl = 0;
  if (token->klass == TK_ACTION) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Action);
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
  return (Ast*)decl;
}

internal Ast*
build_keyElement()
{
  Ast_KeyElement* key_elem = 0;
  if (token_is_expression(token)) {
    key_elem = arena_push_struct(ast_storage, Ast_KeyElement);
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
  return (Ast*)key_elem;
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
    elems->head.next = li;
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
  Ast_ActionRef* ref = 0;
  if (token->klass == TK_DOTPREFIX || token_is_nonTypeName(token)) {
    ref = arena_push_struct(ast_storage, Ast_ActionRef);
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
  return (Ast*)ref;
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
    actions->head.next = li;
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
  Ast_TableEntry* entry = 0;
  if (token_is_keysetExpression(token)) {
    entry = arena_push_struct(ast_storage, Ast_TableEntry);
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
  return (Ast*)entry;
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
    entries->head.next = li;
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
  Ast* prop = 0;
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
      prop = (Ast*)key_prop;
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
      Ast_TableActions* actions_prop = arena_push_struct(ast_storage, Ast_TableActions);
      actions_prop->kind = AST_TABLE_ACTIONS;
      actions_prop->id = node_id++;
      actions_prop->line_no = token->line_no;
      prop = (Ast*)actions_prop;
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
      Ast_TableEntries* entries_prop = arena_push_struct(ast_storage, Ast_TableEntries);
      entries_prop->kind = AST_TABLE_ENTRIES;
      entries_prop->id = node_id++;
      entries_prop->line_no = token->line_no;
      entries_prop->is_const = is_const;
      prop = (Ast*)entries_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            entries_prop->entries = build_entriesList();
          } else error("at line %d: keyset expression was expected, got `%s`.", token->line_no, token->lexeme);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token_is_nonTableKwName(token)) {
      Ast_TableSingleEntry* entry_prop = arena_push_struct(ast_storage, Ast_TableSingleEntry);
      entry_prop->kind = AST_TABLE_SINGLE_ENTRY;
      entry_prop->id = node_id++;
      entry_prop->line_no = token->line_no;
      entry_prop->name = build_name(false);
      prop = (Ast*)entry_prop;
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
    props->head.next = li;
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
  Ast_Table* table = 0;
  if (token->klass == TK_TABLE) {
    next_token();
    table = arena_push_struct(ast_storage, Ast_Table);
    table->kind = AST_TABLE;
    table->id = node_id++;
    table->line_no = token->line_no;
    table->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_tableProperty(token)) {
        table->prop_list = build_tablePropertyList();
      } else error("at line %d: table property was expected, got `%s`.", token->line_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `table` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)table;
}

internal Ast*
build_controlLocalDeclaration()
{
  Ast* decl = 0;
  if (token->klass == TK_CONST) {
    decl = build_constantDeclaration();
  } else if (token->klass == TK_ACTION) {
    decl = build_actionDeclaration();
  } else if (token->klass == TK_TABLE) {
    decl = build_tableDeclaration();
  } else if (token_is_typeRef(token)) {
    Ast* type_ref = build_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      decl = build_instantiation(type_ref);
    } else if (token_is_name(token)) {
      decl = build_variableDeclaration(type_ref);
    } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_no, token->lexeme);
  return decl;
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
    decls->head.next = li;
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
  Ast_Control* decl = 0;
  if (token->klass == TK_CONTROL) {
    decl = arena_push_struct(ast_storage, Ast_Control);
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
  return (Ast*)decl;
}

internal Ast*
build_packageTypeDeclaration()
{
  Ast_Package* decl = 0;
  if (token->klass == TK_PACKAGE) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Package);
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
  return (Ast*)decl;
}

internal Ast*
build_typedefDeclaration()
{
  Ast* decl = 0;
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
      decl = (Ast*)type_decl;
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

internal Ast*
build_typeDeclaration()
{
  Ast* decl = 0;
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

internal Ast*
build_conditionalStatement()
{
  Ast_IfStmt* if_stmt = 0;
  if (token->klass == TK_IF) {
    next_token();
    if_stmt = arena_push_struct(ast_storage, Ast_IfStmt);
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
  return (Ast*)if_stmt;
}

internal Ast*
build_exitStatement()
{
  Ast* exit_stmt = 0;
  if (token->klass == TK_EXIT) {
    next_token();
    exit_stmt = arena_push_struct(ast_storage, Ast);
    exit_stmt->kind = AST_EXIT_STMT;
    exit_stmt->id = node_id++;
    exit_stmt->line_no = token->line_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)exit_stmt;
}

internal Ast*
build_returnStatement()
{
  Ast_ReturnStmt* ret_stmt = 0;
  if (token->klass == TK_RETURN) {
    next_token();
    ret_stmt = arena_push_struct(ast_storage, Ast_ReturnStmt);
    ret_stmt->kind = AST_RETURN_STMT;
    ret_stmt->id = node_id++;
    ret_stmt->line_no = token->line_no;
    if (token_is_expression(token))
      ret_stmt->expr = build_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `return` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)ret_stmt;
}

internal Ast*
build_switchLabel()
{
  Ast* label = 0;
  if (token_is_name(token)) {
    label = build_name(false);
  } else if (token->klass == TK_DEFAULT) {
    next_token();
    label = arena_push_struct(ast_storage, Ast);
    label->kind = AST_DEFAULT_STMT;
    label->id = node_id++;
    label->line_no = token->line_no;
  } else error("at line %d: switch label was expected, got `%s`.", token->line_no, token->lexeme);
  return label;
}

internal Ast*
build_switchCase()
{
  Ast_SwitchCase* switch_case = 0;
  if (token_is_switchLabel(token)) {
    switch_case = arena_push_struct(ast_storage, Ast_SwitchCase);
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
  return (Ast*)switch_case;
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
    cases->head.next = li;
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
  Ast_SwitchStmt* stmt = 0;
  if (token->klass == TK_SWITCH) {
    next_token();
    stmt = arena_push_struct(ast_storage, Ast_SwitchStmt);
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
  return (Ast*)stmt;
}

internal Ast*
build_statement(Ast* type_name)
{
  Ast* stmt = 0;
  if (token_is_typeName(token) || type_name) {
    stmt = build_directApplication(type_name);
  } else if (token_is_assignmentOrMethodCallStatement(token)) {
    stmt = build_assignmentOrMethodCallStatement();
  } else if (token->klass == TK_IF) {
    stmt = build_conditionalStatement();
  } else if (token->klass == TK_SEMICOLON) {
    next_token();
    stmt = arena_push_struct(ast_storage, Ast);
    stmt->kind = AST_EMPTY_ELEMENT;
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

internal Ast*
build_statementOrDecl()
{
  Ast* stmt = 0;
  if (token_is_statementOrDeclaration(token)) {
    if (token_is_typeRef(token)) {
      Ast* type_ref = build_typeRef();
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
    stmts->head.next = li;
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
  Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    stmt->kind = AST_BLOCK_STMT;
    stmt->id = node_id++;
    stmt->line_no = token->line_no;
    next_token();
    stmt->stmt_list = build_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)stmt;
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
    ids->head.next = li;
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
  Ast_Error* decl = 0;
  if (token->klass == TK_ERROR) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_Error);
    decl->kind = AST_ERROR;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        if (token_is_name(token)) {
          decl->id_list = build_identifierList();
        } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_no, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: `error` was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)decl;
}

internal Ast*
build_matchKindDeclaration()
{
  Ast_MatchKind* decl = 0;
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    decl = arena_push_struct(ast_storage, Ast_MatchKind);
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
  return (Ast*)decl;
}

internal Ast*
build_functionDeclaration(Ast* type_ref)
{
  Ast_Function* decl = 0;
  if (token_is_typeOrVoid(token)) {
    decl = arena_push_struct(ast_storage, Ast_Function);
    decl->kind = AST_FUNCTION;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->proto = build_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      decl->stmt = build_blockStatement();
    } else error("at line %d: `{` was expected, got `%s`.", token->line_no, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_no, token->lexeme);
  return (Ast*)decl;
}

internal Ast*
build_declaration()
{
  Ast* decl = 0;
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
      Ast* type_ref = build_typeRef();
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
    decls->head.next = li;
    li->object = build_declaration();
    while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
      if (token_is_declaration(token)) {
        DList* li = arena_push_struct(ast_storage, DList);
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
  program->decl_list = build_declarationList();
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
  Ast* arg = 0;
  if (token->klass == TK_DONTCARE) {
    next_token();
    arg = arena_push_struct(ast_storage, Ast);
    arg->kind = AST_DONTCARE;
    arg->id = node_id++;
    arg->line_no = token->line_no;
  } else if (token_is_typeRef(token)) {
    arg = build_typeRef();
  } else error("at line %d: type argument was expected, got `%s`.", token->line_no, token->lexeme);
  return arg;
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
    args->head.next = li;
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
  Ast* primary = 0;
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
        Ast_Name* name = (Ast_Name*)build_nonTypeName(false);
        name->kind = AST_DOTNAME;
        primary = (Ast*)name;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        Ast_Name* name = (Ast_Name*)build_typeName(false);
        name->kind = AST_DOTNAME;
        primary = (Ast*)name;
      } else error("at line %d: unexpected token `%s`.", token->line_no, token->lexeme);
    } else if (token_is_nonTypeName(token)) {
      primary = build_nonTypeName(false);
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      Ast_ExprList* expr_list = arena_push_struct(ast_storage, Ast_ExprList);
      expr_list->kind = AST_EXPRLIST;
      expr_list->id = node_id++;
      expr_list->line_no = token->line_no;
      expr_list->expr_list = build_expressionList();
      primary = (Ast*)expr_list;
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_no, token->lexeme);
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        Ast_CastExpr* cast_expr = arena_push_struct(ast_storage, Ast_CastExpr);
        cast_expr->kind = AST_CAST_EXPR;
        cast_expr->id = node_id++;
        cast_expr->line_no = token->line_no;
        cast_expr->to_type = build_typeRef();
        primary = (Ast*)cast_expr;
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
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NOT;
      unary_expr->operand = build_expression(1);
      primary = (Ast*)unary_expr;
    } else if (token->klass == TK_TILDA) {
      next_token();
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_BITWISE_NOT;
      unary_expr->operand = build_expression(1);
      primary = (Ast*)unary_expr;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      Ast_UnaryExpr* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpr);
      unary_expr->kind = AST_UNARY_EXPR;
      unary_expr->id = node_id++;
      unary_expr->line_no = token->line_no;
      unary_expr->op = OP_NEG;
      unary_expr->operand = build_expression(1);
      primary = (Ast*)unary_expr;
    } else if (token_is_typeName(token)) {
      primary = build_typeName();
    } else if (token->klass == TK_ERROR) {
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_NAME;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->strname = token->lexeme;
      primary = (Ast*)name;
      next_token();
    } else assert(0);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  return primary;
}

internal int
get_operator_priority(Token* token)
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
  Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOTPREFIX) {
        next_token();
        Ast_MemberSelect* select_expr = arena_push_struct(ast_storage, Ast_MemberSelect);
        select_expr->kind = AST_MEMBER_SELECT;
        select_expr->id = node_id++;
        select_expr->line_no = token->line_no;
        select_expr->lhs_expr = expr;
        expr = (Ast*)select_expr;
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
        subscript_expr->index = expr;
        subscript_expr->after_colon = build_arraySubscript();
        expr = (Ast*)subscript_expr;
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
        call_expr->callee_expr = expr;
        call_expr->args = build_argumentList();
        expr = (Ast*)call_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_no, token->lexeme);
      }
      else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        ((Ast_Expression*)expr)->type_args = build_realTypeArgumentList();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_no, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        Ast_KVPair* kv_pair = arena_push_struct(ast_storage, Ast_KVPair);
        kv_pair->kind = AST_KVPAIR;
        kv_pair->id = node_id++;
        kv_pair->line_no = token->line_no;
        kv_pair->name = expr;
        kv_pair->expr = build_expression(1);
        expr = (Ast*)kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          Ast_BinaryExpr* bin_expr = arena_push_struct(ast_storage, Ast_BinaryExpr);
          bin_expr->kind = AST_BINARY_EXPR;
          bin_expr->id = node_id++;
          bin_expr->line_no = token->line_no;
          bin_expr->left_operand = expr;
          bin_expr->op = token_to_binop(token);
          next_token();
          bin_expr->right_operand = build_expression(priority + 1);
          expr = (Ast*)bin_expr;
        } else break;
      } else assert(0);
    }
  } else error("at line %d: an expression was expected, got `%s`.", token->line_no, token->lexeme);
  return expr;
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
