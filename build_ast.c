#include "arena.h"
#include "hash.h"
#include "token.h"
#include "ast.h"
#include "lex.h"
#include "symtable.h"
#include "build_ast.h"
#include <memory.h>  // memset


internal struct Arena* ast_storage;

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
init_ast_node(struct Ast* ast, struct Token* token)
{
  ast->id = node_id++;
  ast->line_nr = token->line_nr;
  node_count += 1;
}

#define new_ast_node(type, token) ({ \
  struct type* ast = arena_push(ast_storage, sizeof(*ast)); \
  memset(ast, 0, sizeof(*ast)); \
  ast->kind = type; \
  init_ast_node((struct Ast*)ast, token); \
  ast; })

internal struct Token*
next_token()
{
  assert (token_at < tokens_array->elem_count);
  prev_token = token;
  prev_token_at = token_at;
  token = array_get(tokens_array, ++token_at);
  while (token->klass == Token_Comment) {
    token = array_get(tokens_array, ++token_at);
  }
  if (token->klass == Token_Identifier) {
    struct SymtableEntry* entry = scope_resolve_name(get_current_scope(), token->lexeme);
    if (entry->id_kw) {
      struct Symbol* id_kw = entry->id_kw;
      if (id_kw->symbol_kind == Symbol_Keyword) {
        token->klass = ((struct Symbol_Keyword*)id_kw)->token_klass;
        return token;
      }
    }
    if (entry->id_type) {
      struct Symbol* id_type = entry->id_type;
      if (id_type->symbol_kind == Symbol_Type) {
        token->klass = Token_TypeIdentifier;
        return token;
      }
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
  return token->klass == Token_TypeIdentifier || token->klass == Token_DotPrefix;
}

internal bool
token_is_prefixedType(struct Token* token)
{
  return token->klass == Token_TypeIdentifier || token->klass == Token_DotPrefix;
}

internal bool
token_is_baseType(struct Token* token)
{
  bool result = token->klass == Token_Bool || token->klass == Token_Error || token->klass == Token_Int
    || token->klass == Token_Bit || token->klass == Token_Varbit || token->klass == Token_String;
  return result;
}

internal bool
token_is_typeRef(struct Token* token)
{
  bool result = token_is_baseType(token) || token_is_prefixedType(token) || token->klass == Token_Tuple;
  return result;
}

internal bool
token_is_direction(struct Token* token)
{
  bool result = token->klass == Token_In || token->klass == Token_Out || token->klass == Token_InOut;
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
  bool result = token->klass == Token_Header || token->klass == Token_HeaderUnion || token->klass == Token_Struct
    || token->klass == Token_Enum;
  return result;
}

internal bool
token_is_typeDeclaration(struct Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == Token_Typedef || token->klass == Token_Type
    || token->klass == Token_Parser || token->klass == Token_Control || token->klass == Token_Package;
  return result;
}

internal bool
token_is_nonTypeName(struct Token* token)
{
  bool result = token->klass == Token_Identifier || token->klass == Token_Apply || token->klass == Token_Key
    || token->klass == Token_Actions || token->klass == Token_State || token->klass == Token_Entries || token->klass == Token_Type;
  return result;
}

internal bool
token_is_name(struct Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == Token_TypeIdentifier;
  return result;
}

internal bool
token_is_nonTableKwName(struct Token* token)
{
  bool result = token->klass == Token_Identifier || token->klass == Token_TypeIdentifier
    || token->klass == Token_Apply || token->klass == Token_State || token->klass == Token_Type;
  return result;
}

internal bool
token_is_typeArg(struct Token* token)
{
  bool result = token->klass == Token_Dontcare || token_is_typeRef(token) || token_is_nonTypeName(token);
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
  bool result = token_is_typeRef(token) || token->klass == Token_Void || token->klass == Token_Identifier;
  return result;
}

internal bool
token_is_actionRef(struct Token* token)
{
  bool result = token->klass == Token_DotPrefix || token_is_nonTypeName(token)
    || token->klass == Token_ParenthOpen;
  return result;
}

internal bool
token_is_tableProperty(struct Token* token)
{
  bool result = token->klass == Token_Key || token->klass == Token_Actions
    || token->klass == Token_Const || token->klass == Token_Entries
    || token_is_nonTableKwName(token);
  return result;
}

internal bool
token_is_switchLabel(struct Token* token)
{
  bool result = token_is_name(token) || token->klass == Token_Default;
  return result;
}

internal bool
token_is_expressionPrimary(struct Token* token)
{
  bool result = token->klass == Token_Integer || token->klass == Token_True || token->klass == Token_False
    || token->klass == Token_StringLiteral || token->klass == Token_DotPrefix || token_is_nonTypeName(token)
    || token->klass == Token_BraceOpen || token->klass == Token_ParenthOpen || token->klass == Token_Exclamation
    || token->klass == Token_Tilda || token->klass == Token_UnaryMinus || token_is_typeName(token)
    || token->klass == Token_Error || token_is_prefixedType(token);
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
    name = new_ast_node(Ast_Name, token);
    name->strname = token->lexeme;
    if (is_type) {
      new_type(get_current_scope(), name->strname, (struct Ast*)name, token->line_nr);
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
    } else if (token->klass == Token_TypeIdentifier) {
      struct Ast_Name* type_name = new_ast_node(Ast_Name, token);
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
    while (token->klass == Token_Comma) {
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
  if (token->klass == Token_AngleOpen) {
    next_token();
    if (token_is_typeParameterList(token)) {
      params = build_typeParameterList();
      if (token->klass == Token_AngleClose) {
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
    if (token->klass == Token_Dontcare) {
      struct Ast_Dontcare* dontcare = new_ast_node(Ast_Dontcare, token);
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
  return token_is_typeOrVoid(token) || token->klass == Token_TypeIdentifier;
}

internal enum AstParamDirection
build_direction()
{
  enum AstParamDirection dir = AstParamDir_NONE_;
  if (token_is_direction(token)) {
    if (token->klass == Token_In) {
      dir = AstParamDir_In;
    } else if (token->klass == Token_Out) {
      dir = AstParamDir_Out;
    } else if (token->klass == Token_InOut) {
      dir = AstParamDir_InOut;
    } else assert(0);
    next_token();
  }
  return dir;
}

internal struct Ast*
build_parameter()
{
  struct Ast_Parameter* param = new_ast_node(Ast_Parameter, token);
  param->direction = build_direction();
  if (token_is_typeRef(token)) {
    param->type = build_typeRef();
    if (token_is_name(token)) {
      param->name = build_name(false);
      if (token->klass == Token_Equal) {
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
    while (token->klass == Token_Comma) {
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
    } else if (token->klass == Token_Void) {
      struct Ast_Name* void_name = new_ast_node(Ast_Name, token);
      void_name->strname = token->lexeme;
      type = (struct Ast*)void_name;
      next_token();
    } else if (token->klass == Token_Identifier) {
      struct Ast_Name* name = new_ast_node(Ast_Name, token);
      name->strname = token->lexeme;
      type = (struct Ast*)name;
      if (is_type) {
        new_type(get_current_scope(), name->strname, type, token->line_nr);
      }
      next_token();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type;
}

internal struct Ast*
build_functionPrototype(struct Ast* type_ref)
{
  struct Ast_FunctionProto* proto = 0;
  if (token_is_typeOrVoid(token) || type_ref) {
    proto = new_ast_node(Ast_FunctionProto, token);
    if (type_ref) {
      proto->return_type = type_ref;
    } else {
      proto->return_type = build_typeOrVoid(true);
    }
    if (token_is_name(token)) {
      proto->name = build_name(false);
      proto->type_params = build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        proto->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
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
    if (token->klass == Token_TypeIdentifier && peek_token()->klass == Token_ParenthOpen) {
      /* Constructor */
      proto = new_ast_node(Ast_FunctionProto, token);
      proto->name = build_name(false);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        proto->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      proto = (struct Ast_FunctionProto*)build_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_Extern) {
    next_token();
    struct Ast_ExternDecl* extern_decl = new_ast_node(Ast_ExternDecl, token);
    decl = (struct Ast*)extern_decl;
    bool is_function_proto = false;
    if (token_is_typeOrVoid(token) && token_is_nonTypeName(token)) {
      is_function_proto = token_is_typeOrVoid(token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(token)) {
      is_function_proto = true;
    } else if (token_is_nonTypeName(token)) {
      is_function_proto = false;
    } else error("at line %d: extern declaration was expected, got `%s`.", token->line_nr, token->lexeme);

    if (is_function_proto) {
      decl = (struct Ast*)build_functionPrototype(0);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else {
      extern_decl->name = build_nonTypeName(true);
      extern_decl->type_params = build_optTypeParameters();
      if (token->klass == Token_BraceOpen) {
        next_token();
        extern_decl->method_protos = build_methodPrototypes();
        if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_Integer) {
    int_node = new_ast_node(Ast_IntLiteral, token);
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
  static int bool_true = 1;
  static int bool_false = 0;
  struct Ast_BoolLiteral* bool_node = 0;
  if (token->klass == Token_True || token->klass == Token_False) {
    bool_node = new_ast_node(Ast_BoolLiteral, token);
    bool_node->value = (token->klass == Token_True);
    next_token();
  }
  return (struct Ast*)bool_node;
}

internal struct Ast*
build_stringLiteral()
{
  struct Ast_StringLiteral* string = 0;
  if (token->klass == Token_StringLiteral) {
    string = new_ast_node(Ast_StringLiteral, token);
    string->value = token->lexeme;
    next_token();
  }
  return (struct Ast*)string;
}

internal struct Ast*
build_integerTypeSize()
{
  struct Ast_IntTypeSize* type_size = new_ast_node(Ast_IntTypeSize, token);
  if (token->klass == Token_Integer) {
    type_size->size = build_integer();
  } else if (token->klass == Token_ParenthOpen) {
    type_size->size = build_expression(1);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)type_size;
}

internal struct Ast*
build_baseType()
{
  struct Ast_BaseType* base_type = 0;
  if (token_is_baseType(token)) {
    base_type = new_ast_node(Ast_BaseType, token);
    struct Ast_Name* type_name = new_ast_node(Ast_Name, token);
    base_type->type_name = (struct Ast*)type_name;
    if (token->klass == Token_Bool) {
      base_type->base_type = AstBaseType_Bool;
      type_name->strname = "bool";
      next_token();
    } else if (token->klass == Token_Error) {
      base_type->base_type = AstBaseType_Error;
      type_name->strname = "error";
      next_token();
    } else if (token->klass == Token_Int) {
      base_type->base_type = AstBaseType_Int;
      type_name->strname = "int";
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_Bit) {
      base_type->base_type = AstBaseType_Bit;
      type_name->strname = "bit";
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_Varbit) {
      base_type->base_type = AstBaseType_Varbit;
      type_name->strname = "varbit";
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_String) {
      base_type->base_type = AstBaseType_String;
      type_name->strname = "string";
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)base_type;
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
    while (token->klass == Token_Comma) {
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
  if (token->klass == Token_Tuple) {
    next_token();
    type = new_ast_node(Ast_Tuple, token);
    if (token->klass == Token_AngleOpen) {
      next_token();
      type->type_args = build_typeArgumentList();
      if (token->klass == Token_AngleClose) {
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
  if (token->klass == Token_BracketOpen) {
    next_token();
    stack = new_ast_node(Ast_HeaderStack, token);
    if (token_is_expression(token)) {
      stack->stack_expr = build_expression(1);
      if (token->klass == Token_BracketClose) {
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
  if (token->klass == Token_AngleOpen) {
    next_token();
    type = new_ast_node(Ast_SpecializedType, token);
    type->type_args = build_typeArgumentList();
    if (token->klass == Token_AngleClose) {
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
  if (token->klass == Token_DotPrefix) {
    next_token();
    is_dotprefixed = true;
  }
  if (token->klass == Token_TypeIdentifier) {
    name = new_ast_node(Ast_Name, token);
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
    if (token->klass == Token_AngleOpen) {
      struct Ast* specd_type = build_specializedType();
      specd_type->name = name;
      name = specd_type;
    } if (token->klass == Token_BracketOpen) {
      struct Ast* stack_type = build_headerStackType();
      stack_type->name = name;
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
    } else if (token->klass == Token_Tuple) {
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
  struct Ast_StructField* field = new_ast_node(Ast_StructField, token);
  if (token_is_typeRef(token)) {
    field->type = build_typeRef();
    if (token_is_name(token)) {
      field->name = build_name(false);
      if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_Header) {
    next_token();
    decl = new_ast_node(Ast_HeaderDecl, token);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == Token_BraceOpen) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_HeaderUnion) {
    next_token();
    decl = new_ast_node(Ast_HeaderUnionDecl, token);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == Token_BraceOpen) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_Struct) {
    next_token();
    decl = new_ast_node(Ast_StructDecl, token);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == Token_BraceOpen) {
        next_token();
        decl->fields = build_structFieldList();
        if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_Equal) {
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
    id = new_ast_node(Ast_SpecifiedIdent, token);
    id->name = build_name(false);
    if (token->klass == Token_Equal) {
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
    while (token->klass == Token_Comma) {
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
  if (token->klass == Token_Enum) {
    next_token();
    decl = new_ast_node(Ast_EnumDecl, token);
    if (token->klass == Token_Bit) {
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        if (token->klass == Token_Integer) {
          decl->type_size = build_integer();
          if (token->klass == Token_AngleClose) {
            next_token();
          } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: an integer was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
    if (token_is_name(token)) {
      decl->name = build_name(true);
      if (token->klass == Token_BraceOpen) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          decl->id_list = build_specifiedIdentifierList();
          if (token->klass == Token_BraceClose) {
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
    if (token->klass == Token_Header) {
      decl = build_headerTypeDeclaration();
    } else if (token->klass == Token_HeaderUnion) {
      decl = build_headerUnionDeclaration();
    } else if (token->klass == Token_Struct) {
      decl = build_structTypeDeclaration();
    } else if (token->klass == Token_Enum) {
      decl = build_enumDeclaration();
    } else assert(0);
  } else error("at line %d: structure declaration was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_parserTypeDeclaration()
{
  struct Ast_ParserType* type = 0;
  if (token->klass == Token_Parser) {
    next_token();
    type = new_ast_node(Ast_ParserType, token);
    if (token_is_name(token)) {
      type->name = build_name(true);
      type->type_params = build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        type->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
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
  if (token->klass == Token_ParenthOpen) {
    next_token();
    ctor_params = build_parameterList();
    if (token->klass == Token_ParenthClose) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
  }
  return ctor_params;
}

internal struct Ast*
build_constantDeclaration()
{
  struct Ast_ConstDecl* decl = 0;
  if (token->klass == Token_Const) {
    next_token();
    decl = new_ast_node(Ast_ConstDecl, token);
    if (token_is_typeRef(token)) {
      decl->type_ref = build_typeRef();
      if (token_is_name(token)) {
        decl->name = build_name(false);
        if (token->klass == Token_Equal) {
          next_token();
          if (token_is_expression(token)) {
            decl->expr = build_expression(1);
            if (token->klass == Token_Semicolon) {
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
  bool result = token->klass == Token_Const || token->klass == Token_Extern || token->klass == Token_Action
    || token->klass == Token_Parser || token_is_typeDeclaration(token) || token->klass == Token_Control
    || token_is_typeRef(token) || token->klass == Token_Error || token->klass == Token_MatchKind
    || token_is_typeOrVoid(token) || token->klass == Token_DotPrefix;
  return result;
}

internal bool
token_is_lvalue(struct Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == Token_DotPrefix;
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(struct Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == Token_ParenthOpen || token->klass == Token_AngleOpen
    || token->klass == Token_Equal;
  return result;
}

internal bool
token_is_statement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == Token_If
    || token->klass == Token_Semicolon || token->klass == Token_BraceOpen || token->klass == Token_Exit
    || token->klass == Token_Return || token->klass == Token_Switch;
  return result;
}

internal bool
token_is_statementOrDeclaration(struct Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == Token_Const || token_is_statement(token);
  return result;
}

internal bool
token_is_argument(struct Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == Token_Dontcare;
  return result;
}

internal bool
token_is_parserLocalElement(struct Token* token)
{
  bool result = token->klass == Token_Const || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == Token_BraceOpen || token->klass == Token_Const || token_is_typeRef(token)
    || token->klass == Token_Semicolon;
  return result;
}

internal bool
token_is_simpleKeysetExpression(struct Token* token) {
  bool result = token_is_expression(token) || token->klass == Token_Default || token->klass == Token_Dontcare;
  return result;
}

internal bool
token_is_keysetExpression(struct Token* token)
{
  bool result = token->klass == Token_Tuple || token_is_simpleKeysetExpression(token);
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
  bool result = token->klass == Token_Const || token->klass == Token_Action
    || token->klass == Token_Table || token_is_typeRef(token) || token_is_typeRef(token);
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
      struct Ast_Argument* name_arg = new_ast_node(Ast_Argument, token);
      arg = (struct Ast*)name_arg;
      name_arg->name = build_name(false);
      if (token->klass == Token_Equal) {
        next_token();
        if (token_is_expression(token)) {
          name_arg->init_expr = build_expression(1);
        } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Dontcare) {
      struct Ast_Dontcare* dontcare_arg = new_ast_node(Ast_Dontcare, token);
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
    while (token->klass == Token_Comma) {
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
    decl = new_ast_node(Ast_VarDecl, token);
    decl->type = type_ref ? type_ref : build_typeRef();
    if (token_is_name(token)) {
      decl->name = build_name(false);
      decl->init_expr = build_optInitializer();
      if (token->klass == Token_Semicolon) {
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
    inst = new_ast_node(Ast_Instantiation, token);
    inst->type_ref = type_ref ? type_ref : build_typeRef();
    if (token->klass == Token_ParenthOpen) {
      next_token();
      inst->args = build_argumentList();
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token_is_name(token)) {
          inst->name = build_name(false);
          if (token->klass == Token_Semicolon) {
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
    if (token->klass == Token_Const) {
      elem = build_constantDeclaration();
    } else if (token_is_typeRef(token)) {
      struct Ast* type_ref = build_typeRef();
      if (token->klass == Token_ParenthOpen) {
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
    applic = new_ast_node(Ast_DirectApplication, token);
    applic->name = type_name ? type_name : build_typeName();
    if (token->klass == Token_DotPrefix) {
      next_token();
      if (token->klass == Token_Apply) {
        next_token();
        if (token->klass == Token_ParenthOpen) {
          next_token();
          applic->args = build_argumentList();
          if (token->klass == Token_ParenthClose) {
            next_token();
            if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_DotPrefix) {
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
  struct Ast_IndexedArrayExpr* index = new_ast_node(Ast_IndexedArrayExpr, token);
  if (token_is_expression(token)) {
    index->index = build_expression(1);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  if (token->klass == Token_Colon) {
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
  if (token->klass == Token_DotPrefix) {
    next_token();
    struct Ast_Name* dot_member = (struct Ast_Name*)build_name(false);
    dot_member->is_dotprefixed = true;
    expr = (struct Ast*)dot_member;
  } else if (token->klass == Token_BracketOpen) {
    next_token();
    expr = build_arrayIndex();
    if (token->klass == Token_BracketClose) {
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
    lvalue = new_ast_node(Ast_Lvalue, token);
    lvalue->name = build_prefixedNonTypeName();
    if (token->klass == Token_DotPrefix || token->klass == Token_BracketOpen) {
      struct List* lvalue_expr = arena_push(ast_storage, sizeof(*lvalue_expr));
      memset(lvalue_expr, 0, sizeof(*lvalue_expr));
      list_init(lvalue_expr);
      struct ListLink* link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_lvalueExpr();
      list_append_link(lvalue_expr, link);
      while (token->klass == Token_DotPrefix || token->klass == Token_BracketOpen) {
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
    if (token->klass == Token_AngleOpen) {
      next_token();
      type_args = build_typeArgumentList();
      if (token->klass == Token_AngleClose) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
    if (token->klass == Token_ParenthOpen) {
      next_token();
      struct Ast_MethodCallStmt* call_stmt = new_ast_node(Ast_MethodCallStmt, token);
      call_stmt->lvalue = lvalue;
      call_stmt->type_args = type_args;
      call_stmt->args = build_argumentList();
      stmt = (struct Ast*)call_stmt;
      if (token->klass == Token_ParenthClose) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Equal) {
      next_token();
      struct Ast_AssignmentStmt* assgn_stmt = new_ast_node(Ast_AssignmentStmt, token);
      assgn_stmt->lvalue = lvalue;
      assgn_stmt->expr = build_expression(1);
      stmt = (struct Ast*)assgn_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_BraceOpen) {
    stmt = new_ast_node(Ast_BlockStmt, token);
    next_token();
    stmt->stmt_list = build_parserStatements();
    if (token->klass == Token_BraceClose) {
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
  } else if (token->klass == Token_BraceOpen) {
    stmt = build_parserBlockStatements();
  } else if (token->klass == Token_Const) {
    stmt = build_constantDeclaration();
  } else if (token->klass == Token_Semicolon) {
    stmt = (struct Ast*)new_ast_node(Ast_EmptyStmt, token);
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
    while (token->klass == Token_Comma) {
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
  } else if (token->klass == Token_Default) {
    next_token();
    expr = (struct Ast*)new_ast_node(Ast_Default, token);
  } else if (token->klass == Token_Dontcare) {
    next_token();
    expr = (struct Ast*)new_ast_node(Ast_Dontcare, token);
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct Ast*
build_tupleKeysetExpression()
{
  struct Ast_TupleKeyset* tuple_keyset = 0;
  if (token->klass == Token_ParenthOpen) {
    tuple_keyset = new_ast_node(Ast_TupleKeyset, token);
    next_token();
    struct List* exprs = arena_push(ast_storage, sizeof(*exprs));
    memset(exprs, 0, sizeof(exprs));
    list_init(exprs);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_simpleKeysetExpression();
    list_append_link(exprs, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_simpleKeysetExpression();
      list_append_link(exprs, link);
    }
    tuple_keyset->expr_list = exprs;
    if (token->klass == Token_ParenthClose) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)tuple_keyset;
}

internal struct Ast*
build_keysetExpression()
{
  struct Ast* expr = 0;
  if (token->klass == Token_ParenthOpen) {
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
    select_case = new_ast_node(Ast_SelectCase, token);
    select_case->keyset = build_keysetExpression();
    if (token->klass == Token_Colon) {
      next_token();
      if (token_is_name(token)) {
        select_case->name = build_name(false);
        if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_Select) {
    next_token();
    select_expr = new_ast_node(Ast_SelectExpr, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      select_expr->expr_list = build_expressionList();
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          select_expr->case_list = build_selectCaseList();
          if (token->klass == Token_BraceClose) {
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
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else if (token->klass == Token_Select) {
    state_expr = build_selectExpression();
  } else error("at line %d: state expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return state_expr;
}

internal struct Ast*
build_transitionStatement()
{
  struct Ast* stmt = 0;
  if (token->klass == Token_Transition) {
    next_token();
    stmt = build_stateExpression();
  } else error("at line %d: `transition` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct Ast*
build_parserState()
{
  struct Ast_ParserState* state = 0;
  if (token->klass == Token_State) {
    next_token();
    state = new_ast_node(Ast_ParserState, token);
    state->name = build_name(false);
    if (token->klass == Token_BraceOpen) {
      next_token();
      state->stmt_list = build_parserStatements();
      state->trans_stmt = build_transitionStatement();
      if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_State) {
    states = arena_push(ast_storage, sizeof(*states));
    memset(states, 0, sizeof(*states));
    list_init(states);
    struct ListLink* link = arena_push(ast_storage, sizeof(*link));
    memset(link, 0, sizeof(*link));
    link->object = build_parserState();
    list_append_link(states, link);
    while (token->klass == Token_State) {
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
  if (token->klass == Token_Parser) {
    decl = new_ast_node(Ast_ParserDecl, token);
    decl->type_decl = build_parserTypeDeclaration();
    if (token->klass == Token_Semicolon) {
      next_token(); /* <parserTypeDeclaration> */
    } else {
      decl->ctor_params = build_optConstructorParameters();
      if (token->klass == Token_BraceOpen) {
        next_token();
        decl->local_elements = build_parserLocalElements();
        decl->states = build_parserStates();
        if (token->klass == Token_BraceClose) {
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
  struct Ast_ControlType* decl = 0;
  if (token->klass == Token_Control) {
    next_token();
    decl = new_ast_node(Ast_ControlType, token);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
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
  if (token->klass == Token_Action) {
    next_token();
    decl = new_ast_node(Ast_ActionDecl, token);
    if (token_is_name(token)) {
      decl->name = build_name(false);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token->klass == Token_BraceOpen) {
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
    key_elem = new_ast_node(Ast_KeyElement, token);
    key_elem->expr = build_expression(1);
    if (token->klass == Token_Colon) {
      next_token();
      key_elem->name = build_name(false);
      if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_DotPrefix || token_is_nonTypeName(token)) {
    ref = new_ast_node(Ast_ActionRef, token);
    ref->name = build_prefixedNonTypeName();
    if (token->klass == Token_ParenthOpen) {
      next_token();
      ref->args = build_argumentList();
      if (token->klass == Token_ParenthClose) {
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
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    while (token_is_actionRef(token)) {
      link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_actionRef();
      list_append_link(actions, link);
      if (token->klass == Token_Semicolon) {
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
    entry = new_ast_node(Ast_TableEntry, token);
    entry->keyset = build_keysetExpression();
    if (token->klass == Token_Colon) {
      next_token();
      entry->action = build_actionRef();
      if (token->klass == Token_Semicolon) {
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
    if (token->klass == Token_Const) {
      next_token();
      is_const = true;
    }
    if (token->klass == Token_Key) {
      next_token();
      struct Ast_TableProp_Key* key_prop = new_ast_node(Ast_TableProp_Key, token);
      prop = (struct Ast*)key_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          key_prop->keyelem_list = build_keyElementList();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Actions) {
      next_token();
      struct Ast_TableProp_Actions* actions_prop = new_ast_node(Ast_TableProp_Actions, token);
      prop = (struct Ast*)actions_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          actions_prop->action_list = build_actionList();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Entries) {
      next_token();
      struct Ast_TableProp_Entries* entries_prop = new_ast_node(Ast_TableProp_Entries, token);
      entries_prop->is_const = is_const;
      prop = (struct Ast*)entries_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          entries_prop->entries = build_entriesList();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTableKwName(token)) {
      struct Ast_TableProp_SingleEntry* entry_prop = new_ast_node(Ast_TableProp_SingleEntry, token);
      entry_prop->name = build_name(false);
      prop = (struct Ast*)entry_prop;
      if (token->klass == Token_Equal) {
        next_token();
        entry_prop->init_expr = build_initializer();
        if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_Table) {
    next_token();
    table = new_ast_node(Ast_TableDecl, token);
    table->name = build_name(false);
    if (token->klass == Token_BraceOpen) {
      next_token();
      table->prop_list = build_tablePropertyList();
      if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_Const) {
    decl = build_constantDeclaration();
  } else if (token->klass == Token_Action) {
    decl = build_actionDeclaration();
  } else if (token->klass == Token_Table) {
    decl = build_tableDeclaration();
  } else if (token_is_typeRef(token)) {
    struct Ast* type_ref = build_typeRef();
    if (token->klass == Token_ParenthOpen) {
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
  if (token->klass == Token_Control) {
    decl = new_ast_node(Ast_ControlDecl, token);
    decl->type_decl = build_controlTypeDeclaration();
    if (token->klass == Token_Semicolon) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      decl->ctor_params = build_optConstructorParameters();
      if (token->klass == Token_BraceOpen) {
        next_token();
        decl->local_decls = build_controlLocalDeclarations();
        if (token->klass == Token_Apply) {
          next_token();
          decl->apply_stmt = build_blockStatement();
          if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_Package) {
    next_token();
    decl = new_ast_node(Ast_PackageDecl, token);
    if (token_is_name(token)) {
      decl->name = build_name(true);
      decl->type_params = build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        decl->params = build_parameterList();
        if (token->klass == Token_ParenthClose) {
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
  if (token->klass == Token_Typedef || token->klass == Token_Type) {
    bool is_typedef = false;
    if (token->klass == Token_Typedef) {
      is_typedef = true;
      next_token();
    } else if (token->klass == Token_Type) {
      next_token();
    } else assert(0);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      struct Ast_TypeDecl* type_decl = new_ast_node(Ast_TypeDecl, token);
      type_decl->is_typedef = is_typedef;
      decl = (struct Ast*)type_decl;
      if (token_is_typeRef(token)) {
        type_decl->type_ref = build_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->type_ref = build_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        type_decl->name = build_name(true);
        if (token->klass == Token_Semicolon) {
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
    } else if (token->klass == Token_Typedef || token->klass == Token_Type) {
      decl = build_typedefDeclaration();
    } else if (token->klass == Token_Parser) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      decl = build_parserDeclaration();
    } else if (token->klass == Token_Control) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      decl = build_controlDeclaration();
    } else if (token->klass == Token_Package) {
      decl = build_packageTypeDeclaration();
      if (token->klass == Token_Semicolon) {
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
  if (token->klass == Token_If) {
    next_token();
    if_stmt = new_ast_node(Ast_IfStmt, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_expression(token)) {
        if_stmt->cond_expr = build_expression(1);
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token_is_statement(token)) {
            if_stmt->stmt = build_statement(0);
            if (token->klass == Token_Else) {
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
  if (token->klass == Token_Exit) {
    next_token();
    exit_stmt = new_ast_node(Ast_ExitStmt, token);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_nr, token->lexeme);
  return (struct Ast*)exit_stmt;
}

internal struct Ast*
build_returnStatement()
{
  struct Ast_ReturnStmt* ret_stmt = 0;
  if (token->klass == Token_Return) {
    next_token();
    ret_stmt = new_ast_node(Ast_ReturnStmt, token);
    if (token_is_expression(token))
      ret_stmt->expr = build_expression(1);
    if (token->klass == Token_Semicolon) {
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
    struct Ast_SwitchLabel* name_label = new_ast_node(Ast_SwitchLabel, token);
    label = (struct Ast*)name_label;
    name_label->name = build_name(false);
  } else if (token->klass == Token_Default) {
    next_token();
    label = (struct Ast*)new_ast_node(Ast_Default, token);
  } else error("at line %d: switch label was expected, got `%s`.", token->line_nr, token->lexeme);
  return label;
}

internal struct Ast*
build_switchCase()
{
  struct Ast_SwitchCase* switch_case = 0;
  if (token_is_switchLabel(token)) {
    switch_case = new_ast_node(Ast_SwitchCase, token);
    switch_case->label = build_switchLabel();
    if (token->klass == Token_Colon) {
      next_token();
      if (token->klass == Token_BraceOpen) {
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
  if (token->klass == Token_Switch) {
    next_token();
    stmt = new_ast_node(Ast_SwitchStmt, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      stmt->expr = build_expression(1);
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          stmt->switch_cases = build_switchCases();
          if (token->klass == Token_BraceClose) {
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
  } else if (token->klass == Token_If) {
    stmt = build_conditionalStatement();
  } else if (token->klass == Token_Semicolon) {
    next_token();
    stmt = (struct Ast*)new_ast_node(Ast_EmptyStmt, token);
  } else if (token->klass == Token_BraceOpen) {
    stmt = build_blockStatement();
  } else if (token->klass == Token_Exit) {
    stmt = build_exitStatement();
  } else if (token->klass == Token_Return) {
    stmt = build_returnStatement();
  } else if (token->klass == Token_Switch) {
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
      if (token->klass == Token_ParenthOpen) {
        stmt = build_instantiation(type_ref);
      } else if (token_is_name(token)) {
        stmt = build_variableDeclaration(type_ref);
      } else {
        stmt = build_statement(type_ref);
      }
    } else if (token_is_statement(token)) {
      stmt = build_statement(0);
    } else if (token->klass == Token_Const) {
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
  if (token->klass == Token_BraceOpen) {
    stmt = new_ast_node(Ast_BlockStmt, token);
    next_token();
    stmt->stmt_list = build_statementOrDeclList();
    if (token->klass == Token_BraceClose) {
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
    while (token->klass == Token_Comma) {
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
  if (token->klass == Token_Error) {
    next_token();
    decl = new_ast_node(Ast_ErrorDecl, token);
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == Token_BraceClose) {
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
  if (token->klass == Token_MatchKind) {
    next_token();
    decl = new_ast_node(Ast_MatchKindDecl, token);
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        decl->id_list = build_identifierList();
        if (token->klass == Token_BraceClose) {
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
    decl = new_ast_node(Ast_FunctionDecl, token);
    decl->proto = build_functionPrototype(type_ref);
    if (token->klass == Token_BraceOpen) {
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
    if (token->klass == Token_Const) {
      decl = build_constantDeclaration();
    } else if (token->klass == Token_Extern) {
      decl = build_externDeclaration();
    } else if (token->klass == Token_Action) {
      decl = build_actionDeclaration();
    } else if (token_is_typeDeclaration(token)) {
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      decl = build_typeDeclaration();
    } else if (token->klass == Token_Error) {
      decl = build_errorDeclaration();
    } else if (token->klass == Token_MatchKind) {
      decl = build_matchKindDeclaration();
    } else if (token_is_typeRef(token)) {
      struct Ast* type_ref = build_typeRef();
      if (token->klass == Token_ParenthOpen) {
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
  struct Ast_P4Program* program = new_ast_node(Ast_P4Program, token);
  struct List* decls = arena_push(ast_storage, sizeof(*decls));
  memset(decls, 0, sizeof(*decls));
  list_init(decls);
  while (token_is_declaration(token) || token->klass == Token_Semicolon) {
    if (token_is_declaration(token)) {
      struct ListLink* link = arena_push(ast_storage, sizeof(*link));
      memset(link, 0, sizeof(*link));
      link->object = build_declaration();
      list_append_link(decls, link);
    } else if (token->klass == Token_Semicolon) {
      next_token(); /* empty declaration */
    }
  }
  program->decl_list = decls;
  if (token->klass != Token_EndOfInput_) {
    error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
  }
  return (struct Ast*)program;
}

internal bool
token_is_realTypeArg(struct Token* token)
{
  bool result = token->klass == Token_Dontcare || token_is_typeRef(token);
  return result;
}

internal bool
token_is_binaryOperator(struct Token* token)
{
  bool result = token->klass == Token_Star || token->klass == Token_Slash
    || token->klass == Token_Plus || token->klass == Token_Minus
    || token->klass == Token_AngleOpenEqual || token->klass == Token_AngleCloseEqual
    || token->klass == Token_AngleOpen || token->klass == Token_AngleClose
    || token->klass == Token_ExclamationEqual || token->klass == Token_TwoEqual
    || token->klass == Token_TwoPipe || token->klass == Token_TwoAmpersand
    || token->klass == Token_Pipe || token->klass == Token_Ampersand
    || token->klass == Token_Circumflex || token->klass == Token_TwoAngleOpen
    || token->klass == Token_TwoAngleClose || token->klass == Token_ThreeAmpersand
    || token->klass == Token_Equal;
  return result;
}

internal bool
token_is_exprOperator(struct Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == Token_DotPrefix
    || token->klass == Token_BracketOpen || token->klass == Token_ParenthOpen
    || token->klass == Token_AngleOpen;
  return result;
}

internal struct Ast*
build_realTypeArg()
{
  struct Ast* arg = 0;
  if (token->klass == Token_Dontcare) {
    next_token();
    arg = (struct Ast*)new_ast_node(Ast_Dontcare, token);
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
    while (token->klass == Token_Comma) {
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
    if (token->klass == Token_Integer) {
      primary = build_integer();
    } else if (token->klass == Token_True) {
      primary = build_boolean();
    } else if (token->klass == Token_False) {
      primary = build_boolean();
    } else if (token->klass == Token_StringLiteral) {
      primary = build_stringLiteral();
    } else if (token->klass == Token_DotPrefix) {
      next_token();
      if (token->klass == Token_Identifier) {
        struct Ast_Name* name = (struct Ast_Name*)build_nonTypeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else if (token->klass == Token_TypeIdentifier) {
        struct Ast_Name* name = (struct Ast_Name*)build_typeName(false);
        name->is_dotprefixed = true;
        primary = (struct Ast*)name;
      } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTypeName(token)) {
      primary = build_nonTypeName(false);
    } else if (token->klass == Token_BraceOpen) {
      next_token();
      struct Ast_ExpressionListExpr* expr_list = new_ast_node(Ast_ExpressionListExpr, token);
      expr_list->expr_list = build_expressionList();
      primary = (struct Ast*)expr_list;
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_typeRef(token)) {
        struct Ast_CastExpr* cast = new_ast_node(Ast_CastExpr, token);
        cast->to_type = build_typeRef();
        primary = (struct Ast*)cast;
        if (token->klass == Token_ParenthClose) {
          next_token();
          cast->expr = build_expression(1);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token_is_expression(token)) {
        primary = build_expression(1);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Exclamation) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      unary_expr->op = AstExprOp_LogNot;
      enum AstExprOperator* op = arena_push(ast_storage, sizeof(*op));
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == Token_Tilda) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      unary_expr->op = AstExprOp_BitNot;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token->klass == Token_UnaryMinus) {
      next_token();
      struct Ast_UnaryExpr* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      unary_expr->op = AstExprOp_Minus;
      unary_expr->operand = build_expression(1);
      primary = (struct Ast*)unary_expr;
    } else if (token_is_typeName(token)) {
      primary = build_typeName();
    } else if (token->klass == Token_Error) {
      struct Ast_Name* name = new_ast_node(Ast_Name, token);
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
  if (token->klass == Token_TwoAmpersand || token->klass == Token_TwoPipe) {
    prio = 1;
  } else if (token->klass == Token_TwoEqual || token->klass == Token_ExclamationEqual
      || token->klass == Token_AngleOpen /* Less */ || token->klass == Token_AngleClose /* Greater */
      || token->klass == Token_AngleOpenEqual /* LessEqual */ || token->klass == Token_AngleCloseEqual /* GreaterEqual */) {
    prio = 2;
  }
  else if (token->klass == Token_Plus || token->klass == Token_Minus
           || token->klass == Token_Ampersand || token->klass == Token_Pipe
           || token->klass == Token_Circumflex || token->klass == Token_TwoAngleOpen /* BitshiftLeft */
           || token->klass == Token_TwoAngleClose /* BitshiftRight */) {
    prio = 3;
  }
  else if (token->klass == Token_Star || token->klass == Token_Slash) {
    prio = 4;
  }
  else if (token->klass == Token_ThreeAmpersand) {
    prio = 5;
  }
  else assert(0);
  return prio;
}

internal enum AstExprOperator
token_to_binop(struct Token* token)
{
  switch (token->klass) {
    case Token_TwoAmpersand:
      return AstExprOp_And;
    case Token_TwoPipe:
      return AstExprOp_Or;
    case Token_TwoEqual:
      return AstExprOp_Equal;
    case Token_ExclamationEqual:
      return AstExprOp_NotEqual;
    case Token_AngleOpen:
      return AstExprOp_Less;
    case Token_AngleClose:
      return AstExprOp_Greater;
    case Token_AngleOpenEqual:
      return AstExprOp_LessEqual;
    case Token_AngleCloseEqual:
      return AstExprOp_GreaterEqual;
    case Token_Plus:
      return AstExprOp_Add;
    case Token_Minus:
      return AstExprOp_Sub;
    case Token_Star:
      return AstExprOp_Mul;
    case Token_Slash:
      return AstExprOp_Div;
    case Token_Ampersand:
      return AstExprOp_BitAnd;
    case Token_Pipe:
      return AstExprOp_BitOr;
    case Token_Circumflex:
      return AstExprOp_BitXor;
    case Token_TwoAngleOpen:
      return AstExprOp_BitShiftLeft;
    case Token_TwoAngleClose:
      return AstExprOp_BitShiftRight;
    case Token_ThreeAmpersand:
      return AstExprOp_Mask;
    default: return AstExprOp_NONE_;
  }
}

internal struct Ast*
build_expression(int priority_threshold)
{
  struct Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == Token_DotPrefix) {
        next_token();
        struct Ast_MemberSelectExpr* select_expr = new_ast_node(Ast_MemberSelectExpr, token);
        select_expr->expr = expr;
        expr = (struct Ast*)select_expr;
        if (token_is_name(token)) {
          select_expr->member_name = build_name(false);
        } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_BracketOpen) {
        next_token();
        struct Ast_IndexedArrayExpr* index_expr = new_ast_node(Ast_IndexedArrayExpr, token);
        index_expr->index = expr;
        index_expr->colon_index = build_arrayIndex();
        expr = (struct Ast*)index_expr;
        if (token->klass == Token_BracketClose) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_ParenthOpen) {
        next_token();
        struct Ast_FunctionCallExpr* call_expr = new_ast_node(Ast_FunctionCallExpr, token);
        call_expr->expr = expr;
        call_expr->args = build_argumentList();
        expr = (struct Ast*)call_expr;
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_AngleOpen && token_is_realTypeArg(peek_token())) {
        next_token();
        expr->type_args = build_realTypeArgumentList();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token->klass == Token_Equal) {
        next_token();
        struct Ast_KvPair* kv_pair = new_ast_node(Ast_KvPair, token);
        kv_pair->name = expr;
        kv_pair->expr = build_expression(1);
        expr = (struct Ast*)kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          struct Ast_BinaryExpr* bin_expr = new_ast_node(Ast_BinaryExpr, token);
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

struct Ast*
build_ast_program(struct Ast** p4program_, int* ast_node_count_, struct UnboundedArray* tokens_array_,
              struct Arena* ast_storage_)
{
  tokens_array = tokens_array_;
  ast_storage = ast_storage_;

  ast_attr_set_storage(ast_storage);

  token_at = 0;
  token = array_get(tokens_array, token_at);
  next_token();
  push_scope();
  struct Ast* p4program = build_p4program();
  pop_scope();
  return p4program;
}
