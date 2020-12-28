#include "dp4c.h"

external Arena arena;
external Namespace_Entry** symtable;
external int max_symtable_len;
external int scope_level;

internal Ident_Keyword* error_kw = 0;
internal Ident_Var* error_var = 0;
internal Ident_Type* error_type = 0;

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

Ident_Type*
sym_get_type(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Type* result = (Ident_Type*)ns->ns_type;
  if (result)
    assert (result->object_kind == ID_TYPE);
  return result;
}

Ident_Type*
sym_add_type(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_Type* new_ident = arena_push_struct(&arena, Ident_Type);
  zero_struct(new_ident, Ident_Type);
  new_ident->name = name;
  new_ident->scope_level = scope_level;
  new_ident->object_kind = ID_TYPE;
  new_ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)new_ident;
  printf("add type '%s'\n", new_ident->name);
  return new_ident;
}

Ident*
sym_add_typevar(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* new_ident = arena_push_struct(&arena, Ident);
  zero_struct(new_ident, Ident);
  new_ident->name = name;
  new_ident->scope_level = scope_level;
  new_ident->object_kind = ID_TYPEVAR;
  new_ident->next_in_scope = ns->ns_type;
  ns->ns_type = new_ident;
  printf("add typevar '%s'\n", new_ident->name);
  return new_ident;
}

Ident*
sym_add_error_code(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* new_ident = arena_push_struct(&arena, Ident);
  zero_struct(new_ident, Ident);
  new_ident->name = name;
  new_ident->scope_level = scope_level;
  new_ident->object_kind = ID_STRUCT_MEMBER;
  new_ident->next_in_scope = ns->ns_global;
  ns->ns_global = new_ident;
  printf("add error '%s'\n", new_ident->name);
  return new_ident;
}

Ident_Type*
sym_get_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");
  Ident_Type* result = (Ident_Type*)ns->ns_type;
  if (result)
    assert (result->object_kind == ID_TYPE);
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
    assert (ns->ns_global == (Ident*)error_var);
    return;
  }

  error_var->scope_level = scope_level;
  error_var->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)error_var;
}

void
sym_remove_error_var()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (!ns->ns_global)
    return;

  assert (ns->ns_global == (Ident*)error_var);
  ns->ns_global = ns->ns_global->next_in_scope;
}

internal void
sym_add_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (ns->ns_type)
  {
    assert (ns->ns_type == (Ident*)error_type);
    return;
  }

  error_type->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)error_type;
}

internal void
sym_remove_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");

  if (!ns->ns_type)
    return;

  assert (ns->ns_type == (Ident*)error_type);
  ns->ns_type = ns->ns_type->next_in_scope;
}

Ident_MemberSelector*
sym_add_selector(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_MemberSelector* new_ident = arena_push_struct(&arena, Ident_MemberSelector);
  zero_struct(new_ident, Ident_MemberSelector);
  new_ident->name = name;
  new_ident->scope_level = scope_level;
  new_ident->object_kind = ID_STRUCT_MEMBER;
  new_ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)new_ident;
  printf("add selector '%s'\n", new_ident->name);
  return new_ident;
}

Ident_MemberSelector*
sym_get_selector(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident_MemberSelector* result = (Ident_MemberSelector*)ns->ns_global;
  if (result)
    assert (result->object_kind == ID_STRUCT_MEMBER);
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
  ident->object_kind = ID_KEYWORD;
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

  error_type = sym_add_type("error");
  sym_add_type("void");
  sym_add_type("bool");
  sym_add_type("bit");
  sym_add_type("varbit");
  sym_add_type("int");
  sym_add_type("string");

  error_var = arena_push_struct(&arena, Ident_Var);
  zero_struct(error_var, Ident_Var);
  error_var->name = "error";
  error_var->object_kind = ID_VAR;
  error_var->id_info = sym_get_type("error");
}

