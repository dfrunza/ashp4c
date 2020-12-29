#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

external Ast_Ident* error_type_ast;
internal Typexpr_EnumType* error_typexpr;
external Ast_Ident* void_type_ast;
internal Typexpr_Basic* void_typexpr;
external Ast_Ident* bool_type_ast;
internal Typexpr_Basic* bool_typexpr;
external Ast_Ident* bit_type_ast;
internal Typexpr_Basic* bit_typexpr;
external Ast_Ident* varbit_type_ast;
internal Typexpr_Basic* varbit_typexpr;
external Ast_Ident* int_type_ast;
internal Typexpr_Basic* int_typexpr;
external Ast_Ident* string_type_ast;
internal Typexpr_Basic* string_typexpr;

internal Typexpr*
visit_typeref(Ast_Typeref* typeref_ast)
{
  assert (!typeref_ast->typexpr);
  Typexpr* typeref_typexpr = typeref_ast->type_ast->typexpr;
  typeref_ast->typexpr = typeref_typexpr;
  return typeref_typexpr;
}

internal Typexpr_TypeParameter*
visit_type_parameter(Ast_TypeParameter* parameter_ast)
{
  assert (!parameter_ast->typexpr);
  Typexpr_TypeParameter* parameter_typexpr = 0;
  if (parameter_ast->parameter_kind == AST_TYPPARAM_VAR)
  {
    parameter_typexpr = arena_push_struct(&arena, Typexpr_TypeParameter);
    zero_struct(parameter_typexpr, Typexpr_TypeParameter);
    parameter_ast->typexpr = (Typexpr*)parameter_typexpr;
    parameter_typexpr->kind = TYP_TYPE_PARAMETER;
    parameter_typexpr->name = parameter_ast->name;
  }
  else
    error("at line %d: type variable was expected");
  return parameter_typexpr;
}

internal Typexpr_Parameter*
visit_parameter(Ast_Parameter* parameter_ast)
{
  assert (!parameter_ast->typexpr);
  Typexpr_Parameter* parameter_typexpr = arena_push_struct(&arena, Typexpr_Parameter);
  zero_struct(parameter_typexpr, Typexpr_Parameter);
  parameter_ast->typexpr = (Typexpr*)parameter_typexpr;
  parameter_typexpr->kind = TYP_PARAMETER;

  if (parameter_ast->direction == AST_DIR_IN)
    parameter_typexpr->direction = TYP_DIR_IN;
  else if (parameter_ast->direction == AST_DIR_OUT)
    parameter_typexpr->direction = TYP_DIR_OUT;
  else if (parameter_ast->direction == AST_DIR_INOUT)
    parameter_typexpr->direction = TYP_DIR_INOUT;

  parameter_typexpr->type = visit_typeref(parameter_ast->typeref);

  return parameter_typexpr;
}

internal Typexpr_Function*
visit_function_prototype(Ast_FunctionDecl* function_ast)
{
  assert (!function_ast->typexpr);
  Typexpr_Function* function_typexpr = arena_push_struct(&arena, Typexpr_Function);
  zero_struct(function_typexpr, Typexpr_Function);
  function_ast->typexpr = (Typexpr*)function_typexpr;
  function_typexpr->kind = TYP_FUNCTION;
  function_typexpr->name = function_ast->name;
  function_typexpr->is_prototype = true;

  function_typexpr->return_type = function_ast->return_type_ast->typexpr;

  function_typexpr->sentinel_type_parameter = arena_push_struct(&arena, Typexpr_TypeParameter);
  zero_struct(function_typexpr->sentinel_type_parameter, Typexpr_TypeParameter);
  function_typexpr->sentinel_type_parameter->kind = TYP_TYPE_PARAMETER;
  function_typexpr->last_type_parameter = function_typexpr->sentinel_type_parameter;

  Ast_TypeParameter* type_parameter_ast = function_ast->first_type_parameter;
  while (type_parameter_ast)
  {
    Typexpr_TypeParameter* type_parameter_typexpr = visit_type_parameter(type_parameter_ast);
    function_typexpr->last_type_parameter->next_type_parameter = type_parameter_typexpr;
    function_typexpr->last_type_parameter = type_parameter_typexpr;
    function_typexpr->type_parameter_count += 1;

    type_parameter_ast = (Ast_TypeParameter*)type_parameter_ast->next_parameter;
  }

  function_typexpr->sentinel_parameter = arena_push_struct(&arena, Typexpr_Parameter);
  zero_struct(function_typexpr->sentinel_parameter, Typexpr_Parameter);
  function_typexpr->sentinel_parameter->kind = TYP_PARAMETER;
  function_typexpr->last_parameter = function_typexpr->sentinel_parameter;
  
  Ast_Parameter* parameter_ast = function_ast->first_parameter;
  while (parameter_ast)
  {
    Typexpr_Parameter* parameter_typexpr = visit_parameter(parameter_ast);
    function_typexpr->last_parameter->next_parameter = parameter_typexpr;
    function_typexpr->last_parameter = parameter_typexpr;
    function_typexpr->parameter_count += 1;

    parameter_ast = (Ast_Parameter*)parameter_ast->next_parameter;
  }
  return function_typexpr;
}

internal Typexpr_ExternObject*
visit_extern_object_prototype(Ast_ExternObjectDecl* object_ast)
{
  assert (!object_ast->typexpr);
  Typexpr_ExternObject* object_typexpr = arena_push_struct(&arena, Typexpr_ExternObject);
  zero_struct(object_typexpr, Typexpr_ExternObject);
  object_ast->typexpr = (Typexpr*)object_typexpr;
  object_typexpr->kind = TYP_EXTERN_OBJECT;
  object_typexpr->name = object_ast->name;
  object_typexpr->is_prototype = true;

  object_typexpr->sentinel_function = arena_push_struct(&arena, Typexpr_Function);
  zero_struct(object_typexpr->sentinel_function, Typexpr_Function);
  object_typexpr->sentinel_function->kind = TYP_FUNCTION;
  object_typexpr->last_function = object_typexpr->sentinel_function;

  Ast_FunctionDecl* method_ast = object_ast->first_method;
  while (method_ast)
  {
    Typexpr_Function* method_typexpr = visit_function_prototype(method_ast);
    object_typexpr->last_function->next_function = method_typexpr;
    object_typexpr->last_function = method_typexpr;
    object_typexpr->method_count += 1;

    method_ast = (Ast_FunctionDecl*)method_ast->next_decl;
  }
  return object_typexpr;
}

internal Typexpr_Function*
visit_extern_function_prototype(Ast_FunctionDecl* function_ast)
{
  assert (!function_ast->typexpr);
  Typexpr_Function* function_typexpr = visit_function_prototype(function_ast);
  return function_typexpr;
}

internal Typexpr*
visit_parser_prototype(Ast_ParserDecl* parser_ast)
{
  assert (!parser_ast->typexpr);
  Typexpr* parser_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(parser_typexpr, Typexpr);
  parser_ast->typexpr = parser_typexpr;
  parser_typexpr->kind = TYP_PARSER;
  parser_typexpr->is_prototype = true;
  return parser_typexpr;
}

internal Typexpr*
visit_parser_type(Ast_ParserDecl* parser_ast)
{
  assert (!parser_ast->typexpr);
  Typexpr* parser_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(parser_typexpr, Typexpr);
  parser_ast->typexpr = parser_typexpr;
  parser_typexpr->kind = TYP_PARSER;
  return parser_typexpr;
}

internal Typexpr*
visit_control_prototype(Ast_ControlDecl* control_ast)
{
  assert (!control_ast->typexpr);
  Typexpr* control_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(control_typexpr, Typexpr);
  control_ast->typexpr = control_typexpr;
  control_typexpr->kind = TYP_CONTROL;
  control_typexpr->is_prototype = true;
  return control_typexpr;
}

internal Typexpr*
visit_control_type(Ast_ControlDecl* control_ast)
{
  assert (!control_ast->typexpr);
  Typexpr* control_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(control_typexpr, Typexpr);
  control_ast->typexpr = control_typexpr;
  control_typexpr->kind = TYP_CONTROL;
  return control_typexpr;
}

internal Typexpr*
visit_package_prototype(Ast_PackageDecl* package_ast)
{
  assert (!package_ast->typexpr);
  Typexpr* package_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(package_typexpr, Typexpr);
  package_ast->typexpr = package_typexpr;
  package_typexpr->kind = TYP_PACKAGE;
  package_typexpr->is_prototype = true;
  return package_typexpr;
}

internal Typexpr*
visit_typedef(Ast_Typedef* typedef_ast)
{
  assert (!typedef_ast->typexpr);
  Typexpr* typedef_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(typedef_typexpr, Typexpr);
  typedef_ast->typexpr = typedef_typexpr;
  typedef_typexpr->kind = TYP_TYPEDEF;
  return typedef_typexpr;
}

internal Typexpr*
visit_header_prototype(Ast_HeaderDecl* header_ast)
{
  assert (!header_ast->typexpr);
  Typexpr* header_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(header_typexpr, Typexpr);
  header_ast->typexpr = header_typexpr;
  header_typexpr->kind = TYP_HEADER;
  header_typexpr->is_prototype = true;
  return header_typexpr;
}

internal Typexpr*
visit_header_type(Ast_HeaderDecl* header_ast)
{
  assert (!header_ast->typexpr);
  Typexpr* header_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(header_typexpr, Typexpr);
  header_ast->typexpr = header_typexpr;
  header_typexpr->kind = TYP_HEADER;
  return header_typexpr;
}

internal Typexpr_EnumField*
visit_error_code(Ast_ErrorCode* code_ast)
{
  assert (!code_ast->typexpr);
  Typexpr_EnumField* code_typexpr = arena_push_struct(&arena, Typexpr_EnumField);
  zero_struct(code_typexpr, Typexpr_EnumField);
  code_ast->typexpr = (Typexpr*)code_typexpr;
  code_typexpr->kind = TYP_ENUM_FIELD;
  code_typexpr->name = code_ast->name;
  return code_typexpr;
}

internal Typexpr_EnumType*
visit_error_type(Ast_ErrorType* error_ast)
{
  assert (!error_ast->typexpr);
  error_ast->typexpr = error_type_ast->typexpr;

  if (!error_ast->error_code)
    error("at line %d: error declaration must have at least one error code", error_ast->line_nr);

  Ast_ErrorCode* error_code_ast = error_ast->error_code;
  while (error_code_ast)
  {
    Typexpr_EnumField* error_code_typexpr = visit_error_code(error_code_ast);
    error_typexpr->last_field->next_field = error_code_typexpr;
    error_typexpr->last_field = error_code_typexpr;
    error_typexpr->field_count += 1;

    error_code_ast = (Ast_ErrorCode*)error_code_ast->next_ident;
  }
  return error_typexpr;
}

internal Typexpr*
visit_struct_prototype(Ast_StructDecl* struct_ast)
{
  assert (!struct_ast->typexpr);
  Typexpr* struct_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(struct_typexpr, Typexpr);
  struct_ast->typexpr = struct_typexpr;
  struct_typexpr->kind = TYP_STRUCT;
  struct_typexpr->is_prototype = true;
  return struct_typexpr;
}

internal Typexpr*
visit_struct_type(Ast_StructDecl* struct_ast)
{
  assert (!struct_ast->typexpr);
  Typexpr* struct_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(struct_typexpr, Typexpr);
  struct_ast->typexpr = struct_typexpr;
  struct_typexpr->kind = TYP_STRUCT;
  return struct_typexpr;
}

internal void
visit_p4declaration(Ast_Declaration* p4decl_ast)
{
  if (p4decl_ast->kind == AST_STRUCT_PROTOTYPE)
    visit_struct_prototype((Ast_StructDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_STRUCT_DECL)
    visit_struct_type((Ast_StructDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_HEADER_PROTOTYPE)
    visit_header_prototype((Ast_HeaderDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_HEADER_DECL)
    visit_header_type((Ast_HeaderDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_ERROR_TYPE)
    visit_error_type((Ast_ErrorType*)p4decl_ast);
  else if (p4decl_ast->kind == AST_TYPEDEF)
    visit_typedef((Ast_Typedef*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PARSER_PROTOTYPE)
    visit_parser_prototype((Ast_ParserDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PARSER_DECL)
    visit_parser_type((Ast_ParserDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_CONTROL_PROTOTYPE)
    visit_control_prototype((Ast_ControlDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_CONTROL_DECL)
    visit_control_type((Ast_ControlDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_PROTOTYPE)
    visit_package_prototype((Ast_PackageDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_OBJECT_PROTOTYPE)
    visit_extern_object_prototype((Ast_ExternObjectDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_FUNCTION_PROTOTYPE)
    visit_extern_function_prototype((Ast_FunctionDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_INSTANCE)
    ; // not a type declataration - pass.
  else
    assert (false);
}

internal void
visit_p4program(Ast_P4Program* p4program)
{
  Ast_Declaration* p4decl_ast = p4program->declaration;
  while (p4decl_ast)
  {
    visit_p4declaration(p4decl_ast);
    p4decl_ast = p4decl_ast->next_decl;
  }
}

void
build_typexpr()
{
  // 'error' type
  error_typexpr = arena_push_struct(&arena, Typexpr_EnumType);
  zero_struct(error_typexpr, Typexpr_EnumType);
  error_type_ast->typexpr = (Typexpr*)error_typexpr;
  error_typexpr->kind = TYP_ENUM;
  error_typexpr->name = error_type_ast->name;

  error_typexpr->sentinel_field = arena_push_struct(&arena, Typexpr_EnumField);
  zero_struct(error_typexpr->sentinel_field, Typexpr_EnumField);
  error_typexpr->sentinel_field->kind = TYP_ENUM_FIELD;
  error_typexpr->last_field = error_typexpr->sentinel_field;

  // 'void' type
  void_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(void_typexpr, Typexpr_Basic);
  void_type_ast->typexpr = (Typexpr*)void_typexpr;
  void_typexpr->kind = TYP_BASIC;
  void_typexpr->basic_type = BASTYP_VOID;
  void_typexpr->name = void_type_ast->name;

  // 'bool' type
  bool_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(bool_typexpr, Typexpr_Basic);
  bool_type_ast->typexpr = (Typexpr*)bool_typexpr;
  bool_typexpr->kind = TYP_BASIC;
  bool_typexpr->basic_type = BASTYP_BOOL;
  bool_typexpr->name = bool_type_ast->name;

  // 'bit' type
  bit_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(bit_typexpr, Typexpr_Basic);
  bit_type_ast->typexpr = (Typexpr*)bit_typexpr;
  bit_typexpr->kind = TYP_BASIC;
  bit_typexpr->basic_type = BASTYP_BIT;
  bit_typexpr->name = bit_type_ast->name;

  // 'varbit' type
  varbit_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(varbit_typexpr, Typexpr_Basic);
  varbit_type_ast->typexpr = (Typexpr*)varbit_typexpr;
  varbit_typexpr->kind = TYP_BASIC;
  varbit_typexpr->basic_type = BASTYP_VARBIT;
  varbit_typexpr->name = varbit_type_ast->name;

  // 'int' type
  int_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(int_typexpr, Typexpr_Basic);
  int_type_ast->typexpr = (Typexpr*)int_typexpr;
  int_typexpr->kind = TYP_BASIC;
  int_typexpr->basic_type = BASTYP_INT;
  int_typexpr->name = int_type_ast->name;

  // 'string' type
  string_typexpr = arena_push_struct(&arena, Typexpr_Basic);
  zero_struct(string_typexpr, Typexpr_Basic);
  string_type_ast->typexpr = (Typexpr*)string_typexpr;
  string_typexpr->kind = TYP_BASIC;
  string_typexpr->basic_type = BASTYP_STRING;
  string_typexpr->name = string_type_ast->name;

  visit_p4program(p4program);

  arena_print_usage(&arena, "Memory (build_typexpr): ");
}

