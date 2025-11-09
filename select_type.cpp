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
  assert(p4program->kind == AST_p4program);
  visit_declarationList(checker, p4program->p4program.decl_list);
}

static void visit_declarationList(TypeChecker* checker, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_declaration(TypeChecker* checker, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visit_externDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visit_functionDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visit_parserDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visit_controlDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visit_typeDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visit_errorDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visit_matchKindDeclaration(checker, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visit_instantiation(checker, decl->declaration.decl);
  } else assert(0);
}

static void visit_name(TypeChecker* checker, Ast* name, Type* required_ty)
{
  assert(name->kind == AST_name);
  PotentialType* name_tau;
  Type* name_ty;

  name_tau = (PotentialType*)map_lookup(checker->potype_map, name, 0);
  if (map_count(&name_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, name->line_no, name->column_no);
  }
  if (required_ty) {
    if (!match_type(checker, name_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          checker->source_file, name->line_no, name->column_no);
    } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      map_insert(checker->type_env, name, name_ty->effective_type(), 0);
    }
  } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      map_insert(checker->type_env, name, name_ty->effective_type(), 0);
  }
}

static void visit_parameterList(TypeChecker* checker, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parameter(TypeChecker* checker, Ast* param)
{
  assert(param->kind == AST_parameter);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr, 0);
  }
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  //visit_typeRef(checker, inst->instantiation.type);
  //visit_argumentList(checker, inst->instantiation.args);
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
}

static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserLocalElement(TypeChecker* checker, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visit_instantiation(checker, local_element->parserLocalElement.element);
  } else assert(0);
}

static void visit_parserStates(TypeChecker* checker, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserState(TypeChecker* checker, Ast* state)
{
  assert(state->kind == AST_parserState);
  visit_parserStatements(checker, state->parserState.stmt_list);
  visit_transitionStatement(checker, state->parserState.transition_stmt);
}

static void visit_parserStatements(TypeChecker* checker, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_parserStatement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void visit_parserBlockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(checker, block_stmt->parserBlockStatement.stmt_list);
}

static void visit_transitionStatement(TypeChecker* checker, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visit_stateExpression(checker, transition_stmt->transitionStatement.stmt);
}

static void visit_stateExpression(TypeChecker* checker, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(checker, state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  Type* list_ty;

  visit_expressionList(checker, select_expr->selectExpression.expr_list, 0);
  list_ty = (Type*)map_lookup(checker->type_env, select_expr->selectExpression.expr_list, 0);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list, list_ty);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list, Type* required_ty)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(checker, container_of(ast, Ast, tree), required_ty);
  }
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case, Type* required_ty)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(checker, select_case->selectCase.keyset_expr, required_ty);
  visit_name(checker, select_case->selectCase.name, 0);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr, Type* required_ty)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  Type* keyset_ty;

  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr, required_ty);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr, required_ty);
  } else assert(0);
  keyset_ty = (Type*)map_lookup(checker->type_env, keyset_expr->keysetExpression.expr, 0);
  assert(keyset_ty);
  map_insert(checker->type_env, keyset_expr, keyset_ty, 0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr, Type* required_ty)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  Type* tuple_ty;

  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list, required_ty);
  tuple_ty = (Type*)map_lookup(checker->type_env, tuple_expr->tupleKeysetExpression.expr_list, 0);
  map_insert(checker->type_env, tuple_expr, tuple_ty, 0);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr, Type* required_ty)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  Type* simple_ty;

  if (required_ty->product.count != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, simple_expr->line_no, simple_expr->column_no);
  } else {
    if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
      visit_expression(checker, simple_expr->simpleKeysetExpression.expr, required_ty->product.members[0]);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
      visit_default(checker, simple_expr->simpleKeysetExpression.expr);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
      visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
    } else assert(0);
    simple_ty = (Type*)checker->type_array->append(sizeof(Type));
    simple_ty->ty_former = TypeEnum::PRODUCT;
    simple_ty->ast = simple_expr;
    simple_ty->product.count = 1;
    simple_ty->product.members = (Type**)checker->storage->malloc(simple_ty->product.count * sizeof(Type*));
    simple_ty->product.members[0] = (Type*)map_lookup(checker->type_env, simple_expr->simpleKeysetExpression.expr, 0);
    map_insert(checker->type_env, simple_expr, simple_ty, 0);
  }
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AST_simpleExpressionList);
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
    list_ty->product.members[i] = (Type*)map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  map_insert(checker->type_env, expr_list, list_ty, 0);
}

/** CONTROL **/

static void visit_controlDeclaration(TypeChecker* checker, Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  visit_typeDeclaration(checker, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(checker, control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(checker, control_decl->controlDeclaration.local_decls);
  visit_blockStatement(checker, control_decl->controlDeclaration.apply_stmt);
}

static void visit_controlTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
}

static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_controlLocalDeclaration(TypeChecker* checker, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visit_actionDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visit_tableDeclaration(checker, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visit_instantiation(checker, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

static void visit_externDeclaration(TypeChecker* checker, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visit_externTypeDeclaration(checker, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visit_functionPrototype(checker, extern_decl->externDeclaration.decl);
  } else assert(0);
}

static void visit_externTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
}

static void visit_methodPrototypes(TypeChecker* checker, Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(checker, container_of(ast, Ast, tree));
  }
}

static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref, Type* required_ty)
{
  assert(type_ref->kind == AST_typeRef);
  Type* ref_ty;

  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visit_baseTypeBoolean(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visit_baseTypeInteger(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visit_baseTypeBit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visit_baseTypeVarbit(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visit_baseTypeString(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visit_baseTypeVoid(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visit_baseTypeError(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visit_name(checker, type_ref->typeRef.type, required_ty);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  ref_ty = (Type*)map_lookup(checker->type_env, type_ref->typeRef.type, 0);
  if (required_ty) {
    if (!type_equiv(checker, ref_ty, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          checker->source_file, type_ref->line_no, type_ref->column_no);
    }
  }
  map_insert(checker->type_env, type_ref, ref_ty, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Type* index_ty;

  index_ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  visit_expression(checker, type_decl->headerStackType.stack_expr, index_ty);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  Type* bool_ty;

  bool_ty = checker->root_scope->builtin_lookup("bool", NameSpace::TYPE)->type;
  map_insert(checker->type_env, bool_type, bool_ty, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  Type* int_ty;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  int_ty = checker->root_scope->builtin_lookup("int", NameSpace::TYPE)->type;
  map_insert(checker->type_env, int_type, int_ty, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  Type* bit_ty;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  bit_ty = checker->root_scope->builtin_lookup("bit", NameSpace::TYPE)->type;
  map_insert(checker->type_env, bit_type, bit_ty, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  Type* varbit_ty;

  varbit_ty = checker->root_scope->builtin_lookup("varbit", NameSpace::TYPE)->type;
  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  map_insert(checker->type_env, varbit_type, varbit_ty, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* string_type)
{
  assert(string_type->kind == AST_baseTypeString);
  Type* string_ty;

  string_ty = checker->root_scope->builtin_lookup("string", NameSpace::TYPE)->type;
  map_insert(checker->type_env, string_type, string_ty, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  Type* void_ty;

  void_ty = checker->root_scope->builtin_lookup("void", NameSpace::TYPE)->type;
  map_insert(checker->type_env, void_type, void_ty, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  Type* error_ty;

  error_ty = checker->root_scope->builtin_lookup("error", NameSpace::TYPE)->type;
  map_insert(checker->type_env, error_type, error_ty, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg, 0);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(checker, type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(checker, container_of(ast, Ast, tree));
  }
}

static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl, 0);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visit_parserTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visit_controlTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visit_packageTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else assert(0);
}

static void visit_derivedTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = (Type*)map_lookup(checker->type_env, type_decl->derivedTypeDeclaration.decl, 0);
  map_insert(checker->type_env, type_decl, decl_ty, 0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
}

static void visit_structFieldList(TypeChecker* checker, Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(checker, container_of(ast, Ast, tree));
  }
}

static void visit_structField(TypeChecker* checker, Ast* field)
{
  assert(field->kind == AST_structField);
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields);
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
}

static void visit_identifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
}

static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(checker, container_of(ast, Ast, tree));
  }
}

static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(checker, ident->specifiedIdentifier.init_expr, 0);
  }
}

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl, Type* required_ty)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Type* ref_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref, required_ty);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  ref_ty = (Type*)map_lookup(checker->type_env, typedef_decl->typedefDeclaration.type_ref, 0);
  map_insert(checker->type_env, typedef_decl, ref_ty, 0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  Type* lhs_ty;

  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  lhs_ty = (Type*)map_lookup(checker->type_env, assign_stmt->assignmentStatement.lhs_expr, 0);
  assert(lhs_ty);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr, lhs_ty);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call, Type* required_ty)
{
  assert(func_call->kind == AST_functionCall);
  PotentialType* func_tau;
  Type* func_ty;

  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr, required_ty);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr, required_ty);
  } else assert(0);
  visit_argumentList(checker, func_call->functionCall.args, 0);
  func_tau = (PotentialType*)map_lookup(checker->potype_map, func_call, 0);
  if (map_count(&func_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, func_call->line_no, func_call->column_no);
  }
  if (required_ty) {
    if (!match_type(checker, func_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, func_call->line_no, func_call->column_no);
    } else {
      func_ty = (Type*)func_tau->set.members.first->key;
      map_insert(checker->type_env, func_call, func_ty->effective_type(), 0);
    }
  } else {
    func_ty = (Type*)func_tau->set.members.first->key;
    map_insert(checker->type_env, func_call, func_ty->effective_type(), 0);
  }
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt, Type* required_ty)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr, required_ty);
  }
}

static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
}

static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visit_expression(checker, cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(checker, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(checker, cond_stmt->conditionalStatement.else_stmt);
  }
}

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt, Type* required_ty)
{
  assert(applic_stmt->kind == AST_directApplication);
  visit_argumentList(checker, applic_stmt->directApplication.args, required_ty);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(checker, applic_stmt->directApplication.name, required_ty);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name, required_ty);
  } else assert(0);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visit_switchStatement(checker, stmt->statement.stmt);
  } else assert(0);
}

static void visit_blockStatement(TypeChecker* checker, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visit_statementOrDeclList(checker, block_stmt->blockStatement.stmt_list);
}

static void visit_statementOrDeclList(TypeChecker* checker, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchStatement(TypeChecker* checker, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(checker, switch_stmt->switchStatement.expr, 0);
  visit_switchCases(checker, switch_stmt->switchStatement.switch_cases);
}

static void visit_switchCases(TypeChecker* checker, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(checker, container_of(ast, Ast, tree));
  }
}

static void visit_switchCase(TypeChecker* checker, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visit_switchLabel(checker, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(checker, switch_case->switchCase.stmt);
  }
}

static void visit_switchLabel(TypeChecker* checker, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    visit_name(checker, label->switchLabel.label, 0);
  } else if (label->switchLabel.label->kind == AST_default) {
    visit_default(checker, label->switchLabel.label);
  } else assert(0);
}

static void visit_statementOrDeclaration(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visit_statement(checker, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visit_instantiation(checker, stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

static void visit_tableDeclaration(TypeChecker* checker, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  visit_tablePropertyList(checker, table_decl->tableDeclaration.prop_list);
}

static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(checker, container_of(ast, Ast, tree));
  }
}

static void visit_tableProperty(TypeChecker* checker, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visit_keyProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visit_actionsProperty(checker, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visit_entriesProperty(checker, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visit_simpleProperty(checker, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

static void visit_keyProperty(TypeChecker* checker, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visit_keyElementList(checker, key_prop->keyProperty.keyelem_list);
}

static void visit_keyElementList(TypeChecker* checker, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(checker, container_of(ast, Ast, tree));
  }
}

static void visit_keyElement(TypeChecker* checker, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(checker, element->keyElement.expr, 0);
}

static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(checker, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(TypeChecker* checker, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(checker, container_of(ast, Ast, tree), 0);
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref, Type* required_ty)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(checker, action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args, required_ty);
  }
}

#if 0
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visit_entriesList(checker, entries_prop->entriesProperty.entries_list);
}

static void visit_entriesList(TypeChecker* checker, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(checker, container_of(ast, Ast, tree));
  }
}

static void visit_entry(TypeChecker* checker, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(checker, entry->entry.keyset, 0);
  visit_actionRef(checker, entry->entry.action, 0);
}

static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(checker, simple_prop->simpleProperty.init_expr, 0);
}
#endif

static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr, 0);
  }
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args, Type* required_ty)
{
  assert(args->kind == AST_argumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(checker, container_of(ast, Ast, tree), required_ty);
  }
}

static void visit_argument(TypeChecker* checker, Ast* arg, Type* required_ty)
{
  assert(arg->kind == AST_argument);
  Type* arg_ty;

  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(checker, arg->argument.arg, required_ty);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(checker, arg->argument.arg);
  } else assert(0);
  arg_ty = (Type*)map_lookup(checker->type_env, arg->argument.arg, 0);
  assert(arg_ty);
  map_insert(checker->type_env, arg, arg_ty, 0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AST_expressionList);
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
    list_ty->product.members[i] = (Type*)map_lookup(checker->type_env, container_of(ast, Ast, tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  map_insert(checker->type_env, expr_list, list_ty, 0);
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, Type* required_ty)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  Type* expr_ty;

  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(checker, lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  expr_ty = (Type*)map_lookup(checker->type_env, lvalue_expr->lvalueExpression.expr, 0);
  assert(expr_ty);
  map_insert(checker->type_env, lvalue_expr, expr_ty, 0);
}

static void visit_expression(TypeChecker* checker, Ast* expr, Type* required_ty)
{
  assert(expr->kind == AST_expression);
  Type* expr_ty;

  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(checker, expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
  expr_ty = (Type*)map_lookup(checker->type_env, expr->expression.expr, 0);
  if (!expr_ty) {
    assert(expr_ty);
  }
  map_insert(checker->type_env, expr, expr_ty, 0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr, Type* required_ty)
{
  assert(cast_expr->kind == AST_castExpression);
  Type* cast_ty;

  visit_typeRef(checker, cast_expr->castExpression.type, required_ty);
  visit_expression(checker, cast_expr->castExpression.expr, 0);
  cast_ty = (Type*)map_lookup(checker->type_env, cast_expr->castExpression.type, 0);
  map_insert(checker->type_env, cast_expr, cast_ty, 0);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr, Type* required_ty)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand, required_ty);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr, Type* required_ty)
{
  assert(binary_expr->kind == AST_binaryExpression);
  PotentialType* op_tau;
  Type* op_ty;

  visit_expression(checker, binary_expr->binaryExpression.left_operand, required_ty);
  visit_expression(checker, binary_expr->binaryExpression.right_operand, required_ty);
  op_tau = (PotentialType*)map_lookup(checker->potype_map, binary_expr, 0);
  if (map_count(&op_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, binary_expr->line_no, binary_expr->column_no);
  }
  if (required_ty) {
    if (!match_type(checker, op_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, binary_expr->line_no, binary_expr->column_no);
    } else {
      op_ty = (Type*)op_tau->set.members.first->key;
      map_insert(checker->type_env, binary_expr, op_ty->effective_type(), 0);
    }
  } else {
    op_ty = (Type*)op_tau->set.members.first->key;
    map_insert(checker->type_env, binary_expr, op_ty->effective_type(), 0);
  }
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector, Type* required_ty)
{
  assert(selector->kind == AST_memberSelector);
  PotentialType* selector_tau;
  Type* selector_ty;

  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  selector_tau = (PotentialType*)map_lookup(checker->potype_map, selector, 0);
  if (map_count(&selector_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        checker->source_file, selector->line_no, selector->column_no);
  }
  if (required_ty) {
    if (!match_type(checker, selector_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            checker->source_file, selector->line_no, selector->column_no);
    } else {
      selector_ty = (Type*)selector_tau->set.members.first->key;
      map_insert(checker->type_env, selector, selector_ty->effective_type(), 0);
    }
  } else {
    selector_ty = (Type*)selector_tau->set.members.first->key;
    map_insert(checker->type_env, selector, selector_ty->effective_type(), 0);
  }
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  Type* lhs_ty;

  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
  lhs_ty = (Type*)map_lookup(checker->type_env, subscript->arraySubscript.lhs_expr, 0);
  map_insert(checker->type_env, subscript, lhs_ty, 0);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(checker, index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index, 0);
  }
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}
