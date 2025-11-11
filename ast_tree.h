#pragma once

struct AstTree {
  struct AstTree* first_child;
  struct AstTree* right_sibling;
};

struct AstTreeCtor {
  AstTree* last_sibling;

  void append_node(AstTree* tree, AstTree* node);
};
