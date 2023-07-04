#include <memory.h>  // memset
#include <stdio.h>
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

Scope*
push_scope(Scope* scope, Scope* parent_scope)
{
  scope->scope_level = parent_scope->scope_level + 1;
  scope->parent_scope = parent_scope;
  return scope;
}

Scope*
pop_scope(Scope* scope)
{
  Scope* parent_scope = scope->parent_scope;
  return parent_scope;
}

NameDecl*
declare_scope_name(Arena* storage, Hashmap* decls, char* strname, enum NameSpace ns,
  int line_no, int column_no)
{
  NameDecl* decl = arena_push_struct(storage, NameDecl);
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  HashmapEntry* he = hashmap_get_entry_string(decls, strname);
  NameSpaceEntry* ns_entry = he->object;
  if (!ns_entry) { ns_entry = arena_push_struct(storage, NameSpaceEntry); }
  decl->next_in_scope = ns_entry->decls[ns];
  ns_entry->decls[ns] = decl;
  he->object = ns_entry;
  return decl;
}

NameDecl*
declare_struct_field(Arena* storage, Hashmap* fields, char* strname, int line_no, int column_no)
{
  NameDecl* decl = arena_push_struct(storage, NameDecl);
  decl->strname = strname;
  decl->line_no = line_no;
  decl->column_no = column_no;
  HashmapEntry* he = hashmap_get_entry_string(fields, strname);
  assert(!he->object);
  he->object = decl;
  return decl;
}

NameSpaceEntry*
scope_lookup_name(Scope* scope, char* strname)
{
  NameSpaceEntry* ns = 0;
  while (scope) {
    HashmapEntry* he = hashmap_lookup_entry_string(&scope->decls, strname);
    if (he && he->object) {
      ns = (NameSpaceEntry*)he->object;
      if (ns->decls[NS_TYPE] || ns->decls[NS_VAR] || ns->decls[NS_KEYWORD]) {
        break;
      }
    }
    scope = scope->parent_scope;
  }
  return ns;
}

void
Debug_print_scope_decls(Scope* scope)
{
  int count = 0;
  HashmapCursor entry_it = {};
  hashmap_cursor_reset(&entry_it, &scope->decls);
  printf("Names in scope 0x%x\n\n", scope);
  for (HashmapEntry* entry = hashmap_move_cursor(&entry_it);
       entry != 0; entry = hashmap_move_cursor(&entry_it)) {
    NameSpaceEntry* ns = entry->object;
    if (ns->decls[NS_TYPE]) {
      NameDecl* decl = ns->decls[NS_TYPE];
      while (decl) {
        printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
        decl = decl->next_in_scope;
        count += 1;
      }
    }
    if (ns->decls[NS_VAR]) {
      NameDecl* decl = ns->decls[NS_VAR];
      printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
      count += 1;
    }
  }
  printf("\nTotal: %d\n", count);
}

void
Debug_print_field_decls(Hashmap* scope)
{
  int count = 0;
  HashmapCursor entry_it = {};
  hashmap_cursor_reset(&entry_it, scope);
  printf("Names in scope 0x%x\n\n", scope);
  for (HashmapEntry* entry = hashmap_move_cursor(&entry_it);
       entry != 0; entry = hashmap_move_cursor(&entry_it)) {
    NameDecl* decl = entry->object;
    printf("%s  ...  at %d:%d\n", decl->strname, decl->line_no, decl->column_no);
  }
  printf("\nTotal: %d\n", count);
}

