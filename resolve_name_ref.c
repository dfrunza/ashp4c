#include "arena.h"
#include "ast.h"
#include "symtable.h"

void
resolve_name_ref(struct Scope* scope)
{
  struct HashmapEntryIterator it = {};
  hashmap_iter_init(&it, &scope->symtable);
  struct HashmapEntry* hmap_entry = hashmap_iter_next(&it);
  while (hmap_entry) {
    struct SymtableEntry* sym_entry = hmap_entry->object;
    // printf("%s\n", sym_entry->name);
    if (sym_entry->ns_general) {
      if (sym_entry->ns_general->object_kind == Object_NameRef) {
        int x = 0;
      }
    }
    hmap_entry = hashmap_iter_next(&it);
  }
}
