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
list_init(struct List* list)
{
  assert(list->head_lp == 0);
  assert(list->tail_lp == 0);
  list->head_lp = &list->sentinel;
  list->tail_lp = list->head_lp;
}

internal void
list_append_link(struct List* list, struct ListLink* link)
{
  assert(list->tail_lp->next_lp == 0);
  assert(link->prev_lp == 0);
  list->tail_lp->next_lp = link;
  link->prev_lp = list->tail_lp;
  list->tail_lp = link;
  list->link_count += 1;
}

internal char*
visit_Name(struct Cst* cst_name)
{
  char* cst_strname = 0;
  if (cst_name->kind == Cst_NonTypeName) {
    cst_strname = ((struct Cst_NonTypeName*)cst_name)->name;
  } else if (cst_name->kind == Cst_TypeName) {
    cst_strname = ((struct Cst_TypeName*)cst_name)->name;
  }
  else assert(0);
  char* ast_strname = arena_push(&ast_arena, cstr_len(cst_strname) + 1);
  cstr_copy(ast_strname, cst_strname);
  return ast_strname;
}

internal struct Ast*
visit_TypeRef(struct Cst* cst_type_ref)
{
  if (cst_type_ref->kind == Cst_BaseType) {
    struct Cst_BaseType* cst_base_type = (struct Cst_BaseType*)cst_type_ref;
    struct Ast_BaseType* ast_base_type = new_ast_node(Ast_BaseType, cst_base_type);
    if (cst_base_type->base_type == CstBaseType_Bool) {
      ast_base_type->name = "bool";
    } else if (cst_base_type->base_type == CstBaseType_Error) {
      ast_base_type->name = "error";
    } else if (cst_base_type->base_type == CstBaseType_Int) {
      ast_base_type->name = "int";
    } else if (cst_base_type->base_type == CstBaseType_Bit) {
      ast_base_type->name = "bit";
    } else if (cst_base_type->base_type == CstBaseType_Varbit) {
      ast_base_type->name = "varbit";
    }
    else assert(0);
    return (struct Ast*)ast_base_type;
  } else if (cst_type_ref->kind == Cst_TypeName) {
    struct Cst_TypeName* cst_type_name = (struct Cst_TypeName*)cst_type_ref;
    struct Ast_TypeName* ast_type_name = new_ast_node(Ast_TypeName, cst_type_name);
    ast_type_name->name = arena_push(&ast_arena, cstr_len(cst_type_name->name) + 1);
    cstr_copy(ast_type_name->name, cst_type_name->name);
    return (struct Ast*)ast_type_name;
  } else if (cst_type_ref->kind == Cst_PrefixedTypeName) {
    struct Cst_PrefixedTypeName* cst_type_name = (struct Cst_PrefixedTypeName*)cst_type_ref;
    struct Ast_TypeName* ast_type_name = new_ast_node(Ast_TypeName, cst_type_name);
    ast_type_name->name = arena_push(&ast_arena, cstr_len(cst_type_name->first_name) + 1);
    cstr_copy(ast_type_name->name, cst_type_name->first_name);
    ast_type_name->dot_name = arena_push(&ast_arena, cstr_len(cst_type_name->second_name) + 1);
    cstr_copy(ast_type_name->dot_name, cst_type_name->second_name);
    return (struct Ast*)ast_type_name;
  } else if (cst_type_ref->kind == Cst_NonTypeName) {
    struct Cst_NonTypeName* cst_type_name = (struct Cst_NonTypeName*)cst_type_ref;
    struct Ast_TypeName* ast_type_name = new_ast_node(Ast_TypeName, cst_type_name);
    ast_type_name->name = arena_push(&ast_arena, cstr_len(cst_type_name->name) + 1);
    cstr_copy(ast_type_name->name, cst_type_name->name);
    return (struct Ast*)ast_type_name;
  }
  else assert(0);
  return 0;
}

internal struct Ast*
visit_ExternDecl(struct Cst_ExternDecl* cst_extern_decl)
{
  assert(cst_extern_decl->kind == Cst_ExternDecl);
  struct Ast_ExternDecl* ast_extern_decl = new_ast_node(Ast_ExternDecl, cst_extern_decl);
  ast_extern_decl->name = visit_Name(cst_extern_decl->name);
  return (struct Ast*)ast_extern_decl;
}

internal struct Ast*
visit_ControlType(struct Cst_ControlType* cst_control_type)
{
  assert(cst_control_type->kind == Cst_ControlType);
  struct Ast_ControlType* ast_control_type = new_ast_node(Ast_ControlType, cst_control_type);
  ast_control_type->name = visit_Name(cst_control_type->name);
  return (struct Ast*)ast_control_type;
}

internal struct Ast*
visit_Control(struct Cst_Control* cst_control)
{
  assert(cst_control->kind == Cst_Control);
  struct Ast_Control* ast_control = new_ast_node(Ast_Control, cst_control);
  ast_control->control_type = visit_ControlType((struct Cst_ControlType*)cst_control->type_decl);
  return (struct Ast*)ast_control;
}

internal struct Ast*
visit_HeaderDecl(struct Cst_HeaderDecl* cst_header_decl)
{
  assert(cst_header_decl->kind == Cst_HeaderDecl);
  struct Ast_HeaderDecl* ast_header_decl = new_ast_node(Ast_HeaderDecl, cst_header_decl);
  ast_header_decl->name = visit_Name(cst_header_decl->name);
  return (struct Ast*)ast_header_decl;
}

internal struct Ast*
visit_StructDecl(struct Cst_StructDecl* cst_struct_decl)
{
  assert(cst_struct_decl->kind == Cst_StructDecl);
  struct Ast_StructDecl* ast_struct_decl = new_ast_node(Ast_StructDecl, cst_struct_decl);
  ast_struct_decl->name = visit_Name(cst_struct_decl->name);
  return (struct Ast*)ast_struct_decl;
}

internal struct Ast*
visit_Instantiation(struct Cst_Instantiation* cst_instantiation)
{
  assert(cst_instantiation->kind == Cst_Instantiation);
  struct Ast_Instantiation* ast_instantiation = new_ast_node(Ast_Instantiation, cst_instantiation);
  ast_instantiation->name = visit_Name(cst_instantiation->name);
  ast_instantiation->type_ref = visit_TypeRef(cst_instantiation->type_ref);
  return (struct Ast*)ast_instantiation;
}

internal struct Ast*
visit_Package(struct Cst_Package* cst_package)
{
  assert(cst_package->kind == Cst_Package);
  struct Ast_Package* ast_package = new_ast_node(Ast_Package, cst_package);
  ast_package->name = visit_Name(cst_package->name);
  return (struct Ast*)ast_package;
}

internal struct Ast*
visit_FunctionProto(struct Cst_FunctionProto* cst_function_proto)
{
  assert(cst_function_proto->kind == Cst_FunctionProto);
  struct Ast_FunctionProto* ast_function_proto = new_ast_node(Ast_FunctionProto, cst_function_proto);
  ast_function_proto->name = visit_Name(cst_function_proto->name);
  ast_function_proto->return_type = visit_TypeRef(cst_function_proto->return_type);
  return (struct Ast*)ast_function_proto;
}

internal struct Ast*
visit_Error(struct Cst_Error* cst_error)
{
  assert(cst_error->kind == Cst_Error);
  struct Ast_Error* ast_error = new_ast_node(Ast_Error, cst_error);
  return (struct Ast*)ast_error;
}

internal struct Ast*
visit_ParserType(struct Cst_ParserType* cst_parser_type)
{
  assert(cst_parser_type->kind == Cst_ParserType);
  struct Ast_ParserType* ast_parser_type = new_ast_node(Ast_ParserType, cst_parser_type);
  ast_parser_type->name = visit_Name(cst_parser_type->name);
  return (struct Ast*)ast_parser_type;
}

internal struct Ast*
visit_Parser(struct Cst_Parser* cst_parser)
{
  assert(cst_parser->kind == Cst_Parser);
  struct Ast_Parser* ast_parser = new_ast_node(Ast_Parser, cst_parser);
  ast_parser->parser_type = visit_ParserType((struct Cst_ParserType*)cst_parser->type_decl);
  return (struct Ast*)ast_parser;
}

internal struct Ast*
visit_TypeDecl(struct Cst_TypeDecl* cst_type_decl)
{
  assert(cst_type_decl->kind == Cst_TypeDecl);
  struct Ast_TypeDecl* ast_type_decl = new_ast_node(Ast_TypeDecl, cst_type_decl);
  ast_type_decl->is_typedef = cst_type_decl->is_typedef;
  ast_type_decl->name = visit_Name(cst_type_decl->name);
  ast_type_decl->type_ref = visit_TypeRef(cst_type_decl->type_ref);
  return (struct Ast*)ast_type_decl;
}

internal struct Ast*
visit_ConstDecl(struct Cst_ConstDecl* cst_const_decl)
{
  assert(cst_const_decl->kind == Cst_ConstDecl);
  struct Ast_ConstDecl* ast_const_decl = new_ast_node(Ast_ConstDecl, cst_const_decl);
  ast_const_decl->name = visit_Name(cst_const_decl->name);
  ast_const_decl->type_ref = visit_TypeRef(cst_const_decl->type_ref);
  return (struct Ast*)ast_const_decl;
}

internal struct Ast*
visit_ActionDecl(struct Cst_ActionDecl* cst_action_decl)
{
  assert(cst_action_decl->kind == Cst_ActionDecl);
  struct Ast_ActionDecl* ast_action_decl = new_ast_node(Ast_ActionDecl, cst_action_decl);
  ast_action_decl->name = visit_Name(cst_action_decl->name);
  return (struct Ast*)ast_action_decl;
}

internal struct Ast*
visit_FunctionDecl(struct Cst_FunctionDecl* cst_function_decl)
{
  assert(cst_function_decl->kind == Cst_FunctionDecl);
  struct Ast_FunctionDecl* ast_function_decl = new_ast_node(Ast_FunctionDecl, cst_function_decl);
  ast_function_decl->proto = visit_FunctionProto((struct Cst_FunctionProto*)cst_function_decl->proto);
  return (struct Ast*)ast_function_decl;
}

internal struct Ast*
visit_P4Program(struct Cst_P4Program* cst_p4program)
{
  assert(cst_p4program->kind == Cst_P4Program);
  struct Ast_P4Program* ast_p4program = new_ast_node(Ast_P4Program, cst_p4program);
  list_init(&ast_p4program->decl_list);
  struct Cst* cst_decl = cst_p4program->decl_list;
  while (cst_decl) {
    struct Ast* ast_decl = 0;
    if (cst_decl->kind == Cst_ConstDecl) {
      ast_decl = visit_ConstDecl((struct Cst_ConstDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_ExternDecl) {
      ast_decl = visit_ExternDecl((struct Cst_ExternDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_ActionDecl) {
      ast_decl = visit_ActionDecl((struct Cst_ActionDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_Parser) {
      ast_decl = visit_Parser((struct Cst_Parser*)cst_decl);
    } else if (cst_decl->kind == Cst_TypeDecl) {
      ast_decl = visit_TypeDecl((struct Cst_TypeDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_Control) {
      visit_Control((struct Cst_Control*)cst_decl);
    } else if (cst_decl->kind == Cst_Instantiation) {
      ast_decl = visit_Instantiation((struct Cst_Instantiation*)cst_decl);
    } else if (cst_decl->kind == Cst_Error) {
      ast_decl = visit_Error((struct Cst_Error*)cst_decl);
    } else if (cst_decl->kind == Cst_MatchKind) {
      assert(!"todo");
    } else if (cst_decl->kind == Cst_FunctionDecl) {
      ast_decl = visit_FunctionDecl((struct Cst_FunctionDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_HeaderDecl) {
      ast_decl = visit_HeaderDecl((struct Cst_HeaderDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_StructDecl) {
      ast_decl = visit_StructDecl((struct Cst_StructDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_Package) {
      ast_decl = visit_Package((struct Cst_Package*)cst_decl);
    } else if (cst_decl->kind == Cst_FunctionProto) {
      ast_decl = visit_FunctionProto((struct Cst_FunctionProto*)cst_decl);
    }
    else assert(0);
    
    struct ListLink* llink = arena_push(&ast_arena, sizeof(struct ListLink));
    llink->object = ast_decl;
    list_append_link(&ast_p4program->decl_list, llink);
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
