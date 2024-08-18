#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>   /* exit */
#include "foundation.h"
#include "frontend.h"

static char*  source_file;
static Arena* storage;
static Scope* root_scope;
static Map*   type_env, *potype_map;
static Map*   scope_map, *decl_map;
static Array* type_array;

/** PROGRAM **/

static void visit_p4program(Ast* p4program);
static void visit_declarationList(Ast* decl_list);
static void visit_declaration(Ast* decl);
static void visit_name(Ast* name, Type* required_ty, PotentialType* args_tau);
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
static void visit_typeArgumentList(Ast* args);
static void visit_typeDeclaration(Ast* type_decl);
static void visit_derivedTypeDeclaration(Ast* type_decl);
static void visit_headerTypeDeclaration(Ast* header_decl);
static void visit_headerUnionDeclaration(Ast* union_decl);
static void visit_structTypeDeclaration(Ast* struct_decl);
static void visit_structFieldList(Ast* fields);
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
static void visit_functionCall(Ast* func_call, Type* required_ty);
static void visit_returnStatement(Ast* return_stmt, Type* required_ty, PotentialType* args_tau);
static void visit_exitStatement(Ast* exit_stmt);
static void visit_conditionalStatement(Ast* cond_stmt);
static void visit_directApplication(Ast* applic_stmt, Type* required_ty, PotentialType* args_tau);
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
static void visit_actionRef(Ast* action_ref, Type* required_ty, PotentialType* args_tau);
static void visit_entriesProperty(Ast* entries_prop);
static void visit_entriesList(Ast* entries_list);
static void visit_entry(Ast* entry);
static void visit_simpleProperty(Ast* simple_prop);
static void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(Ast* func_decl);
static void visit_argumentList(Ast* args, Type* required_ty);
static void visit_argument(Ast* arg, Type* required_ty, PotentialType* args_tau);
static void visit_expressionList(Ast* expr_list, Type* required_ty, PotentialType* args_tau);
static void visit_lvalueExpression(Ast* lvalue_expr, Type* required_ty, PotentialType* args_tau);
static void visit_expression(Ast* expr, Type* required_ty, PotentialType* args_tau);
static void visit_castExpression(Ast* cast_expr, Type* required_ty, PotentialType* args_tau);
static void visit_unaryExpression(Ast* unary_expr, Type* required_ty, PotentialType* args_tau);
static void visit_binaryExpression(Ast* binary_expr, Type* required_ty, PotentialType* args_tau);
static void visit_memberSelector(Ast* selector, Type* required_ty, PotentialType* args_tau);
static void visit_arraySubscript(Ast* subscript, Type* required_ty, PotentialType* args_tau);
static void visit_indexExpression(Ast* index_expr, Type* required_ty, PotentialType* args_tau);
static void visit_booleanLiteral(Ast* bool_literal, Type* required_ty, PotentialType* args_tau);
static void visit_integerLiteral(Ast* int_literal, Type* required_ty, PotentialType* args_tau);
static void visit_stringLiteral(Ast* str_literal, Type* required_ty, PotentialType* args_tau);
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

static bool
match_type(PotentialType* tau, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = tau->members.first; m != 0; m = m->next) {
    ty = (Type*)m->key;
    if (type_equiv(ty, required_ty)) {
      i += 1;
    }
  }
  return (i == 1);
}

static bool
match_function(PotentialType* tau, Type* return_ty, PotentialType* args_tau)
{
  Type* func_ty, *params_ty;
  MapEntry* m;
  int i, j;

  i = 0;
  for (m = tau->members.first; m != 0; m = m->next) {
    func_ty = (Type*)m->key;
    params_ty = func_ty->function.params;
    assert(func_ty->ty_former == TYPE_FUNCTION);
    if (return_ty) {
      if (!type_equiv(func_ty->function.return_, return_ty)) continue;
    }
    if (params_ty->product.count != args_tau->product.count) continue;
    for (j = 0; j < params_ty->product.count; j++) {
      if (!match_type(args_tau->product.members[j], params_ty->product.members[j])) break;
    }
    if (j != params_ty->product.count) continue;
    i += 1;
  } 
  return (i == 1);
}

void
select_type(Arena* storage_, char* source_file_, Ast* p4program, Scope* root_scope_,
    Array* type_array_, Map* scope_map_, Map* decl_map_, Map* type_env_, Map* potype_map_)
{
  storage = storage_;
  source_file = source_file_;
  root_scope = root_scope_;
  type_array = type_array_;
  scope_map = scope_map_;
  decl_map = decl_map_;
  type_env = type_env_;
  potype_map = potype_map_;

  visit_p4program(p4program);
}

/** PROGRAM **/

static void
visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visit_declarationList(p4program->p4program.decl_list);
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
visit_name(Ast* name, Type* required_ty, PotentialType* args_tau)
{
  assert(name->kind == AST_name);
  PotentialType* name_tau;
  Type* name_ty;

  name_tau = map_lookup(potype_map, name, 0);
  if (required_ty) {
    if (!match_type(name_tau, required_ty)) {
      error("%s:%d:%d: error: type mismatch.",
          source_file, name->line_no, name->column_no);
    }
  } else {
    if (map_count(&name_tau->members) != 1) {
      error("%s:%d:%d: error: ambiguous or unknown type.",
          source_file, name->line_no, name->column_no);
    }
    name_ty = (Type*)name_tau->members.first->key;
    map_insert(storage, type_env, name, name_ty, 0);
  }
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
  //visit_typeRef(param->parameter.type);
  //visit_name(param->parameter.name);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0, 0);
  }
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  //visit_name(type_decl->packageTypeDeclaration.name);
  //visit_parameterList(type_decl->packageTypeDeclaration.params);
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  //visit_typeRef(inst->instantiation.type);
  //visit_argumentList(inst->instantiation.args);
  //visit_name(inst->instantiation.name);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  //visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    //visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  //visit_name(type_decl->parserTypeDeclaration.name);
  //visit_parameterList(type_decl->parserTypeDeclaration.params);
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
  //visit_name(state->parserState.name);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
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
    visit_functionCall(stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->parserStatement.stmt, 0, 0);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
}

static void
visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
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
    visit_name(state_expr->stateExpression.expr, 0, 0);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list, 0, 0);
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
  //visit_name(select_case->selectCase.name);
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
  Type* expr_ty;

  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    expr_ty = builtin_type(root_scope, "int");
    visit_expression(simple_expr->simpleKeysetExpression.expr, expr_ty, 0);
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
  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

static void
visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  //visit_name(type_decl->controlTypeDeclaration.name);
  //visit_parameterList(type_decl->controlTypeDeclaration.params);
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
  //visit_name(type_decl->externTypeDeclaration.name);
  //visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
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
  if (func_proto->functionPrototype.return_type) {
    //visit_typeRef(func_proto->functionPrototype.return_type);
  }
  //visit_name(func_proto->functionPrototype.name);
  //visit_parameterList(func_proto->functionPrototype.params);
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
    visit_name(type_ref->typeRef.type, 0, 0);
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
  Type* index_ty;

  visit_typeRef(type_decl->headerStackType.type);
  index_ty = builtin_type(root_scope, "int");
  visit_expression(type_decl->headerStackType.stack_expr, index_ty, 0);
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  //visit_name(bool_type->baseTypeBoolean.name);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  //visit_name(int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  //visit_name(bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  //visit_name(varbit_type->baseTypeVarbit.name);
  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  //visit_name(str_type->baseTypeString.name);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  //visit_name(void_type->baseTypeVoid.name);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  //visit_name(error_type->baseTypeError.name);
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
    visit_name(type_arg->typeArg.arg, 0, 0);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

static void
visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  Ast* ast;

  for (ast = args->typeArgumentList.first_child;
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
  //visit_name(header_decl->headerTypeDeclaration.name);
  //visit_structFieldList(header_decl->headerTypeDeclaration.fields);
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  //visit_name(union_decl->headerUnionDeclaration.name);
  //visit_structFieldList(union_decl->headerUnionDeclaration.fields);
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  //visit_name(struct_decl->structTypeDeclaration.name);
  //visit_structFieldList(struct_decl->structTypeDeclaration.fields);
}

static void
visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  Ast* ast;

  for (ast = fields->structFieldList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(ast);
  }
}

static void
visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
  //visit_typeRef(field->structField.type);
  //visit_name(field->structField.name);
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  //visit_name(enum_decl->enumDeclaration.name);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  //visit_identifierList(error_decl->errorDeclaration.fields);
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  //visit_identifierList(match_decl->matchKindDeclaration.fields);
}

static void
visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  Ast* ast;

  for (ast = ident_list->identifierList.first_child;
       ast != 0; ast = ast->right_sibling) {
    //visit_name(ast);
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
  //visit_name(ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr, 0, 0);
  }
}

static void
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  //visit_name(typedef_decl->typedefDeclaration.name);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  Type* lhs_ty;

  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0, 0);
  } else assert(0);
  lhs_ty = map_lookup(type_env, assign_stmt->assignmentStatement.lhs_expr, 0);
  assert(lhs_ty);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, lhs_ty, 0);
}

static void
visit_functionCall(Ast* func_call, Type* required_ty)
{
  assert(func_call->kind == AST_functionCall);
  PotentialType* args_tau;

  args_tau = map_lookup(potype_map, func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(func_call->functionCall.lhs_expr, required_ty, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, required_ty, args_tau);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args, 0);
}

static void
visit_returnStatement(Ast* return_stmt, Type* required_ty, PotentialType* args_tau)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, required_ty, args_tau);
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
  visit_expression(cond_stmt->conditionalStatement.cond_expr, 0, 0);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

static void
visit_directApplication(Ast* applic_stmt, Type* required_ty, PotentialType* args_tau)
{
  assert(applic_stmt->kind == AST_directApplication);
  visit_argumentList(applic_stmt->directApplication.args, required_ty);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(applic_stmt->directApplication.name, required_ty, args_tau);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
}

static void
visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->statement.stmt, 0, 0);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(stmt->statement.stmt, 0, 0);
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
  visit_expression(switch_stmt->switchStatement.expr, 0, 0);
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
    visit_name(label->switchLabel.label, 0, 0);
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
  //visit_name(table_decl->tableDeclaration.name);
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
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
  visit_expression(element->keyElement.expr, 0, 0);
  //visit_name(element->keyElement.match);
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
    visit_actionRef(ast, 0, 0);
  }
}

static void
visit_actionRef(Ast* action_ref, Type* required_ty, PotentialType* args_tau)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(action_ref->actionRef.name, 0, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args, required_ty);
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
  visit_actionRef(entry->entry.action, 0, 0);
}

static void
visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  //visit_name(simple_prop->simpleProperty.name);
  visit_expression(simple_prop->simpleProperty.init_expr, 0, 0);
}

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  //visit_name(action_decl->actionDeclaration.name);
  //visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  //visit_typeRef(var_decl->variableDeclaration.type);
  //visit_name(var_decl->variableDeclaration.name);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0, 0);
  }
}

/** EXPRESSIONS **/

static void
visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

static void
visit_argumentList(Ast* args, Type* required_ty)
{
  assert(args->kind == AST_argumentList);
  Ast* ast;

  for (ast = args->argumentList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast, required_ty, 0);
  }
}

static void
visit_argument(Ast* arg, Type* required_ty, PotentialType* args_tau)
{
  assert(arg->kind == AST_argument);
  Type* arg_ty;

  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(arg->argument.arg, required_ty, args_tau);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  arg_ty = map_lookup(type_env, arg->argument.arg, 0);
  map_insert(storage, type_env, arg, arg_ty, 0);
}

static void
visit_expressionList(Ast* expr_list, Type* required_ty, PotentialType* args_tau)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;

  for (ast = expr_list->expressionList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast, required_ty, args_tau);
  }
}

static void
visit_lvalueExpression(Ast* lvalue_expr, Type* required_ty, PotentialType* args_tau)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  Type* expr_ty;

  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(lvalue_expr->lvalueExpression.expr, required_ty, args_tau);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, required_ty, args_tau);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr, required_ty, args_tau);
  } else assert(0);
  expr_ty = map_lookup(type_env, lvalue_expr->lvalueExpression.expr, 0);
  assert(expr_ty);
  map_insert(storage, type_env, lvalue_expr, expr_ty, 0);
}

static void
visit_expression(Ast* expr, Type* required_ty, PotentialType* args_tau)
{
  assert(expr->kind == AST_expression);
  Type* expr_ty;

  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expression.expr, required_ty, args_tau);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  expr_ty = map_lookup(type_env, expr->expression.expr, 0);
  assert(expr_ty);
  map_insert(storage, type_env, expr, expr_ty, 0);
}

static void
visit_castExpression(Ast* cast_expr, Type* required_ty, PotentialType* args_tau)
{
  assert(cast_expr->kind == AST_castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr, required_ty, args_tau);
}

static void
visit_unaryExpression(Ast* unary_expr, Type* required_ty, PotentialType* args_tau)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, required_ty, args_tau);
}

static void
visit_binaryExpression(Ast* binary_expr, Type* required_ty, PotentialType* args_tau)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand, required_ty, args_tau);
  visit_expression(binary_expr->binaryExpression.right_operand, required_ty, args_tau);
}

static void
visit_memberSelector(Ast* selector, Type* required_ty, PotentialType* args_tau)
{
  assert(selector->kind == AST_memberSelector);
  PotentialType* selector_tau;

  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0, 0);
  } else assert(0);
  selector_tau = map_lookup(potype_map, selector, 0);
  if (!match_function(selector_tau, required_ty, args_tau)) {
    error("%s:%d:%d: error: type mismatch.",
          source_file, selector->line_no, selector->column_no);
  }
}

static void
visit_arraySubscript(Ast* subscript, Type* required_ty, PotentialType* args_tau)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, required_ty, args_tau);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, required_ty, args_tau);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr, required_ty, args_tau);
}

static void
visit_indexExpression(Ast* index_expr, Type* required_ty, PotentialType* args_tau)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(index_expr->indexExpression.start_index, required_ty, args_tau);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, required_ty, args_tau);
  }
}

static void
visit_booleanLiteral(Ast* bool_literal, Type* required_ty, PotentialType* args_tau)
{
  assert(bool_literal->kind == AST_booleanLiteral);
}

static void
visit_integerLiteral(Ast* int_literal, Type* required_ty, PotentialType* args_tau)
{
  assert(int_literal->kind == AST_integerLiteral);
  PotentialType* literal_tau;

  if (required_ty) {
    literal_tau = map_lookup(potype_map, int_literal, 0);
    if (!match_type(literal_tau, required_ty)) {
      error("%s:%d:%d: error: type mismatch.",
            source_file, int_literal->line_no, int_literal->column_no);
    }
  }
}

static void
visit_stringLiteral(Ast* str_literal, Type* required_ty, PotentialType* args_tau)
{
  assert(str_literal->kind == AST_stringLiteral);
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
}
