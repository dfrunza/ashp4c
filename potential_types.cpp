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

bool TypeChecker::match_type(PotentialType* potential_types, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = potential_types->set.members.first; m != 0; m = m->next) {
    ty = ((Type*)m->key)->effective_type();
    if (type_equiv(this, ty, required_ty->actual_type())) {
      i += 1;
    }
  }
  return (i == 1);
}

bool TypeChecker::match_params(PotentialType* potential_args, Type* params_ty)
{
  int i;

  if (params_ty->product.count != potential_args->product.count) return 0;
  for (i = 0; i < params_ty->product.count; i++) {
    if (!match_type(potential_args->product.members[i],
                    params_ty->product.members[i])) break;
  }
  return (i == params_ty->product.count);
}


void TypeChecker::collect_matching_member(PotentialType* tau, Type* product_ty,
    char* strname, PotentialType* potential_args)
{
  Type* member_ty;

  for (int i = 0; i < product_ty->product.count; i++) {
    member_ty = product_ty->product.members[i];
    if (cstr_match(member_ty->strname, strname)) {
      if (member_ty->ty_former == TypeEnum::FUNCTION) {
        if (match_params(potential_args, member_ty->function.params)) {
          tau->set.members.insert(member_ty, 0, 1);
        }
      } else {
        tau->set.members.insert(member_ty, 0, 1);
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
  checker->potype_map = (Map*)checker->storage->malloc(sizeof(Map));
  checker->potype_map->storage = checker->storage;
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

static void visit_name(TypeChecker* checker, Ast* name, PotentialType* potential_args)
{
  assert(name->kind == AstEnum::name);
  Scope* scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  PotentialType* tau;
  Type* ty, *ctors_ty;
  static Array* name_ty;

  if (!name_ty) name_ty = Array::create(checker->storage, sizeof(Type*), 1);
  name_ty->elem_count = 0;
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(name, tau, 0);
  scope = (Scope*)checker->scope_map->lookup(name, 0);
  name_entry = scope->lookup(name->name.strname, NameSpace::VAR | NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::VAR >> 1];
  if (name_decl) {
    ty = (Type*)checker->type_env->lookup(name_decl->ast, 0);
    *(Type**)name_ty->append(sizeof(Type*)) = ty->actual_type();
    assert(!name_decl->next_in_scope);
  }
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  for(; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = (Type*)checker->type_env->lookup(name_decl->ast, 0);
    *(Type**)name_ty->append(sizeof(Type*)) = ty->actual_type();
  }
  for (int i = 0; i < name_ty->elem_count; i++) {
    ty = *(Type**)name_ty->get(i, sizeof(Type*));
    if (potential_args) {
      if (ty->ty_former == TypeEnum::FUNCTION) {
        if (checker->match_params(potential_args, ty->function.params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::PARSER) {
        if (checker->match_params(potential_args, ty->parser.ctor_params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::CONTROL) {
        if (checker->match_params(potential_args, ty->control.ctor_params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::EXTERN) {
        ctors_ty = ty->extern_.ctors;
        for (int j = 0; j < ctors_ty->product.count; j++) {
          ty = ctors_ty->product.members[j];
          if (checker->match_params(potential_args, ty->function.params)) {
            tau->set.members.insert(ty, 0, 0);
          }
        }
      } else assert(0);
    } else {
      tau->set.members.insert(ty, 0, 0);
    }
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
  visit_typeRef(checker, param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(checker, param->parameter.init_expr, 0);
  }
}

static void visit_packageTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  visit_parameterList(checker, type_decl->packageTypeDeclaration.params);
}

static void visit_instantiation(TypeChecker* checker, Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  PotentialType* tau;
  Type* inst_ty;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(inst, tau, 0);
  visit_typeRef(checker, inst->instantiation.type);
  visit_argumentList(checker, inst->instantiation.args);
  inst_ty = (Type*)checker->type_env->lookup(inst, 0);
  tau->set.members.insert(inst_ty->actual_type(), 0, 1);
}

/** PARSER **/

static void visit_parserDeclaration(TypeChecker* checker, Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_typeDeclaration(checker, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(checker, parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(checker, parser_decl->parserDeclaration.local_elements);
  visit_parserStates(checker, parser_decl->parserDeclaration.states);
}

static void visit_parserTypeDeclaration(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  visit_parameterList(checker, type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(checker, type_decl->parserTypeDeclaration.method_protos);
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
    visit_functionCall(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(checker, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  }  else assert(0);
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
  PotentialType* tau;

  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(checker, state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(checker, state_expr->stateExpression.expr);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(state_expr->stateExpression.expr, 0);
  checker->potype_map->insert(state_expr, tau, 0);
}

static void visit_selectExpression(TypeChecker* checker, Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(checker, select_expr->selectExpression.expr_list);
  visit_selectCaseList(checker, select_expr->selectExpression.case_list);
}

static void visit_selectCaseList(TypeChecker* checker, Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  AstTree* ast;
  PotentialType* tau, *tau_case;
  int i;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(case_list, tau, 0);
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(checker, container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)checker->storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_case = (PotentialType*)checker->potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_case;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_selectCase(TypeChecker* checker, Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  PotentialType* tau;

  visit_keysetExpression(checker, select_case->selectCase.keyset_expr);
  visit_name(checker, select_case->selectCase.name, 0);
  tau = (PotentialType*)checker->potype_map->lookup(select_case->selectCase.name, 0);
  checker->potype_map->insert(select_case, tau, 0);
}

static void visit_keysetExpression(TypeChecker* checker, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  PotentialType* tau;

  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(checker, keyset_expr->keysetExpression.expr);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(keyset_expr->keysetExpression.expr, 0);
  checker->potype_map->insert(keyset_expr, tau, 0);
}

static void visit_tupleKeysetExpression(TypeChecker* checker, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  PotentialType* tau;

  visit_simpleExpressionList(checker, tuple_expr->tupleKeysetExpression.expr_list);
  tau = (PotentialType*)checker->potype_map->lookup(tuple_expr->tupleKeysetExpression.expr_list, 0);
  checker->potype_map->insert(tuple_expr, tau, 0);
}

static void visit_simpleKeysetExpression(TypeChecker* checker, Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  PotentialType* tau;

  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(checker, simple_expr->simpleKeysetExpression.expr, 0);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(checker, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(checker, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = checker->storage;
  tau->product.count = 1;
  tau->product.members = (PotentialType**)checker->storage->malloc(tau->product.count * sizeof(PotentialType*));
  tau->product.members[0] = (PotentialType*)checker->potype_map->lookup(simple_expr->simpleKeysetExpression.expr, 0);
  checker->potype_map->insert(simple_expr, tau, 0);
}

static void visit_simpleExpressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  AstTree* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(checker, container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)checker->storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = (PotentialType*)checker->potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
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
  visit_parameterList(checker, type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(checker, type_decl->controlTypeDeclaration.method_protos);
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
  visit_methodPrototypes(checker, type_decl->externTypeDeclaration.method_protos);
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
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(checker, func_proto->functionPrototype.return_type);
  }
  visit_parameterList(checker, func_proto->functionPrototype.params);
}

/** TYPES **/

static void visit_typeRef(TypeChecker* checker, Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
  PotentialType* tau;

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
    visit_name(checker, type_ref->typeRef.type, 0);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(checker, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(checker, type_ref->typeRef.type);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(type_ref->typeRef.type, 0);
  checker->potype_map->insert(type_ref, tau, 0);
}

static void visit_tupleType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  PotentialType* tau;

  visit_typeArgumentList(checker, type_decl->tupleType.type_args);
  tau = (PotentialType*)checker->potype_map->lookup(type_decl->tupleType.type_args, 0);
  checker->potype_map->insert(type_decl, tau, 0);
}

static void visit_headerStackType(TypeChecker* checker, Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  PotentialType* tau;

  visit_typeRef(checker, type_decl->headerStackType.type);
  visit_expression(checker, type_decl->headerStackType.stack_expr, 0);
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(type_decl, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(type_decl, 0), 0, 0);
}

static void visit_baseTypeBoolean(TypeChecker* checker, Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(bool_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(bool_type, 0), 0, 0);
}

static void visit_baseTypeInteger(TypeChecker* checker, Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  PotentialType* tau;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(checker, int_type->baseTypeInteger.size);
  }
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(int_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(int_type, 0), 0, 0);
}

static void visit_baseTypeBit(TypeChecker* checker, Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  PotentialType* tau;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(checker, bit_type->baseTypeBit.size);
  }
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(bit_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(bit_type, 0), 0, 0);
}

static void visit_baseTypeVarbit(TypeChecker* checker, Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  PotentialType* tau;

  visit_integerTypeSize(checker, varbit_type->baseTypeVarbit.size);
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(varbit_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(varbit_type, 0), 0, 0);
}

static void visit_baseTypeString(TypeChecker* checker, Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(str_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(str_type, 0), 0, 0);
}

static void visit_baseTypeVoid(TypeChecker* checker, Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(void_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(void_type, 0), 0, 0);
}

static void visit_baseTypeError(TypeChecker* checker, Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(error_type, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(error_type, 0), 0, 0);
}

static void visit_integerTypeSize(TypeChecker* checker, Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(type_size, tau, 0);
  tau->set.members.insert(checker->type_env->lookup(type_size, 0), 0, 0);
}

static void visit_realTypeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, type_arg->realTypeArg.arg);
  } else assert(0);
}

static void visit_typeArg(TypeChecker* checker, Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(checker, type_arg->typeArg.arg);
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
    visit_typedefDeclaration(checker, type_decl->typeDeclaration.decl);
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
  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(checker, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

static void visit_headerTypeDeclaration(TypeChecker* checker, Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  visit_structFieldList(checker, header_decl->headerTypeDeclaration.fields);
}

static void visit_headerUnionDeclaration(TypeChecker* checker, Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  visit_structFieldList(checker, union_decl->headerUnionDeclaration.fields);
}

static void visit_structTypeDeclaration(TypeChecker* checker, Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  visit_structFieldList(checker, struct_decl->structTypeDeclaration.fields);
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
  visit_typeRef(checker, field->structField.type);
}

static void visit_enumDeclaration(TypeChecker* checker, Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_specifiedIdentifierList(checker, enum_decl->enumDeclaration.fields);
}

static void visit_errorDeclaration(TypeChecker* checker, Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  visit_identifierList(checker, error_decl->errorDeclaration.fields);
}

static void visit_matchKindDeclaration(TypeChecker* checker, Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  visit_identifierList(checker, match_decl->matchKindDeclaration.fields);
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

static void visit_typedefDeclaration(TypeChecker* checker, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(checker, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(checker, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

static void visit_assignmentStatement(TypeChecker* checker, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  visit_expression(checker, assign_stmt->assignmentStatement.rhs_expr, 0);
}

static void visit_functionCall(TypeChecker* checker, Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  PotentialType* tau, *args_tau;

  visit_argumentList(checker, func_call->functionCall.args);
  args_tau = (PotentialType*)checker->potype_map->lookup(func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, func_call->functionCall.lhs_expr, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, func_call->functionCall.lhs_expr, args_tau);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(func_call->functionCall.lhs_expr, 0);
  checker->potype_map->insert(func_call, tau, 0);
}

static void visit_returnStatement(TypeChecker* checker, Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(checker, return_stmt->returnStatement.expr, 0);
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

static void visit_directApplication(TypeChecker* checker, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(checker, applic_stmt->directApplication.name, 0);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(checker, applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(checker, applic_stmt->directApplication.args);
}

static void visit_statement(TypeChecker* checker, Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(checker, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(checker, stmt->statement.stmt);
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
  visit_methodPrototypes(checker, table_decl->tableDeclaration.method_protos);
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
  visit_name(checker, element->keyElement.match, 0);
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
    visit_actionRef(checker, container_of(ast, Ast, tree));
  }
}

static void visit_actionRef(TypeChecker* checker, Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(checker, action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(checker, action_ref->actionRef.args);
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
  visit_keysetExpression(checker, entry->entry.keyset);
  visit_actionRef(checker, entry->entry.action);
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
  visit_parameterList(checker, action_decl->actionDeclaration.params);
  visit_blockStatement(checker, action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

static void visit_variableDeclaration(TypeChecker* checker, Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  PotentialType* tau;
  Type* var_ty;

  visit_typeRef(checker, var_decl->variableDeclaration.type);
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(var_decl, tau, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(checker, var_decl->variableDeclaration.init_expr, 0);
  }
  var_ty = (Type*)checker->type_env->lookup(var_decl, 0);
  tau->set.members.insert(var_ty->actual_type(), 0, 1);
}

/** EXPRESSIONS **/

static void visit_functionDeclaration(TypeChecker* checker, Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(checker, func_decl->functionDeclaration.proto);
  visit_blockStatement(checker, func_decl->functionDeclaration.stmt);
}

static void visit_argumentList(TypeChecker* checker, Ast* args)
{
  assert(args->kind == AstEnum::argumentList);
  AstTree* ast;
  PotentialType* tau, *tau_arg;
  int i;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(args, tau, 0);
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(checker, container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)checker->storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_arg = (PotentialType*)checker->potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_arg;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_argument(TypeChecker* checker, Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  PotentialType* tau;

  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(checker, arg->argument.arg, 0);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(checker, arg->argument.arg);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(arg->argument.arg, 0);
  checker->potype_map->insert(arg, tau, 0);
}

static void visit_expressionList(TypeChecker* checker, Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  AstTree* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(checker, container_of(ast, Ast, tree), 0);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)checker->storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = (PotentialType*)checker->potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

static void visit_lvalueExpression(TypeChecker* checker, Ast* lvalue_expr, PotentialType* potential_args)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  PotentialType* tau;

  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(checker, lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(lvalue_expr->lvalueExpression.expr, 0);
  checker->potype_map->insert(lvalue_expr, tau, 0);
}

static void visit_expression(TypeChecker* checker, Ast* expr, PotentialType* potential_args)
{
  assert(expr->kind == AstEnum::expression);
  PotentialType* tau;

  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    visit_name(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(checker, expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(checker, expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(checker, expr->expression.expr);
  } else assert(0);
  tau = (PotentialType*)checker->potype_map->lookup(expr->expression.expr, 0);
  checker->potype_map->insert(expr, tau, 0);
}

static void visit_castExpression(TypeChecker* checker, Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  PotentialType* tau;

  visit_typeRef(checker, cast_expr->castExpression.type);
  visit_expression(checker, cast_expr->castExpression.expr, 0);
  tau = (PotentialType*)checker->potype_map->lookup(cast_expr->castExpression.type, 0);
  checker->potype_map->insert(cast_expr, tau, 0);
}

static void visit_unaryExpression(TypeChecker* checker, Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(checker, unary_expr->unaryExpression.operand, 0);
}

static void visit_binaryExpression(TypeChecker* checker, Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  PotentialType* tau;
  PotentialType potential_args = {};
  PotentialType* member_args[2] = {0, 0};
  Type* ty;
  NameDeclaration* name_decl;

  potential_args.product.count = 2;
  potential_args.product.members = member_args;
  visit_expression(checker, binary_expr->binaryExpression.left_operand, 0);
  visit_expression(checker, binary_expr->binaryExpression.right_operand, 0);
  potential_args.product.members[0] = (PotentialType*)checker->potype_map->lookup(binary_expr->binaryExpression.left_operand, 0);
  potential_args.product.members[1] = (PotentialType*)checker->potype_map->lookup(binary_expr->binaryExpression.right_operand, 0);
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(binary_expr, tau, 0);
  name_decl = checker->root_scope->builtin_lookup(binary_expr->binaryExpression.strname, NameSpace::TYPE);
  for (; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = name_decl->type;
    if (checker->match_params(&potential_args, ty->function.params)) {
      tau->set.members.insert(ty, 0, 0);
    }
  }
}

static void visit_memberSelector(TypeChecker* checker, Ast* selector, PotentialType* potential_args)
{
  assert(selector->kind == AstEnum::memberSelector);
  Ast* name;
  PotentialType* tau, *tau_lhs;
  MapEntry* m;
  Type* lhs_ty;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(selector, tau, 0);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  name = selector->memberSelector.name;
  tau_lhs = (PotentialType*)checker->potype_map->lookup(selector->memberSelector.lhs_expr, 0);
  for (m = tau_lhs->set.members.first; m != 0; m = m->next) {
    lhs_ty = ((Type*)m->key)->effective_type();
    if (lhs_ty->ty_former == TypeEnum::EXTERN) {
      checker->collect_matching_member(tau, lhs_ty->extern_.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::ENUM ||
               lhs_ty->ty_former == TypeEnum::MATCH_KIND || lhs_ty->ty_former == TypeEnum::ERROR) {
      checker->collect_matching_member(tau, lhs_ty->enum_.fields, name->name.strname, 0);
    } else if (lhs_ty->ty_former == TypeEnum::STRUCT || lhs_ty->ty_former == TypeEnum::HEADER ||
               lhs_ty->ty_former == TypeEnum::UNION) {
      checker->collect_matching_member(tau, lhs_ty->struct_.fields, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::STACK) {
      /* TODO */
    } else if (lhs_ty->ty_former == TypeEnum::TABLE) {
      checker->collect_matching_member(tau, lhs_ty->table.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::PARSER || lhs_ty->ty_former == TypeEnum::CONTROL) {
      checker->collect_matching_member(tau, lhs_ty->parser.methods, name->name.strname, potential_args);
    }
  }
}

static void visit_arraySubscript(TypeChecker* checker, Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  PotentialType* tau;

  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(checker, subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(checker, subscript->arraySubscript.index_expr);
  tau = (PotentialType*)checker->potype_map->lookup(subscript->arraySubscript.lhs_expr, 0);
  checker->potype_map->insert(subscript, tau, 0);
}

static void visit_indexExpression(TypeChecker* checker, Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  PotentialType* tau;

  visit_expression(checker, index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(checker, index_expr->indexExpression.end_index, 0);
  }
  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(index_expr, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(index_expr, 0), 0, 0);
}

static void visit_booleanLiteral(TypeChecker* checker, Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(bool_literal, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(bool_literal, 0), 0, 0);
}

static void visit_integerLiteral(TypeChecker* checker, Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(int_literal, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(int_literal, 0), 0, 0);
}

static void visit_stringLiteral(TypeChecker* checker, Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(str_literal, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(str_literal, 0), 0, 0);
}

static void visit_default(TypeChecker* checker, Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(default_, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(default_, 0), 0, 0);
}

static void visit_dontcare(TypeChecker* checker, Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
  PotentialType* tau;

  tau = (PotentialType*)checker->storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = checker->storage;
  checker->potype_map->insert(dontcare, tau, 0);
  tau->set.members.insert((Type*)checker->type_env->lookup(dontcare, 0), 0, 0);
}
