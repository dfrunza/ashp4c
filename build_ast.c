#define DEBUG_ENABLED 0

#include "arena.h"
#include "token.h"
#include "lex.h"
#include "build_ast.h"

enum IdentKind
{
  Ident_None,
  Ident_Keyword,
  Ident_Type,
};

struct Ident {
  enum IdentKind ident_kind;
  char* name;
  int scope_level;
  struct Ident* next_in_scope;
};  

struct Ident_Keyword {
  struct Ident;
  enum TokenClass token_klass;
};

struct Symtable_Entry {
  char* name;
  struct Ident* ns_kw;
  struct Ident* ns_type;
  struct Symtable_Entry* next;
};

internal struct Arena* arena;

internal struct Token* tokens;
internal int token_count;
internal struct Token* token = 0;
internal struct Token* prev_token = 0;

internal struct Symtable_Entry** symtable;
internal int max_symtable_len = 2003;  // table entry units
internal int scope_level = 0;

internal int node_id = 1;
internal struct AstTree* ast_tree;

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
  ast_tree->node_count += 1;
}

#define new_ast_node(type, token) ({ \
  struct Ast* ast = arena_push(arena, sizeof(struct Ast)); \
  *ast = (struct Ast){}; \
  ast->kind = type; \
  init_ast_node(ast, token); \
  ast; })

void
list_init(struct AstList* list)
{
  assert(list->head == 0);
  assert(list->tail == 0);
  list->head = &list->sentinel;
  list->tail = list->head;
}

void
list_append_link(struct AstList* list, struct AstListLink* link)
{
  assert(list->tail->next == 0);
  assert(link->prev == 0);
  list->tail->next = link;
  link->prev = list->tail;
  list->tail = link;
  list->link_count += 1;
}

internal uint32_t
name_hash(char* name, int table_len)
{
  uint32_t sum = 0;
  char* pc = name;
  while (*pc) {
    sum = (sum + (uint32_t)(*pc)*65599) % table_len;
    pc++;
  }
  return sum;
}

internal int
new_scope()
{
  int new_scope_level = ++scope_level;
  DEBUG("push scope %d\n", new_scope_level);
  return new_scope_level;
}

internal void
delete_scope()
{
  int prev_level = scope_level - 1;
  assert (prev_level >= 0);

  int i = 0;
  while (i < max_symtable_len) {
    struct Symtable_Entry* ns = symtable[i];
    while (ns) {
      struct Ident* ident = ns->ns_kw;
      if (ident && ident->scope_level > prev_level) {
        ns->ns_kw = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= prev_level);
        ident->next_in_scope = 0;
      }
      ident = ns->ns_type;
      if (ident && ident->scope_level > prev_level) {
        ns->ns_type = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= prev_level);
        ident->next_in_scope = 0;
      }
      ns = ns->next;
    }
    i++;
  }
  DEBUG("pop scope %d\n", prev_level);
  scope_level = prev_level;
}

internal bool
ident_is_declared(struct Ident* ident)
{
  bool is_declared = (ident && ident->scope_level >= scope_level);
  return is_declared;
}

internal struct Symtable_Entry*
get_symtable_entry(char* name)
{
  uint32_t h = name_hash(name, max_symtable_len);
  struct Symtable_Entry* entry = symtable[h];
  while (entry) {
    if (cstr_match(entry->name, name))
      break;
    entry = entry->next;
  }
  if (!entry) {
    entry = arena_push(arena, sizeof(struct Symtable_Entry));
    entry->name = name;
    entry->next = symtable[h];
    symtable[h] = entry;
  }
  return entry;
}

struct Ident*
new_type(char* name, int line_nr)
{
  struct Symtable_Entry* ns = get_symtable_entry(name);
  struct Ident* ident = ns->ns_type;
  if (!ident) {
    ident = arena_push(arena, sizeof(struct Ident));
    ident->name = name;
    ident->scope_level = scope_level;
    ident->ident_kind = Ident_Type;
    ident->next_in_scope = ns->ns_type;
    ns->ns_type = (struct Ident*)ident;
    DEBUG("new type `%s` at line %d.\n", ident->name, line_nr);
  }
  return ident;
}

internal struct Ident_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  struct Symtable_Entry* namespace = get_symtable_entry(name);
  assert (namespace->ns_kw == 0);
  struct Ident_Keyword* ident = arena_push(arena, sizeof(struct Ident_Keyword));
  ident->name = name;
  ident->scope_level = scope_level;
  ident->token_klass = token_klass;
  ident->ident_kind = Ident_Keyword;
  namespace->ns_kw = (struct Ident*)ident;
  return ident;
}

internal struct Token*
next_token()
{
  assert(token < tokens + token_count);
  prev_token = token++;
  while (token->klass == Token_Comment) {
    token++;
  }
  if (token->klass == Token_Identifier) {
    struct Symtable_Entry* ns = get_symtable_entry(token->lexeme);
    if (ns->ns_kw) {
      struct Ident* ident = ns->ns_kw;
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

void*
ast_getattr(struct Ast* ast, char* attr_name)
{
  uint32_t h = name_hash(attr_name, AST_ATTRTABLE_LEN);
  struct AstAttribute* entry = ast->attrs[h];
  while (entry) {
    if (cstr_match(entry->name, attr_name))
      break;
    entry = entry->next_attr;
  }
  void* attr_value = 0;
  if (entry) {
    attr_value = entry->value;
  }
  return attr_value;
}

void
ast_setattr(struct Ast* ast, char* attr_name, void* attr_value, enum AstAttributeType attr_type)
{
  uint32_t h = name_hash(attr_name, AST_ATTRTABLE_LEN);
  struct AstAttribute* entry = ast->attrs[h];
  while (entry) {
    if (cstr_match(entry->name, attr_name))
      break;
    entry = entry->next_attr;
  }
  if (!entry) {
    entry = arena_push(arena, sizeof(struct AstAttribute));
    entry->name = attr_name;
    entry->next_attr = ast->attrs[h];
    ast->attrs[h] = entry;
    ast->attr_count += 1;
  }
  entry->type = attr_type;
  entry->value = attr_value;
}

void*
ast_delattr(struct Ast* ast, char* attr_name)
{
  assert(!"TODO");
  return 0;
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
  struct Ast* name = 0;
  if (token_is_nonTypeName(token)) {
    name = new_ast_node(Ast_NonTypeName, token);
    ast_setattr(name, "name", token->lexeme, AstAttr_String);
    if (is_type) {
      new_type(ast_getattr(name, "name"), token->line_nr);
    }
    next_token();
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_nr, token->lexeme);
  return name;
}

internal struct Ast*
build_name(bool is_type)
{
  struct Ast* name = 0;
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      name = build_nonTypeName(is_type);
    } else if (token->klass == Token_TypeIdentifier) {
      struct Ast* type_name = new_ast_node(Ast_TypeName, token);
      ast_setattr(type_name, "name", token->lexeme, AstAttr_String);
      name = type_name;
      next_token();
    } else assert(0);
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return name;
}

internal struct AstList*
build_typeParameterList()
{
  struct AstList* params = 0;
  if (token_is_typeParameterList(token)) {
    params = arena_push(arena, sizeof(struct AstList));
    list_init(params);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_name(true);
    list_append_link(params, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_name(true);
      list_append_link(params, link);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return params;
}

internal struct AstList*
build_optTypeParameters()
{
  struct AstList* params = 0;
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
      struct Ast* dontcare = new_ast_node(Ast_Dontcare, token);
      arg = dontcare;
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
  enum AstParamDirection dir = AstParamDir_None;
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
  struct Ast* param = new_ast_node(Ast_Parameter, token);
  enum AstParamDirection* direction = arena_push(arena, sizeof(enum AstParamDirection));
  *direction = build_direction();
  ast_setattr(param, "direction", direction, AstAttr_Integer);
  if (token_is_typeRef(token)) {
    ast_setattr(param, "type", build_typeRef(), AstAttr_Ast);
    if (token_is_name(token)) {
      ast_setattr(param, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_Equal) {
        next_token();
        if (token_is_expression(token)) {
          ast_setattr(param, "init_expr", build_expression(1), AstAttr_Ast);
        } else error("at line %d: expression was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return param;
}

internal struct AstList*
build_parameterList()
{
  struct AstList* params = 0;
  if (token_is_parameter(token)) {
    params = arena_push(arena, sizeof(struct AstList));
    list_init(params);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_parameter();
    list_append_link(params, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
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
      type = build_typeRef();
    } else if (token->klass == Token_Void) {
      struct Ast* void_name = new_ast_node(Ast_TypeName, token);
      ast_setattr(void_name, "name", token->lexeme, AstAttr_String);
      type = void_name;
      next_token();
    } else if (token->klass == Token_Identifier) {
      struct Ast* name = new_ast_node(Ast_NonTypeName, token);
      ast_setattr(name, "name", token->lexeme, AstAttr_String);
      type = name;
      if (is_type) {
        new_type(ast_getattr(name, "name"), token->line_nr);
      }
      next_token();
    } else assert(0);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return type;
}

internal struct Ast*
build_functionPrototype(struct Ast* type_ref)
{
  struct Ast* proto = 0;
  if (token_is_typeOrVoid(token) || type_ref) {
    proto = new_ast_node(Ast_FunctionProto, token);
    if (type_ref) {
      ast_setattr(proto, "return_type", type_ref, AstAttr_Ast);
    } else {
      ast_setattr(proto, "return_type", build_typeOrVoid(true), AstAttr_Ast);
    }
    if (token_is_name(token)) {
      ast_setattr(proto, "name", build_name(false), AstAttr_Ast);
      ast_setattr(proto, "type_params", build_optTypeParameters(), AstAttr_AstList);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(proto, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: function name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return proto;
}

internal struct Ast*
build_methodPrototype()
{
  struct Ast* proto = 0;
  if (token_is_methodPrototype(token)) {
    if (token->klass == Token_TypeIdentifier && peek_token()->klass == Token_ParenthOpen) {
      /* Constructor */
      proto = new_ast_node(Ast_FunctionProto, token);
      ast_setattr(proto, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(proto, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` as expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_typeOrVoid(token)) {
      proto = build_functionPrototype(0);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return proto;
}

internal struct AstList*
build_methodPrototypes()
{
  struct AstList* protos = 0;
  if (token_is_methodPrototype(token)) {
    protos = arena_push(arena, sizeof(struct AstList));
    list_init(protos);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_methodPrototype();
    list_append_link(protos, link);
    while (token_is_methodPrototype(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
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
    struct Ast* extern_decl = new_ast_node(Ast_ExternDecl, token);
    decl = extern_decl;
    bool is_function_proto = false;
    if (token_is_typeOrVoid(token) && token_is_nonTypeName(token)) {
      is_function_proto = token_is_typeOrVoid(token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(token)) {
      is_function_proto = true;
    } else if (token_is_nonTypeName(token)) {
      is_function_proto = false;
    } else error("at line %d: extern declaration was expected, got `%s`.", token->line_nr, token->lexeme);

    if (is_function_proto) {
      decl = build_functionPrototype(0);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else {
      ast_setattr(extern_decl, "name", build_nonTypeName(true), AstAttr_Ast);
      ast_setattr(extern_decl, "type_params", build_optTypeParameters(), AstAttr_AstList);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(extern_decl, "method_protos", build_methodPrototypes(), AstAttr_AstList);
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
  struct Ast* int_node = 0;
  if (token->klass == Token_Integer) {
    int_node = new_ast_node(Ast_Int, token);
    ast_setattr(int_node, "flags", &token->i.flags, AstAttr_Integer);
    ast_setattr(int_node, "width", &token->i.width, AstAttr_Integer);
    ast_setattr(int_node, "value", &token->i.value, AstAttr_Integer);
    next_token();
  }
  return int_node;
}

internal struct Ast*
build_boolean()
{
  static int bool_true = 1;
  static int bool_false = 0;
  struct Ast* bool_node = 0;
  if (token->klass == Token_True || token->klass == Token_False) {
    bool_node = new_ast_node(Ast_Bool, token);
    ast_setattr(bool_node, "value", &bool_false, AstAttr_Integer);
    if (token->klass == Token_True) {
      ast_setattr(bool_node, "value", &bool_true, AstAttr_Integer);
    }
    next_token();
  }
  return bool_node;
}

internal struct Ast*
build_stringLiteral()
{
  struct Ast* string = 0;
  if (token->klass == Token_StringLiteral) {
    string = new_ast_node(Ast_StringLiteral, token);
    ast_setattr(string, "value", token->lexeme, AstAttr_String);
    next_token();
  }
  return string;
}

internal struct Ast*
build_integerTypeSize()
{
  struct Ast* type_size = new_ast_node(Ast_IntTypeSize, token);
  if (token->klass == Token_Integer) {
    ast_setattr(type_size, "size", build_integer(), AstAttr_Ast);
  } else if (token->klass == Token_ParenthOpen) {
    ast_setattr(type_size, "size", build_expression(1), AstAttr_Ast);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  return type_size;
}

internal struct Ast*
build_baseType()
{
  struct Ast* base_type = 0;
  enum AstBaseTypeKind* type_value = arena_push(arena, sizeof(enum AstBaseTypeKind));
  if (token_is_baseType(token)) {
    base_type = new_ast_node(Ast_BaseType, token);
    if (token->klass == Token_Bool) {
      *type_value = AstBaseType_Bool;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
    } else if (token->klass == Token_Error) {
      *type_value = AstBaseType_Error;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
    } else if (token->klass == Token_Int) {
      *type_value = AstBaseType_Int;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        ast_setattr(base_type, "size", build_integerTypeSize(), AstAttr_Ast);
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_Bit) {
      *type_value = AstBaseType_Bit;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        ast_setattr(base_type, "size", build_integerTypeSize(), AstAttr_Ast);
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_Varbit) {
      *type_value = AstBaseType_Varbit;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        ast_setattr(base_type, "size", build_integerTypeSize(), AstAttr_Ast);
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
    } else if (token->klass == Token_String) {
      *type_value = AstBaseType_String;
      ast_setattr(base_type, "base_type", type_value, AstAttr_Integer);
      next_token();
    }
    else assert(0);
  } else error("at line %d: type as expected, got `%s`.", token->line_nr, token->lexeme);
  return base_type;
}

internal struct AstList*
build_typeArgumentList()
{
  struct AstList* args = 0;
  if (token_is_typeArg(token)) {
    args = arena_push(arena, sizeof(struct AstList));
    list_init(args);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_typeArg();
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_typeArg();
      list_append_link(args, link);
    }
  }
  return args;
}

internal struct Ast*
build_tupleType()
{
  struct Ast* type = 0;
  if (token->klass == Token_Tuple) {
    next_token();
    type = new_ast_node(Ast_Tuple, token);
    if (token->klass == Token_AngleOpen) {
      next_token();
      ast_setattr(type, "type_args", build_typeArgumentList(), AstAttr_AstList);
      if (token->klass == Token_AngleClose) {
        next_token();
      } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `tuple` was expected, got `%s`.", token->line_nr, token->lexeme);
  return type;
}

internal struct Ast*
build_headerStackType()
{
  struct Ast* stack = 0;
  if (token->klass == Token_BracketOpen) {
    next_token();
    stack = new_ast_node(Ast_HeaderStack, token);
    if (token_is_expression(token)) {
      ast_setattr(stack, "stack_expr", build_expression(1), AstAttr_AstList);
      if (token->klass == Token_BracketClose) {
        next_token();
      } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: an expression expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `[` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stack;
}

internal struct Ast*
build_specializedType()
{
  struct Ast* type = 0;
  if (token->klass == Token_AngleOpen) {
    next_token();
    type = new_ast_node(Ast_SpecdType, token);
    ast_setattr(type, "type_args", build_typeArgumentList(), AstAttr_AstList);
    if (token->klass == Token_AngleClose) {
      next_token();
    } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
  return type;
}

internal struct Ast*
build_prefixedType()
{
  struct Ast* name = 0;
  bool* is_dotprefixed = arena_push(arena, sizeof(bool));
  *is_dotprefixed = false;
  if (token->klass == Token_DotPrefix) {
    next_token();
    *is_dotprefixed = true;
  }
  if (token->klass == Token_TypeIdentifier) {
    name = new_ast_node(Ast_TypeName, token);
    ast_setattr(name, "name", token->lexeme, AstAttr_String);
    ast_setattr(name, "is_dotprefixed", is_dotprefixed, AstAttr_Integer);
    next_token();
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return name;
}

internal struct Ast*
build_typeName()
{
  struct Ast* name = 0;
  if (token_is_typeName(token)) {
    name = build_prefixedType();
    if (token->klass == Token_AngleOpen) {
      struct Ast* specd_type = build_specializedType();
      ast_setattr(specd_type, "name", name, AstAttr_Ast);
      name = specd_type;
    } if (token->klass == Token_BracketOpen) {
      struct Ast* stack_type = build_headerStackType();
      ast_setattr(stack_type, "name", name, AstAttr_Ast);
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
  struct Ast* field = new_ast_node(Ast_StructField, token);
  if (token_is_typeRef(token)) {
    ast_setattr(field, "type", build_typeRef(), AstAttr_Ast);
    if (token_is_name(token)) {
      ast_setattr(field, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: struct field was expected, got `%s`.", token->line_nr, token->lexeme);
  return field;
}

internal struct AstList*
build_structFieldList()
{
  struct AstList* fields = 0;
  if (token_is_structField(token)) {
    fields = arena_push(arena, sizeof(struct AstList));
    list_init(fields);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_structField();
    while (token_is_structField(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_structField();
    }
  }
  return fields;
}

internal struct Ast*
build_headerTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Header) {
    next_token();
    decl = new_ast_node(Ast_HeaderDecl, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(decl, "fields", build_structFieldList(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token(token);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `header` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_headerUnionDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_HeaderUnion) {
    next_token();
    decl = new_ast_node(Ast_HeaderUnionDecl, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(decl, "fields", build_structFieldList(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `header_union` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_structTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Struct) {
    next_token();
    decl = new_ast_node(Ast_StructDecl, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(decl, "fields", build_structFieldList(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `struct` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
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
  struct Ast* id = 0;
  if (token_is_specifiedIdentifier(token)) {
    id = new_ast_node(Ast_SpecdId, token);
    ast_setattr(id, "name", build_name(false), AstAttr_Ast);
    if (token->klass == Token_Equal) {
      next_token();
      if (token_is_expression(token)) {
        ast_setattr(id, "init_expr", build_initializer(), AstAttr_Ast);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return id;
}

internal struct AstList*
build_specifiedIdentifierList()
{
  struct AstList* ids = 0;
  if (token_is_specifiedIdentifier(token)) {
    ids = arena_push(arena, sizeof(struct AstList));
    list_init(ids);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_specifiedIdentifier();
    list_append_link(ids, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_specifiedIdentifier();
      list_append_link(ids, link);
    }
  }
  return ids;
}

internal struct Ast*
build_enumDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Enum) {
    next_token();
    decl = new_ast_node(Ast_EnumDecl, token);
    if (token->klass == Token_Bit) {
      next_token();
      if (token->klass == Token_AngleOpen) {
        next_token();
        if (token->klass == Token_Integer) {
          ast_setattr(decl, "type_size", build_integer(), AstAttr_Ast);
          if (token->klass == Token_AngleClose) {
            next_token();
          } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: an integer was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `<` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      if (token->klass == Token_BraceOpen) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          ast_setattr(decl, "id_list", build_specifiedIdentifierList(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `enum` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
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
  struct Ast* type = 0;
  if (token->klass == Token_Parser) {
    next_token();
    type = new_ast_node(Ast_ParserType, token);
    if (token_is_name(token)) {
      ast_setattr(type, "name", build_name(true), AstAttr_Ast);
      ast_setattr(type, "type_params", build_optTypeParameters(), AstAttr_AstList);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(type, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_nr, token->lexeme);
  return type;
}

internal struct AstList*
build_optConstructorParameters()
{
  struct AstList* ctor_params = 0;
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
  struct Ast* decl = 0;
  if (token->klass == Token_Const) {
    next_token();
    decl = new_ast_node(Ast_ConstDecl, token);
    if (token_is_typeRef(token)) {
      ast_setattr(decl, "type_ref", build_typeRef(), AstAttr_Ast);
      if (token_is_name(token)) {
        ast_setattr(decl, "name", build_name(false), AstAttr_Ast);
        if (token->klass == Token_Equal) {
          next_token();
          if (token_is_expression(token)) {
            ast_setattr(decl, "expr", build_expression(1), AstAttr_Ast);
            if (token->klass == Token_Semicolon) {
              next_token();
            } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
          } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `const` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
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
      struct Ast* named_arg = new_ast_node(Ast_Argument, token);
      arg = named_arg;
      ast_setattr(named_arg, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_Equal) {
        next_token();
        if (token_is_expression(token)) {
          ast_setattr(named_arg, "init_expr", build_expression(1), AstAttr_Ast);
        } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Dontcare) {
      struct Ast* dontcare_arg = new_ast_node(Ast_Dontcare, token);
      arg = dontcare_arg;
      next_token();
    } else assert(0);
  } else error("at line %d: an argument was expected, got `%s`.", token->line_nr, token->lexeme);
  return arg;
}

internal struct AstList*
build_argumentList()
{
  struct AstList* args = 0;
  if (token_is_argument(token)) {
    args = arena_push(arena, sizeof(struct AstList));
    list_init(args);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_argument();
    list_append_link(args, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_argument();
      list_append_link(args, link);
    }
  }
  return args;
}

internal struct Ast*
build_variableDeclaration(struct Ast* type_ref)
{
  struct Ast* decl = 0;
  if (token_is_typeRef(token) || type_ref) {
    decl = new_ast_node(Ast_VarDecl, token);
    if (type_ref) {
      ast_setattr(decl, "type", type_ref, AstAttr_Ast);
    } else {
      ast_setattr(decl, "type", build_typeRef(), AstAttr_Ast);
    }
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(false), AstAttr_Ast);
      ast_setattr(decl, "init_expr", build_optInitializer(), AstAttr_Ast);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_instantiation(struct Ast* type_ref)
{
  struct Ast* inst = 0;
  if (token_is_typeRef(token) || type_ref) {
    inst = new_ast_node(Ast_Instantiation, token);
    if (type_ref) {
      ast_setattr(inst, "type_ref", type_ref, AstAttr_Ast);
    } else {
      ast_setattr(inst, "type_ref", build_typeRef(), AstAttr_Ast);
    }
    if (token->klass == Token_ParenthOpen) {
      next_token();
      ast_setattr(inst, "args", build_argumentList(), AstAttr_AstList);
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token_is_name(token)) {
          ast_setattr(inst, "name", build_name(false), AstAttr_Ast);
          if (token->klass == Token_Semicolon) {
            next_token();
          } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: instance name was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return inst;
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

internal struct AstList*
build_parserLocalElements()
{
  struct AstList* elems = 0;
  if (token_is_parserLocalElement(token)) {
    elems = arena_push(arena, sizeof(struct AstList));
    list_init(elems);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_parserLocalElement();
    list_append_link(elems, link);
    while (token_is_parserLocalElement(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_parserLocalElement();
      list_append_link(elems, link);
    }
  }
  return elems;
}

internal struct Ast*
build_directApplication(struct Ast* type_name)
{
  struct Ast* applic = 0;
  if (token_is_typeName(token) || type_name) {
    applic = new_ast_node(Ast_DirectApplic, token);
    if (type_name) {
      ast_setattr(applic, "name", type_name, AstAttr_Ast);
    } else {
      ast_setattr(applic, "name", build_typeName(), AstAttr_Ast);
    }
    if (token->klass == Token_DotPrefix) {
      next_token();
      if (token->klass == Token_Apply) {
        next_token();
        if (token->klass == Token_ParenthOpen) {
          next_token();
          ast_setattr(applic, "args", build_argumentList(), AstAttr_AstList);
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
  return applic;
}

internal struct Ast*
build_prefixedNonTypeName()
{
  struct Ast* name = 0;
  bool* is_dotprefixed = arena_push(arena, sizeof(bool));
  *is_dotprefixed = false;
  if (token->klass == Token_DotPrefix) {
    next_token();
    *is_dotprefixed = true;
  }
  if (token_is_nonTypeName) {
    name = build_nonTypeName(false);
    ast_setattr(name, "is_dotprefixed", is_dotprefixed, AstAttr_Integer);
  } else error("at line %d: non-type name was expected, ", token->line_nr, token->lexeme);
  return name;
}

internal struct Ast*
build_arrayIndex()
{
  struct Ast* index = new_ast_node(Ast_ArrayIndex, token);
  if (token_is_expression(token)) {
    ast_setattr(index, "index", build_expression(1), AstAttr_AstList);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  if (token->klass == Token_Colon) {
    next_token();
    if (token_is_expression(token)) {
      ast_setattr(index, "colon_index", build_expression(1), AstAttr_Ast);
    } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  }
  return index;
}

internal struct Ast*
build_lvalueExpr()
{
  struct Ast* expr = 0;
  if (token->klass == Token_DotPrefix) {
    next_token();
    bool* is_dotprefixed = arena_push(arena, sizeof(bool));
    *is_dotprefixed = true;
    struct Ast* dot_member = build_name(false);
    ast_setattr(dot_member, "is_dotprefixed", is_dotprefixed, AstAttr_Integer);
    expr = dot_member;
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
  struct Ast* lvalue = 0;
  if (token_is_lvalue(token)) {
    lvalue = new_ast_node(Ast_Lvalue, token);
    ast_setattr(lvalue, "name", build_prefixedNonTypeName(), AstAttr_Ast);
    if (token->klass == Token_DotPrefix || token->klass == Token_BracketOpen) {
      struct AstList* lvalue_expr = arena_push(arena, sizeof(struct AstList));
      list_init(lvalue_expr);
      struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_lvalueExpr();
      list_append_link(lvalue_expr, link);
      while (token->klass == Token_DotPrefix || token->klass == Token_BracketOpen) {
        link = arena_push(arena, sizeof(struct AstListLink));
        link->object = build_lvalueExpr();
        list_append_link(lvalue_expr, link);
      }
      ast_setattr(lvalue, "expr", lvalue_expr, AstAttr_AstList);
    }
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_nr, token->lexeme);
  return lvalue;
}

internal struct Ast*
build_assignmentOrMethodCallStatement()
{
  struct Ast* stmt = 0;
  if (token_is_lvalue(token)) {
    struct Ast* lvalue = build_lvalue();
    struct AstList* type_args = 0;
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
      struct Ast* call_stmt = new_ast_node(Ast_MethodCallStmt, token);
      ast_setattr(call_stmt, "lvalue", lvalue, AstAttr_Ast);
      ast_setattr(call_stmt, "type_args", type_args, AstAttr_AstList);
      ast_setattr(call_stmt, "args", build_argumentList(), AstAttr_AstList);
      stmt = call_stmt;
      if (token->klass == Token_ParenthClose) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Equal) {
      next_token();
      struct Ast* assgn_stmt = new_ast_node(Ast_AssignmentStmt, token);
      ast_setattr(assgn_stmt, "lvalue", lvalue, AstAttr_Ast);
      ast_setattr(assgn_stmt, "expr", build_expression(1), AstAttr_Ast);
      stmt = assgn_stmt;
    } else error("at line %d: assignment or function call was expected, got `%s`.", token->line_nr, token->lexeme);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: lvalue was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct AstList*
build_parserStatements()
{
  struct AstList* stmts = 0;
  if (token_is_parserStatement(token)) {
    stmts = arena_push(arena, sizeof(struct AstList));
    list_init(stmts);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_parserStatement();
    list_append_link(stmts, link);
    while (token_is_parserStatement(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_parserStatement();
      list_append_link(stmts, link);
    }
  }
  return stmts;
}

internal struct Ast*
build_parserBlockStatements()
{
  struct Ast* stmt = 0;
  if (token->klass == Token_BraceOpen) {
    stmt = new_ast_node(Ast_BlockStmt, token);
    next_token();
    ast_setattr(stmt, "stmt_list", build_parserStatements(), AstAttr_AstList);
    if (token->klass == Token_BraceClose) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
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
    stmt = new_ast_node(Ast_EmptyStmt, token);
  } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct AstList*
build_expressionList()
{
  struct AstList* exprs = 0;
  if (token_is_expression(token)) {
    exprs = arena_push(arena, sizeof(struct AstList));
    list_init(exprs);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_expression(1);
    list_append_link(exprs, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
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
    expr = new_ast_node(Ast_Default, token);
  } else if (token->klass == Token_Dontcare) {
    next_token();
    expr = new_ast_node(Ast_Dontcare, token);
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal struct Ast*
build_tupleKeysetExpression()
{
  struct Ast* tuple_keyset = 0;
  if (token->klass == Token_ParenthOpen) {
    tuple_keyset = new_ast_node(Ast_TupleKeyset, token);
    next_token();
    struct AstList* exprs = arena_push(arena, sizeof(struct AstList));
    list_init(exprs);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_simpleKeysetExpression();
    list_append_link(exprs, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_simpleKeysetExpression();
      list_append_link(exprs, link);
    }
    ast_setattr(tuple_keyset, "expr_list", exprs, AstAttr_AstList);
    if (token->klass == Token_ParenthClose) {
      next_token();
    } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  return tuple_keyset;
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
  struct Ast* select_case = 0;
  if (token_is_keysetExpression(token)) {
    select_case = new_ast_node(Ast_SelectCase, token);
    ast_setattr(select_case, "keyset", build_keysetExpression(), AstAttr_Ast);
    if (token->klass == Token_Colon) {
      next_token();
      if (token_is_name(token)) {
        ast_setattr(select_case, "name", build_name(false), AstAttr_Ast);
        if (token->klass == Token_Semicolon) {
          next_token();
        } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: keyset expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return select_case;
}

internal struct AstList*
build_selectCaseList()
{
  struct AstList* cases = 0;
  if (token_is_selectCase(token)) {
    cases = arena_push(arena, sizeof(struct AstList));
    list_init(cases);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_selectCase();
    list_append_link(cases, link);
    while (token_is_selectCase(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_selectCase();
      list_append_link(cases, link);
    }
  }
  return cases;
}

internal struct Ast*
build_selectExpression()
{
  struct Ast* select_expr = 0;
  if (token->klass == Token_Select) {
    next_token();
    select_expr = new_ast_node(Ast_SelectExpr, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      ast_setattr(select_expr, "expr_list", build_expressionList(), AstAttr_AstList);
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          ast_setattr(select_expr, "case_list", build_selectCaseList(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `select` was expected, got `%s`.", token->line_nr, token->lexeme);
  return select_expr;
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
  struct Ast* state = 0;
  if (token->klass == Token_State) {
    next_token();
    state = new_ast_node(Ast_ParserState, token);
    ast_setattr(state, "name", build_name(false), AstAttr_Ast);
    if (token->klass == Token_BraceOpen) {
      next_token();
      ast_setattr(state, "stmt_list", build_parserStatements(), AstAttr_AstList);
      ast_setattr(state, "trans_stmt", build_transitionStatement(), AstAttr_Ast);
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `state` was expected, got `%s`.", token->line_nr, token->lexeme);
  return state;
}

internal struct AstList*
build_parserStates()
{
  struct AstList* states = 0;
  if (token->klass == Token_State) {
    states = arena_push(arena, sizeof(struct AstList));
    list_init(states);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_parserState();
    list_append_link(states, link);
    while (token->klass == Token_State) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_parserState();
      list_append_link(states, link);
    }
  } else error("at line %d: `state` was expected, got `%s`.", token->line_nr, token->lexeme);
  return states;
}

internal struct Ast*
build_parserDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Parser) {
    decl = new_ast_node(Ast_Parser, token);
    ast_setattr(decl, "type_decl", build_parserTypeDeclaration(), AstAttr_Ast);
    if (token->klass == Token_Semicolon) {
      next_token(); /* <parserTypeDeclaration> */
    } else {
      ast_setattr(decl, "ctor_params", build_optConstructorParameters(), AstAttr_AstList);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(decl, "local_elements", build_parserLocalElements(), AstAttr_AstList);
        ast_setattr(decl, "states", build_parserStates(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: `parser` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_controlTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Control) {
    next_token();
    decl = new_ast_node(Ast_ControlType, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      ast_setattr(decl, "type_params", build_optTypeParameters(), AstAttr_AstList);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(decl, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `control` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_actionDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Action) {
    next_token();
    decl = new_ast_node(Ast_ActionDecl, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(decl, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token->klass == Token_BraceOpen) {
            ast_setattr(decl, "stmt", build_blockStatement(), AstAttr_Ast);
          } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `action` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_keyElement()
{
  struct Ast* key_elem = 0;
  if (token_is_expression(token)) {
    key_elem = new_ast_node(Ast_KeyElement, token);
    ast_setattr(key_elem, "expr", build_expression(1), AstAttr_Ast);
    if (token->klass == Token_Colon) {
      next_token();
      ast_setattr(key_elem, "name", build_name(false), AstAttr_Ast);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return key_elem;
}

internal struct AstList*
build_keyElementList()
{
  struct AstList* elems = 0;
  if (token_is_expression(token)) {
    elems = arena_push(arena, sizeof(struct AstList));
    list_init(elems);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_keyElement();
    list_append_link(elems, link);
    while (token_is_expression(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_keyElement();
      list_append_link(elems, link);
    }
  }
  return elems;
}

internal struct Ast*
build_actionRef()
{
  struct Ast* ref = 0;
  if (token->klass == Token_DotPrefix || token_is_nonTypeName(token)) {
    ref = new_ast_node(Ast_ActionRef, token);
    ast_setattr(ref, "name", build_prefixedNonTypeName(), AstAttr_Ast);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      ast_setattr(ref, "args", build_argumentList(), AstAttr_AstList);
      if (token->klass == Token_ParenthClose) {
        next_token();
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: non-type name was expected, got `%s`.", token->line_nr, token->lexeme);
  return ref;
}

internal struct AstList*
build_actionList()
{
  struct AstList* actions = 0;
  if (token_is_actionRef(token)) {
    actions = arena_push(arena, sizeof(struct AstList));
    list_init(actions);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_actionRef();
    list_append_link(actions, link);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    while (token_is_actionRef(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
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
  struct Ast* entry = 0;
  if (token_is_keysetExpression(token)) {
    entry = new_ast_node(Ast_TableEntry, token);
    ast_setattr(entry, "keyset", build_keysetExpression(), AstAttr_Ast);
    if (token->klass == Token_Colon) {
      next_token();
      ast_setattr(entry, "action", build_actionRef(), AstAttr_Ast);
      if (token->klass == Token_Semicolon) {
        next_token();
      } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: keyset was expected, got `%s`.", token->line_nr, token->lexeme);
  return entry;
}

internal struct AstList*
build_entriesList()
{
  struct AstList* entries = 0;
  if (token_is_keysetExpression(token)) {
    entries = arena_push(arena, sizeof(struct AstList));
    list_init(entries);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_entry();
    list_append_link(entries, link);
    while (token_is_keysetExpression(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
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
    bool* is_const = arena_push(arena, sizeof(bool));
    *is_const = false;
    if (token->klass == Token_Const) {
      next_token();
      *is_const = true;
    }
    if (token->klass == Token_Key) {
      next_token();
      struct Ast* key_prop = new_ast_node(Ast_TableProp_Key, token);
      prop = key_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          ast_setattr(key_prop, "keyelem_list", build_keyElementList(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Actions) {
      next_token();
      struct Ast* actions_prop = new_ast_node(Ast_TableProp_Actions, token);
      prop = actions_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          ast_setattr(actions_prop, "action_list", build_actionList(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Entries) {
      next_token();
      struct Ast* entries_prop = new_ast_node(Ast_TableProp_Entries, token);
      ast_setattr(entries_prop, "is_const", is_const, AstAttr_Integer);
      prop = entries_prop;
      if (token->klass == Token_Equal) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          ast_setattr(entries_prop, "entries", build_entriesList(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTableKwName(token)) {
      struct Ast* entry_prop = new_ast_node(Ast_TableProp_SingleEntry, token);
      ast_setattr(entry_prop, "name", build_name(false), AstAttr_Ast);
      prop = entry_prop;
      if (token->klass == Token_Equal) {
        next_token();
        ast_setattr(entry_prop, "init_expr", build_initializer(), AstAttr_Ast);
        if (token->klass == Token_Semicolon) {
          next_token();
        } else error("at line %d: `;` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `=` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else assert(0);
  } else error("at line %d: table property was expected, got `%s`.", token->line_nr, token->lexeme);
  return prop;
}

internal struct AstList*
build_tablePropertyList()
{
  struct AstList* props = 0;
  if (token_is_tableProperty(token)) {
    props = arena_push(arena, sizeof(struct AstList));
    list_init(props);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_tableProperty();
    list_append_link(props, link);
    while (token_is_tableProperty(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_tableProperty();
      list_append_link(props, link);
    }
  } else error("at line %d: table property was expected, got `%s`.", token->line_nr, token->lexeme);
  return props;
}

internal struct Ast*
build_tableDeclaration()
{
  struct Ast* table = 0;
  if (token->klass == Token_Table) {
    next_token();
    table = new_ast_node(Ast_TableDecl, token);
    ast_setattr(table, "name", build_name(false), AstAttr_Ast);
    if (token->klass == Token_BraceOpen) {
      next_token();
      ast_setattr(table, "prop_list", build_tablePropertyList(), AstAttr_AstList);
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `table` was expected, got `%s`.", token->line_nr, token->lexeme);
  return table;
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

internal struct AstList*
build_controlLocalDeclarations()
{
  struct AstList* decls = 0;
  if (token_is_controlLocalDeclaration(token)) {
    decls = arena_push(arena, sizeof(struct AstList));
    list_init(decls);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_controlLocalDeclaration();
    list_append_link(decls, link);
    while (token_is_controlLocalDeclaration(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_controlLocalDeclaration();
      list_append_link(decls, link);
    }
  }
  return decls;
}

internal struct Ast*
build_controlDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Control) {
    decl = new_ast_node(Ast_Control, token);
    ast_setattr(decl, "type_decl", build_controlTypeDeclaration(), AstAttr_Ast);
    if (token->klass == Token_Semicolon) {
      next_token(); /* <controlTypeDeclaration> */
    } else {
      ast_setattr(decl, "ctor_params", build_optConstructorParameters(), AstAttr_AstList);
      if (token->klass == Token_BraceOpen) {
        next_token();
        ast_setattr(decl, "local_decls", build_controlLocalDeclarations(), AstAttr_AstList);
        if (token->klass == Token_Apply) {
          next_token();
          ast_setattr(decl, "apply_stmt", build_blockStatement(), AstAttr_Ast);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `apply` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
    }
  } else error("at line %d: `control` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_packageTypeDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Package) {
    next_token();
    decl = new_ast_node(Ast_Package, token);
    if (token_is_name(token)) {
      ast_setattr(decl, "name", build_name(true), AstAttr_Ast);
      ast_setattr(decl, "type_params", build_optTypeParameters(), AstAttr_AstList);
      if (token->klass == Token_ParenthOpen) {
        next_token();
        ast_setattr(decl, "params", build_parameterList(), AstAttr_AstList);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `package` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_typedefDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Typedef || token->klass == Token_Type) {
    bool* is_typedef = arena_push(arena, sizeof(bool));
    *is_typedef = true;
    if (token->klass == Token_Typedef) {
      next_token();
    } else if (token->klass == Token_Type) {
      *is_typedef = false;
      next_token();
    } else assert(0);

    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      struct Ast* type_decl = new_ast_node(Ast_TypeDecl, token);
      ast_setattr(type_decl, "is_typedef", is_typedef, AstAttr_Integer);
      decl = type_decl;
      if (token_is_typeRef(token)) {
        ast_setattr(type_decl, "type_ref", build_typeRef(), AstAttr_Ast);
      } else if (token_is_derivedTypeDeclaration(token)) {
        ast_setattr(type_decl, "type_ref", build_derivedTypeDeclaration(), AstAttr_Ast);
      } else assert(0);

      if (token_is_name(token)) {
        ast_setattr(type_decl, "name", build_name(true), AstAttr_Ast);
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
  struct Ast* if_stmt = 0;
  if (token->klass == Token_If) {
    next_token();
    if_stmt = new_ast_node(Ast_IfStmt, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_expression(token)) {
        ast_setattr(if_stmt, "cond_expr", build_expression(1), AstAttr_Ast);
        if (token->klass == Token_ParenthClose) {
          next_token();
          if (token_is_statement(token)) {
            ast_setattr(if_stmt, "stmt", build_statement(0), AstAttr_Ast);
            if (token->klass == Token_Else) {
              next_token();
              if (token_is_statement(token)) {
                ast_setattr(if_stmt, "else_stmt", build_statement(0), AstAttr_Ast);
              } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
            }
          } else error("at line %d: statement was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `if` was expected, got `%s`.", token->line_nr, token->lexeme);
  return if_stmt;
}

internal struct Ast*
build_exitStatement()
{
  struct Ast* exit_stmt = 0;
  if (token->klass == Token_Exit) {
    next_token();
    exit_stmt = new_ast_node(Ast_ExitStmt, token);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `exit` was expected, got `%s`.", token->line_nr, token->lexeme);
  return exit_stmt;
}

internal struct Ast*
build_returnStatement()
{
  struct Ast* ret_stmt = 0;
  if (token->klass == Token_Return) {
    next_token();
    ret_stmt = new_ast_node(Ast_ReturnStmt, token);
    if (token_is_expression(token))
      ast_setattr(ret_stmt, "expr", build_expression(1), AstAttr_Ast);
    if (token->klass == Token_Semicolon) {
      next_token();
    } else error("at line %d: `;` expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `return` was expected, got `%s`.", token->line_nr, token->lexeme);
  return ret_stmt;
}

internal struct Ast*
build_switchLabel()
{
  struct Ast* label = 0;
  if (token_is_name(token)) {
    struct Ast* name_label = new_ast_node(Ast_SwitchLabel, token);
    label = name_label;
    ast_setattr(name_label, "name", build_name(false), AstAttr_Ast);
  } else if (token->klass == Token_Default) {
    next_token();
    label = new_ast_node(Ast_Default, token);
  } else error("at line %d: switch label was expected, got `%s`.", token->line_nr, token->lexeme);
  return label;
}

internal struct Ast*
build_switchCase()
{
  struct Ast* switch_case = 0;
  if (token_is_switchLabel(token)) {
    switch_case = new_ast_node(Ast_SwitchCase, token);
    ast_setattr(switch_case, "label", build_switchLabel(), AstAttr_Ast);
    if (token->klass == Token_Colon) {
      next_token();
      if (token->klass == Token_BraceOpen) {
        ast_setattr(switch_case, "stmt", build_blockStatement(), AstAttr_Ast);
      }
    } else error("at line %d: `:` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: switch label was expected, got `%s`.", token->line_nr, token->lexeme);
  return switch_case;
}

internal struct AstList*
build_switchCases()
{
  struct AstList* cases = 0;
  if (token_is_switchLabel(token)) {
    cases = arena_push(arena, sizeof(struct AstList));
    list_init(cases);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_switchCase();
    list_append_link(cases, link);
    while (token_is_switchLabel(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_switchCase();
      list_append_link(cases, link);
    }
  }
  return cases;
}

internal struct Ast*
build_switchStatement()
{
  struct Ast* stmt = 0;
  if (token->klass == Token_Switch) {
    next_token();
    stmt = new_ast_node(Ast_SwitchStmt, token);
    if (token->klass == Token_ParenthOpen) {
      next_token();
      ast_setattr(stmt, "expr", build_expression(1), AstAttr_Ast);
      if (token->klass == Token_ParenthClose) {
        next_token();
        if (token->klass == Token_BraceOpen) {
          next_token();
          ast_setattr(stmt, "switch_cases", build_switchCases(), AstAttr_AstList);
          if (token->klass == Token_BraceClose) {
            next_token();
          } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
        } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `(` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `switch` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
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
    stmt = new_ast_node(Ast_EmptyStmt, token);
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

internal struct AstList*
build_statementOrDeclList()
{
  struct AstList* stmts = 0;
  if (token_is_statementOrDeclaration(token)) {
    stmts = arena_push(arena, sizeof(struct AstList));
    list_init(stmts);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_statementOrDecl();
    list_append_link(stmts, link);
    while (token_is_statementOrDeclaration(token)) {
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_statementOrDecl();
      list_append_link(stmts, link);
    }
  }
  return stmts;
}

internal struct Ast*
build_blockStatement()
{
  struct Ast* stmt = 0;
  if (token->klass == Token_BraceOpen) {
    stmt = new_ast_node(Ast_BlockStmt, token);
    next_token();
    ast_setattr(stmt, "stmt_list", build_statementOrDeclList(), AstAttr_AstList);
    if (token->klass == Token_BraceClose) {
      next_token();
    } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  return stmt;
}

internal struct AstList*
build_identifierList()
{
  struct AstList* ids = 0;
  if (token_is_name(token)) {
    ids = arena_push(arena, sizeof(struct AstList));
    list_init(ids);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_name(false);
    list_append_link(ids, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_name(false);
      list_append_link(ids, link);
    }
  } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
  return ids;
}

internal struct Ast*
build_errorDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_Error) {
    next_token();
    decl = new_ast_node(Ast_Error, token);
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        ast_setattr(decl, "id_list", build_identifierList(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `error` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_matchKindDeclaration()
{
  struct Ast* decl = 0;
  if (token->klass == Token_MatchKind) {
    next_token();
    decl = new_ast_node(Ast_MatchKind, token);
    if (token->klass == Token_BraceOpen) {
      next_token();
      if (token_is_name(token)) {
        ast_setattr(decl, "id_list", build_identifierList(), AstAttr_AstList);
        if (token->klass == Token_BraceClose) {
          next_token();
        } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: `match_kind` was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
}

internal struct Ast*
build_functionDeclaration(struct Ast* type_ref)
{
  struct Ast* decl = 0;
  if (token_is_typeOrVoid(token)) {
    decl = new_ast_node(Ast_FunctionDecl, token);
    ast_setattr(decl, "proto", build_functionPrototype(type_ref), AstAttr_Ast);
    if (token->klass == Token_BraceOpen) {
      ast_setattr(decl, "stmt", build_blockStatement(), AstAttr_Ast);
    } else error("at line %d: `{` was expected, got `%s`.", token->line_nr, token->lexeme);
  } else error("at line %d: type was expected, got `%s`.", token->line_nr, token->lexeme);
  return decl;
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
  struct Ast* prog = new_ast_node(Ast_P4Program, token);
  struct AstList* decls = arena_push(arena, sizeof(struct AstList));
  list_init(decls);
  while (token_is_declaration(token) || token->klass == Token_Semicolon) {
    if (token_is_declaration(token)) {
      struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
      link->object = build_declaration();
      list_append_link(decls, link);
    } else if (token->klass == Token_Semicolon) {
      next_token(); /* empty declaration */
    }
  }
  ast_setattr(prog, "decl_list", decls, AstAttr_AstList);
  if (token->klass != Token_EndOfInput_) {
    error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
  }
  return prog;
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
    arg = new_ast_node(Ast_Dontcare, token);
  } else if (token_is_typeRef(token)) {
    arg = build_typeRef();
  } else error("at line %d: type argument was expected, got `%s`.", token->line_nr, token->lexeme);
  return arg;
}

internal struct AstList*
build_realTypeArgumentList()
{
  struct AstList* args = 0;
  if (token_is_realTypeArg(token)) {
    args = arena_push(arena, sizeof(struct AstList));
    list_init(args);
    struct AstListLink* link = arena_push(arena, sizeof(struct AstListLink));
    link->object = build_realTypeArg();
    list_append_link(args, link);
    while (token->klass == Token_Comma) {
      next_token();
      link = arena_push(arena, sizeof(struct AstListLink));
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
      bool* is_dotprefixed = arena_push(arena, sizeof(bool));
      *is_dotprefixed = true;
      if (token->klass == Token_Identifier) {
        struct Ast* name = build_nonTypeName(false);
        ast_setattr(name, "is_dotprefixed", is_dotprefixed, AstAttr_Integer);
        primary = name;
      } else if (token->klass == Token_TypeIdentifier) {
        struct Ast* name = build_typeName(false);
        ast_setattr(name, "is_dotprefixed", is_dotprefixed, AstAttr_Integer);
        primary = name;
      } else error("at line %d: unexpected token `%s`.", token->line_nr, token->lexeme);
    } else if (token_is_nonTypeName(token)) {
      primary = build_nonTypeName(false);
    } else if (token->klass == Token_BraceOpen) {
      next_token();
      struct Ast* expr_list = new_ast_node(Ast_ExpressionListExpr, token);
      ast_setattr(expr_list, "expr_list", build_expressionList(), AstAttr_AstList);
      primary = expr_list;
      if (token->klass == Token_BraceClose) {
        next_token();
      } else error("at line %d: `}` was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_ParenthOpen) {
      next_token();
      if (token_is_typeRef(token)) {
        struct Ast* cast = new_ast_node(Ast_CastExpr, token);
        ast_setattr(cast, "to_type", build_typeRef(), AstAttr_Ast);
        primary = cast;
        if (token->klass == Token_ParenthClose) {
          next_token();
          ast_setattr(cast, "expr", build_expression(1), AstAttr_Ast);
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token_is_expression(token)) {
        primary = build_expression(1);
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
    } else if (token->klass == Token_Exclamation) {
      next_token();
      struct Ast* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      enum AstExprOperator* op = arena_push(arena, sizeof(enum AstExprOperator));
      *op = AstUnOp_LogNot;
      ast_setattr(unary_expr, "op", op, AstAttr_Integer);
      ast_setattr(unary_expr, "expr", build_expression(1), AstAttr_Ast);
      primary = unary_expr;
    } else if (token->klass == Token_Tilda) {
      next_token();
      struct Ast* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      enum AstExprOperator* op = arena_push(arena, sizeof(enum AstExprOperator));
      *op = AstUnOp_BitNot;
      ast_setattr(unary_expr, "op", op, AstAttr_Integer);
      ast_setattr(unary_expr, "expr", build_expression(1), AstAttr_Ast);
      primary = unary_expr;
    } else if (token->klass == Token_UnaryMinus) {
      next_token();
      struct Ast* unary_expr = new_ast_node(Ast_UnaryExpr, token);
      enum AstExprOperator* op = arena_push(arena, sizeof(enum AstExprOperator));
      *op = AstUnOp_ArMinus;
      ast_setattr(unary_expr, "op", op, AstAttr_Integer);
      ast_setattr(unary_expr, "expr", build_expression(1), AstAttr_Ast);
      primary = unary_expr;
    } else if (token_is_typeName(token)) {
      primary = build_typeName();
    } else if (token->klass == Token_Error) {
      next_token();
      struct Ast* name = new_ast_node(Ast_NonTypeName, token);
      ast_setattr(name, "name", token->lexeme, AstAttr_Ast);
      primary = name;
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
      return AstBinOp_LogAnd;
    case Token_TwoPipe:
      return AstBinOp_LogOr;
    case Token_TwoEqual:
      return AstBinOp_LogEqual;
    case Token_ExclamationEqual:
      return AstBinOp_LogNotEqual;
    case Token_AngleOpen:
      return AstBinOp_LogLess;
    case Token_AngleClose:
      return AstBinOp_LogGreater;
    case Token_AngleOpenEqual:
      return AstBinOp_LogLessEqual;
    case Token_AngleCloseEqual:
      return AstBinOp_LogGreaterEqual;
    case Token_Plus:
      return AstBinOp_ArAdd;
    case Token_Minus:
      return AstBinOp_ArSub;
    case Token_Star:
      return AstBinOp_ArMul;
    case Token_Slash:
      return AstBinOp_ArDiv;
    case Token_Ampersand:
      return AstBinOp_BitAnd;
    case Token_Pipe:
      return AstBinOp_BitOr;
    case Token_Circumflex:
      return AstBinOp_BitXor;
    case Token_TwoAngleOpen:
      return AstBinOp_BitShLeft;
    case Token_TwoAngleClose:
      return AstBinOp_BitShRight;
    case Token_ThreeAmpersand:
      return AstBinOp_Mask;
    default: return AstOp_None;
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
        struct Ast* select_expr = new_ast_node(Ast_MemberSelectExpr, token);
        ast_setattr(select_expr, "expr", expr, AstAttr_Ast);
        expr = select_expr;
        if (token_is_name(token)) {
          ast_setattr(select_expr, "member_name", build_name(false), AstAttr_Ast);
        } else error("at line %d: name was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_BracketOpen) {
        next_token();
        struct Ast* index_expr = new_ast_node(Ast_IndexedArrayExpr, token);
        ast_setattr(index_expr, "expr", expr, AstAttr_Ast);
        ast_setattr(index_expr, "index_expr", build_arrayIndex(), AstAttr_Ast);
        expr = index_expr;
        if (token->klass == Token_BracketClose) {
          next_token();
        } else error("at line %d: `]` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_ParenthOpen) {
        next_token();
        struct Ast* call_expr = new_ast_node(Ast_FunctionCallExpr, token);
        ast_setattr(call_expr, "expr", expr, AstAttr_Ast);
        ast_setattr(call_expr, "args", build_argumentList(), AstAttr_AstList);
        expr = call_expr;
        if (token->klass == Token_ParenthClose) {
          next_token();
        } else error("at line %d: `)` was expected, got `%s`.", token->line_nr, token->lexeme);
      }
      else if (token->klass == Token_AngleOpen && token_is_realTypeArg(peek_token())) {
        next_token();
        struct Ast* args_expr = new_ast_node(Ast_TypeArgsExpr, token);
        ast_setattr(args_expr, "expr", expr, AstAttr_Ast);
        ast_setattr(args_expr, "type_args", build_realTypeArgumentList(), AstAttr_AstList);
        expr = args_expr;
        if (token->klass == Token_AngleClose) {
          next_token();
        } else error("at line %d: `>` was expected, got `%s`.", token->line_nr, token->lexeme);
      } else if (token->klass == Token_Equal) {
        next_token();
        struct Ast* kv_pair = new_ast_node(Ast_KvPair, token);
        ast_setattr(kv_pair, "name", expr, AstAttr_Ast);
        ast_setattr(kv_pair, "expr", build_expression(1), AstAttr_Ast);
        expr = kv_pair;
      }
      else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          struct Ast* bin_expr = new_ast_node(Ast_BinaryExpr, token);
          ast_setattr(bin_expr, "left_operand", expr, AstAttr_Ast);
          enum AstExprOperator* op = arena_push(arena, sizeof(enum AstExprOperator));
          *op = token_to_binop(token);
          ast_setattr(bin_expr, "op", op, AstAttr_Integer);
          next_token();
          ast_setattr(bin_expr, "right_operand", build_expression(priority + 1), AstAttr_Ast);
          expr = bin_expr;
        } else break;
      } else assert(0);
    }
  } else error("at line %d: an expression was expected, got `%s`.", token->line_nr, token->lexeme);
  return expr;
}

internal void
init_symtable()
{
  symtable = arena_push(arena, max_symtable_len*sizeof(struct Symtable_Entry*));
  int i = 0;
  while (i < max_symtable_len) {
    symtable[i++] = 0;
  }
}

struct AstTree
build_AstTree(struct TokenSequence* tksequence)
{
  tokens = tksequence->tokens;
  token_count = tksequence->count;
  arena = tksequence->arena;

  init_symtable();
  add_keyword("action", Token_Action);
  add_keyword("actions", Token_Actions);
  add_keyword("entries", Token_Entries);
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
  add_keyword("type", Token_Type);
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
  add_keyword("const", Token_Const);
  add_keyword("bool", Token_Bool);
  add_keyword("true", Token_True);
  add_keyword("false", Token_False);
  add_keyword("void", Token_Void);
  add_keyword("int", Token_Int);
  add_keyword("bit", Token_Bit);
  add_keyword("varbit", Token_Varbit);
  add_keyword("string", Token_String);

  token = tokens;
  next_token();
  ast_tree = arena_push(arena, sizeof(struct AstTree));
  ast_tree->p4program = build_p4program();
  ast_tree->arena = arena;
  return *ast_tree;
}
