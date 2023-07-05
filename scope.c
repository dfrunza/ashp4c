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

NameDeclSlot*
scope_lookup_name(Scope* scope, char* strname)
{
  NameDeclSlot* decl_slot = 0;
  while (scope) {
    HashmapEntry* he = hashmap_lookup_entry_string(&scope->decls, strname);
    if (he && he->object) {
      decl_slot = (NameDeclSlot*)he->object;
      if (decl_slot->decls[NS_TYPE] || decl_slot->decls[NS_VAR] || decl_slot->decls[NS_KEYWORD]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return decl_slot;
}

void
declslot_push_decl(Arena* storage, Hashmap* decl_table, NameDecl* decl, enum NameSpace ns)
{
  HashmapEntry* he = hashmap_get_entry_string(decl_table, decl->strname);
  NameDeclSlot* decl_slot = he->object;
  if (!decl_slot) {
    decl_slot = arena_push_struct(storage, NameDeclSlot);
    he->object = decl_slot;
  }
  decl->next_in_slot = decl_slot->decls[ns];
  decl_slot->decls[ns] = decl;
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
    NameDeclSlot* decl_slot = entry->object;
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

