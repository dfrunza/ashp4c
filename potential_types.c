#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static char*  source_file;
static Arena* storage;
static Scope* root_scope;
static Map*   type_env, *potype_map;
static Map*   scope_map, *decl_map;

/** PROGRAM **/

static void visit_p4program(Ast* p4program);
static void visit_declarationList(Ast* decl_list);
static void visit_declaration(Ast* decl);
static void visit_name(Ast* name, PotentialType* potential_args);
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
static void visit_argumentList(Ast* args);
static void visit_argument(Ast* arg);
static void visit_expressionList(Ast* expr_list);
static void visit_lvalueExpression(Ast* lvalue_expr, PotentialType* potential_args);
static void visit_expression(Ast* expr, PotentialType* potential_args);
static void visit_castExpression(Ast* cast_expr);
static void visit_unaryExpression(Ast* unary_expr);
static void visit_binaryExpression(Ast* binary_expr);
static void visit_memberSelector(Ast* selector, PotentialType* potential_args);
static void visit_arraySubscript(Ast* subscript);
static void visit_indexExpression(Ast* index_expr);
static void visit_booleanLiteral(Ast* bool_literal);
static void visit_integerLiteral(Ast* int_literal);
static void visit_stringLiteral(Ast* str_literal);
static void visit_default(Ast* default_);
static void visit_dontcare(Ast* dontcare);

bool
match_type(PotentialType* potential_types, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = potential_types->set.members.first; m != 0; m = m->next) {
    ty = effective_type((Type*)m->key);
    if (type_equiv(ty, actual_type(required_ty))) {
      i += 1;
    }
  }
  return (i == 1);
}

bool
match_params(PotentialType* potential_args, Type* params_ty)
{
  int i;

  if (params_ty->product.count != potential_args->product.count) return 0;
  for (i = 0; i < params_ty->product.count; i++) {
    if (!match_type(potential_args->product.members[i],
                    params_ty->product.members[i])) break;
  }
  return (i == params_ty->product.count);
}


static void
collect_matching_member(Arena* storage, PotentialType* tau, Type* product_ty,
    char* strname, PotentialType* potential_args)
{
  Type* member_ty;

  for (int i = 0; i < product_ty->product.count; i++) {
    member_ty = product_ty->product.members[i];
    if (cstr_match(member_ty->strname, strname)) {
      if (member_ty->ty_former == TYPE_FUNCTION) {
        if (match_params(potential_args, member_ty->function.params)) {
          map_insert(storage, &tau->set.members, member_ty, 0, 1);
        }
      } else {
        map_insert(storage, &tau->set.members, member_ty, 0, 1);
      }
    }
  }
}

static void
Debug_print_potential_types(PotentialType* tau)
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

Map*
potential_types(Arena* storage_, char* source_file_, Ast* p4program, Scope* root_scope_,
    Map* scope_map_, Map* decl_map_, Map* type_env_)
{
  storage = storage_;
  source_file = source_file_;
  root_scope = root_scope_;
  scope_map = scope_map_;
  decl_map = decl_map_;
  type_env = type_env_;
  potype_map = arena_malloc(storage, sizeof(Map));

  visit_p4program(p4program);
  return potype_map;
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
visit_name(Ast* name, PotentialType* potential_args)
{
  assert(name->kind == AST_name);
  Scope* scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  PotentialType* tau;
  Type* ty, *ctors_ty;
  static Array* name_ty;

  if (!name_ty) name_ty = array_create(storage, sizeof(Type*), 1);
  name_ty->elem_count = 0;
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, name, tau, 0);
  scope = map_lookup(scope_map, name, 0);
  name_entry = scope_lookup(scope, name->name.strname, NAMESPACE_VAR|NAMESPACE_TYPE);
  name_decl = name_entry->ns[NAMESPACE_VAR >> 1];
  if (name_decl) {
    ty = map_lookup(type_env, name_decl->ast, 0);
    *(Type**)array_append(storage, name_ty, sizeof(Type*)) = actual_type(ty);
    assert(!name_decl->next_in_scope);
  }
  name_decl = name_entry->ns[NAMESPACE_TYPE >> 1];
  for(; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = map_lookup(type_env, name_decl->ast, 0);
    *(Type**)array_append(storage, name_ty, sizeof(Type*)) = actual_type(ty);
  }
  for (int i = 0; i < name_ty->elem_count; i++) {
    ty = *(Type**)array_get(name_ty, i, sizeof(Type*));
    if (potential_args) {
      if (ty->ty_former == TYPE_FUNCTION) {
        if (match_params(potential_args, ty->function.params)) {
          map_insert(storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_PARSER) {
        if (match_params(potential_args, ty->parser.ctor_params)) {
          map_insert(storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_CONTROL) {
        if (match_params(potential_args, ty->control.ctor_params)) {
          map_insert(storage, &tau->set.members, ty, 0, 0);
        }
      } else if (ty->ty_former == TYPE_EXTERN) {
        ctors_ty = ty->extern_.ctors;
        for (int j = 0; j < ctors_ty->product.count; j++) {
          ty = ctors_ty->product.members[j];
          if (match_params(potential_args, ty->function.params)) {
            map_insert(storage, &tau->set.members, ty, 0, 0);
          }
        }
      } else assert(0);
    } else {
      map_insert(storage, &tau->set.members, ty, 0, 0);
    }
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
  visit_typeRef(param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0);
  }
}

static void
visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  visit_parameterList(type_decl->packageTypeDeclaration.params);
}

static void
visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  PotentialType* tau;
  Type* inst_ty;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, inst, tau, 0);
  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  inst_ty = map_lookup(type_env, inst, 0);
  map_insert(storage, &tau->set.members, actual_type(inst_ty), 0, 1);
}

/** PARSER **/

static void
visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

static void
visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  visit_parameterList(type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos);
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
    visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  }  else assert(0);
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
  PotentialType* tau;

  if (state_expr->stateExpression.expr->kind == AST_name) {
    visit_name(state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
  tau = map_lookup(potype_map, state_expr->stateExpression.expr, 0);
  map_insert(storage, potype_map, state_expr, tau, 0);
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
  PotentialType* tau, *tau_case;
  int i;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(storage, potype_map, case_list, tau, 0);
  for (ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = case_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_case = map_lookup(potype_map, ast, 0);
    tau->product.members[i] = tau_case;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void
visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  PotentialType* tau;

  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name, 0);
  tau = map_lookup(potype_map, select_case->selectCase.name, 0);
  map_insert(storage, potype_map, select_case, tau, 0);
}

static void
visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  PotentialType* tau;

  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
  tau = map_lookup(potype_map, keyset_expr->keysetExpression.expr, 0);
  map_insert(storage, potype_map, keyset_expr, tau, 0);
}

static void
visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  PotentialType* tau;

  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
  tau = map_lookup(potype_map, tuple_expr->tupleKeysetExpression.expr_list, 0);
  map_insert(storage, potype_map, tuple_expr, tau, 0);
}

static void
visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  PotentialType* tau;

  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr, 0);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  tau->product.count = 1;
  tau->product.members = arena_malloc(storage, tau->product.count*sizeof(PotentialType*));
  tau->product.members[0] = map_lookup(potype_map, simple_expr->simpleKeysetExpression.expr, 0);
  map_insert(storage, potype_map, simple_expr, tau, 0);
}

static void
visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  Ast* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(storage, potype_map, expr_list, tau, 0);
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = map_lookup(potype_map, ast, 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
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
  visit_parameterList(type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(type_decl->controlTypeDeclaration.method_protos);
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
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
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
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
}

/** TYPES **/

static void
visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  PotentialType* tau;

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
    visit_name(type_ref->typeRef.type, 0);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  tau = map_lookup(potype_map, type_ref->typeRef.type, 0);
  map_insert(storage, potype_map, type_ref, tau, 0);
}

static void
visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  PotentialType* tau;

  visit_typeArgumentList(type_decl->tupleType.type_args);
  tau = map_lookup(potype_map, type_decl->tupleType.type_args, 0);
  map_insert(storage, potype_map, type_decl, tau, 0);
}

static void
visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  PotentialType* tau;

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr, 0);
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, type_decl, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, type_decl, 0), 0, 0);
}

static void
visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, bool_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, bool_type, 0), 0, 0);
}

static void
visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  PotentialType* tau;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, int_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, int_type, 0), 0, 0);
}

static void
visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  PotentialType* tau;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, bit_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, bit_type, 0), 0, 0);
}

static void
visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  PotentialType* tau;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, varbit_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, varbit_type, 0), 0, 0);
}

static void
visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, str_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, str_type, 0), 0, 0);
}

static void
visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, void_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, void_type, 0), 0, 0);
}

static void
visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, error_type, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, error_type, 0), 0, 0);
}

static void
visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, type_size, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, type_size, 0), 0, 0);
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
  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
}

static void
visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
}

static void
visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
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
  visit_typeRef(field->structField.type);
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
visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

static void
visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, 0);
}

static void
visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  PotentialType* tau, *args_tau;

  visit_argumentList(func_call->functionCall.args);
  args_tau = map_lookup(potype_map, func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visit_expression(func_call->functionCall.lhs_expr, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, args_tau);
  } else assert(0);
  tau = map_lookup(potype_map, func_call->functionCall.lhs_expr, 0);
  map_insert(storage, potype_map, func_call, tau, 0);
}

static void
visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, 0);
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
visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visit_name(applic_stmt->directApplication.name, 0);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

static void
visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
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
    visit_blockStatement(stmt->statement.stmt);
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
  visit_methodPrototypes(table_decl->tableDeclaration.method_protos);
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
  visit_name(element->keyElement.match, 0);
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
    visit_actionRef(ast);
  }
}

static void
visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visit_name(action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
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
  visit_keysetExpression(entry->entry.keyset);
  visit_actionRef(entry->entry.action);
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
  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void
visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  PotentialType* tau;
  Type* var_ty;

  visit_typeRef(var_decl->variableDeclaration.type);
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, var_decl, tau, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0);
  }
  var_ty = map_lookup(type_env, var_decl, 0);
  map_insert(storage, &tau->set.members, actual_type(var_ty), 0, 1);
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
visit_argumentList(Ast* args)
{
  assert(args->kind == AST_argumentList);
  Ast* ast;
  PotentialType* tau, *tau_arg;
  int i;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(storage, potype_map, args, tau, 0);
  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(ast);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = args->first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_arg = map_lookup(potype_map, ast, 0);
    tau->product.members[i] = tau_arg;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void
visit_argument(Ast* arg)
{
  assert(arg->kind == AST_argument);
  PotentialType* tau;

  if (arg->argument.arg->kind == AST_expression) {
    visit_expression(arg->argument.arg, 0);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  tau = map_lookup(potype_map, arg->argument.arg, 0);
  map_insert(storage, potype_map, arg, tau, 0);
}

static void
visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  Ast* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_PRODUCT;
  map_insert(storage, potype_map, expr_list, tau, 0);
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(ast, 0);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = arena_malloc(storage, tau->product.count*sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = map_lookup(potype_map, ast, 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void
visit_lvalueExpression(Ast* lvalue_expr, PotentialType* potential_args)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  PotentialType* tau;

  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visit_name(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  tau = map_lookup(potype_map, lvalue_expr->lvalueExpression.expr, 0);
  map_insert(storage, potype_map, lvalue_expr, tau, 0);
}

static void
visit_expression(Ast* expr, PotentialType* potential_args)
{
  assert(expr->kind == AST_expression);
  PotentialType* tau;

  if (expr->expression.expr->kind == AST_expression) {
    visit_expression(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visit_name(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visit_memberSelector(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  tau = map_lookup(potype_map, expr->expression.expr, 0);
  map_insert(storage, potype_map, expr, tau, 0);
}

static void
visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  PotentialType* tau;

  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr, 0);
  tau = map_lookup(potype_map, cast_expr->castExpression.type, 0);
  map_insert(storage, potype_map, cast_expr, tau, 0);
}

static void
visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, 0);
}

static void
visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  PotentialType* tau;
  PotentialType potential_args = {0};
  PotentialType* member_args[2] = {0, 0};
  Type* ty;
  NameDeclaration* name_decl;

  potential_args.product.count = 2;
  potential_args.product.members = member_args;
  visit_expression(binary_expr->binaryExpression.left_operand, 0);
  visit_expression(binary_expr->binaryExpression.right_operand, 0);
  potential_args.product.members[0] = map_lookup(potype_map, binary_expr->binaryExpression.left_operand, 0);
  potential_args.product.members[1] = map_lookup(potype_map, binary_expr->binaryExpression.right_operand, 0);
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, binary_expr, tau, 0);
  name_decl = builtin_lookup(root_scope, binary_expr->binaryExpression.strname, NAMESPACE_TYPE);
  for (; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = name_decl->type;
    if (match_params(&potential_args, ty->function.params)) {
      map_insert(storage, &tau->set.members, ty, 0, 0);
    }
  }
}

static void
visit_memberSelector(Ast* selector, PotentialType* potential_args)
{
  assert(selector->kind == AST_memberSelector);
  Ast* name;
  PotentialType* tau, *tau_lhs;
  MapEntry* m;
  Type* lhs_ty;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, selector, tau, 0);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  name = selector->memberSelector.name;
  tau_lhs = map_lookup(potype_map, selector->memberSelector.lhs_expr, 0);
  for (m = tau_lhs->set.members.first; m != 0; m = m->next) {
    lhs_ty = effective_type(m->key);
    if (lhs_ty->ty_former == TYPE_EXTERN) {
      collect_matching_member(storage, tau, lhs_ty->extern_.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_ENUM ||
               lhs_ty->ty_former == TYPE_MATCH_KIND || lhs_ty->ty_former == TYPE_ERROR) {
      collect_matching_member(storage, tau, lhs_ty->enum_.fields, name->name.strname, 0);
    } else if (lhs_ty->ty_former == TYPE_STRUCT || lhs_ty->ty_former == TYPE_HEADER ||
               lhs_ty->ty_former == TYPE_HEADER_UNION) {
      collect_matching_member(storage, tau, lhs_ty->struct_.fields, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_HEADER_STACK) {
      /* TODO */
    } else if (lhs_ty->ty_former == TYPE_TABLE) {
      collect_matching_member(storage, tau, lhs_ty->table.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TYPE_PARSER || lhs_ty->ty_former == TYPE_CONTROL) {
      collect_matching_member(storage, tau, lhs_ty->parser.methods, name->name.strname, potential_args);
    }
  }
}

static void
visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  PotentialType* tau;

  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
  tau = map_lookup(potype_map, subscript->arraySubscript.lhs_expr, 0);
  map_insert(storage, potype_map, subscript, tau, 0);
}

static void
visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  PotentialType* tau;

  visit_expression(index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, 0);
  }
  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, index_expr, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, index_expr, 0), 0, 0);
}

static void
visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, bool_literal, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, bool_literal, 0), 0, 0);
}

static void
visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, int_literal, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, int_literal, 0), 0, 0);
}

static void
visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, str_literal, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, str_literal, 0), 0, 0);
}

static void
visit_default(Ast* default_)
{
  assert(default_->kind == AST_default);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, default_, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, default_, 0), 0, 0);
}

static void
visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  PotentialType* tau;

  tau = arena_malloc(storage, sizeof(PotentialType));
  tau->kind = POTYPE_SET;
  map_insert(storage, potype_map, dontcare, tau, 0);
  map_insert(storage, &tau->set.members, map_lookup(type_env, dontcare, 0), 0, 0);
}
