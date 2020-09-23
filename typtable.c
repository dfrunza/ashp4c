#include "dp4c.h"

external Arena arena;
external TypeTable_Entry* typetable;
external TypeTable_Entry* typetable_free;
external int max_typetable_len;
external Ast_P4Program* p4program;
internal TypeTable_Entry* error_type;

internal TypeTable_Entry*
new_table_entry()
{
  TypeTable_Entry* ttb_entry = typetable_free;
  zero_struct(ttb_entry, TypeTable_Entry);
  typetable_free++;
  assert (typetable_free <= (typetable + max_typetable_len-1));
  return ttb_entry;
}

internal TypeTable_Entry*
visit_function_prototype(Ast_FunctionPrototype* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_FUNCTION;
  ttb_entry->name = decl->name;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_extern_object_decl(Ast_ExternObjectDecl* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_STRUCT;
  ttb_entry->name = decl->name;
  ttb_entry->is_decl = true;

  Ast_FunctionPrototype* method = decl->method;
  while (method)
  {
    TypeTable_Entry* method_ttb_entry = visit_function_prototype(method);
    method = (Ast_FunctionPrototype*)method->next_decl;
  }
  return ttb_entry;
}

internal TypeTable_Entry*
visit_parser_type_decl(Ast_ParserType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_PARSER;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_parser_type(Ast_ParserType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_PARSER;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_control_type_decl(Ast_ControlType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_CONTROL;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_control_type(Ast_ControlType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_CONTROL;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_package_type_decl(Ast_PackageTypeDecl* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_PACKAGE;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_typedef(Ast_Typedef* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_TYPEDEF;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_header_type_decl(Ast_HeaderType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_HEADER;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_header_type(Ast_HeaderType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_HEADER;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_error_code(Ast_ErrorCode* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_ENUM_FIELD;
  ttb_entry->name = decl->name;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_error_type(Ast_ErrorType* decl)
{
  TypeTable_Entry* ttb_entry = error_type;
  if (decl->code_count != 0)
  {
    Ast_ErrorCode* error_code = decl->error_code;
    while (error_code)
    {
      TypeTable_Entry* error_code_ttb_entry = visit_error_code(error_code);
      TypeTable_Entry* last_field = ttb_entry->enum_type.last_field;
      last_field->enum_field.next_field = ttb_entry;
      ttb_entry->enum_type.last_field = ttb_entry;
      ttb_entry->enum_type.field_count += 1;

      error_code = (Ast_ErrorCode*)error_code->next_id;
    }
  }
  else
    error("at line %d: empty error definition is disallowed", decl->line_nr);
  return ttb_entry;
}

internal TypeTable_Entry*
visit_struct_type_decl(Ast_StructType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_STRUCT;
  ttb_entry->is_decl = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_struct_type(Ast_StructType* decl)
{
  TypeTable_Entry* ttb_entry = new_table_entry();
  ttb_entry->kind = TYP_STRUCT;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_declaration(Ast_Declaration* decl)
{
  TypeTable_Entry* ttb_entry = 0;
  if (decl->kind == AST_STRUCT_TYPE_DECL)
    ttb_entry = visit_struct_type_decl((Ast_StructType*)decl);
  else if (decl->kind == AST_STRUCT_TYPE)
    ttb_entry = visit_struct_type((Ast_StructType*)decl);
  else if (decl->kind == AST_HEADER_TYPE_DECL)
    ttb_entry = visit_header_type_decl((Ast_HeaderType*)decl);
  else if (decl->kind == AST_HEADER_TYPE)
    ttb_entry = visit_header_type((Ast_HeaderType*)decl);
  else if (decl->kind == AST_ERROR_TYPE)
    ttb_entry = visit_error_type((Ast_ErrorType*)decl);
  else if (decl->kind == AST_TYPEDEF)
    ttb_entry = visit_typedef((Ast_Typedef*)decl);
  else if (decl->kind == AST_PARSER_TYPE_DECL)
    ttb_entry = visit_parser_type_decl((Ast_ParserType*)decl);
  else if (decl->kind == AST_PARSER_TYPE)
    ttb_entry = visit_parser_type((Ast_ParserType*)decl);
  else if (decl->kind == AST_CONTROL_TYPE_DECL)
    ttb_entry = visit_control_type_decl((Ast_ControlType*)decl);
  else if (decl->kind == AST_CONTROL_TYPE)
    ttb_entry = visit_control_type((Ast_ControlType*)decl);
  else if (decl->kind == AST_PACKAGE_TYPE_DECL)
    ttb_entry = visit_package_type_decl((Ast_PackageTypeDecl*)decl);
  else if (decl->kind == AST_EXTERN_OBJECT_DECL)
    ttb_entry = visit_extern_object_decl((Ast_ExternObjectDecl*)decl);
  else if (decl->kind == AST_FUNCTION_PROTOTYPE)
    ttb_entry = visit_function_prototype((Ast_FunctionPrototype*)decl);
  else if (decl->kind == AST_INSTANTIATION)
    ;
  else
    assert (false);
  return ttb_entry;
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
  error_type = new_table_entry();
  error_type->kind = TYP_ENUM;
  error_type->name = "error";
  error_type->enum_type.enum_field = new_table_entry();
  error_type->enum_type.enum_field->kind = TYP_ENUM_FIELD;
  error_type->enum_type.last_field = error_type->enum_type.enum_field;
  visit_p4program(p4program);
  arena_print_usage(&arena, "Memory (build_typetable): ");
}

