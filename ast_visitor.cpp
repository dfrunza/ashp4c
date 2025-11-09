#include "ast_visitor.h"

/** PROGRAM **/

void* AstVisitor::visit_p4program(Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  this->visit_declarationList(p4program->p4program.decl_list);
  return 0;
}

void* AstVisitor::visit_declarationList(Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_declaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_declaration(Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    this->visit_variableDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    this->visit_externDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    this->visit_actionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    this->visit_functionDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    this->visit_parserDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    this->visit_parserTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    this->visit_controlDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    this->visit_controlTypeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    this->visit_typeDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    this->visit_errorDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    this->visit_matchKindDeclaration(decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    this->visit_instantiation(decl->declaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_name(Ast* name)
{
  assert(name->kind == AST_name);
  return 0;
}

void* AstVisitor::visit_parameterList(Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parameter(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parameter(Ast* param)
{
  assert(param->kind == AST_parameter);
  this->visit_typeRef(param->parameter.type);
  this->visit_name(param->parameter.name);
  if (param->parameter.init_expr) {
    this->visit_expression(param->parameter.init_expr);
  }
  return 0;
}

void* AstVisitor::visit_packageTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  this->visit_name(type_decl->packageTypeDeclaration.name);
  this->visit_parameterList(type_decl->packageTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_instantiation(Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  this->visit_typeRef(inst->instantiation.type);
  this->visit_argumentList(inst->instantiation.args);
  this->visit_name(inst->instantiation.name);
  return 0;
}

/** PARSER **/

void* AstVisitor::visit_parserDeclaration(Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
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
  assert(type_decl->kind == AST_parserTypeDeclaration);
  this->visit_name(type_decl->parserTypeDeclaration.name);
  this->visit_parameterList(type_decl->parserTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_parserLocalElements(Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserLocalElement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserLocalElement(Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    this->visit_variableDeclaration(local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    this->visit_instantiation(local_element->parserLocalElement.element);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_parserStates(Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserState(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserState(Ast* state)
{
  assert(state->kind == AST_parserState);
  this->visit_name(state->parserState.name);
  this->visit_parserStatements(state->parserState.stmt_list);
  this->visit_transitionStatement(state->parserState.transition_stmt);
  return 0;
}

void* AstVisitor::visit_parserStatements(Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_parserStatement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_parserStatement(Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    this->visit_assignmentStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    this->visit_functionCall(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    this->visit_directApplication(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    this->visit_parserBlockStatement(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    this->visit_variableDeclaration(stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_parserBlockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  this->visit_parserStatements(block_stmt->parserBlockStatement.stmt_list);
  return 0;
}

void* AstVisitor::visit_transitionStatement(Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  this->visit_stateExpression(transition_stmt->transitionStatement.stmt);
  return 0;
}

void* AstVisitor::visit_stateExpression(Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    this->visit_name(state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    this->visit_selectExpression(state_expr->stateExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_selectExpression(Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  this->visit_expressionList(select_expr->selectExpression.expr_list);
  this->visit_selectCaseList(select_expr->selectExpression.case_list);
  return 0;
}

void* AstVisitor::visit_selectCaseList(Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_selectCase(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_selectCase(Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  this->visit_keysetExpression(select_case->selectCase.keyset_expr);
  this->visit_name(select_case->selectCase.name);
  return 0;
}

void* AstVisitor::visit_keysetExpression(Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    this->visit_tupleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    this->visit_simpleKeysetExpression(keyset_expr->keysetExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_tupleKeysetExpression(Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  this->visit_simpleExpressionList(tuple_expr->tupleKeysetExpression.expr_list);
  return 0;
}

void* AstVisitor::visit_simpleKeysetExpression(Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    this->visit_expression(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    this->visit_default(simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    this->visit_dontcare(simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_simpleExpressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
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
  assert(control_decl->kind == AST_controlDeclaration);
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
  assert(type_decl->kind == AST_controlTypeDeclaration);
  this->visit_name(type_decl->controlTypeDeclaration.name);
  this->visit_parameterList(type_decl->controlTypeDeclaration.params);
  return 0;
}

void* AstVisitor::visit_controlLocalDeclarations(Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_controlLocalDeclaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_controlLocalDeclaration(Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    this->visit_variableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    this->visit_actionDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    this->visit_tableDeclaration(local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    this->visit_instantiation(local_decl->controlLocalDeclaration.decl);
  } else assert(0);
  return 0;
}

/** EXTERN **/

void* AstVisitor::visit_externDeclaration(Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    this->visit_externTypeDeclaration(extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    this->visit_functionPrototype(extern_decl->externDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_externTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  this->visit_name(type_decl->externTypeDeclaration.name);
  this->visit_methodPrototypes(type_decl->externTypeDeclaration.method_protos);
  return 0;
}

void* AstVisitor::visit_methodPrototypes(Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_functionPrototype(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_functionPrototype(Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
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
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    this->visit_baseTypeBoolean(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    this->visit_baseTypeInteger(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    this->visit_baseTypeBit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    this->visit_baseTypeVarbit(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    this->visit_baseTypeString(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    this->visit_baseTypeVoid(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    this->visit_baseTypeError(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    this->visit_name(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    this->visit_headerStackType(type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    this->visit_tupleType(type_ref->typeRef.type);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_tupleType(Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  this->visit_typeArgumentList(type_decl->tupleType.type_args);
  return 0;
}

void* AstVisitor::visit_headerStackType(Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  this->visit_typeRef(type_decl->headerStackType.type);
  this->visit_expression(type_decl->headerStackType.stack_expr);
  return 0;
}

void* AstVisitor::visit_baseTypeBoolean(Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  this->visit_name(bool_type->baseTypeBoolean.name);
  return 0;
}

void* AstVisitor::visit_baseTypeInteger(Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  this->visit_name(int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    this->visit_integerTypeSize(int_type->baseTypeInteger.size);
  }
  return 0;
}

void* AstVisitor::visit_baseTypeBit(Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  this->visit_name(bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    this->visit_integerTypeSize(bit_type->baseTypeBit.size);
  }
  return 0;
}

void* AstVisitor::visit_baseTypeVarbit(Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  this->visit_name(varbit_type->baseTypeVarbit.name);
  this->visit_integerTypeSize(varbit_type->baseTypeVarbit.size);
  return 0;
}

void* AstVisitor::visit_baseTypeString(Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  this->visit_name(str_type->baseTypeString.name);
  return 0;
}

void* AstVisitor::visit_baseTypeVoid(Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  this->visit_name(void_type->baseTypeVoid.name);
  return 0;
}

void* AstVisitor::visit_baseTypeError(Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  this->visit_name(error_type->baseTypeError.name);
  return 0;
}

void* AstVisitor::visit_integerTypeSize(Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
  return 0;
}

void* AstVisitor::visit_realTypeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    this->visit_typeRef(type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    this->visit_dontcare(type_arg->realTypeArg.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_typeArg(Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    this->visit_typeRef(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    this->visit_name(type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    this->visit_dontcare(type_arg->typeArg.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_typeArgumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_typeArg(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_typeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    this->visit_derivedTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    this->visit_typedefDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    this->visit_parserTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    this->visit_controlTypeDeclaration(type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    this->visit_packageTypeDeclaration(type_decl->typeDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_derivedTypeDeclaration(Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    this->visit_headerTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    this->visit_headerUnionDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    this->visit_structTypeDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    this->visit_enumDeclaration(type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_headerTypeDeclaration(Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  this->visit_name(header_decl->headerTypeDeclaration.name);
  this->visit_structFieldList(header_decl->headerTypeDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_headerUnionDeclaration(Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  this->visit_name(union_decl->headerUnionDeclaration.name);
  this->visit_structFieldList(union_decl->headerUnionDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_structTypeDeclaration(Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  this->visit_name(struct_decl->structTypeDeclaration.name);
  this->visit_structFieldList(struct_decl->structTypeDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_structFieldList(Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_structField(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_structField(Ast* field)
{
  assert(field->kind == AST_structField);
  this->visit_typeRef(field->structField.type);
  this->visit_name(field->structField.name);
  return 0;
}

void* AstVisitor::visit_enumDeclaration(Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  this->visit_name(enum_decl->enumDeclaration.name);
  this->visit_specifiedIdentifierList(enum_decl->enumDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_errorDeclaration(Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  this->visit_identifierList(error_decl->errorDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_matchKindDeclaration(Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  this->visit_identifierList(match_decl->matchKindDeclaration.fields);
  return 0;
}

void* AstVisitor::visit_identifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_name(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_specifiedIdentifierList(Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_specifiedIdentifier(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_specifiedIdentifier(Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  this->visit_name(ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    this->visit_expression(ident->specifiedIdentifier.init_expr);
  }
  return 0;
}

void* AstVisitor::visit_typedefDeclaration(Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    this->visit_typeRef(typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    this->visit_derivedTypeDeclaration(typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  this->visit_name(typedef_decl->typedefDeclaration.name);
  return 0;
}

/** STATEMENTS **/

void* AstVisitor::visit_assignmentStatement(Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    this->visit_expression(assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    this->visit_lvalueExpression(assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  this->visit_expression(assign_stmt->assignmentStatement.rhs_expr);
  return 0;
}

void* AstVisitor::visit_functionCall(Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    this->visit_expression(func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    this->visit_lvalueExpression(func_call->functionCall.lhs_expr);
  } else assert(0);
  this->visit_argumentList(func_call->functionCall.args);
  return 0;
}

void* AstVisitor::visit_returnStatement(Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    this->visit_expression(return_stmt->returnStatement.expr);
  }
  return 0;
}

void* AstVisitor::visit_exitStatement(Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
  return 0;
}

void* AstVisitor::visit_conditionalStatement(Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  this->visit_expression(cond_stmt->conditionalStatement.cond_expr);
  this->visit_statement(cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    this->visit_statement(cond_stmt->conditionalStatement.else_stmt);
  }
  return 0;
}

void* AstVisitor::visit_directApplication(Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    this->visit_name(applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    this->visit_typeRef(applic_stmt->directApplication.name);
  } else assert(0);
  this->visit_argumentList(applic_stmt->directApplication.args);
  return 0;
}

void* AstVisitor::visit_statement(Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    this->visit_assignmentStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    this->visit_functionCall(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    this->visit_directApplication(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    this->visit_conditionalStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    this->visit_blockStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    this->visit_exitStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    this->visit_returnStatement(stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    this->visit_switchStatement(stmt->statement.stmt);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_blockStatement(Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  this->visit_statementOrDeclList(block_stmt->blockStatement.stmt_list);
  return 0;
}

void* AstVisitor::visit_statementOrDeclList(Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_statementOrDeclaration(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_switchStatement(Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  this->visit_expression(switch_stmt->switchStatement.expr);
  this->visit_switchCases(switch_stmt->switchStatement.switch_cases);
  return 0;
}

void* AstVisitor::visit_switchCases(Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_switchCase(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_switchCase(Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  this->visit_switchLabel(switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    this->visit_blockStatement(switch_case->switchCase.stmt);
  }
  return 0;
}

void* AstVisitor::visit_switchLabel(Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    this->visit_name(label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AST_default) {
    this->visit_default(label->switchLabel.label);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_statementOrDeclaration(Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    this->visit_variableDeclaration(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    this->visit_statement(stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    this->visit_instantiation(stmt->statementOrDeclaration.stmt);
  } else assert(0);
  return 0;
}

/** TABLES **/

void* AstVisitor::visit_tableDeclaration(Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  this->visit_name(table_decl->tableDeclaration.name);
  this->visit_tablePropertyList(table_decl->tableDeclaration.prop_list);
  return 0;
}

void* AstVisitor::visit_tablePropertyList(Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_tableProperty(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_tableProperty(Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    this->visit_keyProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    this->visit_actionsProperty(table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    this->visit_entriesProperty(table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    this->visit_simpleProperty(table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
  return 0;
}

void* AstVisitor::visit_keyProperty(Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  this->visit_keyElementList(key_prop->keyProperty.keyelem_list);
  return 0;
}

void* AstVisitor::visit_keyElementList(Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_keyElement(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_keyElement(Ast* element)
{
  assert(element->kind == AST_keyElement);
  this->visit_expression(element->keyElement.expr);
  this->visit_name(element->keyElement.match);
  return 0;
}

void* AstVisitor::visit_actionsProperty(Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  this->visit_actionList(actions_prop->actionsProperty.action_list);
  return 0;
}

void* AstVisitor::visit_actionList(Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_actionRef(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_actionRef(Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  this->visit_name(action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    this->visit_argumentList(action_ref->actionRef.args);
  }
  return 0;
}

#if 0
void* AstVisitor::visit_entriesProperty(Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  this->visit_entriesList(entries_prop->entriesProperty.entries_list);
  return 0;
}

void* AstVisitor::visit_entriesList(Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_entry(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_entry(Ast* entry)
{
  assert(entry->kind == AST_entry);
  this->visit_keysetExpression(entry->entry.keyset);
  this->visit_actionRef(entry->entry.action);
  return 0;
}

void* AstVisitor::visit_simpleProperty(Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  this->visit_name(simple_prop->simpleProperty.name);
  this->visit_expression(simple_prop->simpleProperty.init_expr);
  return 0;
}
#endif

void* AstVisitor::visit_actionDeclaration(Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  this->visit_name(action_decl->actionDeclaration.name);
  this->visit_parameterList(action_decl->actionDeclaration.params);
  this->visit_blockStatement(action_decl->actionDeclaration.stmt);
  return 0;
}

/** VARIABLES **/

void* AstVisitor::visit_variableDeclaration(Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
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
  assert(func_decl->kind == AST_functionDeclaration);
  this->visit_functionPrototype(func_decl->functionDeclaration.proto);
  this->visit_blockStatement(func_decl->functionDeclaration.stmt);
  return 0;
}

void* AstVisitor::visit_argumentList(Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_argument(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_argument(Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    this->visit_expression(arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    this->visit_dontcare(arg->argument.arg);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_expressionList(Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    this->visit_expression(container_of(ast, Ast, tree));
  }
  return 0;
}

void* AstVisitor::visit_lvalueExpression(Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    this->visit_name(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    this->visit_memberSelector(lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    this->visit_arraySubscript(lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_expression(Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    this->visit_expression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    this->visit_booleanLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    this->visit_integerLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    this->visit_stringLiteral(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    this->visit_name(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    this->visit_expressionList(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    this->visit_castExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    this->visit_unaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    this->visit_binaryExpression(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    this->visit_memberSelector(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    this->visit_arraySubscript(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    this->visit_functionCall(expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    this->visit_assignmentStatement(expr->expression.expr);
  } else assert(0);
  return 0;
}

void* AstVisitor::visit_castExpression(Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  this->visit_typeRef(cast_expr->castExpression.type);
  this->visit_expression(cast_expr->castExpression.expr);
  return 0;
}

void* AstVisitor::visit_unaryExpression(Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  this->visit_expression(unary_expr->unaryExpression.operand);
  return 0;
}

void* AstVisitor::visit_binaryExpression(Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  this->visit_expression(binary_expr->binaryExpression.left_operand);
  this->visit_expression(binary_expr->binaryExpression.right_operand);
  return 0;
}

void* AstVisitor::visit_memberSelector(Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    this->visit_expression(selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    this->visit_lvalueExpression(selector->memberSelector.lhs_expr);
  } else assert(0);
  this->visit_name(selector->memberSelector.name);
  return 0;
}

void* AstVisitor::visit_arraySubscript(Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    this->visit_expression(subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    this->visit_lvalueExpression(subscript->arraySubscript.lhs_expr);
  } else assert(0);
  this->visit_indexExpression(subscript->arraySubscript.index_expr);
  return 0;
}

void* AstVisitor::visit_indexExpression(Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  this->visit_expression(index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    this->visit_expression(index_expr->indexExpression.end_index);
  }
  return 0;
}

void* AstVisitor::visit_booleanLiteral(Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  return 0;
}

void* AstVisitor::visit_integerLiteral(Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  return 0;
}

void* AstVisitor::visit_stringLiteral(Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  return 0;
}

void* AstVisitor::visit_default(Ast* default_)
{
  assert(default_->kind == AST_default);
  return 0;
}

void* AstVisitor::visit_dontcare(Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  return 0;
}

void ast_visitor_init(AstVisitor* visitor)
{ }
