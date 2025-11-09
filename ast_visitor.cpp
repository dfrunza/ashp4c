#include "ast_visitor.h"

/** PROGRAM **/

void* AstVisitor::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AstEnum::p4program);
  this->visit_declarationList(p4program->p4program.decl_list);
  return 0;
}

void* AstVisitor::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AstEnum::declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_declaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_declaration(Ast* decl)
{
  assert(decl->kind == AstEnum::declaration);
  if (decl->declaration.decl->kind == AstEnum::variableDeclaration) {
    this->visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::externDeclaration) {
    this->visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::actionDeclaration) {
    this->visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::functionDeclaration) {
    this->visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserDeclaration) {
    this->visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::parserTypeDeclaration) {
    this->visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlDeclaration) {
    this->visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::controlTypeDeclaration) {
    this->visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::typeDeclaration) {
    this->visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::errorDeclaration) {
    this->visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::matchKindDeclaration) {
    this->visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AstEnum::instantiation) {
    this->visit_instantiation(decl->declaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_name(Ast* name)
{
  assert(name->kind == AstEnum::name);
  return 0;
}

void* AstVisitor::visit_parameterList(Ast* params)
{
  assert(params->kind == AstEnum::parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parameter(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parameter(Ast* param)
{
  assert(param->kind == AstEnum::parameter);
  this->visit_typeRef(param->parameter.type);
  this->visit_name(param->parameter.name);
  if (param->parameter.init_expr) {
    this->visit_expression(param->parameter.init_expr);
  }
  return 0;
}

void* AstVisitor::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::packageTypeDeclaration);
  this->visit_name(type_decl->packageTypeDeclaration.name);
  this->visit_parameterList(type_decl->packageTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AstEnum::instantiation);
  this->visit_typeRef(inst->instantiation.type);
  this->visit_argumentList(inst->instantiation.args);
  this->visit_name(inst->instantiation.name);
  return 0;
}

/** PARSER **/

void* AstVisitor::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AstEnum::parserDeclaration);
  this->visit_typeDeclaration(parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    this->visit_parameterList(parser_decl->parserDeclaration.ctor_params);
  }
  this->visit_parserLocalElements(parser_decl->parserDeclaration.local_elements);
  this->visit_parserStates(parser_decl->parserDeclaration.states);
  return 0;
}

void* AstVisitor::visit_parserTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::parserTypeDeclaration);
  this->visit_name(type_decl->parserTypeDeclaration.name);
  this->visit_parameterList(type_decl->parserTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AstEnum::parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserLocalElement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AstEnum::parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AstEnum::variableDeclaration) {
    this->visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AstEnum::instantiation) {
    this->visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_parserStates(Ast* states)
{
  assert(states->kind == AstEnum::parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserState(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserState(Ast* state)
{
  assert(state->kind == AstEnum::parserState);
  this->visit_name(state->parserState.name);
  this->visit_parserStatements(state->parserState.stmt_list);
  this->visit_transitionStatement(state->parserState.transition_stmt);
  return 0;
}

void* AstVisitor::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AstEnum::parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserStatement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::parserStatement);
  if (stmt->parserStatement.stmt->kind == AstEnum::assignmentStatement) {
    this->visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::functionCall) {
    this->visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::directApplication) {
    this->visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::parserBlockStatement) {
    this->visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::variableDeclaration) {
    this->visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::parserBlockStatement);
  this->visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
  return 0;
}

void* AstVisitor::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AstEnum::transitionStatement);
  this->visit_stateExpression(transition_stmt->transitionStatement.stmt);
  return 0;
}

void* AstVisitor::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AstEnum::stateExpression);
  if (state_expr->stateExpression.expr->kind == AstEnum::name) {
    this->visit_name(state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AstEnum::selectExpression) {
    this->visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AstEnum::selectExpression);
  this->visit_expressionList(select_expr->selectExpression.expr_list);
  this->visit_selectCaseList(select_expr->selectExpression.case_list);
  return 0;
}

void* AstVisitor::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AstEnum::selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_selectCase(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AstEnum::selectCase);
  this->visit_keysetExpression(select_case->selectCase.keyset_expr);
  this->visit_name(select_case->selectCase.name);
  return 0;
}

void* AstVisitor::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AstEnum::keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AstEnum::tupleKeysetExpression) {
    this->visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AstEnum::simpleKeysetExpression) {
    this->visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AstEnum::tupleKeysetExpression);
  this->visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
  return 0;
}

void* AstVisitor::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AstEnum::simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::expression) {
    this->visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::default_) {
    this->visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AstEnum::dontcare) {
    this->visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_simpleKeysetExpression(container_of(ast, Ast, tree));
  }
  return 0;
}

/** CONTROL **/

void* AstVisitor::visit_controlDeclaration(Ast* control_decl)
{
  assert(control_decl->kind == AstEnum::controlDeclaration);
  this->visit_typeDeclaration(control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    this->visit_parameterList(control_decl->controlDeclaration.ctor_params);
  }
  this->visit_controlLocalDeclarations(control_decl->controlDeclaration.local_decls);
  this->visit_blockStatement(control_decl->controlDeclaration.apply_stmt);
  return 0;
}

void* AstVisitor::visit_controlTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::controlTypeDeclaration);
  this->visit_name(type_decl->controlTypeDeclaration.name);
  this->visit_parameterList(type_decl->controlTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AstEnum::controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_controlLocalDeclaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AstEnum::controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::variableDeclaration) {
    this->visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::actionDeclaration) {
    this->visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::tableDeclaration) {
    this->visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AstEnum::instantiation) {
    this->visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
  return 0;
}

/** EXTERN **/

void* AstVisitor::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AstEnum::externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AstEnum::externTypeDeclaration) {
    this->visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AstEnum::functionPrototype) {
    this->visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::externTypeDeclaration);
  this->visit_name(type_decl->externTypeDeclaration.name);
  this->visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
  return 0;
}

void* AstVisitor::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AstEnum::methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_functionPrototype(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AstEnum::functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    this->visit_typeRef(func_proto->functionPrototype.return_type);
  }
  this->visit_name(func_proto->functionPrototype.name);
  this->visit_parameterList(func_proto->functionPrototype.params);
  return 0;
}

/** TYPES **/

void* AstVisitor::visit_typeRef(Ast* type_ref)
{
  assert(type_ref->kind == AstEnum::typeRef);
  if (type_ref->typeRef.type->kind == AstEnum::baseTypeBoolean) {
    this->visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeInteger) {
    this->visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeBit) {
    this->visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVarbit) {
    this->visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeString) {
    this->visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeVoid) {
    this->visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::baseTypeError) {
    this->visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::name) {
    this->visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::headerStackType) {
    this->visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AstEnum::tupleType) {
    this->visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::tupleType);
  this->visit_typeArgumentList(type_decl->tupleType.type_args);
  return 0;
}

void* AstVisitor::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::headerStackType);
  this->visit_typeRef(type_decl->headerStackType.type);
  this->visit_expression(type_decl->headerStackType.stack_expr);
  return 0;
}

void* AstVisitor::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AstEnum::baseTypeBoolean);
  this->visit_name(bool_type->baseTypeBoolean.name);
  return 0;
}

void* AstVisitor::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AstEnum::baseTypeInteger);
  this->visit_name(int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    this->visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  return 0;
}

void* AstVisitor::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AstEnum::baseTypeBit);
  this->visit_name(bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    this->visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  return 0;
}

void* AstVisitor::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AstEnum::baseTypeVarbit);
  this->visit_name(varbit_type->baseTypeVarbit.name);
  this->visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  return 0;
}

void* AstVisitor::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AstEnum::baseTypeString);
  this->visit_name(str_type->baseTypeString.name);
  return 0;
}

void* AstVisitor::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AstEnum::baseTypeVoid);
  this->visit_name(void_type->baseTypeVoid.name);
  return 0;
}

void* AstVisitor::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AstEnum::baseTypeError);
  this->visit_name(error_type->baseTypeError.name);
  return 0;
}

void* AstVisitor::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AstEnum::integerTypeSize);
  return 0;
}

void* AstVisitor::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AstEnum::typeRef) {
    this->visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AstEnum::dontcare) {
    this->visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AstEnum::typeArg);
  if (type_arg->typeArg.arg->kind == AstEnum::typeRef) {
    this->visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::name) {
    this->visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AstEnum::dontcare) {
    this->visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_typeArg(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AstEnum::derivedTypeDeclaration) {
    this->visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::typedefDeclaration) {
    this->visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::parserTypeDeclaration) {
    this->visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::controlTypeDeclaration) {
    this->visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AstEnum::packageTypeDeclaration) {
    this->visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AstEnum::derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerTypeDeclaration) {
    this->visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::headerUnionDeclaration) {
    this->visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::structTypeDeclaration) {
    this->visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AstEnum::enumDeclaration) {
    this->visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AstEnum::headerTypeDeclaration);
  this->visit_name(header_decl->headerTypeDeclaration.name);
  this->visit_structFieldList(header_decl->headerTypeDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AstEnum::headerUnionDeclaration);
  this->visit_name(union_decl->headerUnionDeclaration.name);
  this->visit_structFieldList(union_decl->headerUnionDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AstEnum::structTypeDeclaration);
  this->visit_name(struct_decl->structTypeDeclaration.name);
  this->visit_structFieldList(struct_decl->structTypeDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_structFieldList(Ast* field_list)
{
  assert(field_list->kind == AstEnum::structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_structField(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_structField(Ast* field)
{
  assert(field->kind == AstEnum::structField);
  this->visit_typeRef(field->structField.type);
  this->visit_name(field->structField.name);
  return 0;
}

void* AstVisitor::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AstEnum::enumDeclaration);
  this->visit_name(enum_decl->enumDeclaration.name);
  this->visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AstEnum::errorDeclaration);
  this->visit_identifierList(error_decl->errorDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AstEnum::matchKindDeclaration);
  this->visit_identifierList(match_decl->matchKindDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::identifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_name(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AstEnum::specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_specifiedIdentifier(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AstEnum::specifiedIdentifier);
  this->visit_name(ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    this->visit_expression(ident->specifiedIdentifier.init_expr);
  }
  return 0;
}

void* AstVisitor::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AstEnum::typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::typeRef) {
    this->visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AstEnum::derivedTypeDeclaration) {
    this->visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  this->visit_name(typedef_decl->typedefDeclaration.name);
  return 0;
}

/** STATEMENTS **/

void* AstVisitor::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AstEnum::assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::expression) {
    this->visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AstEnum::lvalueExpression) {
    this->visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  this->visit_expression(assign_stmt->assignmentStatement.rhs_expr);
  return 0;
}

void* AstVisitor::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AstEnum::functionCall);
  if (func_call->functionCall.lhs_expr->kind == AstEnum::expression) {
    this->visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AstEnum::lvalueExpression) {
    this->visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  this->visit_argumentList(func_call->functionCall.args);
  return 0;
}

void* AstVisitor::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AstEnum::returnStatement);
  if (return_stmt->returnStatement.expr) {
    this->visit_expression(return_stmt->returnStatement.expr);
  }
  return 0;
}

void* AstVisitor::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AstEnum::exitStatement);
  return 0;
}

void* AstVisitor::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AstEnum::conditionalStatement);
  this->visit_expression(cond_stmt->conditionalStatement.cond_expr);
  this->visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    this->visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
  return 0;
}

void* AstVisitor::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AstEnum::directApplication);
  if (applic_stmt->directApplication.name->kind == AstEnum::name) {
    this->visit_name(applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AstEnum::typeRef) {
    this->visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  this->visit_argumentList(applic_stmt->directApplication.args);
  return 0;
}

void* AstVisitor::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statement);
  if (stmt->statement.stmt->kind == AstEnum::assignmentStatement) {
    this->visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::functionCall) {
    this->visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::directApplication) {
    this->visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::conditionalStatement) {
    this->visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AstEnum::blockStatement) {
    this->visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::exitStatement) {
    this->visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::returnStatement) {
    this->visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AstEnum::switchStatement) {
    this->visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AstEnum::blockStatement);
  this->visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
  return 0;
}

void* AstVisitor::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AstEnum::statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_statementOrDeclaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AstEnum::switchStatement);
  this->visit_expression(switch_stmt->switchStatement.expr);
  this->visit_switchCases(switch_stmt->switchStatement.switch_cases);
  return 0;
}

void* AstVisitor::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AstEnum::switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_switchCase(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AstEnum::switchCase);
  this->visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    this->visit_blockStatement(switch_case->switchCase.stmt);
  }
  return 0;
}

void* AstVisitor::visit_switchLabel(Ast* label)
{
  assert(label->kind == AstEnum::switchLabel);
  if (label->switchLabel.label->kind == AstEnum::name) {
    this->visit_name(label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AstEnum::default_) {
    this->visit_default(label->switchLabel.label);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AstEnum::statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AstEnum::variableDeclaration) {
    this->visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::statement) {
    this->visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AstEnum::instantiation) {
    this->visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
  return 0;
}

/** TABLES **/

void* AstVisitor::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AstEnum::tableDeclaration);
  this->visit_name(table_decl->tableDeclaration.name);
  this->visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  return 0;
}

void* AstVisitor::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AstEnum::tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_tableProperty(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AstEnum::tableProperty);
  if (table_prop->tableProperty.prop->kind == AstEnum::keyProperty) {
    this->visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::actionsProperty) {
    this->visit_actionsProperty(table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AstEnum::entriesProperty) {
    this->visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AstEnum::simpleProperty) {
    this->visit_simpleProperty(table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
  return 0;
}

void* AstVisitor::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AstEnum::keyProperty);
  this->visit_keyElementList(key_prop->keyProperty.keyelem_list);
  return 0;
}

void* AstVisitor::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AstEnum::keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_keyElement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_keyElement(Ast* element)
{
  assert(element->kind == AstEnum::keyElement);
  this->visit_expression(element->keyElement.expr);
  this->visit_name(element->keyElement.match);
  return 0;
}

void* AstVisitor::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AstEnum::actionsProperty);
  this->visit_actionList(actions_prop->actionsProperty.action_list);
  return 0;
}

void* AstVisitor::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AstEnum::actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_actionRef(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AstEnum::actionRef);
  this->visit_name(action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    this->visit_argumentList(action_ref->actionRef.args);
  }
  return 0;
}

#if 0
void* AstVisitor::visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AstEnum::entriesProperty);
  this->visit_entriesList(entries_prop->entriesProperty.entries_list);
  return 0;
}

void* AstVisitor::visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AstEnum::entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_entry(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_entry(Ast* entry)
{
  assert(entry->kind == AstEnum::entry);
  this->visit_keysetExpression(entry->entry.keyset);
  this->visit_actionRef(entry->entry.action);
  return 0;
}

void* AstVisitor::visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AstEnum::simpleProperty);
  this->visit_name(simple_prop->simpleProperty.name);
  this->visit_expression(simple_prop->simpleProperty.init_expr);
  return 0;
}
#endif

void* AstVisitor::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AstEnum::actionDeclaration);
  this->visit_name(action_decl->actionDeclaration.name);
  this->visit_parameterList(action_decl->actionDeclaration.params);
  this->visit_blockStatement(action_decl->actionDeclaration.stmt);
  return 0;
}

/** VARIABLES **/

void* AstVisitor::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AstEnum::variableDeclaration);
  this->visit_typeRef(var_decl->variableDeclaration.type);
  this->visit_name(var_decl->variableDeclaration.name);
  if (var_decl->variableDeclaration.init_expr) {
    this->visit_expression(var_decl->variableDeclaration.init_expr);
  }
  return 0;
}

/** EXPRESSIONS **/

void* AstVisitor::visit_functionDeclaration(Ast* func_decl)
{
  assert(func_decl->kind == AstEnum::functionDeclaration);
  this->visit_functionPrototype(func_decl->functionDeclaration.proto);
  this->visit_blockStatement(func_decl->functionDeclaration.stmt);
  return 0;
}

void* AstVisitor::visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AstEnum::argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_argument(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_argument(Ast* arg)
{
  assert(arg->kind == AstEnum::argument);
  if (arg->argument.arg->kind == AstEnum::expression) {
    this->visit_expression(arg->argument.arg);
  } else if (arg->argument.arg->kind == AstEnum::dontcare) {
    this->visit_dontcare(arg->argument.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AstEnum::expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_expression(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AstEnum::lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::name) {
    this->visit_name(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::memberSelector) {
    this->visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AstEnum::arraySubscript) {
    this->visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_expression(Ast* expr)
{
  assert(expr->kind == AstEnum::expression);
  if (expr->expression.expr->kind == AstEnum::expression) {
    this->visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::booleanLiteral) {
    this->visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::integerLiteral) {
    this->visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::stringLiteral) {
    this->visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::name) {
    this->visit_name(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::expressionList) {
    this->visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::castExpression) {
    this->visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::unaryExpression) {
    this->visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::binaryExpression) {
    this->visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::memberSelector) {
    this->visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::arraySubscript) {
    this->visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::functionCall) {
    this->visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AstEnum::assignmentStatement) {
    this->visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AstEnum::castExpression);
  this->visit_typeRef(cast_expr->castExpression.type);
  this->visit_expression(cast_expr->castExpression.expr);
  return 0;
}

void* AstVisitor::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AstEnum::unaryExpression);
  this->visit_expression(unary_expr->unaryExpression.operand);
  return 0;
}

void* AstVisitor::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AstEnum::binaryExpression);
  this->visit_expression(binary_expr->binaryExpression.left_operand);
  this->visit_expression(binary_expr->binaryExpression.right_operand);
  return 0;
}

void* AstVisitor::visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AstEnum::memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AstEnum::expression) {
    this->visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AstEnum::lvalueExpression) {
    this->visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
  this->visit_name(selector->memberSelector.name);
  return 0;
}

void* AstVisitor::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AstEnum::arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AstEnum::expression) {
    this->visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AstEnum::lvalueExpression) {
    this->visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  this->visit_indexExpression(subscript->arraySubscript.index_expr);
  return 0;
}

void* AstVisitor::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AstEnum::indexExpression);
  this->visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    this->visit_expression(index_expr->indexExpression.end_index);
  }
  return 0;
}

void* AstVisitor::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AstEnum::booleanLiteral);
  return 0;
}

void* AstVisitor::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AstEnum::integerLiteral);
  return 0;
}

void* AstVisitor::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AstEnum::stringLiteral);
  return 0;
}

void* AstVisitor::visit_default(Ast* default_)
{
  assert(default_->kind == AstEnum::default_);
  return 0;
}

void* AstVisitor::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AstEnum::dontcare);
  return 0;
}

void ast_visitor_init(AstVisitor* visitor)
{ }
