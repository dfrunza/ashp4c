#include "dp4c.h"

external Arena arena;
external Namespace_Entry** symtable;
external int max_symtable_len;
external int scope_level;

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
scope_pop_level(int to_level)
{
  if (scope_level <= to_level)
    return scope_level;

  int i = 0;
  while (i < max_symtable_len)
  {
    Namespace_Entry* ns = symtable[i];
    while (ns)
    {
      Ident* ident = ns->ns_global;
      if (ident && ident->scope_level > to_level)
      {
        ns->ns_global = ident->next_in_scope;
        if (ident->next_in_scope)
          assert (ident->next_in_scope->scope_level <= to_level);
        ident->next_in_scope = 0;
      }
      ident = ns->ns_type;
      if (ident && ident->scope_level > to_level)
      {
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

bool
sym_ident_is_declared(Ident* ident)
{
  bool is_declared = (ident && ident->scope_level >= scope_level);
  return is_declared;
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
    name_info->name = name;
    name_info->next = symtable[h];
    symtable[h] = name_info;
  }
  return name_info;
}

Ident*
sym_get_var(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident_var = (Ident*)ns->ns_global;
  if (ident_var)
    assert (ident_var->ident_kind == ID_VAR);
  return ident_var;
}

Ident*
sym_new_var(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_VAR;
  ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)ident;
  printf("new var '%s'\n", ident->name);
  return ident;
}

void
sym_import_var(Ident* var_ident)
{
  Namespace_Entry* ns = sym_get_namespace(var_ident->name);

  if (ns->ns_global)
  {
    assert (ns->ns_global == (Ident*)var_ident);
    return;
  }

  var_ident->next_in_scope = ns->ns_global;
  ns->ns_global = (Ident*)var_ident;
}

void
sym_unimport_var(Ident* var_ident)
{
  Namespace_Entry* ns = sym_get_namespace(var_ident->name);

  if (!ns->ns_global)
    return;

  assert (ns->ns_global == (Ident*)var_ident);
  ns->ns_global = ns->ns_global->next_in_scope;
}

Ident*
sym_get_type(char* name)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* result = (Ident*)ns->ns_type;
  if (result)
    assert (result->ident_kind == ID_TYPE || result->ident_kind == ID_TYPEVAR);
  return result;
}

Ident*
sym_new_type(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPE;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("new type '%s'\n", ident->name);
  return ident;
}

Ident*
sym_new_typevar(char* name, Ast* ast)
{
  Namespace_Entry* ns = sym_get_namespace(name);
  Ident* ident = arena_push_struct(&arena, Ident);
  ident->ast = ast;
  ident->name = name;
  ident->scope_level = scope_level;
  ident->ident_kind = ID_TYPEVAR;
  ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)ident;
  printf("new typevar '%s'\n", ident->name);
  return ident;
}

Ident*
sym_get_error_type()
{
  Namespace_Entry* ns = sym_get_namespace("error");
  Ident* result = (Ident*)ns->ns_type;
  if (result)
    assert (result->ident_kind == ID_TYPE);
  return result;
}

void
sym_import_type(Ident* type_ident)
{
  Namespace_Entry* ns = sym_get_namespace(type_ident->name);

  if (ns->ns_type)
  {
    assert (ns->ns_type == (Ident*)type_ident);
    return;
  }

  type_ident->next_in_scope = ns->ns_type;
  ns->ns_type = (Ident*)type_ident;
}

void
sym_unimport_type(Ident* type_ident)
{
  Namespace_Entry* ns = sym_get_namespace(type_ident->name);

  if (!ns->ns_type)
    return;

  assert (ns->ns_type == (Ident*)type_ident);
  ns->ns_type = ns->ns_type->next_in_scope;
}

Ident_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  Namespace_Entry* namespace = sym_get_namespace(name);
  assert (namespace->ns_global == 0);
  Ident_Keyword* ident = arena_push_struct(&arena, Ident_Keyword);
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
}

