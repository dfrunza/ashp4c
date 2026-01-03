#include <basic.h>
#include <ast.h>
#include <passes/builtin_methods.h>

void BuiltinMethodsPass::do_pass(Ast* ast)
{
  visit_p4program(ast);
}

/** PROGRAM **/

void BuiltinMethodsPass::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  visit_declarationList(p4program->p4program.decl_list);
}

void BuiltinMethodsPass::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  TreeIterator it(&decl_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_declaration(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_declaration(Ast* decl)
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

void BuiltinMethodsPass::visit_name(Ast* name)
{
  assert(name->kind == AstEnum::name);
}

void BuiltinMethodsPass::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  TreeIterator it(&params->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parameter(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  visit_typeRef(param->parameter.type);
  visit_name(param->parameter.name);
  if (param->parameter.init_expr) {
    visit_expression(param->parameter.init_expr);
  }
}

void BuiltinMethodsPass::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  visit_name(type_decl->packageTypeDeclaration.name);
  visit_parameterList(type_decl->packageTypeDeclaration.params);
}

void BuiltinMethodsPass::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  visit_typeRef(inst->instantiation.type);
  visit_argumentList(inst->instantiation.args);
  visit_name(inst->instantiation.name);
}

/** PARSER **/

void BuiltinMethodsPass::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  visit_parserStates(parser_decl->parserDeclaration.states);
}

void BuiltinMethodsPass::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  Ast* return_type = Ast::create(storage, AstEnum::baseTypeVoid, 0, 0);
  return_type->name.strname = "void";

  Ast* type_ref = Ast::create(storage, AstEnum::typeRef, 0, 0);
  type_ref->typeRef.type = return_type;

  Ast* method = Ast::create(storage, AstEnum::functionPrototype, type_decl->line_no, type_decl->column_no);
  method->functionPrototype.return_type = type_ref;
  method->functionPrototype.params = type_decl->parserTypeDeclaration.params->clone(storage);

  Ast* name = Ast::create(storage, AstEnum::name, 0, 0);
  name->name.strname = "apply";
  method->functionPrototype.name = name;

  Ast* method_protos = type_decl->parserTypeDeclaration.method_protos;
  TreeConstructor tree_ctor;
  tree_ctor.append_node(&method_protos->tree, &method->tree);
}

void BuiltinMethodsPass::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  TreeIterator it(&local_elements->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserLocalElement(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
}

void BuiltinMethodsPass::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  TreeIterator it(&states->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserState(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  visit_name(state->parserState.name);
  visit_parserStatements(state->parserState.stmt_list);
  visit_transitionStatement(state->parserState.transition_stmt);
}

void BuiltinMethodsPass::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  TreeIterator it(&stmts->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_parserStatement(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_parserStatement(Ast* stmt)
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
  } else assert(0);
}

void BuiltinMethodsPass::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
}

void BuiltinMethodsPass::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  visit_stateExpression(transition_stmt->transitionStatement.stmt);
}

void BuiltinMethodsPass::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    visit_name(state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
}

void BuiltinMethodsPass::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  visit_expressionList(select_expr->selectExpression.expr_list);
  visit_selectCaseList(select_expr->selectExpression.case_list);
}

void BuiltinMethodsPass::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  TreeIterator it(&case_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_selectCase(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  visit_keysetExpression(select_case->selectCase.keyset_expr);
  visit_name(select_case->selectCase.name);
}

void BuiltinMethodsPass::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
}

void BuiltinMethodsPass::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
}

void BuiltinMethodsPass::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
}

void BuiltinMethodsPass::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  TreeIterator it(&expr_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_simpleKeysetExpression(Ast::owner_of(tree));
  }
}

/** CONTROL **/

void BuiltinMethodsPass::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
}

void BuiltinMethodsPass::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);

  Ast* return_type = Ast::create(storage, AstEnum::baseTypeVoid, 0, 0);
  return_type->name.strname = "void";

  Ast* type_ref = Ast::create(storage, AstEnum::typeRef, 0, 0);
  type_ref->typeRef.type = return_type;

  Ast* method = Ast::create(storage, AstEnum::functionPrototype, type_decl->line_no, type_decl->column_no);
  method->functionPrototype.return_type = type_ref;
  method->functionPrototype.params = type_decl->controlTypeDeclaration.params->clone(storage);

  Ast* name = Ast::create(storage, AstEnum::name, 0, 0);
  name->name.strname = "apply";
  method->functionPrototype.name = name;

  Ast* method_protos = type_decl->controlTypeDeclaration.method_protos;
  TreeConstructor tree_ctor;
  tree_ctor.append_node(&method_protos->tree, &method->tree);
}

void BuiltinMethodsPass::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  TreeIterator it(&local_decls->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_controlLocalDeclaration(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_controlLocalDeclaration(Ast* local_decl)
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

void BuiltinMethodsPass::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
}

void BuiltinMethodsPass::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  visit_name(type_decl->externTypeDeclaration.name);
  visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
}

void BuiltinMethodsPass::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  TreeIterator it(&protos->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_functionPrototype(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visit_typeRef(func_proto->functionPrototype.return_type);
  }
  visit_name(func_proto->functionPrototype.name);
  visit_parameterList(func_proto->functionPrototype.params);
}

/** TYPES **/

void BuiltinMethodsPass::visit_typeRef(Ast* type_ref)
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
    visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
}

void BuiltinMethodsPass::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  visit_typeArgumentList(type_decl->tupleType.type_args);
}

void BuiltinMethodsPass::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  visit_typeRef(type_decl->headerStackType.type);
  visit_expression(type_decl->headerStackType.stack_expr);
}

void BuiltinMethodsPass::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  visit_name(bool_type->baseTypeBoolean.name);
}

void BuiltinMethodsPass::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  visit_name(int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
}

void BuiltinMethodsPass::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  visit_name(bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
}

void BuiltinMethodsPass::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  visit_name(varbit_type->baseTypeVarbit.name);
  visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
}

void BuiltinMethodsPass::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  visit_name(str_type->baseTypeString.name);
}

void BuiltinMethodsPass::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  visit_name(void_type->baseTypeVoid.name);
}

void BuiltinMethodsPass::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  visit_name(error_type->baseTypeError.name);
}

void BuiltinMethodsPass::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
}

void BuiltinMethodsPass::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
}

void BuiltinMethodsPass::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
}

void BuiltinMethodsPass::visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::typeArgumentList);
  TreeIterator it(&arg_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_typeArg(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_typeDeclaration(Ast* type_decl)
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

void BuiltinMethodsPass::visit_derivedTypeDeclaration(Ast* type_decl)
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

void BuiltinMethodsPass::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  visit_name(header_decl->headerTypeDeclaration.name);
  visit_structFieldList(header_decl->headerTypeDeclaration.fields);
}

void BuiltinMethodsPass::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  visit_name(union_decl->headerUnionDeclaration.name);
  visit_structFieldList(union_decl->headerUnionDeclaration.fields);
}

void BuiltinMethodsPass::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  visit_name(struct_decl->structTypeDeclaration.name);
  visit_structFieldList(struct_decl->structTypeDeclaration.fields);
}

void BuiltinMethodsPass::visit_structFieldList(Ast* field_list)
{
  assert(field_list->kind == AstEnum::structFieldList);
  TreeIterator it(&field_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_structField(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  visit_typeRef(field->structField.type);
  visit_name(field->structField.name);
}

void BuiltinMethodsPass::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  visit_name(enum_decl->enumDeclaration.name);
  visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
}

void BuiltinMethodsPass::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  visit_identifierList(error_decl->errorDeclaration.fields);
}

void BuiltinMethodsPass::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  visit_identifierList(match_decl->matchKindDeclaration.fields);
}

void BuiltinMethodsPass::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
  TreeIterator it(&ident_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_name(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  TreeIterator it(&ident_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_specifiedIdentifier(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  visit_name(ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    visit_expression(ident->specifiedIdentifier.init_expr);
  }
}

void BuiltinMethodsPass::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  visit_name(typedef_decl->typedefDeclaration.name);
}

/** STATEMENTS **/

void BuiltinMethodsPass::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visit_expression(assign_stmt->assignmentStatement.rhs_expr);
}

void BuiltinMethodsPass::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  visit_argumentList(func_call->functionCall.args);
}

void BuiltinMethodsPass::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    visit_expression(return_stmt->returnStatement.expr);
  }
}

void BuiltinMethodsPass::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
}

void BuiltinMethodsPass::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  visit_expression(cond_stmt->conditionalStatement.cond_expr);
  visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
}

void BuiltinMethodsPass::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    visit_name(applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  visit_argumentList(applic_stmt->directApplication.args);
}

void BuiltinMethodsPass::visit_statement(Ast* stmt)
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

void BuiltinMethodsPass::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
}

void BuiltinMethodsPass::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  TreeIterator it(&stmt_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_statementOrDeclaration(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  visit_expression(switch_stmt->switchStatement.expr);
  visit_switchCases(switch_stmt->switchStatement.switch_cases);
}

void BuiltinMethodsPass::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  TreeIterator it(&switch_cases->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_switchCase(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visit_blockStatement(switch_case->switchCase.stmt);
  }
}

void BuiltinMethodsPass::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    visit_name(label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    visit_default(label->switchLabel.label);
  } else assert(0);
}

void BuiltinMethodsPass::visit_statementOrDeclaration(Ast* stmt)
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

void BuiltinMethodsPass::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);

  Ast* return_type = Ast::create(storage, AstEnum::baseTypeVoid, 0, 0);
  return_type->name.strname = "void";

  Ast* type_ref = Ast::create(storage, AstEnum::typeRef, 0, 0);
  type_ref->typeRef.type = return_type;

  Ast* method = Ast::create(storage, AstEnum::functionPrototype, table_decl->line_no, table_decl->column_no);
  method->functionPrototype.return_type = type_ref;

  Ast* params = Ast::create(storage, AstEnum::parameterList, table_decl->line_no, table_decl->column_no);
  method->functionPrototype.params = params;

  Ast* name = Ast::create(storage, AstEnum::name, 0, 0);
  name->name.strname = "apply";
  method->functionPrototype.name = name;

  Ast* method_protos = table_decl->tableDeclaration.method_protos;
  TreeConstructor tree_ctor;
  tree_ctor.append_node(&method_protos->tree, &method->tree);
}

void BuiltinMethodsPass::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  TreeIterator it(&prop_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_tableProperty(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_tableProperty(Ast* table_prop)
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

void BuiltinMethodsPass::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  visit_keyElementList(key_prop->keyProperty.keyelem_list);
}

void BuiltinMethodsPass::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  TreeIterator it(&element_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_keyElement(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  visit_expression(element->keyElement.expr);
  visit_name(element->keyElement.match);
}

void BuiltinMethodsPass::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  visit_actionList(actions_prop->actionsProperty.action_list);
}

void BuiltinMethodsPass::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  TreeIterator it(&action_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_actionRef(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  visit_name(action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visit_argumentList(action_ref->actionRef.args);
  }
}

void BuiltinMethodsPass::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  visit_name(action_decl->actionDeclaration.name);
  visit_parameterList(action_decl->actionDeclaration.params);
  visit_blockStatement(action_decl->actionDeclaration.stmt);
}

/** VARIABLES **/

void BuiltinMethodsPass::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  visit_typeRef(var_decl->variableDeclaration.type);
  visit_name(var_decl->variableDeclaration.name);
  if (var_decl->variableDeclaration.init_expr) {
    visit_expression(var_decl->variableDeclaration.init_expr);
  }
}

/** EXPRESSIONS **/

void BuiltinMethodsPass::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  visit_functionPrototype(func_decl->functionDeclaration.proto);
  visit_blockStatement(func_decl->functionDeclaration.stmt);
}

void BuiltinMethodsPass::visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::argumentList);
  TreeIterator it(&arg_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_argument(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  if (arg->argument.arg->kind == AstEnum::expression) {
    visit_expression(arg->argument.arg);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    visit_dontcare(arg->argument.arg);
  } else assert(0);
}

void BuiltinMethodsPass::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  TreeIterator it(&expr_list->tree);
  for (Tree* tree = it.next();
       tree != 0; tree = it.next()) {
    visit_expression(Ast::owner_of(tree));
  }
}

void BuiltinMethodsPass::visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    visit_name(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
}

void BuiltinMethodsPass::visit_expression(Ast* expr)
{
  assert(expr->kind == AstEnum::expression);
  if (expr->expression.expr->kind == AstEnum::expression) {
    visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    visit_name(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
}

void BuiltinMethodsPass::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  visit_typeRef(cast_expr->castExpression.type);
  visit_expression(cast_expr->castExpression.expr);
}

void BuiltinMethodsPass::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  visit_expression(unary_expr->unaryExpression.operand);
}

void BuiltinMethodsPass::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  visit_expression(binary_expr->binaryExpression.left_operand);
  visit_expression(binary_expr->binaryExpression.right_operand);
}

void BuiltinMethodsPass::visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AstEnum::memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
  visit_name(selector->memberSelector.name);
}

void BuiltinMethodsPass::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visit_indexExpression(subscript->arraySubscript.index_expr);
}

void BuiltinMethodsPass::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visit_expression(index_expr->indexExpression.end_index);
  }
}

void BuiltinMethodsPass::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
}

void BuiltinMethodsPass::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
}

void BuiltinMethodsPass::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
}

void BuiltinMethodsPass::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
}

void BuiltinMethodsPass::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
}

