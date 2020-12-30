#include "dp4c.h"

external Arena arena;
external Ast_P4Program* p4program;

external Ast_Ident* error_type_ast;
internal Typexpr_Enum* error_typexpr;
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

internal Typexpr* visit_expression(Ast_Expression* expr_ast);

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
  parameter_typexpr->name = parameter_ast->name;
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

internal Typexpr_Parser*
visit_parser_prototype(Ast_ParserDecl* parser_ast)
{
  assert (!parser_ast->typexpr);
  Typexpr_Parser* parser_typexpr = arena_push_struct(&arena, Typexpr_Parser);
  zero_struct(parser_typexpr, Typexpr_Parser);
  parser_ast->typexpr = (Typexpr*)parser_typexpr;
  parser_typexpr->kind = TYP_PARSER;
  parser_typexpr->is_prototype = true;

  parser_typexpr->sentinel_type_parameter = arena_push_struct(&arena, Typexpr_TypeParameter);
  zero_struct(parser_typexpr->sentinel_type_parameter, Typexpr_TypeParameter);
  parser_typexpr->sentinel_type_parameter->kind = TYP_TYPE_PARAMETER;
  parser_typexpr->last_type_parameter = parser_typexpr->sentinel_type_parameter;

  Ast_TypeParameter* type_parameter_ast = parser_ast->first_type_parameter;
  while (type_parameter_ast)
  {
    Typexpr_TypeParameter* type_parameter_typexpr = visit_type_parameter(type_parameter_ast);
    parser_typexpr->last_type_parameter->next_type_parameter = type_parameter_typexpr;
    parser_typexpr->last_type_parameter = type_parameter_typexpr;
    parser_typexpr->type_parameter_count += 1;

    type_parameter_ast = (Ast_TypeParameter*)type_parameter_ast->next_parameter;
  }

  parser_typexpr->sentinel_parameter = arena_push_struct(&arena, Typexpr_Parameter);
  zero_struct(parser_typexpr->sentinel_parameter, Typexpr_Parameter);
  parser_typexpr->sentinel_parameter->kind = TYP_PARAMETER;
  parser_typexpr->last_parameter = parser_typexpr->sentinel_parameter;
  
  Ast_Parameter* parameter_ast = parser_ast->first_parameter;
  while (parameter_ast)
  {
    Typexpr_Parameter* parameter_typexpr = visit_parameter(parameter_ast);
    parser_typexpr->last_parameter->next_parameter = parameter_typexpr;
    parser_typexpr->last_parameter = parameter_typexpr;
    parser_typexpr->parameter_count += 1;

    parameter_ast = (Ast_Parameter*)parameter_ast->next_parameter;
  }

  return parser_typexpr;
}

internal Typexpr_BinaryExpr*
visit_binary_expression(Ast_BinaryExpr* expr_ast)
{
  assert (!expr_ast->typexpr);
  Typexpr_BinaryExpr* expr_typexpr = arena_push_struct(&arena, Typexpr_BinaryExpr);
  zero_struct(expr_typexpr, Typexpr_BinaryExpr);
  expr_typexpr->kind = TYP_BINARY_EXPR;

  expr_typexpr->l_type = visit_expression(expr_ast->l_operand);
  expr_typexpr->r_type = visit_expression(expr_ast->r_operand);

  if (expr_ast->op == AST_OP_MEMBER_SELECTOR)
    expr_typexpr->op = TYP_OP_MEMBER_SELECTOR;
  else if (expr_ast->op == AST_OP_LOGIC_EQUAL)
    expr_typexpr->op = TYP_OP_LOGIC_EQUAL;
  else if (expr_ast->op == AST_OP_FUNCTION_CALL)
    expr_typexpr->op = TYP_OP_FUNCTION_CALL;
  else if (expr_ast->op == AST_OP_ASSIGN)
    expr_typexpr->op = TYP_OP_ASSIGN;
  else if (expr_ast->op == AST_OP_ADDITION)
    expr_typexpr->op = TYP_OP_ADDITION;
  else if (expr_ast->op == AST_OP_SUBTRACT)
    expr_typexpr->op = TYP_OP_SUBSTRACT;

  return expr_typexpr;
}

internal Typexpr_Unknown*
visit_ident_expression(Ast_IdentExpr* ident_ast)
{
  assert (!ident_ast->typexpr);
  Typexpr_Unknown* ident_typexpr = arena_push_struct(&arena, Typexpr_Unknown);
  zero_struct(ident_typexpr, Typexpr_Unknown);
  ident_ast->typexpr = (Typexpr*)ident_typexpr;
  ident_typexpr->kind = TYP_UNKNOWN;
  ident_typexpr->ast = (Ast*)ident_ast;
  return ident_typexpr;
}

internal Typexpr*
visit_type_ident_expression(Ast_TypeIdentExpr* ident_ast)
{
  assert (!ident_ast->typexpr);
  Typexpr_Unknown* ident_typexpr = arena_push_struct(&arena, Typexpr_Unknown);
  zero_struct(ident_typexpr, Typexpr_Unknown);
  ident_ast->typexpr = (Typexpr*)ident_typexpr;
  ident_typexpr->kind = TYP_UNKNOWN;
  ident_typexpr->ast = (Ast*)ident_ast;
  return (Typexpr*)ident_typexpr;
}

internal Typexpr_Argument*
visit_argument(Ast_Expression* argument_ast)
{
  assert (!argument_ast->typexpr);
  Typexpr_Argument* argument_typexpr = arena_push_struct(&arena, Typexpr_Argument);
  zero_struct(argument_typexpr, Typexpr_Argument);
  argument_typexpr->kind = TYP_ARGUMENT;

  argument_typexpr->type = visit_expression(argument_ast);
  
  return argument_typexpr;
}

internal Typexpr_FunctionCall*
visit_function_call(Ast_FunctionCall* call_ast)
{
  assert (!call_ast->typexpr);
  Typexpr_FunctionCall* call_typexpr = arena_push_struct(&arena, Typexpr_FunctionCall);
  zero_struct(call_typexpr, Typexpr_FunctionCall);
  call_typexpr->kind = TYP_FUNCTION_CALL;

  call_typexpr->function = visit_expression(call_ast->function);

  call_typexpr->sentinel_argument = arena_push_struct(&arena, Typexpr_Argument);
  zero_struct(call_typexpr->sentinel_argument, Typexpr_Argument);
  call_typexpr->sentinel_argument->kind = TYP_ARGUMENT;
  call_typexpr->last_argument = call_typexpr->sentinel_argument;

  Ast_Expression* argument_ast = call_ast->first_argument;
  while (argument_ast)
  {
    Typexpr_Argument* argument_typexpr = visit_argument(argument_ast);
    call_typexpr->last_argument->next_argument = argument_typexpr;
    call_typexpr->last_argument = argument_typexpr;
    call_typexpr->argument_count += 1;

    argument_ast = argument_ast->next_expression;
  }
  return call_typexpr;
}

internal Typexpr*
visit_expression(Ast_Expression* expr_ast)
{
  assert (!expr_ast->typexpr);
  Typexpr* expr_typexpr = 0;

  if (expr_ast->kind == AST_FUNCTION_CALL)
    expr_typexpr = (Typexpr*)visit_function_call((Ast_FunctionCall*)expr_ast);
  else if (expr_ast->kind == AST_BINARY_EXPR)
    expr_typexpr = (Typexpr*)visit_binary_expression((Ast_BinaryExpr*)expr_ast);
  else if (expr_ast->kind == AST_IDENT_EXPR)
    expr_typexpr = (Typexpr*)visit_ident_expression((Ast_IdentExpr*)expr_ast);
  else if (expr_ast->kind == AST_TYPE_IDENT_EXPR)
    expr_typexpr = (Typexpr*)visit_type_ident_expression((Ast_TypeIdentExpr*)expr_ast);
  else if (expr_ast->kind == AST_INTEGER_EXPR)
    ;
  else if (expr_ast->kind == AST_WINTEGER_EXPR)
    ;
  else if (expr_ast->kind == AST_SINTEGER_EXPR)
    ;
  else
    assert (false);

  return expr_typexpr;
}

internal Typexpr*
visit_parser_state(Ast_ParserState* state_ast)
{
  assert (!state_ast->typexpr);
  Typexpr* state_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(state_typexpr, Typexpr);
  state_ast->typexpr = state_typexpr;

  Ast_Expression* expr_ast = state_ast->first_statement;
  while (expr_ast)
  {
    visit_expression(expr_ast);
    expr_ast = expr_ast->next_expression;
  }

  return state_typexpr;
}

internal Typexpr_Parser*
visit_parser_decl(Ast_ParserDecl* parser_ast)
{
  assert (!parser_ast->typexpr);
  Typexpr_Parser* parser_typexpr = visit_parser_prototype(parser_ast);
  parser_typexpr->is_prototype = false;

  Ast_ParserState* state_ast = parser_ast->first_parser_state;
  while (state_ast)
  {
    visit_parser_state(state_ast);
    state_ast = state_ast->next_state;
  }
  return parser_typexpr;
}

internal Typexpr_Control*
visit_control_prototype(Ast_ControlDecl* control_ast)
{
  assert (!control_ast->typexpr);
  Typexpr_Control* control_typexpr = arena_push_struct(&arena, Typexpr_Control);
  zero_struct(control_typexpr, Typexpr_Control);
  control_ast->typexpr = (Typexpr*)control_typexpr;
  control_typexpr->kind = TYP_CONTROL;
  control_typexpr->is_prototype = true;

  control_typexpr->sentinel_type_parameter = arena_push_struct(&arena, Typexpr_TypeParameter);
  zero_struct(control_typexpr->sentinel_type_parameter, Typexpr_TypeParameter);
  control_typexpr->sentinel_type_parameter->kind = TYP_TYPE_PARAMETER;
  control_typexpr->last_type_parameter = control_typexpr->sentinel_type_parameter;

  Ast_TypeParameter* type_parameter_ast = control_ast->first_type_parameter;
  while (type_parameter_ast)
  {
    Typexpr_TypeParameter* type_parameter_typexpr = visit_type_parameter(type_parameter_ast);
    control_typexpr->last_type_parameter->next_type_parameter = type_parameter_typexpr;
    control_typexpr->last_type_parameter = type_parameter_typexpr;
    control_typexpr->type_parameter_count += 1;

    type_parameter_ast = (Ast_TypeParameter*)type_parameter_ast->next_parameter;
  }

  control_typexpr->sentinel_parameter = arena_push_struct(&arena, Typexpr_Parameter);
  zero_struct(control_typexpr->sentinel_parameter, Typexpr_Parameter);
  control_typexpr->sentinel_parameter->kind = TYP_PARAMETER;
  control_typexpr->last_parameter = control_typexpr->sentinel_parameter;
  
  Ast_Parameter* parameter_ast = control_ast->first_parameter;
  while (parameter_ast)
  {
    Typexpr_Parameter* parameter_typexpr = visit_parameter(parameter_ast);
    control_typexpr->last_parameter->next_parameter = parameter_typexpr;
    control_typexpr->last_parameter = parameter_typexpr;
    control_typexpr->parameter_count += 1;

    parameter_ast = (Ast_Parameter*)parameter_ast->next_parameter;
  }

  return control_typexpr;
}

internal Typexpr*
visit_control_decl(Ast_ControlDecl* control_ast)
{
  assert (!control_ast->typexpr);
  Typexpr* control_typexpr = arena_push_struct(&arena, Typexpr);
  zero_struct(control_typexpr, Typexpr);
  control_ast->typexpr = control_typexpr;
  control_typexpr->kind = TYP_CONTROL;
  return control_typexpr;
}

internal Typexpr_Package*
visit_package_prototype(Ast_PackageDecl* package_ast)
{
  assert (!package_ast->typexpr);
  Typexpr_Package* package_typexpr = arena_push_struct(&arena, Typexpr_Package);
  zero_struct(package_typexpr, Typexpr_Package);
  package_ast->typexpr = (Typexpr*)package_typexpr;
  package_typexpr->kind = TYP_PACKAGE;
  package_typexpr->is_prototype = true;

  package_typexpr->sentinel_type_parameter = arena_push_struct(&arena, Typexpr_TypeParameter);
  zero_struct(package_typexpr->sentinel_type_parameter, Typexpr_TypeParameter);
  package_typexpr->sentinel_type_parameter->kind = TYP_TYPE_PARAMETER;
  package_typexpr->last_type_parameter = package_typexpr->sentinel_type_parameter;

  Ast_TypeParameter* type_parameter_ast = package_ast->first_type_parameter;
  while (type_parameter_ast)
  {
    Typexpr_TypeParameter* type_parameter_typexpr = visit_type_parameter(type_parameter_ast);
    package_typexpr->last_type_parameter->next_type_parameter = type_parameter_typexpr;
    package_typexpr->last_type_parameter = type_parameter_typexpr;
    package_typexpr->type_parameter_count += 1;

    type_parameter_ast = (Ast_TypeParameter*)type_parameter_ast->next_parameter;
  }

  package_typexpr->sentinel_parameter = arena_push_struct(&arena, Typexpr_Parameter);
  zero_struct(package_typexpr->sentinel_parameter, Typexpr_Parameter);
  package_typexpr->sentinel_parameter->kind = TYP_PARAMETER;
  package_typexpr->last_parameter = package_typexpr->sentinel_parameter;
  
  Ast_Parameter* parameter_ast = package_ast->first_parameter;
  while (parameter_ast)
  {
    Typexpr_Parameter* parameter_typexpr = visit_parameter(parameter_ast);
    package_typexpr->last_parameter->next_parameter = parameter_typexpr;
    package_typexpr->last_parameter = parameter_typexpr;
    package_typexpr->parameter_count += 1;

    parameter_ast = (Ast_Parameter*)parameter_ast->next_parameter;
  }

  return package_typexpr;
}

internal Typexpr_Typedef*
visit_typedef(Ast_Typedef* typedef_ast)
{
  assert (!typedef_ast->typexpr);
  Typexpr_Typedef* typedef_typexpr = arena_push_struct(&arena, Typexpr_Typedef);
  zero_struct(typedef_typexpr, Typexpr_Typedef);
  typedef_ast->typexpr = (Typexpr*)typedef_typexpr;
  typedef_typexpr->kind = TYP_TYPEDEF;
  typedef_typexpr->name = typedef_ast->name;

  typedef_typexpr->type = visit_typeref(typedef_ast->typeref);

  return typedef_typexpr;
}

internal Typexpr_StructField*
visit_struct_field(Ast_StructField* field_ast)
{
  assert (!field_ast->typexpr);
  Typexpr_StructField* field_typexpr = arena_push_struct(&arena, Typexpr_StructField);
  zero_struct(field_typexpr, Typexpr_StructField);
  field_ast->typexpr = (Typexpr*)field_typexpr;
  field_typexpr->kind = TYP_STRUCT_FIELD;
  field_typexpr->name = field_ast->name;

  field_typexpr->type = visit_typeref(field_ast->typeref);

  return field_typexpr;
}

internal Typexpr_Header*
visit_header_prototype(Ast_HeaderDecl* header_ast)
{
  assert (!header_ast->typexpr);
  Typexpr_Header* header_typexpr = arena_push_struct(&arena, Typexpr_Header);
  zero_struct(header_typexpr, Typexpr_Header);
  header_ast->typexpr = (Typexpr*)header_typexpr;
  header_typexpr->kind = TYP_HEADER;
  header_typexpr->is_prototype = true;
  return header_typexpr;
}

internal Typexpr_Header*
visit_header_decl(Ast_HeaderDecl* header_ast)
{
  assert (!header_ast->typexpr);
  Typexpr_Header* header_typexpr = visit_header_prototype(header_ast);
  header_typexpr->is_prototype = false;

  header_typexpr->sentinel_field = arena_push_struct(&arena, Typexpr_StructField);
  zero_struct(header_typexpr->sentinel_field, Typexpr_StructField);
  header_typexpr->sentinel_field->kind = TYP_STRUCT_FIELD;
  header_typexpr->last_field = header_typexpr->sentinel_field;

  Ast_StructField* field_ast = header_ast->first_field;
  while (field_ast)
  {
    Typexpr_StructField* field_typexpr = visit_struct_field(field_ast);
    header_typexpr->last_field->next_field = field_typexpr;
    header_typexpr->last_field = field_typexpr;
    header_typexpr->field_count += 1;

    field_ast = (Ast_StructField*)field_ast->next_field;
  }

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

internal Typexpr_Enum*
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

    error_code_ast = (Ast_ErrorCode*)error_code_ast->next_code;
  }
  return error_typexpr;
}

internal Typexpr_Struct*
visit_struct_prototype(Ast_StructDecl* struct_ast)
{
  assert (!struct_ast->typexpr);
  Typexpr_Struct* struct_typexpr = arena_push_struct(&arena, Typexpr_Struct);
  zero_struct(struct_typexpr, Typexpr_Struct);
  struct_ast->typexpr = (Typexpr*)struct_typexpr;
  struct_typexpr->kind = TYP_STRUCT;
  struct_typexpr->is_prototype = true;
  return struct_typexpr;
}

internal Typexpr_Struct*
visit_struct_decl(Ast_StructDecl* struct_ast)
{
  assert (!struct_ast->typexpr);
  Typexpr_Struct* struct_typexpr = visit_struct_prototype(struct_ast);
  struct_typexpr->is_prototype = false;

  struct_typexpr->sentinel_field = arena_push_struct(&arena, Typexpr_StructField);
  zero_struct(struct_typexpr->sentinel_field, Typexpr_StructField);
  struct_typexpr->sentinel_field->kind = TYP_STRUCT_FIELD;
  struct_typexpr->last_field = struct_typexpr->sentinel_field;

  Ast_StructField* field_ast = struct_ast->first_field;
  while (field_ast)
  {
    Typexpr_StructField* field_typexpr = visit_struct_field(field_ast);
    struct_typexpr->last_field->next_field = field_typexpr;
    struct_typexpr->last_field = field_typexpr;
    struct_typexpr->field_count += 1;

    field_ast = (Ast_StructField*)field_ast->next_field;
  }

  return struct_typexpr;
}

internal Typexpr*
visit_package_instantiation(Ast_PackageInstantiation* instance_ast)
{
  return 0;  // not a type declaration
}

internal Typexpr*
visit_p4declaration(Ast_Declaration* p4decl_ast)
{
  assert (!p4decl_ast->typexpr);

  if (p4decl_ast->kind == AST_STRUCT_PROTOTYPE)
    visit_struct_prototype((Ast_StructDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_STRUCT_DECL)
    visit_struct_decl((Ast_StructDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_HEADER_PROTOTYPE)
    visit_header_prototype((Ast_HeaderDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_HEADER_DECL)
    visit_header_decl((Ast_HeaderDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_ERROR_TYPE)
    visit_error_type((Ast_ErrorType*)p4decl_ast);
  else if (p4decl_ast->kind == AST_TYPEDEF)
    visit_typedef((Ast_Typedef*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PARSER_PROTOTYPE)
    visit_parser_prototype((Ast_ParserDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PARSER_DECL)
    visit_parser_decl((Ast_ParserDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_CONTROL_PROTOTYPE)
    visit_control_prototype((Ast_ControlDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_CONTROL_DECL)
    visit_control_decl((Ast_ControlDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_PROTOTYPE)
    visit_package_prototype((Ast_PackageDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_OBJECT_PROTOTYPE)
    visit_extern_object_prototype((Ast_ExternObjectDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_EXTERN_FUNCTION_PROTOTYPE)
    visit_extern_function_prototype((Ast_FunctionDecl*)p4decl_ast);
  else if (p4decl_ast->kind == AST_PACKAGE_INSTANCE)
    visit_package_instantiation((Ast_PackageInstantiation*)p4decl_ast);
  else
    assert (false);

  return 0;
}

internal Typexpr*
visit_p4program(Ast_P4Program* p4program_ast)
{
  assert (!p4program_ast->typexpr);

  Ast_Declaration* p4decl_ast = p4program_ast->first_declaration;
  while (p4decl_ast)
  {
    visit_p4declaration(p4decl_ast);
    p4decl_ast = p4decl_ast->next_decl;
  }

  return 0;
}

void
build_typexpr()
{
  // 'error' type
  error_typexpr = arena_push_struct(&arena, Typexpr_Enum);
  zero_struct(error_typexpr, Typexpr_Enum);
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
}

