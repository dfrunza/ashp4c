#include <stdio.h>
#include "basic.h"
#include "cstring.h"
#include "frontend.h"

bool TypeChecker::match_type(PotentialType* potential_types, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = potential_types->set.members.first; m != 0; m = m->next) {
    ty = ((Type*)m->key)->effective_type();
    if (((TypeChecker*)this)->type_equiv(ty, required_ty->actual_type())) {
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
    if (cstring::match(member_ty->strname, strname)) {
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

void PotentialTypesPass::do_pass()
{
  potype_map = (Map*)storage->malloc(sizeof(Map));
  potype_map->storage = storage;
  visit_p4program(p4program);
}

/** PROGRAM **/

void PotentialTypesPass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

void PotentialTypesPass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_declaration(Ast* decl)
{
  assert(decl->kind == AstEnum::declaration);
  if (decl->declaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::externDeclaration) {
    visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::functionDeclaration) {
    visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserDeclaration) {
    visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlDeclaration) {
    visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::typeDeclaration) {
    visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::errorDeclaration) {
    visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::matchKindDeclaration) {
    visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(decl->declaration.decl);
  } else assert(0);
}

void PotentialTypesPass::visit_name(Ast* name, PotentialType* potential_args)
{
  assert(name->kind == AstEnum::name);
  Scope* scope;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  PotentialType* tau;
  Type* ty, *ctors_ty;
  static Array* name_ty;

  if (!name_ty) name_ty = Array::create(storage, sizeof(Type*), 1);
  name_ty->elem_count = 0;
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(name, tau, 0);
  scope = (Scope*)scope_map->lookup(name, 0);
  name_entry = scope->lookup(name->name.strname, NameSpace::VAR | NameSpace::TYPE);
  name_decl = name_entry->ns[(int)NameSpace::VAR >> 1];
  if (name_decl) {
    ty = (Type*)type_env->lookup(name_decl->ast, 0);
    *(Type**)name_ty->append() = ty->actual_type();
    assert(!name_decl->next_in_scope);
  }
  name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
  for(; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = (Type*)type_env->lookup(name_decl->ast, 0);
    *(Type**)name_ty->append() = ty->actual_type();
  }
  for (int i = 0; i < name_ty->elem_count; i++) {
    ty = *(Type**)name_ty->get(i);
    if (potential_args) {
      if (ty->ty_former == TypeEnum::FUNCTION) {
        if (type_checker->match_params(potential_args, ty->function.params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::PARSER) {
        if (type_checker->match_params(potential_args, ty->parser.ctor_params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::CONTROL) {
        if (type_checker->match_params(potential_args, ty->control.ctor_params)) {
          tau->set.members.insert(ty, 0, 0);
        }
      } else if (ty->ty_former == TypeEnum::EXTERN) {
        ctors_ty = ty->extern_.ctors;
        for (int j = 0; j < ctors_ty->product.count; j++) {
          ty = ctors_ty->product.members[j];
          if (type_checker->match_params(potential_args, ty->function.params)) {
            tau->set.members.insert(ty, 0, 0);
          }
        }
      } else assert(0);
    } else {
      tau->set.members.insert(ty, 0, 0);
    }
  }
}

void PotentialTypesPass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  visit_typeRef(param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0);
  }
}

void PotentialTypesPass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  visit_parameterList(type_decl->packageTypeDeclaration.params);
}

void PotentialTypesPass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  PotentialType* tau;
  Type* inst_ty;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(inst, tau, 0);
  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  inst_ty = (Type*)type_env->lookup(inst, 0);
  tau->set.members.insert(inst_ty->actual_type(), 0, 1);
}

/** PARSER **/

void PotentialTypesPass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

void PotentialTypesPass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  visit_parameterList(type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos);
}

void PotentialTypesPass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void PotentialTypesPass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
}

void PotentialTypesPass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  }  else assert(0);
}

void PotentialTypesPass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

void PotentialTypesPass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void PotentialTypesPass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  PotentialType* tau;

  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(state_expr->stateExpression.expr, 0);
  potype_map->insert(state_expr, tau, 0);
}

void PotentialTypesPass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

void PotentialTypesPass::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  AstTree* ast;
  PotentialType* tau, *tau_case;
  int i;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = storage;
  potype_map->insert(case_list, tau, 0);
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_case = (PotentialType*)potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_case;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypesPass::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  PotentialType* tau;

  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name, 0);
  tau = (PotentialType*)potype_map->lookup(select_case->selectCase.name, 0);
  potype_map->insert(select_case, tau, 0);
}

void PotentialTypesPass::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  PotentialType* tau;

  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(keyset_expr->keysetExpression.expr, 0);
  potype_map->insert(keyset_expr, tau, 0);
}

void PotentialTypesPass::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  PotentialType* tau;

  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
  tau = (PotentialType*)potype_map->lookup(tuple_expr->tupleKeysetExpression.expr_list, 0);
  potype_map->insert(tuple_expr, tau, 0);
}

void PotentialTypesPass::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  PotentialType* tau;

  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr, 0);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = storage;
  tau->product.count = 1;
  tau->product.members = (PotentialType**)storage->malloc(tau->product.count * sizeof(PotentialType*));
  tau->product.members[0] = (PotentialType*)potype_map->lookup(simple_expr->simpleKeysetExpression.expr, 0);
  potype_map->insert(simple_expr, tau, 0);
}

void PotentialTypesPass::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  AstTree* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = storage;
  potype_map->insert(expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = (PotentialType*)potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

/** CONTROL **/

void PotentialTypesPass::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

void PotentialTypesPass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  visit_parameterList(type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(type_decl->controlTypeDeclaration.method_protos);
}

void PotentialTypesPass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AstEnum::controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::actionDeclaration) {
    visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::tableDeclaration) {
    visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::instantiation) {
    visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
}

/** EXTERN **/

void PotentialTypesPass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

void PotentialTypesPass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
}

void PotentialTypesPass::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
}

/** TYPES **/

void PotentialTypesPass::visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
  PotentialType* tau;

  if (type_ref->typeRef.type->kind == AstEnum::baseTypeBoolean) {
    visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeInteger) {
    visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeBit) {
    visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVarbit) {
    visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeString) {
    visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVoid) {
    visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeError) {
    visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::name) {
    visit_name(type_ref->typeRef.type, 0);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(type_ref->typeRef.type, 0);
  potype_map->insert(type_ref, tau, 0);
}

void PotentialTypesPass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  PotentialType* tau;

  visit_typeArgumentList(type_decl->tupleType.type_args);
  tau = (PotentialType*)potype_map->lookup(type_decl->tupleType.type_args, 0);
  potype_map->insert(type_decl, tau, 0);
}

void PotentialTypesPass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  PotentialType* tau;

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr, 0);
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(type_decl, tau, 0);
  tau->set.members.insert(type_env->lookup(type_decl, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(bool_type, tau, 0);
  tau->set.members.insert(type_env->lookup(bool_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  PotentialType* tau;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(int_type, tau, 0);
  tau->set.members.insert(type_env->lookup(int_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  PotentialType* tau;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(bit_type, tau, 0);
  tau->set.members.insert(type_env->lookup(bit_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  PotentialType* tau;

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(varbit_type, tau, 0);
  tau->set.members.insert(type_env->lookup(varbit_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(str_type, tau, 0);
  tau->set.members.insert(type_env->lookup(str_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(void_type, tau, 0);
  tau->set.members.insert(type_env->lookup(void_type, 0), 0, 0);
}

void PotentialTypesPass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(error_type, tau, 0);
  tau->set.members.insert(type_env->lookup(error_type, 0), 0, 0);
}

void PotentialTypesPass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(type_size, tau, 0);
  tau->set.members.insert(type_env->lookup(type_size, 0), 0, 0);
}

void PotentialTypesPass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void PotentialTypesPass::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

void PotentialTypesPass::visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  AstTree* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
}

void PotentialTypesPass::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
}

void PotentialTypesPass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
}

void PotentialTypesPass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
}

void PotentialTypesPass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
}

void PotentialTypesPass::visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  AstTree* ast;

  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  visit_typeRef(field->structField.type);
}

void PotentialTypesPass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

void PotentialTypesPass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  visit_identifierList(error_decl->errorDeclaration.fields);
}

void PotentialTypesPass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  visit_identifierList(match_decl->matchKindDeclaration.fields);
}

void PotentialTypesPass::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
}

void PotentialTypesPass::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr, 0);
  }
}

void PotentialTypesPass::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

void PotentialTypesPass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, 0);
}

void PotentialTypesPass::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  PotentialType* tau, *args_tau;

  visit_argumentList(func_call->functionCall.args);
  args_tau = (PotentialType*)potype_map->lookup(func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, args_tau);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(func_call->functionCall.lhs_expr, 0);
  potype_map->insert(func_call, tau, 0);
}

void PotentialTypesPass::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, 0);
  }
}

void PotentialTypesPass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void PotentialTypesPass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void PotentialTypesPass::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(applic_stmt->directApplication.name, 0);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

void PotentialTypesPass::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

void PotentialTypesPass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void PotentialTypesPass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr, 0);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void PotentialTypesPass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void PotentialTypesPass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(label->switchLabel.label, 0);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void PotentialTypesPass::visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::statement) {
    visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::instantiation) {
    visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
}

/** TABLES **/

void PotentialTypesPass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  visit_methodPrototypes(table_decl->tableDeclaration.method_protos);
}

void PotentialTypesPass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    visit_actionsProperty(table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AstEnum::entriesProperty) {
    visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::simpleProperty) {
    visit_simpleProperty(table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
}

void PotentialTypesPass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void PotentialTypesPass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(element->keyElement.expr, 0);
  visit_name(element->keyElement.match, 0);
}

void PotentialTypesPass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void PotentialTypesPass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

#if 0
void PotentialTypesPass::visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  visit_entriesList(entries_prop->entriesProperty.entries_list);
}

void PotentialTypesPass::visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(container_of(ast, Ast, tree));
  }
}

void PotentialTypesPass::visit_entry(Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  visit_keysetExpression(entry->entry.keyset);
  visit_actionRef(entry->entry.action);
}

void PotentialTypesPass::visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  visit_expression(simple_prop->simpleProperty.init_expr, 0);
}
#endif

void PotentialTypesPass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

void PotentialTypesPass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  PotentialType* tau;
  Type* var_ty;

  visit_typeRef(var_decl->variableDeclaration.type);
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(var_decl, tau, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0);
  }
  var_ty = (Type*)type_env->lookup(var_decl, 0);
  tau->set.members.insert(var_ty->actual_type(), 0, 1);
}

/** EXPRESSIONS **/

void PotentialTypesPass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

void PotentialTypesPass::visit_argumentList(Ast* args)
{
  assert(args->kind == AstEnum::argumentList);
  AstTree* ast;
  PotentialType* tau, *tau_arg;
  int i;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = storage;
  potype_map->insert(args, tau, 0);
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(container_of(ast, Ast, tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_arg = (PotentialType*)potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_arg;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypesPass::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  PotentialType* tau;

  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg, 0);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(arg->argument.arg, 0);
  potype_map->insert(arg, tau, 0);
}

void PotentialTypesPass::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  AstTree* ast;
  PotentialType* tau, *tau_expr;
  int i;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::PRODUCT;
  tau->set.members.storage = storage;
  potype_map->insert(expr_list, tau, 0);
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(container_of(ast, Ast, tree), 0);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = (PotentialType**)storage->malloc(tau->product.count * sizeof(PotentialType*));
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    tau_expr = (PotentialType*)potype_map->lookup(container_of(ast, Ast, tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypesPass::visit_lvalueExpression(Ast* lvalue_expr, PotentialType* potential_args)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  PotentialType* tau;

  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(lvalue_expr->lvalueExpression.expr, 0);
  potype_map->insert(lvalue_expr, tau, 0);
}

void PotentialTypesPass::visit_expression(Ast* expr, PotentialType* potential_args)
{
  assert(expr->kind == AstEnum::expression);
  PotentialType* tau;

  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    visit_name(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(expr->expression.expr, potential_args);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  tau = (PotentialType*)potype_map->lookup(expr->expression.expr, 0);
  potype_map->insert(expr, tau, 0);
}

void PotentialTypesPass::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  PotentialType* tau;

  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr, 0);
  tau = (PotentialType*)potype_map->lookup(cast_expr->castExpression.type, 0);
  potype_map->insert(cast_expr, tau, 0);
}

void PotentialTypesPass::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, 0);
}

void PotentialTypesPass::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  PotentialType* tau;
  PotentialType potential_args = {};
  PotentialType* member_args[2] = {0, 0};
  Type* ty;
  NameDeclaration* name_decl;

  potential_args.product.count = 2;
  potential_args.product.members = member_args;
  visit_expression(binary_expr->binaryExpression.left_operand, 0);
  visit_expression(binary_expr->binaryExpression.right_operand, 0);
  potential_args.product.members[0] = (PotentialType*)potype_map->lookup(binary_expr->binaryExpression.left_operand, 0);
  potential_args.product.members[1] = (PotentialType*)potype_map->lookup(binary_expr->binaryExpression.right_operand, 0);
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(binary_expr, tau, 0);
  name_decl = root_scope->builtin_lookup(binary_expr->binaryExpression.strname, NameSpace::TYPE);
  for (; name_decl != 0; name_decl = name_decl->next_in_scope) {
    ty = name_decl->type;
    if (type_checker->match_params(&potential_args, ty->function.params)) {
      tau->set.members.insert(ty, 0, 0);
    }
  }
}

void PotentialTypesPass::visit_memberSelector(Ast* selector, PotentialType* potential_args)
{
  assert(selector->kind == AstEnum::memberSelector);
  Ast* name;
  PotentialType* tau, *tau_lhs;
  MapEntry* m;
  Type* lhs_ty;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(selector, tau, 0);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  name = selector->memberSelector.name;
  tau_lhs = (PotentialType*)potype_map->lookup(selector->memberSelector.lhs_expr, 0);
  for (m = tau_lhs->set.members.first; m != 0; m = m->next) {
    lhs_ty = ((Type*)m->key)->effective_type();
    if (lhs_ty->ty_former == TypeEnum::EXTERN) {
      type_checker->collect_matching_member(tau, lhs_ty->extern_.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::ENUM ||
               lhs_ty->ty_former == TypeEnum::MATCH_KIND || lhs_ty->ty_former == TypeEnum::ERROR) {
      type_checker->collect_matching_member(tau, lhs_ty->enum_.fields, name->name.strname, 0);
    } else if (lhs_ty->ty_former == TypeEnum::STRUCT || lhs_ty->ty_former == TypeEnum::HEADER ||
               lhs_ty->ty_former == TypeEnum::UNION) {
      type_checker->collect_matching_member(tau, lhs_ty->struct_.fields, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::STACK) {
      /* TODO */
    } else if (lhs_ty->ty_former == TypeEnum::TABLE) {
      type_checker->collect_matching_member(tau, lhs_ty->table.methods, name->name.strname, potential_args);
    } else if (lhs_ty->ty_former == TypeEnum::PARSER || lhs_ty->ty_former == TypeEnum::CONTROL) {
      type_checker->collect_matching_member(tau, lhs_ty->parser.methods, name->name.strname, potential_args);
    }
  }
}

void PotentialTypesPass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  PotentialType* tau;

  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
  tau = (PotentialType*)potype_map->lookup(subscript->arraySubscript.lhs_expr, 0);
  potype_map->insert(subscript, tau, 0);
}

void PotentialTypesPass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  PotentialType* tau;

  visit_expression(index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, 0);
  }
  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(index_expr, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(index_expr, 0), 0, 0);
}

void PotentialTypesPass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(bool_literal, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(bool_literal, 0), 0, 0);
}

void PotentialTypesPass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(int_literal, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(int_literal, 0), 0, 0);
}

void PotentialTypesPass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(str_literal, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(str_literal, 0), 0, 0);
}

void PotentialTypesPass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(default_, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(default_, 0), 0, 0);
}

void PotentialTypesPass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
  PotentialType* tau;

  tau = (PotentialType*)storage->malloc(sizeof(PotentialType));
  tau->kind = PotentialTypeEnum::SET;
  tau->set.members.storage = storage;
  potype_map->insert(dontcare, tau, 0);
  tau->set.members.insert((Type*)type_env->lookup(dontcare, 0), 0, 0);
}
