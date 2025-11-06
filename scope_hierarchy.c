#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

/** PROGRAM **/

static void visit_p4program(ScopeBuilder* scope_builder, Ast* p4program);
static void visit_declarationList(ScopeBuilder* scope_builder, Ast* decl_list);
static void visit_declaration(ScopeBuilder* scope_builder, Ast* decl);
static void visit_name(ScopeBuilder* scope_builder, Ast* name);
static void visit_parameterList(ScopeBuilder* scope_builder, Ast* params);
static void visit_parameter(ScopeBuilder* scope_builder, Ast* param);
static void visit_packageTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_instantiation(ScopeBuilder* scope_builder, Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(ScopeBuilder* scope_builder, Ast* parser_decl);
static void visit_parserTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_parserLocalElements(ScopeBuilder* scope_builder, Ast* local_elements);
static void visit_parserLocalElement(ScopeBuilder* scope_builder, Ast* local_element);
static void visit_parserStates(ScopeBuilder* scope_builder, Ast* states);
static void visit_parserState(ScopeBuilder* scope_builder, Ast* state);
static void visit_parserStatements(ScopeBuilder* scope_builder, Ast* stmts);
static void visit_parserStatement(ScopeBuilder* scope_builder, Ast* stmt);
static void visit_parserBlockStatement(ScopeBuilder* scope_builder, Ast* block_stmt);
static void visit_transitionStatement(ScopeBuilder* scope_builder, Ast* transition_stmt);
static void visit_stateExpression(ScopeBuilder* scope_builder, Ast* state_expr);
static void visit_selectExpression(ScopeBuilder* scope_builder, Ast* select_expr);
static void visit_selectCaseList(ScopeBuilder* scope_builder, Ast* case_list);
static void visit_selectCase(ScopeBuilder* scope_builder, Ast* select_case);
static void visit_keysetExpression(ScopeBuilder* scope_builder, Ast* keyset_expr);
static void visit_tupleKeysetExpression(ScopeBuilder* scope_builder, Ast* tuple_expr);
static void visit_simpleKeysetExpression(ScopeBuilder* scope_builder, Ast* simple_expr);
static void visit_simpleExpressionList(ScopeBuilder* scope_builder, Ast* expr_list);

/** CONTROL **/

static void visit_controlDeclaration(ScopeBuilder* scope_builder, Ast* control_decl);
static void visit_controlTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_controlLocalDeclarations(ScopeBuilder* scope_builder, Ast* local_decls);
static void visit_controlLocalDeclaration(ScopeBuilder* scope_builder, Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(ScopeBuilder* scope_builder, Ast* extern_decl);
static void visit_externTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_methodPrototypes(ScopeBuilder* scope_builder, Ast* protos);
static void visit_functionPrototype(ScopeBuilder* scope_builder, Ast* func_proto);

/** TYPES **/

static void visit_typeRef(ScopeBuilder* scope_builder, Ast* type_ref);
static void visit_tupleType(ScopeBuilder* scope_builder, Ast* type);
static void visit_headerStackType(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_baseTypeBoolean(ScopeBuilder* scope_builder, Ast* bool_type);
static void visit_baseTypeInteger(ScopeBuilder* scope_builder, Ast* int_type);
static void visit_baseTypeBit(ScopeBuilder* scope_builder, Ast* bit_type);
static void visit_baseTypeVarbit(ScopeBuilder* scope_builder, Ast* varbit_type);
static void visit_baseTypeString(ScopeBuilder* scope_builder, Ast* str_type);
static void visit_baseTypeVoid(ScopeBuilder* scope_builder, Ast* void_type);
static void visit_baseTypeError(ScopeBuilder* scope_builder, Ast* error_type);
static void visit_integerTypeSize(ScopeBuilder* scope_builder, Ast* type_size);
static void visit_realTypeArg(ScopeBuilder* scope_builder, Ast* type_arg);
static void visit_typeArg(ScopeBuilder* scope_builder, Ast* type_arg);
static void visit_typeArgumentList(ScopeBuilder* scope_builder, Ast* arg_list);
static void visit_typeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_derivedTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl);
static void visit_headerTypeDeclaration(ScopeBuilder* scope_builder, Ast* header_decl);
static void visit_headerUnionDeclaration(ScopeBuilder* scope_builder, Ast* union_decl);
static void visit_structTypeDeclaration(ScopeBuilder* scope_builder, Ast* struct_decl);
static void visit_structFieldList(ScopeBuilder* scope_builder, Ast* field_list);
static void visit_structField(ScopeBuilder* scope_builder, Ast* field);
static void visit_enumDeclaration(ScopeBuilder* scope_builder, Ast* enum_decl);
static void visit_errorDeclaration(ScopeBuilder* scope_builder, Ast* error_decl);
static void visit_matchKindDeclaration(ScopeBuilder* scope_builder, Ast* match_decl);
static void visit_identifierList(ScopeBuilder* scope_builder, Ast* ident_list);
static void visit_specifiedIdentifierList(ScopeBuilder* scope_builder, Ast* ident_list);
static void visit_specifiedIdentifier(ScopeBuilder* scope_builder, Ast* ident);
static void visit_typedefDeclaration(ScopeBuilder* scope_builder, Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(ScopeBuilder* scope_builder, Ast* assign_stmt);
static void visit_functionCall(ScopeBuilder* scope_builder, Ast* func_call);
static void visit_returnStatement(ScopeBuilder* scope_builder, Ast* return_stmt);
static void visit_exitStatement(ScopeBuilder* scope_builder, Ast* exit_stmt);
static void visit_conditionalStatement(ScopeBuilder* scope_builder, Ast* cond_stmt);
static void visit_directApplication(ScopeBuilder* scope_builder, Ast* applic_stmt);
static void visit_statement(ScopeBuilder* scope_builder, Ast* stmt);
static void visit_blockStatement(ScopeBuilder* scope_builder, Ast* block_stmt);
static void visit_statementOrDeclList(ScopeBuilder* scope_builder, Ast* stmt_list);
static void visit_switchStatement(ScopeBuilder* scope_builder, Ast* switch_stmt);
static void visit_switchCases(ScopeBuilder* scope_builder, Ast* switch_cases);
static void visit_switchCase(ScopeBuilder* scope_builder, Ast* switch_case);
static void visit_switchLabel(ScopeBuilder* scope_builder, Ast* label);
static void visit_statementOrDeclaration(ScopeBuilder* scope_builder, Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(ScopeBuilder* scope_builder, Ast* table_decl);
static void visit_tablePropertyList(ScopeBuilder* scope_builder, Ast* prop_list);
static void visit_tableProperty(ScopeBuilder* scope_builder, Ast* table_prop);
static void visit_keyProperty(ScopeBuilder* scope_builder, Ast* key_prop);
static void visit_keyElementList(ScopeBuilder* scope_builder, Ast* element_list);
static void visit_keyElement(ScopeBuilder* scope_builder, Ast* element);
static void visit_actionsProperty(ScopeBuilder* scope_builder, Ast* actions_prop);
static void visit_actionList(ScopeBuilder* scope_builder, Ast* action_list);
static void visit_actionRef(ScopeBuilder* scope_builder, Ast* action_ref);
static void visit_entriesProperty(ScopeBuilder* scope_builder, Ast* entries_prop);
static void visit_entriesList(ScopeBuilder* scope_builder, Ast* entries_list);
static void visit_entry(ScopeBuilder* scope_builder, Ast* entry);
static void visit_simpleProperty(ScopeBuilder* scope_builder, Ast* simple_prop);
static void visit_actionDeclaration(ScopeBuilder* scope_builder, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(ScopeBuilder* scope_builder, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(ScopeBuilder* scope_builder, Ast* func_decl);
static void visit_argumentList(ScopeBuilder* scope_builder, Ast* arg_list);
static void visit_argument(ScopeBuilder* scope_builder, Ast* arg);
static void visit_expressionList(ScopeBuilder* scope_builder, Ast* expr_list);
static void visit_lvalueExpression(ScopeBuilder* scope_builder, Ast* lvalue_expr);
static void visit_expression(ScopeBuilder* scope_builder, Ast* expr);
static void visit_castExpression(ScopeBuilder* scope_builder, Ast* cast_expr);
static void visit_unaryExpression(ScopeBuilder* scope_builder, Ast* unary_expr);
static void visit_binaryExpression(ScopeBuilder* scope_builder, Ast* binary_expr);
static void visit_memberSelector(ScopeBuilder* scope_builder, Ast* selector);
static void visit_arraySubscript(ScopeBuilder* scope_builder, Ast* subscript);
static void visit_indexExpression(ScopeBuilder* scope_builder, Ast* index_expr);
static void visit_booleanLiteral(ScopeBuilder* scope_builder, Ast* bool_literal);
static void visit_integerLiteral(ScopeBuilder* scope_builder, Ast* int_literal);
static void visit_stringLiteral(ScopeBuilder* scope_builder, Ast* str_literal);
static void visit_default(ScopeBuilder* scope_builder, Ast* default_);
static void visit_dontcare(ScopeBuilder* scope_builder, Ast* dontcare);

void scope_hierarchy(ScopeBuilder* scope_builder)
{
  scope_builder->current_scope = scope_builder->root_scope;
  scope_builder->scope_map = (Map*)arena_malloc(scope_builder->storage, sizeof(Map));
  scope_builder->scope_map->storage = scope_builder->storage;
  visit_p4program(scope_builder, scope_builder->p4program);
  assert(scope_builder->current_scope == scope_builder->root_scope);
}

/** PROGRAM **/

static void visit_p4program(ScopeBuilder* scope_builder, Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, p4program, scope_builder->current_scope, 0);
  assert(m);
  visit_declarationList(scope_builder, p4program->p4program.decl_list);
  scope_builder->current_scope = prev_scope;
}

static void visit_declarationList(ScopeBuilder* scope_builder, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(ScopeBuilder* scope_builder, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  Scope* scope;
  MapEntry* m;

  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(scope_builder, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(scope_builder, decl->declaration.decl);
  } else assert(0);
  scope = (Scope*)(Scope*)map_lookup(scope_builder->scope_map, decl->declaration.decl, 0);
  m = map_insert(scope_builder->scope_map, decl, scope, 0);
  assert(m);
}

static void visit_name(ScopeBuilder* scope_builder, Ast* name)
{
  assert(name->kind == AST_name);
}

static void visit_parameterList(ScopeBuilder* scope_builder, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_parameter(ScopeBuilder* scope_builder, Ast* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef(scope_builder, param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(scope_builder, param->parameter.init_expr);
  }
}

static void visit_packageTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, type_decl, scope_builder->current_scope, 0);
  assert(m);
  visit_parameterList(scope_builder, type_decl->packageTypeDeclaration.params);
  scope_builder->current_scope = prev_scope;
}

static void visit_instantiation(ScopeBuilder* scope_builder, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef(scope_builder, inst->instantiation.type);
  visit_argumentList(scope_builder, inst->instantiation.args);
}

/** PARSER **/

static void visit_parserDeclaration(ScopeBuilder* scope_builder, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Scope* prev_scope;
  MapEntry* m;

  visit_typeDeclaration(scope_builder, parser_decl->parserDeclaration.proto);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = (Scope*)map_lookup(scope_builder->scope_map, parser_decl->parserDeclaration.proto, 0);
  m = map_insert(scope_builder->scope_map, parser_decl, scope_builder->current_scope, 0);
  assert(m);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(scope_builder, parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(scope_builder, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(scope_builder, parser_decl->parserDeclaration.states);
  scope_builder->current_scope = prev_scope;
}

static void visit_parserTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, type_decl, scope_builder->current_scope, 0);
  assert(m);
  visit_parameterList(scope_builder, type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(scope_builder, type_decl->parserTypeDeclaration.method_protos);
  scope_builder->current_scope = prev_scope;
}

static void visit_parserLocalElements(ScopeBuilder* scope_builder, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(ScopeBuilder* scope_builder, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(scope_builder, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(scope_builder, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(ScopeBuilder* scope_builder, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(ScopeBuilder* scope_builder, Ast* state)
{
  assert(state->kind == AST_parserState);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, state, scope_builder->current_scope, 0);
  assert(m);
  visit_parserStatements(scope_builder, state->parserState.stmt_list);
  visit_transitionStatement(scope_builder, state->parserState.transition_stmt);
  scope_builder->current_scope = prev_scope;
}

static void visit_parserStatements(ScopeBuilder* scope_builder, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(ScopeBuilder* scope_builder, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);

  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(scope_builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(scope_builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(scope_builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(scope_builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(scope_builder, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(ScopeBuilder* scope_builder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, block_stmt, scope_builder->current_scope, 0);
  assert(m);
  visit_parserStatements(scope_builder, block_stmt->parserBlockStatement.stmt_list);
  scope_builder->current_scope = prev_scope;
}

static void visit_transitionStatement(ScopeBuilder* scope_builder, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(scope_builder, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(ScopeBuilder* scope_builder, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    ;
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(scope_builder, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(ScopeBuilder* scope_builder, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(scope_builder, select_expr->selectExpression.expr_list);
  visit_selectCaseList(scope_builder, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(ScopeBuilder* scope_builder, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_selectCase(ScopeBuilder* scope_builder, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(scope_builder, select_case->selectCase.keyset_expr);
}

static void visit_keysetExpression(ScopeBuilder* scope_builder, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(scope_builder, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(scope_builder, keyset_expr->keysetExpression.expr);
  } else assert(0);
}

static void visit_tupleKeysetExpression(ScopeBuilder* scope_builder, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visit_simpleExpressionList(scope_builder, tuple_expr->tupleKeysetExpression.expr_list);
}

static void visit_simpleKeysetExpression(ScopeBuilder* scope_builder, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(scope_builder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(scope_builder, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(scope_builder, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

static void visit_simpleExpressionList(ScopeBuilder* scope_builder, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(scope_builder, container_of(ast, Ast, tree));
  }
}

/** CONTROL **/

static void visit_controlDeclaration(ScopeBuilder* scope_builder, Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  Scope* prev_scope;
  MapEntry* m;

  visit_typeDeclaration(scope_builder, control_decl->controlDeclaration.proto);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = (Scope*)map_lookup(scope_builder->scope_map, control_decl->controlDeclaration.proto, 0);
  m = map_insert(scope_builder->scope_map, control_decl, scope_builder->current_scope, 0);
  assert(m);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(scope_builder, control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(scope_builder, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(scope_builder, control_decl->controlDeclaration.apply_stmt);
  scope_builder->current_scope = prev_scope;
}

static void visit_controlTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, type_decl, scope, 0);
  assert(m);
  visit_parameterList(scope_builder, type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(scope_builder, type_decl->controlTypeDeclaration.method_protos);
  scope_builder->current_scope = prev_scope;
}

static void visit_controlLocalDeclarations(ScopeBuilder* scope_builder, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(ScopeBuilder* scope_builder, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(scope_builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(scope_builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(scope_builder, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(scope_builder, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(ScopeBuilder* scope_builder, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  Scope* scope;
  MapEntry* m;

  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(scope_builder, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(scope_builder, extern_decl->externDeclaration.decl);
  } else assert(0);
  scope = (Scope*)map_lookup(scope_builder->scope_map, extern_decl->externDeclaration.decl, 0);
  m = map_insert(scope_builder->scope_map, extern_decl, scope, 0);
  assert(m);
}

static void visit_externTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, type_decl, scope_builder->current_scope, 0);
  assert(m);
  visit_methodPrototypes(scope_builder, type_decl->externTypeDeclaration.method_protos);
  scope_builder->current_scope = prev_scope;
}

static void visit_methodPrototypes(ScopeBuilder* scope_builder, Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_functionPrototype(ScopeBuilder* scope_builder, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  Scope* scope, *prev_scope;
  MapEntry* m;

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(scope_builder, func_proto->functionPrototype.return_type);
  }
  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, func_proto, scope_builder->current_scope, 0);
  assert(m);
  visit_parameterList(scope_builder, func_proto->functionPrototype.params);
  scope_builder->current_scope = prev_scope;
}

/** TYPES **/

static void visit_typeRef(ScopeBuilder* scope_builder, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    ;
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(scope_builder, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(scope_builder, type_ref->typeRef.type);
  } else assert(0);
}

static void visit_tupleType(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visit_typeArgumentList(scope_builder, type_decl->tupleType.type_args);
}

static void visit_headerStackType(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  visit_typeRef(scope_builder, type_decl->headerStackType.type);
  visit_expression(scope_builder, type_decl->headerStackType.stack_expr);
}

static void visit_baseTypeBoolean(ScopeBuilder* scope_builder, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
}

static void visit_baseTypeInteger(ScopeBuilder* scope_builder, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(scope_builder, int_type->baseTypeInteger.size);
  }
}

static void visit_baseTypeBit(ScopeBuilder* scope_builder, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(scope_builder, bit_type->baseTypeBit.size);
  }
}

static void visit_baseTypeVarbit(ScopeBuilder* scope_builder, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  visit_integerTypeSize(scope_builder, varbit_type->baseTypeVarbit.size);
}

static void visit_baseTypeString(ScopeBuilder* scope_builder, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
}

static void visit_baseTypeVoid(ScopeBuilder* scope_builder, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
}

static void visit_baseTypeError(ScopeBuilder* scope_builder, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
}

static void visit_integerTypeSize(ScopeBuilder* scope_builder, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void visit_realTypeArg(ScopeBuilder* scope_builder, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(scope_builder, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(scope_builder, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(ScopeBuilder* scope_builder, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(scope_builder, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    ;
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(scope_builder, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(ScopeBuilder* scope_builder, Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_typeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  Scope* scope;
  MapEntry* m;

  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(scope_builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(scope_builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(scope_builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(scope_builder, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(scope_builder, type_decl->typeDeclaration.decl);
  } else assert(0);
  scope = (Scope*)map_lookup(scope_builder->scope_map, type_decl->typeDeclaration.decl, 0);
  m = map_insert(scope_builder->scope_map, type_decl, scope, 0);
  assert(m);
}

static void visit_derivedTypeDeclaration(ScopeBuilder* scope_builder, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  Scope* scope;
  MapEntry* m;

  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(scope_builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(scope_builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(scope_builder, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(scope_builder, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  scope = (Scope*)map_lookup(scope_builder->scope_map, type_decl->derivedTypeDeclaration.decl, 0);
  m = map_insert(scope_builder->scope_map, type_decl, scope, 0);
  assert(m);
}

static void visit_headerTypeDeclaration(ScopeBuilder* scope_builder, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, header_decl, scope, 0);
  assert(m);
  visit_structFieldList(scope_builder, header_decl->headerTypeDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_headerUnionDeclaration(ScopeBuilder* scope_builder, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, union_decl, scope, 0);
  assert(m);
  visit_structFieldList(scope_builder, union_decl->headerUnionDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_structTypeDeclaration(ScopeBuilder* scope_builder, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, struct_decl, scope, 0);
  assert(m);
  visit_structFieldList(scope_builder, struct_decl->structTypeDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_structFieldList(ScopeBuilder* scope_builder, Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_structField(ScopeBuilder* scope_builder, Ast* field)
{
  assert(field->kind == AST_structField);
  visit_typeRef(scope_builder, field->structField.type);
}

static void visit_enumDeclaration(ScopeBuilder* scope_builder, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, enum_decl, scope, 0);
  assert(m);
  visit_specifiedIdentifierList(scope_builder, enum_decl->enumDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_errorDeclaration(ScopeBuilder* scope_builder, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, error_decl, scope, 0);
  assert(m);
  visit_identifierList(scope_builder, error_decl->errorDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_matchKindDeclaration(ScopeBuilder* scope_builder, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, match_decl, scope, 0);
  assert(m);
  visit_identifierList(scope_builder, match_decl->matchKindDeclaration.fields);
  scope_builder->current_scope = prev_scope;
}

static void visit_identifierList(ScopeBuilder* scope_builder, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    ;
  }
}

static void visit_specifiedIdentifierList(ScopeBuilder* scope_builder, Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifier(ScopeBuilder* scope_builder, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(scope_builder, ident->specifiedIdentifier.init_expr);
  }
}

static void visit_typedefDeclaration(ScopeBuilder* scope_builder, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(scope_builder, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(scope_builder, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(ScopeBuilder* scope_builder, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(scope_builder, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(scope_builder, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(scope_builder, assign_stmt->assignmentStatement.rhs_expr);
}

static void visit_functionCall(ScopeBuilder* scope_builder, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(scope_builder, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(scope_builder, func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(scope_builder, func_call->functionCall.args);
}

static void visit_returnStatement(ScopeBuilder* scope_builder, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(scope_builder, return_stmt->returnStatement.expr);
  }
}

static void visit_exitStatement(ScopeBuilder* scope_builder, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void visit_conditionalStatement(ScopeBuilder* scope_builder, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(scope_builder, cond_stmt->conditionalStatement.cond_expr);
  visit_statement(scope_builder, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(scope_builder, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(ScopeBuilder* scope_builder, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    ;
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(scope_builder, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(scope_builder, applic_stmt->directApplication.args);
}

static void visit_statement(ScopeBuilder* scope_builder, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  Scope* scope, *prev_scope;
  MapEntry* m;

  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    scope = scope_create(scope_builder->storage, 3);
    prev_scope = scope_builder->current_scope;
    scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
    m = map_insert(scope_builder->scope_map, stmt, scope_builder->current_scope, 0);
    assert(m);
    visit_blockStatement(scope_builder, stmt->statement.stmt);
    scope_builder->current_scope = prev_scope;
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(scope_builder, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(scope_builder, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(ScopeBuilder* scope_builder, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(scope_builder, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(ScopeBuilder* scope_builder, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(ScopeBuilder* scope_builder, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(scope_builder, switch_stmt->switchStatement.expr);
  visit_switchCases(scope_builder, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(ScopeBuilder* scope_builder, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(ScopeBuilder* scope_builder, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(scope_builder, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(scope_builder, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(ScopeBuilder* scope_builder, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    ;
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(scope_builder, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(ScopeBuilder* scope_builder, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(scope_builder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(scope_builder, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(scope_builder, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(ScopeBuilder* scope_builder, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 3);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, table_decl, scope, 0);
  assert(m);
  visit_tablePropertyList(scope_builder, table_decl->tableDeclaration.prop_list);
  visit_methodPrototypes(scope_builder, table_decl->tableDeclaration.method_protos);
  scope_builder->current_scope = prev_scope;
}

static void visit_tablePropertyList(ScopeBuilder* scope_builder, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(ScopeBuilder* scope_builder, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(scope_builder, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(scope_builder, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

static void visit_keyProperty(ScopeBuilder* scope_builder, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(scope_builder, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(ScopeBuilder* scope_builder, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(ScopeBuilder* scope_builder, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(scope_builder, element->keyElement.expr);
}

static void visit_actionsProperty(ScopeBuilder* scope_builder, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(scope_builder, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(ScopeBuilder* scope_builder, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(ScopeBuilder* scope_builder, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  if (action_ref->actionRef.args) {
    visit_argumentList(scope_builder, action_ref->actionRef.args);
  }
}

#if 0
static void visit_entriesProperty(ScopeBuilder* scope_builder, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(scope_builder, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(ScopeBuilder* scope_builder, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_entry(ScopeBuilder* scope_builder, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(scope_builder, entry->entry.keyset);
  visit_actionRef(scope_builder, entry->entry.action);
}

static void visit_simpleProperty(ScopeBuilder* scope_builder, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(scope_builder, simple_prop->simpleProperty.init_expr);
}
#endif

static void visit_actionDeclaration(ScopeBuilder* scope_builder, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Scope* scope, *prev_scope;
  MapEntry* m;

  scope = scope_create(scope_builder->storage, 2);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = scope_push(scope, scope_builder->current_scope);
  m = map_insert(scope_builder->scope_map, action_decl, scope_builder->current_scope, 0);
  assert(m);
  visit_parameterList(scope_builder, action_decl->actionDeclaration.params);
  visit_blockStatement(scope_builder, action_decl->actionDeclaration.stmt);
  scope_builder->current_scope = prev_scope;
}

/** VARIABLES **/

static void visit_variableDeclaration(ScopeBuilder* scope_builder, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef(scope_builder, var_decl->variableDeclaration.type);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(scope_builder, var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(ScopeBuilder* scope_builder, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  Scope* prev_scope;
  MapEntry* m;

  visit_functionPrototype(scope_builder, func_decl->functionDeclaration.proto);
  prev_scope = scope_builder->current_scope;
  scope_builder->current_scope = (Scope*)map_lookup(scope_builder->scope_map, func_decl->functionDeclaration.proto, 0);
  m = map_insert(scope_builder->scope_map, func_decl, scope_builder->current_scope, 0);
  assert(m);
  visit_blockStatement(scope_builder, func_decl->functionDeclaration.stmt);
  scope_builder->current_scope = prev_scope;
}

static void visit_argumentList(ScopeBuilder* scope_builder, Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_argument(ScopeBuilder* scope_builder, Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(scope_builder, arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(scope_builder, arg->argument.arg);
  } else assert(0);
}

static void visit_expressionList(ScopeBuilder* scope_builder, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(scope_builder, container_of(ast, Ast, tree));
  }
}

static void visit_lvalueExpression(ScopeBuilder* scope_builder, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    ;
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(scope_builder, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(scope_builder, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

static void visit_expression(ScopeBuilder* scope_builder, Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    ;
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(scope_builder, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(scope_builder, expr->expression.expr);
  } else assert(0);
}

static void visit_castExpression(ScopeBuilder* scope_builder, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(scope_builder, cast_expr->castExpression.type);
  visit_expression(scope_builder, cast_expr->castExpression.expr);
}

static void visit_unaryExpression(ScopeBuilder* scope_builder, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(scope_builder, unary_expr->unaryExpression.operand);
}

static void visit_binaryExpression(ScopeBuilder* scope_builder, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(scope_builder, binary_expr->binaryExpression.left_operand);
  visit_expression(scope_builder, binary_expr->binaryExpression.right_operand);
}

static void visit_memberSelector(ScopeBuilder* scope_builder, Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(scope_builder, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(scope_builder, selector->memberSelector.lhs_expr);
  } else assert(0);
}

static void visit_arraySubscript(ScopeBuilder* scope_builder, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(scope_builder, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(scope_builder, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(scope_builder, subscript->arraySubscript.index_expr);
}

static void visit_indexExpression(ScopeBuilder* scope_builder, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(scope_builder, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(scope_builder, index_expr->indexExpression.end_index);
  }
}

static void visit_booleanLiteral(ScopeBuilder* scope_builder, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void visit_integerLiteral(ScopeBuilder* scope_builder, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void visit_stringLiteral(ScopeBuilder* scope_builder, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void visit_default(ScopeBuilder* scope_builder, Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void visit_dontcare(ScopeBuilder* scope_builder, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}
