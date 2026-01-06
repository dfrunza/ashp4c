#pragma once

#include "frontend/ast.h"

struct AstVisitor {

  /** PROGRAM **/

  virtual void* visit_p4program(Ast* ast);
  virtual void* visit_declarationList(Ast* ast);
  virtual void* visit_declaration(Ast* ast);
  virtual void* visit_name(Ast* ast);
  virtual void* visit_parameterList(Ast* ast);
  virtual void* visit_parameter(Ast* ast);
  virtual void* visit_packageTypeDeclaration(Ast* ast);
  virtual void* visit_instantiation(Ast* ast);

  /** PARSER **/

  virtual void* visit_parserDeclaration(Ast* ast);
  virtual void* visit_parserTypeDeclaration(Ast* ast);
  virtual void* visit_parserLocalElements(Ast* ast);
  virtual void* visit_parserLocalElement(Ast* ast);
  virtual void* visit_parserStates(Ast* ast);
  virtual void* visit_parserState(Ast* ast);
  virtual void* visit_parserStatements(Ast* ast);
  virtual void* visit_parserStatement(Ast* ast);
  virtual void* visit_parserBlockStatement(Ast* ast);
  virtual void* visit_transitionStatement(Ast* ast);
  virtual void* visit_stateExpression(Ast* ast);
  virtual void* visit_selectExpression(Ast* ast);
  virtual void* visit_selectCaseList(Ast* ast);
  virtual void* visit_selectCase(Ast* ast);
  virtual void* visit_keysetExpression(Ast* ast);
  virtual void* visit_tupleKeysetExpression(Ast* ast);
  virtual void* visit_simpleKeysetExpression(Ast* ast);
  virtual void* visit_simpleExpressionList(Ast* ast);

  /** CONTROL **/

  virtual void* visit_controlDeclaration(Ast* ast);
  virtual void* visit_controlTypeDeclaration(Ast* ast);
  virtual void* visit_controlLocalDeclarations(Ast* ast);
  virtual void* visit_controlLocalDeclaration(Ast* ast);

  /** EXTERN **/

  virtual void* visit_externDeclaration(Ast* ast);
  virtual void* visit_externTypeDeclaration(Ast* ast);
  virtual void* visit_methodPrototypes(Ast* ast);
  virtual void* visit_functionPrototype(Ast* ast);

  /** TYPES **/

  virtual void* visit_typeRef(Ast* ast);
  virtual void* visit_tupleType(Ast* ast);
  virtual void* visit_headerStackType(Ast* ast);
  virtual void* visit_baseTypeBoolean(Ast* ast);
  virtual void* visit_baseTypeInteger(Ast* ast);
  virtual void* visit_baseTypeBit(Ast* ast);
  virtual void* visit_baseTypeVarbit(Ast* ast);
  virtual void* visit_baseTypeString(Ast* ast);
  virtual void* visit_baseTypeVoid(Ast* ast);
  virtual void* visit_baseTypeError(Ast* ast);
  virtual void* visit_integerTypeSize(Ast* ast);
  virtual void* visit_realTypeArg(Ast* ast);
  virtual void* visit_typeArg(Ast* ast);
  virtual void* visit_typeArgumentList(Ast* ast);
  virtual void* visit_typeDeclaration(Ast* ast);
  virtual void* visit_derivedTypeDeclaration(Ast* ast);
  virtual void* visit_headerTypeDeclaration(Ast* ast);
  virtual void* visit_headerUnionDeclaration(Ast* ast);
  virtual void* visit_structTypeDeclaration(Ast* ast);
  virtual void* visit_structFieldList(Ast* ast);
  virtual void* visit_structField(Ast* ast);
  virtual void* visit_enumDeclaration(Ast* ast);
  virtual void* visit_errorDeclaration(Ast* ast);
  virtual void* visit_matchKindDeclaration(Ast* ast);
  virtual void* visit_identifierList(Ast* ast);
  virtual void* visit_specifiedIdentifierList(Ast* ast);
  virtual void* visit_specifiedIdentifier(Ast* ast);
  virtual void* visit_typedefDeclaration(Ast* ast);

  /** STATEMENTS **/

  virtual void* visit_assignmentStatement(Ast* ast);
  virtual void* visit_functionCall(Ast* ast);
  virtual void* visit_returnStatement(Ast* ast);
  virtual void* visit_exitStatement(Ast* ast);
  virtual void* visit_conditionalStatement(Ast* ast);
  virtual void* visit_directApplication(Ast* ast);
  virtual void* visit_statement(Ast* ast);
  virtual void* visit_blockStatement(Ast* ast);
  virtual void* visit_statementOrDeclList(Ast* ast);
  virtual void* visit_switchStatement(Ast* ast);
  virtual void* visit_switchCases(Ast* ast);
  virtual void* visit_switchCase(Ast* ast);
  virtual void* visit_switchLabel(Ast* ast);
  virtual void* visit_statementOrDeclaration(Ast* ast);

  /** TABLES **/

  virtual void* visit_tableDeclaration(Ast* ast);
  virtual void* visit_tablePropertyList(Ast* ast);
  virtual void* visit_tableProperty(Ast* ast);
  virtual void* visit_keyProperty(Ast* ast);
  virtual void* visit_keyElementList(Ast* ast);
  virtual void* visit_keyElement(Ast* ast);
  virtual void* visit_actionsProperty(Ast* ast);
  virtual void* visit_actionList(Ast* ast);
  virtual void* visit_actionRef(Ast* ast);
#if 0
  virtual void* visit_entriesProperty(Ast* ast);
  virtual void* visit_entriesList(Ast* ast);
  virtual void* visit_entry(Ast* ast);
  virtual void* visit_simpleProperty(Ast* ast);
#endif
  virtual void* visit_actionDeclaration(Ast* ast);

  /** VARIABLES **/

  virtual void* visit_variableDeclaration(Ast* ast);

  /** EXPRESSIONS **/

  virtual void* visit_functionDeclaration(Ast* ast);
  virtual void* visit_argumentList(Ast* ast);
  virtual void* visit_argument(Ast* ast);
  virtual void* visit_expressionList(Ast* ast);
  virtual void* visit_lvalueExpression(Ast* ast);
  virtual void* visit_expression(Ast* ast);
  virtual void* visit_castExpression(Ast* ast);
  virtual void* visit_unaryExpression(Ast* ast);
  virtual void* visit_binaryExpression(Ast* ast);
  virtual void* visit_memberSelector(Ast* ast);
  virtual void* visit_arraySubscript(Ast* ast);
  virtual void* visit_indexExpression(Ast* ast);
  virtual void* visit_booleanLiteral(Ast* ast);
  virtual void* visit_integerLiteral(Ast* ast);
  virtual void* visit_stringLiteral(Ast* ast);
  virtual void* visit_default(Ast* ast);
  virtual void* visit_dontcare(Ast* ast);
};
