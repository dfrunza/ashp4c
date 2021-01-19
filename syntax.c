#include "dp4c.h"
#include "lex.h"
#include "syntax.h"

external Arena arena;

external struct Token* tokenized_input;
internal struct Token* token = 0;
internal struct Token* prev_token = 0;
external int tokenized_input_len;

external struct Namespace_Entry** symtable;
external int max_symtable_len;
external int scope_level;

external Ast_P4Program* p4program;

Ast_TypeIdent* error_type_ast = 0;
Ast_TypeIdent* void_type_ast = 0;
Ast_TypeIdent* bool_type_ast = 0;
Ast_TypeIdent* bit_type_ast = 0;
Ast_TypeIdent* varbit_type_ast = 0;
Ast_TypeIdent* int_type_ast = 0;
Ast_TypeIdent* string_type_ast = 0;
Ast_Integer* bool_true_ast = 0;
Ast_Integer* bool_false_ast = 0;

internal struct Ast* build_expression(int priority_threshold);
internal struct Ast* build_typeRef();
internal struct Ast* build_blockStatement();
internal struct Ast* build_statement();
internal struct Ast* build_parserStatement();

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

internal int
scope_push_level()
{
  int new_scope_level = ++scope_level;
  printf("push scope %d\n", new_scope_level);
  return new_scope_level;
}

internal int
scope_pop_level(int to_level)
{
  if (scope_level <= to_level)
    return scope_level;

  int i = 0;
  while (i < max_symtable_len) {
    struct Namespace_Entry* ns = symtable[i];
    while (ns) {
      struct Ident* ident = ns->ns_global;
      if (ident && ident->scope_level > to_level) {
        ns->ns_global = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= to_level);
        ident->next_in_scope = 0;
      }
      ident = ns->ns_type;
      if (ident && ident->scope_level > to_level) {
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

internal bool
ident_is_declared(struct Ident* ident)
{
  bool is_declared = (ident && ident->scope_level >= scope_level);
  return is_declared;
}

internal struct Namespace_Entry*
get_namespace(char* name)
{
  uint32_t h = name_hash(name);
  struct Namespace_Entry* name_info = symtable[h];
  while(name_info) {
    if (cstr_match(name_info->name, name))
      break;
    name_info = name_info->next;
  }
  if (!name_info) {
    name_info = arena_push_struct(&arena, Namespace_Entry);
    name_info->name = name;
    name_info->next = symtable[h];
    symtable[h] = name_info;
  }
  return name_info;
}

internal struct Ident*
lookup_ident(char* name)
{
  struct Namespace_Entry* ns = get_namespace(name);
  struct Ident* ident_var = (struct Ident*)ns->ns_global;
  if (ident_var)
    assert (ident_var->ident_kind == ID_IDENT);
  return ident_var;
}

internal struct Ident*
new_ident(char* name)
{
  struct Ident* ident = 0;
  struct Namespace_Entry* ns = get_namespace(name);
  ident = (struct Ident*)ns->ns_global;
  if (ident_is_declared(ident)) {
    error("redeclaration of ident");
  } else {
    struct Ident* ident = arena_push_struct(&arena, Ident);
    ident->name = name;
    ident->scope_level = scope_level;
    ident->ident_kind = ID_IDENT;
    ident->next_in_scope = ns->ns_global;
    ns->ns_global = (struct Ident*)ident;
    printf("new ident '%s'\n", ident->name);
  }
  return ident;
}

struct Ident*
lookup_type(char* name)
{
  struct Namespace_Entry* ns = get_namespace(name);
  struct Ident* result = (struct Ident*)ns->ns_type;
  if (result)
    assert(result->ident_kind == ID_TYPE);
  return result;
}

struct Ident*
new_type(char* name)
{
  struct Ident* ident = 0;
  struct Namespace_Entry* ns = get_namespace(name);
  if (ident_is_declared(ident)) {
    error("redeclaration of type");
  } else {
    struct Ident* ident = arena_push_struct(&arena, Ident);
    ident->name = name;
    ident->scope_level = scope_level;
    ident->ident_kind = ID_TYPE;
    ident->next_in_scope = ns->ns_type;
    ns->ns_type = (struct Ident*)ident;
    printf("new type '%s'\n", ident->name);
  }
  return ident;
}

internal struct Ident_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  struct Namespace_Entry* namespace = get_namespace(name);
  assert (namespace->ns_global == 0);
  struct Ident_Keyword* ident = arena_push_struct(&arena, Ident_Keyword);
  ident->name = name;
  ident->scope_level = scope_level;
  ident->token_klass = token_klass;
  ident->ident_kind = ID_KEYWORD;
  namespace->ns_global = (struct Ident*)ident;
  return ident;
}

internal struct Token*
next_token()
{
  assert(token < tokenized_input + tokenized_input_len);
  prev_token = token++;
  while (token->klass == TOK_COMMENT)
    token++;

  if (token->klass == TOK_IDENTIFIER) {
    struct Namespace_Entry* ns = get_namespace(token->lexeme);
    if (ns->ns_global) {
      struct Ident* ident = ns->ns_global;
      if (ident->ident_kind == ID_KEYWORD) {
        token->klass = ((struct Ident_Keyword*)ident)->token_klass;
        return token;
      }
    }

    if (ns->ns_type) {
      struct Ident* ident = ns->ns_type;
      if (ident->ident_kind == ID_TYPE) {
        token->klass = TOK_TYPE_IDENTIFIER;
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
  struct Token* peek_token = next_token();
  token = prev_token;
  return peek_token;
}

#define new_ast_node(TYPE) ({ \
  struct TYPE* node = arena_push(&arena, sizeof(struct TYPE)); \
  *node = (struct TYPE){}; \
  node->kind = TYPE; \
  node->line_nr = token->line_nr; \
  node; })

internal bool
token_is_typeName(struct Token* token)
{
  return token->klass == TOK_TYPE_IDENTIFIER;
}

internal bool
token_is_prefixedType(struct Token* token)
{
  return token->klass == TOK_TYPE_IDENTIFIER;
}

internal bool
token_is_baseType(struct Token* token)
{
  bool result = token->klass == TOK_BOOL || token->klass == TOK_ERROR || token->klass == TOK_INT
    || token->klass == TOK_BIT || token->klass == TOK_VARBIT;
  return result;
}

internal bool
token_is_typeRef(struct Token* token)
{
  bool result = token_is_baseType(token) || token->klass == TOK_TYPE_IDENTIFIER || token->klass == TOK_TUPLE;
  return result;
}

internal bool
token_is_direction(struct Token* token)
{
  bool result = token->klass == TOK_IN || token->klass == TOK_OUT || token->klass == TOK_INOUT;
  return result;
}

internal bool
token_is_parameter(struct Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

internal bool
token_is_functionPrototype()
{
  bool result = token_is_typeRef(token) || token->klass == TOK_VOID || token->klass == TOK_IDENTIFIER;
  return result;
}

internal bool
token_is_derivedTypeDeclaration(struct Token* token)
{
  bool result = token->klass == TOK_HEADER || token->klass == TOK_HEADER_UNION || token->klass == TOK_STRUCT
    || token->klass == TOK_ENUM;
  return result;
}

internal bool
token_is_typeDeclaration(struct Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TOK_TYPEDEF || token->klass == TOK_TYPE
    || token->klass == TOK_PARSER || token->klass == TOK_CONTROL || token->klass == TOK_PACKAGE;
  return result;
}

internal bool
token_is_nonTypeName(struct Token* token)
{
  bool result = token->klass == TOK_IDENTIFIER || token->klass == TOK_APPLY || token->klass == TOK_KEY
    || token->klass == TOK_ACTIONS || token->klass == TOK_STATE || token->klass == TOK_ENTRIES || token->klass == TOK_TYPE;
  return result;
}

internal bool
token_is_name(struct Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TOK_TYPE_IDENTIFIER;
  return result;
}

internal bool
token_is_nonTableKwName(struct Token* token)
{
  bool result = token->klass == TOK_IDENTIFIER || token->klass == TOK_TYPE_IDENTIFIER
    || token->klass == TOK_APPLY || token->klass == TOK_STATE || token->klass == TOK_TYPE;
  return result;
}

internal bool
token_is_typeArg(struct Token* token)
{
  bool result = token->klass == TOK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
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
  bool result = token_is_typeRef(token) || token->klass == TOK_VOID || token->klass == TOK_IDENTIFIER;
  return result;
}

internal bool
token_is_expressionPrimary(struct Token* token)
{
  bool result = token->klass == TOK_INTEGER || token->klass == TOK_TRUE || token->klass == TOK_FALSE
    || token->klass == TOK_STRING_LITERAL || token->klass == TOK_UNARY_DOTPREFIX || token_is_nonTypeName(token)
    || token->klass == TOK_BRACE_OPEN || token->klass == TOK_PARENTH_OPEN || token->klass == TOK_LOGIC_NOT
    || token->klass == TOK_UNARY_MINUS || token_is_typeName(token) || token->klass == TOK_ERROR
    || token_is_prefixedType(token);
  return result;
}

internal bool
token_is_expression(struct Token* token)
{
  return token_is_expressionPrimary(token);
}

internal struct Ast_NonTypeName*
build_nonTypeName()
{
  struct Ast_NonTypeName* name = 0;
  if (token_is_nonTypeName(token)) {
    name = new_ast_node(Ast_NonTypeName);
    name->name = token->lexeme;
    next_token();
  } else error("");
  return name;
}

internal struct Ast_Name*
build_name()
{
  struct Ast_Name* name = 0;
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      name = (struct Ast_Name*)build_nonTypeName();
    } else if (token->klass == TOK_TYPE_IDENTIFIER) {
      struct Ast_TypeName* type_name = new_ast_node(Ast_TypeName);
      type_name->name = token->lexeme;
      name = (struct Ast_Name*)type_name;
      next_token();
    } else assert(false);
  } else error("");
  return name;
}

internal struct Ast*
build_typeParameterList()
{
  if (token_is_typeParameterList(token)) {
    struct Ast_Name* name = build_name();
    if (name->kind == Ast_NonTypeName)
      new_type(((struct Ast_NonTypeName*)name)->name);
    while (token->klass == TOK_COMMA) {
      next_token();
      build_name();
    }
  } else error("");
  return 0;
}

internal struct Ast*
build_optTypeParameters()
{
  if (token->klass == TOK_ANGLE_OPEN) {
    next_token();
    if (token_is_typeParameterList(token)) {
      build_typeParameterList();
      if (token->klass == TOK_ANGLE_CLOSE) {
        next_token();
      } else error("");
    } else error("");
  }
  return 0;
}

internal struct Ast*
build_typeArg()
{
  if (token_is_typeArg(token))
  {
    if (token->klass == TOK_DONTCARE) {
      next_token();
    } else if (token_is_typeRef(token)) {
      build_typeRef();
    } else if (token_is_nonTypeName(token)) {
      build_nonTypeName();
    } else assert(false);
  } else error("");
  return 0;
}

internal bool
token_is_methodPrototype(struct Token* token)
{
  return token_is_functionPrototype() | token->klass == TOK_TYPE_IDENTIFIER;
}

internal struct Ast*
build_direction()
{
  if (token_is_direction(token)) {
    next_token();
  }
  return 0;
}

internal struct Ast*
build_parameter()
{
  build_direction();
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == TOK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          build_expression(1);
        } else error("");
      }
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_parameterList()
{
  if (token_is_parameter(token)) {
    build_parameter();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_parameter();
    }
  }
  return 0;
}

internal struct Ast*
build_typeOrVoid()
{
  struct Ast* type = 0;
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      type = build_typeRef();
    } else if (token->klass == TOK_VOID) {
      struct Ast_TypeName* void_name = new_ast_node(Ast_TypeName);
      void_name->name = token->lexeme;
      type = (struct Ast*)void_name;
      next_token();
    } else if (token->klass == TOK_IDENTIFIER) {
      struct Ast_NonTypeName* name = new_ast_node(Ast_NonTypeName);
      name->name = token->lexeme;
      type = (struct Ast*)name;
      next_token();
    } else error("");
  } else error("");
  return type;
}

internal struct Ast*
build_functionPrototype()
{
  if (token_is_typeOrVoid(token)) {
    scope_push_level();
    struct Ast* return_type = build_typeOrVoid();
    if (return_type->kind == Ast_NonTypeName)
      new_type(((struct Ast_NonTypeName*)return_type)->name);
    if (token_is_name(token)) {
      struct Ast_Name* function_name = build_name();
      new_ident(function_name->name);
      build_optTypeParameters();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        if (token_is_parameter(token)) {
          build_parameterList();
          if (token->klass == TOK_PARENTH_CLOSE) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    } else error("");
    scope_pop_level(scope_level-1);
  } else error("");
  return 0;
}

internal struct Ast*
build_methodPrototype()
{
  if (token_is_methodPrototype(token)) {
    if (token->klass == TOK_TYPE_IDENTIFIER && peek_token()->klass == TOK_PARENTH_OPEN) {
      next_token();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        build_parameterList();
        if (token->klass == TOK_PARENTH_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else if (token_is_functionPrototype(token)) {
      build_functionPrototype();
    } else error("");

    if (token->klass == TOK_SEMICOLON) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_methodPrototypes()
{
  while (token_is_methodPrototype(token))
    build_methodPrototype();
  return 0;
}

internal Ast_Declaration*
build_externDeclaration()
{
  if (token->klass == TOK_EXTERN) {
    next_token();
    if (token_is_nonTypeName(token)) {
      struct Ast_NonTypeName* decl_name = build_nonTypeName();
      if (decl_name->kind == Ast_NonTypeName)
        new_type(((struct Ast_NonTypeName*)decl_name)->name);
      build_optTypeParameters();
      if (token->klass == TOK_BRACE_OPEN) {
        scope_push_level();
        next_token();
        if (token_is_methodPrototype(token)) {
          build_methodPrototypes();
          if (token->klass == TOK_BRACE_CLOSE) {
            next_token();
          } else error("");
        } else error("");
        scope_pop_level(scope_level-1);
      } else error("");
    } else if (token_is_functionPrototype(token)) {
      build_functionPrototype();
      if (token->klass == TOK_SEMICOLON) {
        next_token();
      } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_integerTypeSize()
{
  if (token->klass == TOK_INTEGER) {
    next_token();
  } else if (token->klass == TOK_PARENTH_OPEN) {
    build_expression(1);
  } else error("");
  return 0;
}

internal struct Ast*
build_baseType()
{
  struct Ast_BaseType* base_type = 0;
  if (token_is_baseType(token)) {
    base_type = new_ast_node(Ast_BaseType);
    if (token->klass == TOK_BOOL) {
      base_type->base_type = BASETYPE_BOOL;
      next_token();
    } else if (token->klass == TOK_ERROR) {
      base_type->base_type = BASETYPE_ERROR;
      next_token();
    } else if (token->klass == TOK_INT) {
      base_type->base_type = BASETYPE_INT;
      next_token();
      if (token->klass == TOK_ANGLE_OPEN) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == TOK_ANGLE_CLOSE) {
          next_token();
        } else error("");
      }
    } else if (token->klass == TOK_BIT) {
      base_type->base_type = BASETYPE_BIT;
      next_token();
      if (token->klass == TOK_ANGLE_OPEN) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == TOK_ANGLE_CLOSE) {
          next_token();
        } else error("");
      }
    } else if (token->klass == TOK_VARBIT) {
      base_type->base_type = BASETYPE_VARBIT;
      next_token();
      if (token->klass == TOK_ANGLE_OPEN) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == TOK_ANGLE_CLOSE) {
          next_token();
        } else error("");
      }
    } else assert(false);
  }
  return (struct Ast*)base_type;
}

internal struct Ast*
build_typeArgumentList()
{
  if (token_is_typeArg(token)) {
    build_typeArg();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_typeArg();
    }
  }
  return 0;
}

internal struct Ast*
build_tupleType()
{
  if (token->klass == TOK_TUPLE) {
    next_token();
    if (token->klass == TOK_ANGLE_OPEN) {
      next_token();
      build_typeArgumentList();
      if (token->klass == TOK_ANGLE_CLOSE) {
        next_token();
      } else error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: '<' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_headerStackType()
{
  if (token->klass == TOK_BRACKET_OPEN) {
    next_token();
    if (token_is_expression(token)) {
      build_expression(1);
      if (token->klass != TOK_BRACKET_CLOSE) {
        next_token();
      } else error("at line %d: ']' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: expression expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_specializedType()
{
  if (token->klass == TOK_ANGLE_OPEN) {
    next_token();
    build_typeArgumentList();
    if (token->klass == TOK_ANGLE_CLOSE) {
      next_token();
    } else error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_prefixedType()
{
  struct Ast_TypeName* name = 0;
  if (token->klass == TOK_TYPE_IDENTIFIER) {
    name = new_ast_node(Ast_TypeName);
    name->name = token->lexeme;
    next_token();
    if (token->klass == TOK_DOTPREFIX) {
      next_token();
      if (token->klass == TOK_TYPE_IDENTIFIER) {
        struct Ast_PrefixedType* pfx_type = new_ast_node(Ast_PrefixedType);
        pfx_type->first_name = name;
        pfx_type->second_name = new_ast_node(Ast_TypeName);
        pfx_type->second_name->name = token->lexeme;
        next_token();
      } else error("");
    }
  } else error("");
  return (struct Ast*)name;
}

internal struct Ast*
build_typeName()
{
  struct Ast* name = 0;
  if (token->klass == TOK_TYPE_IDENTIFIER) {
    name = build_prefixedType();
    if (token->klass == TOK_ANGLE_OPEN) {
      name = build_specializedType();
    } if (token->klass == TOK_BRACKET_OPEN) {
      name = build_headerStackType();
    }
  } else error("");
  return name;
}

internal struct Ast*
build_typeRef()
{
  struct Ast* ref = 0;
  if (token_is_typeRef(token)) {
    if (token_is_baseType(token)) {
      ref = build_baseType();
    } else if (token->klass == TOK_TYPE_IDENTIFIER) {
      /* <typeName> | <specializedType> | <headerStackType> */
      ref = build_typeName();
    } else if (token->klass == TOK_TUPLE) {
      ref = build_tupleType();
    } else assert(false);
  } else error("");
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
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      new_ident(name->name);
      if (token->klass == TOK_SEMICOLON) {
        next_token();
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_structFieldList()
{
  while (token_is_structField(token)) {
    build_structField();
    if (token->klass == TOK_COMMA) {
      next_token();
      build_structField();
    }
  }
  return 0;
}

internal struct Ast*
build_headerTypeDeclaration()
{
  if (token->klass == TOK_HEADER) {
    next_token();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      if (name->kind == Ast_NonTypeName)
        new_type(((struct Ast_NonTypeName*)name)->name);
      if (token->klass == TOK_BRACE_OPEN) {
        scope_push_level();
        next_token();
        build_structFieldList();
        if (token->klass == TOK_BRACE_CLOSE) {
          next_token(token);
        } else error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
        scope_pop_level(scope_level-1);
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_headerUnionDeclaration()
{
  if (token->klass == TOK_HEADER_UNION) {
    next_token();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == TOK_BRACE_OPEN) {
        next_token();
        build_structFieldList();
        if (token->klass == TOK_BRACE_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_structTypeDeclaration()
{
  if (token->klass == TOK_STRUCT) {
    next_token();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      if (name->kind == Ast_NonTypeName)
        new_type(((struct Ast_NonTypeName*)name)->name);
      if (token->klass == TOK_BRACE_OPEN) {
        next_token();
        build_structFieldList();
        if (token->klass == TOK_BRACE_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
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
  if (token->klass == TOK_EQUAL) {
    next_token();
    build_initializer();
  }
  return 0;
}

internal struct Ast*
build_specifiedIdentifier()
{
  if (token_is_specifiedIdentifier(token)) {
    build_name();
    if (token->klass == TOK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        build_initializer();
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_specifiedIdentifierList()
{
  if (token_is_specifiedIdentifier(token)) {
    build_specifiedIdentifier();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_specifiedIdentifier();
    }
  }
  return 0;
}

internal struct Ast*
build_enumDeclaration()
{
  if (token->klass == TOK_ENUM) {
    next_token();
    if (token->klass == TOK_BIT) {
      if (token->klass == TOK_ANGLE_OPEN) {
        next_token();
        if (token->klass == TOK_INTEGER) {
          next_token();
          if (token->klass == TOK_ANGLE_CLOSE) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    }
    if (token_is_name) {
      build_name();
      if (token->klass == TOK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          build_specifiedIdentifierList();
          if (token->klass == TOK_BRACE_CLOSE) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    if (token->klass == TOK_HEADER) {
      build_headerTypeDeclaration();
    } else if (token->klass == TOK_HEADER_UNION) {
      build_headerUnionDeclaration();
    } else if (token->klass == TOK_STRUCT) {
      build_structTypeDeclaration();
    } else if (token->klass == TOK_ENUM) {
      build_enumDeclaration();
    } else assert(false);
  } else error("");
  return 0;
}

internal struct Ast*
build_parserTypeDeclaration()
{
  if (token->klass == TOK_PARSER) {
    next_token();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      if (name->kind == Ast_NonTypeName)
        new_type(((struct Ast_NonTypeName*)name)->name);
      build_optTypeParameters();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        build_parameterList();
        if (token->klass == TOK_PARENTH_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_optConstructorParameters()
{
  if (token->klass == TOK_PARENTH_OPEN) {
    next_token();
    build_parameterList();
    if (token->klass == TOK_PARENTH_CLOSE) {
      next_token();
    } else error("");
  }
  return 0;
}

internal struct Ast*
build_constantDeclaration()
{
  if (token->klass == TOK_CONST) {
    next_token();
    if (token_is_typeRef(token)) {
      build_typeRef();
      if (token_is_name(token)) {
        build_name();
        if (token->klass == TOK_EQUAL) {
          next_token();
          if (token_is_expression(token)) {
            build_expression(1);
            if (token->klass == TOK_SEMICOLON) {
              next_token();
            } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal bool
token_is_declaration(struct Token* token)
{
  bool result = token->klass == TOK_CONST || token->klass == TOK_EXTERN || token->klass == TOK_ACTION
    || token->klass == TOK_PARSER || token_is_typeDeclaration(token) || token->klass == TOK_CONTROL
    || token_is_typeRef(token) || token->klass == TOK_ERROR || token->klass == TOK_MATCH_KIND
    || token_is_typeOrVoid(token);
  return result;
}

internal bool
token_is_lvalue(struct Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == TOK_DOTPREFIX;
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(struct Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TOK_PARENTH_OPEN || token->klass == TOK_ANGLE_OPEN
    || token->klass == TOK_EQUAL;
  return result;
}

internal bool
token_is_statement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TOK_IF
    || token->klass == TOK_SEMICOLON || token->klass == TOK_BRACE_OPEN || token->klass == TOK_EXIT
    || token->klass == TOK_RETURN;
  return result;
}

internal bool
token_is_statementOrDeclaration(struct Token* token)
{
  bool result = token->klass == TOK_VAR || token->klass == TOK_CONST || token_is_statement(token);
  return result;
}

internal bool
token_is_argument(struct Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TOK_DONTCARE;
  return result;
}

internal bool
token_is_parserLocalElement(struct Token* token)
{
  bool result = token->klass == TOK_CONST || token->klass == TOK_VAR || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TOK_BRACE_OPEN || token->klass == TOK_CONST || token->klass == TOK_VAR
    || token->klass == TOK_SEMICOLON;
  return result;
}

internal bool
token_is_simpleKeysetExpression(struct Token* token) {
  bool result = token_is_expression(token) || token->klass == TOK_DEFAULT || token->klass == TOK_DONTCARE;
  return result;
}

internal bool
token_is_keysetExpression(struct Token* token)
{
  bool result = token->klass == TOK_TUPLE || token_is_simpleKeysetExpression(token);
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
  bool result = token->klass == TOK_CONST || token->klass == TOK_ACTION
    || token_is_typeRef(token) || token->klass == TOK_VAR;
  return result;
}

internal struct Ast*
build_argument()
{
  assert(token_is_argument(token));
  if (token_is_expression(token)) {
    build_expression(1);
  } else if (token_is_name(token)) {
    build_name();
    if (token->klass == TOK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        build_expression(1);
      } else error("");
    } else error("");
  } else if (token->klass == TOK_DONTCARE) {
    next_token();
  } else error("");
  return 0;
}

internal struct Ast*
build_argumentList()
{
  if (token_is_argument(token)) {
    build_argument();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_argument();
    }
  }
  return 0;
}

internal struct Ast*
build_variableDeclaration()
{
  if (token->klass == TOK_VAR) {
    next_token();
    if (token_is_typeRef(token)) {
      build_typeRef();
      if (token_is_name(token)) {
        build_name();
        build_optInitializer();
        if (token->klass == TOK_SEMICOLON) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_instantiation()
{
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token->klass == TOK_PARENTH_OPEN) {
      next_token();
      build_argumentList();
      if (token->klass == TOK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          build_name();
          if (token->klass == TOK_SEMICOLON) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_parserLocalElement()
{
  if (token_is_parserLocalElement(token)) {
    if (token->klass == TOK_CONST) {
      build_constantDeclaration();
    } else if (token->klass == TOK_VAR) {
      build_variableDeclaration();
    } else if (token_is_typeRef(token)) {
      build_instantiation();
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_parserLocalElements()
{
  while (token_is_parserLocalElement(token)) {
    build_parserLocalElement();
  }
  return 0;
}

internal struct Ast*
build_directApplication()
{
  if (token_is_typeName(token)) {
    build_typeName();
    if (token->klass == TOK_DOTPREFIX) {
      next_token();
      if (token->klass == TOK_APPLY) {
        next_token();
        if (token->klass == TOK_PARENTH_OPEN) {
          next_token();
          build_argumentList();
          if (token->klass == TOK_PARENTH_CLOSE) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_prefixedNonTypeName()
{
  struct Ast* name = 0;
  if (token->klass == TOK_DOTPREFIX) {
    struct Ast_DotPrefix* dot_prefix = new_ast_node(Ast_DotPrefix);
    name = (struct Ast*)dot_prefix;
    next_token();
  }
  if (token_is_nonTypeName) {
    name = (struct Ast*)build_nonTypeName();
  } else error("");
  return name;
}

internal struct Ast*
build_lvalue()
{
  if (token_is_lvalue(token)) {
    build_prefixedNonTypeName();
    if (token->klass == TOK_DOTPREFIX) {
      next_token();
      build_name();
    }
    if (token->klass == TOK_BRACKET_OPEN) {
      next_token();
      if (token_is_expression(token)) {
        build_expression(1);
        if (token->klass == TOK_COLON) {
          next_token();
          if (token_is_expression(token)) {
            build_expression(1);
          } else error("");
        }
        if (token->klass == TOK_BRACKET_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    }
  } else error("");
  return 0;
}

internal struct Ast*
build_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    build_lvalue();
    if (token->klass == TOK_ANGLE_OPEN) {
      next_token();
      build_typeArgumentList();
      if (token->klass == TOK_ANGLE_CLOSE) {
        next_token();
      } else error("");
    }

    if (token->klass == TOK_PARENTH_OPEN) {
      next_token();
      build_argumentList();
      if (token->klass == TOK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TOK_SEMICOLON) {
          next_token();
        } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
      } else error("");
    } else error("");
  }
  return 0;
}

internal struct Ast*
build_parserStatements()
{
  while (token_is_parserStatement(token)) {
    build_parserStatement();
  }
  return 0;
}

internal struct Ast*
build_parserBlockStatements()
{
  if (token->klass == TOK_BRACE_OPEN) {
    next_token();
    build_parserStatements();
    if (token->klass == TOK_BRACE_CLOSE) {
      next_token();
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_parserStatement()
{
  if (token_is_assignmentOrMethodCallStatement(token)) {
    build_assignmentOrMethodCallStatement();
  } else if (token_is_typeName(token)) {
    build_directApplication();
  } else if (token->klass == TOK_BRACE_OPEN) {
    build_parserBlockStatements();
  } else if (token->klass == TOK_CONST) {
    build_constantDeclaration();
  } else if (token->klass == TOK_VAR) {
    build_variableDeclaration();
  } else if (token->klass == TOK_SEMICOLON) {
    ; // <emptyStatement>
  } else error("");
  return 0;
}

internal struct Ast*
build_expressionList()
{
  if (token_is_expression(token)) {
    build_expression(1);
    while (token->klass == TOK_COMMA) {
      build_expression(1);
    }
  }
  return 0;
}

internal struct Ast*
build_simpleKeysetExpression()
{
  struct Ast* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expression(1);
  } else if (token->klass == TOK_DEFAULT) {
    next_token();
  } else if (token->klass == TOK_DONTCARE) {
    next_token();
  } else error("");
  return expr;
}

internal struct Ast*
build_tupleKeysetExpression()
{
  struct Ast* expr = 0;
  if (token->klass == TOK_PARENTH_OPEN) {
    next_token();
    build_simpleKeysetExpression();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_simpleKeysetExpression();
    }
    if (token->klass == TOK_PARENTH_CLOSE) {
      next_token();
    } else error("");
  } else error("");
  return expr;
}

internal struct Ast*
build_keysetExpression()
{
  struct Ast* expr = 0;
  if (token->klass == TOK_PARENTH_OPEN) {
    build_tupleKeysetExpression();
  } else if (token_is_simpleKeysetExpression(token)) {
    build_simpleKeysetExpression();
  } else error("");
  return expr;
}

internal struct Ast*
build_selectCase()
{
  if (token_is_keysetExpression(token)) {
    build_keysetExpression();
    if (token->klass == TOK_COLON) {
      next_token();
      if (token_is_name(token)) {
        build_name();
        if (token->klass == TOK_SEMICOLON) {
          next_token();
        } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_selectCaseList()
{
  while (token_is_selectCase(token)) {
    build_selectCase();
  }
  return 0;
}

internal struct Ast*
build_selectExpression()
{
  if (token->klass == TOK_SELECT) {
    next_token();
    if (token->klass == TOK_PARENTH_OPEN) {
      next_token();
      build_expressionList();
      if (token->klass == TOK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TOK_BRACE_OPEN) {
          scope_push_level();
          next_token();
          build_selectCaseList();
          if (token->klass == TOK_BRACE_CLOSE) {
            next_token();
          } else error("");
          scope_pop_level(scope_level-1);
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_stateExpression()
{
  if (token_is_name(token)) {
    build_name();
    if (token->klass == TOK_SEMICOLON) {
      next_token();
    } else error("");
  } else if (token->klass == TOK_SELECT) {
    build_selectExpression();
  }
  else error("");
  return 0;
}

internal struct Ast*
build_transitionStatement()
{
  if (token->klass == TOK_TRANSITION) {
    next_token();
    build_stateExpression();
  } else error("");
  return 0;
}

internal struct Ast*
build_parserState()
{
  if (token->klass == TOK_STATE) {
    next_token();
    struct Ast_Name* name = build_name();
    new_ident(name->name);
    if (token->klass == TOK_BRACE_OPEN) {
      scope_push_level();
      next_token();
      build_parserStatements();
      build_transitionStatement();
      if (token->klass == TOK_BRACE_CLOSE) {
        next_token();
      } else error("");
      scope_pop_level(scope_level-1);
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_parserStates()
{
  if (token->klass == TOK_STATE) {
    build_parserState();
    while (token->klass == TOK_STATE) {
      build_parserState();
    }
  } else error("");
  return 0;
}

internal struct Ast*
build_parserDeclaration()
{
  assert(token->klass == TOK_PARSER);
  build_parserTypeDeclaration();
  if (token->klass == TOK_SEMICOLON) {
    next_token(); /* <parserTypeDeclaration> */
  } else {
    build_optConstructorParameters();
    if (token->klass == TOK_BRACE_OPEN) {
      scope_push_level();
      next_token();
      build_parserLocalElements();
      build_parserStates();
      if (token->klass == TOK_BRACE_CLOSE) {
        next_token();
      } else error("");
      scope_pop_level(scope_level-1);
    } else error("");
  }
  return 0;
}

internal struct Ast*
build_controlTypeDeclaration()
{
  if (token->klass == TOK_CONTROL) {
    next_token();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      if (name->kind == Ast_NonTypeName)
        new_type(((struct Ast_NonTypeName*)name)->name);
      build_optTypeParameters();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        if (token_is_parameter(token)) {
          build_parameterList();
          if (token->klass == TOK_PARENTH_CLOSE) {
            next_token();
          } else error("");
        }
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_actionDeclaration()
{
  if (token->klass == TOK_ACTION) {
    next_token();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == TOK_PARENTH_OPEN) {
        build_parameterList();
        if (token->klass == TOK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TOK_BRACE_OPEN) {
            build_blockStatement();
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_controlLocalDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == TOK_CONST) {
    decl = build_constantDeclaration();
  } else if (token->klass == TOK_ACTION) {
    decl = build_actionDeclaration();
  } else if (token_is_typeRef(token)) {
    decl = build_instantiation();
  } else if (token->klass == TOK_VAR) {
    decl = build_variableDeclaration();
  } else error("");
  return decl;
}

internal struct Ast*
build_controlDeclaration()
{
  if (token->klass == TOK_CONTROL) {
    build_controlTypeDeclaration();
    if (token->klass == TOK_SEMICOLON) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      build_optConstructorParameters();
      if (token->klass == TOK_BRACE_OPEN) {
        next_token();
        while (token_is_controlLocalDeclaration(token)) {
          build_controlLocalDeclaration();
        }
        if (token->klass == TOK_APPLY) {
          next_token();
          build_blockStatement();
          if (token->klass == TOK_BRACE_CLOSE) {
            next_token();
          } else error("");
        } else error("");
      } else error("");
    }
  } else error("");
  return 0;
}

internal struct Ast*
build_packageTypeDeclaration()
{
  if (token->klass == TOK_PACKAGE) {
    next_token();
    if (token_is_name(token)) {
      struct Ast_Name* name = build_name();
      if (name->kind == Ast_NonTypeName)
        new_type(name->name);
      build_optTypeParameters();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        build_parameterList();
        if (token->klass == TOK_PARENTH_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_typedefDeclaration()
{
  if (token->klass == TOK_TYPEDEF || token->klass == TOK_TYPE) {
    if (token->klass == TOK_TYPEDEF) {
      next_token();
    } else if (token->klass == TOK_TYPE) {
      next_token();
    } else assert(false);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      if (token_is_typeRef(token)) {
        build_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        build_derivedTypeDeclaration();
      } else assert(false);

      if (token_is_name(token)) {
        struct Ast_Name* name = build_name();
        if (name->kind == Ast_NonTypeName)
          new_type(((struct Ast_NonTypeName*)name)->name);
        if (token->klass == TOK_SEMICOLON) {
          next_token();
        } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_typeDeclaration()
{
  if (token_is_typeDeclaration(token)) {
    if (token_is_derivedTypeDeclaration(token)) {
      build_derivedTypeDeclaration();
    } else if (token->klass == TOK_TYPEDEF || token->klass == TOK_TYPE) {
      build_typedefDeclaration();
    } else if (token->klass == TOK_PARSER) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      build_parserDeclaration();
    } else if (token->klass == TOK_CONTROL) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      build_controlDeclaration();
    } else if (token->klass == TOK_PACKAGE) {
      build_packageTypeDeclaration();
      if (token->klass == TOK_SEMICOLON) {
        next_token();
      } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_conditionalStatement()
{
  if (token->klass == TOK_IF) {
    next_token();
    if (token->klass == TOK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(token)) {
        build_expression(1);
        if (token->klass == TOK_PARENTH_CLOSE) {
          next_token();
          if (token_is_statement(token)) {
            build_statement();
            if (token->klass == TOK_ELSE) {
              next_token();
              if (token_is_statement(token))
                build_statement();
              else error("");
            }
          } else error("");
        } else error("");
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_exitStatement()
{
  if (token->klass == TOK_EXIT) {
    next_token();
    if (token->klass == TOK_SEMICOLON) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_returnStatement()
{
  if (token->klass == TOK_RETURN) {
    next_token();
    if (token_is_expression(token))
      build_expression(1);
    if (token->klass == TOK_SEMICOLON) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("");
  return 0;
}

internal struct Ast*
build_statement()
{
  if (token_is_assignmentOrMethodCallStatement(token))
    build_assignmentOrMethodCallStatement();
  else if (token_is_typeName(token))
    build_directApplication();
  else if (token->klass == TOK_IF)
    build_conditionalStatement();
  else if (token->klass == TOK_SEMICOLON)
    ; // empty statement
  else if (token->klass == TOK_BRACE_OPEN)
    build_blockStatement();
  else if (token->klass == TOK_EXIT)
    build_exitStatement();
  else if (token->klass == TOK_RETURN)
    build_returnStatement();
  else error("");
  return 0;
}

internal struct Ast*
build_statementOrDeclList()
{
  if (token_is_statementOrDeclaration(token)) {
    if (token->klass == TOK_VAR) {
      build_variableDeclaration();
    } else if (token_is_typeRef(token) && peek_token()->klass == TOK_PARENTH_OPEN) {
      build_instantiation();
    } else if (token_is_statement(token)) {
      build_statement();
    } else if (token->klass == TOK_CONST)
      build_constantDeclaration();
    else assert(false);
  }
  return 0;
}

internal struct Ast*
build_blockStatement()
{
  if (token->klass == TOK_BRACE_OPEN) {
    scope_push_level();
    next_token();
    build_statementOrDeclList();
    if (token->klass == TOK_BRACE_CLOSE) {
      next_token();
    } else error("");
    scope_pop_level(scope_level-1);
  } else error("");
  return 0;
}

internal struct Ast*
build_identifierList()
{
  if (token_is_name(token)) {
    build_name();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_name();
    }
  } else error("");
  return 0;
}

internal struct Ast*
build_errorDeclaration()
{
  if (token->klass == TOK_ERROR) {
    scope_push_level();
    next_token();
    if (token->klass == TOK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        build_identifierList();
        if (token->klass == TOK_BRACE_CLOSE) {
          next_token();
        } else error("");
      } else error("");
    } else error("");
    scope_pop_level(scope_level-1);
  } else error("");
  return 0;
}

internal struct Ast*
build_matchKindDeclaration()
{
  if (token->klass == TOK_MATCH_KIND) {
    next_token();
    if (token->klass == TOK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        build_identifierList();
      } else error("");
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_functionDeclaration()
{
  if (token_is_typeOrVoid(token)) {
    build_functionPrototype();
    if (token->klass == TOK_BRACE_OPEN) {
      build_blockStatement();
    } else error("");
  } else error("");
  return 0;
}

internal struct Ast*
build_declaration()
{
  if (token_is_declaration(token)) {
    if (token->klass == TOK_CONST)
      build_constantDeclaration();
    else if (token->klass == TOK_EXTERN)
      build_externDeclaration();
    else if (token->klass == TOK_ACTION)
      build_actionDeclaration();
    else if (token_is_typeDeclaration(token))
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      build_typeDeclaration();
    else if (token_is_typeRef(token) && peek_token()->klass == TOK_PARENTH_OPEN)
      build_instantiation();
    else if (token->klass == TOK_ERROR)
      build_errorDeclaration();
    else if (token->klass == TOK_MATCH_KIND)
      build_matchKindDeclaration();
    else if (token_is_typeOrVoid(token))
      build_functionDeclaration();
    else assert(false);
  } else error("");
  return 0;
}

internal struct Ast*
build_p4program()
{
  if (token_is_declaration(token)) {
    build_declaration();
    while (token_is_declaration(token)) {
      build_declaration();
    }
  } else if (token->klass == TOK_SEMICOLON) {
    next_token(); /* <emptyDeclaration> */
  } else error("at line %d: declaration expected, got '%s'", token->line_nr, token->lexeme);
  if (token->klass != TOK_EOI)
    error("at line %d: unexpected token '%s'", token->line_nr, token->lexeme);
  return 0;
}

internal bool
token_is_realTypeArg(struct Token* token)
{
  bool result = token->klass == TOK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

internal bool
token_is_binaryOperator(struct Token* token)
{
  bool result = token->klass == TOK_STAR || token->klass == TOK_SLASH
    || token->klass == TOK_PLUS || token->klass == TOK_MINUS
    || token->klass == TOK_ANGLE_OPEN || token->klass == TOK_ANGLE_CLOSE
    /* || token->klass == TOK_NOT_EQUAL */ || token->klass == TOK_LOGIC_EQUAL
    || token->klass == TOK_DOTPREFIX
    || token->klass == TOK_ANGLE_OPEN || token->klass == TOK_ANGLE_CLOSE
    || token->klass == TOK_BRACKET_OPEN || token->klass == TOK_PARENTH_OPEN;
  return result;
}

internal struct Ast*
build_realTypeArg()
{
  if (token->klass == TOK_DONTCARE) {
    next_token();
  } else if (token_is_typeRef(token)) {
    build_typeRef();
  } else error("");
  return 0;
}

internal struct Ast*
build_realTypeArgumentList()
{
  if (token_is_realTypeArg(token)) {
    build_realTypeArg();
    while (token->klass == TOK_COMMA) {
      next_token();
      build_realTypeArg();
    }
  }
  return 0;
}

internal struct Ast*
build_expressionPrimary()
{
  struct Ast* primary = 0;
  if (token_is_expression(token)) {
    if (token->klass == TOK_INTEGER) {
      next_token();
    } else if (token->klass == TOK_TRUE) {
      next_token();
    } else if (token->klass == TOK_FALSE) {
      next_token();
    } else if (token->klass == TOK_STRING_LITERAL) {
      next_token();
    } else if (token->klass == TOK_UNARY_DOTPREFIX) {
      next_token();
      build_nonTypeName();
    } else if (token_is_nonTypeName(token)) {
      primary = (struct Ast*)build_nonTypeName();
    } else if (token->klass == TOK_BRACE_OPEN) {
      next_token();
      build_expressionList();
      if (token->klass == TOK_BRACE_CLOSE) {
        next_token();
      } else error("");
    } else if (token->klass == TOK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(token))
        build_expression(1);
      if (token->klass == TOK_PARENTH_CLOSE) {
        next_token();
      } else error("");
    } else if (token->klass == TOK_LOGIC_NOT) {
      next_token();
      build_expression(1);
    } else if (token->klass == TOK_UNARY_MINUS) {
      next_token();
      build_expression(1);
    } else if (token_is_typeName(token)) {
      build_typeName();
    } else if (token->klass == TOK_ERROR) {
      next_token();
    } else if (token->klass == TOK_CAST) {
      next_token();
      if (token->klass == TOK_PARENTH_OPEN) {
        next_token();
        if (token_is_typeRef(token)) {
          build_typeRef();
          if (token->klass == TOK_PARENTH_CLOSE) {
            next_token();
            if (token_is_expression(token)) {
              build_expression(1);
            } else error("");
          } else error("");
        } else error("");
      } else error("");
    } else assert(false);
  } else error("");
  return primary;
}

internal int
get_operator_priority(struct Token* token)
{
  int prio = 0;
  if (token->klass == TOK_LOGIC_EQUAL)
    prio = 1;
  else if (token->klass == TOK_PLUS || token->klass == TOK_MINUS)
    prio = 2;
  else if (token->klass == TOK_BRACKET_OPEN
           || token->klass == TOK_PARENTH_OPEN
           || token->klass == TOK_ANGLE_OPEN) {
    prio = 3;
  }
  else if (token->klass == TOK_DOTPREFIX) {
    prio = 4;
  }
  else assert(false);
  return prio;
}

internal struct Ast*
build_expression(int priority_threshold)
{
  struct Ast* expr = 0;
  if (token_is_expression(token) || token_is_binaryOperator(token)) {
    build_expressionPrimary();
    while (token_is_binaryOperator(token)) {
      int priority = get_operator_priority(token);
      if (priority >= priority_threshold) {
        if (token->klass == TOK_DOTPREFIX) {
          next_token();
          if (token_is_name(token)) {
            build_name();
          } else error("");
        }
        else if (token->klass == TOK_BRACKET_OPEN) {
          next_token();
          if (token_is_expression(token)) {
            build_expression(1);
            if (token->klass == TOK_COLON) {
              next_token();
              if (token_is_expression(token)) {
                build_expression(1);
              } else error("");
            }
            if (token->klass == TOK_BRACKET_CLOSE) {
              next_token();
            } else error("");
          } else error("");
        }
        else if (token->klass == TOK_PARENTH_OPEN) {
          next_token();
          build_argumentList();
          if (token->klass == TOK_PARENTH_CLOSE) {
            next_token();
          } else error("");
        }
        else if (token->klass == TOK_ANGLE_OPEN) {
          next_token();
          if (token_is_realTypeArg(token)) {
            build_realTypeArgumentList();
          } else error("");
        } else {
          next_token();
          if (token_is_expression(token)) {
            build_expression(priority_threshold + 1);
          } else error("");
        }
      } else break;
    }
  } else error("");
  return expr;
}

void
build_ast()
{
  add_keyword("action", TOK_ACTION);
  add_keyword("enum", TOK_ENUM);
  add_keyword("in", TOK_IN);
  add_keyword("package", TOK_PACKAGE);
  add_keyword("select", TOK_SELECT);
  add_keyword("switch", TOK_SWITCH);
  add_keyword("tuple", TOK_TUPLE);
  add_keyword("control", TOK_CONTROL);
  add_keyword("error", TOK_ERROR);
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
  add_keyword("apply", TOK_APPLY);
  add_keyword("var", TOK_VAR);
  add_keyword("cast", TOK_CAST);
  add_keyword("const", TOK_CONST);
  add_keyword("bool", TOK_BOOL);
  add_keyword("true", TOK_TRUE);
  add_keyword("false", TOK_FALSE);
  add_keyword("void", TOK_VOID);
  add_keyword("int", TOK_INT);
  add_keyword("bit", TOK_BIT);
  add_keyword("varbit", TOK_VARBIT);

  token = tokenized_input;
  next_token();
  build_p4program();
}
