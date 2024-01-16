#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena*   storage;
static Scope*   root_scope, *enclosing_scope;
static Set*     opened_scopes;
static Set*     type_table;
static UnboundedArray* type_array;

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
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

static Type*
create_product_type(int i, int j, Arena* storage)
{
  Type* product_ty, *ty;

  if (j == i) {
    return 0;
  } else if ((j - i) == 1) {
    return (Type*)array_get_element(type_array, i, sizeof(Type));
  } else if ((j - i) == 2) {
    product_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    product_ty->ctor = TYPE_PRODUCT;
    product_ty->product.rhs = (Type*)array_get_element(type_array, i+1, sizeof(Type));
    product_ty->product.lhs = (Type*)array_get_element(type_array, i, sizeof(Type));
    return product_ty;
  } else if ((j - i) > 2) {
    product_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    product_ty->ctor = TYPE_PRODUCT;
    product_ty->product.rhs = (Type*)array_get_element(type_array, j-1, sizeof(Type));
    product_ty->product.lhs = (Type*)array_get_element(type_array, j-2, sizeof(Type));
    for (int k = j-3; k >= i; k--) {
      ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
      ty->ctor = TYPE_PRODUCT;
      ty->product.lhs = (Type*)array_get_element(type_array, k, sizeof(Type));
      ty->product.rhs = product_ty;
      product_ty = ty;
    }
    return product_ty;
  } else assert(0);
  return 0;
}

void
insert_type_table_entry(Set* table, Ast* ast, Type* type)
{
  SetMember* m;

  m = set_add_member(table, storage, (uint64_t)ast, (uint64_t)type);
  assert(m);
}

Type*
lookup_type_table(Set* table, Ast* ast)
{
  SetMember* m;

  m = set_lookup_member(table, (uint64_t)ast);
  if (m) {
    return (Type*)m->value;
  }
  return 0;
}

Type*
actual_type(Type* type)
{
  if (type->ctor == TYPE_TYPE) {
    return type->type.type;
  } else {
    return type;
  }
  assert(0);
  return 0;
}

Set*
build_type_table(Ast* p4program, Scope* root_scope_, UnboundedArray** type_array_,
        Set* opened_scopes_, Arena* storage_)
{
  struct BuiltinType {
    char* strname;
    enum TypeEnum type;
  };

  struct BuiltinType builtin_types[] = {
    {"void",   TYPE_VOID},
    {"bool",   TYPE_BOOL},
    {"int",    TYPE_INT},
    {"bit",    TYPE_BIT},
    {"varbit", TYPE_VARBIT},
    {"string", TYPE_STRING},
    {"error",  TYPE_ENUM},
    {"match_kind", TYPE_ENUM},
    {"_",      TYPE_DONTCARE},
  };

  Type* builtin_ty;
  NameDecl* name_decl;

  storage = storage_;
  root_scope = root_scope_;
  opened_scopes = opened_scopes_;
  type_table = arena_malloc(storage, sizeof(Set));
  *type_table = (Set){};
  type_array = array_create(storage, sizeof(Type), 1008);

  for (int i = 0; i < sizeof(builtin_types)/sizeof(builtin_types[0]); i++) {
    name_decl = scope_lookup_namespace(root_scope, builtin_types[i].strname, NS_TYPE)->ns[NS_TYPE];
    builtin_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    builtin_ty->ctor = builtin_types[i].type;
    builtin_ty->strname = builtin_types[i].strname;
    name_decl->type = builtin_ty;
    insert_type_table_entry(type_table, name_decl->ast, builtin_ty);
  }

  enclosing_scope = root_scope;
  visit_p4program(p4program);
  assert(enclosing_scope == root_scope);

  *type_array_ = type_array;
  return type_table;
}

/** PROGRAM **/

static void
visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, p4program);

  visit_declarationList(p4program->p4program.decl_list);

  enclosing_scope = prev_scope;
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
  assert(name->kind == AST_name);
  Type* name_ty;

  name_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  name_ty->ctor = TYPE_NAMEREF;
  name_ty->strname = name->name.strname;
  name_ty->nameref.name = name;
  name_ty->nameref.scope = enclosing_scope;

  insert_type_table_entry(type_table, name, name_ty);
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
  visit_typeRef(param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  Ast* ast, *name, *params;
  Type* package_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, type_decl);

  if (type_decl->packageTypeDeclaration.type_params) {
    visit_typeParameterList(type_decl->packageTypeDeclaration.type_params);
  }
  visit_parameterList(type_decl->packageTypeDeclaration.params);

  name = type_decl->packageTypeDeclaration.name;
  package_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  package_ty->ctor = TYPE_FUNCTION;
  package_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, type_decl, package_ty);

  i = type_array->elem_count;
  params = type_decl->packageTypeDeclaration.params;
  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->parameter.type;
  }
  package_ty->function.params = create_product_type(i, type_array->elem_count, storage);

  enclosing_scope = prev_scope;
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  visit_typeRef(inst->instantiation.type_ref);
  visit_argumentList(inst->instantiation.args);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, parser_decl);

  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);

  enclosing_scope = prev_scope;
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  Ast* ast, *name, *params;
  Type* parser_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, type_decl);

  if (type_decl->parserTypeDeclaration.type_params) {
    visit_typeParameterList(type_decl->parserTypeDeclaration.type_params);
  }
  visit_parameterList(type_decl->parserTypeDeclaration.params);

  name = type_decl->parserTypeDeclaration.name;
  parser_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  parser_ty->ctor = TYPE_FUNCTION;
  parser_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, type_decl, parser_ty);

  i = type_array->elem_count;
  params = type_decl->parserTypeDeclaration.params;
  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->parameter.type;
  }
  parser_ty->function.params = create_product_type(i, type_array->elem_count, storage);

  enclosing_scope = prev_scope;
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
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, state);

  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);

  enclosing_scope = prev_scope;
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

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, block_stmt);

  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);

  enclosing_scope = prev_scope;
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
    ;
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
    ;
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

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, control_decl);

  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);

  enclosing_scope = prev_scope;
}

static void
visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  Ast* ast, *name, *params;
  Type* control_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, type_decl);

  if (type_decl->controlTypeDeclaration.type_params) {
    visit_typeParameterList(type_decl->controlTypeDeclaration.type_params);
  }
  visit_parameterList(type_decl->controlTypeDeclaration.params);

  name = type_decl->controlTypeDeclaration.name;
  control_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  control_ty->ctor = TYPE_FUNCTION;
  control_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, type_decl, control_ty);

  i = type_array->elem_count;
  params = type_decl->packageTypeDeclaration.params;
  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->parameter.type;
  }
  control_ty->function.params = create_product_type(i, type_array->elem_count, storage);

  enclosing_scope = prev_scope;
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
  Ast* ast, *name, *methods;
  Type* extern_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, type_decl);

  if (type_decl->externTypeDeclaration.type_params) {
    visit_typeParameterList(type_decl->externTypeDeclaration.type_params);
  }
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);

  name = type_decl->externTypeDeclaration.name;
  extern_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  extern_ty->ctor = TYPE_EXTERN;
  extern_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, type_decl, extern_ty);

  i = type_array->elem_count;
  methods = type_decl->externTypeDeclaration.method_protos;
  for (ast = methods->methodPrototypes.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast;
  }
  extern_ty->extern_.methods = create_product_type(i, type_array->elem_count, storage);

  enclosing_scope = prev_scope;
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
  Ast* ast, *name, *params, *return_type;
  Type* func_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, func_proto);

  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }

  if (func_proto->functionPrototype.type_params) {
    visit_typeParameterList(func_proto->functionPrototype.type_params);
  }
  visit_parameterList(func_proto->functionPrototype.params);

  name = func_proto->functionPrototype.name;
  func_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  func_ty->ctor = TYPE_FUNCTION;
  func_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, func_proto, func_ty);

  i = type_array->elem_count;
  params = func_proto->functionPrototype.params;
  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->parameter.type;
  }
  func_ty->function.params = create_product_type(i, type_array->elem_count, storage);

  return_type = func_proto->functionPrototype.return_type;
  if (return_type) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = return_type;
    func_ty->function.return_ = ty;
  }

  enclosing_scope = prev_scope;
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref)
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
    visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_specializedType) {
    visit_specializedType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);

  ref_ty = lookup_type_table(type_table, type_ref->typeRef.type);
  insert_type_table_entry(type_table, type_ref, ref_ty);
}

static void
visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  Ast* ast, *args;
  Type* tuple_ty, *ty;
  int i;

  visit_typeArgumentList(type_decl->tupleType.type_args);

  i = type_array->elem_count;
  args = type_decl->tupleType.type_args;
  for (ast = args->typeArgumentList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast;
  }
  tuple_ty = create_product_type(i, type_array->elem_count, storage);

  insert_type_table_entry(type_table, type_decl, tuple_ty);
}

static void
visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  Type* stack_ty, *ty;

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);

  stack_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  stack_ty->ctor = TYPE_ARRAY;

  insert_type_table_entry(type_table, type_decl, stack_ty);

  ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  ty->ctor = TYPE_IDREF;
  ty->idref.ref = type_decl->headerStackType.type;
  stack_ty->array.element = ty;
}

static void
visit_specializedType(Ast* type_decl)
{
  assert(type_decl->kind == AST_specializedType);
  Type* specd_ty, *ty;

  visit_typeRef(type_decl->specializedType.type);
  visit_typeArgumentList(type_decl->specializedType.type_args);

  specd_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  specd_ty->ctor = TYPE_SPECIALIZED;

  insert_type_table_entry(type_table, type_decl, specd_ty);

  ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  ty->ctor = TYPE_IDREF;
  ty->idref.ref = type_decl->specializedType.type;
  specd_ty->specialized.ref = ty;
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  NameDecl* name_decl;

  name_decl = scope_lookup_namespace(root_scope, "bool", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, bool_type, name_decl->type);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  NameDecl* name_decl;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }

  name_decl = scope_lookup_namespace(root_scope, "int", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, int_type, name_decl->type);
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  NameDecl* name_decl;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }

  name_decl = scope_lookup_namespace(root_scope, "bit", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, bit_type, name_decl->type);
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  NameDecl* name_decl;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);

  name_decl = scope_lookup_namespace(root_scope, "varbit", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, varbit_type, name_decl->type);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  NameDecl* name_decl;

  name_decl = scope_lookup_namespace(root_scope, "string", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, str_type, name_decl->type);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  NameDecl* name_decl;

  name_decl = scope_lookup_namespace(root_scope, "void", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, void_type, name_decl->type);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  NameDecl* name_decl;

  name_decl = scope_lookup_namespace(root_scope, "error", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, error_type, name_decl->type);
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
  Ast* ast, *name;
  Type* param_ty;

  for (ast = param_list->typeParameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    name = ast;
    param_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    param_ty->ctor = TYPE_TYPEVAR;
    param_ty->strname = name->name.strname;

    insert_type_table_entry(type_table, name, param_ty);
  }
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
  Type* arg_ty;

  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);

  arg_ty = lookup_type_table(type_table, type_arg->typeArg.arg);
  insert_type_table_entry(type_table, type_arg, arg_ty);
}

static void
visit_realTypeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_realTypeArgumentList);
  Ast* ast;

  for (ast = arg_list->realTypeArgumentList.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_realTypeArg(ast);
  }
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

  decl_ty = lookup_type_table(type_table, type_decl->derivedTypeDeclaration.decl);
  insert_type_table_entry(type_table, type_decl, decl_ty);
}

static void
visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  Ast* ast, *name, *fields;
  Type* header_ty, *ty;
  int i;

  visit_structFieldList(header_decl->headerTypeDeclaration.fields);

  name = header_decl->headerTypeDeclaration.name;
  header_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  header_ty->ctor = TYPE_STRUCT;
  header_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, header_decl, header_ty);

  i = type_array->elem_count;
  fields = header_decl->headerTypeDeclaration.fields;
  for (ast = fields->structFieldList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->structField.type;
  }
  header_ty->struct_.fields = create_product_type(i, type_array->elem_count, storage);
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  Ast* ast, *name, *fields;
  Type* union_ty, *ty;
  int i;

  visit_structFieldList(union_decl->headerUnionDeclaration.fields);

  name = union_decl->headerUnionDeclaration.name;
  union_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  union_ty->ctor = TYPE_STRUCT;
  union_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, union_decl, union_ty);

  i = type_array->elem_count;
  fields = union_decl->headerUnionDeclaration.fields;
  for (ast = fields->structFieldList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->structField.type;
  }
  union_ty->struct_.fields = create_product_type(i, type_array->elem_count, storage);
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  Ast* ast, *name, *fields;
  Type* struct_ty, *ty;
  int i;

  visit_structFieldList(struct_decl->structTypeDeclaration.fields);

  name = struct_decl->structTypeDeclaration.name;
  struct_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  struct_ty->ctor = TYPE_STRUCT;
  struct_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, struct_decl, struct_ty);

  i = type_array->elem_count;
  fields = struct_decl->headerTypeDeclaration.fields;
  for (ast = fields->structFieldList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->structField.type;
  }
  struct_ty->struct_.fields = create_product_type(i, type_array->elem_count, storage);
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
  visit_typeRef(field->structField.type);
}

static void
visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  Ast* name;
  Type* enum_ty;

  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);

  name = enum_decl->enumDeclaration.name;
  enum_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  enum_ty->ctor = TYPE_ENUM;
  enum_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, enum_decl, enum_ty);
}

static void
visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visit_identifierList(error_decl->errorDeclaration.fields);
}

static void
visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visit_identifierList(match_decl->matchKindDeclaration.fields);
}

static void
visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  Ast* ast;

  for (ast = ident_list->identifierList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ;
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
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr);
  }
}

static void
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  Ast* name;
  Type* typedef_ty, *ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);

  name = typedef_decl->typedefDeclaration.name;
  typedef_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  typedef_ty->ctor = TYPE_TYPEDEF;
  typedef_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, typedef_decl, typedef_ty);

  ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  ty->ctor = TYPE_IDREF;
  ty->idref.ref = typedef_decl->typedefDeclaration.type_ref;
  typedef_ty->typedef_.ref = ty;
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
  Ast* lhs_expr;

  lhs_expr = func_call->functionCall.lhs_expr;
  if (lhs_expr->kind == AST_expression) {
    visit_expression(lhs_expr);
  } else if (lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(lhs_expr);
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
    ;
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
    prev_scope = enclosing_scope;
    enclosing_scope = lookup_opened_scope(opened_scopes, stmt);
    visit_blockStatement(stmt->statement.stmt);
    enclosing_scope = prev_scope;
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
    ;
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
  Type* table_ty;

  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);

  name = table_decl->tableDeclaration.name;
  table_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  table_ty->ctor = TYPE_TABLE;
  table_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, table_decl, table_ty);
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
  visit_expression(element->keyElement.expr);
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
  visit_expression(simple_prop->simpleProperty.init_expr);
}

static void
visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  Ast* ast, *name, *params;
  Type* action_ty, *ty;
  int i;
  Scope* prev_scope;

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, action_decl);

  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);

  name = action_decl->actionDeclaration.name;
  action_ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
  action_ty->ctor = TYPE_FUNCTION;
  action_ty->strname = name->name.strname;

  insert_type_table_entry(type_table, action_decl, action_ty);

  i = type_array->elem_count;
  params = action_decl->actionDeclaration.params;
  for (ast = params->parameterList.first_child;
       ast != 0; ast = ast->right_sibling) {
    ty = (Type*)array_append_element(type_array, storage, sizeof(Type));
    ty->ctor = TYPE_IDREF;
    ty->idref.ref = ast->parameter.type;
  }
  action_ty->function.params = create_product_type(i, type_array->elem_count, storage);

  enclosing_scope = prev_scope;
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visit_typeRef(var_decl->variableDeclaration.type);
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

  prev_scope = enclosing_scope;
  enclosing_scope = lookup_opened_scope(opened_scopes, func_decl);

  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);

  enclosing_scope = prev_scope;
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
    ;
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
    ;
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
    ;
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
  if (expr->expression.type_args) {
    visit_realTypeArgumentList(expr->expression.type_args);
  }
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
  NameDecl* name_decl;

  name_decl = scope_lookup_namespace(root_scope, "_", NS_TYPE)->ns[NS_TYPE];
  insert_type_table_entry(type_table, dontcare, name_decl->type);
}

