template<class T>
struct List
{
  List* next;
  List* prev;

  void insert_in_between(List* left, List* right)
  {
    prev = left;
    next = right;
    if (right) {
      right->prev = this;
    }
    if (left) {
      left->next = this;
    }
  }
};