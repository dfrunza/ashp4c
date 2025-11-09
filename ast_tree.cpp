#include "ast_tree.h"

void AstTreeCtor::ast_tree_append_node(AstTree* tree, AstTree* node) {
  AstTree* first_child;

  first_child = tree->first_child;
  if (first_child) {
    this->last_sibling->right_sibling = node;
  } else {
    tree->first_child = node;
  }
  this->last_sibling = node;
}
