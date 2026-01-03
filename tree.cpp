#include <tree.h>

TreeConstructor::TreeConstructor()
{
  last_sibling = 0;
}

void TreeConstructor::append_node(Tree* tree, Tree* node)
{
  Tree* first_child = tree->first_child;
  if (first_child) {
    last_sibling->right_sibling = node;
  } else {
    tree->first_child = node;
  }
  last_sibling = node;
}

TreeIterator::TreeIterator()
{
  tree = 0;
}

TreeIterator::TreeIterator(Tree* root)
{
  begin(root);
}

void TreeIterator::begin(Tree* root)
{
  tree = root->first_child;
}

Tree* TreeIterator::next()
{
  Tree* result = tree;
  if (tree) {
    tree = tree->right_sibling;
  }
  return result;
}
