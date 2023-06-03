#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

void
tyset_add_type(Arena *type_storage, TypeSet* set, Type* type)
{
  DListItem* new_li = arena_push_struct(type_storage, DListItem);
  new_li->object = type;
  list_append_item(&set->members, new_li, 1);
}

void
tyset_import_set(Arena *type_storage, TypeSet* to_set, TypeSet* from_set)
{
  for (DListItem* li = from_set->members.sentinel.next;
       li != 0; li = li->next) {
    tyset_add_type(type_storage, to_set, li->object);
    ;
  }
}

bool
tyset_contains_type(TypeSet* set, Type* target_type)
{
  bool found_it = false;
  for (DListItem* li = set->members.sentinel.next;
       li != 0; li = li->next) {
    Type* type = li->object;
    if (type == target_type) {
      found_it = true;
      break;
    }
    if (type->ctor == target_type->ctor) {
      if (type->ctor == TYPE_VOID || type->ctor == TYPE_BOOL ||
          type->ctor == TYPE_INT || type->ctor == TYPE_BIT ||
          type->ctor == TYPE_VARBIT || type->ctor == TYPE_STRING) {
        found_it = true;
        break;
      }
    }
  }
  return found_it;
}

