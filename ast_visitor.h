#pragma once
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

struct AstVisitor {

  /** PROGRAM **/

  void* visit_p4program(Ast* ast);
  void* visit_declarationList(Ast* ast);
  void* visit_declaration(Ast* ast);
  void* visit_name(Ast* ast);
  void* visit_parameterList(Ast* ast);
  void* visit_parameter(Ast* ast);
  void* visit_packageTypeDeclaration(Ast* ast);
  void* visit_instantiation(Ast* ast);

  /** PARSER **/

  void* visit_parserDeclaration(Ast* ast);
  void* visit_parserTypeDeclaration(Ast* ast);
  void* visit_parserLocalElements(Ast* ast);
  void* visit_parserLocalElement(Ast* ast);
  void* visit_parserStates(Ast* ast);
  void* visit_parserState(Ast* ast);
  void* visit_parserStatements(Ast* ast);
  void* visit_parserStatement(Ast* ast);
  void* visit_parserBlockStatement(Ast* ast);
  void* visit_transitionStatement(Ast* ast);
  void* visit_stateExpression(Ast* ast);
  void* visit_selectExpression(Ast* ast);
  void* visit_selectCaseList(Ast* ast);
  void* visit_selectCase(Ast* ast);
  void* visit_keysetExpression(Ast* ast);
  void* visit_tupleKeysetExpression(Ast* ast);
  void* visit_simpleKeysetExpression(Ast* ast);
  void* visit_simpleExpressionList(Ast* ast);

  /** CONTROL **/

  void* visit_controlDeclaration(Ast* ast);
  void* visit_controlTypeDeclaration(Ast* ast);
  void* visit_controlLocalDeclarations(Ast* ast);
  void* visit_controlLocalDeclaration(Ast* ast);

  /** EXTERN **/

  void* visit_externDeclaration(Ast* ast);
  void* visit_externTypeDeclaration(Ast* ast);
  void* visit_methodPrototypes(Ast* ast);
  void* visit_functionPrototype(Ast* ast);

  /** TYPES **/

  void* visit_typeRef(Ast* ast);
  void* visit_tupleType(Ast* ast);
  void* visit_headerStackType(Ast* ast);
  void* visit_baseTypeBoolean(Ast* ast);
  void* visit_baseTypeInteger(Ast* ast);
  void* visit_baseTypeBit(Ast* ast);
  void* visit_baseTypeVarbit(Ast* ast);
  void* visit_baseTypeString(Ast* ast);
  void* visit_baseTypeVoid(Ast* ast);
  void* visit_baseTypeError(Ast* ast);
  void* visit_integerTypeSize(Ast* ast);
  void* visit_realTypeArg(Ast* ast);
  void* visit_typeArg(Ast* ast);
  void* visit_typeArgumentList(Ast* ast);
  void* visit_typeDeclaration(Ast* ast);
  void* visit_derivedTypeDeclaration(Ast* ast);
  void* visit_headerTypeDeclaration(Ast* ast);
  void* visit_headerUnionDeclaration(Ast* ast);
  void* visit_structTypeDeclaration(Ast* ast);
  void* visit_structFieldList(Ast* ast);
  void* visit_structField(Ast* ast);
  void* visit_enumDeclaration(Ast* ast);
  void* visit_errorDeclaration(Ast* ast);
  void* visit_matchKindDeclaration(Ast* ast);
  void* visit_identifierList(Ast* ast);
  void* visit_specifiedIdentifierList(Ast* ast);
  void* visit_specifiedIdentifier(Ast* ast);
  void* visit_typedefDeclaration(Ast* ast);

  /** STATEMENTS **/

  void* visit_assignmentStatement(Ast* ast);
  void* visit_functionCall(Ast* ast);
  void* visit_returnStatement(Ast* ast);
  void* visit_exitStatement(Ast* ast);
  void* visit_conditionalStatement(Ast* ast);
  void* visit_directApplication(Ast* ast);
  void* visit_statement(Ast* ast);
  void* visit_blockStatement(Ast* ast);
  void* visit_statementOrDeclList(Ast* ast);
  void* visit_switchStatement(Ast* ast);
  void* visit_switchCases(Ast* ast);
  void* visit_switchCase(Ast* ast);
  void* visit_switchLabel(Ast* ast);
  void* visit_statementOrDeclaration(Ast* ast);

  /** TABLES **/

  void* visit_tableDeclaration(Ast* ast);
  void* visit_tablePropertyList(Ast* ast);
  void* visit_tableProperty(Ast* ast);
  void* visit_keyProperty(Ast* ast);
  void* visit_keyElementList(Ast* ast);
  void* visit_keyElement(Ast* ast);
  void* visit_actionsProperty(Ast* ast);
  void* visit_actionList(Ast* ast);
  void* visit_actionRef(Ast* ast);
  void* visit_entriesProperty(Ast* ast);
  void* visit_entriesList(Ast* ast);
  void* visit_entry(Ast* ast);
  void* visit_simpleProperty(Ast* ast);
  void* visit_actionDeclaration(Ast* ast);

  /** VARIABLES **/

  void* visit_variableDeclaration(Ast* ast);

  /** EXPRESSIONS **/

  void* visit_functionDeclaration(Ast* ast);
  void* visit_argumentList(Ast* ast);
  void* visit_argument(Ast* ast);
  void* visit_expressionList(Ast* ast);
  void* visit_lvalueExpression(Ast* ast);
  void* visit_expression(Ast* ast);
  void* visit_castExpression(Ast* ast);
  void* visit_unaryExpression(Ast* ast);
  void* visit_binaryExpression(Ast* ast);
  void* visit_memberSelector(Ast* ast);
  void* visit_arraySubscript(Ast* ast);
  void* visit_indexExpression(Ast* ast);
  void* visit_booleanLiteral(Ast* ast);
  void* visit_integerLiteral(Ast* ast);
  void* visit_stringLiteral(Ast* ast);
  void* visit_default(Ast* ast);
  void* visit_dontcare(Ast* ast);
} AstVisitor;
