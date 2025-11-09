#pragma once

typedef struct AstTree {
  struct AstTree* first_child;
  struct AstTree* right_sibling;
} AstTree;

typedef struct AstTreeCtor {
  AstTree* last_sibling;

  void ast_tree_append_node(AstTree* tree, AstTree* node);
} AstTreeCtor;
