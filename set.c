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

SetMember*
set_lookup_member(Set* set, void* key)
{
  return search_member(set->root, key);
}

void*
set_lookup_value(Set* set, void* key, void* default_)
{
  SetMember* m;

  m = set_lookup_member(set, key);
  if (m) {
    return m->value;
  }
  return default_;
}

static SetMember*
insert_member(Arena* storage, SetMember** branch, SetMember* member, void* key, void* value)
{
  if (!member) {
    member = arena_malloc(storage, sizeof(SetMember));
    *branch = member;
    member->key = key;
    member->value = value;
    member->left_branch = 0;
    member->right_branch = 0;
    return member;
  } else if (member->key == key) {
    return 0;
  } else if (key < member->key) {
    return insert_member(storage, &member->left_branch, member->left_branch, key, value);
  } else {
    return insert_member(storage, &member->right_branch, member->right_branch, key, value);
  }
  assert(0);
  return 0;
}

SetMember*
set_add_member(Set* set, Arena* storage, void* key, void* value)
{
  return insert_member(storage, &set->root, set->root, key, value);
}

static SetMember*
search_or_insert_member(Arena* storage, SetMember** branch, SetMember* member, void* key, void* value)
{
  if (!member) {
    member = arena_malloc(storage, sizeof(SetMember));
    *branch = member;
    member->key = key;
    member->value = value;
    member->left_branch = 0;
    member->right_branch = 0;
    return member;
  } else if (member->key == key) {
    return member;
  } else if (key < member->key) {
    return search_or_insert_member(storage, &member->left_branch, member->left_branch, key, value);
  } else {
    return search_or_insert_member(storage, &member->right_branch, member->right_branch, key, value);
  }
  assert(0);
  return 0;
}

SetMember*
set_add_or_lookup_member(Set* set, Arena* storage, void* key, void* value)
{
  return search_or_insert_member(storage, &set->root, set->root, key, value);
}

Set*
set_open_inner_set(Set* set, Arena* storage, void* key)
{
  SetMember* m;
  Set* s;

  m = set_add_or_lookup_member(set, storage, key, 0);
  if (m->value == 0) {
    s = arena_malloc(storage, sizeof(Set));
    *s = (Set){};
    m->value = s;
  }
  return (Set*)m->value;
}

static void
traverse_and_collect(SetMember* member, UnboundedArray* array, Arena* storage)
{
  if (member) {
    *(SetMember**)array_append_element(array, storage, sizeof(SetMember*)) = member;
    traverse_and_collect(member->left_branch, array, storage);
    traverse_and_collect(member->right_branch, array, storage);
  }
}

int
set_members_to_array(Set* set, UnboundedArray* array, Arena* storage)
{
  array->elem_count = 0;
  if (!set->root) {
    return array->elem_count;
  }
  traverse_and_collect(set->root, array, storage);
  return array->elem_count;
}

static void
traverse_and_enumerate(SetMember* member, void (*visitor)(SetMember*))
{
  if (member) {
    visitor(member);
    traverse_and_enumerate(member->left_branch, visitor);
    traverse_and_enumerate(member->right_branch, visitor);
  }
}

void
set_enumerate_members(Set* set, void (*visitor)(SetMember*))
{
  if (!set->root) {
    return;
  }
  traverse_and_enumerate(set->root, visitor);
}

