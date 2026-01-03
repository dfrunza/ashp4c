#include <list.h>

template<class T>
void List<T>::insert_between(List<T>* left, List<T>* right)
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

template<class T>
void List<T>::insert_before(List<T>* link)
{
  this->next = link;
  link->prev = this;
}

#include <arena.h>
template struct List<PageBlock>;