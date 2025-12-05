#include <basic.h>
#include <passes/select_type.h>

void SelectTypePass::do_pass()
{
  visit_p4program(p4program);
}

/** PROGRAM **/

void SelectTypePass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

void SelectTypePass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  Tree<Ast>* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_declaration(ast->owner(&Ast::tree));
  }
}

void SelectTypePass::visit_declaration(Ast* decl)
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

void SelectTypePass::visit_name(Ast* name, Type* required_ty)
{
  assert(name->kind == AstEnum::name);
  PotentialType* name_tau;
  Type* name_ty;

  name_tau = potype_map->lookup(name, 0);
  if (name_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, name->line_no, name->column_no);
  }
  if (required_ty) {
    if (!type_checker->match_type(name_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          source_file, name->line_no, name->column_no);
    } else {
      name_ty = name_tau->set.members.first->key;
      type_env->insert(name, name_ty->effective_type(), 0);
    }
  } else {
      name_ty = name_tau->set.members.first->key;
      type_env->insert(name, name_ty->effective_type(), 0);
  }
}

void SelectTypePass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  Tree<Ast>* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parameter(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr, 0);
  }
}

void SelectTypePass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
}

void SelectTypePass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  //visit_typeRef(inst->instantiation.type);
  //visit_argumentList(inst->instantiation.args);
}

/** PARSER **/

void SelectTypePass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

void SelectTypePass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
}

void SelectTypePass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  Tree<Ast>* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserLocalElement(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void SelectTypePass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  Tree<Ast>* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserState(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
}

void SelectTypePass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  Tree<Ast>* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_parserStatement(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->parserStatement.stmt, 0);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else assert(0);
}

void SelectTypePass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

void SelectTypePass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void SelectTypePass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(state_expr->stateExpression.expr, 0);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

void SelectTypePass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  Type* list_ty;

  visit_expressionList(select_expr->selectExpression.expr_list, 0);
  list_ty = type_env->lookup(select_expr->selectExpression.expr_list, 0);
  visit_selectCaseList(select_expr->selectExpression.case_list, list_ty);
}

void SelectTypePass::visit_selectCaseList(Ast* case_list, Type* required_ty)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  Tree<Ast>* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_selectCase(owner_of(ast, &Ast::tree), required_ty);
  }
}

void SelectTypePass::visit_selectCase(Ast* select_case, Type* required_ty)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr, required_ty);
  visit_name(select_case->selectCase.name, 0);
}

void SelectTypePass::visit_keysetExpression(Ast* keyset_expr, Type* required_ty)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  Type* keyset_ty;

  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr, required_ty);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr, required_ty);
  } else assert(0);
  keyset_ty = type_env->lookup(keyset_expr->keysetExpression.expr, 0);
  assert(keyset_ty);
  type_env->insert(keyset_expr, keyset_ty, 0);
}

void SelectTypePass::visit_tupleKeysetExpression(Ast* tuple_expr, Type* required_ty)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  Type* tuple_ty;

  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list, required_ty);
  tuple_ty = type_env->lookup(tuple_expr->tupleKeysetExpression.expr_list, 0);
  type_env->insert(tuple_expr, tuple_ty, 0);
}

void SelectTypePass::visit_simpleKeysetExpression(Ast* simple_expr, Type* required_ty)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  Type* simple_ty;

  if (required_ty->product.count != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, simple_expr->line_no, simple_expr->column_no);
  } else {
    if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
      visit_expression(simple_expr->simpleKeysetExpression.expr, required_ty->product.members[0]);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
      visit_default(simple_expr->simpleKeysetExpression.expr);
    } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
      visit_dontcare(simple_expr->simpleKeysetExpression.expr);
    } else assert(0);
    simple_ty = type_array->append();
    simple_ty->ty_former = TypeEnum::PRODUCT;
    simple_ty->ast = simple_expr;
    simple_ty->product.count = 1;
    simple_ty->product.members = storage->allocate<Type *>(simple_ty->product.count);
    simple_ty->product.members[0] = type_env->lookup(simple_expr->simpleKeysetExpression.expr, 0);
    type_env->insert(simple_expr, simple_ty, 0);
  }
}

void SelectTypePass::visit_simpleExpressionList(Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  Tree<Ast>* ast;
  Type* list_ty;
  int i;

  list_ty = type_array->append();
  list_ty->ty_former = TypeEnum::PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_simpleKeysetExpression(owner_of(ast, &Ast::tree), required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = storage->allocate<Type *>(list_ty->product.count);
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = type_env->lookup(owner_of(ast, &Ast::tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  type_env->insert(expr_list, list_ty, 0);
}

/** CONTROL **/

void SelectTypePass::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

void SelectTypePass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
}

void SelectTypePass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  Tree<Ast>* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_controlLocalDeclaration(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_controlLocalDeclaration(Ast* local_decl)
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

void SelectTypePass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

void SelectTypePass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
}

void SelectTypePass::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  Tree<Ast>* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_functionPrototype(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
}

/** TYPES **/

void SelectTypePass::visit_typeRef(Ast* type_ref, Type* required_ty)
{
  assert(type_ref->kind == AstEnum::typeRef);
  Type* ref_ty;

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
    visit_name(type_ref->typeRef.type, required_ty);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  ref_ty = type_env->lookup(type_ref->typeRef.type, 0);
  if (required_ty) {
    if (!type_checker->type_equiv(ref_ty, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
          source_file, type_ref->line_no, type_ref->column_no);
    }
  }
  type_env->insert(type_ref, ref_ty, 0);
}

void SelectTypePass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  visit_typeArgumentList(type_decl->tupleType.type_args);
}

void SelectTypePass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  Type* index_ty;

  index_ty = root_scope->lookup_builtin("int", NameSpace::TYPE)->type;
  visit_expression(type_decl->headerStackType.stack_expr, index_ty);
}

void SelectTypePass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  Type* bool_ty;

  bool_ty = root_scope->lookup_builtin("bool", NameSpace::TYPE)->type;
  type_env->insert(bool_type, bool_ty, 0);
}

void SelectTypePass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  Type* int_ty;

  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  int_ty = root_scope->lookup_builtin("int", NameSpace::TYPE)->type;
  type_env->insert(int_type, int_ty, 0);
}

void SelectTypePass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  Type* bit_ty;

  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  bit_ty = root_scope->lookup_builtin("bit", NameSpace::TYPE)->type;
  type_env->insert(bit_type, bit_ty, 0);
}

void SelectTypePass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  Type* varbit_ty;

  varbit_ty = root_scope->lookup_builtin("varbit", NameSpace::TYPE)->type;
  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  type_env->insert(varbit_type, varbit_ty, 0);
}

void SelectTypePass::visit_baseTypeString(Ast* string_type)
{
  assert(string_type->kind == AstEnum::baseTypeString);
  Type* string_ty;

  string_ty = root_scope->lookup_builtin("string", NameSpace::TYPE)->type;
  type_env->insert(string_type, string_ty, 0);
}

void SelectTypePass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  Type* void_ty;

  void_ty = root_scope->lookup_builtin("void", NameSpace::TYPE)->type;
  type_env->insert(void_type, void_ty, 0);
}

void SelectTypePass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  Type* error_ty;

  error_ty = root_scope->lookup_builtin("error", NameSpace::TYPE)->type;
  type_env->insert(error_type, error_ty, 0);
}

void SelectTypePass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

void SelectTypePass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg, 0);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void SelectTypePass::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(type_arg->typeArg.arg, 0);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

void SelectTypePass::visit_typeArgumentList(Ast* args)
{
  assert(args->kind == AstEnum::typeArgumentList);
  Tree<Ast>* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_typeArg(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    visit_typedefDeclaration(type_decl->typeDeclaration.decl, 0);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
}

void SelectTypePass::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  Type* decl_ty;

  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  decl_ty = type_env->lookup(type_decl->derivedTypeDeclaration.decl, 0);
  type_env->insert(type_decl, decl_ty, 0);
}

void SelectTypePass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
}

void SelectTypePass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
}

void SelectTypePass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
}

void SelectTypePass::visit_structFieldList(Ast* fields)
{
  assert(fields->kind == AstEnum::structFieldList);
  Tree<Ast>* ast;

  for (ast = fields->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_structField(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
}

void SelectTypePass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

void SelectTypePass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
}

void SelectTypePass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
}

void SelectTypePass::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
}

void SelectTypePass::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  Tree<Ast>* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_specifiedIdentifier(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr, 0);
  }
}

void SelectTypePass::visit_typedefDeclaration(Ast* typedef_decl, Type* required_ty)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  Type* ref_ty;

  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref, required_ty);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  ref_ty = type_env->lookup(typedef_decl->typedefDeclaration.type_ref, 0);
  type_env->insert(typedef_decl, ref_ty, 0);
}

/** STATEMENTS **/

void SelectTypePass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  Type* lhs_ty;

  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr, 0);
  } else assert(0);
  lhs_ty = type_env->lookup(assign_stmt->assignmentStatement.lhs_expr, 0);
  assert(lhs_ty);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr, lhs_ty);
}

void SelectTypePass::visit_functionCall(Ast* func_call, Type* required_ty)
{
  assert(func_call->kind == AstEnum::functionCall);
  PotentialType* func_tau;
  Type* func_ty;

  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr, required_ty);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr, required_ty);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args, 0);
  func_tau = potype_map->lookup(func_call, 0);
  if (func_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, func_call->line_no, func_call->column_no);
  }
  if (required_ty) {
    if (!type_checker->match_type(func_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, func_call->line_no, func_call->column_no);
    } else {
      func_ty = func_tau->set.members.first->key;
      type_env->insert(func_call, func_ty->effective_type(), 0);
    }
  } else {
    func_ty = func_tau->set.members.first->key;
    type_env->insert(func_call, func_ty->effective_type(), 0);
  }
}

void SelectTypePass::visit_returnStatement(Ast* return_stmt, Type* required_ty)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr, required_ty);
  }
}

void SelectTypePass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void SelectTypePass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr, 0);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void SelectTypePass::visit_directApplication(Ast* applic_stmt, Type* required_ty)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  visit_argumentList(applic_stmt->directApplication.args, required_ty);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(applic_stmt->directApplication.name, required_ty);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name, required_ty);
  } else assert(0);
}

void SelectTypePass::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    visit_functionCall(stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    visit_directApplication(stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    visit_returnStatement(stmt->statement.stmt, 0);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
}

void SelectTypePass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void SelectTypePass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  Tree<Ast>* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_statementOrDeclaration(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr, 0);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void SelectTypePass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  Tree<Ast>* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_switchCase(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void SelectTypePass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(label->switchLabel.label, 0);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void SelectTypePass::visit_statementOrDeclaration(Ast* stmt)
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

void SelectTypePass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
}

void SelectTypePass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  Tree<Ast>* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_tableProperty(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_tableProperty(Ast* table_prop)
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

void SelectTypePass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void SelectTypePass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  Tree<Ast>* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_keyElement(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(element->keyElement.expr, 0);
}

void SelectTypePass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void SelectTypePass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  Tree<Ast>* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_actionRef(owner_of(ast, &Ast::tree), 0);
  }
}

void SelectTypePass::visit_actionRef(Ast* action_ref, Type* required_ty)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(action_ref->actionRef.name, 0);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args, required_ty);
  }
}

#if 0
void SelectTypePass::visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  visit_entriesList(entries_prop->entriesProperty.entries_list);
}

void SelectTypePass::visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree<Ast>* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_entry(owner_of(ast, &Ast::tree));
  }
}

void SelectTypePass::visit_entry(Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  visit_keysetExpression(entry->entry.keyset, 0);
  visit_actionRef(entry->entry.action, 0);
}

void SelectTypePass::visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  visit_expression(simple_prop->simpleProperty.init_expr, 0);
}
#endif

void SelectTypePass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

void SelectTypePass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr, 0);
  }
}

/** EXPRESSIONS **/

void SelectTypePass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

void SelectTypePass::visit_argumentList(Ast* args, Type* required_ty)
{
  assert(args->kind == AstEnum::argumentList);
  Tree<Ast>* ast;

  for (ast = args->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_argument(owner_of(ast, &Ast::tree), required_ty);
  }
}

void SelectTypePass::visit_argument(Ast* arg, Type* required_ty)
{
  assert(arg->kind == AstEnum::argument);
  Type* arg_ty;

  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg, required_ty);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
  arg_ty = type_env->lookup(arg->argument.arg, 0);
  assert(arg_ty);
  type_env->insert(arg, arg_ty, 0);
}

void SelectTypePass::visit_expressionList(Ast* expr_list, Type* required_ty)
{
  assert(expr_list->kind == AstEnum::expressionList);
  Tree<Ast>* ast;
  Type* list_ty;
  int i;

  list_ty = type_array->append();
  list_ty->ty_former = TypeEnum::PRODUCT;
  list_ty->ast = expr_list;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visit_expression(owner_of(ast, &Ast::tree), required_ty);
    list_ty->product.count += 1;
  }
  if (list_ty->product.count > 0) {
    list_ty->product.members = storage->allocate<Type *>(list_ty->product.count);
  }
  i = 0;
  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    list_ty->product.members[i] = type_env->lookup(owner_of(ast, &Ast::tree), 0);
    i += 1;
  }
  assert(i == list_ty->product.count);
  type_env->insert(expr_list, list_ty, 0);
}

void SelectTypePass::visit_lvalueExpression(Ast* lvalue_expr, Type* required_ty)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  Type* expr_ty;

  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr, required_ty);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  expr_ty = type_env->lookup(lvalue_expr->lvalueExpression.expr, 0);
  assert(expr_ty);
  type_env->insert(lvalue_expr, expr_ty, 0);
}

void SelectTypePass::visit_expression(Ast* expr, Type* required_ty)
{
  assert(expr->kind == AstEnum::expression);
  Type* expr_ty;

  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    visit_name(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(expr->expression.expr, required_ty);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  expr_ty = type_env->lookup(expr->expression.expr, 0);
  if (!expr_ty) {
    assert(expr_ty);
  }
  type_env->insert(expr, expr_ty, 0);
}

void SelectTypePass::visit_castExpression(Ast* cast_expr, Type* required_ty)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  Type* cast_ty;

  visit_typeRef(cast_expr->castExpression.type, required_ty);
  visit_expression(cast_expr->castExpression.expr, 0);
  cast_ty = type_env->lookup(cast_expr->castExpression.type, 0);
  type_env->insert(cast_expr, cast_ty, 0);
}

void SelectTypePass::visit_unaryExpression(Ast* unary_expr, Type* required_ty)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand, required_ty);
}

void SelectTypePass::visit_binaryExpression(Ast* binary_expr, Type* required_ty)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  PotentialType* op_tau;
  Type* op_ty;

  visit_expression(binary_expr->binaryExpression.left_operand, required_ty);
  visit_expression(binary_expr->binaryExpression.right_operand, required_ty);
  op_tau = potype_map->lookup(binary_expr, 0);
  if (op_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, binary_expr->line_no, binary_expr->column_no);
  }
  if (required_ty) {
    if (!type_checker->match_type(op_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, binary_expr->line_no, binary_expr->column_no);
    } else {
      op_ty = op_tau->set.members.first->key;
      type_env->insert(binary_expr, op_ty->effective_type(), 0);
    }
  } else {
    op_ty = op_tau->set.members.first->key;
    type_env->insert(binary_expr, op_ty->effective_type(), 0);
  }
}

void SelectTypePass::visit_memberSelector(Ast* selector, Type* required_ty)
{
  assert(selector->kind == AstEnum::memberSelector);
  PotentialType* selector_tau;
  Type* selector_ty;

  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr, 0);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr, 0);
  } else assert(0);
  selector_tau = potype_map->lookup(selector, 0);
  if (selector_tau->set.members.count() != 1) {
    error("%s:%d:%d: error: failed type check.",
        source_file, selector->line_no, selector->column_no);
  }
  if (required_ty) {
    if (!type_checker->match_type(selector_tau, required_ty)) {
      error("%s:%d:%d: error: failed type check.",
            source_file, selector->line_no, selector->column_no);
    } else {
      selector_ty = selector_tau->set.members.first->key;
      type_env->insert(selector, selector_ty->effective_type(), 0);
    }
  } else {
    selector_ty = selector_tau->set.members.first->key;
    type_env->insert(selector, selector_ty->effective_type(), 0);
  }
}

void SelectTypePass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  Type* lhs_ty;

  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr, 0);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr, 0);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
  lhs_ty = type_env->lookup(subscript->arraySubscript.lhs_expr, 0);
  type_env->insert(subscript, lhs_ty, 0);
}

void SelectTypePass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  visit_expression(index_expr->indexExpression.start_index, 0);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index, 0);
  }
}

void SelectTypePass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
}

void SelectTypePass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
}

void SelectTypePass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
}

void SelectTypePass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
}

void SelectTypePass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
}
