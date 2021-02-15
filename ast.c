#define DEBUG_ENABLED 1

#include "basic.h"
#include "arena.h"
#include "syntax.h"
#include "ast.h"

external Arena arena;
internal int node_id = 1;

#define new_ast_node(type) ({ \
  struct type* node = arena_push(&arena, sizeof(struct type)); \
  *node = (struct type){}; \
  node->kind = type; \
  node->id = node_id++; \
  node; })

struct Ast*
build_ast(struct Cst_P4Program* cst)
{
  struct Ast_P4Program* p4program = new_ast_node(Ast_P4Program);
  return (struct Ast*)p4program;
}
