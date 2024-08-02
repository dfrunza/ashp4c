#include <stdio.h>
#include <stdint.h>
#include "foundation.h"

static SetMember*
search_member(SetMember* member, void* key)
{
  if (!member) {
    return 0;
  } else if (member->key == key) {
    return member;
  } else if (key < member->key) {
    return search_member(member->left_branch, key);
  } else {
    return search_member(member->right_branch, key);
  }
  assert(0);
  return 0;
}

static SetMember*
add_member(Set* set, Arena* storage, SetMember** branch, SetMember* member,
           void* key, void* value, bool return_if_found)
{
  if (!member) {
    member = arena_malloc(storage, sizeof(SetMember));
    *branch = member;
    member->key = key;
    member->value = value;
    member->left_branch = 0;
    member->right_branch = 0;
    member->next = set->first;
    set->first = member;
    return member;
  } else if (member->key == key) {
    if (return_if_found) { return member; } else { return 0; }
  } else if (key < member->key) {
    return add_member(set, storage, &member->left_branch, member->left_branch,
                                  key, value, return_if_found);
  } else {
    return add_member(set, storage, &member->right_branch, member->right_branch,
                                  key, value, return_if_found);
  }
  assert(0);
  return 0;
}

void*
set_lookup(Set* set, void* key, void* default_, SetMember** member)
{
  SetMember* m;
  void* value;

  m = search_member(set->root, key);
  value = default_;
  if (m) {
    value = m->value;
  }
  if (member) {
    *member = m;
  }
  return value;
}

SetMember*
set_add(Set* set, Arena* storage, void* key, void* value, bool return_if_found)
{
  return add_member(set, storage, &set->root, set->root, key, value, return_if_found);
}

Set*
set_create_inner(Set* set, Arena* storage, void* key)
{
  SetMember* m;
  Set* s;

  m = set_add(set, storage, key, 0, 1);
  if (m->value == 0) {
    s = arena_malloc(storage, sizeof(Set));
    *s = (Set){0};
    m->value = s;
  }
  return (Set*)m->value;
}

