#include <stdio.h>
#include <stdint.h>
#include "foundation.h"

static SetMember*
search_member(SetMember* member, uint64_t key)
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
set_lookup_member(Set* set, uint64_t key)
{
  return search_member(set->root, key);
}

static SetMember*
insert_member(Set* set, Arena* storage, SetMember** branch, SetMember* member, uint64_t key, uint64_t value)
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
    return insert_member(set, storage, &member->left_branch, member->left_branch, key, value);
  } else {
    return insert_member(set, storage, &member->right_branch, member->right_branch, key, value);
  }
  assert(0);
  return 0;
}

SetMember*
set_add_member(Set* set, Arena* storage, uint64_t key, uint64_t value)
{
  return insert_member(set, storage, &set->root, set->root, key, value);
}

static SetMember*
search_or_insert_member(Set* set, Arena* storage, SetMember** branch, SetMember* member, uint64_t key, uint64_t value)
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
    return insert_member(set, storage, &member->left_branch, member->left_branch, key, value);
  } else {
    return insert_member(set, storage, &member->right_branch, member->right_branch, key, value);
  }
  assert(0);
  return 0;
}

SetMember*
set_lookup_or_add_member(Set* set, Arena* storage, uint64_t key, uint64_t value)
{
  return search_or_insert_member(set, storage, &set->root, set->root, key, value);
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

void
set_members_to_array(Set* set, UnboundedArray* array, Arena* storage)
{
  array->elem_count = 0;
  if (!set->root) {
    return;
  }
  traverse_and_collect(set->root, array, storage);
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

