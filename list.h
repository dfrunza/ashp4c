template<class T>
struct List
{
  List* next;
  List* prev;

  void insert_in_between(List* left, List* right)
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

  void insert_before(List* link)
  {
    this->next = link;
    link->prev = this;
  }
};