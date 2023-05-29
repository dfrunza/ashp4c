#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

void
tyset_add_type(Arena *type_storage, TypeSet* set, Type* type)
{
  DList* set_li = arena_push_struct(type_storage, DList);
  set_li->object = type;
  dlist_concat(set->last_member, set_li);
  set->last_member = set_li;
  set->member_count += 1;
}

void
tyset_import_set(Arena *type_storage, TypeSet* to_set, TypeSet* from_set)
{
  DList* set_li = from_set->members.next;
  while (set_li) {
    tyset_add_type(type_storage, to_set, set_li->object);
    set_li = set_li->next;
  }
}

bool
tyset_contains_type(TypeSet* set, Type* target_type)
{
  DList* set_li = set->members.next;
  bool found_it = false;
  while (set_li) {
    Type* type = set_li->object;
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
    set_li = set_li->next;
  }
  return found_it;
}

