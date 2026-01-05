#pragma once

struct Tree {
  Tree* first_child;
  Tree* right_sibling;
};

struct TreeConstructor {
  Tree* last_sibling;

  TreeConstructor();
  void append_node(Tree* tree, Tree* node);
};

struct TreeIterator {
  Tree* tree;

  TreeIterator();
  TreeIterator(Tree* root);
  void begin(Tree* root);
  Tree* next();
};