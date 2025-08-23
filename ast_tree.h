#pragma once

typedef struct AstTree {
  struct AstTree* first_child;
  struct AstTree* right_sibling;
} AstTree;

typedef struct AstTreeCtor {
  AstTree* last_sibling;
} AstTreeCtor;

void ast_tree_append_node(AstTree* tree, AstTreeCtor* ctor, AstTree* node);
