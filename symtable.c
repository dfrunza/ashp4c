#define DEBUG_ENABLED 1

#include "basic.h"
#include "arena.h"
#include "token.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* symtable_storage;
internal struct UnboundedArray scope_stack = {};


struct Scope*
get_current_scope()
{
  struct Scope* scope = *(struct Scope**)array_get(&scope_stack, scope_stack.elem_count - 1);
  return scope;
}

struct Scope*
new_scope(int capacity_log2)
{
  struct Scope* new_scope = arena_push(symtable_storage, sizeof(*new_scope));
  memset(new_scope, 0, sizeof(*new_scope));
  scope_init(new_scope, 4);
  return new_scope;
}

struct Scope*
push_scope()
{
  assert (scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  struct Scope* new_scope = arena_push(symtable_storage, sizeof(*new_scope));
  memset(new_scope, 0, sizeof(*new_scope));
  array_append(&scope_stack, &new_scope);
  scope_init(new_scope, 4);
  new_scope->scope_level = current_scope->scope_level + 1;
  if (DEBUG_ENABLED) {
    printf("push scope %d\n", new_scope->scope_level);
  }
  new_scope->parent_scope = current_scope;
  return new_scope;
}

struct Scope*
pop_scope()
{
  assert (scope_stack.elem_count > 0);
  struct Scope* current_scope = get_current_scope();
  assert (current_scope->scope_level > 0);
  if (DEBUG_ENABLED) {
    printf("pop scope %d\n", current_scope->scope_level);
  }
  scope_stack.elem_count -= 1;
  current_scope = get_current_scope();
  return current_scope;
}

struct SymtableEntry*
find_symtable_entry(struct Scope* scope, char* name)
{
  uint32_t h = hash_string(name, scope->capacity_log2);
  struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
  while (entry) {
    if (cstr_match(entry->name, name))
      break;
    entry = entry->next_entry;
  }
  return entry;
}

struct SymtableEntry*
get_symtable_entry(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = find_symtable_entry(scope, name);
  if (!entry) {
    if (scope->entry_count >= scope->capacity) {
      struct Arena temp_storage = {};
      struct SymtableEntry** entries_array = arena_push(&temp_storage, scope->capacity);
      int i, j = 0;
      for (i = 0; i < scope->capacity; i++) {
        struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&scope->symtable, i);
        while (entry) {
          entries_array[j] = entry;
          struct SymtableEntry* next_entry = entry->next_entry;
          entry->next_entry = 0;
          entry = next_entry;
          j++;
        }
      }
      assert (j == scope->entry_count);
      scope->capacity = (1 << ++scope->capacity_log2) - 1;
      struct SymtableEntry* null_entry = 0;
      for (i = scope->entry_count; i < scope->capacity; i++) {
        array_append(&scope->symtable, &null_entry);
      }
      for (i = 0; i < scope->capacity; i++) {
        array_set(&scope->symtable, i, &null_entry);
      }
      for (i = 0; i < scope->entry_count; i++) {
        uint32_t h = hash_string(entries_array[i]->name, scope->capacity_log2);
        entries_array[i]->next_entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
        array_set(&scope->symtable, h, &entries_array[i]);
      }
      arena_delete(&temp_storage);
    }
    int h = hash_string(name, scope->capacity_log2);
    entry = arena_push(symtable_storage, sizeof(*entry));
    memset(entry, 0, sizeof(*entry));
    entry->name = name;
    entry->next_entry = *(struct SymtableEntry**)array_get(&scope->symtable, h);
    array_set(&scope->symtable, h, &entry);
    scope->entry_count += 1;
  }
  return entry;
}

struct SymtableEntry*
scope_resolve_name(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = 0;
  while (scope) {
    entry = get_symtable_entry(scope, name);
    if (entry->id_kw || entry->id_type || entry->id_ident) {
      break;
    }
    scope = scope->parent_scope;
  }
  return entry;
}

struct Symbol*
new_type(struct Scope* scope, char* name, struct Ast* ast, int line_nr)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, name);
  struct Symbol* id_type = arena_push(symtable_storage, sizeof(*id_type));
  memset(id_type, 0, sizeof(*id_type));
  id_type->name = name;
  id_type->ast = ast;
  id_type->symbol_kind = Symbol_Type;
  id_type->next_in_scope = entry->id_type;
  entry->id_type = (struct Symbol*)id_type;
  if (DEBUG_ENABLED) {
    printf("new type `%s` at line %d.\n", id_type->name, line_nr);
  }
  return id_type;
}

struct Symbol*
new_ident(struct Scope* scope, char* name, struct Ast* ast, int line_nr)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, name);
  struct Symbol* id_ident = arena_push(symtable_storage, sizeof(*id_ident));
  memset(id_ident, 0, sizeof(*id_ident));
  id_ident->name = name;
  id_ident->ast = ast;
  id_ident->symbol_kind = Symbol_Ident;
  id_ident->next_in_scope = entry->id_ident;
  entry->id_ident = (struct Symbol*)id_ident;
  if (DEBUG_ENABLED) {
    printf("new identifier `%s` at line %d.\n", id_ident->name, line_nr);
  }
  return id_ident;
}

internal struct Symbol_Keyword*
add_keyword(struct Scope* scope, char* name, enum TokenClass token_klass)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, name);
  assert (entry->id_kw == 0);
  struct Symbol_Keyword* id_kw = arena_push(symtable_storage, sizeof(*id_kw));
  memset(id_kw, 0, sizeof(*id_kw));
  id_kw->name = name;
  id_kw->token_klass = token_klass;
  id_kw->symbol_kind = Symbol_Keyword;
  entry->id_kw = (struct Symbol*)id_kw;
  return id_kw;
}

internal struct Symbol*
add_base_type(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, name);
  assert (entry->id_type == 0);
  struct Symbol* id_type = arena_push(symtable_storage, sizeof(*id_type));
  memset(id_type, 0, sizeof(*id_type));
  id_type->name = name;
  id_type->symbol_kind = Symbol_Type;
  entry->id_type = (struct Symbol*)id_type;
  return id_type;
}

internal void
add_all_keywords(struct Scope* scope)
{
  add_keyword(scope, "action", Token_Action);
  add_keyword(scope, "actions", Token_Actions);
  add_keyword(scope, "entries", Token_Entries);
  add_keyword(scope, "enum", Token_Enum);
  add_keyword(scope, "in", Token_In);
  add_keyword(scope, "package", Token_Package);
  add_keyword(scope, "select", Token_Select);
  add_keyword(scope, "switch", Token_Switch);
  add_keyword(scope, "tuple", Token_Tuple);
  add_keyword(scope, "control", Token_Control);
  add_keyword(scope, "error", Token_Error);
  add_keyword(scope, "header", Token_Header);
  add_keyword(scope, "inout", Token_InOut);
  add_keyword(scope, "parser", Token_Parser);
  add_keyword(scope, "state", Token_State);
  add_keyword(scope, "table", Token_Table);
  add_keyword(scope, "key", Token_Key);
  add_keyword(scope, "typedef", Token_Typedef);
  add_keyword(scope, "type", Token_Type);
  add_keyword(scope, "default", Token_Default);
  add_keyword(scope, "extern", Token_Extern);
  add_keyword(scope, "header_union", Token_HeaderUnion);
  add_keyword(scope, "out", Token_Out);
  add_keyword(scope, "transition", Token_Transition);
  add_keyword(scope, "else", Token_Else);
  add_keyword(scope, "exit", Token_Exit);
  add_keyword(scope, "if", Token_If);
  add_keyword(scope, "match_kind", Token_MatchKind);
  add_keyword(scope, "return", Token_Return);
  add_keyword(scope, "struct", Token_Struct);
  add_keyword(scope, "apply", Token_Apply);
  add_keyword(scope, "const", Token_Const);
  add_keyword(scope, "bool", Token_Bool);
  add_keyword(scope, "true", Token_True);
  add_keyword(scope, "false", Token_False);
  add_keyword(scope, "void", Token_Void);
  add_keyword(scope, "int", Token_Int);
  add_keyword(scope, "bit", Token_Bit);
  add_keyword(scope, "varbit", Token_Varbit);
  add_keyword(scope, "string", Token_String);
}

void
add_all_base_types(struct Scope* scope)
{
  add_base_type(scope, "void");
  add_base_type(scope, "bool");
  add_base_type(scope, "error");
  add_base_type(scope, "int");
  add_base_type(scope, "bit");
  add_base_type(scope, "varbit");
  add_base_type(scope, "string");
}

internal struct Symbol*
add_builtin_ident(struct Scope* scope, char* name)
{
  struct SymtableEntry* entry = get_symtable_entry(scope, name);
  assert (entry->id_ident == 0);
  struct Symbol* id_ident = arena_push(symtable_storage, sizeof(*id_ident));
  memset(id_ident, 0, sizeof(*id_ident));
  id_ident->name = name;
  id_ident->symbol_kind = Symbol_Ident;
  entry->id_ident = (struct Symbol*)id_ident;
  return id_ident;
}

void
scope_init(struct Scope* scope, int capacity_log2)
{
  struct SymtableEntry* null_entry = 0;
  array_init(&scope->symtable, sizeof(null_entry), symtable_storage);
  scope->capacity = (1 << capacity_log2) - 1;
  int i;
  for (i = scope->entry_count; i < scope->capacity; i++) {
    array_append(&scope->symtable, &null_entry);
  }
  scope->capacity_log2 = capacity_log2;
  scope->entry_count = scope->entry_count;
}

void
symtable_init()
{
  struct Scope* global_scope = arena_push(symtable_storage, sizeof(*global_scope));
  memset(global_scope, 0, sizeof(*global_scope));
  scope_init(global_scope, 5);
  add_all_keywords(global_scope);
  add_all_base_types(global_scope);
  add_builtin_ident(global_scope, "accept");
  array_init(&scope_stack, sizeof(global_scope), symtable_storage);
  array_append(&scope_stack, &global_scope);
}

void
symtable_set_storage(struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;
}

