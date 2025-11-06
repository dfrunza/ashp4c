#include "ast_visitor.h"

/** PROGRAM **/

static void* visit_p4program(AstVisitor* visitor, Ast* p4program)
{
  assert(p4program->kind == AST_p4program);
  visitor->visit_declarationList(visitor, p4program->p4program.decl_list);
  return 0;
}

static void* visit_declarationList(AstVisitor* visitor, Ast* decl_list)
{
  assert(decl_list->kind == AST_declarationList);
  AstTree* ast;

  for (ast = decl_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_declaration(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_declaration(AstVisitor* visitor, Ast* decl)
{
  assert(decl->kind == AST_declaration);
  if (decl->declaration.decl->kind == AST_variableDeclaration) {
    visitor->visit_variableDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_externDeclaration) {
    visitor->visit_externDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_actionDeclaration) {
    visitor->visit_actionDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_functionDeclaration) {
    visitor->visit_functionDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserDeclaration) {
    visitor->visit_parserDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_parserTypeDeclaration) {
    visitor->visit_parserTypeDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlDeclaration) {
    visitor->visit_controlDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_controlTypeDeclaration) {
    visitor->visit_controlTypeDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_typeDeclaration) {
    visitor->visit_typeDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_errorDeclaration) {
    visitor->visit_errorDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_matchKindDeclaration) {
    visitor->visit_matchKindDeclaration(visitor, decl->declaration.decl);
  } else if (decl->declaration.decl->kind == AST_instantiation) {
    visitor->visit_instantiation(visitor, decl->declaration.decl);
  } else assert(0);
  return 0;
}

static void* visit_name(AstVisitor* visitor, Ast* name)
{
  assert(name->kind == AST_name);
  return 0;
}

static void* visit_parameterList(AstVisitor* visitor, Ast* params)
{
  assert(params->kind == AST_parameterList);
  AstTree* ast;

  for (ast = params->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_parameter(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_parameter(AstVisitor* visitor, Ast* param)
{
  assert(param->kind == AST_parameter);
  visitor->visit_typeRef(visitor, param->parameter.type);
  visitor->visit_name(visitor, param->parameter.name);
  if (param->parameter.init_expr) {
    visitor->visit_expression(visitor, param->parameter.init_expr);
  }
  return 0;
}

static void* visit_packageTypeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_packageTypeDeclaration);
  visitor->visit_name(visitor, type_decl->packageTypeDeclaration.name);
  visitor->visit_parameterList(visitor, type_decl->packageTypeDeclaration.params);
  return 0;
}

static void* visit_instantiation(AstVisitor* visitor, Ast* inst)
{
  assert(inst->kind == AST_instantiation);
  visitor->visit_typeRef(visitor, inst->instantiation.type);
  visitor->visit_argumentList(visitor, inst->instantiation.args);
  visitor->visit_name(visitor, inst->instantiation.name);
  return 0;
}

/** PARSER **/

static void* visit_parserDeclaration(AstVisitor* visitor, Ast* parser_decl)
{
  assert(parser_decl->kind == AST_parserDeclaration);
  visitor->visit_typeDeclaration(visitor, parser_decl->parserDeclaration.proto);
  if (parser_decl->parserDeclaration.ctor_params) {
    visitor->visit_parameterList(visitor, parser_decl->parserDeclaration.ctor_params);
  }
  visitor->visit_parserLocalElements(visitor, parser_decl->parserDeclaration.local_elements);
  visitor->visit_parserStates(visitor, parser_decl->parserDeclaration.states);
  return 0;
}

static void* visit_parserTypeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_parserTypeDeclaration);
  visitor->visit_name(visitor, type_decl->parserTypeDeclaration.name);
  visitor->visit_parameterList(visitor, type_decl->parserTypeDeclaration.params);
  return 0;
}

static void* visit_parserLocalElements(AstVisitor* visitor, Ast* local_elements)
{
  assert(local_elements->kind == AST_parserLocalElements);
  AstTree* ast;

  for (ast = local_elements->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_parserLocalElement(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_parserLocalElement(AstVisitor* visitor, Ast* local_element)
{
  assert(local_element->kind == AST_parserLocalElement);
  if (local_element->parserLocalElement.element->kind == AST_variableDeclaration) {
    visitor->visit_variableDeclaration(visitor, local_element->parserLocalElement.element);
  } else if (local_element->parserLocalElement.element->kind == AST_instantiation) {
    visitor->visit_instantiation(visitor, local_element->parserLocalElement.element);
  } else assert(0);
  return 0;
}

static void* visit_parserStates(AstVisitor* visitor, Ast* states)
{
  assert(states->kind == AST_parserStates);
  AstTree* ast;

  for (ast = states->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_parserState(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_parserState(AstVisitor* visitor, Ast* state)
{
  assert(state->kind == AST_parserState);
  visitor->visit_name(visitor, state->parserState.name);
  visitor->visit_parserStatements(visitor, state->parserState.stmt_list);
  visitor->visit_transitionStatement(visitor, state->parserState.transition_stmt);
  return 0;
}

static void* visit_parserStatements(AstVisitor* visitor, Ast* stmts)
{
  assert(stmts->kind == AST_parserStatements);
  AstTree* ast;

  for (ast = stmts->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_parserStatement(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_parserStatement(AstVisitor* visitor, Ast* stmt)
{
  assert(stmt->kind == AST_parserStatement);
  if (stmt->parserStatement.stmt->kind == AST_assignmentStatement) {
    visitor->visit_assignmentStatement(visitor, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_functionCall) {
    visitor->visit_functionCall(visitor, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_directApplication) {
    visitor->visit_directApplication(visitor, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_parserBlockStatement) {
    visitor->visit_parserBlockStatement(visitor, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_variableDeclaration) {
    visitor->visit_variableDeclaration(visitor, stmt->parserStatement.stmt);
  } else if (stmt->parserStatement.stmt->kind == AST_emptyStatement) {
    ;
  } else assert(0);
  return 0;
}

static void* visit_parserBlockStatement(AstVisitor* visitor, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_parserBlockStatement);
  visitor->visit_parserStatements(visitor, block_stmt->parserBlockStatement.stmt_list);
  return 0;
}

static void* visit_transitionStatement(AstVisitor* visitor, Ast* transition_stmt)
{
  assert(transition_stmt->kind == AST_transitionStatement);
  visitor->visit_stateExpression(visitor, transition_stmt->transitionStatement.stmt);
  return 0;
}

static void* visit_stateExpression(AstVisitor* visitor, Ast* state_expr)
{
  assert(state_expr->kind == AST_stateExpression);
  if (state_expr->stateExpression.expr->kind == AST_name) {
    visitor->visit_name(visitor, state_expr->stateExpression.expr);
  } else if (state_expr->stateExpression.expr->kind == AST_selectExpression) {
    visitor->visit_selectExpression(visitor, state_expr->stateExpression.expr);
  } else assert(0);
  return 0;
}

static void* visit_selectExpression(AstVisitor* visitor, Ast* select_expr)
{
  assert(select_expr->kind == AST_selectExpression);
  visitor->visit_expressionList(visitor, select_expr->selectExpression.expr_list);
  visitor->visit_selectCaseList(visitor, select_expr->selectExpression.case_list);
  return 0;
}

static void* visit_selectCaseList(AstVisitor* visitor, Ast* case_list)
{
  assert(case_list->kind == AST_selectCaseList);
  AstTree* ast;

  for (ast = case_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_selectCase(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_selectCase(AstVisitor* visitor, Ast* select_case)
{
  assert(select_case->kind == AST_selectCase);
  visitor->visit_keysetExpression(visitor, select_case->selectCase.keyset_expr);
  visitor->visit_name(visitor, select_case->selectCase.name);
  return 0;
}

static void* visit_keysetExpression(AstVisitor* visitor, Ast* keyset_expr)
{
  assert(keyset_expr->kind == AST_keysetExpression);
  if (keyset_expr->keysetExpression.expr->kind == AST_tupleKeysetExpression) {
    visitor->visit_tupleKeysetExpression(visitor, keyset_expr->keysetExpression.expr);
  } else if (keyset_expr->keysetExpression.expr->kind == AST_simpleKeysetExpression) {
    visitor->visit_simpleKeysetExpression(visitor, keyset_expr->keysetExpression.expr);
  } else assert(0);
  return 0;
}

static void* visit_tupleKeysetExpression(AstVisitor* visitor, Ast* tuple_expr)
{
  assert(tuple_expr->kind == AST_tupleKeysetExpression);
  visitor->visit_simpleExpressionList(visitor, tuple_expr->tupleKeysetExpression.expr_list);
  return 0;
}

static void* visit_simpleKeysetExpression(AstVisitor* visitor, Ast* simple_expr)
{
  assert(simple_expr->kind == AST_simpleKeysetExpression);
  if (simple_expr->simpleKeysetExpression.expr->kind == AST_expression) {
    visitor->visit_expression(visitor, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_default) {
    visitor->visit_default(visitor, simple_expr->simpleKeysetExpression.expr);
  } else if (simple_expr->simpleKeysetExpression.expr->kind == AST_dontcare) {
    visitor->visit_dontcare(visitor, simple_expr->simpleKeysetExpression.expr);
  } else assert(0);
  return 0;
}

static void* visit_simpleExpressionList(AstVisitor* visitor, Ast* expr_list)
{
  assert(expr_list->kind == AST_simpleExpressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_simpleKeysetExpression(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

/** CONTROL **/

static void* visit_controlDeclaration(AstVisitor* visitor, Ast* control_decl)
{
  assert(control_decl->kind == AST_controlDeclaration);
  visitor->visit_typeDeclaration(visitor, control_decl->controlDeclaration.proto);
  if (control_decl->controlDeclaration.ctor_params) {
    visitor->visit_parameterList(visitor, control_decl->controlDeclaration.ctor_params);
  }
  visitor->visit_controlLocalDeclarations(visitor, control_decl->controlDeclaration.local_decls);
  visitor->visit_blockStatement(visitor, control_decl->controlDeclaration.apply_stmt);
  return 0;
}

static void* visit_controlTypeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_controlTypeDeclaration);
  visitor->visit_name(visitor, type_decl->controlTypeDeclaration.name);
  visitor->visit_parameterList(visitor, type_decl->controlTypeDeclaration.params);
  return 0;
}

static void* visit_controlLocalDeclarations(AstVisitor* visitor, Ast* local_decls)
{
  assert(local_decls->kind == AST_controlLocalDeclarations);
  AstTree* ast;

  for (ast = local_decls->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_controlLocalDeclaration(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_controlLocalDeclaration(AstVisitor* visitor, Ast* local_decl)
{
  assert(local_decl->kind == AST_controlLocalDeclaration);
  if (local_decl->controlLocalDeclaration.decl->kind == AST_variableDeclaration) {
    visitor->visit_variableDeclaration(visitor, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_actionDeclaration) {
    visitor->visit_actionDeclaration(visitor, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_tableDeclaration) {
    visitor->visit_tableDeclaration(visitor, local_decl->controlLocalDeclaration.decl);
  } else if (local_decl->controlLocalDeclaration.decl->kind == AST_instantiation) {
    visitor->visit_instantiation(visitor, local_decl->controlLocalDeclaration.decl);
  } else assert(0);
  return 0;
}

/** EXTERN **/

static void* visit_externDeclaration(AstVisitor* visitor, Ast* extern_decl)
{
  assert(extern_decl->kind == AST_externDeclaration);
  if (extern_decl->externDeclaration.decl->kind == AST_externTypeDeclaration) {
    visitor->visit_externTypeDeclaration(visitor, extern_decl->externDeclaration.decl);
  } else if (extern_decl->externDeclaration.decl->kind == AST_functionPrototype) {
    visitor->visit_functionPrototype(visitor, extern_decl->externDeclaration.decl);
  } else assert(0);
  return 0;
}

static void* visit_externTypeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_externTypeDeclaration);
  visitor->visit_name(visitor, type_decl->externTypeDeclaration.name);
  visitor->visit_methodPrototypes(visitor, type_decl->externTypeDeclaration.method_protos);
  return 0;
}

static void* visit_methodPrototypes(AstVisitor* visitor, Ast* protos)
{
  assert(protos->kind == AST_methodPrototypes);
  AstTree* ast;

  for (ast = protos->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_functionPrototype(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_functionPrototype(AstVisitor* visitor, Ast* func_proto)
{
  assert(func_proto->kind == AST_functionPrototype);
  if (func_proto->functionPrototype.return_type) {
    visitor->visit_typeRef(visitor, func_proto->functionPrototype.return_type);
  }
  visitor->visit_name(visitor, func_proto->functionPrototype.name);
  visitor->visit_parameterList(visitor, func_proto->functionPrototype.params);
  return 0;
}

/** TYPES **/

static void* visit_typeRef(AstVisitor* visitor, Ast* type_ref)
{
  assert(type_ref->kind == AST_typeRef);
  if (type_ref->typeRef.type->kind == AST_baseTypeBoolean) {
    visitor->visit_baseTypeBoolean(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeInteger) {
    visitor->visit_baseTypeInteger(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeBit) {
    visitor->visit_baseTypeBit(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVarbit) {
    visitor->visit_baseTypeVarbit(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeString) {
    visitor->visit_baseTypeString(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeVoid) {
    visitor->visit_baseTypeVoid(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_baseTypeError) {
    visitor->visit_baseTypeError(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_name) {
    visitor->visit_name(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_headerStackType) {
    visitor->visit_headerStackType(visitor, type_ref->typeRef.type);
  } else if (type_ref->typeRef.type->kind == AST_tupleType) {
    visitor->visit_tupleType(visitor, type_ref->typeRef.type);
  } else assert(0);
  return 0;
}

static void* visit_tupleType(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_tupleType);
  visitor->visit_typeArgumentList(visitor, type_decl->tupleType.type_args);
  return 0;
}

static void* visit_headerStackType(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_headerStackType);
  visitor->visit_typeRef(visitor, type_decl->headerStackType.type);
  visitor->visit_expression(visitor, type_decl->headerStackType.stack_expr);
  return 0;
}

static void* visit_baseTypeBoolean(AstVisitor* visitor, Ast* bool_type)
{
  assert(bool_type->kind == AST_baseTypeBoolean);
  visitor->visit_name(visitor, bool_type->baseTypeBoolean.name);
  return 0;
}

static void* visit_baseTypeInteger(AstVisitor* visitor, Ast* int_type)
{
  assert(int_type->kind == AST_baseTypeInteger);
  visitor->visit_name(visitor, int_type->baseTypeInteger.name);
  if (int_type->baseTypeInteger.size) {
    visitor->visit_integerTypeSize(visitor, int_type->baseTypeInteger.size);
  }
  return 0;
}

static void* visit_baseTypeBit(AstVisitor* visitor, Ast* bit_type)
{
  assert(bit_type->kind == AST_baseTypeBit);
  visitor->visit_name(visitor, bit_type->baseTypeBit.name);
  if (bit_type->baseTypeBit.size) {
    visitor->visit_integerTypeSize(visitor, bit_type->baseTypeBit.size);
  }
  return 0;
}

static void* visit_baseTypeVarbit(AstVisitor* visitor, Ast* varbit_type)
{
  assert(varbit_type->kind == AST_baseTypeVarbit);
  visitor->visit_name(visitor, varbit_type->baseTypeVarbit.name);
  visitor->visit_integerTypeSize(visitor, varbit_type->baseTypeVarbit.size);
  return 0;
}

static void* visit_baseTypeString(AstVisitor* visitor, Ast* str_type)
{
  assert(str_type->kind == AST_baseTypeString);
  visitor->visit_name(visitor, str_type->baseTypeString.name);
  return 0;
}

static void* visit_baseTypeVoid(AstVisitor* visitor, Ast* void_type)
{
  assert(void_type->kind == AST_baseTypeVoid);
  visitor->visit_name(visitor, void_type->baseTypeVoid.name);
  return 0;
}

static void* visit_baseTypeError(AstVisitor* visitor, Ast* error_type)
{
  assert(error_type->kind == AST_baseTypeError);
  visitor->visit_name(visitor, error_type->baseTypeError.name);
  return 0;
}

static void* visit_integerTypeSize(AstVisitor* visitor, Ast* type_size)
{
  assert(type_size->kind == AST_integerTypeSize);
  return 0;
}

static void* visit_realTypeArg(AstVisitor* visitor, Ast* type_arg)
{
  assert(type_arg->kind == AST_realTypeArg);
  if (type_arg->realTypeArg.arg->kind == AST_typeRef) {
    visitor->visit_typeRef(visitor, type_arg->realTypeArg.arg);
  } else if (type_arg->realTypeArg.arg->kind == AST_dontcare) {
    visitor->visit_dontcare(visitor, type_arg->realTypeArg.arg);
  } else assert(0);
  return 0;
}

static void* visit_typeArg(AstVisitor* visitor, Ast* type_arg)
{
  assert(type_arg->kind == AST_typeArg);
  if (type_arg->typeArg.arg->kind == AST_typeRef) {
    visitor->visit_typeRef(visitor, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_name) {
    visitor->visit_name(visitor, type_arg->typeArg.arg);
  } else if (type_arg->typeArg.arg->kind == AST_dontcare) {
    visitor->visit_dontcare(visitor, type_arg->typeArg.arg);
  } else assert(0);
  return 0;
}

static void* visit_typeArgumentList(AstVisitor* visitor, Ast* arg_list)
{
  assert(arg_list->kind == AST_typeArgumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_typeArg(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_typeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_typeDeclaration);
  if (type_decl->typeDeclaration.decl->kind == AST_derivedTypeDeclaration) {
    visitor->visit_derivedTypeDeclaration(visitor, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_typedefDeclaration) {
    visitor->visit_typedefDeclaration(visitor, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_parserTypeDeclaration) {
    visitor->visit_parserTypeDeclaration(visitor, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_controlTypeDeclaration) {
    visitor->visit_controlTypeDeclaration(visitor, type_decl->typeDeclaration.decl);
  } else if (type_decl->typeDeclaration.decl->kind == AST_packageTypeDeclaration) {
    visitor->visit_packageTypeDeclaration(visitor, type_decl->typeDeclaration.decl);
  } else assert(0);
  return 0;
}

static void* visit_derivedTypeDeclaration(AstVisitor* visitor, Ast* type_decl)
{
  assert(type_decl->kind == AST_derivedTypeDeclaration);
  if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerTypeDeclaration) {
    visitor->visit_headerTypeDeclaration(visitor, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_headerUnionDeclaration) {
    visitor->visit_headerUnionDeclaration(visitor, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_structTypeDeclaration) {
    visitor->visit_structTypeDeclaration(visitor, type_decl->derivedTypeDeclaration.decl);
  } else if (type_decl->derivedTypeDeclaration.decl->kind == AST_enumDeclaration) {
    visitor->visit_enumDeclaration(visitor, type_decl->derivedTypeDeclaration.decl);
  } else assert(0);
  return 0;
}

static void* visit_headerTypeDeclaration(AstVisitor* visitor, Ast* header_decl)
{
  assert(header_decl->kind == AST_headerTypeDeclaration);
  visitor->visit_name(visitor, header_decl->headerTypeDeclaration.name);
  visitor->visit_structFieldList(visitor, header_decl->headerTypeDeclaration.fields);
  return 0;
}

static void* visit_headerUnionDeclaration(AstVisitor* visitor, Ast* union_decl)
{
  assert(union_decl->kind == AST_headerUnionDeclaration);
  visitor->visit_name(visitor, union_decl->headerUnionDeclaration.name);
  visitor->visit_structFieldList(visitor, union_decl->headerUnionDeclaration.fields);
  return 0;
}

static void* visit_structTypeDeclaration(AstVisitor* visitor, Ast* struct_decl)
{
  assert(struct_decl->kind == AST_structTypeDeclaration);
  visitor->visit_name(visitor, struct_decl->structTypeDeclaration.name);
  visitor->visit_structFieldList(visitor, struct_decl->structTypeDeclaration.fields);
  return 0;
}

static void* visit_structFieldList(AstVisitor* visitor, Ast* field_list)
{
  assert(field_list->kind == AST_structFieldList);
  AstTree* ast;

  for (ast = field_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_structField(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_structField(AstVisitor* visitor, Ast* field)
{
  assert(field->kind == AST_structField);
  visitor->visit_typeRef(visitor, field->structField.type);
  visitor->visit_name(visitor, field->structField.name);
  return 0;
}

static void* visit_enumDeclaration(AstVisitor* visitor, Ast* enum_decl)
{
  assert(enum_decl->kind == AST_enumDeclaration);
  visitor->visit_name(visitor, enum_decl->enumDeclaration.name);
  visitor->visit_specifiedIdentifierList(visitor, enum_decl->enumDeclaration.fields);
  return 0;
}

static void* visit_errorDeclaration(AstVisitor* visitor, Ast* error_decl)
{
  assert(error_decl->kind == AST_errorDeclaration);
  visitor->visit_identifierList(visitor, error_decl->errorDeclaration.fields);
  return 0;
}

static void* visit_matchKindDeclaration(AstVisitor* visitor, Ast* match_decl)
{
  assert(match_decl->kind == AST_matchKindDeclaration);
  visitor->visit_identifierList(visitor, match_decl->matchKindDeclaration.fields);
  return 0;
}

static void* visit_identifierList(AstVisitor* visitor, Ast* ident_list)
{
  assert(ident_list->kind == AST_identifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_name(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_specifiedIdentifierList(AstVisitor* visitor, Ast* ident_list)
{
  assert(ident_list->kind == AST_specifiedIdentifierList);
  AstTree* ast;

  for (ast = ident_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_specifiedIdentifier(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_specifiedIdentifier(AstVisitor* visitor, Ast* ident)
{
  assert(ident->kind == AST_specifiedIdentifier);
  visitor->visit_name(visitor, ident->specifiedIdentifier.name);
  if (ident->specifiedIdentifier.init_expr) {
    visitor->visit_expression(visitor, ident->specifiedIdentifier.init_expr);
  }
  return 0;
}

static void* visit_typedefDeclaration(AstVisitor* visitor, Ast* typedef_decl)
{
  assert(typedef_decl->kind == AST_typedefDeclaration);
  if (typedef_decl->typedefDeclaration.type_ref->kind == AST_typeRef) {
    visitor->visit_typeRef(visitor, typedef_decl->typedefDeclaration.type_ref);
  } else if (typedef_decl->typedefDeclaration.type_ref->kind == AST_derivedTypeDeclaration) {
    visitor->visit_derivedTypeDeclaration(visitor, typedef_decl->typedefDeclaration.type_ref);
  } else assert(0);
  visitor->visit_name(visitor, typedef_decl->typedefDeclaration.name);
  return 0;
}

/** STATEMENTS **/

static void* visit_assignmentStatement(AstVisitor* visitor, Ast* assign_stmt)
{
  assert(assign_stmt->kind == AST_assignmentStatement);
  if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_expression) {
    visitor->visit_expression(visitor, assign_stmt->assignmentStatement.lhs_expr);
  } else if (assign_stmt->assignmentStatement.lhs_expr->kind == AST_lvalueExpression) {
    visitor->visit_lvalueExpression(visitor, assign_stmt->assignmentStatement.lhs_expr);
  } else assert(0);
  visitor->visit_expression(visitor, assign_stmt->assignmentStatement.rhs_expr);
  return 0;
}

static void* visit_functionCall(AstVisitor* visitor, Ast* func_call)
{
  assert(func_call->kind == AST_functionCall);
  if (func_call->functionCall.lhs_expr->kind == AST_expression) {
    visitor->visit_expression(visitor, func_call->functionCall.lhs_expr);
  } else if (func_call->functionCall.lhs_expr->kind == AST_lvalueExpression) {
    visitor->visit_lvalueExpression(visitor, func_call->functionCall.lhs_expr);
  } else assert(0);
  visitor->visit_argumentList(visitor, func_call->functionCall.args);
  return 0;
}

static void* visit_returnStatement(AstVisitor* visitor, Ast* return_stmt)
{
  assert(return_stmt->kind == AST_returnStatement);
  if (return_stmt->returnStatement.expr) {
    visitor->visit_expression(visitor, return_stmt->returnStatement.expr);
  }
  return 0;
}

static void* visit_exitStatement(AstVisitor* visitor, Ast* exit_stmt)
{
  assert(exit_stmt->kind == AST_exitStatement);
  return 0;
}

static void* visit_conditionalStatement(AstVisitor* visitor, Ast* cond_stmt)
{
  assert(cond_stmt->kind == AST_conditionalStatement);
  visitor->visit_expression(visitor, cond_stmt->conditionalStatement.cond_expr);
  visitor->visit_statement(visitor, cond_stmt->conditionalStatement.stmt);
  if (cond_stmt->conditionalStatement.else_stmt) {
    visitor->visit_statement(visitor, cond_stmt->conditionalStatement.else_stmt);
  }
  return 0;
}

static void* visit_directApplication(AstVisitor* visitor, Ast* applic_stmt)
{
  assert(applic_stmt->kind == AST_directApplication);
  if (applic_stmt->directApplication.name->kind == AST_name) {
    visitor->visit_name(visitor, applic_stmt->directApplication.name);
  } else if (applic_stmt->directApplication.name->kind == AST_typeRef) {
    visitor->visit_typeRef(visitor, applic_stmt->directApplication.name);
  } else assert(0);
  visitor->visit_argumentList(visitor, applic_stmt->directApplication.args);
  return 0;
}

static void* visit_statement(AstVisitor* visitor, Ast* stmt)
{
  assert(stmt->kind == AST_statement);
  if (stmt->statement.stmt->kind == AST_assignmentStatement) {
    visitor->visit_assignmentStatement(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_functionCall) {
    visitor->visit_functionCall(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_directApplication) {
    visitor->visit_directApplication(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_conditionalStatement) {
    visitor->visit_conditionalStatement(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_emptyStatement) {
    ;
  } else if (stmt->statement.stmt->kind == AST_blockStatement) {
    visitor->visit_blockStatement(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_exitStatement) {
    visitor->visit_exitStatement(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_returnStatement) {
    visitor->visit_returnStatement(visitor, stmt->statement.stmt);
  } else if (stmt->statement.stmt->kind == AST_switchStatement) {
    visitor->visit_switchStatement(visitor, stmt->statement.stmt);
  } else assert(0);
  return 0;
}

static void* visit_blockStatement(AstVisitor* visitor, Ast* block_stmt)
{
  assert(block_stmt->kind == AST_blockStatement);
  visitor->visit_statementOrDeclList(visitor, block_stmt->blockStatement.stmt_list);
  return 0;
}

static void* visit_statementOrDeclList(AstVisitor* visitor, Ast* stmt_list)
{
  assert(stmt_list->kind == AST_statementOrDeclList);
  AstTree* ast;

  for (ast = stmt_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_statementOrDeclaration(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_switchStatement(AstVisitor* visitor, Ast* switch_stmt)
{
  assert(switch_stmt->kind == AST_switchStatement);
  visitor->visit_expression(visitor, switch_stmt->switchStatement.expr);
  visitor->visit_switchCases(visitor, switch_stmt->switchStatement.switch_cases);
  return 0;
}

static void* visit_switchCases(AstVisitor* visitor, Ast* switch_cases)
{
  assert(switch_cases->kind == AST_switchCases);
  AstTree* ast;

  for (ast = switch_cases->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_switchCase(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_switchCase(AstVisitor* visitor, Ast* switch_case)
{
  assert(switch_case->kind == AST_switchCase);
  visitor->visit_switchLabel(visitor, switch_case->switchCase.label);
  if (switch_case->switchCase.stmt) {
    visitor->visit_blockStatement(visitor, switch_case->switchCase.stmt);
  }
  return 0;
}

static void* visit_switchLabel(AstVisitor* visitor, Ast* label)
{
  assert(label->kind == AST_switchLabel);
  if (label->switchLabel.label->kind == AST_name) {
    visitor->visit_name(visitor, label->switchLabel.label);
  } else if (label->switchLabel.label->kind == AST_default) {
    visitor->visit_default(visitor, label->switchLabel.label);
  } else assert(0);
  return 0;
}

static void* visit_statementOrDeclaration(AstVisitor* visitor, Ast* stmt)
{
  assert(stmt->kind == AST_statementOrDeclaration);
  if (stmt->statementOrDeclaration.stmt->kind == AST_variableDeclaration) {
    visitor->visit_variableDeclaration(visitor, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_statement) {
    visitor->visit_statement(visitor, stmt->statementOrDeclaration.stmt);
  } else if (stmt->statementOrDeclaration.stmt->kind == AST_instantiation) {
    visitor->visit_instantiation(visitor, stmt->statementOrDeclaration.stmt);
  } else assert(0);
  return 0;
}

/** TABLES **/

static void* visit_tableDeclaration(AstVisitor* visitor, Ast* table_decl)
{
  assert(table_decl->kind == AST_tableDeclaration);
  visitor->visit_name(visitor, table_decl->tableDeclaration.name);
  visitor->visit_tablePropertyList(visitor, table_decl->tableDeclaration.prop_list);
  return 0;
}

static void* visit_tablePropertyList(AstVisitor* visitor, Ast* prop_list)
{
  assert(prop_list->kind == AST_tablePropertyList);
  AstTree* ast;

  for (ast = prop_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_tableProperty(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_tableProperty(AstVisitor* visitor, Ast* table_prop)
{
  assert(table_prop->kind == AST_tableProperty);
  if (table_prop->tableProperty.prop->kind == AST_keyProperty) {
    visitor->visit_keyProperty(visitor, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_actionsProperty) {
    visitor->visit_actionsProperty(visitor, table_prop->tableProperty.prop);
  }
#if 0
  else if (table_prop->tableProperty.prop->kind == AST_entriesProperty) {
    visitor->visit_entriesProperty(visitor, table_prop->tableProperty.prop);
  } else if (table_prop->tableProperty.prop->kind == AST_simpleProperty) {
    visitor->visit_simpleProperty(visitor, table_prop->tableProperty.prop);
  }
#endif
  else assert(0);
  return 0;
}

static void* visit_keyProperty(AstVisitor* visitor, Ast* key_prop)
{
  assert(key_prop->kind == AST_keyProperty);
  visitor->visit_keyElementList(visitor, key_prop->keyProperty.keyelem_list);
  return 0;
}

static void* visit_keyElementList(AstVisitor* visitor, Ast* element_list)
{
  assert(element_list->kind == AST_keyElementList);
  AstTree* ast;

  for (ast = element_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_keyElement(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_keyElement(AstVisitor* visitor, Ast* element)
{
  assert(element->kind == AST_keyElement);
  visitor->visit_expression(visitor, element->keyElement.expr);
  visitor->visit_name(visitor, element->keyElement.match);
  return 0;
}

static void* visit_actionsProperty(AstVisitor* visitor, Ast* actions_prop)
{
  assert(actions_prop->kind == AST_actionsProperty);
  visitor->visit_actionList(visitor, actions_prop->actionsProperty.action_list);
  return 0;
}

static void* visit_actionList(AstVisitor* visitor, Ast* action_list)
{
  assert(action_list->kind == AST_actionList);
  AstTree* ast;

  for (ast = action_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_actionRef(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_actionRef(AstVisitor* visitor, Ast* action_ref)
{
  assert(action_ref->kind == AST_actionRef);
  visitor->visit_name(visitor, action_ref->actionRef.name);
  if (action_ref->actionRef.args) {
    visitor->visit_argumentList(visitor, action_ref->actionRef.args);
  }
  return 0;
}

#if 0
static void* visit_entriesProperty(AstVisitor* visitor, Ast* entries_prop)
{
  assert(entries_prop->kind == AST_entriesProperty);
  visitor->visit_entriesList(visitor, entries_prop->entriesProperty.entries_list);
  return 0;
}

static void* visit_entriesList(AstVisitor* visitor, Ast* entries_list)
{
  assert(entries_list->kind == AST_entriesList);
  AstTree* ast;

  for (ast = entries_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_entry(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_entry(AstVisitor* visitor, Ast* entry)
{
  assert(entry->kind == AST_entry);
  visitor->visit_keysetExpression(visitor, entry->entry.keyset);
  visitor->visit_actionRef(visitor, entry->entry.action);
  return 0;
}

static void* visit_simpleProperty(AstVisitor* visitor, Ast* simple_prop)
{
  assert(simple_prop->kind == AST_simpleProperty);
  visitor->visit_name(visitor, simple_prop->simpleProperty.name);
  visitor->visit_expression(visitor, simple_prop->simpleProperty.init_expr);
  return 0;
}
#endif

static void* visit_actionDeclaration(AstVisitor* visitor, Ast* action_decl)
{
  assert(action_decl->kind == AST_actionDeclaration);
  visitor->visit_name(visitor, action_decl->actionDeclaration.name);
  visitor->visit_parameterList(visitor, action_decl->actionDeclaration.params);
  visitor->visit_blockStatement(visitor, action_decl->actionDeclaration.stmt);
  return 0;
}

/** VARIABLES **/

static void* visit_variableDeclaration(AstVisitor* visitor, Ast* var_decl)
{
  assert(var_decl->kind == AST_variableDeclaration);
  visitor->visit_typeRef(visitor, var_decl->variableDeclaration.type);
  visitor->visit_name(visitor, var_decl->variableDeclaration.name);
  if (var_decl->variableDeclaration.init_expr) {
    visitor->visit_expression(visitor, var_decl->variableDeclaration.init_expr);
  }
  return 0;
}

/** EXPRESSIONS **/

static void* visit_functionDeclaration(AstVisitor* visitor, Ast* func_decl)
{
  assert(func_decl->kind == AST_functionDeclaration);
  visitor->visit_functionPrototype(visitor, func_decl->functionDeclaration.proto);
  visitor->visit_blockStatement(visitor, func_decl->functionDeclaration.stmt);
  return 0;
}

static void* visit_argumentList(AstVisitor* visitor, Ast* arg_list)
{
  assert(arg_list->kind == AST_argumentList);
  AstTree* ast;

  for (ast = arg_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_argument(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_argument(AstVisitor* visitor, Ast* arg)
{
  assert(arg->kind == AST_argument);
  if (arg->argument.arg->kind == AST_expression) {
    visitor->visit_expression(visitor, arg->argument.arg);
  } else if (arg->argument.arg->kind == AST_dontcare) {
    visitor->visit_dontcare(visitor, arg->argument.arg);
  } else assert(0);
  return 0;
}

static void* visit_expressionList(AstVisitor* visitor, Ast* expr_list)
{
  assert(expr_list->kind == AST_expressionList);
  AstTree* ast;

  for (ast = expr_list->tree.first_child;
       ast != 0; ast = ast->right_sibling) {
    visitor->visit_expression(visitor, container_of(ast, Ast, tree));
  }
  return 0;
}

static void* visit_lvalueExpression(AstVisitor* visitor, Ast* lvalue_expr)
{
  assert(lvalue_expr->kind == AST_lvalueExpression);
  if (lvalue_expr->lvalueExpression.expr->kind == AST_name) {
    visitor->visit_name(visitor, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_memberSelector) {
    visitor->visit_memberSelector(visitor, lvalue_expr->lvalueExpression.expr);
  } else if (lvalue_expr->lvalueExpression.expr->kind == AST_arraySubscript) {
    visitor->visit_arraySubscript(visitor, lvalue_expr->lvalueExpression.expr);
  } else assert(0);
  return 0;
}

static void* visit_expression(AstVisitor* visitor, Ast* expr)
{
  assert(expr->kind == AST_expression);
  if (expr->expression.expr->kind == AST_expression) {
    visitor->visit_expression(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_booleanLiteral) {
    visitor->visit_booleanLiteral(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_integerLiteral) {
    visitor->visit_integerLiteral(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_stringLiteral) {
    visitor->visit_stringLiteral(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_name) {
    visitor->visit_name(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_expressionList) {
    visitor->visit_expressionList(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_castExpression) {
    visitor->visit_castExpression(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_unaryExpression) {
    visitor->visit_unaryExpression(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_binaryExpression) {
    visitor->visit_binaryExpression(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_memberSelector) {
    visitor->visit_memberSelector(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_arraySubscript) {
    visitor->visit_arraySubscript(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_functionCall) {
    visitor->visit_functionCall(visitor, expr->expression.expr);
  } else if (expr->expression.expr->kind == AST_assignmentStatement) {
    visitor->visit_assignmentStatement(visitor, expr->expression.expr);
  } else assert(0);
  return 0;
}

static void* visit_castExpression(AstVisitor* visitor, Ast* cast_expr)
{
  assert(cast_expr->kind == AST_castExpression);
  visitor->visit_typeRef(visitor, cast_expr->castExpression.type);
  visitor->visit_expression(visitor, cast_expr->castExpression.expr);
  return 0;
}

static void* visit_unaryExpression(AstVisitor* visitor, Ast* unary_expr)
{
  assert(unary_expr->kind == AST_unaryExpression);
  visitor->visit_expression(visitor, unary_expr->unaryExpression.operand);
  return 0;
}

static void* visit_binaryExpression(AstVisitor* visitor, Ast* binary_expr)
{
  assert(binary_expr->kind == AST_binaryExpression);
  visitor->visit_expression(visitor, binary_expr->binaryExpression.left_operand);
  visitor->visit_expression(visitor, binary_expr->binaryExpression.right_operand);
  return 0;
}

static void* visit_memberSelector(AstVisitor* visitor, Ast* selector)
{
  assert(selector->kind == AST_memberSelector);
  if (selector->memberSelector.lhs_expr->kind == AST_expression) {
    visitor->visit_expression(visitor, selector->memberSelector.lhs_expr);
  } else if (selector->memberSelector.lhs_expr->kind == AST_lvalueExpression) {
    visitor->visit_lvalueExpression(visitor, selector->memberSelector.lhs_expr);
  } else assert(0);
  visitor->visit_name(visitor, selector->memberSelector.name);
  return 0;
}

static void* visit_arraySubscript(AstVisitor* visitor, Ast* subscript)
{
  assert(subscript->kind == AST_arraySubscript);
  if (subscript->arraySubscript.lhs_expr->kind == AST_expression) {
    visitor->visit_expression(visitor, subscript->arraySubscript.lhs_expr);
  } else if (subscript->arraySubscript.lhs_expr->kind == AST_lvalueExpression) {
    visitor->visit_lvalueExpression(visitor, subscript->arraySubscript.lhs_expr);
  } else assert(0);
  visitor->visit_indexExpression(visitor, subscript->arraySubscript.index_expr);
  return 0;
}

static void* visit_indexExpression(AstVisitor* visitor, Ast* index_expr)
{
  assert(index_expr->kind == AST_indexExpression);
  visitor->visit_expression(visitor, index_expr->indexExpression.start_index);
  if (index_expr->indexExpression.end_index) {
    visitor->visit_expression(visitor, index_expr->indexExpression.end_index);
  }
  return 0;
}

static void* visit_booleanLiteral(AstVisitor* visitor, Ast* bool_literal)
{
  assert(bool_literal->kind == AST_booleanLiteral);
  return 0;
}

static void* visit_integerLiteral(AstVisitor* visitor, Ast* int_literal)
{
  assert(int_literal->kind == AST_integerLiteral);
  return 0;
}

static void* visit_stringLiteral(AstVisitor* visitor, Ast* str_literal)
{
  assert(str_literal->kind == AST_stringLiteral);
  return 0;
}

static void* visit_default(AstVisitor* visitor, Ast* default_)
{
  assert(default_->kind == AST_default);
  return 0;
}

static void* visit_dontcare(AstVisitor* visitor, Ast* dontcare)
{
  assert(dontcare->kind == AST_dontcare);
  return 0;
}

void ast_visitor_init(AstVisitor* visitor)
{
  /** PROGRAM **/

  visitor->visit_p4program = visit_p4program;
  visitor->visit_declarationList = visit_declarationList;
  visitor->visit_declaration = visit_declaration;
  visitor->visit_name = visit_name;
  visitor->visit_parameterList = visit_parameterList;
  visitor->visit_parameter = visit_parameter;
  visitor->visit_packageTypeDeclaration = visit_packageTypeDeclaration;
  visitor->visit_instantiation = visit_instantiation;

  /** PARSER **/

  visitor->visit_parserDeclaration = visit_parserDeclaration;
  visitor->visit_parserTypeDeclaration = visit_parserTypeDeclaration;
  visitor->visit_parserLocalElements = visit_parserLocalElements;
  visitor->visit_parserLocalElement = visit_parserLocalElement;
  visitor->visit_parserStates = visit_parserStates;
  visitor->visit_parserState = visit_parserState;
  visitor->visit_parserStatements = visit_parserStatements;
  visitor->visit_parserStatement = visit_parserStatement;
  visitor->visit_parserBlockStatement = visit_parserBlockStatement;
  visitor->visit_transitionStatement = visit_transitionStatement;
  visitor->visit_stateExpression = visit_stateExpression;
  visitor->visit_selectExpression = visit_selectExpression;
  visitor->visit_selectCaseList = visit_selectCaseList;
  visitor->visit_selectCase = visit_selectCase;
  visitor->visit_keysetExpression = visit_keysetExpression;
  visitor->visit_tupleKeysetExpression = visit_tupleKeysetExpression;
  visitor->visit_simpleKeysetExpression = visit_simpleKeysetExpression;
  visitor->visit_simpleExpressionList = visit_simpleExpressionList;

  /** CONTROL **/

  visitor->visit_controlDeclaration = visit_controlDeclaration;
  visitor->visit_controlTypeDeclaration = visit_controlTypeDeclaration;
  visitor->visit_controlLocalDeclarations = visit_controlLocalDeclarations;
  visitor->visit_controlLocalDeclaration = visit_controlLocalDeclaration;

  /** EXTERN **/

  visitor->visit_externDeclaration = visit_externDeclaration;
  visitor->visit_externTypeDeclaration = visit_externTypeDeclaration;
  visitor->visit_methodPrototypes = visit_methodPrototypes;
  visitor->visit_functionPrototype = visit_functionPrototype;

  /** TYPES **/

  visitor->visit_typeRef = visit_typeRef;
  visitor->visit_tupleType = visit_tupleType;
  visitor->visit_headerStackType = visit_headerStackType;
  visitor->visit_baseTypeBoolean = visit_baseTypeBoolean;
  visitor->visit_baseTypeInteger = visit_baseTypeInteger;
  visitor->visit_baseTypeBit = visit_baseTypeBit;
  visitor->visit_baseTypeVarbit = visit_baseTypeVarbit;
  visitor->visit_baseTypeString = visit_baseTypeString;
  visitor->visit_baseTypeVoid = visit_baseTypeVoid;
  visitor->visit_baseTypeError = visit_baseTypeError;
  visitor->visit_integerTypeSize = visit_integerTypeSize;
  visitor->visit_realTypeArg = visit_realTypeArg;
  visitor->visit_typeArg = visit_typeArg;
  visitor->visit_typeArgumentList = visit_typeArgumentList;
  visitor->visit_typeDeclaration = visit_typeDeclaration;
  visitor->visit_derivedTypeDeclaration = visit_derivedTypeDeclaration;
  visitor->visit_headerTypeDeclaration = visit_headerTypeDeclaration;
  visitor->visit_headerUnionDeclaration = visit_headerUnionDeclaration;
  visitor->visit_structTypeDeclaration = visit_structTypeDeclaration;
  visitor->visit_structFieldList = visit_structFieldList;
  visitor->visit_structField = visit_structField;
  visitor->visit_enumDeclaration = visit_enumDeclaration;
  visitor->visit_errorDeclaration = visit_errorDeclaration;
  visitor->visit_matchKindDeclaration = visit_matchKindDeclaration;
  visitor->visit_identifierList = visit_identifierList;
  visitor->visit_specifiedIdentifierList = visit_specifiedIdentifierList;
  visitor->visit_specifiedIdentifier = visit_specifiedIdentifier;
  visitor->visit_typedefDeclaration = visit_typedefDeclaration;

  /** STATEMENTS **/

  visitor->visit_assignmentStatement = visit_assignmentStatement;
  visitor->visit_functionCall = visit_functionCall;
  visitor->visit_returnStatement = visit_returnStatement;
  visitor->visit_exitStatement = visit_exitStatement;
  visitor->visit_conditionalStatement = visit_conditionalStatement;
  visitor->visit_directApplication = visit_directApplication;
  visitor->visit_statement = visit_statement;
  visitor->visit_blockStatement = visit_blockStatement;
  visitor->visit_statementOrDeclList = visit_statementOrDeclList;
  visitor->visit_switchStatement = visit_switchStatement;
  visitor->visit_switchCases = visit_switchCases;
  visitor->visit_switchCase = visit_switchCase;
  visitor->visit_switchLabel = visit_switchLabel;
  visitor->visit_statementOrDeclaration = visit_statementOrDeclaration;

  /** TABLES **/

  visitor->visit_tableDeclaration = visit_tableDeclaration;
  visitor->visit_tablePropertyList = visit_tablePropertyList;
  visitor->visit_tableProperty = visit_tableProperty;
  visitor->visit_keyProperty = visit_keyProperty;
  visitor->visit_keyElementList = visit_keyElementList;
  visitor->visit_keyElement = visit_keyElement;
  visitor->visit_actionsProperty = visit_actionsProperty;
  visitor->visit_actionList = visit_actionList;
  visitor->visit_actionRef = visit_actionRef;
#if 0
  visitor->visit_entriesProperty = visit_entriesProperty;
  visitor->visit_entriesList = visit_entriesList;
  visitor->visit_entry = visit_entry;
  visitor->visit_simpleProperty = visit_simpleProperty;
#endif
  visitor->visit_actionDeclaration = visit_actionDeclaration;

  /** VARIABLES **/

  visitor->visit_variableDeclaration = visit_variableDeclaration;

  /** EXPRESSIONS **/

  visitor->visit_functionDeclaration = visit_functionDeclaration;
  visitor->visit_argumentList = visit_argumentList;
  visitor->visit_argument = visit_argument;
  visitor->visit_expressionList = visit_expressionList;
  visitor->visit_lvalueExpression = visit_lvalueExpression;
  visitor->visit_expression = visit_expression;
  visitor->visit_castExpression = visit_castExpression;
  visitor->visit_unaryExpression = visit_unaryExpression;
  visitor->visit_binaryExpression = visit_binaryExpression;
  visitor->visit_memberSelector = visit_memberSelector;
  visitor->visit_arraySubscript = visit_arraySubscript;
  visitor->visit_indexExpression = visit_indexExpression;
  visitor->visit_booleanLiteral = visit_booleanLiteral;
  visitor->visit_integerLiteral = visit_integerLiteral;
  visitor->visit_stringLiteral = visit_stringLiteral;
  visitor->visit_default = visit_default;
  visitor->visit_dontcare = visit_dontcare;
}
