#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena*  storage;
static Hashmap type_table = {};

/** PROGRAM **/

static void visit_p4program(Ast* p4program);
static void visit_declarationList(Ast* decl_list);
static void visit_declaration(Ast* decl);
static void visit_name(Ast* name);
static void visit_parameterList(Ast* params);
static void visit_parameter(Ast* param);
static void visit_packageTypeDeclaration(Ast* package_decl);
static void visit_instantiation(Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(Ast* parser_decl);
static void visit_parserTypeDeclaration(Ast* type_decl);
static void visit_parserLocalElements(Ast* local_elements);
static void visit_parserLocalElement(Ast* local_element);
static void visit_parserStates(Ast* states);
static void visit_parserState(Ast* state);
static void visit_parserStatements(Ast* stmts);
static void visit_parserStatement(Ast* stmt);
static void visit_parserBlockStatement(Ast* block_stmt);
static void visit_transitionStatement(Ast* transition_stmt);
static void visit_stateExpression(Ast* state_expr);
static void visit_selectExpression(Ast* select_expr);
static void visit_selectCaseList(Ast* case_list);
static void visit_selectCase(Ast* select_case);
static void visit_keysetExpression(Ast* keyset_expr);
static void visit_tupleKeysetExpression(Ast* tuple_expr);
static void visit_simpleKeysetExpression(Ast* simple_expr);
static void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(Ast* control_decl);
static void visit_controlTypeDeclaration(Ast* type_decl);
static void visit_controlLocalDeclarations(Ast* local_decls);
static void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(Ast* extern_decl);
static void visit_externTypeDeclaration(Ast* type_decl);
static void visit_methodPrototypes(Ast* protos);
static void visit_functionPrototype(Ast* func_proto);

/** TYPES **/

static void visit_typeRef(Ast* type_ref);
static void visit_tupleType(Ast* type);
static void visit_headerStackType(Ast* type_decl);
static void visit_specializedType(Ast* type_decl);
static void visit_baseTypeBoolean(Ast* bool_type);
static void visit_baseTypeInteger(Ast* int_type);
static void visit_baseTypeBit(Ast* bit_type);
static void visit_baseTypeVarbit(Ast* varbit_type);
static void visit_baseTypeString(Ast* str_type);
static void visit_baseTypeVoid(Ast* void_type);
static void visit_baseTypeError(Ast* error_type);
static void visit_integerTypeSize(Ast* type_size);
static void visit_typeParameterList(Ast* param_list);
static void visit_realTypeArg(Ast* type_arg);
static void visit_typeArg(Ast* type_arg);
static void visit_realTypeArgumentList(Ast* arg_list);
static void visit_typeArgumentList(Ast* arg_list);
static void visit_typeDeclaration(Ast* type_decl);
static void visit_derivedTypeDeclaration(Ast* type_decl);
static void visit_headerTypeDeclaration(Ast* header_decl);
static void visit_headerUnionDeclaration(Ast* union_decl);
static void visit_structTypeDeclaration(Ast* struct_decl);
static void visit_structFieldList(Ast* field_list);
static void visit_structField(Ast* field);
static void visit_enumDeclaration(Ast* enum_decl);
static void visit_errorDeclaration(Ast* error_decl);
static void visit_matchKindDeclaration(Ast* match_decl);
static void visit_identifierList(Ast* ident_list);
static void visit_specifiedIdentifierList(Ast* ident_list);
static void visit_specifiedIdentifier(Ast* ident);
static void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(Ast* assign_stmt);
static void visit_functionCall(Ast* func_call);
static void visit_returnStatement(Ast* return_stmt);
static void visit_exitStatement(Ast* exit_stmt);
static void visit_conditionalStatement(Ast* cond_stmt);
static void visit_directApplication(Ast* applic_stmt);
static void visit_statement(Ast* stmt);
static void visit_blockStatement(Ast* block_stmt);
static void visit_statementOrDeclList(Ast* stmt_list);
static void visit_switchStatement(Ast* switch_stmt);
static void visit_switchCases(Ast* switch_cases);
static void visit_switchCase(Ast* switch_case);
static void visit_switchLabel(Ast* label);
static void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(Ast* table_decl);
static void visit_tablePropertyList(Ast* prop_list);
static void visit_tableProperty(Ast* table_prop);
static void visit_keyProperty(Ast* key_prop);
static void visit_keyElementList(Ast* element_list);
static void visit_keyElement(Ast* element);
static void visit_actionsProperty(Ast* actions_prop);
static void visit_actionList(Ast* action_list);
static void visit_actionRef(Ast* action_ref);
static void visit_entriesProperty(Ast* entries_prop);
static void visit_entriesList(Ast* entries_list);
static void visit_entry(Ast* entry);
static void visit_simpleProperty(Ast* simple_prop);
static void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(Ast* func_decl);
static void visit_argumentList(Ast* arg_list);
static void visit_argument(Ast* arg);
static void visit_expressionList(Ast* expr_list);
static void visit_lvalueExpression(Ast* lvalue_expr);
static void visit_expression(Ast* expr);
static void visit_castExpression(Ast* cast_expr);
static void visit_unaryExpression(Ast* unary_expr);
static void visit_binaryExpression(Ast* binary_expr);
static void visit_memberSelector(Ast* selector);
static void visit_arraySubscript(Ast* subscript);
static void visit_indexExpression(Ast* index_expr);
static void visit_booleanLiteral(Ast* bool_literal);
static void visit_integerLiteral(Ast* int_literal);
static void visit_stringLiteral(Ast* str_literal);
static void visit_default(Ast* _default);
static void visit_dontcare(Ast* dontcare);

static Ast*
name_of_type(Ast* type)
{
  if (type->kind == AST_typeRef) {
    Ast* type_ref = type;
    return name_of_type(type_ref->type);
  } else if (type->kind == AST_baseTypeBoolean) {
    Ast* bool_type = type;
    return bool_type->name;
  } else if (type->kind == AST_baseTypeInteger) {
    Ast* integer_type = type;
    return integer_type->name;
  } else if (type->kind == AST_baseTypeBit) {
    Ast* bit_type = type;
    return bit_type->name;
  } else if (type->kind == AST_baseTypeVarbit) {
    Ast* varbit_type = type;
    return varbit_type->name;
  } else if (type->kind == AST_baseTypeString) {
    Ast* string_type = type;
    return string_type->name;
  } else if (type->kind == AST_baseTypeVoid) {
    Ast* void_type = type;
    return void_type->name;
  } else if (type->kind == AST_baseTypeError) {
    Ast* error_type = type;
    return error_type->name;
  } else if (type->kind == AST_name) {
    Ast* name = type;
    return name;
  } else if (type->kind == AST_specializedType) {
    Ast* speclzd_type = type;
    return speclzd_type->name;
  } else if (type->kind == AST_headerStackType) {
    Ast* stack_type = type;
    return stack_type->name;
  } else if (type->kind == AST_tupleType) {
    Ast* tuple_type = type;
    return tuple_type->name;
  } else if (type->kind == AST_derivedTypeDeclaration) {
    Ast* derived_type = type;
    return name_of_type(derived_type->decl);
  } else if (type->kind == AST_structTypeDeclaration) {
    Ast* struct_type = type;
    return struct_type->name;
  } else if (type->kind == AST_headerTypeDeclaration) {
    Ast* header_type = type;
    return header_type->name;
  } else if (type->kind == AST_headerUnionDeclaration) {
    Ast* union_type = type;
    return union_type->name;
  } else if (type->kind == AST_enumDeclaration) {
    Ast* enum_type = type;
    return enum_type->name;
  } else if (type->kind == AST_dontcare) {
    Ast* dontcare_type = type;
    return dontcare_type->name;
  } else assert(0);
  return 0;
}

/** PROGRAM **/

static void
visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visit_declarationList(p4program->decl_list);
}

static void
visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  for (Ast* ast = decl_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(ast);
  }
}

static void
visit_declaration(Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_externDeclaration) {
    visit_externDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(decl->decl);
  } else if (decl->decl->kind == AST_instantiation) {
    visit_instantiation(decl->decl);
  } else assert(0);
}

static void
visit_name(Ast* name)
{
  assert(name->kind == AST_name);
  if (!hashmap_lookup(&type_table, HKEY_STRING, name->strname)) {
    Type_TypeVar* name_ty = arena_malloc(storage, sizeof(*name_ty));
    name_ty->ctor = TYPE_TYPEVAR;
    name_ty->strname = name->strname;
    hashmap_set(&type_table, storage, &name_ty, sizeof(name_ty), HKEY_STRING, name->strname);
  }
}

static void
visit_parameterList(Ast* params)
{
  assert(params->kind == AST_parameterList);
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(ast);
  }
}

static void
visit_parameter(Ast* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef(param->type);
  if (param->init_expr) {
    visit_expression(param->init_expr);
  }
}

static void
visit_packageTypeDeclaration(Ast* package_decl)
{
  assert(package_decl->kind == AST_packageTypeDeclaration);
  Ast* name = package_decl->name;
  Type_Function* package_ty = arena_malloc(storage, sizeof(*package_ty));
  package_ty->ctor = TYPE_FUNCTION;
  package_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &package_ty, sizeof(package_ty), HKEY_STRING, name->strname);
  if (package_decl->type_params) {
    visit_typeParameterList(package_decl->type_params);
  }
  Ast* params = package_decl->params;
  visit_parameterList(params);
  /* list_create(&package_ty->params_ty); */
  if (params->first_child) {
    Ast* ast = params->first_child;
    Ast* param = ast;
    for (ast = ast->right_sibling; ast != 0; ast = ast->right_sibling) {
      param = ast;
      /*
      list_append(&package_ty->params_ty, *(Type**)hashmap_lookup(
            &type_table, HKEY_STRING, name_of_type(param->type)->strname)); */
    }
  }
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef(inst->type_ref);
  visit_argumentList(inst->args);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_typeDeclaration(parser_decl->proto);
  if (parser_decl->ctor_params) {
    visit_parameterList(parser_decl->ctor_params);
  }
  visit_parserLocalElements(parser_decl->local_elements);
  visit_parserStates(parser_decl->states);
}

static void
visit_parserTypeDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserTypeDeclaration);
  Ast* name = parser_decl->name;
  Type_Function* parser_ty = arena_malloc(storage, sizeof(*parser_ty));
  parser_ty->ctor = TYPE_FUNCTION;
  parser_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &parser_ty, sizeof(parser_ty), HKEY_STRING, name->strname);
  if (parser_decl->type_params) {
    visit_typeParameterList(parser_decl->type_params);
  }
  Ast* params = parser_decl->params;
  visit_parameterList(params);
  /* list_create(&parser_ty->params_ty, storage, sizeof(Type*)); */
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* param = ast;
    /*
    list_append(&parser_ty->params_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(param->type)->strname)); */
  }
}

static void
visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  for (Ast* ast = local_elements->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(ast);
  }
}

static void
visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_element->element);
  } else if (local_element->element->kind == AST_instantiation) {
    visit_instantiation(local_element->element);
  } else assert(0);
}

static void
visit_parserStates(Ast* states)
{
  assert(states->kind == AST_parserStates);
  for (Ast* ast = states->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(ast);
  }
}

static void
visit_parserState(Ast* state)
{
  assert(state->kind == AST_parserState);
  visit_parserStatements(state->stmt_list);
  visit_transitionStatement(state->transition_stmt);
}

static void
visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  for (Ast* ast = stmts->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(ast);
  }
}

static void
visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->stmt);
  } else if (stmt->stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->stmt);
  } else if (stmt->stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->stmt);
  } else assert(0);
}

static void
visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(block_stmt->stmt_list);
}

static void
visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(transition_stmt->stmt);
}

static void
visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->expr->kind == AST_name) {
    ;
  } else if (state_expr->expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(select_expr->expr_list);
  visit_selectCaseList(select_expr->case_list);
}

static void
visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  for (Ast* ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(ast);
  }
}

static void
visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(select_case->keyset_expr);
}

static void
visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->expr);
  } else if (keyset_expr->expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->expr);
  } else assert(0);
}

static void
visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->expr_list);
}

static void
visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->expr->kind == AST_expression) {
    visit_expression(simple_expr->expr);
  } else if (simple_expr->expr->kind == AST_default) {
    visit_default(simple_expr->expr);
  } else if (simple_expr->expr->kind == AST_dontcare) {
    visit_dontcare(simple_expr->expr);
  } else assert(0);
}

static void
visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  for (Ast* ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(ast);
  }
}

/** CONTROL **/

static void
visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  visit_typeDeclaration(control_decl->proto);
  if (control_decl->ctor_params) {
    visit_parameterList(control_decl->ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->local_decls);
  visit_blockStatement(control_decl->apply_stmt);
}

static void
visit_controlTypeDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AST_controlTypeDeclaration);
  Ast* name = control_decl->name;
  Type_Function* control_ty = arena_malloc(storage, sizeof(*control_ty));
  control_ty->ctor = TYPE_FUNCTION;
  control_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &control_ty, sizeof(control_ty), HKEY_STRING, name->strname);
  if (control_decl->type_params) {
    visit_typeParameterList(control_decl->type_params);
  }
  Ast* params = control_decl->params;
  visit_parameterList(params);
  /* list_create(&control_ty->params_ty, storage, sizeof(Type*)); */
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* param = ast;
    /*
    list_append(&control_ty->params_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(param->type)->strname)); */
  }
}

static void
visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  for (Ast* ast = local_decls->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(ast);
  }
}

static void
visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_decl->decl);
  } else if (local_decl->decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(local_decl->decl);
  } else if (local_decl->decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(local_decl->decl);
  } else if (local_decl->decl->kind == AST_instantiation) {
    visit_instantiation(local_decl->decl);
  } else assert(0);
}

/** EXTERN **/

static void
visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->decl);
  } else if (extern_decl->decl->kind == AST_functionPrototype) {
    visit_functionPrototype(extern_decl->decl);
  } else assert(0);
}

static void
visit_externTypeDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externTypeDeclaration);
  Ast* name = extern_decl->name;
  Type_Product* extern_ty = arena_malloc(storage, sizeof(*extern_ty));
  extern_ty->ctor = TYPE_PRODUCT;
  extern_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &extern_ty, sizeof(extern_ty), HKEY_STRING, name->strname);
  if (extern_decl->type_params) {
    visit_typeParameterList(extern_decl->type_params);
  }
  /*
  list_create(&extern_ty->members_ty, storage, sizeof(Type*)); */
  visit_methodPrototypes(extern_decl->method_protos);
}

static void
visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  for (Ast* ast = protos->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(ast);
  }
}

static void
visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  Ast* name = func_proto->name;
  Type_Function* func_ty = arena_malloc(storage, sizeof(*func_ty));
  func_ty->ctor = TYPE_FUNCTION;
  func_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &func_proto, sizeof(func_proto), HKEY_STRING, name->strname);
  if (func_proto->return_type) {
    Ast* type_ref = func_proto->return_type;
    visit_typeRef(type_ref);
    func_ty->return_ty = *(Type**)hashmap_lookup(&type_table, HKEY_STRING,
        name_of_type(type_ref->type)->strname);
  }
  if (func_proto->type_params) {
    visit_typeParameterList(func_proto->type_params);
  }
  Ast* params = func_proto->params;
  visit_parameterList(params);
  /* list_create(&func_ty->params_ty, storage, sizeof(Type*)); */
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* param = ast;
    /*
    list_append(&func_ty->params_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(param->type)->strname)); */
  }
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeString) {
    visit_baseTypeString(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(type_ref->type);
  } else if (type_ref->type->kind == AST_baseTypeError) {
    visit_baseTypeError(type_ref->type);
  } else if (type_ref->type->kind == AST_name) {
    visit_name(type_ref->type);
  } else if (type_ref->type->kind == AST_specializedType) {
    visit_specializedType(type_ref->type);
  } else if (type_ref->type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->type);
  } else if (type_ref->type->kind == AST_tupleType) {
    visit_tupleType(type_ref->type);
  } else assert(0);
}

static void
visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  Ast* name = type_decl->name;
  Type_Product* tuple_ty = arena_malloc(storage, sizeof(*tuple_ty));
  tuple_ty->ctor = TYPE_PRODUCT;
  tuple_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &tuple_ty, sizeof(tuple_ty), HKEY_STRING, name->strname);
  Ast* type_args = type_decl->type_args;
  visit_typeArgumentList(type_args);
  /* list_create(&tuple_ty->members_ty, storage, sizeof(Type*)); */
  for (Ast* ast = type_args->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* type_arg = ast;
    /*
    list_append(&tuple_ty->members_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(type_arg->arg)->strname)); */
  }
}

static void
visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Ast* name = type_decl->name;
  Type_Array* stack_ty = arena_malloc(storage, sizeof(*stack_ty));
  stack_ty->ctor = TYPE_ARRAY;
  stack_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &stack_ty, sizeof(stack_ty), HKEY_STRING, name->strname);
  Ast* type = type_decl->type;
  visit_typeRef(type);
  stack_ty->element_ty = *(Type**)hashmap_lookup(&type_table, HKEY_STRING,
      name_of_type(type)->strname);
  visit_expression(type_decl->stack_expr);
}

static void
visit_specializedType(Ast* type_decl)
{
  assert(type_decl->kind == AST_specializedType);
  Ast* name = type_decl->name;
  Type_Generic* speclzd_ty = arena_malloc(storage, sizeof(*speclzd_ty));
  speclzd_ty->ctor = TYPE_GENERIC;
  speclzd_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &speclzd_ty, sizeof(speclzd_ty), HKEY_STRING, name->strname);
  Ast* type = type_decl->type;
  visit_typeRef(type);
  speclzd_ty->referred_ty = *(Type**)hashmap_lookup(&type_table, HKEY_STRING,
      name_of_type(type)->strname);
  Ast* type_args = type_decl->type_args;
  visit_typeArgumentList(type_args);
  /* list_create(&speclzd_ty->args_ty, storage, sizeof(Type*)); */
  for (Ast* ast = type_args->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* type_arg = ast;
    /*
    list_append(&speclzd_ty->args_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(type_arg->arg)->strname)); */
  }
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  if (int_type->size) {
    visit_integerTypeSize(int_type->size);
  }
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  if (bit_type->size) {
    visit_integerTypeSize(bit_type->size);
  }
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  visit_integerTypeSize(varbit_type->size);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
}

static void
visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void
visit_typeParameterList(Ast* param_list)
{
  assert(param_list->kind == AST_typeParameterList);
  for (Ast* ast = param_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_name(ast);
  }
}

static void
visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->arg);
  } else if (type_arg->arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->arg);
  } else assert(0);
}

static void
visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->arg);
  } else if (type_arg->arg->kind == AST_name) {
    ;
  } else if (type_arg->arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->arg);
  } else assert(0);
}

static void
visit_realTypeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_realTypeArgumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_realTypeArg(ast);
  }
}

static void
visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(ast);
  }
}

static void
visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->decl);
  } else assert(0);
}

static void
visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->decl);
  } else if (type_decl->decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(type_decl->decl);
  } else assert(0);
}

static void
visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* name = header_decl->name;
  Type_Product* header_ty = arena_malloc(storage, sizeof(*header_ty));
  header_ty->ctor = TYPE_PRODUCT;
  header_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &header_ty, sizeof(header_ty), HKEY_STRING, name->strname);
  Ast* fields = header_decl->fields;
  visit_structFieldList(fields);
  /* list_create(&header_ty->members_ty, storage, sizeof(Type*)); */
  for (Ast* ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* field = ast;
    /*
    list_append(&header_ty->members_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(field->type)->strname)); */
  }
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* name = union_decl->name;
  Type_Union* union_ty = arena_malloc(storage, sizeof(*union_ty));
  union_ty->ctor = TYPE_UNION;
  union_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &union_ty, sizeof(union_ty), HKEY_STRING, name->strname);
  Ast* fields = union_decl->fields;
  visit_structFieldList(fields);
  /* list_create(&union_ty->members_ty, storage, sizeof(Type*)); */
  for (Ast* ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* field = ast;
    /*
    list_append(&union_ty->members_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(field->type)->strname)); */
  }
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* name = struct_decl->name;
  Type_Product* struct_ty = arena_malloc(storage, sizeof(*struct_ty));
  struct_ty->ctor = TYPE_PRODUCT;
  struct_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &struct_ty, sizeof(struct_ty), HKEY_STRING, name->strname);
  Ast* fields = struct_decl->fields;
  visit_structFieldList(fields);
  /*
  list_create(&struct_ty->members_ty, storage, sizeof(Type*)); */
  for (Ast* ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* field = ast;
    /*
    list_append(&struct_ty->members_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(field->type)->strname)); */
  }
}

static void
visit_structFieldList(Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  for (Ast* ast = field_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(ast);
  }
}

static void
visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
  visit_typeRef(field->type);
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name = enum_decl->name;
  Type_Basic* enum_ty = arena_malloc(storage, sizeof(*enum_ty));
  enum_ty->ctor = TYPE_INT;
  enum_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &enum_ty, sizeof(enum_ty), HKEY_STRING, name->strname);
  visit_specifiedIdentifierList(enum_decl->fields);
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visit_identifierList(error_decl->fields);
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visit_identifierList(match_decl->fields);
}

static void
visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  for (Ast* ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    ;
  }
}

static void
visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  for (Ast* ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(ast);
  }
}

static void
visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  if (ident->init_expr) {
    visit_expression(ident->init_expr);
  }
}

static void
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->type_ref);
  } else if (typedef_decl->type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->type_ref);
  } else assert(0);
  Ast* name = typedef_decl->name;
  Type_Typedef* typedef_ty = arena_malloc(storage, sizeof(*typedef_ty));
  typedef_ty->ctor = TYPE_TYPEDEF;
  typedef_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &typedef_ty, sizeof(typedef_ty), HKEY_STRING, name->strname);
  typedef_ty->referred_ty = *(Type**)hashmap_lookup(&type_table, HKEY_STRING,
      name_of_type(typedef_decl->type_ref)->strname);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->lhs_expr);
  } else if (assign_stmt->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->rhs_expr);
}

static void
visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  Ast* lhs_expr = func_call->lhs_expr;
  if (lhs_expr->kind == AST_expression) {
    visit_expression(lhs_expr);
  } else if (lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->args);
}

static void
visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->expr) {
    visit_expression(return_stmt->expr);
  }
}

static void
visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void
visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(cond_stmt->cond_expr);
  visit_statement(cond_stmt->stmt);
  if (cond_stmt->else_stmt) {
    visit_statement(cond_stmt->else_stmt);
  }
}

static void
visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->name->kind == AST_name) {
    ;
  } else if (applic_stmt->name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->name);
  } else assert(0);
  visit_argumentList(applic_stmt->args);
}

static void
visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->stmt);
  } else if (stmt->stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->stmt);
  } else if (stmt->stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_emptyStatement) {
  } else if (stmt->stmt->kind == AST_blockStatement) {
    visit_blockStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_exitStatement) {
    visit_exitStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_returnStatement) {
    visit_returnStatement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_switchStatement) {
    visit_switchStatement(stmt->stmt);
  } else assert(0);
}

static void
visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(block_stmt->stmt_list);
}

static void
visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  for (Ast* ast = stmt_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(ast);
  }
}

static void
visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(switch_stmt->expr);
  visit_switchCases(switch_stmt->switch_cases);
}

static void
visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  for (Ast* ast = switch_cases->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(ast);
  }
}

static void
visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(switch_case->label);
  if (switch_case->stmt) {
    visit_blockStatement(switch_case->stmt);
  }
}

static void
visit_switchLabel(Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->label->kind == AST_name) {
    ;
  } else if (label->label->kind == AST_default) {
    visit_default(label->label);
  } else assert(0);
}

static void
visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->stmt);
  } else if (stmt->stmt->kind == AST_statement) {
    visit_statement(stmt->stmt);
  } else if (stmt->stmt->kind == AST_instantiation) {
    visit_instantiation(stmt->stmt);
  } else assert(0);
}

/** TABLES **/

static void
visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  visit_tablePropertyList(table_decl->prop_list);
}

static void
visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  for (Ast* ast = prop_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(ast);
  }
}

static void
visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->prop->kind == AST_keyProperty) {
    visit_keyProperty(table_prop->prop);
  } else if (table_prop->prop->kind == AST_actionsProperty) {
    visit_actionsProperty(table_prop->prop);
  } else if (table_prop->prop->kind == AST_entriesProperty) {
    visit_entriesProperty(table_prop->prop);
  } else if (table_prop->prop->kind == AST_simpleProperty) {
    visit_simpleProperty(table_prop->prop);
  } else assert(0);
}

static void
visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(key_prop->keyelem_list);
}

static void
visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  for (Ast* ast = element_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(ast);
  }
}

static void
visit_keyElement(Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(element->expr);
}

static void
visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(actions_prop->action_list);
}

static void
visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  for (Ast* ast = action_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(ast);
  }
}

static void
visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  if (action_ref->args) {
    visit_argumentList(action_ref->args);
  }
}

static void
visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(entries_prop->entries_list);
}

static void
visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  for (Ast* ast = entries_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(ast);
  }
}

static void
visit_entry(Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(entry->keyset);
  visit_actionRef(entry->action);
}

static void
visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(simple_prop->init_expr);
}

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Ast* name = action_decl->name;
  Type_Function* action_ty = arena_malloc(storage, sizeof(*action_ty));
  action_ty->ctor = TYPE_FUNCTION;
  action_ty->strname = name->strname;
  hashmap_set(&type_table, storage, &action_decl, sizeof(action_decl), HKEY_STRING, name->strname);
  Ast* params = action_decl->params;
  visit_parameterList(params);
  /* list_create(&action_ty->params_ty, storage, sizeof(Type*)); */
  for (Ast* ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    Ast* param = ast;
    /*
    list_append(&action_ty->params_ty, *(Type**)hashmap_lookup(
          &type_table, HKEY_STRING, name_of_type(param->type)->strname)); */
  }
  visit_blockStatement(action_decl->stmt);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef(var_decl->type);
  if (var_decl->init_expr) {
    visit_expression(var_decl->init_expr);
  }
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(func_decl->proto);
  visit_blockStatement(func_decl->stmt);
}

static void
visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  for (Ast* ast = arg_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast);
  }
}

static void
visit_argument(Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->arg->kind == AST_expression) {
    visit_expression(arg->arg);
  } else if (arg->arg->kind == AST_dontcare) {
    visit_dontcare(arg->arg);
  } else assert(0);
}

static void
visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  for (Ast* ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast);
  }
}

static void
visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->expr->kind == AST_name) {
    ;
  } else if (lvalue_expr->expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->expr);
  } else if (lvalue_expr->expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->expr);
  } else assert(0);
}

static void
visit_expression(Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expr->kind == AST_expression) {
    visit_expression(expr->expr);
  } else if (expr->expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expr);
  } else if (expr->expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expr);
  } else if (expr->expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expr);
  } else if (expr->expr->kind == AST_name) {
    ;
  } else if (expr->expr->kind == AST_specializedType) {
    visit_specializedType(expr->expr);
  } else if (expr->expr->kind == AST_headerStackType) {
    visit_headerStackType(expr->expr);
  } else if (expr->expr->kind == AST_expressionList) {
    visit_expressionList(expr->expr);
  } else if (expr->expr->kind == AST_castExpression) {
    visit_castExpression(expr->expr);
  } else if (expr->expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expr);
  } else if (expr->expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expr);
  } else if (expr->expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expr);
  } else if (expr->expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expr);
  } else if (expr->expr->kind == AST_functionCall) {
    visit_functionCall(expr->expr);
  } else if (expr->expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expr);
  } else assert(0);
  if (expr->type_args) {
    visit_realTypeArgumentList(expr->type_args);
  }
}

static void
visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(cast_expr->type);
  visit_expression(cast_expr->expr);
}

static void
visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->operand);
}

static void
visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(binary_expr->left_operand);
  visit_expression(binary_expr->right_operand);
}

static void
visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->lhs_expr->kind == AST_expression) {
    visit_expression(selector->lhs_expr);
  } else if (selector->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->lhs_expr);
  } else assert(0);
}

static void
visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->lhs_expr->kind == AST_expression) {
    visit_expression(subscript->lhs_expr);
  } else if (subscript->lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->index_expr);
}

static void
visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(index_expr->start_index);
  if (index_expr->end_index) {
    visit_expression(index_expr->end_index);
  }
}

static void
visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void
visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void
visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void
visit_default(Ast* _default)
{
  assert(_default->kind == AST_default);
}

static void
visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}

Hashmap*
type_decl(Ast* ast, Arena* _storage)
{
  storage = _storage;
  hashmap_create(&type_table, storage, 15, 1023);

  struct BuiltinType {
    char* strname;
    enum TypeEnum type;
  };
  struct BuiltinType basic_types[] = {
    {"void",   TYPE_VOID},
    {"bool",   TYPE_BOOL},
    {"int",    TYPE_INT},
    {"bit",    TYPE_BIT},
    {"varbit", TYPE_VARBIT},
    {"string", TYPE_STRING},
    {"error",  TYPE_ERROR},
    {"match_kind", TYPE_MATCH_KIND},
  };
  for (int i = 0; i < sizeof(basic_types)/sizeof(basic_types[0]); i++) {
    Type_Basic* basic_ty = arena_malloc(storage, sizeof(*basic_ty));
    basic_ty->ctor = basic_types[i].type;
    basic_ty->strname = basic_types[i].strname;
    hashmap_set(&type_table, storage, &basic_ty, sizeof(basic_ty), HKEY_STRING, basic_ty->strname);
  }

  Type_TypeVar* dontcare_ty = arena_malloc(storage, sizeof(*dontcare_ty));
  dontcare_ty->ctor = TYPE_TYPEVAR;
  dontcare_ty->strname = "_";
  hashmap_set(&type_table, storage, &dontcare_ty, sizeof(dontcare_ty), HKEY_STRING, dontcare_ty->strname);

  visit_p4program(ast);
  return &type_table;
}

