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

external struct Cst_P4Program* p4program;

internal struct Cst* build_expression(int priority_threshold);
internal struct Cst* build_typeRef();
internal struct Cst* build_blockStatement();
internal struct Cst* build_statement();
internal struct Cst* build_parserStatement();

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
    assert (ident_var->ident_kind == Ident_Ident);
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
    ident->ident_kind = Ident_Ident;
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
    assert(result->ident_kind == Ident_Type);
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
    ident->ident_kind = Ident_Type;
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
  ident->ident_kind = Ident_Keyword;
  namespace->ns_global = (struct Ident*)ident;
  return ident;
}

internal struct Token*
next_token()
{
  assert(token < tokenized_input + tokenized_input_len);
  prev_token = token++;
  while (token->klass == Token_Comment)
    token++;

  if (token->klass == Token_Identifier) {
    struct Namespace_Entry* ns = get_namespace(token->lexeme);
    if (ns->ns_global) {
      struct Ident* ident = ns->ns_global;
      if (ident->ident_kind == Ident_Keyword) {
        token->klass = ((struct Ident_Keyword*)ident)->token_klass;
        return token;
      }
    }

    if (ns->ns_type) {
      struct Ident* ident = ns->ns_type;
      if (ident->ident_kind == Ident_Type) {
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
  return token->klass == Token_TypeIdentifier;
}

internal bool
token_is_prefixedType(struct Token* token)
{
  return token->klass == Token_TypeIdentifier;
}

internal bool
token_is_baseType(struct Token* token)
{
  bool result = token->klass == Token_Bool || token->klass == Token_Error || token->klass == Token_Int
    || token->klass == Token_Bit || token->klass == Token_Varbit;
  return result;
}

internal bool
token_is_typeRef(struct Token* token)
{
  bool result = token_is_baseType(token) || token->klass == Token_TypeIdentifier || token->klass == Token_Tuple;
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
token_is_functionPrototype()
{
  bool result = token_is_typeRef(token) || token->klass == Token_Void || token->klass == Token_Identifier;
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
token_is_expressionPrimary(struct Token* token)
{
  bool result = token->klass == Token_Integer || token->klass == Token_True || token->klass == Token_False
    || token->klass == Token_StringLiteral || token->klass == Token_UnaryDotprefix || token_is_nonTypeName(token)
    || token->klass == Token_BraceOpen || token->klass == Token_ParenthOpen || token->klass == Token_LogicNot
    || token->klass == Token_UnaryMinus || token_is_typeName(token) || token->klass == Token_Error
    || token_is_prefixedType(token) || token->klass == Token_Cast;
  return result;
}

internal bool
token_is_expression(struct Token* token)
{
  return token_is_expressionPrimary(token);
}

internal struct Cst_NonTypeName*
build_nonTypeName()
{
  struct Cst_NonTypeName* name = 0;
  if (token_is_nonTypeName(token)) {
    name = new_ast_node(Cst_NonTypeName);
    name->name = token->lexeme;
    next_token();
  } else error("at line %d: ", token->line_nr);
  return name;
}

internal struct Cst_Name*
build_name()
{
  struct Cst_Name* name = 0;
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      name = (struct Cst_Name*)build_nonTypeName();
    } else if (token->klass == Token_TypeIdentifier) {
      struct Cst_TypeName* type_name = new_ast_node(Cst_TypeName);
      type_name->name = token->lexeme;
      name = (struct Cst_Name*)type_name;
      next_token();
    } else assert(false);
  } else error("at line %d: ", token->line_nr);
  return name;
}

internal struct Cst*
build_typeParameterList()
{
  if (token_is_typeParameterList(token)) {
    struct Cst_Name* name = build_name();
    if (name->kind == Cst_NonTypeName)
      new_type(((struct Cst_NonTypeName*)name)->name);
    while (token->klass == Token_Comma) {
      next_token();
      build_name();
    }
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_optTypeParameters()
{
  if (token->klass == Token_AngleOpen) {
    next_token();
    if (token_is_typeParameterList(token)) {
      build_typeParameterList();
      if (token->klass == Token_AngleClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  }
  return 0;
}

internal struct Cst*
build_typeArg()
{
  if (token_is_typeArg(token))
  {
    if (token->klass == Token_Dontcare) {
      next_token();
    } else if (token_is_typeRef(token)) {
      build_typeRef();
    } else if (token_is_nonTypeName(token)) {
      build_nonTypeName();
    } else assert(false);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal bool
token_is_methodPrototype(struct Token* token)
{
  return token_is_functionPrototype() | token->klass == Token_TypeIdentifier;
}

internal struct Cst*
build_direction()
{
  if (token_is_direction(token)) {
    next_token();
  }
  return 0;
}

internal struct Cst*
build_parameter()
{
  build_direction();
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == Token_Equal) {
        next_token();
        if (token_is_expression(token)) {
          build_expression(1);
        } else error("at line %d: ", token->line_nr);
      }
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parameterList()
{
  if (token_is_parameter(token)) {
    build_parameter();
    while (token->klass == Token_Comma) {
      next_token();
      build_parameter();
    }
  }
  return 0;
}

internal struct Cst*
build_typeOrVoid()
{
  struct Cst* type = 0;
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      type = build_typeRef();
    } else if (token->klass == Token_Void) {
      struct Cst_TypeName* void_name = new_ast_node(Cst_TypeName);
      void_name->name = token->lexeme;
      type = (struct Cst*)void_name;
      next_token();
    } else if (token->klass == Token_Identifier) {
      struct Cst_NonTypeName* name = new_ast_node(Cst_NonTypeName);
      name->name = token->lexeme;
      type = (struct Cst*)name;
      next_token();
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return type;
}

internal struct Cst*
build_functionPrototype()
{
  if (token_is_typeOrVoid(token)) {
    scope_push_level();
    struct Cst* return_type = build_typeOrVoid();
    if (return_type->kind == Cst_NonTypeName)
      new_type(((struct Cst_NonTypeName*)return_type)->name);
    if (token_is_name(token)) {
      struct Cst_Name* function_name = build_name();
      new_ident(function_name->name);
      build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        if (token_is_parameter(token)) {
          build_parameterList();
          if (token->klass == Token_ParenthClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
    scope_pop_level(scope_level-1);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_methodPrototype()
{
  if (token_is_methodPrototype(token)) {
    if (token->klass == Token_TypeIdentifier && peek_token()->klass == Token_ParenthOpen) {
      next_token();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else if (token_is_functionPrototype(token)) {
      build_functionPrototype();
    } else error("at line %d: ", token->line_nr);

    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_methodPrototypes()
{
  while (token_is_methodPrototype(token))
    build_methodPrototype();
  return 0;
}

internal struct Cst_Declaration*
build_externDeclaration()
{
  if (token->klass == Token_Extern) {
    next_token();
    if (token_is_nonTypeName(token)) {
      struct Cst_NonTypeName* decl_name = build_nonTypeName();
      if (decl_name->kind == Cst_NonTypeName)
        new_type(((struct Cst_NonTypeName*)decl_name)->name);
      build_optTypeParameters();
      if (token->klass == Token_BraceOpen) {
        scope_push_level();
        next_token();
        if (token_is_methodPrototype(token)) {
          build_methodPrototypes();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
        scope_pop_level(scope_level-1);
      } else error("at line %d: ", token->line_nr);
    } else if (token_is_functionPrototype(token)) {
      build_functionPrototype();
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_integerTypeSize()
{
  if (token->klass == Token_Integer) {
    next_token();
  } else if (token->klass == Token_ParenthOpen) {
    build_expression(1);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_baseType()
{
  struct Cst_BaseType* base_type = 0;
  if (token_is_baseType(token)) {
    base_type = new_ast_node(Cst_BaseType);
    if (token->klass == Token_Bool) {
      base_type->base_type = BASETYPE_BOOL;
      next_token();
    } else if (token->klass == Token_Error) {
      base_type->base_type = BASETYPE_ERROR;
      next_token();
    } else if (token->klass == Token_Int) {
      base_type->base_type = BASETYPE_INT;
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      }
    } else if (token->klass == Token_Bit) {
      base_type->base_type = BASETYPE_BIT;
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      }
    } else if (token->klass == Token_Varbit) {
      base_type->base_type = BASETYPE_VARBIT;
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        base_type->size = build_integerTypeSize();
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      }
    } else assert(false);
  }
  return (struct Cst*)base_type;
}

internal struct Cst*
build_typeArgumentList()
{
  if (token_is_typeArg(token)) {
    build_typeArg();
    while (token->klass == Token_Comma) {
      next_token();
      build_typeArg();
    }
  }
  return 0;
}

internal struct Cst*
build_tupleType()
{
  if (token->klass == Token_Tuple) {
    next_token();
    if (token->klass == Token_AngleOpen) {
      next_token();
      build_typeArgumentList();
      if (token->klass == Token_AngleClose) {
        next_token();
      } else error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: '<' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_headerStackType()
{
  if (token->klass == Token_BracketOpen) {
    next_token();
    if (token_is_expression(token)) {
      build_expression(1);
      if (token->klass == Token_BracketClose) {
        next_token();
      } else error("at line %d: ']' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: expression expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_specializedType()
{
  if (token->klass == Token_AngleOpen) {
    next_token();
    build_typeArgumentList();
    if (token->klass == Token_AngleClose) {
      next_token();
    } else error("at line %d: '>' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_prefixedType()
{
  struct Cst_TypeName* name = 0;
  if (token->klass == Token_TypeIdentifier) {
    name = new_ast_node(Cst_TypeName);
    name->name = token->lexeme;
    next_token();
    if (token->klass == Token_Dotprefix) {
      next_token();
      if (token->klass == Token_TypeIdentifier) {
        struct Cst_PrefixedType* pfx_type = new_ast_node(Cst_PrefixedType);
        pfx_type->first_name = name;
        pfx_type->second_name = new_ast_node(Cst_TypeName);
        pfx_type->second_name->name = token->lexeme;
        next_token();
      } else error("at line %d: ", token->line_nr);
    }
  } else error("at line %d: ", token->line_nr);
  return (struct Cst*)name;
}

internal struct Cst*
build_typeName()
{
  struct Cst* name = 0;
  if (token->klass == Token_TypeIdentifier) {
    name = build_prefixedType();
    if (token->klass == Token_AngleOpen) {
      name = build_specializedType();
    } if (token->klass == Token_BracketOpen) {
      name = build_headerStackType();
    }
  } else error("at line %d: ", token->line_nr);
  return name;
}

internal struct Cst*
build_typeRef()
{
  struct Cst* ref = 0;
  if (token_is_typeRef(token)) {
    if (token_is_baseType(token)) {
      ref = build_baseType();
    } else if (token->klass == Token_TypeIdentifier) {
      /* <typeName> | <specializedType> | <headerStackType> */
      ref = build_typeName();
    } else if (token->klass == Token_Tuple) {
      ref = build_tupleType();
    } else assert(false);
  } else error("at line %d: ", token->line_nr);
  return ref;
}

internal bool
token_is_structField(struct Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

internal struct Cst*
build_structField()
{
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      new_ident(name->name);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_structFieldList()
{
  while (token_is_structField(token)) {
    build_structField();
    if (token->klass == Token_Comma) {
      next_token();
      build_structField();
    }
  }
  return 0;
}

internal struct Cst*
build_headerTypeDeclaration()
{
  if (token->klass == Token_Header) {
    next_token();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      if (name->kind == Cst_NonTypeName)
        new_type(((struct Cst_NonTypeName*)name)->name);
      if (token->klass == Token_BraceOpen) {
        scope_push_level();
        next_token();
        build_structFieldList();
        if (token->klass == Token_BraceClose) {
          next_token(token);
        } else error("at line %d: '}' expected, got '%s'", token->line_nr, token->lexeme);
        scope_pop_level(scope_level-1);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_headerUnionDeclaration()
{
  if (token->klass == Token_HeaderUnion) {
    next_token();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == Token_BraceOpen) {
        next_token();
        build_structFieldList();
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_structTypeDeclaration()
{
  if (token->klass == Token_Struct) {
    next_token();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      if (name->kind == Cst_NonTypeName)
        new_type(((struct Cst_NonTypeName*)name)->name);
      if (token->klass == Token_BraceOpen) {
        next_token();
        build_structFieldList();
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal bool
token_is_specifiedIdentifier(struct Token* token)
{
  return token_is_name(token);
}

internal struct Cst*
build_initializer()
{
  return build_expression(1);
}

internal struct Cst*
build_optInitializer()
{
  if (token->klass == Token_Equal) {
    next_token();
    build_initializer();
  }
  return 0;
}

internal struct Cst*
build_specifiedIdentifier()
{
  if (token_is_specifiedIdentifier(token)) {
    build_name();
    if (token->klass == Token_Equal) {
      next_token();
      if (token_is_expression(token)) {
        build_initializer();
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_specifiedIdentifierList()
{
  if (token_is_specifiedIdentifier(token)) {
    build_specifiedIdentifier();
    while (token->klass == Token_Comma) {
      next_token();
      build_specifiedIdentifier();
    }
  }
  return 0;
}

internal struct Cst*
build_enumDeclaration()
{
  if (token->klass == Token_Enum) {
    next_token();
    if (token->klass == Token_Bit) {
      if (token->klass == Token_AngleOpen) {
        next_token();
        if (token->klass == Token_Integer) {
          next_token();
          if (token->klass == Token_AngleClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    }
    if (token_is_name) {
      build_name();
      if (token->klass == Token_BraceOpen) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          build_specifiedIdentifierList();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    if (token->klass == Token_Header) {
      build_headerTypeDeclaration();
    } else if (token->klass == Token_HeaderUnion) {
      build_headerUnionDeclaration();
    } else if (token->klass == Token_Struct) {
      build_structTypeDeclaration();
    } else if (token->klass == Token_Enum) {
      build_enumDeclaration();
    } else assert(false);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserTypeDeclaration()
{
  if (token->klass == Token_Parser) {
    next_token();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      if (name->kind == Cst_NonTypeName)
        new_type(((struct Cst_NonTypeName*)name)->name);
      build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_optConstructorParameters()
{
  if (token->klass == Token_ParenthOpen) {
    next_token();
    build_parameterList();
    if (token->klass == Token_ParenthClose) {
      next_token();
    } else error("at line %d: ", token->line_nr);
  }
  return 0;
}

internal struct Cst*
build_constantDeclaration()
{
  if (token->klass == Token_Const) {
    next_token();
    if (token_is_typeRef(token)) {
      build_typeRef();
      if (token_is_name(token)) {
        build_name();
        if (token->klass == Token_Equal) {
          next_token();
          if (token_is_expression(token)) {
            build_expression(1);
            if (token->klass == Token_Semicolon) {
              next_token();
            } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal bool
token_is_declaration(struct Token* token)
{
  bool result = token->klass == Token_Const || token->klass == Token_Extern || token->klass == Token_Action
    || token->klass == Token_Parser || token_is_typeDeclaration(token) || token->klass == Token_Control
    || token_is_typeRef(token) || token->klass == Token_Error || token->klass == Token_MatchKind
    || token_is_typeOrVoid(token);
  return result;
}

internal bool
token_is_lvalue(struct Token* token)
{
  bool result = token_is_nonTypeName(token) | token->klass == Token_Dotprefix;
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
    || token->klass == Token_Return;
  return result;
}

internal bool
token_is_statementOrDeclaration(struct Token* token)
{
  bool result = token->klass == Token_Var || token->klass == Token_Const || token_is_statement(token);
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
  bool result = token->klass == Token_Const || token->klass == Token_Var || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(struct Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == Token_BraceOpen || token->klass == Token_Const || token->klass == Token_Var
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
    || token_is_typeRef(token) || token->klass == Token_Var;
  return result;
}

internal struct Cst*
build_argument()
{
  assert(token_is_argument(token));
  if (token_is_expression(token)) {
    build_expression(1);
  } else if (token_is_name(token)) {
    build_name();
    if (token->klass == Token_Equal) {
      next_token();
      if (token_is_expression(token)) {
        build_expression(1);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else if (token->klass == Token_Dontcare) {
    next_token();
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_argumentList()
{
  if (token_is_argument(token)) {
    build_argument();
    while (token->klass == Token_Comma) {
      next_token();
      build_argument();
    }
  }
  return 0;
}

internal struct Cst*
build_variableDeclaration()
{
  if (token->klass == Token_Var) {
    next_token();
    if (token_is_typeRef(token)) {
      build_typeRef();
      if (token_is_name(token)) {
        build_name();
        build_optInitializer();
        if (token->klass == Token_Semicolon) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_instantiation()
{
  if (token_is_typeRef(token)) {
    build_typeRef();
    if (token->klass == Token_ParenthOpen) {
      next_token();
      build_argumentList();
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token_is_name(token)) {
          build_name();
          if (token->klass == Token_Semicolon) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserLocalElement()
{
  if (token_is_parserLocalElement(token)) {
    if (token->klass == Token_Const) {
      build_constantDeclaration();
    } else if (token->klass == Token_Var) {
      build_variableDeclaration();
    } else if (token_is_typeRef(token)) {
      build_instantiation();
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserLocalElements()
{
  while (token_is_parserLocalElement(token)) {
    build_parserLocalElement();
  }
  return 0;
}

internal struct Cst*
build_directApplication()
{
  if (token_is_typeName(token)) {
    build_typeName();
    if (token->klass == Token_Dotprefix) {
      next_token();
      if (token->klass == Token_Apply) {
        next_token();
        if (token->klass == Token_ParenthOpen) {
          next_token();
          build_argumentList();
          if (token->klass == Token_ParenthClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_prefixedNonTypeName()
{
  struct Cst* name = 0;
  if (token->klass == Token_Dotprefix) {
    struct Cst_DotPrefix* dot_prefix = new_ast_node(Cst_DotPrefix);
    name = (struct Cst*)dot_prefix;
    next_token();
  }
  if (token_is_nonTypeName) {
    name = (struct Cst*)build_nonTypeName();
  } else error("at line %d: ", token->line_nr);
  return name;
}

internal struct Cst*
build_lvalue()
{
  if (token_is_lvalue(token)) {
    build_prefixedNonTypeName();
    while (token->klass == Token_Dotprefix || token->klass == Token_BracketOpen) {
      if (token->klass == Token_Dotprefix) {
        next_token();
        build_name();
      }
      if (token->klass == Token_BracketOpen) {
        next_token();
        if (token_is_expression(token)) {
          build_expression(1);
          if (token->klass == Token_Colon) {
            next_token();
            if (token_is_expression(token)) {
              build_expression(1);
            } else error("at line %d: ", token->line_nr);
          }
          if (token->klass == Token_BracketClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      }
    }
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    build_lvalue();
    if (token->klass == Token_AngleOpen) {
      next_token();
      build_typeArgumentList();
      if (token->klass == Token_AngleClose) {
        next_token();
        if (token->klass == Token_ParenthOpen) {
          next_token();
          build_argumentList();
          if (token->klass == Token_ParenthClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    }
    if (token->klass == Token_ParenthOpen) {
      next_token();
      build_argumentList();
      if (token->klass == Token_ParenthClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
    } else if (token->klass == Token_Equal) {
      next_token();
      build_expression(1);
    } else error("at line %d: ", token->line_nr);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  }
  return 0;
}

internal struct Cst*
build_parserStatements()
{
  while (token_is_parserStatement(token)) {
    build_parserStatement();
  }
  return 0;
}

internal struct Cst*
build_parserBlockStatements()
{
  if (token->klass == Token_BraceOpen) {
    next_token();
    build_parserStatements();
    if (token->klass == Token_BraceClose) {
      next_token();
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserStatement()
{
  if (token_is_assignmentOrMethodCallStatement(token)) {
    build_assignmentOrMethodCallStatement();
  } else if (token_is_typeName(token)) {
    build_directApplication();
  } else if (token->klass == Token_BraceOpen) {
    build_parserBlockStatements();
  } else if (token->klass == Token_Const) {
    build_constantDeclaration();
  } else if (token->klass == Token_Var) {
    build_variableDeclaration();
  } else if (token->klass == Token_Semicolon) {
    ; // <emptyStatement>
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_expressionList()
{
  if (token_is_expression(token)) {
    build_expression(1);
    while (token->klass == Token_Comma) {
      build_expression(1);
    }
  }
  return 0;
}

internal struct Cst*
build_simpleKeysetExpression()
{
  struct Cst* expr = 0;
  if (token_is_expression(token)) {
    expr = build_expression(1);
  } else if (token->klass == Token_Default) {
    next_token();
  } else if (token->klass == Token_Dontcare) {
    next_token();
  } else error("at line %d: ", token->line_nr);
  return expr;
}

internal struct Cst*
build_tupleKeysetExpression()
{
  struct Cst* expr = 0;
  if (token->klass == Token_ParenthOpen) {
    next_token();
    build_simpleKeysetExpression();
    while (token->klass == Token_Comma) {
      next_token();
      build_simpleKeysetExpression();
    }
    if (token->klass == Token_ParenthClose) {
      next_token();
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return expr;
}

internal struct Cst*
build_keysetExpression()
{
  struct Cst* expr = 0;
  if (token->klass == Token_ParenthOpen) {
    build_tupleKeysetExpression();
  } else if (token_is_simpleKeysetExpression(token)) {
    build_simpleKeysetExpression();
  } else error("at line %d: ", token->line_nr);
  return expr;
}

internal struct Cst*
build_selectCase()
{
  if (token_is_keysetExpression(token)) {
    build_keysetExpression();
    if (token->klass == Token_Colon) {
      next_token();
      if (token_is_name(token)) {
        build_name();
        if (token->klass == Token_Semicolon) {
          next_token();
        } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_selectCaseList()
{
  while (token_is_selectCase(token)) {
    build_selectCase();
  }
  return 0;
}

internal struct Cst*
build_selectExpression()
{
  if (token->klass == Token_Select) {
    next_token();
    if (token->klass == Token_ParenthOpen) {
      next_token();
      build_expressionList();
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          scope_push_level();
          next_token();
          build_selectCaseList();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
          scope_pop_level(scope_level-1);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_stateExpression()
{
  if (token_is_name(token)) {
    build_name();
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: ", token->line_nr);
  } else if (token->klass == Token_Select) {
    build_selectExpression();
  }
  else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_transitionStatement()
{
  if (token->klass == Token_Transition) {
    next_token();
    build_stateExpression();
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserState()
{
  if (token->klass == Token_State) {
    next_token();
    struct Cst_Name* name = build_name();
    new_ident(name->name);
    if (token->klass == Token_BraceOpen) {
      scope_push_level();
      next_token();
      build_parserStatements();
      build_transitionStatement();
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
      scope_pop_level(scope_level-1);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserStates()
{
  if (token->klass == Token_State) {
    build_parserState();
    while (token->klass == Token_State) {
      build_parserState();
    }
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_parserDeclaration()
{
  assert(token->klass == Token_Parser);
  build_parserTypeDeclaration();
  if (token->klass == Token_Semicolon) {
    next_token(); /* <parserTypeDeclaration> */
  } else {
    build_optConstructorParameters();
    if (token->klass == Token_BraceOpen) {
      scope_push_level();
      next_token();
      build_parserLocalElements();
      build_parserStates();
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
      scope_pop_level(scope_level-1);
    } else error("at line %d: ", token->line_nr);
  }
  return 0;
}

internal struct Cst*
build_controlTypeDeclaration()
{
  if (token->klass == Token_Control) {
    next_token();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      if (name->kind == Cst_NonTypeName)
        new_type(((struct Cst_NonTypeName*)name)->name);
      build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_actionDeclaration()
{
  if (token->klass == Token_Action) {
    next_token();
    if (token_is_name(token)) {
      build_name();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token->klass == Token_BraceOpen) {
            build_blockStatement();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_controlLocalDeclaration()
{
  struct Cst* decl = 0;
  if (token->klass == Token_Const) {
    decl = build_constantDeclaration();
  } else if (token->klass == Token_Action) {
    decl = build_actionDeclaration();
  } else if (token_is_typeRef(token)) {
    decl = build_instantiation();
  } else if (token->klass == Token_Var) {
    decl = build_variableDeclaration();
  } else error("at line %d: ", token->line_nr);
  return decl;
}

internal struct Cst*
build_controlDeclaration()
{
  if (token->klass == Token_Control) {
    build_controlTypeDeclaration();
    if (token->klass == Token_Semicolon) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      build_optConstructorParameters();
      if (token->klass == Token_BraceOpen) {
        next_token();
        while (token_is_controlLocalDeclaration(token)) {
          build_controlLocalDeclaration();
        }
        if (token->klass == Token_Apply) {
          next_token();
          build_blockStatement();
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    }
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_packageTypeDeclaration()
{
  if (token->klass == Token_Package) {
    next_token();
    if (token_is_name(token)) {
      struct Cst_Name* name = build_name();
      if (name->kind == Cst_NonTypeName)
        new_type(name->name);
      build_optTypeParameters();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        build_parameterList();
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_typedefDeclaration()
{
  if (token->klass == Token_Typedef || token->klass == Token_Type) {
    if (token->klass == Token_Typedef) {
      next_token();
    } else if (token->klass == Token_Type) {
      next_token();
    } else assert(false);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      if (token_is_typeRef(token)) {
        build_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        build_derivedTypeDeclaration();
      } else assert(false);

      if (token_is_name(token)) {
        struct Cst_Name* name = build_name();
        if (name->kind == Cst_NonTypeName)
          new_type(((struct Cst_NonTypeName*)name)->name);
        if (token->klass == Token_Semicolon) {
          next_token();
        } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_typeDeclaration()
{
  if (token_is_typeDeclaration(token)) {
    if (token_is_derivedTypeDeclaration(token)) {
      build_derivedTypeDeclaration();
    } else if (token->klass == Token_Typedef || token->klass == Token_Type) {
      build_typedefDeclaration();
    } else if (token->klass == Token_Parser) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      build_parserDeclaration();
    } else if (token->klass == Token_Control) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      build_controlDeclaration();
    } else if (token->klass == Token_Package) {
      build_packageTypeDeclaration();
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_conditionalStatement()
{
  if (token->klass == Token_If) {
    next_token();
    if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_expression(token)) {
        build_expression(1);
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token_is_statement(token)) {
            build_statement();
            if (token->klass == Token_Else) {
              next_token();
              if (token_is_statement(token))
                build_statement();
              else error("at line %d: ", token->line_nr);
            }
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_exitStatement()
{
  if (token->klass == Token_Exit) {
    next_token();
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_returnStatement()
{
  if (token->klass == Token_Return) {
    next_token();
    if (token_is_expression(token))
      build_expression(1);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: ';' expected, got '%s'", token->line_nr, token->lexeme);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_statement()
{
  if (token_is_assignmentOrMethodCallStatement(token))
    build_assignmentOrMethodCallStatement();
  else if (token_is_typeName(token))
    build_directApplication();
  else if (token->klass == Token_If)
    build_conditionalStatement();
  else if (token->klass == Token_Semicolon)
    ; // empty statement
  else if (token->klass == Token_BraceOpen)
    build_blockStatement();
  else if (token->klass == Token_Exit)
    build_exitStatement();
  else if (token->klass == Token_Return)
    build_returnStatement();
  else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_statementOrDeclList()
{
  while (token_is_statementOrDeclaration(token)) {
    if (token->klass == Token_Var) {
      build_variableDeclaration();
    } else if (token_is_typeRef(token) && peek_token()->klass == Token_ParenthOpen) {
      build_instantiation();
    } else if (token_is_statement(token)) {
      build_statement();
    } else if (token->klass == Token_Const)
      build_constantDeclaration();
    else assert(false);
  }
  return 0;
}

internal struct Cst*
build_blockStatement()
{
  if (token->klass == Token_BraceOpen) {
    scope_push_level();
    next_token();
    build_statementOrDeclList();
    if (token->klass == Token_BraceClose) {
      next_token();
    } else error("at line %d: ", token->line_nr);
    scope_pop_level(scope_level-1);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_identifierList()
{
  if (token_is_name(token)) {
    build_name();
    while (token->klass == Token_Comma) {
      next_token();
      build_name();
    }
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_errorDeclaration()
{
  if (token->klass == Token_Error) {
    scope_push_level();
    next_token();
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        build_identifierList();
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
    scope_pop_level(scope_level-1);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_matchKindDeclaration()
{
  if (token->klass == Token_MatchKind) {
    next_token();
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        build_identifierList();
      } else error("at line %d: ", token->line_nr);
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_functionDeclaration()
{
  if (token_is_typeOrVoid(token)) {
    build_functionPrototype();
    if (token->klass == Token_BraceOpen) {
      build_blockStatement();
    } else error("at line %d: ", token->line_nr);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_declaration()
{
  if (token_is_declaration(token)) {
    if (token->klass == Token_Const)
      build_constantDeclaration();
    else if (token->klass == Token_Extern)
      build_externDeclaration();
    else if (token->klass == Token_Action)
      build_actionDeclaration();
    else if (token_is_typeDeclaration(token))
      /* <parserDeclaration> | <typeDeclaration> | <controlDeclaration> */
      build_typeDeclaration();
    else if (token_is_typeRef(token) && peek_token()->klass == Token_ParenthOpen)
      build_instantiation();
    else if (token->klass == Token_Error)
      build_errorDeclaration();
    else if (token->klass == Token_MatchKind)
      build_matchKindDeclaration();
    else if (token_is_typeOrVoid(token))
      build_functionDeclaration();
    else assert(false);
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_p4program()
{
  if (token_is_declaration(token)) {
    build_declaration();
    while (token_is_declaration(token)) {
      build_declaration();
    }
  } else if (token->klass == Token_Semicolon) {
    next_token(); /* <emptyDeclaration> */
  } else error("at line %d: declaration expected, got '%s'", token->line_nr, token->lexeme);
  if (token->klass != Token_EndOfInput)
    error("at line %d: unexpected token '%s'", token->line_nr, token->lexeme);
  return 0;
}

internal bool
token_is_realTypeArg(struct Token* token)
{
  bool result = token->klass == Token_Dontcare || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

internal bool
token_is_binaryOperator(struct Token* token)
{
  bool result = token->klass == Token_Star || token->klass == Token_Slash
    || token->klass == Token_Plus || token->klass == Token_Minus
    || token->klass == Token_LessEqual || token->klass == Token_GreaterEqual
    || token->klass == Token_AngleOpen || token->klass == Token_AngleClose
    || token->klass == Token_LogicNotEqual || token->klass == Token_LogicEqual
    || token->klass == Token_LogicOr || token->klass == Token_LogicAnd
    || token->klass == Token_BitwiseOr || token->klass == Token_BitwiseAnd;
  return result;
}

internal bool
token_is_exprOperator(struct Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == Token_Dotprefix
    || token->klass == Token_BracketOpen || token->klass == Token_ParenthOpen
    || token->klass == Token_AngleOpen;
  return result;
}

internal struct Cst*
build_realTypeArg()
{
  if (token->klass == Token_Dontcare) {
    next_token();
  } else if (token_is_typeRef(token)) {
    build_typeRef();
  } else error("at line %d: ", token->line_nr);
  return 0;
}

internal struct Cst*
build_realTypeArgumentList()
{
  if (token_is_realTypeArg(token)) {
    build_realTypeArg();
    while (token->klass == Token_Comma) {
      next_token();
      build_realTypeArg();
    }
  }
  return 0;
}

internal struct Cst*
build_expressionPrimary()
{
  struct Cst* primary = 0;
  if (token_is_expression(token)) {
    if (token->klass == Token_Integer) {
      next_token();
    } else if (token->klass == Token_True) {
      next_token();
    } else if (token->klass == Token_False) {
      next_token();
    } else if (token->klass == Token_StringLiteral) {
      next_token();
    } else if (token->klass == Token_UnaryDotprefix) {
      next_token();
      build_nonTypeName();
    } else if (token_is_nonTypeName(token)) {
      primary = (struct Cst*)build_nonTypeName();
    } else if (token->klass == Token_BraceOpen) {
      next_token();
      build_expressionList();
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
    } else if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_expression(token))
        build_expression(1);
      if (token->klass == Token_ParenthClose) {
        next_token();
      } else error("at line %d: ", token->line_nr);
    } else if (token->klass == Token_LogicNot) {
      next_token();
      build_expression(1);
    } else if (token->klass == Token_UnaryMinus) {
      next_token();
      build_expression(1);
    } else if (token_is_typeName(token)) {
      build_typeName();
    } else if (token->klass == Token_Error) {
      next_token();
    } else if (token->klass == Token_Cast) {
      next_token();
      if (token->klass == Token_ParenthOpen) {
        next_token();
        if (token_is_typeRef(token)) {
          build_typeRef();
          if (token->klass == Token_ParenthClose) {
            next_token();
            if (token_is_expression(token)) {
              build_expression(1);
            } else error("at line %d: ", token->line_nr);
          } else error("at line %d: ", token->line_nr);
        } else error("at line %d: ", token->line_nr);
      } else error("at line %d: ", token->line_nr);
    } else assert(false);
  } else error("at line %d: ", token->line_nr);
  return primary;
}

internal int
get_operator_priority(struct Token* token)
{
  int prio = 0;
  if (token->klass == Token_LogicEqual || token->klass == Token_LogicNotEqual
      || token->klass == Token_AngleOpen /* Less */ || token->klass == Token_AngleClose /* Greater */
      || token->klass == Token_LessEqual || token->klass == Token_GreaterEqual) {
    prio = 1;
  }
  else if (token->klass == Token_LogicAnd || token->klass == Token_LogicOr
           || token->klass == Token_Plus || token->klass == Token_Minus
           || token->klass == Token_BitwiseAnd || token->klass == Token_BitwiseOr) {
    prio = 2;
  }
  else if (token->klass == Token_Star || token->klass == Token_Slash) {
    prio = 3;
  }
  else assert(false);
  return prio;
}

internal struct Cst*
build_expression(int priority_threshold)
{
  struct Cst* expr = 0;
  if (token_is_expression(token)) {
    build_expressionPrimary();
    while (token_is_exprOperator(token)) {
      int priority = get_operator_priority(token);
      if (priority >= priority_threshold) {
        if (token->klass == Token_Dotprefix) {
          next_token();
          if (token_is_name(token)) {
            build_name();
          } else error("at line %d: ", token->line_nr);
        }
        else if (token->klass == Token_BracketOpen) {
          next_token();
          if (token_is_expression(token)) {
            build_expression(1);
            if (token->klass == Token_Colon) {
              next_token();
              if (token_is_expression(token)) {
                build_expression(1);
              } else error("at line %d: ", token->line_nr);
            }
            if (token->klass == Token_BracketClose) {
              next_token();
            } else error("at line %d: ", token->line_nr);
          } else error("at line %d: ", token->line_nr);
        }
        else if (token->klass == Token_ParenthOpen) {
          next_token();
          build_argumentList();
          if (token->klass == Token_ParenthClose) {
            next_token();
          } else error("at line %d: ", token->line_nr);
        }
        else if (token->klass == Token_AngleOpen) {
          next_token();
          if (token_is_realTypeArg(token)) {
            build_realTypeArgumentList();
          } else error("at line %d: ", token->line_nr);
        } else if (token_is_binaryOperator(token)){
          next_token();
          build_expression(priority_threshold + 1);
        } else assert(0);
      } else break;
    }
  } else error("at line %d: ", token->line_nr);
  return expr;
}

void
build_ast()
{
  add_keyword("action", Token_Action);
  add_keyword("actions", Token_Actions);
  add_keyword("enum", Token_Enum);
  add_keyword("in", Token_In);
  add_keyword("package", Token_Package);
  add_keyword("select", Token_Select);
  add_keyword("switch", Token_Switch);
  add_keyword("tuple", Token_Tuple);
  add_keyword("control", Token_Control);
  add_keyword("error", Token_Error);
  add_keyword("header", Token_Header);
  add_keyword("inout", Token_InOut);
  add_keyword("parser", Token_Parser);
  add_keyword("state", Token_State);
  add_keyword("table", Token_Table);
  add_keyword("key", Token_Key);
  add_keyword("typedef", Token_Typedef);
  add_keyword("default", Token_Default);
  add_keyword("extern", Token_Extern);
  add_keyword("header_union", Token_HeaderUnion);
  add_keyword("out", Token_Out);
  add_keyword("transition", Token_Transition);
  add_keyword("else", Token_Else);
  add_keyword("exit", Token_Exit);
  add_keyword("if", Token_If);
  add_keyword("match_kind", Token_MatchKind);
  add_keyword("return", Token_Return);
  add_keyword("struct", Token_Struct);
  add_keyword("apply", Token_Apply);
  add_keyword("var", Token_Var);
  add_keyword("cast", Token_Cast);
  add_keyword("const", Token_Const);
  add_keyword("bool", Token_Bool);
  add_keyword("true", Token_True);
  add_keyword("false", Token_False);
  add_keyword("void", Token_Void);
  add_keyword("int", Token_Int);
  add_keyword("bit", Token_Bit);
  add_keyword("varbit", Token_Varbit);

  token = tokenized_input;
  next_token();
  build_p4program();
}
