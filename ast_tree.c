void ast_tree_append_node(AstTree* tree, Ast* node) {
  Ast* first_child;

  first_child = tree->first_child;
  if (first_child) {
    first_child->right_sibling = node;
  } else {
    tree->first_child = node;
  }
}
