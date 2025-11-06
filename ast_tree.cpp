#include "ast_tree.h"

void ast_tree_append_node(AstTree* tree, AstTreeCtor* ctor, AstTree* node) {
  AstTree* first_child;

  first_child = tree->first_child;
  if (first_child) {
    ctor->last_sibling->right_sibling = node;
  } else {
    tree->first_child = node;
  }
  ctor->last_sibling = node;
}
