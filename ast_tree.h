typedef struct Ast Ast;

typedef struct AstTree {
  struct Ast* first_child;
  struct Ast* right_sibling;
} AstTree;

typedef struct AstTreeCtor {
  AstTree* last_sibling;
} AstTreeCtor;

void ast_tree_append_node(AstTree* tree, AstTreeCtor* ctor, Ast* node);
