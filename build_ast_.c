#include "basic.h"
#include "arena.h"
#include "cst.h"
#include "ast.h"

#define DEBUG_ENABLED 1

internal struct Arena arena;
internal int node_id = 1;
internal struct AstTree* ast_tree;

internal struct Ast* visit_TypeRef(struct Cst* cst_type_ref);

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

internal void
init_ast_node(struct Ast* ast, struct Cst* cst)
{
  ast->id = node_id++;
  ast->line_nr = cst->line_nr;
  ast_tree->node_count += 1;
  if (ast->kind == Ast_P4Program) {
    struct Ast_P4Program* ast_p4program = (struct Ast_P4Program*)ast;
    list_init(&ast_p4program->decl_list);
  } else if (ast->kind == Ast_Error) {
    struct Ast_Error* ast_error = (struct Ast_Error*)ast;
    list_init(&ast_error->id_list);
  } else if (ast->kind == Ast_ExternDecl) {
    struct Ast_ExternDecl* ast_extern_decl = (struct Ast_ExternDecl*)ast;
    list_init(&ast_extern_decl->type_params);
    list_init(&ast_extern_decl->method_protos);
  } else if (ast->kind == Ast_FunctionProto) {
    struct Ast_FunctionProto* ast_function_proto = (struct Ast_FunctionProto*)ast;
    list_init(&ast_function_proto->type_params);
    list_init(&ast_function_proto->params);
  } else if (ast->kind == Ast_MatchKind) {
    struct Ast_MatchKind* ast_match_kind = (struct Ast_MatchKind*)ast;
    list_init(&ast_match_kind->id_list);
  } else if (ast->kind == Ast_StructDecl) {
    struct Ast_StructDecl* ast_struct_decl = (struct Ast_StructDecl*)ast;
    list_init(&ast_struct_decl->fields);
  } else if (ast->kind == Ast_HeaderDecl) {
    struct Ast_HeaderDecl* ast_header_decl = (struct Ast_HeaderDecl*)ast;
    list_init(&ast_header_decl->fields);
  } else if (ast->kind == Ast_ParserType) {
    struct Ast_ParserType* ast_parser_type = (struct Ast_ParserType*)ast;
    list_init(&ast_parser_type->type_params);
    list_init(&ast_parser_type->params);
  }
}

#define new_ast_node(type, cst) ({ \
  struct type* ast = arena_push(&arena, sizeof(struct type)); \
  *ast = (struct type){}; \
  ast->kind = type; \
  init_ast_node((struct Ast*)ast, (struct Cst*)cst); \
  ast; })

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
  // FIXME: dot prefix
  char* ast_strname = arena_push(&arena, cstr_len(cst_strname) + 1);
  cstr_copy(ast_strname, cst_strname);
  return ast_strname;
}

internal struct Ast*
visit_Parameter(struct Cst_Parameter* cst_parameter)
{
  assert(cst_parameter->kind == Cst_Parameter);
  struct Ast_Parameter* ast_parameter = new_ast_node(Ast_Parameter, cst_parameter);
  ast_parameter->name = visit_Name(cst_parameter->name);
  ast_parameter->direction = cst_parameter->direction;
  ast_parameter->type = visit_TypeRef(cst_parameter->type);
  // TODO: ast_parameter->init_expr
  return (struct Ast*)ast_parameter;
}

internal struct Ast*
visit_FunctionProto(struct Cst_FunctionProto* cst_function_proto)
{
  assert(cst_function_proto->kind == Cst_FunctionProto);
  struct Ast_FunctionProto* ast_function_proto = new_ast_node(Ast_FunctionProto, cst_function_proto);
  ast_function_proto->name = visit_Name(cst_function_proto->name);
  if (cst_function_proto->return_type) {
    /* Missing if constructor. */
    ast_function_proto->return_type = visit_TypeRef(cst_function_proto->return_type);
  }
  struct Cst* cst_type_param = cst_function_proto->type_params;
  while (cst_type_param) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Name(cst_type_param);
    list_append_link(&ast_function_proto->type_params, link);
    cst_type_param = cst_type_param->next_node;
  }
  struct Cst* cst_param = cst_function_proto->params;
  while (cst_param) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Parameter((struct Cst_Parameter*)cst_param);
    list_append_link(&ast_function_proto->params, link);
    cst_param = cst_param->next_node;
  }
  return (struct Ast*)ast_function_proto;
}

internal struct Ast*
visit_ExternDecl(struct Cst_ExternDecl* cst_extern_decl)
{
  assert(cst_extern_decl->kind == Cst_ExternDecl);
  struct Ast_ExternDecl* ast_extern_decl = new_ast_node(Ast_ExternDecl, cst_extern_decl);
  ast_extern_decl->name = visit_Name(cst_extern_decl->name);
  struct Cst* cst_type_param = cst_extern_decl->type_params;
  while (cst_type_param) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Name(cst_type_param);
    list_append_link(&ast_extern_decl->type_params, link);
    cst_type_param = cst_type_param->next_node;
  }
  struct Cst* cst_method_proto = cst_extern_decl->method_protos;
  while (cst_method_proto) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_FunctionProto((struct Cst_FunctionProto*)cst_method_proto);
    list_append_link(&ast_extern_decl->method_protos, link);
    cst_method_proto = cst_method_proto->next_node;
  }
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
visit_StructField(struct Cst_StructField* cst_field)
{
  assert(cst_field->kind == Cst_StructField);
  struct Ast_StructField* ast_field = new_ast_node(Ast_StructField, cst_field);
  ast_field->name = visit_Name(cst_field->name);
  ast_field->type = visit_TypeRef(cst_field->type);
  return (struct Ast*)ast_field;
}

internal struct Ast*
visit_HeaderDecl(struct Cst_HeaderDecl* cst_header_decl)
{
  assert(cst_header_decl->kind == Cst_HeaderDecl);
  struct Ast_HeaderDecl* ast_header_decl = new_ast_node(Ast_HeaderDecl, cst_header_decl);
  ast_header_decl->name = visit_Name(cst_header_decl->name);
  struct Cst* cst_field = cst_header_decl->fields;
  while (cst_field) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_StructField((struct Cst_StructField*)cst_field);
    list_append_link(&ast_header_decl->fields, link);
    cst_field = cst_field->next_node;
  }
  return (struct Ast*)ast_header_decl;
}

internal struct Ast*
visit_HeaderUnionDecl(struct Cst_HeaderUnionDecl* cst_header_union_decl)
{
  assert(cst_header_union_decl->kind == Cst_HeaderUnionDecl);
  struct Ast_HeaderUnionDecl* ast_header_union_decl = new_ast_node(Ast_HeaderUnionDecl, cst_header_union_decl);
  ast_header_union_decl->name = visit_Name(cst_header_union_decl->name);
  return (struct Ast*)ast_header_union_decl;
}

internal struct Ast*
visit_StructDecl(struct Cst_StructDecl* cst_struct_decl)
{
  assert(cst_struct_decl->kind == Cst_StructDecl);
  struct Ast_StructDecl* ast_struct_decl = new_ast_node(Ast_StructDecl, cst_struct_decl);
  ast_struct_decl->name = visit_Name(cst_struct_decl->name);
  struct Cst* cst_field = cst_struct_decl->fields;
  while (cst_field) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_StructField((struct Cst_StructField*)cst_field);
    list_append_link(&ast_struct_decl->fields, link);
    cst_field = cst_field->next_node;
  }
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
visit_Error(struct Cst_Error* cst_error)
{
  assert(cst_error->kind == Cst_Error);
  struct Ast_Error* ast_error = new_ast_node(Ast_Error, cst_error);
  struct Cst* cst_id = cst_error->id_list;
  while (cst_id) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Name(cst_id);;
    list_append_link(&ast_error->id_list, link);
    cst_id = cst_id->next_node;
  }
  return (struct Ast*)ast_error;
}

internal struct Ast*
visit_MatchKind(struct Cst_MatchKind* cst_match_kind)
{
  assert(cst_match_kind->kind == Cst_MatchKind);
  struct Ast_MatchKind* ast_match_kind = new_ast_node(Ast_MatchKind, cst_match_kind);
  struct Cst* cst_id = cst_match_kind->id_list;
  while (cst_id) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Name(cst_id);
    list_append_link(&ast_match_kind->id_list, link);
    cst_id = cst_id->next_node;
  }
  return (struct Ast*)ast_match_kind;
}

internal struct Ast*
visit_EnumDecl(struct Cst_EnumDecl* cst_enum_decl)
{
  assert(cst_enum_decl->kind == Cst_EnumDecl);
  struct Ast_EnumDecl* ast_enum_decl = new_ast_node(Ast_EnumDecl, cst_enum_decl);
  return (struct Ast*)ast_enum_decl;
}

internal struct Ast*
visit_ParserType(struct Cst_ParserType* cst_parser_type)
{
  assert(cst_parser_type->kind == Cst_ParserType);
  struct Ast_ParserType* ast_parser_type = new_ast_node(Ast_ParserType, cst_parser_type);
  ast_parser_type->name = visit_Name(cst_parser_type->name);
  struct Cst* cst_type_param = cst_parser_type->type_params;
  while (cst_type_param)  {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Name(cst_type_param);
    list_append_link(&ast_parser_type->type_params, link);
    cst_type_param = cst_type_param->next_node;
  }
  struct Cst* cst_param = cst_parser_type->params;
  while (cst_param) {
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = visit_Parameter((struct Cst_Parameter*)cst_param);
    list_append_link(&ast_parser_type->params, link);
    cst_param = cst_param->next_node;
  }
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
    } else if (cst_base_type->base_type == CstBaseType_String) {
      ast_base_type->name = "string";
    }
    else assert(0);
    return (struct Ast*)ast_base_type;
  } else if (cst_type_ref->kind == Cst_TypeName || cst_type_ref->kind == Cst_NonTypeName) {
    struct Cst_TypeName* cst_type_name = (struct Cst_TypeName*)cst_type_ref;
    struct Ast_TypeName* ast_type_name = new_ast_node(Ast_TypeName, cst_type_name);
    ast_type_name->name = visit_Name((struct Cst*)cst_type_name);
    return (struct Ast*)ast_type_name;
  } else if (cst_type_ref->kind == Cst_HeaderStack) {
    struct Cst_HeaderStack* cst_header_stack = (struct Cst_HeaderStack*)cst_type_ref;
    struct Ast_HeaderStack* ast_header_stack = new_ast_node(Ast_HeaderStack, cst_type_ref);
    ast_header_stack->name = visit_Name(cst_header_stack->name);
    return (struct Ast*)ast_header_stack;
  } else if (cst_type_ref->kind == Cst_SpecdType) {
    struct Cst_SpecdType* cst_specd_type = (struct Cst_SpecdType*)cst_type_ref;
    struct Ast_SpecdType* ast_specd_type = new_ast_node(Ast_SpecdType, cst_type_ref);
    ast_specd_type->name = visit_Name(cst_specd_type->name);
    return (struct Ast*)ast_specd_type;
  } else if (cst_type_ref->kind == Cst_StructDecl) {
    return visit_StructDecl((struct Cst_StructDecl*)cst_type_ref);
  }
  else assert(0);
  return 0;
}

internal struct Ast*
visit_P4Program(struct Cst_P4Program* cst_p4program)
{
  assert(cst_p4program->kind == Cst_P4Program);
  struct Ast_P4Program* ast_p4program = new_ast_node(Ast_P4Program, cst_p4program);
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
      ast_decl = visit_MatchKind((struct Cst_MatchKind*)cst_decl);
    } else if (cst_decl->kind == Cst_EnumDecl) {
      ast_decl = visit_EnumDecl((struct Cst_EnumDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_FunctionDecl) {
      ast_decl = visit_FunctionDecl((struct Cst_FunctionDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_HeaderDecl) {
      ast_decl = visit_HeaderDecl((struct Cst_HeaderDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_HeaderUnionDecl) {
      ast_decl = visit_HeaderUnionDecl((struct Cst_HeaderUnionDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_StructDecl) {
      ast_decl = visit_StructDecl((struct Cst_StructDecl*)cst_decl);
    } else if (cst_decl->kind == Cst_Package) {
      ast_decl = visit_Package((struct Cst_Package*)cst_decl);
    } else if (cst_decl->kind == Cst_FunctionProto) {
      ast_decl = visit_FunctionProto((struct Cst_FunctionProto*)cst_decl);
    }
    else assert(0);
    
    struct ListLink* link = arena_push(&arena, sizeof(struct ListLink));
    link->object = ast_decl;
    list_append_link(&ast_p4program->decl_list, link);
    cst_decl = cst_decl->next_node;
  }
  return (struct Ast*)ast_p4program;
}

struct AstTree
build_AstTree(struct CstTree* cst_tree)
{
  ast_tree = arena_push(&arena, sizeof(struct AstTree));
  struct Cst_P4Program* cst_p4program = (struct Cst_P4Program*)cst_tree->p4program;
  ast_tree->p4program = visit_P4Program(cst_p4program);
  ast_tree->arena = &arena;
  return *ast_tree;
}
