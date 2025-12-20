#include <stdio.h>
#include <basic.h>
#include <passes/potential_type.h>

static void DEBUG_print_potential_types(PotentialType* tau)
{
  int i = 0;
  for (MapEntry<Type, void>* m = tau->set.members.first; m != 0; m = m->next) {
    Type* ty = m->key;
    if (ty->strname) {
      printf("  [%d] 0x%x %s %s\n", i, ty, TypeEnum_to_string(ty->ty_former), ty->strname);
    } else {
      printf("  [%d] 0x%x %s\n", i, ty, TypeEnum_to_string(ty->ty_former));
    }
    i += 1;
  }
}

void PotentialTypePass::do_pass()
{
  potype_map = storage->allocate<Map<Ast, PotentialType>>();
  potype_map->storage = storage;
  visit_p4program(p4program);
}

/** PROGRAM **/

void PotentialTypePass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

void PotentialTypePass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  TreeIterator<Ast> it(&decl_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_declaration(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_declaration(Ast* decl)
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

void PotentialTypePass::visit_name(Ast* name, PotentialType* potential_args)
{
  assert(name->kind == AstEnum::name);
  static Array<Type*>* name_ty;

  if (!name_ty) name_ty = Array<Type*>::create(storage, 1);
  name_ty->elem_count = 0;
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(name, tau, 0);
  Scope* scope = scope_map->lookup(name, 0);
  NameEntry* name_entry = scope->lookup(name->name.strname, NameSpace::VAR | NameSpace::TYPE);
  NameDeclaration* name_decl = name_entry->get_declarations(NameSpace::VAR);
  if (name_decl) {
    Type* ty = type_env->lookup(name_decl->ast, 0);
    *name_ty->append() = ty->actual_type();
    assert(!name_decl->next_in_scope);
  }
  name_decl = name_entry->get_declarations(NameSpace::TYPE);
  for(; name_decl != 0; name_decl = name_decl->next_in_scope) {
    Type* ty = type_env->lookup(name_decl->ast, 0);
    *name_ty->append() = ty->actual_type();
  }
  for (int i = 0; i < name_ty->elem_count; i++) {
    Type* ty = *name_ty->get(i);
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
        Type* ctors_ty = ty->extern_.ctors;
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

void PotentialTypePass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  TreeIterator<Ast> it(&params->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parameter(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  visit_typeRef(param->parameter.type);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0);
  }
}

void PotentialTypePass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  visit_parameterList(type_decl->packageTypeDeclaration.params);
}

void PotentialTypePass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(inst, tau, 0);
  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  Type* inst_ty = type_env->lookup(inst, 0);
  tau->set.members.insert(inst_ty->actual_type(), 0, 1);
}

/** PARSER **/

void PotentialTypePass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

void PotentialTypePass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  visit_parameterList(type_decl->parserTypeDeclaration.params);
  visit_methodPrototypes(type_decl->parserTypeDeclaration.method_protos);
}

void PotentialTypePass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  TreeIterator<Ast> it(&local_elements->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserLocalElement(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void PotentialTypePass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  TreeIterator<Ast> it(&states->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserState(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
}

void PotentialTypePass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  TreeIterator<Ast> it(&stmts->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserStatement(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_parserStatement(Ast* stmt)
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

void PotentialTypePass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

void PotentialTypePass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void PotentialTypePass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  PotentialType* tau;

  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
  tau = potype_map->lookup(state_expr->stateExpression.expr, 0);
  potype_map->insert(state_expr, tau, 0);
}

void PotentialTypePass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

void PotentialTypePass::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  int i;
  TreeIterator<Ast> it;

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::PRODUCT);
  potype_map->insert(case_list, tau, 0);

  it.begin(&case_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_selectCase(Ast::owner_of(tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = storage->allocate<PotentialType*>(tau->product.count);
  }

  i = 0;
  it.begin(&case_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    PotentialType* tau_case = potype_map->lookup(Ast::owner_of(tree), 0);
    tau->product.members[i] = tau_case;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypePass::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);

  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name, 0);
  PotentialType* tau = potype_map->lookup(select_case->selectCase.name, 0);
  potype_map->insert(select_case, tau, 0);
}

void PotentialTypePass::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);

  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
  PotentialType* tau = potype_map->lookup(keyset_expr->keysetExpression.expr, 0);
  potype_map->insert(keyset_expr, tau, 0);
}

void PotentialTypePass::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);

  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
  PotentialType* tau = potype_map->lookup(tuple_expr->tupleKeysetExpression.expr_list, 0);
  potype_map->insert(tuple_expr, tau, 0);
}

void PotentialTypePass::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);

  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr, 0);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::PRODUCT);
  tau->product.count = 1;
  tau->product.members = storage->allocate<PotentialType*>(tau->product.count);
  tau->product.members[0] = potype_map->lookup(simple_expr->simpleKeysetExpression.expr, 0);
  potype_map->insert(simple_expr, tau, 0);
}

void PotentialTypePass::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  TreeIterator<Ast> it;

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::PRODUCT);
  potype_map->insert(expr_list, tau, 0);

  it.begin(&expr_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_simpleKeysetExpression(Ast::owner_of(tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = storage->allocate<PotentialType*>(tau->product.count);
  }

  int i = 0;
  it.begin(&expr_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    PotentialType* tau_expr = potype_map->lookup(Ast::owner_of(tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

/** CONTROL **/

void PotentialTypePass::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

void PotentialTypePass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  visit_parameterList(type_decl->controlTypeDeclaration.params);
  visit_methodPrototypes(type_decl->controlTypeDeclaration.method_protos);
}

void PotentialTypePass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  TreeIterator<Ast> it(&local_decls->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_controlLocalDeclaration(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_controlLocalDeclaration(Ast* local_decl)
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

void PotentialTypePass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

void PotentialTypePass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
}

void PotentialTypePass::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  TreeIterator<Ast> it(&protos->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_functionPrototype(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_parameterList(func_proto->functionPrototype.params);
}

/** TYPES **/

void PotentialTypePass::visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);

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
  PotentialType* tau = potype_map->lookup(type_ref->typeRef.type, 0);
  potype_map->insert(type_ref, tau, 0);
}

void PotentialTypePass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);

  visit_typeArgumentList(type_decl->tupleType.type_args);
  PotentialType* tau = potype_map->lookup(type_decl->tupleType.type_args, 0);
  potype_map->insert(type_decl, tau, 0);
}

void PotentialTypePass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);

  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr, 0);
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(type_decl, tau, 0);
  tau->set.members.insert(type_env->lookup(type_decl, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(bool_type, tau, 0);
  tau->set.members.insert(type_env->lookup(bool_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(int_type, tau, 0);
  tau->set.members.insert(type_env->lookup(int_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(bit_type, tau, 0);
  tau->set.members.insert(type_env->lookup(bit_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);

  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(varbit_type, tau, 0);
  tau->set.members.insert(type_env->lookup(varbit_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(str_type, tau, 0);
  tau->set.members.insert(type_env->lookup(str_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(void_type, tau, 0);
  tau->set.members.insert(type_env->lookup(void_type, 0), 0, 0);
}

void PotentialTypePass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(error_type, tau, 0);
  tau->set.members.insert(type_env->lookup(error_type, 0), 0, 0);
}

void PotentialTypePass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(type_size, tau, 0);
  tau->set.members.insert(type_env->lookup(type_size, 0), 0, 0);
}

void PotentialTypePass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void PotentialTypePass::visit_typeArg(Ast* type_arg)
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

void PotentialTypePass::visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  TreeIterator<Ast> it(&args->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_typeArg(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_typeDeclaration(Ast* type_decl)
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

void PotentialTypePass::visit_derivedTypeDeclaration(Ast* type_decl)
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

void PotentialTypePass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
}

void PotentialTypePass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
}

void PotentialTypePass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
}

void PotentialTypePass::visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  TreeIterator<Ast> it(&fields->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_structField(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  visit_typeRef(field->structField.type);
}

void PotentialTypePass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

void PotentialTypePass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  visit_identifierList(error_decl->errorDeclaration.fields);
}

void PotentialTypePass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  visit_identifierList(match_decl->matchKindDeclaration.fields);
}

void PotentialTypePass::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
}

void PotentialTypePass::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  TreeIterator<Ast> it(&ident_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_specifiedIdentifier(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr, 0);
  }
}

void PotentialTypePass::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
}

/** STATEMENTS **/

void PotentialTypePass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, 0);
}

void PotentialTypePass::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);

  visit_argumentList(func_call->functionCall.args);
  PotentialType* args_tau = potype_map->lookup(func_call->functionCall.args, 0);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr, args_tau);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, args_tau);
  } else assert(0);
  PotentialType* tau = potype_map->lookup(func_call->functionCall.lhs_expr, 0);
  potype_map->insert(func_call, tau, 0);
}

void PotentialTypePass::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, 0);
  }
}

void PotentialTypePass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void PotentialTypePass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void PotentialTypePass::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(applic_stmt->directApplication.name, 0);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

void PotentialTypePass::visit_statement(Ast* stmt)
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

void PotentialTypePass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void PotentialTypePass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  TreeIterator<Ast> it(&stmt_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_statementOrDeclaration(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr, 0);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void PotentialTypePass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  TreeIterator<Ast> it(&switch_cases->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_switchCase(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void PotentialTypePass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(label->switchLabel.label, 0);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void PotentialTypePass::visit_statementOrDeclaration(Ast* stmt)
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

void PotentialTypePass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  visit_methodPrototypes(table_decl->tableDeclaration.method_protos);
}

void PotentialTypePass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  TreeIterator<Ast> it(&prop_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_tableProperty(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_tableProperty(Ast* table_prop)
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

void PotentialTypePass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void PotentialTypePass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  TreeIterator<Ast> it(&element_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_keyElement(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(element->keyElement.expr, 0);
  visit_name(element->keyElement.match, 0);
}

void PotentialTypePass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void PotentialTypePass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  TreeIterator<Ast> it(&action_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_actionRef(Ast::owner_of(tree));
  }
}

void PotentialTypePass::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

void PotentialTypePass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

void PotentialTypePass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);

  visit_typeRef(var_decl->variableDeclaration.type);
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(var_decl, tau, 0);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0);
  }
  Type* var_ty = type_env->lookup(var_decl, 0);
  tau->set.members.insert(var_ty->actual_type(), 0, 1);
}

/** EXPRESSIONS **/

void PotentialTypePass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

void PotentialTypePass::visit_argumentList(Ast* args)
{
  assert(args->kind == AstEnum::argumentList);
  TreeIterator<Ast> it;

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::PRODUCT);
  potype_map->insert(args, tau, 0);

  it.begin(&args->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_argument(Ast::owner_of(tree));
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = storage->allocate<PotentialType*>(tau->product.count);
  }

  int i = 0;
  it.begin(&args->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    PotentialType* tau_arg = potype_map->lookup(Ast::owner_of(tree), 0);
    tau->product.members[i] = tau_arg;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypePass::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);

  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg, 0);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  PotentialType* tau = potype_map->lookup(arg->argument.arg, 0);
  potype_map->insert(arg, tau, 0);
}

void PotentialTypePass::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  TreeIterator<Ast> it;

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::PRODUCT);
  potype_map->insert(expr_list, tau, 0);

  it.begin(&expr_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_expression(Ast::owner_of(tree), 0);
    tau->product.count += 1;
  }
  if (tau->product.count > 0) {
    tau->product.members = storage->allocate<PotentialType*>(tau->product.count);
  }

  int i = 0;
  it.begin(&expr_list->tree);
  for (Tree<Ast>* tree = it.next();
       tree != 0; tree = it.next()) {
    PotentialType* tau_expr = potype_map->lookup(Ast::owner_of(tree), 0);
    tau->product.members[i] = tau_expr;
    i += 1;
  }
  assert(i == tau->product.count);
}

void PotentialTypePass::visit_lvalueExpression(Ast* lvalue_expr, PotentialType* potential_args)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);

  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, potential_args);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  PotentialType* tau = potype_map->lookup(lvalue_expr->lvalueExpression.expr, 0);
  potype_map->insert(lvalue_expr, tau, 0);
}

void PotentialTypePass::visit_expression(Ast* expr, PotentialType* potential_args)
{
  assert(expr->kind == AstEnum::expression);

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
  PotentialType* tau = potype_map->lookup(expr->expression.expr, 0);
  potype_map->insert(expr, tau, 0);
}

void PotentialTypePass::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);

  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr, 0);
  PotentialType* tau = potype_map->lookup(cast_expr->castExpression.type, 0);
  potype_map->insert(cast_expr, tau, 0);
}

void PotentialTypePass::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, 0);
}

void PotentialTypePass::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  PotentialType* member_args[2] = {0, 0};

  PotentialType potential_args = {};
  potential_args.product.count = 2;
  potential_args.product.members = member_args;
  visit_expression(binary_expr->binaryExpression.left_operand, 0);
  visit_expression(binary_expr->binaryExpression.right_operand, 0);
  potential_args.product.members[0] = potype_map->lookup(binary_expr->binaryExpression.left_operand, 0);
  potential_args.product.members[1] = potype_map->lookup(binary_expr->binaryExpression.right_operand, 0);
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(binary_expr, tau, 0);
  for (NameDeclaration* name_decl = root_scope->lookup_builtin(binary_expr->binaryExpression.strname, NameSpace::TYPE);
       name_decl != 0; name_decl = name_decl->next_in_scope) {
    Type* ty = name_decl->type;
    if (type_checker->match_params(&potential_args, ty->function.params)) {
      tau->set.members.insert(ty, 0, 0);
    }
  }
}

void PotentialTypePass::visit_memberSelector(Ast* selector, PotentialType* potential_args)
{
  assert(selector->kind == AstEnum::memberSelector);
  Type* lhs_ty;

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(selector, tau, 0);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  Ast* name = selector->memberSelector.name;
  PotentialType* tau_lhs = potype_map->lookup(selector->memberSelector.lhs_expr, 0);
  for (MapEntry<Type, void>* m = tau_lhs->set.members.first; m != 0; m = m->next) {
    lhs_ty = m->key->effective_type();
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

void PotentialTypePass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);

  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
  PotentialType* tau = potype_map->lookup(subscript->arraySubscript.lhs_expr, 0);
  potype_map->insert(subscript, tau, 0);
}

void PotentialTypePass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);

  visit_expression(index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, 0);
  }
  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(index_expr, tau, 0);
  tau->set.members.insert(type_env->lookup(index_expr, 0), 0, 0);
}

void PotentialTypePass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(bool_literal, tau, 0);
  tau->set.members.insert(type_env->lookup(bool_literal, 0), 0, 0);
}

void PotentialTypePass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(int_literal, tau, 0);
  tau->set.members.insert(type_env->lookup(int_literal, 0), 0, 0);
}

void PotentialTypePass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(str_literal, tau, 0);
  tau->set.members.insert(type_env->lookup(str_literal, 0), 0, 0);
}

void PotentialTypePass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(default_, tau, 0);
  tau->set.members.insert(type_env->lookup(default_, 0), 0, 0);
}

void PotentialTypePass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);

  PotentialType* tau = PotentialType::create(storage, PotentialTypeEnum::SET);
  potype_map->insert(dontcare, tau, 0);
  tau->set.members.insert(type_env->lookup(dontcare, 0), 0, 0);
}
