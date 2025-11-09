#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

/** PROGRAM **/

static void visit_p4program(TypeChecker* checker, Ast* p4program);
static void visit_declarationList(TypeChecker* checker, Ast* decl_list);
static void visit_declaration(TypeChecker* checker, Ast* decl);
static void visit_name(TypeChecker* checker, Ast* name, Type* required_ty);
static void visit_parameterList(TypeChecker* checker, Ast* params);
static void visit_parameter(TypeChecker* checker, Ast* param);
static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_instantiation(TypeChecker* checker, Ast* inst);

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl);
static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements);
static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element);
static void visit_parserStates(TypeChecker* checker, Ast* states);
static void visit_parserState(TypeChecker* checker, Ast* state);
static void visit_parserStatements(TypeChecker* checker, Ast* stmts);
static void visit_parserStatement(TypeChecker* checker, Ast* stmt);
static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt);
static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt);
static void visit_stateExpression(TypeChecker* checker, Ast* state_expr);
static void visit_selectExpression(TypeChecker* checker, Ast* select_expr);
static void visit_selectCaseList(TypeChecker* checker, Ast* case_list, Type* required_ty);
static void visit_selectCase(TypeChecker* checker, Ast* select_case, Type* required_ty);
static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr, Type* required_ty);
static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr, Type* required_ty);
static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr, Type* required_ty);
static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty);

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl);
static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls);
static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl);

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl);
static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_methodPrototypes(TypeChecker* checker, Ast* protos);
static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto);

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref, Type* required_ty);
static void visit_tupleType(TypeChecker* checker, Ast* type);
static void visit_headerStackType(TypeChecker* checker, Ast* type_decl);
static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type);
static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type);
static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type);
static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type);
static void visit_baseTypeString(TypeChecker* checker, Ast* str_type);
static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type);
static void visit_baseTypeError(TypeChecker* checker, Ast* error_type);
static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size);
static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg);
static void visit_typeArg(TypeChecker* checker, Ast* type_arg);
static void visit_typeArgumentList(TypeChecker* checker, Ast* args);
static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl);
static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl);
static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl);
static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl);
static void visit_structFieldList(TypeChecker* checker, Ast* fields);
static void visit_structField(TypeChecker* checker, Ast* field);
static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl);
static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl);
static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl);
static void visit_identifierList(TypeChecker* checker, Ast* ident_list);
static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list);
static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident);
static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl, Type* required_ty);

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt);
static void visit_functionCall(TypeChecker* checker, Ast* func_call, Type* required_ty);
static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt, Type* required_ty);
static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt);
static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt);
static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt, Type* required_ty);
static void visit_statement(TypeChecker* checker, Ast* stmt);
static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt);
static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list);
static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt);
static void visit_switchCases(TypeChecker* checker, Ast* switch_cases);
static void visit_switchCase(TypeChecker* checker, Ast* switch_case);
static void visit_switchLabel(TypeChecker* checker, Ast* label);
static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt);

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl);
static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list);
static void visit_tableProperty(TypeChecker* checker, Ast* table_prop);
static void visit_keyProperty(TypeChecker* checker, Ast* key_prop);
static void visit_keyElementList(TypeChecker* checker, Ast* element_list);
static void visit_keyElement(TypeChecker* checker, Ast* element);
static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop);
static void visit_actionList(TypeChecker* checker, Ast* action_list);
static void visit_actionRef(TypeChecker* checker, Ast* action_ref, Type* required_ty);
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop);
static void visit_entriesList(TypeChecker* checker, Ast* entries_list);
static void visit_entry(TypeChecker* checker, Ast* entry);
static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop);
static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl);
static void visit_argumentList(TypeChecker* checker, Ast* args, Type* required_ty);
static void visit_argument(TypeChecker* checker, Ast* arg, Type* required_ty);
static void visit_expressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty);
static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, Type* required_ty);
static void visit_expression(TypeChecker* checker, Ast* expr, Type* required_ty);
static void visit_castExpression(TypeChecker* checker, Ast* cast_expr, Type* required_ty);
static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr, Type* required_ty);
static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr, Type* required_ty);
static void visit_memberSelector(TypeChecker* checker, Ast* selector, Type* required_ty);
static void visit_arraySubscript(TypeChecker* checker, Ast* subscript);
static void visit_indexExpression(TypeChecker* checker, Ast* index_expr);
static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal);
static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal);
static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal);
static void visit_default(TypeChecker* checker, Ast* default_);
static void visit_dontcare(TypeChecker* checker, Ast* dontcare);

void select_type(TypeChecker* checker)
{
  visit_p4program(checker, checker->p4program);
}

/** PROGRAM **/

static void visit_p4program(TypeChecker* checker, Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(checker, p4program->p4program.decl_list);
}

static void visit_declarationList(TypeChecker* checker, Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(TypeChecker* checker, Ast* decl)
{
  assert(decl->kind == AstEnum::declaration);
  if (decl->declaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::externDeclaration) {
    visit_externDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::functionDeclaration) {
    visit_functionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserDeclaration) {
    visit_parserDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlDeclaration) {
    visit_controlDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::typeDeclaration) {
    visit_typeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::errorDeclaration) {
    visit_errorDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::matchKindDeclaration) {
    visit_matchKindDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(checker, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(TypeChecker* checker, Ast* name, Type* required_ty)
{
  assert(name->kind == AstEnum::name);
  PotentialType* name_tau;
  Type* name_ty;

  name_tau = (PotentialType*)checker->potype_map->lookup(name, 0);
  if (name_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, name->line_no, name->column_no);
  }
  if (required_ty) {
    if (!checker->match_type(name_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          checker->source_file, name->line_no, name->column_no);
    } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      checker->type_env->insert(name, name_ty->effective_type(), 0);
    }
  } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      checker->type_env->insert(name, name_ty->effective_type(), 0);
  }
}

static void visit_parameterList(TypeChecker* checker, Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parameter(TypeChecker* checker, Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr, 0);
  }
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  //visit_typeRef(checker, inst->instantiation.type);
  //visit_argumentList(checker, inst->instantiation.args);
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
}

static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(checker, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(TypeChecker* checker, Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(TypeChecker* checker, Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  visit_parserStatements(checker, state->parserState.stmt_list);
  visit_transitionStatement(checker, state->parserState.transition_stmt);
}

static void visit_parserStatements(TypeChecker* checker, Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(checker, stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(checker, block_stmt->parserBlockStatement.stmt_list);
}

static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(checker, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(TypeChecker* checker, Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(checker, state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  Type* list_ty;

  visit_expressionList(checker, select_expr->selectExpression.expr_list, 0);
  list_ty = (Type*)checker->type_env->lookup(select_expr->selectExpression.expr_list, 0);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list, list_ty);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list, Type* required_ty)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(checker, container_of(ast, Ast, tree), required_ty);
  }
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case, Type* required_ty)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(checker, select_case->selectCase.keyset_expr, required_ty);
  visit_name(checker, select_case->selectCase.name, 0);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr, Type* required_ty)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  Type* keyset_ty;

  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr, required_ty);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr, required_ty);
  } else assert(0);
  keyset_ty = (Type*)checker->type_env->lookup(keyset_expr->keysetExpression.expr, 0);
  assert(keyset_ty);
  checker->type_env->insert(keyset_expr, keyset_ty, 0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr, Type* required_ty)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  Type* tuple_ty;

  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list, required_ty);
  tuple_ty = (Type*)checker->type_env->lookup(tuple_expr->tupleKeysetExpression.expr_list, 0);
  checker->type_env->insert(tuple_expr, tuple_ty, 0);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr, Type* required_ty)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  Type* simple_ty;

  if (required_ty->product.count != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, simple_expr->line_no, simple_expr->column_no);
  } else {
    if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
      visit_expression(checker, simple_expr->simpleKeysetExpression.expr, required_ty->product.members[0]);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
      visit_default(checker, simple_expr->simpleKeysetExpression.expr);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
      visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
    } else assert(0);
    simple_ty = (Type*)checker->type_array->append(sizeof(Type));
    simple_ty->ty_former = TypeEnum::PRODUCT;
    simple_ty->ast = simple_expr;
    simple_ty->product.count = 1;
    simple_ty->product.members = (Type**)checker->storage->malloc(simple_ty->product.count * sizeof(Type*));
    simple_ty->product.members[0] = (Type*)checker->type_env->lookup(simple_expr->simpleKeysetExpression.expr, 0);
    checker->type_env->insert(simple_expr, simple_ty, 0);
  }
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  AstTree* ast;
  Type* list_ty;
  int i;

  list_ty = (Type*)checker->type_array->append(sizeof(Type));
  list_ty->ty_former = TypeEnum::PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(checker, container_of(ast, Ast, tree), required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = (Type**)checker->storage->malloc(list_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  checker->type_env->insert(expr_list, list_ty, 0);
}

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  visit_typeDeclaration(checker, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(checker, control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(checker, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(checker, control_decl->controlDeclaration.apply_stmt);
}

static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
}

static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl)
{
  assert(local_decl->kind == AstEnum::controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::tableDeclaration) {
    visit_tableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(checker, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(checker, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(checker, extern_decl->externDeclaration.decl);
  } else assert(0);
}

static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
}

static void visit_methodPrototypes(TypeChecker* checker, Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(checker, container_of(ast, Ast, tree));
  }
}

static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref, Type* required_ty)
{
  assert(type_ref->kind == AstEnum::typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AstEnum::baseTypeBoolean) {
    visit_baseTypeBoolean(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeInteger) {
    visit_baseTypeInteger(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeBit) {
    visit_baseTypeBit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVarbit) {
    visit_baseTypeVarbit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeString) {
    visit_baseTypeString(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVoid) {
    visit_baseTypeVoid(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeError) {
    visit_baseTypeError(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::name) {
    visit_name(checker, type_ref->typeRef.type, required_ty);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  ref_ty = (Type*)checker->type_env->lookup(type_ref->typeRef.type, 0);
  if (required_ty) {
    if (!type_equiv(checker, ref_ty, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          checker->source_file, type_ref->line_no, type_ref->column_no);
    }
  }
  checker->type_env->insert(type_ref, ref_ty, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  Type* index_ty;

  index_ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  visit_expression(checker, type_decl->headerStackType.stack_expr, index_ty);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  Type* bool_ty;

  bool_ty = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
  checker->type_env->insert(bool_type, bool_ty, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  Type* int_ty;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  int_ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  checker->type_env->insert(int_type, int_ty, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  Type* bit_ty;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  bit_ty = checker->root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
  checker->type_env->insert(bit_type, bit_ty, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  Type* varbit_ty;

  varbit_ty = checker->root_scope->builtin_lookup("varbit", NameSpace::TYPE)->type;
  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  checker->type_env->insert(varbit_type, varbit_ty, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* string_type)
{
  assert(string_type->kind == AstEnum::baseTypeString);
  Type* string_ty;

  string_ty = checker->root_scope->builtin_lookup("string", NameSpace::TYPE)->type;
  checker->type_env->insert(string_type, string_ty, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  Type* void_ty;

  void_ty = checker->root_scope->builtin_lookup("void", NameSpace::TYPE)->type;
  checker->type_env->insert(void_type, void_ty, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  Type* error_ty;

  error_ty = checker->root_scope->builtin_lookup("error", NameSpace::TYPE)->type;
  checker->type_env->insert(error_type, error_ty, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg, 0);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(checker, type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(checker, container_of(ast, Ast, tree));
  }
}

static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl, 0);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    visit_packageTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else assert(0);
}

static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)checker->type_env->lookup(type_decl->derivedTypeDeclaration.decl, 0);
  checker->type_env->insert(type_decl, decl_ty, 0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
}

static void visit_structFieldList(TypeChecker* checker, Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  AstTree* ast;

  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(checker, container_of(ast, Ast, tree));
  }
}

static void visit_structField(TypeChecker* checker, Ast* field)
{
  assert(field->kind == AstEnum::structField);
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields);
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
}

static void visit_identifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
}

static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(checker, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(checker, ident->specifiedIdentifier.init_expr, 0);
  }
}

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl, Type* required_ty)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  Type* ref_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref, required_ty);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  ref_ty = (Type*)checker->type_env->lookup(typedef_decl->typedefDeclaration.type_ref, 0);
  checker->type_env->insert(typedef_decl, ref_ty, 0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  Type* lhs_ty;

  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  lhs_ty = (Type*)checker->type_env->lookup(assign_stmt->assignmentStatement.lhs_expr, 0);
  assert(lhs_ty);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr, lhs_ty);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call, Type* required_ty)
{
  assert(func_call->kind == AstEnum::functionCall);
  PotentialType* func_tau;
  Type* func_ty;

  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr, required_ty);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr, required_ty);
  } else assert(0);
  visit_argumentList(checker, func_call->functionCall.args, 0);
  func_tau = (PotentialType*)checker->potype_map->lookup(func_call, 0);
  if (func_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, func_call->line_no, func_call->column_no);
  }
  if (required_ty) {
    if (!checker->match_type(func_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, func_call->line_no, func_call->column_no);
    } else {
      func_ty = (Type*)func_tau->set.members.first->key;
      checker->type_env->insert(func_call, func_ty->effective_type(), 0);
    }
  } else {
    func_ty = (Type*)func_tau->set.members.first->key;
    checker->type_env->insert(func_call, func_ty->effective_type(), 0);
  }
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt, Type* required_ty)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr, required_ty);
  }
}

static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(checker, cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(checker, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(checker, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt, Type* required_ty)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  visit_argumentList(checker, applic_stmt->directApplication.args, required_ty);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(checker, applic_stmt->directApplication.name, required_ty);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name, required_ty);
  } else assert(0);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(checker, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(checker, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(checker, switch_stmt->switchStatement.expr, 0);
  visit_switchCases(checker, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(TypeChecker* checker, Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(TypeChecker* checker, Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(checker, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(checker, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(TypeChecker* checker, Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(checker, label->switchLabel.label, 0);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(checker, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::statement) {
    visit_statement(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::instantiation) {
    visit_instantiation(checker, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  visit_tablePropertyList(checker, table_decl->tableDeclaration.prop_list);
}

static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(checker, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(TypeChecker* checker, Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    visit_keyProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    visit_actionsProperty(checker, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AstEnum::entriesProperty) {
    visit_entriesProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::simpleProperty) {
    visit_simpleProperty(checker, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

static void visit_keyProperty(TypeChecker* checker, Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(checker, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(TypeChecker* checker, Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(TypeChecker* checker, Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(checker, element->keyElement.expr, 0);
}

static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(checker, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(TypeChecker* checker, Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(checker, container_of(ast, Ast, tree), 0);
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref, Type* required_ty)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(checker, action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args, required_ty);
  }
}

#if 0
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  visit_entriesList(checker, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(TypeChecker* checker, Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(checker, container_of(ast, Ast, tree));
  }
}

static void visit_entry(TypeChecker* checker, Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  visit_keysetExpression(checker, entry->entry.keyset, 0);
  visit_actionRef(checker, entry->entry.action, 0);
}

static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  visit_expression(checker, simple_prop->simpleProperty.init_expr, 0);
}
#endif

static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr, 0);
  }
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args, Type* required_ty)
{
  assert(args->kind == AstEnum::argumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(checker, container_of(ast, Ast, tree), required_ty);
  }
}

static void visit_argument(TypeChecker* checker, Ast* arg, Type* required_ty)
{
  assert(arg->kind == AstEnum::argument);
  Type* arg_ty;

  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(checker, arg->argument.arg, required_ty);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, arg->argument.arg);
  } else assert(0);
  arg_ty = (Type*)checker->type_env->lookup(arg->argument.arg, 0);
  assert(arg_ty);
  checker->type_env->insert(arg, arg_ty, 0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AstEnum::expressionList);
  AstTree* ast;
  Type* list_ty;
  int i;

  list_ty = (Type*)checker->type_array->append(sizeof(Type));
  list_ty->ty_former = TypeEnum::PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(checker, container_of(ast, Ast, tree), required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = (Type**)checker->storage->malloc(list_ty->product.count * sizeof(Type*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = (Type*)checker->type_env->lookup(container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  checker->type_env->insert(expr_list, list_ty, 0);
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, Type* required_ty)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  Type* expr_ty;

  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(checker, lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  expr_ty = (Type*)checker->type_env->lookup(lvalue_expr->lvalueExpression.expr, 0);
  assert(expr_ty);
  checker->type_env->insert(lvalue_expr, expr_ty, 0);
}

static void visit_expression(TypeChecker* checker, Ast* expr, Type* required_ty)
{
  assert(expr->kind == AstEnum::expression);
  Type* expr_ty;

  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    visit_name(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
  expr_ty = (Type*)checker->type_env->lookup(expr->expression.expr, 0);
  if (!expr_ty) {
    assert(expr_ty);
  }
  checker->type_env->insert(expr, expr_ty, 0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr, Type* required_ty)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  Type* cast_ty;

  visit_typeRef(checker, cast_expr->castExpression.type, required_ty);
  visit_expression(checker, cast_expr->castExpression.expr, 0);
  cast_ty = (Type*)checker->type_env->lookup(cast_expr->castExpression.type, 0);
  checker->type_env->insert(cast_expr, cast_ty, 0);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr, Type* required_ty)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand, required_ty);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr, Type* required_ty)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  PotentialType* op_tau;
  Type* op_ty;

  visit_expression(checker, binary_expr->binaryExpression.left_operand, required_ty);
  visit_expression(checker, binary_expr->binaryExpression.right_operand, required_ty);
  op_tau = (PotentialType*)checker->potype_map->lookup(binary_expr, 0);
  if (op_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, binary_expr->line_no, binary_expr->column_no);
  }
  if (required_ty) {
    if (!checker->match_type(op_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, binary_expr->line_no, binary_expr->column_no);
    } else {
      op_ty = (Type*)op_tau->set.members.first->key;
      checker->type_env->insert(binary_expr, op_ty->effective_type(), 0);
    }
  } else {
    op_ty = (Type*)op_tau->set.members.first->key;
    checker->type_env->insert(binary_expr, op_ty->effective_type(), 0);
  }
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector, Type* required_ty)
{
  assert(selector->kind == AstEnum::memberSelector);
  PotentialType* selector_tau;
  Type* selector_ty;

  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  selector_tau = (PotentialType*)checker->potype_map->lookup(selector, 0);
  if (selector_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, selector->line_no, selector->column_no);
  }
  if (required_ty) {
    if (!checker->match_type(selector_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, selector->line_no, selector->column_no);
    } else {
      selector_ty = (Type*)selector_tau->set.members.first->key;
      checker->type_env->insert(selector, selector_ty->effective_type(), 0);
    }
  } else {
    selector_ty = (Type*)selector_tau->set.members.first->key;
    checker->type_env->insert(selector, selector_ty->effective_type(), 0);
  }
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  Type* lhs_ty;

  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
  lhs_ty = (Type*)checker->type_env->lookup(subscript->arraySubscript.lhs_expr, 0);
  checker->type_env->insert(subscript, lhs_ty, 0);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  visit_expression(checker, index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index, 0);
  }
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
}
