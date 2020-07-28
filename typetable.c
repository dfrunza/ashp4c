#include "dp4c.h"

external Arena arena;
external TypeTable_Entry* typetable;
external TypeTable_Entry* typetable_free;
external int max_typetable_len;
external Ast_P4Program* p4program;

internal TypeTable_Entry*
new_table_entry()
{
  TypeTable_Entry* tb_entry = typetable_free;
  zero_struct(tb_entry, TypeTable_Entry);
  typetable_free++;
  assert (typetable_free <= (typetable + max_typetable_len-1));
  return tb_entry;
}

internal void
visit_extern_object_decl(Ast_ExternObjectDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_STRUCT;
}

internal void
visit_function_prototype(Ast_FunctionPrototype* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_FUNCTION;
}

internal void
visit_parser_decl(Ast_ParserDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_PARSER;
}

internal void
visit_control_decl(Ast_ControlDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_CONTROL;
}

internal void
visit_package_type_decl(Ast_PackageTypeDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_PACKAGE;
}

internal void
visit_typedef_decl(Ast_TypedefDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_TYPEDEF;
}

internal void
visit_header_type_decl(Ast_HeaderTypeDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_HEADER;
}

internal void
visit_struct_type_decl(Ast_StructTypeDecl* decl)
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_STRUCT;
}

internal void
visit_declaration(Ast_Declaration* decl)
{
  if (decl->kind == AST_STRUCT_TYPE_DECL)
    visit_struct_type_decl((Ast_StructTypeDecl*)decl);
  else if (decl->kind == AST_HEADER_TYPE_DECL)
    visit_header_type_decl((Ast_HeaderTypeDecl*)decl);
  else if (decl->kind == AST_ERROR_TYPE_DECL)
    ;
  else if (decl->kind == AST_TYPEDEF_DECL)
    visit_typedef_decl((Ast_TypedefDecl*)decl);
  else if (decl->kind == AST_PARSER_DECL)
    visit_parser_decl((Ast_ParserDecl*)decl);
  else if (decl->kind == AST_CONTROL_DECL)
    visit_control_decl((Ast_ControlDecl*)decl);
  else if (decl->kind == AST_ACTION_DECL)
    assert (false);
  else if (decl->kind == AST_PACKAGE_TYPE_DECL)
    visit_package_type_decl((Ast_PackageTypeDecl*)decl);
  else if (decl->kind == AST_EXTERN_OBJECT_DECL)
    visit_extern_object_decl((Ast_ExternObjectDecl*)decl);
  else if (decl->kind == AST_FUNCTION_PROTOTYPE_DECL)
    visit_function_prototype((Ast_FunctionPrototype*)decl);
  else if (decl->kind == AST_INSTANTIATION)
    ;
}

internal void
visit_p4program(Ast_P4Program* p4program)
{
  assert (p4program->kind == AST_P4PROGRAM);
  Ast_Declaration* decl = p4program->declaration;
  while (decl)
  {
    visit_declaration(decl);
    decl = decl->next_decl;
  }
}

void
build_typetable()
{
  TypeTable_Entry* tb_entry = new_table_entry();
  tb_entry->kind = TYP_ENUM;
  tb_entry->name = "error";
  visit_p4program(p4program);
  arena_print_usage(&arena, "Memory (build_typetable): ");
}

