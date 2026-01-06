#include "adt/list.h"

void List::insert_between(List* left, List* right)
{
  this->prev = left;
  this->next = right;
  if (right) {
    right->prev = this;
  }
  if (left) {
    left->next = this;
  }
}

void List::insert_before(List* link)
{
  this->next = link;
  link->prev = this;
}