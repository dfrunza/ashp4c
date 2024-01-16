#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <math.h>  /* floor, ceil, log2 */
#include "foundation.h"

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
  return search_member(set->root, key);
}

static void
insert_member(Set* set, Arena* storage, SetMember** branch, SetMember* member, uint64_t key)
{
  if (!member) {
    member = arena_malloc(storage, sizeof(SetMember));
    *branch = member;
    member->key = key;
    member->left_branch = 0;
    member->right_branch = 0;
  } else if (key < member->key) {
    insert_member(set, storage, &member->left_branch, member->left_branch, key);
  } else {
    insert_member(set, storage, &member->right_branch, member->right_branch, key);
  }
}

void
set_add_member(Set* set, Arena* storage, uint64_t key)
{
  insert_member(set, storage, &set->root, set->root, key);
}

