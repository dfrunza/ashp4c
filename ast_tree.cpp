#include "ast_tree.h"

void AstTreeCtor::append_node(AstTree* tree, AstTree* node) {
  AstTree* first_child;

  first_child = tree->first_child;
  if (first_child) {
    last_sibling->right_sibling = node;
  } else {
    tree->first_child = node;
  }
  last_sibling = node;
}
