#include <basic.h>
#include <ast.h>

char* AstEnum_to_string(enum AstEnum ast)
{
  switch (ast) {
    case AstEnum::none: return "none";

      /** PROGRAM **/

    case AstEnum::p4program: return "p4program";
    case AstEnum::declarationList: return "declarationList";
    case AstEnum::declaration: return "declaration";
    case AstEnum::name: return "name";
    case AstEnum::parameterList: return "parameterList";
    case AstEnum::parameter: return "parameter";
    case AstEnum::paramDirection: return "paramDirection";
    case AstEnum::packageTypeDeclaration: return "packageTypeDeclaration";
    case AstEnum::instantiation: return "instantiation";

      /** PARSER **/

    case AstEnum::parserDeclaration: return "parserDeclaration";
    case AstEnum::parserTypeDeclaration: return "parserTypeDeclaration";
    case AstEnum::parserLocalElements: return "parserLocalElements";
    case AstEnum::parserLocalElement: return "parserLocalElement";
    case AstEnum::parserStates: return "parserStates";
    case AstEnum::parserState: return "parserState";
    case AstEnum::parserStatements: return "parserStatements";
    case AstEnum::parserStatement: return "parserStatement";
    case AstEnum::parserBlockStatement: return "parserBlockStatement";
    case AstEnum::transitionStatement: return "transitionStatement";
    case AstEnum::stateExpression: return "stateExpression";
    case AstEnum::selectExpression: return "selectExpression";
    case AstEnum::selectCaseList: return "selectCaseList";
    case AstEnum::selectCase: return "selectCase";
    case AstEnum::keysetExpression: return "keysetExpression";
    case AstEnum::tupleKeysetExpression: return "tupleKeysetExpression";
    case AstEnum::simpleKeysetExpression: return "simpleKeysetExpression";
    case AstEnum::simpleExpressionList: return "simpleExpressionList";

      /** CONTROL **/

    case AstEnum::controlDeclaration: return "controlDeclaration";
    case AstEnum::controlTypeDeclaration: return "controlTypeDeclaration";
    case AstEnum::controlLocalDeclarations: return "controlLocalDeclarations";
    case AstEnum::controlLocalDeclaration: return "controlLocalDeclaration";

      /** TYPES **/

    case AstEnum::typeRef: return "typeRef";
    case AstEnum::tupleType: return "tupleType";
    case AstEnum::headerStackType: return "headerStackType";
    case AstEnum::baseTypeBoolean: return "baseTypeBoolean";
    case AstEnum::baseTypeInteger: return "baseTypeInteger";
    case AstEnum::baseTypeBit: return "baseTypeBit";
    case AstEnum::baseTypeVarbit: return "baseTypeVarbit";
    case AstEnum::baseTypeString: return "baseTypeString";
    case AstEnum::baseTypeVoid: return "baseTypeVoid";
    case AstEnum::baseTypeError: return "baseTypeError";
    case AstEnum::integerTypeSize: return "integerTypeSize";
    case AstEnum::realTypeArg: return "realTypeArg";
    case AstEnum::typeArg: return "typeArg";
    case AstEnum::typeArgumentList: return "typeArgumentList";
    case AstEnum::typeDeclaration: return "typeDeclaration";
    case AstEnum::derivedTypeDeclaration: return "derivedTypeDeclaration";
    case AstEnum::headerTypeDeclaration: return "headerTypeDeclaration";
    case AstEnum::headerUnionDeclaration: return "headerUnionDeclaration";
    case AstEnum::structTypeDeclaration: return "structTypeDeclaration";
    case AstEnum::structFieldList: return "structFieldList";
    case AstEnum::structField: return "structField";
    case AstEnum::enumDeclaration: return "enumDeclaration";
    case AstEnum::errorDeclaration: return "errorDeclaration";
    case AstEnum::matchKindDeclaration: return "matchKindDeclaration";
    case AstEnum::identifierList: return "identifierList";
    case AstEnum::specifiedIdentifierList: return "specifiedIdentifierList";
    case AstEnum::specifiedIdentifier: return "specifiedIdentifier";
    case AstEnum::typedefDeclaration: return "typedefDeclaration";

      /** STATEMENTS **/

    case AstEnum::assignmentStatement: return "assignmentStatement";
    case AstEnum::emptyStatement: return "emptyStatement";
    case AstEnum::returnStatement: return "returnStatement";
    case AstEnum::exitStatement: return "exitStatement";
    case AstEnum::conditionalStatement: return "conditionalStatement";
    case AstEnum::directApplication: return "directApplication";
    case AstEnum::statement: return "statement";
    case AstEnum::blockStatement: return "blockStatement";
    case AstEnum::statementOrDeclaration: return "statementOrDeclaration";
    case AstEnum::statementOrDeclList: return "statementOrDeclList";
    case AstEnum::switchStatement: return "switchStatement";
    case AstEnum::switchCases: return "switchCases";
    case AstEnum::switchCase: return "switchCase";
    case AstEnum::switchLabel: return "switchLabel";

      /** TABLES **/

    case AstEnum::tableDeclaration: return "tableDeclaration";
    case AstEnum::tablePropertyList: return "tablePropertyList";
    case AstEnum::tableProperty: return "tableProperty";
    case AstEnum::keyProperty: return "keyProperty";
    case AstEnum::keyElementList: return "keyElementList";
    case AstEnum::keyElement: return "keyElement";
    case AstEnum::actionsProperty: return "actionsProperty";
    case AstEnum::actionList: return "actionList";
    case AstEnum::actionRef: return "actionRef";
#if 0
      case AstEnum::entriesProperty: return "entriesProperty";
    case AstEnum::entriesList: return "entriesList";
    case AstEnum::entry: return "entry";
    case AstEnum::simpleProperty: return "simpleProperty";
#endif
    case AstEnum::actionDeclaration: return "actionDeclaration";

      /** VARIABLES **/

    case AstEnum::variableDeclaration: return "variableDeclaration";

      /** EXPRESSIONS **/

    case AstEnum::functionDeclaration: return "functionDeclaration";
    case AstEnum::argumentList: return "argumentList";
    case AstEnum::argument: return "argument";
    case AstEnum::expressionList: return "expressionList";
    case AstEnum::expression: return "expression";
    case AstEnum::lvalueExpression: return "lvalueExpression";
    case AstEnum::binaryExpression: return "binaryExpression";
    case AstEnum::unaryExpression: return "unaryExpression";
    case AstEnum::functionCall: return "functionCall";
    case AstEnum::memberSelector: return "memberSelector";
    case AstEnum::castExpression: return "castExpression";
    case AstEnum::arraySubscript: return "arraySubscript";
    case AstEnum::indexExpression: return "indexExpression";
    case AstEnum::integerLiteral: return "integerLiteral";
    case AstEnum::stringLiteral: return "stringLiteral";
    case AstEnum::dontcare: return "dontcare";
    case AstEnum::default_: return "default";

    default: return "?";
  }
  assert(0);
  return 0;
}

Ast* Ast::owner_of(Tree<Ast>* tree)
{
  return ::owner_of(tree, &Ast::tree);
}

Ast* Ast::create(Arena* storage, enum AstEnum kind, int line_no, int column_no)
{
  Ast* ast = storage->allocate<Ast>(1);
  ast->kind = kind;
  ast->line_no = line_no;
  ast->column_no = column_no;
  return ast;
}

Ast* Ast::clone(Arena* storage)
{
  Ast* clone, *sibling_clone, *child_clone;

  if (this == 0) return (Ast*)0;
  clone = storage->allocate<Ast>(1);
  clone->kind = kind;
  clone->line_no = line_no;
  clone->column_no = column_no;
  if (tree.first_child) {
    child_clone = Ast::owner_of(tree.first_child)->clone(storage);
    clone->tree.first_child = &child_clone->tree;
  }
  if (tree.right_sibling) {
    sibling_clone = Ast::owner_of(tree.right_sibling)->clone(storage);
    clone->tree.right_sibling = &sibling_clone->tree;
  }

  /** PROGRAM **/
  if (kind == AstEnum::p4program) {
    clone->p4program.decl_list = p4program.decl_list->clone(storage);
  } else if (kind == AstEnum::declarationList) {
    ;
  } else if (kind == AstEnum::declaration) {
    clone->declaration.decl = declaration.decl->clone(storage);
  } else if (kind == AstEnum::name) {
    clone->name.strname = name.strname;
  } else if (kind == AstEnum::parameterList) {
    ;
  } else if (kind == AstEnum::parameter) {
    clone->parameter.direction = parameter.direction;
    clone->parameter.name = parameter.name->clone(storage);
    clone->parameter.type = parameter.type->clone(storage);
    clone->parameter.init_expr = parameter.init_expr->clone(storage);
  } else if (kind == AstEnum::packageTypeDeclaration) {
    clone->packageTypeDeclaration.name = packageTypeDeclaration.name->clone(storage);
    clone->packageTypeDeclaration.params = packageTypeDeclaration.params->clone(storage);
  } else if (kind == AstEnum::instantiation) {
    clone->instantiation.name = instantiation.name->clone(storage);
    clone->instantiation.type = instantiation.type->clone(storage);
    clone->instantiation.args = instantiation.args->clone(storage);
  }
    /** PARSER **/
  else if (kind == AstEnum::parserDeclaration) {
    clone->parserDeclaration.proto = parserDeclaration.proto->clone(storage);
    clone->parserDeclaration.ctor_params = parserDeclaration.ctor_params->clone(storage);
    clone->parserDeclaration.local_elements = parserDeclaration.local_elements->clone(storage);
    clone->parserDeclaration.states = parserDeclaration.states->clone(storage);
  } else if (kind == AstEnum::parserTypeDeclaration) {
    clone->parserTypeDeclaration.name = parserTypeDeclaration.name->clone(storage);
    clone->parserTypeDeclaration.params = parserTypeDeclaration.params->clone(storage);
    clone->parserTypeDeclaration.method_protos = parserTypeDeclaration.method_protos->clone(storage);
  } else if (kind == AstEnum::parserLocalElements) {
    ;
  } else if (kind == AstEnum::parserLocalElement) {
    clone->parserLocalElement.element = parserLocalElement.element->clone(storage);
  } else if (kind == AstEnum::parserStates) {
    ;
  } else if (kind == AstEnum::parserState) {
    clone->parserState.name = parserState.name->clone(storage);
    clone->parserState.stmt_list = parserState.stmt_list->clone(storage);
    clone->parserState.transition_stmt = parserState.transition_stmt->clone(storage);
  } else if (kind == AstEnum::parserStatements) {
    ;
  } else if (kind == AstEnum::parserStatement) {
    clone->parserStatement.stmt = parserStatement.stmt->clone(storage);
  } else if (kind == AstEnum::parserBlockStatement) {
    clone->parserBlockStatement.stmt_list = parserBlockStatement.stmt_list->clone(storage);
  } else if (kind == AstEnum::transitionStatement) {
    clone->transitionStatement.stmt = transitionStatement.stmt->clone(storage);
  } else if (kind == AstEnum::stateExpression) {
    clone->stateExpression.expr = stateExpression.expr->clone(storage);
  } else if (kind == AstEnum::selectExpression) {
    clone->selectExpression.expr_list = selectExpression.expr_list->clone(storage);
    clone->selectExpression.case_list = selectExpression.case_list->clone(storage);
  } else if (kind == AstEnum::selectCaseList) {
    ;
  } else if (kind == AstEnum::selectCase) {
    clone->selectCase.keyset_expr = selectCase.keyset_expr->clone(storage);
    clone->selectCase.name = selectCase.name->clone(storage);
  } else if (kind == AstEnum::keysetExpression) {
    clone->keysetExpression.expr = keysetExpression.expr->clone(storage);
  } else if (kind == AstEnum::tupleKeysetExpression) {
    clone->tupleKeysetExpression.expr_list = tupleKeysetExpression.expr_list->clone(storage);
  } else if (kind == AstEnum::simpleKeysetExpression) {
    clone->simpleKeysetExpression.expr = simpleKeysetExpression.expr->clone(storage);
  } else if (kind == AstEnum::simpleExpressionList) {
    ;
  } else if (kind == AstEnum::typeRef) {
    clone->typeRef.type = typeRef.type->clone(storage);
  } else if (kind == AstEnum::tupleType) {
    clone->tupleType.type_args = tupleType.type_args->clone(storage);
  }
    /** CONTROL **/
  else if (kind == AstEnum::controlDeclaration) {
    clone->controlDeclaration.proto = controlDeclaration.proto->clone(storage);
    clone->controlDeclaration.ctor_params = controlDeclaration.ctor_params->clone(storage);
    clone->controlDeclaration.local_decls = controlDeclaration.local_decls->clone(storage);
    clone->controlDeclaration.apply_stmt = controlDeclaration.apply_stmt->clone(storage);
  } else if (kind == AstEnum::controlTypeDeclaration) {
    clone->controlTypeDeclaration.name = controlTypeDeclaration.name->clone(storage);
    clone->controlTypeDeclaration.params = controlTypeDeclaration.params->clone(storage);
    clone->controlTypeDeclaration.method_protos = controlTypeDeclaration.params->clone(storage);
  } else if (kind == AstEnum::controlLocalDeclarations) {
    ;
  } else if (kind == AstEnum::controlLocalDeclaration) {
    clone->controlLocalDeclaration.decl = controlLocalDeclaration.decl->clone(storage);
  }
    /** EXTERN **/
  else if (kind == AstEnum::externDeclaration) {
    clone->externDeclaration.decl = externDeclaration.decl->clone(storage);
  } else if (kind == AstEnum::externTypeDeclaration) {
    clone->externTypeDeclaration.name = externTypeDeclaration.name->clone(storage);
    clone->externTypeDeclaration.method_protos = externTypeDeclaration.method_protos->clone(storage);
  } else if (kind == AstEnum::methodPrototypes) {
    ;
  } else if (kind == AstEnum::functionPrototype) {
    clone->functionPrototype.return_type = functionPrototype.return_type->clone(storage);
    clone->functionPrototype.name = functionPrototype.name->clone(storage);
    clone->functionPrototype.params = functionPrototype.params->clone(storage);
  }
    /** TYPES **/
  else if (kind == AstEnum::typeRef) {
    clone->typeRef.type = typeRef.type->clone(storage);
  } else if (kind == AstEnum::tupleType) {
    clone->tupleType.type_args = tupleType.type_args->clone(storage);
  } else if (kind == AstEnum::headerStackType) {
    clone->headerStackType.type = headerStackType.type->clone(storage);
    clone->headerStackType.stack_expr = headerStackType.stack_expr->clone(storage);
  } else if (kind == AstEnum::baseTypeBoolean) {
    clone->baseTypeBoolean.name = baseTypeBoolean.name->clone(storage);
  } else if (kind == AstEnum::baseTypeInteger) {
    clone->baseTypeInteger.name = baseTypeInteger.name->clone(storage);
    clone->baseTypeInteger.size = baseTypeInteger.size->clone(storage);
  } else if (kind == AstEnum::baseTypeBit) {
    clone->baseTypeBit.name = baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = baseTypeBit.size->clone(storage);
  } else if (kind == AstEnum::baseTypeBit) {
    clone->baseTypeBit.name = baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = baseTypeBit.size->clone(storage);
  } else if (kind == AstEnum::baseTypeString) {
    clone->baseTypeString.name = baseTypeString.name->clone(storage);
  } else if (kind == AstEnum::baseTypeVoid) {
    clone->baseTypeVoid.name = baseTypeVoid.name->clone(storage);
  } else if (kind == AstEnum::baseTypeError) {
    clone->baseTypeError.name = baseTypeError.name->clone(storage);
  } else if (kind == AstEnum::integerTypeSize) {
    clone->integerTypeSize.size = integerTypeSize.size->clone(storage);
  } else if (kind == AstEnum::realTypeArg) {
    clone->realTypeArg.arg = realTypeArg.arg->clone(storage);
  } else if (kind == AstEnum::typeArg) {
    clone->typeArg.arg = typeArg.arg->clone(storage);
  } else if (kind == AstEnum::typeArgumentList) {
    ;
  } else if (kind == AstEnum::typeDeclaration) {
    clone->typeDeclaration.decl = typeDeclaration.decl->clone(storage);
  } else if (kind == AstEnum::derivedTypeDeclaration) {
    clone->derivedTypeDeclaration.decl = derivedTypeDeclaration.decl->clone(storage);
  } else if (kind == AstEnum::headerTypeDeclaration) {
    clone->headerTypeDeclaration.name = headerTypeDeclaration.name->clone(storage);
    clone->headerTypeDeclaration.fields = headerTypeDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::headerUnionDeclaration) {
    clone->headerUnionDeclaration.name = headerUnionDeclaration.name->clone(storage);
    clone->headerUnionDeclaration.fields = headerUnionDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::structTypeDeclaration) {
    clone->structTypeDeclaration.name = structTypeDeclaration.name->clone(storage);
    clone->structTypeDeclaration.fields = structTypeDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::structFieldList) {
    ;
  } else if (kind == AstEnum::structField) {
    clone->structField.type = structField.type->clone(storage);
    clone->structField.name = structField.name->clone(storage);
  } else if (kind == AstEnum::enumDeclaration) {
    clone->enumDeclaration.type_size = enumDeclaration.type_size->clone(storage);
    clone->enumDeclaration.name = enumDeclaration.name->clone(storage);
    clone->enumDeclaration.fields = enumDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::errorDeclaration) {
    clone->errorDeclaration.fields = errorDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::matchKindDeclaration) {
    clone->matchKindDeclaration.fields = matchKindDeclaration.fields->clone(storage);
  } else if (kind == AstEnum::matchKindDeclaration) {
    ;
  } else if (kind == AstEnum::specifiedIdentifierList) {
    ;
  } else if (kind == AstEnum::specifiedIdentifier) {
    clone->specifiedIdentifier.name = specifiedIdentifier.name->clone(storage);
    clone->specifiedIdentifier.init_expr = specifiedIdentifier.init_expr->clone(storage);
  } else if (kind == AstEnum::typedefDeclaration) {
    clone->typedefDeclaration.type_ref = typedefDeclaration.type_ref->clone(storage);
    clone->typedefDeclaration.name = typedefDeclaration.name->clone(storage);
  }
    /** STATEMENTS **/
  else if (kind == AstEnum::assignmentStatement) {
    clone->assignmentStatement.lhs_expr = assignmentStatement.lhs_expr->clone(storage);
    clone->assignmentStatement.rhs_expr = assignmentStatement.rhs_expr->clone(storage);
  } else if (kind == AstEnum::emptyStatement) {
    ;
  } else if (kind == AstEnum::returnStatement) {
    clone->returnStatement.expr = returnStatement.expr->clone(storage);
  } else if (kind == AstEnum::returnStatement) {
    ;
  } else if (kind == AstEnum::conditionalStatement) {
    clone->conditionalStatement.cond_expr = conditionalStatement.cond_expr->clone(storage);
    clone->conditionalStatement.stmt = conditionalStatement.stmt->clone(storage);
    clone->conditionalStatement.else_stmt = conditionalStatement.else_stmt->clone(storage);
  } else if (kind == AstEnum::directApplication) {
    clone->directApplication.name = directApplication.name->clone(storage);
    clone->directApplication.args = directApplication.args->clone(storage);
  } else if (kind == AstEnum::statement) {
    clone->statement.stmt = statement.stmt->clone(storage);
  } else if (kind == AstEnum::blockStatement) {
    clone->blockStatement.stmt_list = blockStatement.stmt_list->clone(storage);
  } else if (kind == AstEnum::statementOrDeclaration) {
    clone->statementOrDeclaration.stmt = statementOrDeclaration.stmt->clone(storage);
  } else if (kind == AstEnum::statementOrDeclList) {
    ;
  } else if (kind == AstEnum::switchStatement) {
    clone->switchStatement.expr = switchStatement.expr->clone(storage);
    clone->switchStatement.switch_cases = switchStatement.switch_cases->clone(storage);
  } else if (kind == AstEnum::switchCases) {
    ;
  } else if (kind == AstEnum::switchCase) {
    clone->switchCase.label = switchCase.label->clone(storage);
    clone->switchCase.stmt = switchCase.stmt->clone(storage);
  } else if (kind == AstEnum::switchLabel) {
    clone->switchLabel.label = switchLabel.label->clone(storage);
  }
    /** TABLES **/
  else if (kind == AstEnum::tableDeclaration) {
    clone->tableDeclaration.name = tableDeclaration.name->clone(storage);
    clone->tableDeclaration.prop_list = tableDeclaration.prop_list->clone(storage);
  } else if (kind == AstEnum::tablePropertyList) {
    ;
  } else if (kind == AstEnum::tableProperty) {
    clone->tableProperty.prop = tableProperty.prop->clone(storage);
  } else if (kind == AstEnum::keyProperty) {
    clone->keyProperty.keyelem_list = keyProperty.keyelem_list->clone(storage);
  } else if (kind == AstEnum::keyElementList) {
    ;
  } else if (kind == AstEnum::keyElement) {
    clone->keyElement.expr = keyElement.expr->clone(storage);
    clone->keyElement.match = keyElement.match->clone(storage);
  } else if (kind == AstEnum::actionsProperty) {
    clone->actionsProperty.action_list = actionsProperty.action_list->clone(storage);
  } else if (kind == AstEnum::actionList) {
    ;
  } else if (kind == AstEnum::actionRef) {
    clone->actionRef.name = actionRef.name->clone(storage);
    clone->actionRef.args = actionRef.args->clone(storage);
  }
#if 0
    else if (kind == AstEnum::entriesProperty) {
    clone->entriesProperty.entries_list = entriesProperty.entries_list->clone(storage);
  } else if (kind == AstEnum::entriesList) {
    ;
  } else if (kind == AstEnum::entry) {
    clone->entry.keyset = entry.keyset->clone(storage);
    clone->entry.action = entry.action->clone(storage);
  } else if (kind == AstEnum::simpleProperty) {
    clone->simpleProperty.name = simpleProperty.name->clone(storage);
    clone->simpleProperty.init_expr = simpleProperty.init_expr->clone(storage);
    clone->simpleProperty.is_const = simpleProperty.is_const;
  }
#endif
  else if (kind == AstEnum::actionDeclaration) {
    clone->actionDeclaration.name = actionDeclaration.name->clone(storage);
    clone->actionDeclaration.params = actionDeclaration.params->clone(storage);
    clone->actionDeclaration.stmt = actionDeclaration.stmt->clone(storage);
  }
    /** VARIABLES **/
  else if (kind == AstEnum::variableDeclaration) {
    clone->variableDeclaration.type = variableDeclaration.type->clone(storage);
    clone->variableDeclaration.name = variableDeclaration.name->clone(storage);
    clone->variableDeclaration.init_expr = variableDeclaration.init_expr->clone(storage);
    clone->variableDeclaration.is_const = variableDeclaration.is_const;
  }
    /** EXPRESSIONS **/
  else if (kind == AstEnum::functionDeclaration) {
    clone->functionDeclaration.proto = functionDeclaration.proto->clone(storage);
    clone->functionDeclaration.stmt = functionDeclaration.stmt->clone(storage);
  } else if (kind == AstEnum::argumentList) {
    ;
  } else if (kind == AstEnum::argument) {
    clone->argument.arg = argument.arg->clone(storage);
  } else if (kind == AstEnum::expressionList) {
    ;
  } else if (kind == AstEnum::expression) {
    clone->expression.expr = expression.expr->clone(storage);
  } else if (kind == AstEnum::lvalueExpression) {
    clone->lvalueExpression.expr = lvalueExpression.expr->clone(storage);
  } else if (kind == AstEnum::binaryExpression) {
    clone->binaryExpression.op = binaryExpression.op;
    clone->binaryExpression.strname = binaryExpression.strname;
    clone->binaryExpression.left_operand = binaryExpression.left_operand->clone(storage);
    clone->binaryExpression.right_operand = binaryExpression.right_operand->clone(storage);
  } else if (kind == AstEnum::unaryExpression) {
    clone->unaryExpression.op = unaryExpression.op;
    clone->unaryExpression.strname = unaryExpression.strname;
    clone->unaryExpression.operand = unaryExpression.operand->clone(storage);
  } else if (kind == AstEnum::functionCall) {
    clone->functionCall.lhs_expr = functionCall.lhs_expr->clone(storage);
    clone->functionCall.args = functionCall.args->clone(storage);
  } else if (kind == AstEnum::memberSelector) {
    clone->memberSelector.lhs_expr = memberSelector.lhs_expr->clone(storage);
    clone->memberSelector.name = memberSelector.name->clone(storage);
  } else if (kind == AstEnum::castExpression) {
    clone->castExpression.type = castExpression.type->clone(storage);
    clone->castExpression.expr = castExpression.expr->clone(storage);
  } else if (kind == AstEnum::arraySubscript) {
    clone->arraySubscript.lhs_expr = arraySubscript.lhs_expr->clone(storage);
    clone->arraySubscript.index_expr = arraySubscript.index_expr->clone(storage);
  } else if (kind == AstEnum::indexExpression) {
    clone->indexExpression.start_index = indexExpression.start_index->clone(storage);
    clone->indexExpression.end_index = indexExpression.end_index->clone(storage);
  } else if (kind == AstEnum::integerLiteral) {
    clone->integerLiteral.is_signed = integerLiteral.is_signed;
    clone->integerLiteral.value = integerLiteral.value;
    clone->integerLiteral.width = integerLiteral.width;
  } else if (kind == AstEnum::booleanLiteral) {
    clone->booleanLiteral.value = booleanLiteral.value;
  } else if (kind == AstEnum::stringLiteral) {
    clone->stringLiteral.value = stringLiteral.value;
  } else if (kind == AstEnum::default_ || kind == AstEnum::dontcare) {
    ;
  }
  else assert(0);
  return clone;
}
