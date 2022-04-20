#include "arena.h"
#include "hashmap.h"
#include "token.h"
#include "lex.h"
#include "symtable.h"
#include "build_ast.h"
#include <memory.h>  // memset


internal struct Arena *m_ast_storage;
internal struct UnboundedArray* m_tokens_array;
internal int m_token_at = 0;
internal struct Token* m_token = 0;
internal int m_prev_token_at = 0;
internal struct Token* m_prev_token = 0;
internal int m_node_id = 100; // first 100 are reserved for built-in nodes

internal struct Ast* build_ast_expression(int priority_threshold);
internal struct Ast* build_ast_typeRef();
internal struct Ast* build_ast_blockStatement();
internal struct Ast* build_ast_statement(struct Ast* type_name);
internal struct Ast* build_ast_parserStatement();


#define new_ast_node(ast_type, ast_kind) ({ \
  ast_type* ast = arena_push(m_ast_storage, sizeof(ast_type)); \
  memset(ast, 0, sizeof(ast_type)); \
  ast->kind = ast_kind; \
  ast->id = m_node_id++; \
  ast; \
})

internal struct Token*
next_token()
{
  assert (m_token_at < m_tokens_array->elem_count);
  m_prev_token = m_token;
  m_prev_token_at = m_token_at;
  m_token = array_get(m_tokens_array, ++m_token_at);
  while (m_token->klass == TK_COMMENT) {
    m_token = array_get(m_tokens_array, ++m_token_at);
  }
  if (m_token->klass == TK_IDENTIFIER) {
    struct NameEntry* entry = scope_lookup_name(get_current_scope(), m_token->lexeme);
    if (entry->ns_keyword) {
      m_token->klass = entry->ns_keyword->token_class;
      return m_token;
    }
    if (entry->ns_type) {
      m_token->klass = TK_TYPE_IDENTIFIER;
      return m_token;
    }
  }
  return m_token;
}

internal struct Token*
peek_token()
{
  m_prev_token = m_token;
  m_prev_token_at = m_token_at;
  struct Token* peek_token = next_token();
  m_token = m_prev_token;
  m_token_at = m_prev_token_at;
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
  bool result = token->klass == TK_INT_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
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

internal bool
token_is_methodPrototype(struct Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

internal struct Ast*
build_ast_nonTypeName(bool is_type)
{
  struct Ast_Name* name = 0;
  if (token_is_nonTypeName(m_token)) {
    name = new_ast_node(struct Ast_Name, AST_NAME);
    name->line_no = m_token->line_no;
    name->strname = m_token->lexeme;
    if (is_type) {
      struct NameDecl* decl = arena_push_struct(m_ast_storage, struct NameDecl);
      decl->strname = name->strname;
      decl->line_no = m_token->line_no;
      declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
    }
    next_token();
  } else error("at line %d: non-type name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_ast_name(bool is_type)
{
  struct Ast_Name* name = 0;
  if (token_is_name(m_token)) {
    if (token_is_nonTypeName(m_token)) {
      name = (struct Ast_Name*)build_ast_nonTypeName(is_type);
    } else if (m_token->klass == TK_TYPE_IDENTIFIER) {
      struct Ast_Name* type_name = new_ast_node(struct Ast_Name, AST_NAME);
      type_name->line_no = m_token->line_no;
      type_name->strname = m_token->lexeme;
      name = type_name;
      next_token();
    } else assert(0);
  } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)name;
}

internal struct List*
build_ast_typeParameterList()
{
  struct List* params = 0;
  if (token_is_typeParameterList(m_token)) {
    params = arena_push_struct(m_ast_storage, struct List);
    list_init(params);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_name(true);
    list_append_link(params, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_name(true);
      list_append_link(params, li);
    }
  } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return params;
}

internal struct List*
build_ast_optTypeParameters()
{
  struct List* params = 0;
  if (m_token->klass == TK_ANGLE_OPEN) {
    next_token();
    if (token_is_typeParameterList(m_token)) {
      params = build_ast_typeParameterList();
      if (m_token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  }
  return params;
}

internal struct Ast*
build_ast_typeArg()
{
  struct Ast* arg = 0;
  if (token_is_typeArg(m_token))
  {
    if (m_token->klass == TK_DONTCARE) {
      struct Ast_Dontcare* dontcare = new_ast_node(struct Ast_Dontcare, AST_DONTCARE);
      dontcare->line_no = m_token->line_no;
      arg = (struct Ast*)dontcare;
      next_token();
    } else if (token_is_typeRef(m_token)) {
      arg = build_ast_typeRef();
    } else if (token_is_nonTypeName(m_token)) {
      arg = build_ast_nonTypeName(false);
    } else assert(0);
  } else error("at line %d: type argument was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return arg;
}

internal enum AstParamDirection
build_ast_direction()
{
  enum AstParamDirection dir = 0;
  if (token_is_direction(m_token)) {
    if (m_token->klass == TK_IN) {
      dir = PARAMDIR_IN;
    } else if (m_token->klass == TK_OUT) {
      dir = PARAMDIR_OUT;
    } else if (m_token->klass == TK_INOUT) {
      dir = PARAMDIR_INOUT;
    } else assert(0);
    next_token();
  }
  return dir;
}

internal struct Ast*
build_ast_parameter()
{
  struct Ast_Param* param = new_ast_node(struct Ast_Param, AST_PARAM);
  param->line_no = m_token->line_no;
  param->direction = build_ast_direction();
  if (token_is_typeRef(m_token)) {
    param->type = build_ast_typeRef();
    if (token_is_name(m_token)) {
      param->name = build_ast_name(false);
      if (m_token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(m_token)) {
          param->init_expr = build_ast_expression(1);
        } else error("at line %d: expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)param;
}

internal struct List*
build_ast_parameterList()
{
  struct List* params = 0;
  if (token_is_parameter(m_token)) {
    params = arena_push_struct(m_ast_storage, struct List);
    list_init(params);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_parameter();
    list_append_link(params, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_parameter();
      list_append_link(params, li);
    }
  }
  return params;
}

internal struct Ast*
build_ast_typeOrVoid(bool is_type)
{
  struct Ast* type = 0;
  if (token_is_typeOrVoid(m_token)) {
    if (token_is_typeRef(m_token)) {
      type = (struct Ast*)build_ast_typeRef();
    } else if (m_token->klass == TK_VOID) {
      struct Ast_Name* void_name = new_ast_node(struct Ast_Name, AST_NAME);
      void_name->line_no = m_token->line_no;
      void_name->strname = m_token->lexeme;
      type = (struct Ast*)void_name;
      next_token();
    } else if (m_token->klass == TK_IDENTIFIER) {
      struct Ast_Name* name = new_ast_node(struct Ast_Name, AST_NAME);
      name->line_no = m_token->line_no;
      name->strname = m_token->lexeme;
      type = (struct Ast*)name;
      if (is_type) {
        struct NameDecl* decl = arena_push_struct(m_ast_storage, struct NameDecl);
        decl->strname = name->strname;
        decl->line_no = m_token->line_no;
        declare_object_in_scope(get_current_scope(), NAMESPACE_TYPE, decl);
      }
      next_token();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_ast_functionPrototype(struct Ast* return_type)
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_typeOrVoid(m_token) || return_type) {
    proto = new_ast_node(struct Ast_FunctionProto, AST_FUNCTION_PROTO);
    proto->line_no = m_token->line_no;
    if (return_type) {
      proto->return_type = return_type;
    } else {
      proto->return_type = build_ast_typeOrVoid(true);
    }
    if (token_is_name(m_token)) {
      proto->name = build_ast_name(false);
      proto->type_params = build_ast_optTypeParameters();
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: function name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)proto;
}

internal struct Ast*
build_ast_methodPrototype()
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_methodPrototype(m_token)) {
    if (m_token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      proto = new_ast_node(struct Ast_FunctionProto, AST_FUNCTION_PROTO);
      proto->line_no = m_token->line_no;
      proto->name = build_ast_name(false);
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (token_is_typeOrVoid(m_token)) {
      proto = (struct Ast_FunctionProto*)build_ast_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)proto;
}

internal struct List*
build_ast_methodPrototypes()
{
  struct List* protos = 0;
  if (token_is_methodPrototype(m_token)) {
    protos = arena_push_struct(m_ast_storage, struct List);
    list_init(protos);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_methodPrototype();
    list_append_link(protos, li);
    while (token_is_methodPrototype(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_methodPrototype();
      list_append_link(protos, li);
    }
  }
  return protos;
}

internal struct Ast*
build_ast_externDeclaration()
{
  struct Ast* decl = 0;
  if (m_token->klass == TK_EXTERN) {
    next_token();
    bool is_function_proto = false;
    if (token_is_typeOrVoid(m_token) && token_is_nonTypeName(m_token)) {
      is_function_proto = token_is_typeOrVoid(m_token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(m_token)) {
      is_function_proto = true;
    } else if (token_is_nonTypeName(m_token)) {
      is_function_proto = false;
    } else error("at line %d: extern declaration was expected, got `%s`.", m_token->line_no, m_token->lexeme);

    if (is_function_proto) {
      struct Ast_FunctionProto* proto = (struct Ast_FunctionProto*)build_ast_functionPrototype(0);
      decl = (struct Ast*)proto;
      proto->is_extern = true;
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else {
      struct Ast_ExternDecl* extern_decl = new_ast_node(struct Ast_ExternDecl, AST_EXTERN_DECL);
      extern_decl->line_no = m_token->line_no;
      decl = (struct Ast*)extern_decl;
      extern_decl->name = build_ast_nonTypeName(true);
      extern_decl->type_params = build_ast_optTypeParameters();
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_decl->method_protos = build_ast_methodPrototypes();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  }
  return decl;
}

internal struct Ast*
build_ast_integer()
{
  struct Ast_IntLiteral* int_node = 0;
  if (m_token->klass == TK_INT_LITERAL) {
    int_node = new_ast_node(struct Ast_IntLiteral, AST_INT_LITERAL);
    int_node->line_no = m_token->line_no;
    int_node->flags = m_token->i.flags;
    int_node->width = m_token->i.width;
    int_node->value = m_token->i.value;
    next_token();
  }
  return (struct Ast*)int_node;
}

internal struct Ast*
build_ast_boolean()
{
  struct Ast_BoolLiteral* bool_node = 0;
  if (m_token->klass == TK_TRUE || m_token->klass == TK_FALSE) {
    bool_node = new_ast_node(struct Ast_BoolLiteral, AST_BOOL_LITERAL);
    bool_node->line_no = m_token->line_no;
    bool_node->value = (m_token->klass == TK_TRUE);
    next_token();
  }
  return (struct Ast*)bool_node;
}

internal struct Ast*
build_ast_stringLiteral()
{
  struct Ast_StringLiteral* string = 0;
  if (m_token->klass == TK_STRING_LITERAL) {
    string = new_ast_node(struct Ast_StringLiteral, AST_STRING_LITERAL);
    string->line_no = m_token->line_no;
    string->value = m_token->lexeme;
    next_token();
  }
  return (struct Ast*)string;
}

internal struct Ast*
build_ast_integerTypeSize()
{
  struct Ast_IntTypeSize* type_size = new_ast_node(struct Ast_IntTypeSize, AST_INT_TYPESIZE);
  type_size->line_no = m_token->line_no;
  if (m_token->klass == TK_INT_LITERAL) {
    type_size->size = build_ast_integer();
  } else if (m_token->klass == TK_PARENTH_OPEN) {
    /* FIXME
    type_size->size = build_ast_expression(1); */
    error("at line %d: integer was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)type_size;
}

internal struct Ast*
build_ast_baseType()
{
  struct Ast* base_type = 0;
  if (token_is_baseType(m_token)) {
    struct Ast_Name* type_name = new_ast_node(struct Ast_Name, AST_NAME);
    type_name->line_no = m_token->line_no;
    if (m_token->klass == TK_BOOL) {
      struct Ast_BaseType_Bool* bool_type = new_ast_node(struct Ast_BaseType_Bool, AST_BASETYPE_BOOL);
      bool_type->line_no = m_token->line_no;
      type_name->strname = "bool";
      bool_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bool_type;
      next_token();
    } else if (m_token->klass == TK_ERROR) {
      struct Ast_BaseType_Error* error_type = new_ast_node(struct Ast_BaseType_Error, AST_BASETYPE_ERROR);
      error_type->line_no = m_token->line_no;
      type_name->strname = "error";
      error_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)error_type;
      next_token();
    } else if (m_token->klass == TK_INT) {
      struct Ast_BaseType_Int* int_type = new_ast_node(struct Ast_BaseType_Int, AST_BASETYPE_INT);
      type_name->line_no = m_token->line_no;
      type_name->strname = "int";
      int_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)int_type;
      next_token();
      if (m_token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = build_ast_integerTypeSize();
        if (m_token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
    } else if (m_token->klass == TK_BIT) {
      struct Ast_BaseType_Bit* bit_type = new_ast_node(struct Ast_BaseType_Bit, AST_BASETYPE_BIT);
      type_name->line_no = m_token->line_no;
      type_name->strname = "bit";
      bit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)bit_type;
      next_token();
      if (m_token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = build_ast_integerTypeSize();
        if (m_token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
    } else if (m_token->klass == TK_VARBIT) {
      struct Ast_BaseType_Varbit* varbit_type = new_ast_node(struct Ast_BaseType_Varbit, AST_BASETYPE_VARBIT);
      type_name->line_no = m_token->line_no;
      type_name->strname = "varbit";
      varbit_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)varbit_type;
      next_token();
      if (m_token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = build_ast_integerTypeSize();
        if (m_token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
    } else if (m_token->klass == TK_STRING) {
      struct Ast_BaseType_String* string_type = new_ast_node(struct Ast_BaseType_String, AST_BASETYPE_STRING);
      string_type->line_no = m_token->line_no;
      type_name->strname = "string";
      string_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)string_type;
      next_token();
    } else if (m_token->klass == TK_VOID) {
      struct Ast_BaseType_Void* void_type = new_ast_node(struct Ast_BaseType_Void, AST_BASETYPE_VOID);
      void_type->line_no = m_token->line_no;
      type_name->strname = "void";
      void_type->name = (struct Ast*)type_name;
      base_type = (struct Ast*)void_type;
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return base_type;
}

internal struct List*
build_ast_typeArgumentList()
{
  struct List* args = 0;
  if (token_is_typeArg(m_token)) {
    args = arena_push_struct(m_ast_storage, struct List);
    list_init(args);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_typeArg();
    list_append_link(args, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_typeArg();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_ast_tupleType()
{
  struct Ast_Tuple* tuple = 0;
  if (m_token->klass == TK_TUPLE) {
    next_token();
    tuple = new_ast_node(struct Ast_Tuple, AST_TUPLE);
    tuple->line_no = m_token->line_no;
    if (m_token->klass == TK_ANGLE_OPEN) {
      next_token();
      tuple->type_args = build_ast_typeArgumentList();
      if (m_token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `<` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `tuple` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)tuple;
}

internal struct Ast*
build_ast_headerStackType()
{
  struct Ast_HeaderStack* stack = 0;
  if (m_token->klass == TK_BRACKET_OPEN) {
    next_token();
    stack = new_ast_node(struct Ast_HeaderStack, AST_HEADER_STACK);
    stack->line_no = m_token->line_no;
    if (token_is_expression(m_token)) {
      stack->stack_expr = build_ast_expression(1);
      if (m_token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("at line %d: `]` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: an expression expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `[` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)stack;
}

internal struct Ast*
build_ast_specializedType()
{
  struct Ast_SpecializedType* type = 0;
  if (m_token->klass == TK_ANGLE_OPEN) {
    next_token();
    type = new_ast_node(struct Ast_SpecializedType, AST_SPECIALIZED_TYPE);
    type->line_no = m_token->line_no;
    type->type_args = build_ast_typeArgumentList();
    if (m_token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `<` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_ast_prefixedType()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (m_token->klass == TK_DOT_PREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (m_token->klass == TK_TYPE_IDENTIFIER) {
    name = new_ast_node(struct Ast_Name, AST_NAME);
    name->line_no = m_token->line_no;
    name->strname = m_token->lexeme;
    name->is_dotprefixed = is_dotprefixed;
    next_token();
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_ast_typeName()
{
  struct Ast* name = 0;
  if (token_is_typeName(m_token)) {
    name = build_ast_prefixedType();
    if (m_token->klass == TK_ANGLE_OPEN) {
      struct Ast* speclzd_type = build_ast_specializedType();
      assert (speclzd_type->kind == AST_SPECIALIZED_TYPE);
      ((struct Ast_SpecializedType*)speclzd_type)->name = name;
      name = speclzd_type;
    } if (m_token->klass == TK_BRACKET_OPEN) {
      struct Ast* stack_type = build_ast_headerStackType();
      assert (stack_type->kind == AST_HEADER_STACK);
      ((struct Ast_HeaderStack*)stack_type)->name = name;
      name = stack_type;
    }
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return name;
}

internal struct Ast*
build_ast_typeRef()
{
  struct Ast* ref = 0;
  if (token_is_typeRef(m_token)) {
    if (token_is_baseType(m_token)) {
      ref = build_ast_baseType();
    } else if (token_is_typeName(m_token)) {
      /* <typeName> | <specializedType> | <headerStackType> */
      ref = build_ast_typeName();
    } else if (m_token->klass == TK_TUPLE) {
      ref = build_ast_tupleType();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return ref;
}

internal bool
token_is_structField(struct Token* m_token)
{
  bool result = token_is_typeRef(m_token);
  return result;
}

internal struct Ast*
build_ast_structField()
{
  struct Ast_StructField* field = new_ast_node(struct Ast_StructField, AST_STRUCT_FIELD);
  field->line_no = m_token->line_no;
  if (token_is_typeRef(m_token)) {
    field->type = build_ast_typeRef();
    if (token_is_name(m_token)) {
      field->name = build_ast_name(false);
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: struct field was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)field;
}

internal struct List*
build_ast_structFieldList()
{
  struct List* fields = 0;
  if (token_is_structField(m_token)) {
    fields = arena_push_struct(m_ast_storage, struct List);
    list_init(fields);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_structField();
    list_append_link(fields, li);
    while (token_is_structField(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_structField();
      list_append_link(fields, li);
    }
  }
  return fields;
}

internal struct Ast*
build_ast_headerTypeDeclaration()
{
  struct Ast_HeaderDecl* decl = 0;
  if (m_token->klass == TK_HEADER) {
    next_token();
    decl = new_ast_node(struct Ast_HeaderDecl, AST_HEADER_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_ast_structFieldList();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token(m_token);
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `header` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_headerUnionDeclaration()
{
  struct Ast_HeaderUnionDecl* decl = 0;
  if (m_token->klass == TK_HEADER_UNION) {
    next_token();
    decl = new_ast_node(struct Ast_HeaderUnionDecl, AST_HEADER_UNION_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_ast_structFieldList();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `header_union` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_structTypeDeclaration()
{
  struct Ast_StructDecl* decl = 0;
  if (m_token->klass == TK_STRUCT) {
    next_token();
    decl = new_ast_node(struct Ast_StructDecl, AST_STRUCT_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->fields = build_ast_structFieldList();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `struct` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal bool
token_is_specifiedIdentifier(struct Token* m_token)
{
  return token_is_name(m_token);
}

internal struct Ast*
build_ast_initializer()
{
  return build_ast_expression(1);
}

internal struct Ast*
build_ast_optInitializer()
{
  struct Ast* init = 0;
  if (m_token->klass == TK_EQUAL) {
    next_token();
    init = build_ast_initializer();
  }
  return init;
}

internal struct Ast*
build_ast_specifiedIdentifier()
{
  struct Ast_SpecifiedIdent* id = 0;
  if (token_is_specifiedIdentifier(m_token)) {
    id = new_ast_node(struct Ast_SpecifiedIdent, AST_SPECIFIED_IDENT);
    id->line_no = m_token->line_no;
    id->name = build_ast_name(false);
    if (m_token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(m_token)) {
        id->init_expr = build_ast_initializer();
      } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)id;
}

internal struct List*
build_ast_specifiedIdentifierList()
{
  struct List* ids = 0;
  if (token_is_specifiedIdentifier(m_token)) {
    ids = arena_push_struct(m_ast_storage, struct List);
    list_init(ids);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_specifiedIdentifier();
    list_append_link(ids, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_specifiedIdentifier();
      list_append_link(ids, li);
    }
  }
  return ids;
}

internal struct Ast*
build_ast_enumDeclaration()
{
  struct Ast_EnumDecl* decl = 0;
  if (m_token->klass == TK_ENUM) {
    next_token();
    decl = new_ast_node(struct Ast_EnumDecl, AST_ENUM_DECL);
    decl->line_no = m_token->line_no;
    if (m_token->klass == TK_BIT) {
      next_token();
      if (m_token->klass == TK_ANGLE_OPEN) {
        next_token();
        if (m_token->klass == TK_INT_LITERAL) {
          decl->type_size = build_ast_integer();
          if (m_token->klass == TK_ANGLE_CLOSE) {
            next_token();
          } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: an integer was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `<` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(m_token)) {
          decl->id_list = build_ast_specifiedIdentifierList();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `enum` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_derivedTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token_is_derivedTypeDeclaration(m_token)) {
    if (m_token->klass == TK_HEADER) {
      decl = build_ast_headerTypeDeclaration();
    } else if (m_token->klass == TK_HEADER_UNION) {
      decl = build_ast_headerUnionDeclaration();
    } else if (m_token->klass == TK_STRUCT) {
      decl = build_ast_structTypeDeclaration();
    } else if (m_token->klass == TK_ENUM) {
      decl = build_ast_enumDeclaration();
    } else assert(0);
  } else error("at line %d: structure declaration was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return decl;
}

internal struct Ast*
build_ast_parserTypeDeclaration()
{
  struct Ast_ParserProto* type = 0;
  if (m_token->klass == TK_PARSER) {
    next_token();
    type = new_ast_node(struct Ast_ParserProto, AST_PARSER_PROTO);
    type->line_no = m_token->line_no; 
    if (token_is_name(m_token)) {
      type->name = build_ast_name(true);
      type->type_params = build_ast_optTypeParameters();
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        type->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `parser` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)type;
}

internal struct List*
build_ast_optConstructorParameters()
{
  struct List* ctor_params = 0;
  if (m_token->klass == TK_PARENTH_OPEN) {
    next_token();
    ctor_params = build_ast_parameterList();
    if (m_token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  }
  return ctor_params;
}

internal struct Ast*
build_ast_constantDeclaration()
{
  struct Ast_ConstDecl* decl = 0;
  if (m_token->klass == TK_CONST) {
    next_token();
    decl = new_ast_node(struct Ast_ConstDecl, AST_CONST_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_typeRef(m_token)) {
      decl->type_ref = build_ast_typeRef();
      if (token_is_name(m_token)) {
        decl->name = build_ast_name(false);
        if (m_token->klass == TK_EQUAL) {
          next_token();
          if (token_is_expression(m_token)) {
            decl->expr = build_ast_expression(1);
            if (m_token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
          } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `const` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal bool
token_is_declaration(struct Token* m_token)
{
  bool result = m_token->klass == TK_CONST || m_token->klass == TK_EXTERN || m_token->klass == TK_ACTION
    || m_token->klass == TK_PARSER || token_is_typeDeclaration(m_token) || m_token->klass == TK_CONTROL
    || token_is_typeRef(m_token) || m_token->klass == TK_ERROR || m_token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(m_token) || m_token->klass == TK_DOT_PREFIX;
  return result;
}

internal bool
token_is_lvalue(struct Token* m_token)
{
  bool result = token_is_nonTypeName(m_token) | m_token->klass == TK_DOT_PREFIX;
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(struct Token* m_token)
{
  bool result = token_is_lvalue(m_token) || m_token->klass == TK_PARENTH_OPEN || m_token->klass == TK_ANGLE_OPEN
    || m_token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_statement(struct Token* m_token)
{
  bool result = token_is_assignmentOrMethodCallStatement(m_token) || token_is_typeName(m_token) || m_token->klass == TK_IF
    || m_token->klass == TK_SEMICOLON || m_token->klass == TK_BRACE_OPEN || m_token->klass == TK_EXIT
    || m_token->klass == TK_RETURN || m_token->klass == TK_SWITCH;
  return result;
}

internal bool
token_is_statementOrDeclaration(struct Token* m_token)
{
  bool result = token_is_typeRef(m_token) || m_token->klass == TK_CONST || token_is_statement(m_token);
  return result;
}

internal bool
token_is_argument(struct Token* m_token)
{
  bool result = token_is_expression(m_token) || token_is_name(m_token) || m_token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_parserLocalElement(struct Token* m_token)
{
  bool result = m_token->klass == TK_CONST || token_is_typeRef(m_token);
  return result;
}

internal bool
token_is_parserStatement(struct Token* m_token)
{
  bool result = token_is_assignmentOrMethodCallStatement(m_token) || token_is_typeName(m_token)
    || m_token->klass == TK_BRACE_OPEN || m_token->klass == TK_CONST || token_is_typeRef(m_token)
    || m_token->klass == TK_SEMICOLON;
  return result;
}

internal bool
token_is_simpleKeysetExpression(struct Token* m_token) {
  bool result = token_is_expression(m_token) || m_token->klass == TK_DEFAULT || m_token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_keysetExpression(struct Token* m_token)
{
  bool result = m_token->klass == TK_TUPLE || token_is_simpleKeysetExpression(m_token);
  return result;
}

internal bool
token_is_selectCase(struct Token* m_token)
{
  return token_is_keysetExpression(m_token);
}

internal bool
token_is_controlLocalDeclaration(struct Token* m_token)
{
  bool result = m_token->klass == TK_CONST || m_token->klass == TK_ACTION
    || m_token->klass == TK_TABLE || token_is_typeRef(m_token) || token_is_typeRef(m_token);
  return result;
}

internal struct Ast*
build_ast_argument()
{
  struct Ast* arg = 0;
  if (token_is_argument(m_token)) {
    if (token_is_expression(m_token)) {
      arg = build_ast_expression(1);
    } else if (token_is_name(m_token)) {
      struct Ast_Argument* name_arg = new_ast_node(struct Ast_Argument, AST_ARGUMENT);
      name_arg->line_no = m_token->line_no;
      arg = (struct Ast*)name_arg;
      name_arg->name = build_ast_name(false);
      if (m_token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(m_token)) {
          name_arg->init_expr = build_ast_expression(1);
        } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_DONTCARE) {
      struct Ast_Dontcare* dontcare_arg = new_ast_node(struct Ast_Dontcare, AST_DONTCARE);
      dontcare_arg->line_no = m_token->line_no;
      arg = (struct Ast*)dontcare_arg;
      next_token();
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return arg;
}

internal struct List*
build_ast_argumentList()
{
  struct List* args = 0;
  if (token_is_argument(m_token)) {
    args = arena_push_struct(m_ast_storage, struct List);
    list_init(args);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_argument();
    list_append_link(args, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_argument();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_ast_variableDeclaration(struct Ast* type_ref)
{
  struct Ast_VarDecl* decl = 0;
  if (token_is_typeRef(m_token) || type_ref) {
    decl = new_ast_node(struct Ast_VarDecl, AST_VAR_DECL);
    decl->line_no = m_token->line_no;
    decl->type = type_ref ? type_ref : build_ast_typeRef();
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(false);
      decl->init_expr = build_ast_optInitializer();
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_instantiation(struct Ast* type_ref)
{
  struct Ast_Instantiation* inst = 0;
  if (token_is_typeRef(m_token) || type_ref) {
    inst = new_ast_node(struct Ast_Instantiation, AST_INSTANTIATION);
    inst->line_no = m_token->line_no;
    inst->type_ref = type_ref ? type_ref : build_ast_typeRef();
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      inst->args = build_ast_argumentList();
      if (m_token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(m_token)) {
          inst->name = build_ast_name(false);
          if (m_token->klass == TK_SEMICOLON) {
            next_token();
          } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: instance name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)inst;
}

internal struct Ast*
build_ast_parserLocalElement()
{
  struct Ast* elem = 0;
  if (token_is_parserLocalElement(m_token)) {
    if (m_token->klass == TK_CONST) {
      elem = build_ast_constantDeclaration();
    } else if (token_is_typeRef(m_token)) {
      struct Ast* type_ref = build_ast_typeRef();
      if (m_token->klass == TK_PARENTH_OPEN) {
        elem = build_ast_instantiation(type_ref);
      } else if (token_is_name(m_token)) {
        elem = build_ast_variableDeclaration(type_ref);
      } else error("at line %d: unexpected token `%s`.", m_token->line_no, m_token->lexeme);
    } else assert(0);
  } else error("at line %d: local declaration was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return elem;
}

internal struct List*
build_ast_parserLocalElements()
{
  struct List* elems = 0;
  if (token_is_parserLocalElement(m_token)) {
    elems = arena_push_struct(m_ast_storage, struct List);
    list_init(elems);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_parserLocalElement();
    list_append_link(elems, li);
    while (token_is_parserLocalElement(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_parserLocalElement();
      list_append_link(elems, li);
    }
  }
  return elems;
}

internal struct Ast*
build_ast_directApplication(struct Ast* type_name)
{
  struct Ast_FunctionCallExpr* apply_expr = 0;
  if (token_is_typeName(m_token) || type_name) {
    apply_expr = new_ast_node(struct Ast_FunctionCallExpr, AST_FUNCTION_CALL_EXPR);
    apply_expr->line_no = m_token->line_no;
    struct Ast_MemberSelectExpr* apply_select = new_ast_node(struct Ast_MemberSelectExpr, AST_MEMBER_SELECT_EXPR);
    apply_select->line_no = m_token->line_no;
    apply_select->lhs_expr = type_name ? type_name : build_ast_typeName();
    struct Ast_Name* apply_name = new_ast_node(struct Ast_Name, AST_NAME);
    apply_name->line_no = m_token->line_no;
    apply_name->strname = "apply";
    apply_select->member_name = (struct Ast*)apply_name;
    apply_expr->callee_expr = (struct Ast*)apply_select;
    if (m_token->klass == TK_DOT_PREFIX) {
      next_token();
      if (m_token->klass == TK_APPLY) {
        next_token();
        if (m_token->klass == TK_PARENTH_OPEN) {
          next_token();
          apply_expr->args = build_ast_argumentList();
          if (m_token->klass == TK_PARENTH_CLOSE) {
            next_token();
            if (m_token->klass == TK_SEMICOLON) {
              next_token();
            } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
          } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `apply` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `.` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)apply_expr;
}

internal struct Ast*
build_ast_prefixedNonTypeName()
{
  struct Ast_Name* name = 0;
  bool is_dotprefixed = false;
  if (m_token->klass == TK_DOT_PREFIX) {
    next_token();
    is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    name = (struct Ast_Name*)build_ast_nonTypeName(false);
    name->is_dotprefixed = is_dotprefixed;
  } else error("at line %d: non-type name was expected, ", m_token->line_no, m_token->lexeme);
  return (struct Ast*)name;
}

internal struct Ast*
build_ast_arraySubscript()
{
  struct Ast_SubscriptExpr* index = new_ast_node(struct Ast_SubscriptExpr, AST_SUBSCRIPT_EXPR);
  index->line_no = m_token->line_no;
  if (token_is_expression(m_token)) {
    index->index = build_ast_expression(1);
  } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  if (m_token->klass == TK_COLON) {
    next_token();
    if (token_is_expression(m_token)) {
      index->colon_index = build_ast_expression(1);
    } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  }
  return (struct Ast*)index;
}

internal struct Ast*
build_ast_lvalueExpr()
{
  struct Ast* expr = 0;
  if (m_token->klass == TK_DOT_PREFIX) {
    next_token();
    struct Ast_Name* dot_member = (struct Ast_Name*)build_ast_name(false);
    dot_member->is_dotprefixed = true;
    expr = (struct Ast*)dot_member;
  } else if (m_token->klass == TK_BRACKET_OPEN) {
    next_token();
    expr = build_ast_arraySubscript();
    if (m_token->klass == TK_BRACKET_CLOSE) {
      next_token();
    } else error("at line %d: `]` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return expr;
}

internal struct Ast*
build_ast_lvalue()
{
  struct Ast* lvalue = 0;
  if (token_is_lvalue(m_token)) {
    struct Ast* name = build_ast_prefixedNonTypeName();
    lvalue = name;
    while(m_token->klass == TK_DOT_PREFIX || m_token->klass == TK_BRACKET_OPEN) {
      if (m_token->klass == TK_DOT_PREFIX) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = new_ast_node(struct Ast_MemberSelectExpr, AST_MEMBER_SELECT_EXPR);
        select_expr->line_no = m_token->line_no;
        select_expr->lhs_expr = lvalue;
        lvalue = (struct Ast*)select_expr;
        if (token_is_name(m_token)) {
          select_expr->member_name = build_ast_name(false);
        } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
      else if (m_token->klass == TK_BRACKET_OPEN) {
        next_token();
        struct Ast_SubscriptExpr* subscript_expr = new_ast_node(struct Ast_SubscriptExpr, AST_SUBSCRIPT_EXPR);
        subscript_expr->line_no = m_token->line_no;
        subscript_expr->index = lvalue;
        subscript_expr->colon_index = build_ast_arraySubscript();
        lvalue = (struct Ast*)subscript_expr;
        if (m_token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
    }
  } else error("at line %d: lvalue was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)lvalue;
}

internal struct Ast*
build_ast_assignmentOrMethodCallStatement()
{
  struct Ast* stmt = 0;
  if (token_is_lvalue(m_token)) {
    struct Ast* lvalue = build_ast_lvalue();
    struct List* type_args = 0;
    stmt = lvalue;
    if (m_token->klass == TK_ANGLE_OPEN) {
      next_token();
      type_args = build_ast_typeArgumentList();
      if (m_token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      struct Ast_FunctionCallExpr* call_stmt = new_ast_node(struct Ast_FunctionCallExpr, AST_FUNCTION_CALL_EXPR);
      call_stmt->line_no = m_token->line_no;
      call_stmt->callee_expr = lvalue;
      call_stmt->type_args = type_args;
      call_stmt->args = build_ast_argumentList();
      stmt = (struct Ast*)call_stmt;
      if (m_token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_EQUAL) {
      next_token();
      struct Ast_AssignmentStmt* assgn_stmt = new_ast_node(struct Ast_AssignmentStmt, AST_ASSIGNMENT_STMT);
      assgn_stmt->line_no = m_token->line_no;
      assgn_stmt->lvalue = lvalue;
      assgn_stmt->expr = build_ast_expression(1);
      stmt = (struct Ast*)assgn_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return stmt;
}

internal struct List*
build_ast_parserStatements()
{
  struct List* stmts = 0;
  if (token_is_parserStatement(m_token)) {
    stmts = arena_push_struct(m_ast_storage, struct List);
    memset(stmts, 0, sizeof(*stmts));
    list_init(stmts);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_parserStatement();
    list_append_link(stmts, li);
    while (token_is_parserStatement(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_parserStatement();
      list_append_link(stmts, li);
    }
  }
  return stmts;
}

internal struct Ast*
build_ast_parserBlockStatements()
{
  struct Ast_BlockStmt* stmt = 0;
  if (m_token->klass == TK_BRACE_OPEN) {
    stmt = new_ast_node(struct Ast_BlockStmt, AST_BLOCK_STMT);
    stmt->line_no = m_token->line_no;
    next_token();
    stmt->stmt_list = build_ast_parserStatements();
    if (m_token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)stmt;
}

internal struct Ast*
build_ast_parserStatement()
{
  struct Ast* stmt = 0;
  if (token_is_typeRef(m_token)) {
    struct Ast* type_ref = build_ast_typeRef();
    if (token_is_name(m_token)) {
      stmt = build_ast_variableDeclaration(type_ref);
    } else {
      stmt = build_ast_directApplication(type_ref);
    }
  } else if (token_is_assignmentOrMethodCallStatement(m_token)) {
    stmt = build_ast_assignmentOrMethodCallStatement();
  } else if (m_token->klass == TK_BRACE_OPEN) {
    stmt = build_ast_parserBlockStatements();
  } else if (m_token->klass == TK_CONST) {
    stmt = build_ast_constantDeclaration();
  } else if (m_token->klass == TK_SEMICOLON) {
    stmt = (struct Ast*)new_ast_node(struct Ast_EmptyStmt, AST_EMPTY_STMT);
    stmt->line_no = m_token->line_no;
  } else error("at line %d: statement was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return stmt;
}

internal struct List*
build_ast_expressionList()
{
  struct List* exprs = 0;
  if (token_is_expression(m_token)) {
    exprs = arena_push_struct(m_ast_storage, struct List);
    list_init(exprs);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_expression(1);
    list_append_link(exprs, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_expression(1);
      list_append_link(exprs, li);
    }
  }
  return exprs;
}

internal struct Ast*
build_ast_simpleKeysetExpression()
{
  struct Ast* expr = 0;
  if (token_is_expression(m_token)) {
    expr = build_ast_expression(1);
  } else if (m_token->klass == TK_DEFAULT) {
    next_token();
    expr = (struct Ast*)new_ast_node(struct Ast_DefaultStmt, AST_DEFAULT_STMT);
    expr->line_no = m_token->line_no;
  } else if (m_token->klass == TK_DONTCARE) {
    next_token();
    expr = (struct Ast*)new_ast_node(struct Ast_Dontcare, AST_DONTCARE);
    expr->line_no = m_token->line_no;
  } else error("at line %d: keyset expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return expr;
}

internal struct Ast*
build_ast_tupleKeysetExpression()
{
  struct Ast_TupleKeyset* tuple_keyset = 0;
  if (m_token->klass == TK_PARENTH_OPEN) {
    tuple_keyset = new_ast_node(struct Ast_TupleKeyset, AST_TUPLE_KEYSET);
    tuple_keyset->line_no = m_token->line_no;
    next_token();
    struct List* exprs = arena_push_struct(m_ast_storage, struct List);
    list_init(exprs);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_simpleKeysetExpression();
    list_append_link(exprs, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_simpleKeysetExpression();
      list_append_link(exprs, li);
    }
    tuple_keyset->expr_list = exprs;
    if (m_token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)tuple_keyset;
}

internal struct Ast*
build_ast_keysetExpression()
{
  struct Ast* expr = 0;
  if (m_token->klass == TK_PARENTH_OPEN) {
    expr = build_ast_tupleKeysetExpression();
  } else if (token_is_simpleKeysetExpression(m_token)) {
    expr = build_ast_simpleKeysetExpression();
  } else error("at line %d: keyset expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return expr;
}

internal struct Ast*
build_ast_selectCase()
{
  struct Ast_SelectCase* select_case = 0;
  if (token_is_keysetExpression(m_token)) {
    select_case = new_ast_node(struct Ast_SelectCase, AST_SELECT_CASE);
    select_case->line_no = m_token->line_no;
    select_case->keyset = build_ast_keysetExpression();
    if (m_token->klass == TK_COLON) {
      next_token();
      if (token_is_name(m_token)) {
        select_case->name = build_ast_name(false);
        if (m_token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: keyset expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)select_case;
}

internal struct List*
build_ast_selectCaseList()
{
  struct List* cases = 0;
  if (token_is_selectCase(m_token)) {
    cases = arena_push_struct(m_ast_storage, struct List);
    list_init(cases);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_selectCase();
    list_append_link(cases, li);
    while (token_is_selectCase(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_selectCase();
      list_append_link(cases, li);
    }
  }
  return cases;
}

internal struct Ast*
build_ast_selectExpression()
{
  struct Ast_SelectExpr* select_expr = 0;
  if (m_token->klass == TK_SELECT) {
    next_token();
    select_expr = new_ast_node(struct Ast_SelectExpr, AST_SELECT_EXPR);
    select_expr->line_no = m_token->line_no;
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      select_expr->expr_list = build_ast_expressionList();
      if (m_token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (m_token->klass == TK_BRACE_OPEN) {
          next_token();
          select_expr->case_list = build_ast_selectCaseList();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `select` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)select_expr;
}

internal struct Ast*
build_ast_stateExpression()
{
  struct Ast* state_expr = 0;
  if (token_is_name(m_token)) {
    state_expr = build_ast_name(false);
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else if (m_token->klass == TK_SELECT) {
    state_expr = build_ast_selectExpression();
  } else error("at line %d: state expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return state_expr;
}

internal struct Ast*
build_ast_transitionStatement()
{
  struct Ast* stmt = 0;
  if (m_token->klass == TK_TRANSITION) {
    next_token();
    stmt = build_ast_stateExpression();
  } else error("at line %d: `transition` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return stmt;
}

internal struct Ast*
build_ast_parserState()
{
  struct Ast_ParserState* state = 0;
  if (m_token->klass == TK_STATE) {
    next_token();
    state = new_ast_node(struct Ast_ParserState, AST_PARSER_STATE);
    state->line_no = m_token->line_no;
    state->name = build_ast_name(false);
    if (m_token->klass == TK_BRACE_OPEN) {
      next_token();
      state->stmt_list = build_ast_parserStatements();
      state->trans_stmt = build_ast_transitionStatement();
      if (m_token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `state` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)state;
}

internal struct List*
build_ast_parserStates()
{
  struct List* states = 0;
  if (m_token->klass == TK_STATE) {
    states = arena_push_struct(m_ast_storage, struct List);
    list_init(states);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_parserState();
    list_append_link(states, li);
    while (m_token->klass == TK_STATE) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_parserState();
      list_append_link(states, li);
    }
  } else error("at line %d: `state` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return states;
}

internal struct Ast*
build_ast_parserDeclaration()
{
  struct Ast_ParserDecl* decl = 0;
  if (m_token->klass == TK_PARSER) {
    decl = new_ast_node(struct Ast_ParserDecl, AST_PARSER_DECL);
    decl->line_no = m_token->line_no;
    decl->type_decl = build_ast_parserTypeDeclaration();
    if (m_token->klass == TK_SEMICOLON) {
      next_token(); /* <parserTypeDeclaration> */
    } else {
      decl->ctor_params = build_ast_optConstructorParameters();
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->local_elements = build_ast_parserLocalElements();
        decl->states = build_ast_parserStates();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  } else error("at line %d: `parser` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_controlTypeDeclaration()
{
  struct Ast_ControlProto* decl = 0;
  if (m_token->klass == TK_CONTROL) {
    next_token();
    decl = new_ast_node(struct Ast_ControlProto, AST_CONTROL_PROTO);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      decl->type_params = build_ast_optTypeParameters();
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `control` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_actionDeclaration()
{
  struct Ast_ActionDecl* decl = 0;
  if (m_token->klass == TK_ACTION) {
    next_token();
    decl = new_ast_node(struct Ast_ActionDecl, AST_ACTION_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(false);
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (m_token->klass == TK_BRACE_OPEN) {
            decl->stmt = build_ast_blockStatement();
          } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `action` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_keyElement()
{
  struct Ast_KeyElement* key_elem = 0;
  if (token_is_expression(m_token)) {
    key_elem = new_ast_node(struct Ast_KeyElement, AST_KEY_ELEMENT);
    key_elem->line_no = m_token->line_no;
    key_elem->expr = build_ast_expression(1);
    if (m_token->klass == TK_COLON) {
      next_token();
      key_elem->name = build_ast_name(false);
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)key_elem;
}

internal struct List*
build_ast_keyElementList()
{
  struct List* elems = 0;
  if (token_is_expression(m_token)) {
    elems = arena_push_struct(m_ast_storage, struct List);
    list_init(elems);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_keyElement();
    list_append_link(elems, li);
    while (token_is_expression(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_keyElement();
      list_append_link(elems, li);
    }
  }
  return elems;
}

internal struct Ast*
build_ast_actionRef()
{
  struct Ast_ActionRef* ref = 0;
  if (m_token->klass == TK_DOT_PREFIX || token_is_nonTypeName(m_token)) {
    ref = new_ast_node(struct Ast_ActionRef, AST_ACTION_REF);
    ref->line_no = m_token->line_no;
    ref->name = build_ast_prefixedNonTypeName();
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      ref->args = build_ast_argumentList();
      if (m_token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  } else error("at line %d: non-type name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)ref;
}

internal struct List*
build_ast_actionList()
{
  struct List* actions = 0;
  if (token_is_actionRef(m_token)) {
    actions = arena_push_struct(m_ast_storage, struct List);
    list_init(actions);
    struct ListLink* li = arena_push_struct(m_ast_storage, struct ListLink);
    li->object = build_ast_actionRef();
    list_append_link(actions, li);
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    while (token_is_actionRef(m_token)) {
      li = arena_push_struct(m_ast_storage, struct ListLink);
      li->object = build_ast_actionRef();
      list_append_link(actions, li);
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  }
  return actions;
}

internal struct Ast*
build_ast_entry()
{
  struct Ast_TableEntry* entry = 0;
  if (token_is_keysetExpression(m_token)) {
    entry = new_ast_node(struct Ast_TableEntry, AST_TABLE_ENTRY);
    entry->line_no = m_token->line_no;
    entry->keyset = build_ast_keysetExpression();
    if (m_token->klass == TK_COLON) {
      next_token();
      entry->action = build_ast_actionRef();
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: keyset was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)entry;
}

internal struct List*
build_ast_entriesList()
{
  struct List* entries = 0;
  if (token_is_keysetExpression(m_token)) {
    entries = arena_push(m_ast_storage, sizeof(*entries));
    memset(entries, 0, sizeof(*entries));
    list_init(entries);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_entry();
    list_append_link(entries, li);
    while (token_is_keysetExpression(m_token)) {
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_entry();
      list_append_link(entries, li);
    }
  } else error("at line %d: keyset expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return entries;
}

internal struct Ast*
build_ast_tableProperty()
{
  struct Ast* prop = 0;
  if (token_is_tableProperty(m_token)) {
    bool is_const = false;
    if (m_token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    if (m_token->klass == TK_KEY) {
      next_token();
      struct Ast_TableKey* key_prop = new_ast_node(struct Ast_TableKey, AST_TABLE_KEY);
      key_prop->line_no = m_token->line_no;
      prop = (struct Ast*)key_prop;
      if (m_token->klass == TK_EQUAL) {
        next_token();
        if (m_token->klass == TK_BRACE_OPEN) {
          next_token();
          key_prop->keyelem_list = build_ast_keyElementList();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_ACTIONS) {
      next_token();
      struct Ast_TableActions* actions_prop = new_ast_node(struct Ast_TableActions, AST_TABLE_ACTIONS);
      actions_prop->line_no = m_token->line_no;
      prop = (struct Ast*)actions_prop;
      if (m_token->klass == TK_EQUAL) {
        next_token();
        if (m_token->klass == TK_BRACE_OPEN) {
          next_token();
          actions_prop->action_list = build_ast_actionList();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_ENTRIES) {
      next_token();
      struct Ast_TableEntries* entries_prop = new_ast_node(struct Ast_TableEntries, AST_TABLE_ENTRIES);
      entries_prop->line_no = m_token->line_no;
      entries_prop->is_const = is_const;
      prop = (struct Ast*)entries_prop;
      if (m_token->klass == TK_EQUAL) {
        next_token();
        if (m_token->klass == TK_BRACE_OPEN) {
          next_token();
          entries_prop->entries = build_ast_entriesList();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (token_is_nonTableKwName(m_token)) {
      struct Ast_TableSingleEntry* entry_prop = new_ast_node(struct Ast_TableSingleEntry, AST_TABLE_SINGLE_ENTRY);
      entry_prop->line_no = m_token->line_no;
      entry_prop->name = build_ast_name(false);
      prop = (struct Ast*)entry_prop;
      if (m_token->klass == TK_EQUAL) {
        next_token();
        entry_prop->init_expr = build_ast_initializer();
        if (m_token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else assert(0);
  } else error("at line %d: table property was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return prop;
}

internal struct List*
build_ast_tablePropertyList()
{
  struct List* props = 0;
  if (token_is_tableProperty(m_token)) {
    props = arena_push(m_ast_storage, sizeof(*props));
    memset(props, 0, sizeof(*props));
    list_init(props);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_tableProperty();
    list_append_link(props, li);
    while (token_is_tableProperty(m_token)) {
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_tableProperty();
      list_append_link(props, li);
    }
  } else error("at line %d: table property was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return props;
}

internal struct Ast*
build_ast_tableDeclaration()
{
  struct Ast_TableDecl* table = 0;
  if (m_token->klass == TK_TABLE) {
    next_token();
    table = new_ast_node(struct Ast_TableDecl, AST_TABLE_DECL);
    table->line_no = m_token->line_no;
    table->name = build_ast_name(false);
    if (m_token->klass == TK_BRACE_OPEN) {
      next_token();
      table->prop_list = build_ast_tablePropertyList();
      if (m_token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `table` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)table;
}

internal struct Ast*
build_ast_controlLocalDeclaration()
{
  struct Ast* decl = 0;
  if (m_token->klass == TK_CONST) {
    decl = build_ast_constantDeclaration();
  } else if (m_token->klass == TK_ACTION) {
    decl = build_ast_actionDeclaration();
  } else if (m_token->klass == TK_TABLE) {
    decl = build_ast_tableDeclaration();
  } else if (token_is_typeRef(m_token)) {
    struct Ast* type_ref = build_ast_typeRef();
    if (m_token->klass == TK_PARENTH_OPEN) {
      decl = build_ast_instantiation(type_ref);
    } else if (token_is_name(m_token)) {
      decl = build_ast_variableDeclaration(type_ref);
    } else error("at line %d: unexpected token `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: local declaration was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return decl;
}

internal struct List*
build_ast_controlLocalDeclarations()
{
  struct List* decls = 0;
  if (token_is_controlLocalDeclaration(m_token)) {
    decls = arena_push(m_ast_storage, sizeof(*decls));
    memset(decls, 0, sizeof(*decls));
    list_init(decls);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_controlLocalDeclaration();
    list_append_link(decls, li);
    while (token_is_controlLocalDeclaration(m_token)) {
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_controlLocalDeclaration();
      list_append_link(decls, li);
    }
  }
  return decls;
}

internal struct Ast*
build_ast_controlDeclaration()
{
  struct Ast_ControlDecl* decl = 0;
  if (m_token->klass == TK_CONTROL) {
    decl = new_ast_node(struct Ast_ControlDecl, AST_CONTROL_DECL);
    decl->line_no = m_token->line_no;
    decl->type_decl = build_ast_controlTypeDeclaration();
    if (m_token->klass == TK_SEMICOLON) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      decl->ctor_params = build_ast_optConstructorParameters();
      if (m_token->klass == TK_BRACE_OPEN) {
        next_token();
        decl->local_decls = build_ast_controlLocalDeclarations();
        if (m_token->klass == TK_APPLY) {
          next_token();
          decl->apply_stmt = build_ast_blockStatement();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `apply` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    }
  } else error("at line %d: `control` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_packageTypeDeclaration()
{
  struct Ast_PackageDecl* decl = 0;
  if (m_token->klass == TK_PACKAGE) {
    next_token();
    decl = new_ast_node(struct Ast_PackageDecl, AST_PACKAGE_DECL);
    decl->line_no = m_token->line_no;
    if (token_is_name(m_token)) {
      decl->name = build_ast_name(true);
      decl->type_params = build_ast_optTypeParameters();
      if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        decl->params = build_ast_parameterList();
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `package` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_typedefDeclaration()
{
  struct Ast* decl = 0;
  if (m_token->klass == TK_TYPEDEF || m_token->klass == TK_TYPE) {
    bool is_typedef = false;
    if (m_token->klass == TK_TYPEDEF) {
      is_typedef = true;
      next_token();
    } else if (m_token->klass == TK_TYPE) {
      next_token();
    } else assert(0);

    if (token_is_typeRef(m_token) || token_is_derivedTypeDeclaration(m_token)) {
      struct Ast_TypeDecl* type_decl = new_ast_node(struct Ast_TypeDecl, AST_TYPE_DECL);
      type_decl->line_no = m_token->line_no;
      type_decl->is_typedef = is_typedef;
      decl = (struct Ast*)type_decl;
      if (token_is_typeRef(m_token)) {
        type_decl->type_ref = build_ast_typeRef();
      } else if (token_is_derivedTypeDeclaration(m_token)) {
        type_decl->type_ref = build_ast_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(m_token)) {
        type_decl->name = build_ast_name(true);
        if (m_token->klass == TK_SEMICOLON) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type definition was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return decl;
}

internal struct Ast*
build_ast_typeDeclaration()
{
  struct Ast* decl = 0;
  if (token_is_typeDeclaration(m_token)) {
    if (token_is_derivedTypeDeclaration(m_token)) {
      decl = build_ast_derivedTypeDeclaration();
    } else if (m_token->klass == TK_TYPEDEF || m_token->klass == TK_TYPE) {
      decl = build_ast_typedefDeclaration();
    } else if (m_token->klass == TK_PARSER) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      decl = build_ast_parserDeclaration();
    } else if (m_token->klass == TK_CONTROL) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      decl = build_ast_controlDeclaration();
    } else if (m_token->klass == TK_PACKAGE) {
      decl = build_ast_packageTypeDeclaration();
      if (m_token->klass == TK_SEMICOLON) {
        next_token();
      } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else assert(0);
  } else error("at line %d: type declaration was expected, got `%s`.", m_token->line_no, m_token->lexeme); 
  return decl;
}

internal struct Ast*
build_ast_conditionalStatement()
{
  struct Ast_IfStmt* if_stmt = 0;
  if (m_token->klass == TK_IF) {
    next_token();
    if_stmt = new_ast_node(struct Ast_IfStmt, AST_IF_STMT);
    if_stmt->line_no = m_token->line_no;
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(m_token)) {
        if_stmt->cond_expr = build_ast_expression(1);
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token_is_statement(m_token)) {
            if_stmt->stmt = build_ast_statement(0);
            if (m_token->klass == TK_ELSE) {
              next_token();
              if (token_is_statement(m_token)) {
                if_stmt->else_stmt = build_ast_statement(0);
              } else error("at line %d: statement was expected, got `%s`.", m_token->line_no, m_token->lexeme);
            }
          } else error("at line %d: statement was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `if` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)if_stmt;
}

internal struct Ast*
build_ast_exitStatement()
{
  struct Ast_ExitStmt* exit_stmt = 0;
  if (m_token->klass == TK_EXIT) {
    next_token();
    exit_stmt = new_ast_node(struct Ast_ExitStmt, AST_EXIT_STMT);
    exit_stmt->line_no = m_token->line_no;
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)exit_stmt;
}

internal struct Ast*
build_ast_returnStatement()
{
  struct Ast_ReturnStmt* ret_stmt = 0;
  if (m_token->klass == TK_RETURN) {
    next_token();
    ret_stmt = new_ast_node(struct Ast_ReturnStmt, AST_RETURN_STMT);
    ret_stmt->line_no = m_token->line_no;
    if (token_is_expression(m_token))
      ret_stmt->expr = build_ast_expression(1);
    if (m_token->klass == TK_SEMICOLON) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `return` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)ret_stmt;
}

internal struct Ast*
build_ast_switchLabel()
{
  struct Ast* label = 0;
  if (token_is_name(m_token)) {
    label = build_ast_name(false);
  } else if (m_token->klass == TK_DEFAULT) {
    next_token();
    label = (struct Ast*)new_ast_node(struct Ast_DefaultStmt, AST_DEFAULT_STMT);
    label->line_no = m_token->line_no;
  } else error("at line %d: switch label was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return label;
}

internal struct Ast*
build_ast_switchCase()
{
  struct Ast_SwitchCase* switch_case = 0;
  if (token_is_switchLabel(m_token)) {
    switch_case = new_ast_node(struct Ast_SwitchCase, AST_SWITCH_CASE);
    switch_case->line_no = m_token->line_no;
    switch_case->label = build_ast_switchLabel();
    if (m_token->klass == TK_COLON) {
      next_token();
      if (m_token->klass == TK_BRACE_OPEN) {
        switch_case->stmt = build_ast_blockStatement();
      }
    } else error("at line %d: `:` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: switch label was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)switch_case;
}

internal struct List*
build_ast_switchCases()
{
  struct List* cases = 0;
  if (token_is_switchLabel(m_token)) {
    cases = arena_push(m_ast_storage, sizeof(*cases));
    memset(cases, 0, sizeof(*cases));
    list_init(cases);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_switchCase();
    list_append_link(cases, li);
    while (token_is_switchLabel(m_token)) {
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_switchCase();
      list_append_link(cases, li);
    }
  }
  return cases;
}

internal struct Ast*
build_ast_switchStatement()
{
  struct Ast_SwitchStmt* stmt = 0;
  if (m_token->klass == TK_SWITCH) {
    next_token();
    stmt = new_ast_node(struct Ast_SwitchStmt, AST_SWITCH_STMT);
    stmt->line_no = m_token->line_no;
    if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      stmt->expr = build_ast_expression(1);
      if (m_token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (m_token->klass == TK_BRACE_OPEN) {
          next_token();
          stmt->switch_cases = build_ast_switchCases();
          if (m_token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `switch` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)stmt;
}

internal struct Ast*
build_ast_statement(struct Ast* type_name)
{
  struct Ast* stmt = 0;
  if (token_is_typeName(m_token) || type_name) {
    stmt = build_ast_directApplication(type_name);
  } else if (token_is_assignmentOrMethodCallStatement(m_token)) {
    stmt = build_ast_assignmentOrMethodCallStatement();
  } else if (m_token->klass == TK_IF) {
    stmt = build_ast_conditionalStatement();
  } else if (m_token->klass == TK_SEMICOLON) {
    next_token();
    stmt = (struct Ast*)new_ast_node(struct Ast_EmptyStmt, AST_EMPTY_STMT);
    stmt->line_no = m_token->line_no;
  } else if (m_token->klass == TK_BRACE_OPEN) {
    stmt = build_ast_blockStatement();
  } else if (m_token->klass == TK_EXIT) {
    stmt = build_ast_exitStatement();
  } else if (m_token->klass == TK_RETURN) {
    stmt = build_ast_returnStatement();
  } else if (m_token->klass == TK_SWITCH) {
    stmt = build_ast_switchStatement();
  } else error("at line %d: statement was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return stmt;
}

internal struct Ast*
build_ast_statementOrDecl()
{
  struct Ast* stmt = 0;
  if (token_is_statementOrDeclaration(m_token)) {
    if (token_is_typeRef(m_token)) {
      struct Ast* type_ref = build_ast_typeRef();
      if (m_token->klass == TK_PARENTH_OPEN) {
        stmt = build_ast_instantiation(type_ref);
      } else if (token_is_name(m_token)) {
        stmt = build_ast_variableDeclaration(type_ref);
      } else {
        stmt = build_ast_statement(type_ref);
      }
    } else if (token_is_statement(m_token)) {
      stmt = build_ast_statement(0);
    } else if (m_token->klass == TK_CONST) {
      stmt = build_ast_constantDeclaration();
    } else assert(0);
  }
  return stmt;
}

internal struct List*
build_ast_statementOrDeclList()
{
  struct List* stmts = 0;
  if (token_is_statementOrDeclaration(m_token)) {
    stmts = arena_push(m_ast_storage, sizeof(*stmts));
    memset(stmts, 0, sizeof(*stmts));
    list_init(stmts);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_statementOrDecl();
    list_append_link(stmts, li);
    while (token_is_statementOrDeclaration(m_token)) {
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_statementOrDecl();
      list_append_link(stmts, li);
    }
  }
  return stmts;
}

internal struct Ast*
build_ast_blockStatement()
{
  struct Ast_BlockStmt* stmt = 0;
  if (m_token->klass == TK_BRACE_OPEN) {
    stmt = new_ast_node(struct Ast_BlockStmt, AST_BLOCK_STMT);
    stmt->line_no = m_token->line_no;
    next_token();
    stmt->stmt_list = build_ast_statementOrDeclList();
    if (m_token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)stmt;
}

internal struct List*
build_ast_identifierList()
{
  struct List* ids = 0;
  if (token_is_name(m_token)) {
    ids = arena_push(m_ast_storage, sizeof(*ids));
    memset(ids, 0, sizeof(*ids));
    list_init(ids);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_name(false);
    list_append_link(ids, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_name(false);
      list_append_link(ids, li);
    }
  } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return ids;
}

internal struct Ast*
build_ast_errorDeclaration()
{
  struct Ast_ErrorDecl* decl = 0;
  if (m_token->klass == TK_ERROR) {
    next_token();
    decl = new_ast_node(struct Ast_ErrorDecl, AST_ERROR_DECL);
    decl->line_no = m_token->line_no;
    if (m_token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(m_token)) {
        decl->id_list = build_ast_identifierList();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `error` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_matchKindDeclaration()
{
  struct Ast_MatchKindDecl* decl = 0;
  if (m_token->klass == TK_MATCH_KIND) {
    next_token();
    decl = new_ast_node(struct Ast_MatchKindDecl, AST_MATCH_KIND_DECL);
    decl->line_no = m_token->line_no;
    if (m_token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(m_token)) {
        decl->id_list = build_ast_identifierList();
        if (m_token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: `match_kind` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_functionDeclaration(struct Ast* type_ref)
{
  struct Ast_FunctionDecl* decl = 0;
  if (token_is_typeOrVoid(m_token)) {
    decl = new_ast_node(struct Ast_FunctionDecl, AST_FUNCTION_DECL);
    decl->line_no = m_token->line_no;
    decl->proto = build_ast_functionPrototype(type_ref);
    if (m_token->klass == TK_BRACE_OPEN) {
      decl->stmt = build_ast_blockStatement();
    } else error("at line %d: `{` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return (struct Ast*)decl;
}

internal struct Ast*
build_ast_declaration()
{
  struct Ast* decl = 0;
  if (token_is_declaration(m_token)) {
    if (m_token->klass == TK_CONST) {
      decl = build_ast_constantDeclaration();
    } else if (m_token->klass == TK_EXTERN) {
      decl = build_ast_externDeclaration();
    } else if (m_token->klass == TK_ACTION) {
      decl = build_ast_actionDeclaration();
    } else if (token_is_typeDeclaration(m_token)) {
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      decl = build_ast_typeDeclaration();
    } else if (m_token->klass == TK_ERROR) {
      decl = build_ast_errorDeclaration();
    } else if (m_token->klass == TK_MATCH_KIND) {
      decl = build_ast_matchKindDeclaration();
    } else if (token_is_typeRef(m_token)) {
      struct Ast* type_ref = build_ast_typeRef();
      if (m_token->klass == TK_PARENTH_OPEN) {
        decl = build_ast_instantiation(type_ref);
      } else if (token_is_name(m_token)) {
        decl = build_ast_functionDeclaration(type_ref);
      } else error("at line %d: unexpected token `%s`.", m_token->line_no, m_token->lexeme);
    } else if (token_is_typeOrVoid(m_token)) {
      decl = build_ast_functionDeclaration(build_ast_typeRef());
    } else assert(0);
  } else error("at line %d: top-level declaration as expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return decl;
}

internal struct Ast*
build_ast_p4program()
{
  struct Ast_P4Program* program = new_ast_node(struct Ast_P4Program, AST_P4PROGRAM);
  program->line_no = m_token->line_no;
  struct List* decls = arena_push(m_ast_storage, sizeof(*decls));
  memset(decls, 0, sizeof(*decls));
  list_init(decls);
  while (token_is_declaration(m_token) || m_token->klass == TK_SEMICOLON) {
    if (token_is_declaration(m_token)) {
      struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_declaration();
      list_append_link(decls, li);
    } else if (m_token->klass == TK_SEMICOLON) {
      next_token(); /* empty declaration */
    }
  }
  program->decl_list = decls;
  if (m_token->klass != TK_END_OF_INPUT) {
    error("at line %d: unexpected token `%s`.", m_token->line_no, m_token->lexeme);
  }
  return (struct Ast*)program;
}

internal bool
token_is_realTypeArg(struct Token* m_token)
{
  bool result = m_token->klass == TK_DONTCARE|| token_is_typeRef(m_token);
  return result;
}

internal bool
token_is_binaryOperator(struct Token* m_token)
{
  bool result = m_token->klass == TK_STAR || m_token->klass == TK_SLASH
    || m_token->klass == TK_PLUS || m_token->klass == TK_MINUS
    || m_token->klass == TK_ANGLE_OPEN_EQUAL || m_token->klass == TK_ANGLE_CLOSE_EQUAL
    || m_token->klass == TK_ANGLE_OPEN || m_token->klass == TK_ANGLE_CLOSE
    || m_token->klass == TK_EXCLAMATION_EQUAL || m_token->klass == TK_DOUBLE_EQUAL
    || m_token->klass == TK_DOUBLE_PIPE || m_token->klass == TK_DOUBLE_AMPERSAND
    || m_token->klass == TK_PIPE || m_token->klass == TK_AMPERSAND
    || m_token->klass == TK_CIRCUMFLEX || m_token->klass == TK_DOUBLE_ANGLE_OPEN
    || m_token->klass == TK_DOUBLE_ANGLE_CLOSE || m_token->klass == TK_TRIPLE_AMPERSAND
    || m_token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_exprOperator(struct Token* m_token)
{
  bool result = token_is_binaryOperator(m_token) || m_token->klass == TK_DOT_PREFIX
    || m_token->klass == TK_BRACKET_OPEN || m_token->klass == TK_PARENTH_OPEN
    || m_token->klass == TK_ANGLE_OPEN;
  return result;
}

internal struct Ast*
build_ast_realTypeArg()
{
  struct Ast* arg = 0;
  if (m_token->klass == TK_DONTCARE) {
    next_token();
    arg = (struct Ast*)new_ast_node(struct Ast_Dontcare, AST_DONTCARE);
    arg->line_no = m_token->line_no;
  } else if (token_is_typeRef(m_token)) {
    arg = build_ast_typeRef();
  } else error("at line %d: type argument was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return arg;
}

internal struct List*
build_ast_realTypeArgumentList()
{
  struct List* args = 0;
  if (token_is_realTypeArg(m_token)) {
    args = arena_push(m_ast_storage, sizeof(*args));
    memset(args, 0, sizeof(*args));
    list_init(args);
    struct ListLink* li = arena_push(m_ast_storage, sizeof(*li));
    memset(li, 0, sizeof(*li));
    li->object = build_ast_realTypeArg();
    list_append_link(args, li);
    while (m_token->klass == TK_COMMA) {
      next_token();
      li = arena_push(m_ast_storage, sizeof(*li));
      memset(li, 0, sizeof(*li));
      li->object = build_ast_realTypeArg();
      list_append_link(args, li);
    }
  }
  return args;
}

internal struct Ast*
build_ast_expressionPrimary()
{
  struct Ast* primary = 0;
  if (token_is_expression(m_token)) {
    if (m_token->klass == TK_INT_LITERAL) {
      primary = build_ast_integer();
    } else if (m_token->klass == TK_TRUE || m_token->klass == TK_FALSE) {
      primary = build_ast_boolean();
    } else if (m_token->klass == TK_STRING_LITERAL) {
      primary = build_ast_stringLiteral();
    } else if (m_token->klass == TK_DOT_PREFIX) {
      next_token();
      if (m_token->klass == TK_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_ast_nonTypeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else if (m_token->klass == TK_TYPE_IDENTIFIER) {
        struct Ast_Name* name = (struct Ast_Name*)build_ast_typeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else error("at line %d: unexpected token `%s`.", m_token->line_no, m_token->lexeme);
    } else if (token_is_nonTypeName(m_token)) {
      primary = build_ast_nonTypeName(false);
    } else if (m_token->klass == TK_BRACE_OPEN) {
      next_token();
      struct Ast_ExprListExpr* expr_list = new_ast_node(struct Ast_ExprListExpr, AST_EXPRLIST_EXPR);
      expr_list->line_no = m_token->line_no;
      expr_list->expr_list = build_ast_expressionList();
      primary = (struct Ast*)expr_list;
      if (m_token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(m_token)) {
        struct Ast_CastExpr* cast_expr = new_ast_node(struct Ast_CastExpr, AST_CAST_EXPR);
        cast_expr->line_no = m_token->line_no;
        cast_expr->to_type = build_ast_typeRef();
        primary = (struct Ast*)cast_expr;
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->expr = build_ast_expression(1);
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else if (token_is_expression(m_token)) {
        primary = build_ast_expression(1);
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
    } else if (m_token->klass == TK_EXCLAMATION) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR);
      unary_expr->line_no = m_token->line_no;
      unary_expr->op = OP_NOT;
      unary_expr->operand = build_ast_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (m_token->klass == TK_TILDA) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR);
      unary_expr->line_no = m_token->line_no;
      unary_expr->op = OP_BITWISE_NOT;
      unary_expr->operand = build_ast_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (m_token->klass == TK_UNARY_MINUS) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(struct Ast_UnaryExpr, AST_UNARY_EXPR);
      unary_expr->line_no = m_token->line_no;
      unary_expr->op = OP_NEG;
      unary_expr->operand = build_ast_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token_is_typeName(m_token)) {
      primary = build_ast_typeName();
    } else if (m_token->klass == TK_ERROR) {
      struct Ast_Name* name = new_ast_node(struct Ast_Name, AST_NAME);
      name->line_no = m_token->line_no;
      name->strname = m_token->lexeme;
      primary = (struct Ast*)name;
      next_token();
    } else assert(0);
  } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return primary;
}

internal int
get_operator_priority(struct Token* m_token)
{
  int prio = 0;
  if (m_token->klass == TK_DOUBLE_AMPERSAND || m_token->klass == TK_DOUBLE_PIPE) {
    /* Logical AND, OR */
    prio = 1;
  } else if (m_token->klass == TK_DOUBLE_EQUAL || m_token->klass == TK_EXCLAMATION_EQUAL
      || m_token->klass == TK_ANGLE_OPEN /* Less */ || m_token->klass == TK_ANGLE_CLOSE /* Greater */
      || m_token->klass == TK_ANGLE_OPEN_EQUAL /* LessEqual */ || m_token->klass == TK_ANGLE_CLOSE_EQUAL /* GreaterEqual */) {
    /* Relational ops  */
    prio = 2;
  }
  else if (m_token->klass == TK_PLUS || m_token->klass == TK_MINUS
           || m_token->klass == TK_AMPERSAND || m_token->klass == TK_PIPE
           || m_token->klass == TK_CIRCUMFLEX || m_token->klass == TK_DOUBLE_ANGLE_OPEN /* BitshiftLeft */
           || m_token->klass == TK_DOUBLE_ANGLE_CLOSE /* BitshiftRight */) {
    /* Addition and Subtraction; Bitwise ops */
    prio = 3;
  }
  else if (m_token->klass == TK_STAR || m_token->klass == TK_SLASH) {
    /* Multiplication and Division */
    prio = 4;
  }
  else if (m_token->klass == TK_TRIPLE_AMPERSAND) {
    /* Masking */
    prio = 5;
  }
  else assert(0);
  return prio;
}

internal enum AstExprOperator
token_to_binop(struct Token* m_token)
{
  switch (m_token->klass) {
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
build_ast_expression(int priority_threshold)
{
  struct Ast* expr = 0;
  if (token_is_expression(m_token)) {
    expr = build_ast_expressionPrimary();
    while (token_is_exprOperator(m_token)) {
      if (m_token->klass == TK_DOT_PREFIX) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = new_ast_node(struct Ast_MemberSelectExpr, AST_MEMBER_SELECT_EXPR);
        select_expr->line_no = m_token->line_no;
        select_expr->lhs_expr = expr;
        expr = (struct Ast*)select_expr;
        if (token_is_name(m_token)) {
          select_expr->member_name = build_ast_name(false);
        } else error("at line %d: name was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
      else if (m_token->klass == TK_BRACKET_OPEN) {
        next_token();
        struct Ast_SubscriptExpr* subscript_expr = new_ast_node(struct Ast_SubscriptExpr, AST_SUBSCRIPT_EXPR);
        subscript_expr->line_no = m_token->line_no;
        subscript_expr->index = expr;
        subscript_expr->colon_index = build_ast_arraySubscript();
        expr = (struct Ast*)subscript_expr;
        if (m_token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
      else if (m_token->klass == TK_PARENTH_OPEN) {
        next_token();
        struct Ast_FunctionCallExpr* call_expr = new_ast_node(struct Ast_FunctionCallExpr, AST_FUNCTION_CALL_EXPR);
        call_expr->line_no = m_token->line_no;
        call_expr->callee_expr = expr;
        call_expr->args = build_ast_argumentList();
        expr = (struct Ast*)call_expr;
        if (m_token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      }
      else if (m_token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        ((struct Ast_Expression*)expr)->type_args = build_ast_realTypeArgumentList();
        if (m_token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", m_token->line_no, m_token->lexeme);
      } else if (m_token->klass == TK_EQUAL) {
        next_token();
        struct Ast_KeyValuePairExpr* kv_pair = new_ast_node(struct Ast_KeyValuePairExpr, AST_KVPAIR_EXPR);
        kv_pair->line_no = m_token->line_no;
        kv_pair->name = expr;
        kv_pair->expr = build_ast_expression(1);
        expr = (struct Ast*)kv_pair;
      }
      else if (token_is_binaryOperator(m_token)){
        int priority = get_operator_priority(m_token);
        if (priority >= priority_threshold) {
          struct Ast_BinaryExpr* bin_expr = new_ast_node(struct Ast_BinaryExpr, AST_BINARY_EXPR);
          bin_expr->line_no = m_token->line_no;
          bin_expr->left_operand = expr;
          bin_expr->op = token_to_binop(m_token);
          next_token();
          bin_expr->right_operand = build_ast_expression(priority + 1);
          expr = (struct Ast*)bin_expr;
        } else break;
      } else assert(0);
    }
  } else error("at line %d: an expression was expected, got `%s`.", m_token->line_no, m_token->lexeme);
  return expr;
}

struct Ast*
build_ast(struct UnboundedArray* tokens_array, struct Arena* ast_storage)
{
  struct NameDecl*
  new_keyword(char* name, enum TokenClass token_class)
  {
    struct NameDecl* decl = arena_push_struct(m_ast_storage, struct NameDecl);
    decl->strname = name;
    decl->token_class = token_class;
    return decl;
  }

  m_tokens_array = tokens_array;
  m_ast_storage = ast_storage;
  symtable_init(m_ast_storage);

  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("action", TK_ACTION));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("actions", TK_ACTIONS));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("entries", TK_ENTRIES));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("enum", TK_ENUM));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("in", TK_IN));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("package", TK_PACKAGE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("select", TK_SELECT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("switch", TK_SWITCH));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("tuple", TK_TUPLE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("control", TK_CONTROL));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("error", TK_ERROR));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("header", TK_HEADER));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("inout", TK_INOUT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("parser", TK_PARSER));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("state", TK_STATE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("table", TK_TABLE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("key", TK_KEY));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("typedef", TK_TYPEDEF));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("type", TK_TYPE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("default", TK_DEFAULT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("extern", TK_EXTERN));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("header_union", TK_HEADER_UNION));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("out", TK_OUT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("transition", TK_TRANSITION));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("else", TK_ELSE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("exit", TK_EXIT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("if", TK_IF));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("match_kind", TK_MATCH_KIND));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("return", TK_RETURN));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("struct", TK_STRUCT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("apply", TK_APPLY));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("const", TK_CONST));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("bool", TK_BOOL));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("true", TK_TRUE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("false", TK_FALSE));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("void", TK_VOID));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("int", TK_INT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("bit", TK_BIT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("varbit", TK_VARBIT));
  declare_object_in_scope(get_root_scope(), NAMESPACE_KEYWORD, new_keyword("string", TK_STRING));

  m_token_at = 0;
  m_token = array_get(m_tokens_array, m_token_at);
  next_token();
  push_scope();
  struct Ast* p4program = build_ast_p4program();
  pop_scope();
  return p4program;
}
