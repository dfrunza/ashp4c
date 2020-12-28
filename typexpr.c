#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

internal Typexpr_EnumType* error_type;

internal void
link_ast_to_typexpr(Ast* ast, Typexpr* typexpr)
{
  ast->typexpr = typexpr;
  typexpr->ast = ast;
}

internal Typexpr_Function*
visit_function_prototype(Ast_FunctionDecl* decl)
{
  Typexpr_Function* ttb_entry = arena_push_struct(&arena, Typexpr_Function);
  zero_struct(ttb_entry, Typexpr_Function);
  ttb_entry->kind = TYP_FUNCTION;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr_ExternObject*
visit_extern_object_prototype(Ast_ExternObjectDecl* decl)
{
  Typexpr_ExternObject* ttb_entry = arena_push_struct(&arena, Typexpr_ExternObject);
  zero_struct(ttb_entry, Typexpr_ExternObject);
  ttb_entry->kind = TYP_EXTERN_OBJECT;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;

  ttb_entry->sentinel_function = arena_push_struct(&arena, Typexpr_Function);
  zero_struct(ttb_entry->sentinel_function, Typexpr_Function);
  ttb_entry->sentinel_function->kind = TYP_FUNCTION;
  ttb_entry->last_function = ttb_entry->sentinel_function;

  Ast_FunctionDecl* method_ast = decl->first_method;
  while (method_ast)
  {
    Typexpr_Function* method_typexpr = visit_function_prototype(method_ast);
    ttb_entry->last_function->next_function = method_typexpr;
    ttb_entry->last_function = method_typexpr;
    ttb_entry->method_count += 1;

    method_ast = (Ast_FunctionDecl*)method_ast->next_decl;
  }

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_extern_function_prototype(Ast_FunctionDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_FUNCTION;
  ttb_entry->name = decl->name;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_parser_prototype(Ast_ParserDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_PARSER;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_parser_type(Ast_ParserDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_PARSER;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_control_prototype(Ast_ControlDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_CONTROL;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_control_type(Ast_ControlDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_CONTROL;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_package_prototype(Ast_PackageDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_PACKAGE;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_typedef(Ast_Typedef* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_TYPEDEF;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_header_prototype(Ast_HeaderDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_HEADER;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_header_type(Ast_HeaderDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_HEADER;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr_EnumField*
visit_error_code(Ast_ErrorCode* decl)
{
  Typexpr_EnumField* ttb_entry = arena_push_struct(&arena, Typexpr_EnumField);
  zero_struct(ttb_entry, Typexpr_EnumField);
  ttb_entry->kind = TYP_ENUM_FIELD;
  ttb_entry->name = decl->name;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr_EnumType*
visit_error_type(Ast_ErrorType* decl)
{
  Typexpr_EnumType* ttb_entry = error_type;

  if (!decl->error_code)
    error("at line %d: error declaration must have at least one error code", decl->line_nr);

  Ast_ErrorCode* error_code_ast = decl->error_code;
  while (error_code_ast)
  {
    Typexpr_EnumField* error_code_typexpr = visit_error_code(error_code_ast);
    ttb_entry->last_field->next_field = error_code_typexpr;
    ttb_entry->last_field = error_code_typexpr;
    ttb_entry->field_count += 1;

    error_code_ast = (Ast_ErrorCode*)error_code_ast->next_ident;
  }

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_struct_prototype(Ast_StructDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_STRUCT;
  ttb_entry->is_prototype = true;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal Typexpr*
visit_struct_type(Ast_StructDecl* decl)
{
  Typexpr* ttb_entry = arena_push_struct(&arena, Typexpr);
  zero_struct(ttb_entry, Typexpr);
  ttb_entry->kind = TYP_STRUCT;

  link_ast_to_typexpr((Ast*)decl, (Typexpr*)ttb_entry);
  return ttb_entry;
}

internal void
visit_p4declaration(Ast_Declaration* decl)
{
  if (decl->kind == AST_STRUCT_PROTOTYPE)
    visit_struct_prototype((Ast_StructDecl*)decl);
  else if (decl->kind == AST_STRUCT_DECL)
    visit_struct_type((Ast_StructDecl*)decl);
  else if (decl->kind == AST_HEADER_PROTOTYPE)
    visit_header_prototype((Ast_HeaderDecl*)decl);
  else if (decl->kind == AST_HEADER_DECL)
    visit_header_type((Ast_HeaderDecl*)decl);
  else if (decl->kind == AST_ERROR_TYPE)
    visit_error_type((Ast_ErrorType*)decl);
  else if (decl->kind == AST_TYPEDEF)
    visit_typedef((Ast_Typedef*)decl);
  else if (decl->kind == AST_PARSER_PROTOTYPE)
    visit_parser_prototype((Ast_ParserDecl*)decl);
  else if (decl->kind == AST_PARSER_DECL)
    visit_parser_type((Ast_ParserDecl*)decl);
  else if (decl->kind == AST_CONTROL_PROTOTYPE)
    visit_control_prototype((Ast_ControlDecl*)decl);
  else if (decl->kind == AST_CONTROL_DECL)
    visit_control_type((Ast_ControlDecl*)decl);
  else if (decl->kind == AST_PACKAGE_PROTOTYPE)
    visit_package_prototype((Ast_PackageDecl*)decl);
  else if (decl->kind == AST_EXTERN_OBJECT_PROTOTYPE)
    visit_extern_object_prototype((Ast_ExternObjectDecl*)decl);
  else if (decl->kind == AST_EXTERN_FUNCTION_PROTOTYPE)
    visit_extern_function_prototype((Ast_FunctionDecl*)decl);
  else if (decl->kind == AST_PACKAGE_INSTANCE)
    ; // not a type declataration - pass.
  else
    assert (false);
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
  error_type = arena_push_struct(&arena, Typexpr_EnumType);
  zero_struct(error_type, Typexpr_EnumType);
  error_type->kind = TYP_ENUM;
  error_type->name = "error";

  error_type->sentinel_field = arena_push_struct(&arena, Typexpr_EnumField);
  zero_struct(error_type->sentinel_field, Typexpr_EnumField);
  error_type->sentinel_field->kind = TYP_ENUM_FIELD;
  error_type->last_field = error_type->sentinel_field;

  visit_p4program(p4program);

  arena_print_usage(&arena, "Memory (build_typexpr): ");
}

