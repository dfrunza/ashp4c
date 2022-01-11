#include "arena.h"
#include "ast.h"
#include "symtable.h"

void
name_resolve_root(struct Scope* scope)
{
  struct HashmapEntryIterator it = {};
  hashmap_iter_init(&it, &scope->symtable);
  struct HashmapEntry* hmap_entry = hashmap_iter_next(&it);
  while (hmap_entry) {
    struct SymtableEntry* sym_entry = hmap_entry->object;
    // printf("%s\n", sym_entry->name);
    if (sym_entry->id_ident) {
      if (sym_entry->id_ident->object_kind == Object_NameRef) {
        int x = 0;
      }
    }
    hmap_entry = hashmap_iter_next(&it);
  }
}
