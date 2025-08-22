#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"
#include "ast_tree.h"

void ast_tree_append_node(AstTree* tree, AstTreeCtor* ctor, Ast* node) {
  Ast* first_child;

  first_child = tree->first_child;
  if (first_child) {
    ctor->last_sibling->tree.right_sibling = node;
  } else {
    tree->first_child = node;
  }
  ctor->last_sibling = node;
}
