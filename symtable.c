#define DEBUG_ENABLED 0

#include "basic.h"
#include "arena.h"
#include "token.h"
#include "symtable.h"
#include <memory.h>  // memset


internal struct Arena* symtable_storage;
internal struct UnboundedArray symtable = {};
internal int symtable_capacity_log2 = 5;
internal int symtable_capacity = 0;
internal int symtable_entry_count = 0;
internal int scope_level = 0;


int
new_scope()
{
  int new_scope_level = ++scope_level;
  DEBUG("push scope %d\n", new_scope_level);
  return new_scope_level;
}

void
delete_scope()
{
  int prev_level = scope_level - 1;
  assert (prev_level >= 0);

  int i = 0;
  while (i < symtable_capacity) {
    struct SymtableEntry* ns = *(struct SymtableEntry**)array_get(&symtable, i);
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
      ns = ns->next_entry;
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

struct SymtableEntry*
get_symtable_entry(char* name)
{
  uint32_t h = hash_string(name, symtable_capacity_log2);
  struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&symtable, h);
  while (entry) {
    if (cstr_match(entry->name, name))
      break;
    entry = entry->next_entry;
  }
  if (!entry) {
    if (symtable_entry_count >= symtable_capacity) {
      struct Arena temp_storage = {};
      struct SymtableEntry** entries_array = arena_push(&temp_storage, symtable_capacity);
      int i, j = 0;
      for (i = 0; i < symtable_capacity; i++) {
        struct SymtableEntry* entry = *(struct SymtableEntry**)array_get(&symtable, i);
        while (entry) {
          entries_array[j] = entry;
          struct SymtableEntry* next_entry = entry->next_entry;
          entry->next_entry = 0;
          entry = next_entry;
          j++;
        }
      }
      assert (j == symtable_entry_count);
      symtable_capacity = (1 << ++symtable_capacity_log2) - 1;
      struct SymtableEntry* null_entry = 0;
      for (i = symtable_entry_count; i < symtable_capacity; i++) {
        array_append(&symtable, &null_entry);
      }
      for (i = 0; i < symtable_capacity; i++) {
        array_set(&symtable, i, &null_entry);
      }
      for (i = 0; i < symtable_entry_count; i++) {
        uint32_t h = hash_string(entries_array[i]->name, symtable_capacity_log2);
        entries_array[i]->next_entry = *(struct SymtableEntry**)array_get(&symtable, h);
        array_set(&symtable, h, &entries_array[i]);
      }
      arena_delete(&temp_storage);
      h = hash_string(name, symtable_capacity_log2);
    }
    entry = arena_push(symtable_storage, sizeof(struct SymtableEntry));
    memset(entry, 0, sizeof(*entry));
    entry->name = name;
    entry->next_entry = *(struct SymtableEntry**)array_get(&symtable, h);
    array_set(&symtable, h, &entry);
    symtable_entry_count += 1;
  }
  return entry;
}

struct Ident*
new_type(char* name, int line_nr)
{
  struct SymtableEntry* ns = get_symtable_entry(name);
  struct Ident* ident = ns->ns_type;
  if (!ident) {
    ident = arena_push(symtable_storage, sizeof(struct Ident));
    memset(ident, 0, sizeof(*ident));
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
  struct SymtableEntry* namespace = get_symtable_entry(name);
  assert (namespace->ns_kw == 0);
  struct Ident_Keyword* ident = arena_push(symtable_storage, sizeof(struct Ident_Keyword));
  memset(ident, 0, sizeof(*ident));
  ident->name = name;
  ident->scope_level = scope_level;
  ident->token_klass = token_klass;
  ident->ident_kind = Ident_Keyword;
  namespace->ns_kw = (struct Ident*)ident;
  return ident;
}

void
symtable_init(struct Arena* symtable_storage_)
{
  symtable_storage = symtable_storage_;

  array_init(&symtable, sizeof(struct SymtableEntry*), symtable_storage);
  symtable_capacity = (1 << symtable_capacity_log2) - 1;
  struct SymtableEntry* null_entry = 0;
  int i;
  for (i = symtable_entry_count; i < symtable_capacity; i++) {
    array_append(&symtable, &null_entry);
  }

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

