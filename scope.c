#include <memory.h>  // memset
#include <stdio.h>
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

Scope*
scope_push(Scope* scope, Scope* parent_scope)
{
  scope->scope_level = parent_scope->scope_level + 1;
  scope->parent_scope = parent_scope;
  return scope;
}

Scope*
scope_pop(Scope* scope)
{
  Scope* parent_scope = scope->parent_scope;
  return parent_scope;
}

DeclSlot*
scope_lookup_name(Scope* scope, char* strname)
{
  DeclSlot* decl_slot = 0;
  while (scope) {
    HashmapEntry* he = hashmap_lookup_entry_stringk(&scope->decls, strname);
    if (he && he->object) {
      decl_slot = (DeclSlot*)he->object;
      if (decl_slot->decls[NS_TYPE] || decl_slot->decls[NS_VAR] || decl_slot->decls[NS_KEYWORD]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return decl_slot;
}

DeclSlot*
declslot_push_decl(Arena* storage, Hashmap* name_table, NameDecl* decl, enum NameSpace ns)
{
  HashmapEntry* he = hashmap_get_entry_stringk(name_table, decl->strname);
  DeclSlot* decl_slot = he->object;
  if (!decl_slot) {
    decl_slot = arena_push_struct(storage, sizeof(*decl_slot));
    he->object = decl_slot;
  }
  decl->next_in_slot = decl_slot->decls[ns];
  decl_slot->decls[ns] = decl;
  return decl_slot;
}

void
Debug_print_namedecls(Scope* scope)
{
  int count = 0;
  HashmapCursor entry_it = {};
  hashmap_cursor_reset(&entry_it, &scope->decls);
  printf("Names in scope 0x%x\n\n", scope);
  for (HashmapEntry* entry = hashmap_move_cursor(&entry_it);
       entry != 0; entry = hashmap_move_cursor(&entry_it)) {
    DeclSlot* decl_slot = entry->object;
    if (decl_slot->decls[NS_TYPE]) {
      NameDecl* decl = decl_slot->decls[NS_TYPE];
      while (decl) {
        printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
        decl = decl->next_in_slot;
        count += 1;
      }
    }
    if (decl_slot->decls[NS_VAR]) {
      NameDecl* decl = decl_slot->decls[NS_VAR];
      printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
      decl = decl->next_in_slot;
      count += 1;
    }
  }
  printf("\nTotal: %d\n", count);
}

