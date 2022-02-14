#include "arena.h"
#include "hashmap.h"
#include "token.h"
#include "ast.h"
#include "lex.h"
#include "symtable.h"
#include "build_ast.h"
#include <memory.h>  // memset


internal struct Arena *ast_storage;
internal struct Arena local_storage = {};

internal struct UnboundedArray* tokens_array;
internal int token_at = 0;
internal struct Token* token = 0;
internal int prev_token_at = 0;
internal struct Token* prev_token = 0;

internal int node_id = 1;
internal int node_count = 0;

internal struct Ast* build_expression(int priority_threshold);
internal struct Ast* build_typeRef();
internal struct Ast* build_blockStatement();
internal struct Ast* build_statement(struct Ast* type_name);
internal struct Ast* build_parserStatement();


internal void
init_ast_node(struct Ast* ast, enum AstKind kind, int line_nr)
{
  ast->kind = kind;
  ast->id = node_id++;
  ast->line_nr = line_nr;
  node_count += 1;
}

#define new_ast_node(ast_type, ast_kind, line_nr) ({ \
  ast_type* ast = arena_push(ast_storage, sizeof(ast_type)); \
  memset(ast, 0, sizeof(ast_type)); \
  init_ast_node((struct Ast*)ast, ast_kind, line_nr); \
  ast; \
})

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
    struct SymtableEntry* entry = scope_resolve_name(get_current_scope(), token->lexeme);
    if (entry->ns_keyword) {
      token->klass = ((struct Object_Keyword*)entry->ns_keyword)->token_klass;
      return token;
    }
    if (entry->ns_type) {
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
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOT_PREFIX;
}

internal bool
token_is_prefixedType(struct Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_DOT_PREFIX;
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
  bool result = token->klass == TK_DOT_PREFIX || token_is_nonTypeName(token)
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
  bool result = token->klass == TK_INTEGER || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token->klass == TK_DOT_PREFIX || token_is_nonTypeName(token)
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

internal struct Ast*
build_nonTypeName(bool is_type)
{
  struct Ast_Name* name = 0;
  if (token_is_nonTypeName(token)) {
    name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
    name->strname = token->lexeme;
    if (is_type) {
      struct NamedObject* descriptor = arena_push(ast_storage, sizeof(*descriptor));
      memset(descriptor, 0, sizeof(*descriptor));
      descriptor->strname = name->strname;
      declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, token->line_nr);
    }
    next_token();
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_nr, token->lexeme);
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
      struct Ast_Name* type_name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
      type_name->strname = token->lexeme;
      name = type_name;
      next_token();
    } else assert(0);
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)name;
}

internal struct List*
build_typeParameterList()
{
  struct List* params = 0;
  if (token_is_typeParameterList(token)) {
    params = arena_push(ast_storage, sizeof(*params));
    memset(params, 0, sizeof(*params));
    list_init(params);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_name(true);
    list_append_link(params, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_name(true);
      list_append_link(params, link);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
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
      } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
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
      struct Ast_Dontcare* dontcare = new_ast_node(struct Ast_Dontcare, AST_DONTCARE, token->line_nr);
      arg = (struct Ast*)dontcare;
      next_token();
    } else if (token_is_typeRef(token)) {
      arg = build_typeRef();
    } else if (token_is_nonTypeName(token)) {
      arg = build_nonTypeName(false);
    } else assert(0);
  } else error("at line %d: type argument was expected, got `%s`.", token->line_nr, token->lexeme);
  return arg;
}

internal bool
token_is_methodPrototype(struct Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

internal enum AstParamDirection
build_direction()
{
  enum AstParamDirection dir = PARAMDIR_NONE;
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
  struct Ast_Param* param = new_ast_node(struct Ast_Param, AST_PARAM, token->line_nr);
  param->direction = build_direction();
  if (token_is_typeRef(token)) {
    param->type = build_typeRef();
    if (token_is_name(token)) {
      param->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          param->init_expr = build_expression(1);
        } else error("at line %d: expression was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)param;
}

internal struct List*
build_parameterList()
{
  struct List* params = 0;
  if (token_is_parameter(token)) {
    params = arena_push(ast_storage, sizeof(*params));
    memset(params, 0, sizeof(*params));
    list_init(params);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_parameter();
    list_append_link(params, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_parameter();
      list_append_link(params, link);
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
      struct Ast_Name* void_name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
      void_name->strname = token->lexeme;
      type = (struct Ast*)void_name;
      next_token();
    } else if (token->klass == TK_IDENTIFIER) {
      struct Ast_Name* name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
      name->strname = token->lexeme;
      type = (struct Ast*)name;
      if (is_type) {
        struct NamedObject* descriptor = arena_push(ast_storage, sizeof(*descriptor));
        memset(descriptor, 0, sizeof(*descriptor));
        descriptor->strname = name->strname;
        declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, descriptor, token->line_nr);
      }
      next_token();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_functionPrototype(struct Ast* return_type)
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_typeOrVoid(token) || return_type) {
    proto = new_ast_node(struct Ast_FunctionProto, AST_FUNCTION_PROTO, token->line_nr);
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
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: function name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)proto;
}

internal struct Ast*
build_methodPrototype()
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      proto = new_ast_node(struct Ast_FunctionProto, AST_FUNCTION_PROTO, token->line_nr);
      proto->name = build_name(false);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      proto = (struct Ast_FunctionProto*)build_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)proto;
}

internal struct List*
build_methodPrototypes()
{
  struct List* protos = 0;
  if (token_is_methodPrototype(token)) {
    protos = arena_push(ast_storage, sizeof(*protos));
    memset(protos, 0, sizeof(*protos));
    list_init(protos);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_methodPrototype();
    list_append_link(protos, link);
    while (token_is_methodPrototype(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_methodPrototype();
      list_append_link(protos, link);
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
    } else error("at line %d: extern declaration was expected, got `%s`.", token->line_nr, token->lexeme);

    if (is_function_proto) {
      struct Ast_FunctionProto* proto = (struct Ast_FunctionProto*)build_functionPrototype(0);
      decl = (struct Ast*)proto;
      proto->is_extern = true;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else {
      struct Ast_ExternDecl* extern_decl = new_ast_node(struct Ast_ExternDecl, AST_EXTERN_DECL, token->line_nr);
      decl = (struct Ast*)extern_decl;
      extern_decl->name = build_nonTypeName(true);
      extern_decl->type_params = build_optTypeParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_decl->method_protos = build_methodPrototypes();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  }
  return decl;
}

internal struct Ast*
build_integer()
{
  struct Ast_IntLiteral* int_node = 0;
  if (token->klass == TK_INTEGER) {
    int_node = new_ast_node(struct Ast_IntLiteral, AST_INT_LITERAL, token->line_nr);
    int_node->flags = token->i.flags;
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
    bool_node = new_ast_node(struct Ast_BoolLiteral, AST_BOOL_LITERAL, token->line_nr);
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
    string = new_ast_node(struct Ast_StringLiteral, AST_STRING_LITERAL, token->line_nr);
    string->value = token->lexeme;
    next_token();
  }
  return (struct Ast*)string;
}

internal struct Ast*
build_integerTypeSize()
{
  struct Ast_IntTypeSize* type_size = new_ast_node(struct Ast_IntTypeSize, AST_INT_TYPESIZE, token->line_nr);
  if (token->klass == TK_INTEGER) {
    type_size->size = build_integer();
  } else if (token->klass == TK_PARENTH_OPEN) {
    type_size->size = build_expression(1);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type_size;
}

internal struct Ast*
build_baseType()
{
  struct Ast* base_type = 0;
  if (token_is_baseType(token)) {
    struct Ast_Name* type_name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
    if (token->klass == TK_BOOL) {
      struct Ast_BaseType_Bool* bool_type = new_ast_node(struct Ast_BaseType_Bool, AST_BASETYPE_BOOL, token->line_nr);
      type_name->strname = "bool";
      bool_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bool_type;
      next_token();
    } else if (token->klass == TK_ERROR) {
      struct Ast_BaseType_Error* error_type = new_ast_node(struct Ast_BaseType_Error, AST_BASETYPE_ERROR, token->line_nr);
      type_name->strname = "error";
      error_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)error_type;
      next_token();
    } else if (token->klass == TK_INT) {
      struct Ast_BaseType_Int* int_type = new_ast_node(struct Ast_BaseType_Int, AST_BASETYPE_INT, token->line_nr);
      type_name->strname = "int";
      int_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)int_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == TK_BIT) {
      struct Ast_BaseType_Bit* bit_type = new_ast_node(struct Ast_BaseType_Bit, AST_BASETYPE_BIT, token->line_nr);
      type_name->strname = "bit";
      bit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == TK_VARBIT) {
      struct Ast_BaseType_Varbit* varbit_type = new_ast_node(struct Ast_BaseType_Varbit, AST_BASETYPE_VARBIT, token->line_nr);
      type_name->strname = "varbit";
      varbit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)varbit_type;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = build_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == TK_STRING) {
      struct Ast_BaseType_String* string_type = new_ast_node(struct Ast_BaseType_String, AST_BASETYPE_STRING, token->line_nr);
      type_name->strname = "string";
      string_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)string_type;
      next_token();
    } else if (token->klass == TK_VOID) {
      struct Ast_BaseType_Void* void_type = new_ast_node(struct Ast_BaseType_Void, AST_BASETYPE_VOID, token->line_nr);
      type_name->strname = "void";
      void_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)void_type;
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", token->line_nr, token->lexeme);
  return base_type;
}

internal struct List*
build_typeArgumentList()
{
  struct List* args = 0;
  if (token_is_typeArg(token)) {
    args = arena_push(ast_storage, sizeof(*args));
    memset(args, 0, sizeof(*args));
    list_init(args);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_typeArg();
    list_append_link(args, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_typeArg();
      list_append_link(args, link);
    }
  }
  return args;
}

internal struct Ast*
build_tupleType()
{
  struct Ast_Tuple* type = 0;
  if (token->klass == TK_TUPLE) {
    next_token();
    type = new_ast_node(struct Ast_Tuple, AST_TUPLE, token->line_nr);
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      type->type_args = build_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `tuple` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_headerStackType()
{
  struct Ast_HeaderStack* stack = 0;
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    stack = new_ast_node(struct Ast_HeaderStack, AST_HEADER_STACK, token->line_nr);
    if (token_is_expression(token)) {
      stack->stack_expr = build_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: an expression expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `[` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)stack;
}

internal struct Ast*
build_specializedType()
{
  struct Ast_SpecializedType* type = 0;
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    type = new_ast_node(struct Ast_SpecializedType, AST_SPECIALIZED_TYPE, token->line_nr);
    type->type_args = build_typeArgumentList();
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_prefixedType()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOT_PREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
    name->strname = token->lexeme;
    name->is_dotprefixed = is_dotprefixed;
    next_token();
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
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
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
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
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
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
  struct Ast_StructField* field = new_ast_node(struct Ast_StructField, AST_STRUCT_FIELD, token->line_nr);
  if (token_is_typeRef(token)) {
    field->type = build_typeRef();
    if (token_is_name(token)) {
      field->name = build_name(false);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: struct field was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)field;
}

internal struct List*
build_structFieldList()
{
  struct List* fields = 0;
  if (token_is_structField(token)) {
    fields = arena_push(ast_storage, sizeof(*fields));
    memset(fields, 0, sizeof(*fields));
    list_init(fields);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_structField();
    list_append_link(fields, link);
    while (token_is_structField(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_structField();
      list_append_link(fields, link);
    }
  }
  return fields;
}

internal struct Ast*
build_headerTypeDeclaration()
{
  struct Ast_HeaderDecl* decl = 0;
  if (token->klass == TK_HEADER) {
    next_token();
    decl = new_ast_node(struct Ast_HeaderDecl, AST_HEADER_DECL, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `header` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_headerUnionDeclaration()
{
  struct Ast_HeaderUnionDecl* decl = 0;
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    decl = new_ast_node(struct Ast_HeaderUnionDecl, AST_HEADER_UNION_DECL, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `header_union` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_structTypeDeclaration()
{
  struct Ast_StructDecl* decl = 0;
  if (token->klass == TK_STRUCT) {
    next_token();
    decl = new_ast_node(struct Ast_StructDecl, AST_STRUCT_DECL, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `struct` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    id = new_ast_node(struct Ast_SpecifiedIdent, AST_SPECIFIED_IDENT, token->line_nr);
    id->name = build_name(false);
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->init_expr = build_initializer();
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)id;
}

internal struct List*
build_specifiedIdentifierList()
{
  struct List* ids = 0;
  if (token_is_specifiedIdentifier(token)) {
    ids = arena_push(ast_storage, sizeof(*ids));
    memset(ids, 0, sizeof(*ids));
    list_init(ids);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_specifiedIdentifier();
    list_append_link(ids, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_specifiedIdentifier();
      list_append_link(ids, link);
    }
  }
  return ids;
}

internal struct Ast*
build_enumDeclaration()
{
  struct Ast_EnumDecl* decl = 0;
  if (token->klass == TK_ENUM) {
    next_token();
    decl = new_ast_node(struct Ast_EnumDecl, AST_ENUM_DECL, token->line_nr);
    if (token->klass == TK_BIT) {
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        if (token->klass == TK_INTEGER) {
          decl->type_size = build_integer();
          if (token->klass == TK_ANGLE_CLOSE) {
            next_token();
          } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: an integer was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          decl->id_list = build_specifiedIdentifierList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `enum` was expected, got `%s`.", token->line_nr, token->lexeme);
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
  } else error("at line %d: structure declaration was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_parserTypeDeclaration()
{
  struct Ast_ParserProto* type = 0;
  if (token->klass == TK_PARSER) {
    next_token();
    type = new_ast_node(struct Ast_ParserProto, AST_PARSER_PROTO, token->line_nr);
    if (token_is_name(token)) {
      type->name = build_name(true);
      type->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        type->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
  }
  return ctor_params;
}

internal struct Ast*
build_constantDeclaration()
{
  struct Ast_ConstDecl* decl = 0;
  if (token->klass == TK_CONST) {
    next_token();
    decl = new_ast_node(struct Ast_ConstDecl, AST_CONST_DECL, token->line_nr);
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
            } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
          } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `const` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal bool
token_is_declaration(struct Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token) || token->klass == TK_DOT_PREFIX;
  return result;
}

internal bool
token_is_lvalue(struct Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == TK_DOT_PREFIX;
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
      struct Ast_Argument* name_arg = new_ast_node(struct Ast_Argument, AST_ARGUMENT, token->line_nr);
      arg = (struct Ast*)name_arg;
      name_arg->name = build_name(false);
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          name_arg->init_expr = build_expression(1);
        } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_DONTCARE) {
      struct Ast_Dontcare* dontcare_arg = new_ast_node(struct Ast_Dontcare, AST_DONTCARE, token->line_nr);
      arg = (struct Ast*)dontcare_arg;
      next_token();
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", token->line_nr, token->lexeme);
  return arg;
}

internal struct List*
build_argumentList()
{
  struct List* args = 0;
  if (token_is_argument(token)) {
    args = arena_push(ast_storage, sizeof(*args));
    memset(args, 0, sizeof(*args));
    list_init(args);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_argument();
    list_append_link(args, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_argument();
      list_append_link(args, link);
    }
  }
  return args;
}

internal struct Ast*
build_variableDeclaration(struct Ast* type_ref)
{
  struct Ast_VarDecl* decl = 0;
  if (token_is_typeRef(token) || type_ref) {
    decl = new_ast_node(struct Ast_VarDecl, AST_VAR_DECL, token->line_nr);
    decl->type = type_ref ? type_ref : build_typeRef();
    if (token_is_name(token)) {
      decl->name = build_name(false);
      decl->init_expr = build_optInitializer();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_instantiation(struct Ast* type_ref)
{
  struct Ast_Instantiation* inst = 0;
  if (token_is_typeRef(token) || type_ref) {
    inst = new_ast_node(struct Ast_Instantiation, AST_INSTANTIATION, token->line_nr);
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
          } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: instance name was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
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
      } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
    } else assert(0);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_nr, token->lexeme);
  return elem;
}

internal struct List*
build_parserLocalElements()
{
  struct List* elems = 0;
  if (token_is_parserLocalElement(token)) {
    elems = arena_push(ast_storage, sizeof(*elems));
    memset(elems, 0, sizeof(*elems));
    list_init(elems);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_parserLocalElement();
    list_append_link(elems, link);
    while (token_is_parserLocalElement(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_parserLocalElement();
      list_append_link(elems, link);
    }
  }
  return elems;
}

internal struct Ast*
build_directApplication(struct Ast* type_name)
{
  struct Ast_DirectApplication* applic = 0;
  if (token_is_typeName(token) || type_name) {
    applic = new_ast_node(struct Ast_DirectApplication, AST_DIRECT_APPLICATION, token->line_nr);
    applic->name = type_name ? type_name : build_typeName();
    if (token->klass == TK_DOT_PREFIX) {
      next_token();
      if (token->klass == TK_APPLY) {
        next_token();
        if (token->klass == TK_PARENTH_OPEN) {
          next_token();
          applic->args = build_argumentList();
          if (token->klass == TK_PARENTH_CLOSE) {
            next_token();
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
          } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `apply` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `.` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type name was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)applic;
}

internal struct Ast*
build_prefixedNonTypeName()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (token->klass == TK_DOT_PREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    name = (struct Ast_Name*)build_nonTypeName(false);
    name->is_dotprefixed = is_dotprefixed;
  } else error("at line %d: non-type name was expected, ", token->line_nr, token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_arrayIndex()
{
  struct Ast_IndexedArrayExpr* index = new_ast_node(struct Ast_IndexedArrayExpr, AST_INDEXEDARRAY_EXPR, token->line_nr);
  if (token_is_expression(token)) {
    index->index = build_expression(1);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  if (token->klass == TK_COLON) {
    next_token();
    if (token_is_expression(token)) {
      index->colon_index = build_expression(1);
    } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  }
  return (struct Ast*)index;
}

internal struct Ast*
build_lvalueExpr()
{
  struct Ast* expr = 0;
  if (token->klass == TK_DOT_PREFIX) {
    next_token();
    struct Ast_Name* dot_member = (struct Ast_Name*)build_name(false);
    dot_member->is_dotprefixed = true;
    expr = (struct Ast*)dot_member;
  } else if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    expr = build_arrayIndex();
    if (token->klass == TK_BRACKET_CLOSE) {
      next_token();
    } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct Ast*
build_lvalue()
{
  struct Ast_Lvalue* lvalue = 0;
  if (token_is_lvalue(token)) {
    lvalue = new_ast_node(struct Ast_Lvalue, AST_LVALUE, token->line_nr);
    lvalue->name = build_prefixedNonTypeName();
    if (token->klass == TK_DOT_PREFIX || token->klass == TK_BRACKET_OPEN) {
      struct List* lvalue_expr = arena_push(ast_storage, sizeof(*lvalue_expr));
      memset(lvalue_expr, 0, sizeof(*lvalue_expr));
      list_init(lvalue_expr);
      struct ListLink* link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_lvalueExpr();
      list_append_link(lvalue_expr, link);
      while (token->klass == TK_DOT_PREFIX || token->klass == TK_BRACKET_OPEN) {
        link = arena_push(ast_storage, sizeof(*link));
        memset(link, 0, sizeof(*link));
        link->object = build_lvalueExpr();
        list_append_link(lvalue_expr, link);
      }
      lvalue->expr = lvalue_expr;
    }
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_nr, token->lexeme);
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
      } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      struct Ast_MethodCallStmt* call_stmt = new_ast_node(struct Ast_MethodCallStmt, AST_METHODCALL_STMT, token->line_nr);
      call_stmt->lvalue = lvalue;
      call_stmt->type_args = type_args;
      call_stmt->args = build_argumentList();
      stmt = (struct Ast*)call_stmt;
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_EQUAL) {
      next_token();
      struct Ast_AssignmentStmt* assgn_stmt = new_ast_node(struct Ast_AssignmentStmt, AST_ASSIGNMENT_STMT, token->line_nr);
      assgn_stmt->lvalue = lvalue;
      assgn_stmt->expr = build_expression(1);
      stmt = (struct Ast*)assgn_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct List*
build_parserStatements()
{
  struct List* stmts = 0;
  if (token_is_parserStatement(token)) {
    stmts = arena_push(ast_storage, sizeof(*stmts));
    memset(stmts, 0, sizeof(*stmts));
    list_init(stmts);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_parserStatement();
    list_append_link(stmts, link);
    while (token_is_parserStatement(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_parserStatement();
      list_append_link(stmts, link);
    }
  }
  return stmts;
}

internal struct Ast*
build_parserBlockStatements()
{
  struct Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = new_ast_node(struct Ast_BlockStmt, AST_BLOCK_STMT, token->line_nr);
    next_token();
    stmt->stmt_list = build_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    stmt = (struct Ast*)new_ast_node(struct Ast_EmptyStmt, AST_EMPTY_STMT, token->line_nr);
  } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct List*
build_expressionList()
{
  struct List* exprs = 0;
  if (token_is_expression(token)) {
    exprs = arena_push(ast_storage, sizeof(*exprs));
    memset(exprs, 0, sizeof(*exprs));
    list_init(exprs);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_expression(1);
    list_append_link(exprs, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_expression(1);
      list_append_link(exprs, link);
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
    expr = (struct Ast*)new_ast_node(struct Ast_DefaultStmt, AST_DEFAULT_STMT, token->line_nr);
  } else if (token->klass == TK_DONTCARE) {
    next_token();
    expr = (struct Ast*)new_ast_node(struct Ast_Dontcare, AST_DONTCARE, token->line_nr);
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct Ast*
build_tupleKeysetExpression()
{
  struct Ast_TupleKeyset* tuple_keyset = 0;
  if (token->klass == TK_PARENTH_OPEN) {
    tuple_keyset = new_ast_node(struct Ast_TupleKeyset, AST_TUPLE_KEYSET, token->line_nr);
    next_token();
    struct List* exprs = arena_push(ast_storage, sizeof(*exprs));
    memset(exprs, 0, sizeof(*exprs));
    list_init(exprs);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_simpleKeysetExpression();
    list_append_link(exprs, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_simpleKeysetExpression();
      list_append_link(exprs, link);
    }
    tuple_keyset->expr_list = exprs;
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
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
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct Ast*
build_selectCase()
{
  struct Ast_SelectCase* select_case = 0;
  if (token_is_keysetExpression(token)) {
    select_case = new_ast_node(struct Ast_SelectCase, AST_SELECT_CASE, token->line_nr);
    select_case->keyset = build_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_name(token)) {
        select_case->name = build_name(false);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)select_case;
}

internal struct List*
build_selectCaseList()
{
  struct List* cases = 0;
  if (token_is_selectCase(token)) {
    cases = arena_push(ast_storage, sizeof(*cases));
    memset(cases, 0, sizeof(*cases));
    list_init(cases);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_selectCase();
    list_append_link(cases, link);
    while (token_is_selectCase(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_selectCase();
      list_append_link(cases, link);
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
    select_expr = new_ast_node(struct Ast_SelectExpr, AST_SELECT_EXPR, token->line_nr);
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
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `select` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else if (token->klass == TK_SELECT) {
    state_expr = build_selectExpression();
  } else error("at line %d: state expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return state_expr;
}

internal struct Ast*
build_transitionStatement()
{
  struct Ast* stmt = 0;
  if (token->klass == TK_TRANSITION) {
    next_token();
    stmt = build_stateExpression();
  } else error("at line %d: `transition` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct Ast*
build_parserState()
{
  struct Ast_ParserState* state = 0;
  if (token->klass == TK_STATE) {
    next_token();
    state = new_ast_node(struct Ast_ParserState, AST_PARSER_STATE, token->line_nr);
    state->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      state->stmt_list = build_parserStatements();
      state->trans_stmt = build_transitionStatement();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `state` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)state;
}

internal struct List*
build_parserStates()
{
  struct List* states = 0;
  if (token->klass == TK_STATE) {
    states = arena_push(ast_storage, sizeof(*states));
    memset(states, 0, sizeof(*states));
    list_init(states);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_parserState();
    list_append_link(states, link);
    while (token->klass == TK_STATE) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_parserState();
      list_append_link(states, link);
    }
  } else error("at line %d: `state` was expected, got `%s`.", token->line_nr, token->lexeme);
  return states;
}

internal struct Ast*
build_parserDeclaration()
{
  struct Ast_ParserDecl* decl = 0;
  if (token->klass == TK_PARSER) {
    decl = new_ast_node(struct Ast_ParserDecl, AST_PARSER_DECL, token->line_nr);
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
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_controlTypeDeclaration()
{
  struct Ast_ControlProto* decl = 0;
  if (token->klass == TK_CONTROL) {
    next_token();
    decl = new_ast_node(struct Ast_ControlProto, AST_CONTROL_PROTO, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `control` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_actionDeclaration()
{
  struct Ast_ActionDecl* decl = 0;
  if (token->klass == TK_ACTION) {
    next_token();
    decl = new_ast_node(struct Ast_ActionDecl, AST_ACTION_DECL, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(false);
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TK_BRACE_OPEN) {
            decl->stmt = build_blockStatement();
          } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `action` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_keyElement()
{
  struct Ast_KeyElement* key_elem = 0;
  if (token_is_expression(token)) {
    key_elem = new_ast_node(struct Ast_KeyElement, AST_KEY_ELEMENT, token->line_nr);
    key_elem->expr = build_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->name = build_name(false);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)key_elem;
}

internal struct List*
build_keyElementList()
{
  struct List* elems = 0;
  if (token_is_expression(token)) {
    elems = arena_push(ast_storage, sizeof(*elems));
    memset(elems, 0, sizeof(*elems));
    list_init(elems);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_keyElement();
    list_append_link(elems, link);
    while (token_is_expression(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_keyElement();
      list_append_link(elems, link);
    }
  }
  return elems;
}

internal struct Ast*
build_actionRef()
{
  struct Ast_ActionRef* ref = 0;
  if (token->klass == TK_DOT_PREFIX || token_is_nonTypeName(token)) {
    ref = new_ast_node(struct Ast_ActionRef, AST_ACTION_REF, token->line_nr);
    ref->name = build_prefixedNonTypeName();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      ref->args = build_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)ref;
}

internal struct List*
build_actionList()
{
  struct List* actions = 0;
  if (token_is_actionRef(token)) {
    actions = arena_push(ast_storage, sizeof(*actions));
    memset(actions, 0, sizeof(*actions));
    list_init(actions);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_actionRef();
    list_append_link(actions, link);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    while (token_is_actionRef(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_actionRef();
      list_append_link(actions, link);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  }
  return actions;
}

internal struct Ast*
build_entry()
{
  struct Ast_TableEntry* entry = 0;
  if (token_is_keysetExpression(token)) {
    entry = new_ast_node(struct Ast_TableEntry, AST_TABLE_ENTRY, token->line_nr);
    entry->keyset = build_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      entry->action = build_actionRef();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: keyset was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)entry;
}

internal struct List*
build_entriesList()
{
  struct List* entries = 0;
  if (token_is_keysetExpression(token)) {
    entries = arena_push(ast_storage, sizeof(*entries));
    memset(entries, 0, sizeof(*entries));
    list_init(entries);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_entry();
    list_append_link(entries, link);
    while (token_is_keysetExpression(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_entry();
      list_append_link(entries, link);
    }
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
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
      struct Ast_TableKey* key_prop = new_ast_node(struct Ast_TableKey, AST_TABLE_KEY, token->line_nr);
      prop = (struct Ast*)key_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          key_prop->keyelem_list = build_keyElementList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      struct Ast_TableActions* actions_prop = new_ast_node(struct Ast_TableActions, AST_TABLE_ACTIONS, token->line_nr);
      prop = (struct Ast*)actions_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          actions_prop->action_list = build_actionList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      struct Ast_TableEntries* entries_prop = new_ast_node(struct Ast_TableEntries, AST_TABLE_ENTRIES, token->line_nr);
      entries_prop->is_const = is_const;
      prop = (struct Ast*)entries_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          entries_prop->entries = build_entriesList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTableKwName(token)) {
      struct Ast_TableSingleEntry* entry_prop = new_ast_node(struct Ast_TableSingleEntry, AST_TABLE_SINGLE_ENTRY, token->line_nr);
      entry_prop->name = build_name(false);
      prop = (struct Ast*)entry_prop;
      if (token->klass == TK_EQUAL) {
        next_token();
        entry_prop->init_expr = build_initializer();
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else assert(0);
  } else error("at line %d: table property was expected, got `%s`.", token->line_nr, token->lexeme);
  return prop;
}

internal struct List*
build_tablePropertyList()
{
  struct List* props = 0;
  if (token_is_tableProperty(token)) {
    props = arena_push(ast_storage, sizeof(*props));
    memset(props, 0, sizeof(*props));
    list_init(props);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_tableProperty();
    list_append_link(props, link);
    while (token_is_tableProperty(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_tableProperty();
      list_append_link(props, link);
    }
  } else error("at line %d: table property was expected, got `%s`.", token->line_nr, token->lexeme);
  return props;
}

internal struct Ast*
build_tableDeclaration()
{
  struct Ast_TableDecl* table = 0;
  if (token->klass == TK_TABLE) {
    next_token();
    table = new_ast_node(struct Ast_TableDecl, AST_TABLE_DECL, token->line_nr);
    table->name = build_name(false);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      table->prop_list = build_tablePropertyList();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `table` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: local declaration was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct List*
build_controlLocalDeclarations()
{
  struct List* decls = 0;
  if (token_is_controlLocalDeclaration(token)) {
    decls = arena_push(ast_storage, sizeof(*decls));
    memset(decls, 0, sizeof(*decls));
    list_init(decls);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_controlLocalDeclaration();
    list_append_link(decls, link);
    while (token_is_controlLocalDeclaration(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_controlLocalDeclaration();
      list_append_link(decls, link);
    }
  }
  return decls;
}

internal struct Ast*
build_controlDeclaration()
{
  struct Ast_ControlDecl* decl = 0;
  if (token->klass == TK_CONTROL) {
    decl = new_ast_node(struct Ast_ControlDecl, AST_CONTROL_DECL, token->line_nr);
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
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `apply` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: `control` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_packageTypeDeclaration()
{
  struct Ast_PackageDecl* decl = 0;
  if (token->klass == TK_PACKAGE) {
    next_token();
    decl = new_ast_node(struct Ast_PackageDecl, AST_PACKAGE_DECL, token->line_nr);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `package` was expected, got `%s`.", token->line_nr, token->lexeme);
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
      struct Ast_TypeDecl* type_decl = new_ast_node(struct Ast_TypeDecl, AST_TYPE_DECL, token->line_nr);
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
        } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type definition was expected, got `%s`.", token->line_nr, token->lexeme);
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
      } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
    } else assert(0);
  } else error("at line %d: type declaration was expected, got `%s`.", token->line_nr, token->lexeme); 
  return decl;
}

internal struct Ast*
build_conditionalStatement()
{
  struct Ast_IfStmt* if_stmt = 0;
  if (token->klass == TK_IF) {
    next_token();
    if_stmt = new_ast_node(struct Ast_IfStmt, AST_IF_STMT, token->line_nr);
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
              } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
            }
          } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `if` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)if_stmt;
}

internal struct Ast*
build_exitStatement()
{
  struct Ast_ExitStmt* exit_stmt = 0;
  if (token->klass == TK_EXIT) {
    next_token();
    exit_stmt = new_ast_node(struct Ast_ExitStmt, AST_EXIT_STMT, token->line_nr);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)exit_stmt;
}

internal struct Ast*
build_returnStatement()
{
  struct Ast_ReturnStmt* ret_stmt = 0;
  if (token->klass == TK_RETURN) {
    next_token();
    ret_stmt = new_ast_node(struct Ast_ReturnStmt, AST_RETURN_STMT, token->line_nr);
    if (token_is_expression(token))
      ret_stmt->expr = build_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `return` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    label = (struct Ast*)new_ast_node(struct Ast_DefaultStmt, AST_DEFAULT_STMT, token->line_nr);
  } else error("at line %d: switch label was expected, got `%s`.", token->line_nr, token->lexeme);
  return label;
}

internal struct Ast*
build_switchCase()
{
  struct Ast_SwitchCase* switch_case = 0;
  if (token_is_switchLabel(token)) {
    switch_case = new_ast_node(struct Ast_SwitchCase, AST_SWITCH_CASE, token->line_nr);
    switch_case->label = build_switchLabel();
    if (token->klass == TK_COLON) {
      next_token();
      if (token->klass == TK_BRACE_OPEN) {
        switch_case->stmt = build_blockStatement();
      }
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: switch label was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)switch_case;
}

internal struct List*
build_switchCases()
{
  struct List* cases = 0;
  if (token_is_switchLabel(token)) {
    cases = arena_push(ast_storage, sizeof(*cases));
    memset(cases, 0, sizeof(*cases));
    list_init(cases);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_switchCase();
    list_append_link(cases, link);
    while (token_is_switchLabel(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_switchCase();
      list_append_link(cases, link);
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
    stmt = new_ast_node(struct Ast_SwitchStmt, AST_SWITCH_STMT, token->line_nr);
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
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `switch` was expected, got `%s`.", token->line_nr, token->lexeme);
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
    stmt = (struct Ast*)new_ast_node(struct Ast_EmptyStmt, AST_EMPTY_STMT, token->line_nr);
  } else if (token->klass == TK_BRACE_OPEN) {
    stmt = build_blockStatement();
  } else if (token->klass == TK_EXIT) {
    stmt = build_exitStatement();
  } else if (token->klass == TK_RETURN) {
    stmt = build_returnStatement();
  } else if (token->klass == TK_SWITCH) {
    stmt = build_switchStatement();
  } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
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
    stmts = arena_push(ast_storage, sizeof(*stmts));
    memset(stmts, 0, sizeof(*stmts));
    list_init(stmts);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_statementOrDecl();
    list_append_link(stmts, link);
    while (token_is_statementOrDeclaration(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_statementOrDecl();
      list_append_link(stmts, link);
    }
  }
  return stmts;
}

internal struct Ast*
build_blockStatement()
{
  struct Ast_BlockStmt* stmt = 0;
  if (token->klass == TK_BRACE_OPEN) {
    stmt = new_ast_node(struct Ast_BlockStmt, AST_BLOCK_STMT, token->line_nr);
    next_token();
    stmt->stmt_list = build_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)stmt;
}

internal struct List*
build_identifierList()
{
  struct List* ids = 0;
  if (token_is_name(token)) {
    ids = arena_push(ast_storage, sizeof(*ids));
    memset(ids, 0, sizeof(*ids));
    list_init(ids);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_name(false);
    list_append_link(ids, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_name(false);
      list_append_link(ids, link);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return ids;
}

internal struct Ast*
build_errorDeclaration()
{
  struct Ast_ErrorDecl* decl = 0;
  if (token->klass == TK_ERROR) {
    next_token();
    decl = new_ast_node(struct Ast_ErrorDecl, AST_ERROR_DECL, token->line_nr);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `error` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_matchKindDeclaration()
{
  struct Ast_MatchKindDecl* decl = 0;
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    decl = new_ast_node(struct Ast_MatchKindDecl, AST_MATCH_KIND_DECL, token->line_nr);
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `match_kind` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_functionDeclaration(struct Ast* type_ref)
{
  struct Ast_FunctionDecl* decl = 0;
  if (token_is_typeOrVoid(token)) {
    decl = new_ast_node(struct Ast_FunctionDecl, AST_FUNCTION_DECL, token->line_nr);
    decl->proto = build_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      decl->stmt = build_blockStatement();
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
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
      } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      decl = build_functionDeclaration(build_typeRef());
    } else assert(0);
  } else error("at line %d: top-level declaration as expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_p4program()
{
  struct Ast_P4Program* program = new_ast_node(struct Ast_P4Program, AST_P4PROGRAM, token->line_nr);
  struct List* decls = arena_push(ast_storage, sizeof(*decls));
  memset(decls, 0, sizeof(*decls));
  list_init(decls);
  while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
    if (token_is_declaration(token)) {
      struct ListLink* link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_declaration();
      list_append_link(decls, link);
    } else if (token->klass == TK_SEMICOLON) {
      next_token(); /* empty declaration */
    }
  }
  program->decl_list = decls;
  if (token->klass != TK_END_OF_INPUT) {
    error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
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
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOT_PREFIX
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
    arg = (struct Ast*)new_ast_node(struct Ast_Dontcare, AST_DONTCARE, token->line_nr);
  } else if (token_is_typeRef(token)) {
    arg = build_typeRef();
  } else error("at line %d: type argument was expected, got `%s`.", token->line_nr, token->lexeme);
  return arg;
}

internal struct List*
build_realTypeArgumentList()
{
  struct List* args = 0;
  if (token_is_realTypeArg(token)) {
    args = arena_push(ast_storage, sizeof(*args));
    memset(args, 0, sizeof(*args));
    list_init(args);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_realTypeArg();
    list_append_link(args, link);
    while (token->klass == TK_COMMA) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_realTypeArg();
      list_append_link(args, link);
    }
  }
  return args;
}

internal struct Ast*
build_expressionPrimary()
{
  struct Ast* primary = 0;
  if (token_is_expression(token)) {
    if (token->klass == TK_INTEGER) {
      primary = build_integer();
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      primary = build_boolean();
    } else if (token->klass == TK_STRING_LITERAL) {
      primary = build_stringLiteral();
    } else if (token->klass == TK_DOT_PREFIX) {
      next_token();
      if (token->klass == TK_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_nonTypeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_typeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTypeName(token)) {
      primary = build_nonTypeName(false);
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      struct Ast_ExprListExpr* expr_list = new_ast_node(struct Ast_ExprListExpr, AST_EXPRLIST_EXPR, token->line_nr);
      expr_list->expr_list = build_expressionList();
      primary = (struct Ast*)expr_list;
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        struct Ast_CastExpr* cast_expr = new_ast_node(struct Ast_CastExpr, AST_CAST_EXPR, token->line_nr);
        cast_expr->to_type = build_typeRef();
        primary = (struct Ast*)cast_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->expr = build_expression(1);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token_is_expression(token)) {
        primary = build_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR, token->line_nr);
      unary_expr->op = OP_NOT;
      enum AstExprOperator* op = arena_push(ast_storage, sizeof(*op));
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == TK_TILDA) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR, token->line_nr);
      unary_expr->op = OP_BITWISE_NOT;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR, token->line_nr);
      unary_expr->op = OP_MINUS;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token_is_typeName(token)) {
      primary = build_typeName();
    } else if (token->klass == TK_ERROR) {
      struct Ast_Name* name = new_ast_node(struct Ast_Name, AST_NAME, token->line_nr);
      name->strname = token->lexeme;
      primary = (struct Ast*)name;
      next_token();
    } else assert(0);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return primary;
}

internal int
get_operator_priority(struct Token* token)
{
  int prio = 0;
  if (token->klass == TK_DOUBLE_AMPERSAND || token->klass == TK_DOUBLE_PIPE) {
    prio = 1;
  } else if (token->klass == TK_DOUBLE_EQUAL || token->klass == TK_EXCLAMATION_EQUAL
      || token->klass == TK_ANGLE_OPEN /* Less */ || token->klass == TK_ANGLE_CLOSE /* Greater */
      || token->klass == TK_ANGLE_OPEN_EQUAL /* LessEqual */ || token->klass == TK_ANGLE_CLOSE_EQUAL /* GreaterEqual */) {
    prio = 2;
  }
  else if (token->klass == TK_PLUS || token->klass == TK_MINUS
           || token->klass == TK_AMPERSAND || token->klass == TK_PIPE
           || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN /* BitshiftLeft */
           || token->klass == TK_DOUBLE_ANGLE_CLOSE /* BitshiftRight */) {
    prio = 3;
  }
  else if (token->klass == TK_STAR || token->klass == TK_SLASH) {
    prio = 4;
  }
  else if (token->klass == TK_TRIPLE_AMPERSAND) {
    prio = 5;
  }
  else assert(0);
  return prio;
}

internal enum AstExprOperator
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
    default: return OP_NONE;
  }
}

internal struct Ast*
build_expression(int priority_threshold)
{
  struct Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOT_PREFIX) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = new_ast_node(struct Ast_MemberSelectExpr, AST_MEMBERSELECT_EXPR, token->line_nr);
        select_expr->expr = expr;
        expr = (struct Ast*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        struct Ast_IndexedArrayExpr* index_expr = new_ast_node(struct Ast_IndexedArrayExpr, AST_INDEXEDARRAY_EXPR, token->line_nr);
        index_expr->index = expr;
        index_expr->colon_index = build_arrayIndex();
        expr = (struct Ast*)index_expr;
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        struct Ast_FunctionCallExpr* call_expr = new_ast_node(struct Ast_FunctionCallExpr, AST_FUNCTIONCALL_EXPR, token->line_nr);
        call_expr->callee_expr = expr;
        call_expr->args = build_argumentList();
        expr = (struct Ast*)call_expr;
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        ((struct Ast_Expression*)expr)->type_args = build_realTypeArgumentList();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        struct Ast_KeyValuePairExpr* kv_pair = new_ast_node(struct Ast_KeyValuePairExpr, AST_KEYVALUE_PAIR_EXPR, token->line_nr);
        kv_pair->name = expr;
        kv_pair->expr = build_expression(1);
        expr = (struct Ast*)kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          struct Ast_BinaryExpr* bin_expr = new_ast_node(struct Ast_BinaryExpr, AST_BINARY_EXPR, token->line_nr);
          bin_expr->left_operand = expr;
          bin_expr->op = token_to_binop(token);
          next_token();
          bin_expr->right_operand = build_expression(priority + 1);
          expr = (struct Ast*)bin_expr;
        } else break;
      } else assert(0);
    }
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct NamedObject*
new_keyword(char* name, enum TokenClass token_klass)
{
  struct Object_Keyword* descriptor = arena_push(&local_storage, sizeof(*descriptor));
  memset(descriptor, 0, sizeof(*descriptor));
  descriptor->strname = name;
  descriptor->object_kind = OBJECT_KEYWORD;
  descriptor->token_klass = token_klass;
  return (struct NamedObject*)descriptor;
}

struct Ast*
build_ast_program(struct UnboundedArray* tokens_array_, struct Arena* ast_storage_)
{
  tokens_array = tokens_array_;
  ast_storage = ast_storage_;

  symtable_begin_build(&local_storage);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("action", TK_ACTION), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("actions", TK_ACTIONS), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("entries", TK_ENTRIES), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("enum", TK_ENUM), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("in", TK_IN), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("package", TK_PACKAGE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("select", TK_SELECT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("switch", TK_SWITCH), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("tuple", TK_TUPLE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("control", TK_CONTROL), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("error", TK_ERROR), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("header", TK_HEADER), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("inout", TK_INOUT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("parser", TK_PARSER), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("state", TK_STATE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("table", TK_TABLE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("key", TK_KEY), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("typedef", TK_TYPEDEF), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("type", TK_TYPE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("default", TK_DEFAULT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("extern", TK_EXTERN), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("header_union", TK_HEADER_UNION), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("out", TK_OUT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("transition", TK_TRANSITION), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("else", TK_ELSE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("exit", TK_EXIT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("if", TK_IF), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("match_kind", TK_MATCH_KIND), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("return", TK_RETURN), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("struct", TK_STRUCT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("apply", TK_APPLY), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("const", TK_CONST), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("bool", TK_BOOL), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("true", TK_TRUE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("false", TK_FALSE), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("void", TK_VOID), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("int", TK_INT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("bit", TK_BIT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("varbit", TK_VARBIT), 0);
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("string", TK_STRING), 0);

  token_at = 0;
  token = array_get(tokens_array, token_at);
  next_token();
  push_scope();
  struct Ast* p4program = build_p4program();
  pop_scope();
  symtable_end_build();
  arena_delete(&local_storage);
  return p4program;
}
