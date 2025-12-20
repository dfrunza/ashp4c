template<class T>
struct Tree {
  Tree* first_child;
  Tree* right_sibling;
};

template<class T>
struct TreeConstructor {
  Tree<T>* last_sibling;

  TreeConstructor()
  {
    last_sibling = 0;
  }

  void append_node(Tree<T>* tree, Tree<T>* node) {
    Tree<T>* first_child = tree->first_child;
    if (first_child) {
      last_sibling->right_sibling = node;
    } else {
      tree->first_child = node;
    }
    last_sibling = node;
  }
};

template<class T>
struct TreeIterator {
  Tree<T>* tree;

  TreeIterator()
  {
    tree = 0;
  }

  TreeIterator(Tree<T>* root)
  {
    begin(root);
  }

  void begin(Tree<T>* root)
  {
    tree = root->first_child;
  }

  Tree<T>* next()
  {
    Tree<T>* result = tree;
    if (tree) {
      tree = tree->right_sibling;
    }
    return result;
  }
};
