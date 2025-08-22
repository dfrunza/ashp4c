#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

/** PROGRAM **/

static void visit_p4program(TypeChecker* checker, Ast* p4program);
static void visit_declarationList(TypeChecker* checker, Ast* decl_list);
static void visit_declaration(TypeChecker* checker, Ast* decl);
static void visit_name(TypeChecker* checker, Ast* name, PotentialType* potential_args);
static void visit_parameterList(TypeChecker* checker, Ast* params);
static void visit_parameter(TypeChecker* checker, Ast* param);
static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* package_decl);
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
static void visit_selectCaseList(TypeChecker* checker, Ast* case_list);
static void visit_selectCase(TypeChecker* checker, Ast* select_case);
static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr);
static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr);
static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr);
static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list);

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

static void visit_typeRef(TypeChecker* checker, Ast* type_ref);
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
static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl);

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt);
static void visit_functionCall(TypeChecker* checker, Ast* func_call);
static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt);
static void visit_exitStatement(TypeChecker* checker, Ast* exit_stmt);
static void visit_conditionalStatement(TypeChecker* checker, Ast* cond_stmt);
static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt);
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
static void visit_actionRef(TypeChecker* checker, Ast* action_ref);
static void visit_entriesProperty(TypeChecker* checker, Ast* entries_prop);
static void visit_entriesList(TypeChecker* checker, Ast* entries_list);
static void visit_entry(TypeChecker* checker, Ast* entry);
static void visit_simpleProperty(TypeChecker* checker, Ast* simple_prop);
static void visit_actionDeclaration(TypeChecker* checker, Ast* action_decl);

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl);

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl);
static void visit_argumentList(TypeChecker* checker, Ast* args);
static void visit_argument(TypeChecker* checker, Ast* arg);
static void visit_expressionList(TypeChecker* checker, Ast* expr_list);
static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, PotentialType* potential_args);
static void visit_expression(TypeChecker* checker, Ast* expr, PotentialType* potential_args);
static void visit_castExpression(TypeChecker* checker, Ast* cast_expr);
static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr);
static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr);
static void visit_memberSelector(TypeChecker* checker, Ast* selector, PotentialType* potential_args);
static void visit_arraySubscript(TypeChecker* checker, Ast* subscript);
static void visit_indexExpression(TypeChecker* checker, Ast* index_expr);
static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal);
static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal);
static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal);
static void visit_default(TypeChecker* checker, Ast* default_);
static void visit_dontcare(TypeChecker* checker, Ast* dontcare);

bool match_type(TypeChecker* checker, PotentialType* potential_types, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = potential_types->set.members.first; m != 0; m = m->next) {
    ty = effective_type((Type*)m->key);
    if (type_equiv(checker, ty, actual_type(required_ty))) {
      i += 1;
    }
  }
  return (i == 1);
}

bool match_params(TypeChecker* checker, PotentialType* potential_args, Type* params_ty)
{
  int i;

  if (params_ty->product.count != potential_args->product.count) return 0;
  for (i = 0; i < params_ty->product.count; i++) {
    if (!match_type(checker, potential_args->product.members[i],
                    params_ty->product.members[i])) break;
  }
  return (i == params_ty->product.count);
}


static void collect_matching_member(TypeChecker* checker, PotentialType* tau, Type* product_ty,
    char* strname, PotentialType* potential_args)
{
  Type* member_ty;

  for (int i = 0; i < product_ty->product.count; i++) {
    member_ty = product_ty->product.members[i];
    if (cstr_match(member_ty->strname, strname)) {
      if (member_ty->ty_former == TYPE_FUNCTION) {
        if (match_params(checker, potential_args, member_ty->function.params)) {
          map_insert(checker->storage, &tau->set.members, member_ty, 0, 1);
        }
      } else {
        map_insert(checker->storage, &tau->set.members, member_ty, 0, 1);
      }
    }
  }
}

static void Debug_print_potential_types(PotentialType* tau)
{
  MapEntry* m;
  Type* ty;
  int i;

  i = 0;
  for (m = tau->set.members.first; m != 0; m = m->next) {
    ty = (Type*)m->key;
    if (ty->strname) {
      printf("  [%d] 0x%x %s %s\n", i, ty, TypeEnum_to_string(ty->ty_former), ty->strname);
    } else {
      printf("  [%d] 0x%x %s\n", i, ty, TypeEnum_to_string(ty->ty_former));
    }
    i += 1;
  }
}

void potential_types(TypeChecker* checker)
{
  checker->potype_map = arena_malloc(checker->storage, sizeof(Map));
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
  Ast* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_declaration(checker, ast);
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

static void visit_name(TypeChecker* checker, Ast* name, PotentialType* potential_args)
{
  assert(name->kind == AST_name);
  Scope* scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  PotentialType* tau;
  Type* ty, *ctors_ty;
  static Array* name_ty;

  if (!name_ty) name_ty = array_create(checker->storage, sizeof(Type*), 1);
  name_ty->elem_count = 0;
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, name, tau, 0);
  scope = map_lookup(checker->scope_map, name, 0);
  name_entry = scope_lookup(scope, name->name.strname, NAMESPACE_VAR|NAMESPACE_TYPE);
  name_decl = name_entry->ns[NAMESPACE_VAR >> 1];
  if (name_decl) {
    ty = map_lookup(checker->type_env, name_decl->ast, 0);
    *(Type**)array_append(checker->storage, name_ty, sizeof(Type*)) = actual_type(ty);
    assert(!name_decl->next_in_scope);
  }
  name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
  for(; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = map_lookup(checker->type_env, name_decl->ast, 0);
    *(Type**)array_append(checker->storage, name_ty, sizeof(Type*)) = actual_type(ty);
  }
  for (int i = 0; i < name_ty->elem_count; i++) {
    ty = *(Type**)array_get(name_ty, i, sizeof(Type*));
    if (potential_args) {
      if (ty->ty_former == TYPE_FUNCTION) {
        if (match_params(checker, potential_args, ty->function.params)) {
          map_insert(checker->storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_PARSER) {
        if (match_params(checker, potential_args, ty->parser.ctor_params)) {
          map_insert(checker->storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_CONTROL) {
        if (match_params(checker, potential_args, ty->control.ctor_params)) {
          map_insert(checker->storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_EXTERN) {
        ctors_ty = ty->extern_.ctors;
        for (int j = 0; j < ctors_ty->product.count; j++) {
          ty = ctors_ty->product.members[j];
          if (match_params(checker, potential_args, ty->function.params)) {
            map_insert(checker->storage, &tau->set.members, ty, 0, 0);
          }
        }
      } else assert(0);
    } else {
      map_insert(checker->storage, &tau->set.members, ty, 0, 0);
    }
  }
}

static void visit_parameterList(TypeChecker* checker, Ast* params)
{
  assert(params->kind == AST_parameterList);
  Ast* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_parameter(checker, ast);
  }
}

static void visit_parameter(TypeChecker* checker, Ast* param)
{
  assert(param->kind == AST_parameter);
  visit_typeRef(checker, param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr, 0);
  }
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  visit_parameterList(checker, type_decl->packageTypeDeclaration.params);
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  PotentialType* tau;
  Type* inst_ty;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, inst, tau, 0);
  visit_typeRef(checker, inst->instantiation.type);
  visit_argumentList(checker, inst->instantiation.args);
  inst_ty = map_lookup(checker->type_env, inst, 0);
  map_insert(checker->storage, &tau->set.members, actual_type(inst_ty), 0, 1);
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_typeDeclaration(checker, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(checker, parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  visit_parameterList(checker, type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(checker, type_decl->parserTypeDeclaration.method_protos);
}

static void visit_parserLocalElements(TypeChecker* checker, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  Ast* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_parserLocalElement(checker, ast);
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
  Ast* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_parserState(checker, ast);
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
  Ast* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_parserStatement(checker, ast);
  }
}

static void visit_parserStatement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  }  else assert(0);
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
  PotentialType* tau;

  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(checker, state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
  tau = map_lookup(checker->potype_map, state_expr->stateExpression.expr, 0);
  map_insert(checker->storage, checker->potype_map, state_expr, tau, 0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visit_expressionList(checker, select_expr->selectExpression.expr_list);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  Ast* ast;
  PotentialType* tau, *tau_case;
  int i;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(checker->storage, checker->potype_map, case_list, tau, 0);
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_selectCase(checker, ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(checker->storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    tau_case = map_lookup(checker->potype_map, ast, 0);
    tau->product.members[i] = tau_case;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  PotentialType* tau;

  visit_keysetExpression(checker, select_case->selectCase.keyset_expr);
  visit_name(checker, select_case->selectCase.name, 0);
  tau = map_lookup(checker->potype_map, select_case->selectCase.name, 0);
  map_insert(checker->storage, checker->potype_map, select_case, tau, 0);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  PotentialType* tau;

  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else assert(0);
  tau = map_lookup(checker->potype_map, keyset_expr->keysetExpression.expr, 0);
  map_insert(checker->storage, checker->potype_map, keyset_expr, tau, 0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  PotentialType* tau;

  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list);
  tau = map_lookup(checker->potype_map, tuple_expr->tupleKeysetExpression.expr_list, 0);
  map_insert(checker->storage, checker->potype_map, tuple_expr, tau, 0);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  PotentialType* tau;

  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(checker, simple_expr->simpleKeysetExpression.expr, 0);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  tau->product.count = 1;
  tau->product.members = arena_malloc(checker->storage, tau->product.count*sizeof(PotentialType*));
  tau->product.members[0] = map_lookup(checker->potype_map, simple_expr->simpleKeysetExpression.expr, 0);
  map_insert(checker->storage, checker->potype_map, simple_expr, tau, 0);
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  Ast* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(checker->storage, checker->potype_map, expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_simpleKeysetExpression(checker, ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(checker->storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    tau_expr = map_lookup(checker->potype_map, ast, 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
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
  visit_parameterList(checker, type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(checker, type_decl->controlTypeDeclaration.method_protos);
}

static void visit_controlLocalDeclarations(TypeChecker* checker, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  Ast* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_controlLocalDeclaration(checker, ast);
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
  visit_methodPrototypes(checker, type_decl->externTypeDeclaration.method_protos);
}

static void visit_methodPrototypes(TypeChecker* checker, Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  Ast* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_functionPrototype(checker, ast);
  }
}

static void visit_functionPrototype(TypeChecker* checker, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(checker, func_proto->functionPrototype.return_type);
  }
  visit_parameterList(checker, func_proto->functionPrototype.params);
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  PotentialType* tau;

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
    visit_name(checker, type_ref->typeRef.type, 0);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  tau = map_lookup(checker->potype_map, type_ref->typeRef.type, 0);
  map_insert(checker->storage, checker->potype_map, type_ref, tau, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  PotentialType* tau;

  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
  tau = map_lookup(checker->potype_map, type_decl->tupleType.type_args, 0);
  map_insert(checker->storage, checker->potype_map, type_decl, tau, 0);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  PotentialType* tau;

  visit_typeRef(checker, type_decl->headerStackType.type);
  visit_expression(checker, type_decl->headerStackType.stack_expr, 0);
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, type_decl, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, type_decl, 0), 0, 0);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, bool_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, bool_type, 0), 0, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  PotentialType* tau;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, int_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, int_type, 0), 0, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  PotentialType* tau;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, bit_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, bit_type, 0), 0, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  PotentialType* tau;

  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, varbit_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, varbit_type, 0), 0, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, str_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, str_type, 0), 0, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, void_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, void_type, 0), 0, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, error_type, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, error_type, 0), 0, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, type_size, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, type_size, 0), 0, 0);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visit_name(checker, type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visit_dontcare(checker, type_arg->typeArg.arg);
  } else assert(0);
}

static void visit_typeArgumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AST_typeArgumentList);
  Ast* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_typeArg(checker, ast);
  }
}

static void visit_typeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl);
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
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  visit_structFieldList(checker, header_decl->headerTypeDeclaration.fields);
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  visit_structFieldList(checker, union_decl->headerUnionDeclaration.fields);
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  visit_structFieldList(checker, struct_decl->structTypeDeclaration.fields);
}

static void visit_structFieldList(TypeChecker* checker, Ast* fields)
{
  assert(fields->kind == AST_structFieldList);
  Ast* ast;

  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_structField(checker, ast);
  }
}

static void visit_structField(TypeChecker* checker, Ast* field)
{
  assert(field->kind == AST_structField);
  visit_typeRef(checker, field->structField.type);
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields);
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visit_identifierList(checker, error_decl->errorDeclaration.fields);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visit_identifierList(checker, match_decl->matchKindDeclaration.fields);
}

static void visit_identifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
}

static void visit_specifiedIdentifierList(TypeChecker* checker, Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  Ast* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_specifiedIdentifier(checker, ast);
  }
}

static void visit_specifiedIdentifier(TypeChecker* checker, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(checker, ident->specifiedIdentifier.init_expr, 0);
  }
}

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr, 0);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  PotentialType* tau, *args_tau;

  visit_argumentList(checker, func_call->functionCall.args);
  args_tau = map_lookup(checker->potype_map, func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr, args_tau);
  } else assert(0);
  tau = map_lookup(checker->potype_map, func_call->functionCall.lhs_expr, 0);
  map_insert(checker->storage, checker->potype_map, func_call, tau, 0);
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr, 0);
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

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(checker, applic_stmt->directApplication.name, 0);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(checker, applic_stmt->directApplication.args);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visit_functionCall(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visit_directApplication(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt);
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
  Ast* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_statementOrDeclaration(checker, ast);
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
  Ast* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_switchCase(checker, ast);
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
  visit_methodPrototypes(checker, table_decl->tableDeclaration.method_protos);
}

static void visit_tablePropertyList(TypeChecker* checker, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  Ast* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_tableProperty(checker, ast);
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
  Ast* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_keyElement(checker, ast);
  }
}

static void visit_keyElement(TypeChecker* checker, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visit_expression(checker, element->keyElement.expr, 0);
  visit_name(checker, element->keyElement.match, 0);
}

static void visit_actionsProperty(TypeChecker* checker, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visit_actionList(checker, actions_prop->actionsProperty.action_list);
}

static void visit_actionList(TypeChecker* checker, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  Ast* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_actionRef(checker, ast);
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(checker, action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args);
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
  Ast* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_entry(checker, ast);
  }
}

static void visit_entry(TypeChecker* checker, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visit_keysetExpression(checker, entry->entry.keyset);
  visit_actionRef(checker, entry->entry.action);
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
  visit_parameterList(checker, action_decl->actionDeclaration.params);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  PotentialType* tau;
  Type* var_ty;

  visit_typeRef(checker, var_decl->variableDeclaration.type);
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, var_decl, tau, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr, 0);
  }
  var_ty = map_lookup(checker->type_env, var_decl, 0);
  map_insert(checker->storage, &tau->set.members, actual_type(var_ty), 0, 1);
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AST_argumentList);
  Ast* ast;
  PotentialType* tau, *tau_arg;
  int i;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(checker->storage, checker->potype_map, args, tau, 0);
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_argument(checker, ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(checker->storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    tau_arg = map_lookup(checker->potype_map, ast, 0);
    tau->product.members[i] = tau_arg;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_argument(TypeChecker* checker, Ast* arg)
{
  assert(arg->kind == AST_argument);
  PotentialType* tau;

  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(checker, arg->argument.arg, 0);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(checker, arg->argument.arg);
  } else assert(0);
  tau = map_lookup(checker->potype_map, arg->argument.arg, 0);
  map_insert(checker->storage, checker->potype_map, arg, tau, 0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(checker->storage, checker->potype_map, expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    visit_expression(checker, ast, 0);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(checker->storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->tree.right_sibling) {
    tau_expr = map_lookup(checker->potype_map, ast, 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, PotentialType* potential_args)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  PotentialType* tau;

  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(checker, lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  tau = map_lookup(checker->potype_map, lvalue_expr->lvalueExpression.expr, 0);
  map_insert(checker->storage, checker->potype_map, lvalue_expr, tau, 0);
}

static void visit_expression(TypeChecker* checker, Ast* expr, PotentialType* potential_args)
{
  assert(expr->kind == AST_expression);
  PotentialType* tau;

  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
  tau = map_lookup(checker->potype_map, expr->expression.expr, 0);
  map_insert(checker->storage, checker->potype_map, expr, tau, 0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  PotentialType* tau;

  visit_typeRef(checker, cast_expr->castExpression.type);
  visit_expression(checker, cast_expr->castExpression.expr, 0);
  tau = map_lookup(checker->potype_map, cast_expr->castExpression.type, 0);
  map_insert(checker->storage, checker->potype_map, cast_expr, tau, 0);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand, 0);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  PotentialType* tau;
  PotentialType potential_args = {0};
  PotentialType* member_args[2] = {0, 0};
  Type* ty;
  NameDeclaration* name_decl;

  potential_args.product.count = 2;
  potential_args.product.members = member_args;
  visit_expression(checker, binary_expr->binaryExpression.left_operand, 0);
  visit_expression(checker, binary_expr->binaryExpression.right_operand, 0);
  potential_args.product.members[0] = map_lookup(checker->potype_map, binary_expr->binaryExpression.left_operand, 0);
  potential_args.product.members[1] = map_lookup(checker->potype_map, binary_expr->binaryExpression.right_operand, 0);
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, binary_expr, tau, 0);
  name_decl = builtin_lookup(checker->root_scope, binary_expr->binaryExpression.strname, NAMESPACE_TYPE);
  for (; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = name_decl->type;
    if (match_params(checker, &potential_args, ty->function.params)) {
      map_insert(checker->storage, &tau->set.members, ty, 0, 0);
    }
  }
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector, PotentialType* potential_args)
{
  assert(selector->kind == AST_memberSelector);
  Ast* name;
  PotentialType* tau, *tau_lhs;
  MapEntry* m;
  Type* lhs_ty;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, selector, tau, 0);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  name = selector->memberSelector.name;
  tau_lhs = map_lookup(checker->potype_map, selector->memberSelector.lhs_expr, 0);
  for (m = tau_lhs->set.members.first; m != 0; m = m->next) {
    lhs_ty = effective_type(m->key);
    if (lhs_ty->ty_former == TYPE_EXTERN) {
      collect_matching_member(checker, tau, lhs_ty->extern_.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_ENUM ||
               lhs_ty->ty_former == TYPE_MATCH_KIND || lhs_ty->ty_former == TYPE_ERROR) {
      collect_matching_member(checker, tau, lhs_ty->enum_.fields, name->name.strname, 0);
    } else if (lhs_ty->ty_former == TYPE_STRUCT || lhs_ty->ty_former == TYPE_HEADER ||
               lhs_ty->ty_former == TYPE_HEADER_UNION) {
      collect_matching_member(checker, tau, lhs_ty->struct_.fields, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_HEADER_STACK) {
      /* TODO */
    } else if (lhs_ty->ty_former == TYPE_TABLE) {
      collect_matching_member(checker, tau, lhs_ty->table.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_PARSER || lhs_ty->ty_former == TYPE_CONTROL) {
      collect_matching_member(checker, tau, lhs_ty->parser.methods, name->name.strname, potential_args);
    }
  }
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  PotentialType* tau;

  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
  tau = map_lookup(checker->potype_map, subscript->arraySubscript.lhs_expr, 0);
  map_insert(checker->storage, checker->potype_map, subscript, tau, 0);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  PotentialType* tau;

  visit_expression(checker, index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index, 0);
  }
  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, index_expr, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, index_expr, 0), 0, 0);
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, bool_literal, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, bool_literal, 0), 0, 0);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, int_literal, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, int_literal, 0), 0, 0);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, str_literal, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, str_literal, 0), 0, 0);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AST_default);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, default_, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, default_, 0), 0, 0);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  PotentialType* tau;

  tau = arena_malloc(checker->storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(checker->storage, checker->potype_map, dontcare, tau, 0);
  map_insert(checker->storage, &tau->set.members, map_lookup(checker->type_env, dontcare, 0), 0, 0);
}
