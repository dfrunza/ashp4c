#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena* storage;
static Scope* root_scope, *current_scope;
static Set*   opened_scopes, *enclosing_scopes;
static Set*   decl_map;

/** PROGRAM **/

static void visit_p4program(Ast* p4program);
static void visit_declarationList(Ast* decl_list);
static void visit_declaration(Ast* decl);
static void visit_name(Ast* name);
static void visit_parameterList(Ast* params);
static void visit_parameter(Ast* param);
static void visit_packageTypeDeclaration(Ast* type_decl);
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
static void visit_baseTypeBoolean(Ast* bool_type);
static void visit_baseTypeInteger(Ast* int_type);
static void visit_baseTypeBit(Ast* bit_type);
static void visit_baseTypeVarbit(Ast* varbit_type);
static void visit_baseTypeString(Ast* str_type);
static void visit_baseTypeVoid(Ast* void_type);
static void visit_baseTypeError(Ast* error_type);
static void visit_integerTypeSize(Ast* type_size);
static void visit_realTypeArg(Ast* type_arg);
static void visit_typeArg(Ast* type_arg);
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
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

Set*
build_symtable(Arena* storage_, char* source_file, Ast* p4program, Scope* root_scope_, Set* opened_scopes_,
    Set** decl_map_)
{
  root_scope = root_scope_;
  opened_scopes = opened_scopes_;
  storage = storage_;
  current_scope = root_scope;
  enclosing_scopes = arena_malloc(storage, sizeof(Set));
  *enclosing_scopes = (Set){0};
  decl_map = arena_malloc(storage, sizeof(Set));
  *decl_map = (Set){0};
  *decl_map_ = decl_map;

  visit_p4program(p4program);

  assert(current_scope == root_scope);
  return enclosing_scopes;
}

/** PROGRAM **/

static void
visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, p4program, 0, 0);

  visit_declarationList(p4program->p4program.decl_list);

  current_scope = prev_scope;
}

static void
visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  Ast* ast;

  for (ast = decl_list->declarationList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(ast);
  }
}

static void
visit_declaration(Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(decl->declaration.decl);
  } else assert(0);
}

static void
visit_name(Ast* name)
{
  SetMember* m;
  assert(name->kind == AST_name);
  m = set_add(enclosing_scopes, storage, name, current_scope, 0);
  assert(m);
}

static void
visit_parameterList(Ast* params)
{
  assert(params->kind == AST_parameterList);
  Ast* ast;

  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(ast);
  }
}

static void
visit_parameter(Ast* param)
{
  assert(param->kind == AST_parameter);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(param->parameter.type);
  visit_name(param->parameter.name);

  name = param->parameter.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = param;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, param, name_decl, 0);

  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->packageTypeDeclaration.name);
  name = type_decl->packageTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = type_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, type_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, type_decl, 0, 0);

  visit_parameterList(type_decl->packageTypeDeclaration.params);

  current_scope = prev_scope;
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);

  name = inst->instantiation.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = inst;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, inst, name_decl, 0);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(parser_decl->parserDeclaration.proto);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, parser_decl, 0, 0);

  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);

  current_scope = prev_scope;
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->parserTypeDeclaration.name);
  name = type_decl->parserTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = type_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, type_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, type_decl, 0, 0);

  visit_parameterList(type_decl->parserTypeDeclaration.params);

  current_scope = prev_scope;
}

static void
visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  Ast* ast;

  for (ast = local_elements->parserLocalElements.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(ast);
  }
}

static void
visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

static void
visit_parserStates(Ast* states)
{
  assert(states->kind == AST_parserStates);
  Ast* ast;

  for (ast = states->parserStates.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(ast);
  }
}

static void
visit_parserState(Ast* state)
{
  assert(state->kind == AST_parserState);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = state->parserState.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = state;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, state, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, state, 0, 0);

  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);

  current_scope = prev_scope;
}

static void
visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  Ast* ast;

  for (ast = stmts->parserStatements.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(ast);
  }
}

static void
visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);

  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else assert(0);
}

static void
visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, block_stmt, 0, 0);

  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);

  current_scope = prev_scope;
}

static void
visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

static void
visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

static void
visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  Ast* ast;

  for (ast = case_list->selectCaseList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(ast);
  }
}

static void
visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name);
}

static void
visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void
visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
}

static void
visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void
visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  Ast* ast;

  for (ast = expr_list->simpleExpressionList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(ast);
  }
}

/** CONTROL **/

static void
visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  Scope* prev_scope;

  visit_typeDeclaration(control_decl->controlDeclaration.proto);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, control_decl, 0, 0);

  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);

  current_scope = prev_scope;
}

static void
visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(type_decl->controlTypeDeclaration.name);
  name = type_decl->controlTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = type_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, type_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, type_decl, 0, 0);

  visit_parameterList(type_decl->controlTypeDeclaration.params);

  current_scope = prev_scope;
}

static void
visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  Ast* ast;

  for (ast = local_decls->controlLocalDeclarations.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(ast);
  }
}

static void
visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void
visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

static void
visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = type_decl->externTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = type_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, type_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, type_decl, 0, 0);

  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);

  current_scope = prev_scope;
}

static void
visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  Ast* ast;

  for (ast = protos->methodPrototypes.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(ast);
  }
}

static void
visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  visit_name(func_proto->functionPrototype.name);
  name = func_proto->functionPrototype.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = func_proto;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, func_proto, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, func_proto, 0, 0);

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }

  visit_parameterList(func_proto->functionPrototype.params);

  current_scope = prev_scope;
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
}

static void
visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visit_typeArgumentList(type_decl->tupleType.type_args);
}

static void
visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "bool", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, bool_type, name_decl, 0);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }

  name_entry = scope_lookup(root_scope, "int", NAMESPACE_TYPE); 
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, int_type, name_decl, 0);
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }

  name_entry = scope_lookup(root_scope, "bit", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, bit_type, name_decl, 0);
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);

  name_entry = scope_lookup(root_scope, "varbit", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, varbit_type, name_decl, 0);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "string", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, str_type, name_decl, 0);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "void", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, void_type, name_decl, 0);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "error", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, error_type, name_decl, 0);
}

static void
visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void
visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

static void
visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

static void
visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  Ast* ast;

  for (ast = arg_list->typeArgumentList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(ast);
  }
}

static void
visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
}

static void
visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

static void
visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = header_decl->headerTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = header_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, header_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, header_decl, 0, 0);

  visit_structFieldList(header_decl->headerTypeDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = union_decl->headerUnionDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = union_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, union_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, union_decl, 0, 0);

  visit_structFieldList(union_decl->headerUnionDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = struct_decl->structTypeDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = struct_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, struct_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, struct_decl, 0, 0);

  visit_structFieldList(struct_decl->structTypeDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_structFieldList(Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  Ast* ast;

  for (ast = field_list->structFieldList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(ast);
  }
}

static void
visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(field->structField.type);
  
  name = field->structField.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = field;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, field, name_decl, 0);
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = enum_decl->enumDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = enum_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, enum_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, enum_decl, 0, 0);

  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, error_decl, 0, 0);

  visit_identifierList(error_decl->errorDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Scope* prev_scope;

  prev_scope = current_scope;
  current_scope = root_scope;

  visit_identifierList(match_decl->matchKindDeclaration.fields);

  current_scope = prev_scope;
}

static void
visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  Ast* ast, *name;
  NameDeclaration* name_decl;

  for (ast = ident_list->identifierList.first_child;
       ast != 0; ast = ast->right_sibling) {
    name = ast;
    name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
    name_decl->ast = name;
    set_add(decl_map, storage, name, name_decl, 0);
  }
}

static void
visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  Ast* ast;

  for (ast = ident_list->specifiedIdentifierList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(ast);
  }
}

static void
visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  Ast* name;
  NameDeclaration* name_decl;

  name = ident->specifiedIdentifier.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = ident;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, ident, name_decl, 0);

  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr);
  }
}

static void
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);

  name = typedef_decl->typedefDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = typedef_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, typedef_decl, name_decl, 0);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr);
}

static void
visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args);
}

static void
visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr);
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
  visit_expression(cond_stmt->conditionalStatement.cond_expr);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

static void
visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

static void
visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  Scope* prev_scope;

  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    prev_scope = current_scope;
    current_scope = set_lookup(opened_scopes, stmt, 0, 0);

    visit_blockStatement(stmt->statement.stmt);

    current_scope = prev_scope;
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

static void
visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

static void
visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  Ast* ast;

  for (ast = stmt_list->statementOrDeclList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(ast);
  }
}

static void
visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(switch_stmt->switchStatement.expr);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

static void
visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  Ast* ast;

  for (ast = switch_cases->switchCases.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(ast);
  }
}

static void
visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

static void
visit_switchLabel(Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    visit_name(label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

static void
visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void
visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = table_decl->tableDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = table_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, table_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, table_decl, 0, 0);

  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);

  current_scope = prev_scope;
}

static void
visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  Ast* ast;

  for (ast = prop_list->tablePropertyList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(ast);
  }
}

static void
visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(table_prop->tableProperty.prop);
  } else assert(0);
}

static void
visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

static void
visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  Ast* ast;

  for (ast = element_list->keyElementList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(ast);
  }
}

static void
visit_keyElement(Ast* element)
{
  assert(element->kind == AST_keyElement);
  Scope* prev_scope;

  visit_expression(element->keyElement.expr);

  prev_scope = current_scope;
  current_scope = root_scope;

  visit_name(element->keyElement.match);

  current_scope = prev_scope;
}

static void
visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

static void
visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  Ast* ast;

  for (ast = action_list->actionList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(ast);
  }
}

static void
visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

static void
visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(entries_prop->entriesProperty.entries_list);
}

static void
visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  Ast* ast;

  for (ast = entries_list->entriesList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(ast);
  }
}

static void
visit_entry(Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(entry->entry.keyset);
  visit_actionRef(entry->entry.action);
}

static void
visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  Ast* name;
  NameDeclaration* name_decl;

  name = simple_prop->simpleProperty.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = simple_prop;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, simple_prop, name_decl, 0);

  visit_expression(simple_prop->simpleProperty.init_expr);
}

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Ast* name;
  NameDeclaration* name_decl;
  Scope* prev_scope;

  name = action_decl->actionDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
  name_decl->ast = action_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, action_decl, name_decl, 0);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, action_decl, 0, 0);

  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);

  current_scope = prev_scope;
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  Ast* name;
  NameDeclaration* name_decl;

  visit_typeRef(var_decl->variableDeclaration.type);
  visit_name(var_decl->variableDeclaration.name);

  name = var_decl->variableDeclaration.name;
  name_decl = scope_bind(current_scope, storage, name->name.strname, NAMESPACE_VAR);
  name_decl->ast = var_decl;
  set_add(decl_map, storage, name, name_decl, 0);
  set_add(decl_map, storage, var_decl, name_decl, 0);

  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  Scope* prev_scope;

  visit_functionPrototype(func_decl->functionDeclaration.proto);

  prev_scope = current_scope;
  current_scope = set_lookup(opened_scopes, func_decl, 0, 0);

  visit_blockStatement(func_decl->functionDeclaration.stmt);

  current_scope = prev_scope;
}

static void
visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  Ast* ast;

  for (ast = arg_list->argumentList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast);
  }
}

static void
visit_argument(Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
}

static void
visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;

  for (ast = expr_list->expressionList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast);
  }
}

static void
visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void
visit_expression(Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
}

static void
visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr);
}

static void
visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand);
}

static void
visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand);
  visit_expression(binary_expr->binaryExpression.right_operand);
}

static void
visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
  visit_name(selector->memberSelector.name);
}

static void
visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
}

static void
visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index);
  }
}

static void
visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "bool", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, bool_literal, name_decl, 0);
}

static void
visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  if (int_literal->integerLiteral.is_signed) {
    name_entry = scope_lookup(root_scope, "int", NAMESPACE_TYPE);
  } else {
    name_entry = scope_lookup(root_scope, "bit", NAMESPACE_TYPE);
  }
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, int_literal, name_decl, 0);
}

static void
visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "string", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, str_literal, name_decl, 0);
}

static void
visit_default(Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void
visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  name_entry = scope_lookup(root_scope, "_", NAMESPACE_TYPE);
  name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
  set_add(decl_map, storage, dontcare, name_decl, 0);
}
