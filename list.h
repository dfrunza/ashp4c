#pragma once

struct List
{
  List* next;
  List* prev;

  void insert_between(List* left, List* right);
  void insert_before(List* link);
};