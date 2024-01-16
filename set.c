#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"

Set*
set_create(Arena* storage, int max_capacity)
{
  assert(max_capacity >= 16);
  int segment_count;
  Set* set;

  segment_count = ceil(log2(max_capacity/16 + 1));
  set = arena_malloc(storage, sizeof(Set) + sizeof(SetMember*) * segment_count);
  set_init(set, storage, segment_count);
  return set;
}

void
set_init(Set* set, Arena* storage, int segment_count)
{
  assert(segment_count >= 1);

  set->entry_count = 0;
  set->capacity = 16;
  set->entries.segment_count = segment_count;
  set->entries.segments[0] = arena_malloc(storage, sizeof(SetMember) * 16);
}

static bool
search_member(SetMember* member, uint64_t key)
{
  if (!member) {
    return false;
  } else if (member->key == key){
    return true;
  } else if (key < member->key) {
    return search_member(member->left_branch, key);
  } else {
    return search_member(member->right_branch, key);
  }
  assert(0);
  return false;
}

bool
set_contains_member(Set* set, uint64_t key)
{
  SetMember* root;

  if (set->entry_count == 0) {
    return false;
  }
  root = (SetMember*)segment_locate_elem(&set->entries, 0, sizeof(SetMember));
  return search_member(root, key);
}

static void
insert_member(Set* set, SetMember** branch, SetMember* member, uint64_t key)
{
  if (!member) {
    member = (SetMember*)segment_locate_elem(&set->entries, set->entry_count, sizeof(SetMember));
    *branch = member;
    member->key = key;
    member->left_branch = 0;
    member->right_branch = 0;
    set->entry_count += 1;
  } else if (key < member->key) {
    insert_member(set, &member->left_branch, member->left_branch, key);
  } else {
    insert_member(set, &member->right_branch, member->right_branch, key);
  }
}

void
set_add_member(Set* set, uint64_t key)
{
  SetMember* root, *branch;

  if (set->entry_count == 0) {
    insert_member(set, &branch, 0, key);
    return;
  }
  root = (SetMember*)segment_locate_elem(&set->entries, 0, sizeof(SetMember));
  insert_member(set, &branch, root, key);
}

