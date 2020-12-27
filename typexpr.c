#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

internal TypeTable_EnumType* error_type;

internal TypeTable_Entry*
visit_function_prototype(Ast_FunctionDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_FUNCTION;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_extern_object_prototype(Ast_ExternObjectDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_EXTERN_OBJECT;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;

  Ast_FunctionDecl* method = decl->method;
  while (method)
  {
    TypeTable_Entry* method_ttb_entry = visit_function_prototype(method);
    method = (Ast_FunctionDecl*)method->next_decl;
  }
  return ttb_entry;
}

internal TypeTable_Entry*
visit_extern_function_prototype(Ast_FunctionDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_FUNCTION;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_parser_prototype(Ast_ParserDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_PARSER;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_parser_type(Ast_ParserDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_PARSER;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_control_prototype(Ast_ControlDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_CONTROL;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_control_type(Ast_ControlDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_CONTROL;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_package_prototype(Ast_PackageDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_PACKAGE;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_typedef(Ast_Typedef* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_TYPEDEF;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_header_prototype(Ast_HeaderDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_HEADER;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_header_type(Ast_HeaderDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_HEADER;
  return ttb_entry;
}

internal TypeTable_EnumField*
visit_error_code(Ast_ErrorCode* decl)
{
  TypeTable_EnumField* ttb_entry = arena_push_struct(&arena, TypeTable_EnumField);
  ttb_entry->kind = TYP_ENUM_FIELD;
  ttb_entry->name = decl->name;
  return ttb_entry;
}

internal TypeTable_EnumType*
visit_error_type(Ast_ErrorType* decl)
{
  TypeTable_EnumType* ttb_entry = error_type;
  if (decl->code_count != 0)
  {
    Ast_ErrorCode* error_code = decl->error_code;
    while (error_code)
    {
      TypeTable_EnumField* error_code_ttb_entry = visit_error_code(error_code);
      TypeTable_EnumField* last_field = ttb_entry->last_field;
      last_field->next_field = error_code_ttb_entry;
      ttb_entry->last_field = error_code_ttb_entry;
      ttb_entry->field_count += 1;

      error_code = (Ast_ErrorCode*)error_code->next_id;
    }
  }
  else
    error("at line %d: empty error definition is disallowed", decl->line_nr);
  return ttb_entry;
}

internal TypeTable_Entry*
visit_struct_prototype(Ast_StructDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_STRUCT;
  ttb_entry->is_prototype = true;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_struct_type(Ast_StructDecl* decl)
{
  TypeTable_Entry* ttb_entry = arena_push_struct(&arena, TypeTable_Entry);
  ttb_entry->kind = TYP_STRUCT;
  return ttb_entry;
}

internal TypeTable_Entry*
visit_p4declaration(Ast_Declaration* decl)
{
  TypeTable_Entry* ttb_entry = 0;
  if (decl->kind == AST_STRUCT_PROTOTYPE)
    ttb_entry = visit_struct_prototype((Ast_StructDecl*)decl);
  else if (decl->kind == AST_STRUCT_DECL)
    ttb_entry = visit_struct_type((Ast_StructDecl*)decl);
  else if (decl->kind == AST_HEADER_PROTOTYPE)
    ttb_entry = visit_header_prototype((Ast_HeaderDecl*)decl);
  else if (decl->kind == AST_HEADER_DECL)
    ttb_entry = visit_header_type((Ast_HeaderDecl*)decl);
  else if (decl->kind == AST_ERROR_TYPE)
    ttb_entry = (TypeTable_Entry*)visit_error_type((Ast_ErrorType*)decl);
  else if (decl->kind == AST_TYPEDEF)
    ttb_entry = visit_typedef((Ast_Typedef*)decl);
  else if (decl->kind == AST_PARSER_PROTOTYPE)
    ttb_entry = visit_parser_prototype((Ast_ParserDecl*)decl);
  else if (decl->kind == AST_PARSER_DECL)
    ttb_entry = visit_parser_type((Ast_ParserDecl*)decl);
  else if (decl->kind == AST_CONTROL_PROTOTYPE)
    ttb_entry = visit_control_prototype((Ast_ControlDecl*)decl);
  else if (decl->kind == AST_CONTROL_DECL)
    ttb_entry = visit_control_type((Ast_ControlDecl*)decl);
  else if (decl->kind == AST_PACKAGE_PROTOTYPE)
    ttb_entry = visit_package_prototype((Ast_PackageDecl*)decl);
  else if (decl->kind == AST_EXTERN_OBJECT_PROTOTYPE)
    ttb_entry = visit_extern_object_prototype((Ast_ExternObjectDecl*)decl);
  else if (decl->kind == AST_EXTERN_FUNCTION_PROTOTYPE)
    ttb_entry = visit_extern_function_prototype((Ast_FunctionDecl*)decl);
  else if (decl->kind == AST_PACKAGE_INSTANCE)
    ; // not a type declataration - pass.
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
    visit_p4declaration(decl);
    decl = decl->next_decl;
  }
}

void
build_typexpr()
{
  error_type = arena_push_struct(&arena, TypeTable_EnumType);
  error_type->kind = TYP_ENUM;
  error_type->name = "error";
  error_type->sentinel_field = arena_push_struct(&arena, TypeTable_EnumField);
  error_type->sentinel_field->kind = TYP_ENUM_FIELD;
  error_type->last_field = error_type->sentinel_field;
  visit_p4program(p4program);
  arena_print_usage(&arena, "Memory (build_typexpr): ");
}

