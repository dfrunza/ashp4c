#include "basic.h"
#include "arena.h"
#include "cst.h"
#include "ast.h"

#define DEBUG_ENABLED 1

external Arena arena;
internal int node_id = 1;

#define new_ast_node(type, cst) ({ \
  struct type* node = arena_push(&arena, sizeof(struct type)); \
  *node = (struct type){}; \
  node->kind = type; \
  node->id = node_id++; \
  node->line_nr = cst->line_nr; \
  node; })

internal void
link_ast_nodes(struct Ast* node_a, struct Ast* node_b)
{
  assert(node_a->next_node == 0);
  assert(node_b->prev_node == 0);
  node_a->next_node = node_b;
  node_b->prev_node = node_a;
}

internal struct Ast*
build_ExternDecl()
{
  return 0;
}

struct Ast*
build_AstP4Program(struct Cst_P4Program* cst_p4program)
{
  struct Ast_P4Program* ast_p4program = new_ast_node(Ast_P4Program, cst_p4program);
  struct Cst* cst_decl = cst_p4program->decl_list;
  struct Ast* ast_decl = 0;
  while (cst_decl) {
    if (cst_decl->kind == Cst_ConstDecl) {

    } else if (cst_decl->kind == Cst_ExternDecl) {
      ast_decl = build_ExternDecl(cst_decl);
    } else if (cst_decl->kind == Cst_ActionDecl) {

    } else if (cst_decl->kind == Cst_Parser) {

    } else if (cst_decl->kind == Cst_TypeDecl) {

    } else if (cst_decl->kind == Cst_Control) {

    } else if (cst_decl->kind == Cst_Instantiation) {

    } else if (cst_decl->kind == Cst_Error) {

    } else if (cst_decl->kind == Cst_MatchKind) {

    } else if (cst_decl->kind == Cst_FunctionDecl) {

    }
    else assert(0);
    
    ast_p4program->decl_count += 1;
    cst_decl = cst_decl->next_node;
  }
  return (struct Ast*)ast_p4program;
}
