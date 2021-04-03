#include "basic.h"
#include "arena.h"
#include "cst.h"
#include "ast.h"

#define DEBUG_ENABLED 1

internal struct Arena ast_arena;
internal int node_id = 1;
internal struct AstTree* ast_tree;

#define new_ast_node(type, cst) ({ \
  struct type* node = arena_push(&ast_arena, sizeof(struct type)); \
  *node = (struct type){}; \
  node->kind = type; \
  node->id = node_id++; \
  node->line_nr = cst->line_nr; \
  ast_tree->node_count += 1; \
  node; })

internal void
link_list_nodes(struct ListLink* node_a, struct ListLink* node_b)
{
  assert(node_a->next == 0);
  assert(node_b->prev == 0);
  node_a->next = node_b;
  node_b->prev = node_a;
}

internal struct Ast*
visit_ExternDecl()
{
  assert(!"todo");
  return 0;
}

internal void
copy_name(char* dest, struct Cst* cst_name)
{
  if (cst_name->kind == Cst_NonTypeName) {
    cstr_copy(dest, ((struct Cst_NonTypeName*)cst_name)->name);
  } else if (cst_name->kind == Cst_TypeName) {
    cstr_copy(dest, ((struct Cst_TypeName*)cst_name)->name);
  }
  else assert(0);
}

internal struct Ast*
visit_HeaderDecl(struct Cst_HeaderDecl* cst_header_decl)
{
  struct Ast_HeaderDecl* ast_header_decl = new_ast_node(Ast_HeaderDecl, cst_header_decl);
  ast_header_decl->name = arena_push(&ast_arena, cstr_len(cst_header_decl->name));
  copy_name(ast_header_decl->name, cst_header_decl->name);
  return (struct Ast*)ast_header_decl;
}

internal struct Ast*
visit_P4Program(struct Cst_P4Program* cst_p4program)
{
  struct Ast_P4Program* ast_p4program = new_ast_node(Ast_P4Program, cst_p4program);
  struct ListLink* prev_link = &ast_p4program->decl_list;
  struct Cst* cst_decl = cst_p4program->decl_list;
  struct Ast* ast_decl = 0;
  while (cst_decl) {
    if (cst_decl->kind == Cst_ConstDecl) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_ExternDecl) {
      ast_decl = visit_ExternDecl(cst_decl);
    } else if (cst_decl->kind == Cst_ActionDecl) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_Parser) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_TypeDecl) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_Control) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_Instantiation) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_Error) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_MatchKind) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_FunctionDecl) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_HeaderDecl) {
      ast_decl = visit_HeaderDecl((struct Cst_HeaderDecl*)cst_decl);
    }
    else assert(0);
    
    struct ListLink* next_link = arena_push(&ast_arena, sizeof(struct ListLink));
    next_link->object = ast_decl;
    link_list_nodes(prev_link, next_link);
    ast_p4program->decl_count += 1;
    cst_decl = cst_decl->next_node;
  }
  return (struct Ast*)ast_p4program;
}

struct AstTree
build_AstTree(struct CstTree* cst_tree)
{
  ast_tree = arena_push(&ast_arena, sizeof(struct AstTree));
  struct Cst_P4Program* cst_p4program = (struct Cst_P4Program*)cst_tree->p4program;
  ast_tree->p4program = visit_P4Program(cst_p4program);
  return *ast_tree;
}
