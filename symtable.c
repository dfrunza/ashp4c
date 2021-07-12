#define DEBUG_ENABLED 1

#include "basic.h"
#include "arena.h"
#include "token.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* symtable_storage;
internal struct UnboundedArray symtable = {};
internal int capacity_log2 = 5;
internal int capacity = 0;
internal int entry_count = 0;
internal int scope_level = 0;


int
push_scope()
{
  int new_scope_level = ++scope_level;
  DEBUG("push scope %d\n", new_scope_level);
  return new_scope_level;
}

internal struct Symbol*
scope_delete_symbol(struct Symbol* symbol)
{
  struct Symbol* next_in_scope = 0;
  if (symbol && symbol->scope_level > (scope_level - 1)) {
    next_in_scope = symbol->next_in_scope;
    if (next_in_scope) {
      assert (next_in_scope->scope_level <= scope_level);
    }
    symbol->next_in_scope = 0;
  }
  return next_in_scope;
}

void
pop_scope()
{
  assert (scope_level > 0);
  int i;
  for (i = 0; i < capacity; i++) {
    struct SymtableEntry* symbol = *(struct SymtableEntry**)array_get(&symtable, i);
    while (symbol) {
      symbol->id_kw = scope_delete_symbol(symbol->id_kw);
      symbol->id_type = scope_delete_symbol(symbol->id_type);
      symbol->id_ident = scope_delete_symbol(symbol->id_ident);
      symbol = symbol->next_entry;
    }
  }
  DEBUG("pop scope %d\n", scope_level - 1);
  scope_level -= 1;
}

struct SymtableEntry*
get_symtable_entry(char* name)
{
  uint32_t h = hash_string(name, capacity_log2);
  struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&symtable, h);
  while (entry) {
    if (cstr_match(entry->name, name))
      break;
    entry = entry->next_entry;
  }
  if (!entry) {
    if (entry_count >= capacity) {
      struct Arena temp_storage = {};
      struct SymtableEntry** entries_array = arena_push(&temp_storage, capacity);
      int i, j = 0;
      for (i = 0; i < capacity; i++) {
        struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&symtable, i);
        while (entry) {
          entries_array[j] = entry;
          struct SymtableEntry* next_entry = entry->next_entry;
          entry->next_entry = 0;
          entry = next_entry;
          j++;
        }
      }
      assert (j == entry_count);
      capacity = (1 << ++capacity_log2) - 1;
      struct SymtableEntry* null_entry = 0;
      for (i = entry_count; i < capacity; i++) {
        array_append(&symtable, &null_entry);
      }
      for (i = 0; i < capacity; i++) {
        array_set(&symtable, i, &null_entry);
      }
      for (i = 0; i < entry_count; i++) {
        uint32_t h = hash_string(entries_array[i]->name, capacity_log2);
        entries_array[i]->next_entry = *(struct SymtableEntry**)array_get(&symtable, h);
        array_set(&symtable, h, &entries_array[i]);
      }
      arena_delete(&temp_storage);
      h = hash_string(name, capacity_log2);
    }
    entry = arena_push(symtable_storage, sizeof(*entry));
    memset(entry, 0, sizeof(*entry));
    entry->name = name;
    entry->next_entry = *(struct SymtableEntry**)array_get(&symtable, h);
    array_set(&symtable, h, &entry);
    entry_count += 1;
  }
  return entry;
}

bool
name_is_declared(char* name, enum SymbolKind kind)
{
  bool is_declared = false;
  struct SymtableEntry* symbol = get_symtable_entry(name);
  if (symbol) {
    if (kind == Symbol_Keyword) {
      is_declared = symbol->id_kw && (symbol->id_kw->scope_level >= scope_level);
    } else if (kind == Symbol_Type) {
      is_declared = symbol->id_type && (symbol->id_type->scope_level >= scope_level);
    } else if (kind == Symbol_Ident) {
      is_declared = symbol->id_ident && (symbol->id_ident->scope_level >= scope_level);
    } else assert(0);
  }
  return is_declared;
}

struct Symbol*
new_type(char* name, struct Ast* ast, int line_nr)
{
  struct SymtableEntry* symbol = get_symtable_entry(name);
  struct Symbol* id_type = arena_push(symtable_storage, sizeof(*id_type));
  memset(id_type, 0, sizeof(*id_type));
  id_type->name = name;
  id_type->scope_level = scope_level;
  id_type->ast = ast;
  id_type->ident_kind = Symbol_Type;
  id_type->next_in_scope = symbol->id_type;
  symbol->id_type = (struct Symbol*)id_type;
  DEBUG("new type `%s` at line %d.\n", id_type->name, line_nr);
  return id_type;
}

struct Symbol*
new_ident(char* name, struct Ast* ast, int line_nr)
{
  struct SymtableEntry* symbol = get_symtable_entry(name);
  struct Symbol* id_ident = arena_push(symtable_storage, sizeof(*id_ident));
  memset(id_ident, 0, sizeof(*id_ident));
  id_ident->name = name;
  id_ident->scope_level = scope_level;
  id_ident->ast = ast;
  id_ident->ident_kind = Symbol_Ident;
  id_ident->next_in_scope = symbol->id_ident;
  symbol->id_ident = (struct Symbol*)id_ident;
  DEBUG("new identifier `%s` at line %d.\n", id_ident->name, line_nr);
  return id_ident;
}

internal struct Symbol_Keyword*
add_keyword(char* name, enum TokenClass token_klass)
{
  struct SymtableEntry* symbol = get_symtable_entry(name);
  assert (symbol->id_kw == 0);
  struct Symbol_Keyword* id_kw = arena_push(symtable_storage, sizeof(*id_kw));
  memset(id_kw, 0, sizeof(*id_kw));
  id_kw->name = name;
  id_kw->scope_level = scope_level;
  id_kw->token_klass = token_klass;
  id_kw->ident_kind = Symbol_Keyword;
  symbol->id_kw = (struct Symbol*)id_kw;
  return id_kw;
}

internal void
add_all_keywords()
{
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
}

void
symtable_init()
{
  struct SymtableEntry* null_entry = 0;
  array_init(&symtable, sizeof(null_entry), symtable_storage);
  capacity = (1 << capacity_log2) - 1;
  int i;
  for (i = entry_count; i < capacity; i++) {
    array_append(&symtable, &null_entry);
  }
  add_all_keywords();
}

void
symtable_flush()
{
  arena_delete(symtable_storage);
  entry_count = 0;
  scope_level = 0;
  symtable_init();
}

void
symtable_set_storage(struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;
}

