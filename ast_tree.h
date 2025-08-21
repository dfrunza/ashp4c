struct Ast;

typedef struct AstTree {
  struct Ast* first_child;
  struct Ast* right_sibling;
} AstTree;
