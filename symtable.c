#include "dp4c.h"

external Arena arena;
external Namespace_Entry** symtable;
external int max_symtable_len;
external int scope_level;

internal Ident_Keyword* error_kw = 0;
internal Ident_Var* error_var_ident = 0;
external Ast_Ident* error_type_ast;
Ident_Type* error_type_ident = 0;
external Ast_Ident* void_type_ast;
Ident_Type* void_type_ident = 0;
external Ast_Ident* bool_type_ast;
Ident_Type* bool_type_ident = 0;
external Ast_Ident* bit_type_ast;
Ident_Type* bit_type_ident = 0;
external Ast_Ident* varbit_type_ast;
Ident_Type* varbit_type_ident;
external Ast_Ident* int_type_ast;
Ident_Type* int_type_ident;
external Ast_Ident* string_type_ast;
Ident_Type* string_type_ident;

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
scope_pop_level()
{
  int i = 0;
  while (i < max_symtable_len)
  {
    Namespace_Entry* ns = symtable[i];
    while (ns)
    {
      Ident* ident = ns->ns_global;
      if (ident && ident->scope_level >= scope_level)
      {
        ns->ns_global = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level < scope_level);
        ident->next_in_scope = 0;
      }
      ident = ns->ns_type;
      if (ident && ident->scope_level >= scope_level)
      {
        ns->ns_type = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level < scope_level);
        ident->next_in_scope = 0;
      }
      ns = ns->next;
    }
    i++;
  }
  printf("pop scope %d\n", scope_level);
  return --scope_level;
}

Namespace_Entry*
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
    zero_struct(name_info, Namespace_Entry);
    name_info->name = name;
    name_info->next = symtable[h];
    symtable[h] = name_info;
  }
  return name_info;
}

Ident_Var*
sym_get_var(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Var* ident_var = (Ident_Var*)ns->ns_global;
  if (ident_var)
    assert (ident_var->ident_kind == ID_VAR);
  return ident_var;
}

Ident_Var*
sym_add_var(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Var* ident = arena_push_struct(&arena, Ident_Var);
  zero_struct(ident, Ident_Var);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_VAR;
  ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)ident;
  printf("add var '%s'\n", ident->name);
  return ident;
}

Ident_Type*
sym_get_type(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Type* result = (Ident_Type*)ns->ns_type;
  if (result)
    assert (result->ident_kind == ID_TYPE || result->ident_kind == ID_TYPEVAR);
  return result;
}

Ident_Type*
sym_add_type(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Type* ident = arena_push_struct(&arena, Ident_Type);
  zero_struct(ident, Ident_Type);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPE;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("add type '%s'\n", ident->name);
  return ident;
}

Ident_Type*
sym_add_typevar(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Type* ident = arena_push_struct(&arena, Ident_Type);
  zero_struct(ident, Ident_Type);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPEVAR;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("add typevar '%s'\n", ident->name);
  return ident;
}

Ident_Type*
sym_get_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");
  Ident_Type* result = (Ident_Type*)ns->ns_type;
  if (result)
    assert (result->ident_kind == ID_TYPE);
  return result;
}

void
sym_remove_error_kw()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (!ns->ns_global)
    return;

  assert (ns->ns_global == (Ident*)error_kw);
  ns->ns_global = ns->ns_global->next_in_scope;
}

void
sym_add_error_kw()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (ns->ns_global)
  {
    assert (ns->ns_global == (Ident*)error_kw);
    return;
  }

  error_kw->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)error_kw;
}

void
sym_add_error_var()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (ns->ns_global)
  {
    assert (ns->ns_global == (Ident*)error_var_ident);
    return;
  }

  error_var_ident->scope_level = scope_level;
  error_var_ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)error_var_ident;
}

void
sym_remove_error_var()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (!ns->ns_global)
    return;

  assert (ns->ns_global == (Ident*)error_var_ident);
  ns->ns_global = ns->ns_global->next_in_scope;
}

internal void
sym_add_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (ns->ns_type)
  {
    assert (ns->ns_type == (Ident*)error_type_ident);
    return;
  }

  error_type_ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)error_type_ident;
}

internal void
sym_remove_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (!ns->ns_type)
    return;

  assert (ns->ns_type == (Ident*)error_type_ident);
  ns->ns_type = ns->ns_type->next_in_scope;
}

Ident_MemberSelector*
sym_add_selector(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_MemberSelector* ident = arena_push_struct(&arena, Ident_MemberSelector);
  zero_struct(ident, Ident_MemberSelector);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_STRUCT_MEMBER;
  ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)ident;
  printf("add selector '%s'\n", ident->name);
  return ident;
}

Ident_MemberSelector*
sym_get_selector(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_MemberSelector* result = (Ident_MemberSelector*)ns->ns_global;
  if (result)
    assert (result->ident_kind == ID_STRUCT_MEMBER);
  return result;
}

internal Ident_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  Namespace_Entry* namespace = sym_get_namespace(name);
  assert (namespace->ns_global == 0);
  Ident_Keyword* ident = arena_push_struct(&arena, Ident_Keyword);
  zero_struct(ident, Ident_Keyword);
  ident->name = name;
  ident->scope_level = scope_level;
  ident->token_klass = token_klass;
  ident->ident_kind = ID_KEYWORD;
  namespace->ns_global = (Ident*)ident;
  return ident;
}

void
sym_init()
{
  add_keyword("action", TOK_KW_ACTION);
  add_keyword("const", TOK_KW_CONST);
  add_keyword("enum", TOK_KW_ENUM);
  add_keyword("in", TOK_KW_IN);
  add_keyword("package", TOK_KW_PACKAGE);
  add_keyword("select", TOK_KW_SELECT);
  add_keyword("switch", TOK_KW_SWITCH);
  add_keyword("tuple", TOK_KW_TUPLE);
  add_keyword("control", TOK_KW_CONTROL);
  error_kw = add_keyword("error", TOK_KW_ERROR);
  add_keyword("header", TOK_KW_HEADER);
  add_keyword("inout", TOK_KW_INOUT);
  add_keyword("parser", TOK_KW_PARSER);
  add_keyword("state", TOK_KW_STATE);
  add_keyword("table", TOK_KW_TABLE);
  add_keyword("typedef", TOK_KW_TYPEDEF);
  add_keyword("default", TOK_KW_DEFAULT);
  add_keyword("extern", TOK_KW_EXTERN);
  add_keyword("header_union", TOK_KW_HEADER_UNION);
  add_keyword("out", TOK_KW_OUT);
  add_keyword("transition", TOK_KW_TRANSITION);
  add_keyword("else", TOK_KW_ELSE);
  add_keyword("exit", TOK_KW_EXIT);
  add_keyword("if", TOK_KW_IF);
  add_keyword("match_kind", TOK_KW_MATCH_KIND);
  add_keyword("return", TOK_KW_RETURN);
  add_keyword("struct", TOK_KW_STRUCT);
  add_keyword("apply", TOK_KW_APPLY);
  //add_keyword("verify", TOK_KW_VERIFY);

  error_type_ident = sym_add_type(error_type_ast->name, (Ast*)error_type_ast);
  void_type_ident = sym_add_type(void_type_ast->name, (Ast*)void_type_ast);
  bool_type_ident = sym_add_type("bool", (Ast*)bool_type_ast);
  bit_type_ident = sym_add_type("bit", (Ast*)bit_type_ast);
  varbit_type_ident = sym_add_type("varbit", (Ast*)varbit_type_ident);
  int_type_ident = sym_add_type("int", (Ast*)int_type_ident);
  string_type_ident = sym_add_type("string", (Ast*)string_type_ident);

  error_var_ident = arena_push_struct(&arena, Ident_Var);
  zero_struct(error_var_ident, Ident_Var);
  error_var_ident->name = "error";
  error_var_ident->ident_kind = ID_VAR;
  error_var_ident->type_ident = sym_get_type("error");
}

