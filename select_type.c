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
static void visit_name(Ast* name, Type* required_ty);
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
static void visit_selectCaseList(Ast* case_list, Type* required_ty);
static void visit_selectCase(Ast* select_case, Type* required_ty);
static void visit_keysetExpression(Ast* keyset_expr, Type* required_ty);
static void visit_tupleKeysetExpression(Ast* tuple_expr, Type* required_ty);
static void visit_simpleKeysetExpression(Ast* simple_expr, Type* required_ty);
static void visit_simpleExpressionList(Ast* expr_list, Type* required_ty);

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

static void visit_typeRef(Ast* type_ref, Type* required_ty);
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
static void visit_typedefDeclaration(Ast* typedef_decl, Type* required_ty);

/** STATEMENTS **/

static void visit_assignmentStatement(Ast* assign_stmt);
static void visit_functionCall(Ast* func_call, Type* required_ty);
static void visit_returnStatement(Ast* return_stmt, Type* required_ty);
static void visit_exitStatement(Ast* exit_stmt);
static void visit_conditionalStatement(Ast* cond_stmt);
static void visit_directApplication(Ast* applic_stmt, Type* required_ty);
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
static void visit_actionRef(Ast* action_ref, Type* required_ty);
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
static void visit_argument(Ast* arg, Type* required_ty);
static void visit_expressionList(Ast* expr_list, Type* required_ty);
static void visit_lvalueExpression(Ast* lvalue_expr, Type* required_ty);
static void visit_expression(Ast* expr, Type* required_ty);
static void visit_castExpression(Ast* cast_expr, Type* required_ty);
static void visit_unaryExpression(Ast* unary_expr, Type* required_ty);
static void visit_binaryExpression(Ast* binary_expr, Type* required_ty);
static void visit_memberSelector(Ast* selector, Type* required_ty);
static void visit_arraySubscript(Ast* subscript);
static void visit_indexExpression(Ast* index_expr);
static void visit_booleanLiteral(Ast* bool_literal);
static void visit_integerLiteral(Ast* int_literal);
static void visit_stringLiteral(Ast* str_literal);
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

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

  for (ast = decl_list->first_child;
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
visit_name(Ast* name, Type* required_ty)
{
  assert(name->kind == AST_name);
  PotentialType* name_tau;
  Type* name_ty;

  name_tau = map_lookup(potype_map, name, 0);
  if (map_count(&name_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, name->line_no, name->column_no);
  }
  if (required_ty) {
    if (!match_type(name_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          source_file, name->line_no, name->column_no);
    } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      map_insert(storage, type_env, name, effective_type(name_ty), 0);
    }
  } else {
      name_ty = (Type*)name_tau->set.members.first->key;
      map_insert(storage, type_env, name, effective_type(name_ty), 0);
  }
}

static void
visit_parameterList(Ast* params)
{
  assert(params->kind == AST_parameterList);
  Ast* ast;

  for (ast = params->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(ast);
  }
}

static void
visit_parameter(Ast* param)
{
  assert(param->kind == AST_parameter);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0);
  }
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  //visit_typeRef(inst->instantiation.type);
  //visit_argumentList(inst->instantiation.args);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
}

static void
visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  Ast* ast;

  for (ast = local_elements->first_child;
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

  for (ast = states->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(ast);
  }
}

static void
visit_parserState(Ast* state)
{
  assert(state->kind == AST_parserState);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
}

static void
visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  Ast* ast;

  for (ast = stmts->first_child;
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
    visit_directApplication(stmt->parserStatement.stmt, 0);
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
    visit_name(state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

static void
visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  Type* list_ty;

  visit_expressionList(select_expr->selectExpression.expr_list, 0);
  list_ty = map_lookup(type_env, select_expr->selectExpression.expr_list, 0);
  visit_selectCaseList(select_expr->selectExpression.case_list, list_ty);
}

static void
visit_selectCaseList(Ast* case_list, Type* required_ty)
{
  assert(case_list->kind == AST_selectCaseList);
  Ast* ast;

  for (ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(ast, required_ty);
  }
}

static void
visit_selectCase(Ast* select_case, Type* required_ty)
{
  assert(select_case->kind == AST_selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr, required_ty);
  visit_name(select_case->selectCase.name, 0);
}

static void
visit_keysetExpression(Ast* keyset_expr, Type* required_ty)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  Type* keyset_ty;

  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr, required_ty);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr, required_ty);
  } else assert(0);
  keyset_ty = map_lookup(type_env, keyset_expr->keysetExpression.expr, 0);
  assert(keyset_ty);
  map_insert(storage, type_env, keyset_expr, keyset_ty, 0);
}

static void
visit_tupleKeysetExpression(Ast* tuple_expr, Type* required_ty)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  Type* tuple_ty;

  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list, required_ty);
  tuple_ty = map_lookup(type_env, tuple_expr->tupleKeysetExpression.expr_list, 0);
  map_insert(storage, type_env, tuple_expr, tuple_ty, 0);
}

static void
visit_simpleKeysetExpression(Ast* simple_expr, Type* required_ty)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  Type* simple_ty;

  if (required_ty->product.count != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, simple_expr->line_no, simple_expr->column_no);
  } else {
    if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
      visit_expression(simple_expr->simpleKeysetExpression.expr, required_ty->product.members[0]);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
      visit_default(simple_expr->simpleKeysetExpression.expr);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
      visit_dontcare(simple_expr->simpleKeysetExpression.expr);
    } else assert(0);
    simple_ty = array_append(storage, type_array, sizeof(Type));
    simple_ty->ty_former = TYPE_PRODUCT;
    simple_ty->ast = simple_expr;
    simple_ty->product.count = 1;
    simple_ty->product.members = arena_malloc(storage, simple_ty->product.count*sizeof(Type*));
    simple_ty->product.members[0] = map_lookup(type_env, simple_expr->simpleKeysetExpression.expr, 0);
    map_insert(storage, type_env, simple_expr, simple_ty, 0);
  }
}

static void
visit_simpleExpressionList(Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  Ast* ast;
  Type* list_ty;
  int i;

  list_ty = array_append(storage, type_array, sizeof(Type));
  list_ty->ty_former = TYPE_PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(ast, required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = arena_malloc(storage, list_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  map_insert(storage, type_env, expr_list, list_ty, 0);
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
}

static void
visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  Ast* ast;

  for (ast = local_decls->first_child;
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
}

static void
visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  Ast* ast;

  for (ast = protos->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(ast);
  }
}

static void
visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref, Type* required_ty)
{
  assert(type_ref->kind == AST_typeRef);
  Type* ref_ty;

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
    visit_name(type_ref->typeRef.type, required_ty);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  ref_ty = map_lookup(type_env, type_ref->typeRef.type, 0);
  if (required_ty) {
    if (!type_equiv(ref_ty, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          source_file, type_ref->line_no, type_ref->column_no);
    }
  }
  map_insert(storage, type_env, type_ref, ref_ty, 0);
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

  index_ty = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
  visit_expression(type_decl->headerStackType.stack_expr, index_ty);
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  Type* bool_ty;

  bool_ty = builtin_lookup(root_scope, "bool", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, bool_type, bool_ty, 0);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  Type* int_ty;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  int_ty = builtin_lookup(root_scope, "int", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, int_type, int_ty, 0);
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  Type* bit_ty;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  bit_ty = builtin_lookup(root_scope, "bit", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, bit_type, bit_ty, 0);
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  Type* varbit_ty;

  varbit_ty = builtin_lookup(root_scope, "varbit", NAMESPACE_TYPE)->type;
  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  map_insert(storage, type_env, varbit_type, varbit_ty, 0);
}

static void
visit_baseTypeString(Ast* string_type)
{
  assert(string_type->kind == AST_baseTypeString);
  Type* string_ty;

  string_ty = builtin_lookup(root_scope, "string", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, string_type, string_ty, 0);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  Type* void_ty;

  void_ty = builtin_lookup(root_scope, "void", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, void_type, void_ty, 0);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  Type* error_ty;

  error_ty = builtin_lookup(root_scope, "error", NAMESPACE_TYPE)->type;
  map_insert(storage, type_env, error_type, error_ty, 0);
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
    visit_typeRef(type_arg->realTypeArg.arg, 0);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

static void
visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

static void
visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  Ast* ast;

  for (ast = args->first_child;
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
    visit_typedefDeclaration(type_decl->typeDeclaration.decl, 0);
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
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = map_lookup(type_env, type_decl->derivedTypeDeclaration.decl, 0);
  map_insert(storage, type_env, type_decl, decl_ty, 0);
}

static void
visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
}

static void
visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  Ast* ast;

  for (ast = fields->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(ast);
  }
}

static void
visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
}

static void
visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
}

static void
visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  Ast* ast;

  for (ast = ident_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(ast);
  }
}

static void
visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr, 0);
  }
}

static void
visit_typedefDeclaration(Ast* typedef_decl, Type* required_ty)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Type* ref_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref, required_ty);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  ref_ty = map_lookup(type_env, typedef_decl->typedefDeclaration.type_ref, 0);
  map_insert(storage, type_env, typedef_decl, ref_ty, 0);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  Type* lhs_ty;

  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  lhs_ty = map_lookup(type_env, assign_stmt->assignmentStatement.lhs_expr, 0);
  assert(lhs_ty);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, lhs_ty);
}

static void
visit_functionCall(Ast* func_call, Type* required_ty)
{
  assert(func_call->kind == AST_functionCall);
  PotentialType* func_tau;
  Type* func_ty;

  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(func_call->functionCall.lhs_expr, required_ty);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, required_ty);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args, 0);
  func_tau = map_lookup(potype_map, func_call, 0);
  if (map_count(&func_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, func_call->line_no, func_call->column_no);
  }
  if (required_ty) {
    if (!match_type(func_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, func_call->line_no, func_call->column_no);
    } else {
      func_ty = (Type*)func_tau->set.members.first->key;
      map_insert(storage, type_env, func_call, effective_type(func_ty), 0);
    }
  } else {
    func_ty = (Type*)func_tau->set.members.first->key;
    map_insert(storage, type_env, func_call, effective_type(func_ty), 0);
  }
}

static void
visit_returnStatement(Ast* return_stmt, Type* required_ty)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, required_ty);
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
  visit_expression(cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

static void
visit_directApplication(Ast* applic_stmt, Type* required_ty)
{
  assert(applic_stmt->kind == AST_directApplication);
  visit_argumentList(applic_stmt->directApplication.args, required_ty);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(applic_stmt->directApplication.name, required_ty);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->directApplication.name, required_ty);
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
    visit_directApplication(stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(stmt->statement.stmt, 0);
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

  for (ast = stmt_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(ast);
  }
}

static void
visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visit_expression(switch_stmt->switchStatement.expr, 0);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

static void
visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  Ast* ast;

  for (ast = switch_cases->first_child;
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
    visit_name(label->switchLabel.label, 0);
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
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
}

static void
visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  Ast* ast;

  for (ast = prop_list->first_child;
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

  for (ast = element_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(ast);
  }
}

static void
visit_keyElement(Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(element->keyElement.expr, 0);
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

  for (ast = action_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(ast, 0);
  }
}

static void
visit_actionRef(Ast* action_ref, Type* required_ty)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args, required_ty);
  }
}

#if 0
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

  for (ast = entries_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(ast);
  }
}

static void
visit_entry(Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(entry->entry.keyset, 0);
  visit_actionRef(entry->entry.action, 0);
}

static void
visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visit_expression(simple_prop->simpleProperty.init_expr, 0);
}
#endif

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0);
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

  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast, required_ty);
  }
}

static void
visit_argument(Ast* arg, Type* required_ty)
{
  assert(arg->kind == AST_argument);
  Type* arg_ty;

  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(arg->argument.arg, required_ty);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  arg_ty = map_lookup(type_env, arg->argument.arg, 0);
  assert(arg_ty);
  map_insert(storage, type_env, arg, arg_ty, 0);
}

static void
visit_expressionList(Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;
  Type* list_ty;
  int i;

  list_ty = array_append(storage, type_array, sizeof(Type));
  list_ty->ty_former = TYPE_PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast, required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = arena_malloc(storage, list_ty->product.count*sizeof(Type*));
  }
  i = 0;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = map_lookup(type_env, ast, 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  map_insert(storage, type_env, expr_list, list_ty, 0);
}

static void
visit_lvalueExpression(Ast* lvalue_expr, Type* required_ty)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  Type* expr_ty;

  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  expr_ty = map_lookup(type_env, lvalue_expr->lvalueExpression.expr, 0);
  assert(expr_ty);
  map_insert(storage, type_env, lvalue_expr, expr_ty, 0);
}

static void
visit_expression(Ast* expr, Type* required_ty)
{
  assert(expr->kind == AST_expression);
  Type* expr_ty;

  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  expr_ty = map_lookup(type_env, expr->expression.expr, 0);
  if (!expr_ty) {
    assert(expr_ty);
  }
  map_insert(storage, type_env, expr, expr_ty, 0);
}

static void
visit_castExpression(Ast* cast_expr, Type* required_ty)
{
  assert(cast_expr->kind == AST_castExpression);
  Type* cast_ty;

  visit_typeRef(cast_expr->castExpression.type, required_ty);
  visit_expression(cast_expr->castExpression.expr, 0);
  cast_ty = map_lookup(type_env, cast_expr->castExpression.type, 0);
  map_insert(storage, type_env, cast_expr, cast_ty, 0);
}

static void
visit_unaryExpression(Ast* unary_expr, Type* required_ty)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, required_ty);
}

static void
visit_binaryExpression(Ast* binary_expr, Type* required_ty)
{
  assert(binary_expr->kind == AST_binaryExpression);
  PotentialType* op_tau;
  Type* op_ty;

  visit_expression(binary_expr->binaryExpression.left_operand, required_ty);
  visit_expression(binary_expr->binaryExpression.right_operand, required_ty);
  op_tau = map_lookup(potype_map, binary_expr, 0);
  if (map_count(&op_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, binary_expr->line_no, binary_expr->column_no);
  }
  if (required_ty) {
    if (!match_type(op_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, binary_expr->line_no, binary_expr->column_no);
    } else {
      op_ty = (Type*)op_tau->set.members.first->key;
      map_insert(storage, type_env, binary_expr, effective_type(op_ty), 0);
    }
  } else {
    op_ty = (Type*)op_tau->set.members.first->key;
    map_insert(storage, type_env, binary_expr, effective_type(op_ty), 0);
  }
}

static void
visit_memberSelector(Ast* selector, Type* required_ty)
{
  assert(selector->kind == AST_memberSelector);
  PotentialType* selector_tau;
  Type* selector_ty;

  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  selector_tau = map_lookup(potype_map, selector, 0);
  if (map_count(&selector_tau->set.members) != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, selector->line_no, selector->column_no);
  }
  if (required_ty) {
    if (!match_type(selector_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, selector->line_no, selector->column_no);
    } else {
      selector_ty = (Type*)selector_tau->set.members.first->key;
      map_insert(storage, type_env, selector, effective_type(selector_ty), 0);
    }
  } else {
    selector_ty = (Type*)selector_tau->set.members.first->key;
    map_insert(storage, type_env, selector, effective_type(selector_ty), 0);
  }
}

static void
visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  Type* lhs_ty;

  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
  lhs_ty = map_lookup(type_env, subscript->arraySubscript.lhs_expr, 0);
  map_insert(storage, type_env, subscript, lhs_ty, 0);
}

static void
visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visit_expression(index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, 0);
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
visit_default(Ast* default_)
{
  assert(default_->kind == AST_default);
}

static void
visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
}
