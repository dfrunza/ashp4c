#pragma once

template<class T>
struct List
{
  List* next;
  List* prev;

  void insert_between(List* left, List* right);
  void insert_before(List* link);
};