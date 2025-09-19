#pragma once
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

typedef struct AstVisitor {

  /** PROGRAM **/

  void* (*visit_p4program)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_declarationList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_declaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_name)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parameterList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parameter)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_packageTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_instantiation)(struct AstVisitor* visitor, Ast* ast);

  /** PARSER **/

  void* (*visit_parserDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserLocalElements)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserLocalElement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserStates)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserState)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserStatements)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_parserBlockStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_transitionStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_stateExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_selectExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_selectCaseList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_selectCase)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_keysetExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_tupleKeysetExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_simpleKeysetExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_simpleExpressionList)(struct AstVisitor* visitor, Ast* ast);

  /** CONTROL **/

  void* (*visit_controlDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_controlTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_controlLocalDeclarations)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_controlLocalDeclaration)(struct AstVisitor* visitor, Ast* ast);

  /** EXTERN **/

  void* (*visit_externDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_externTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_methodPrototypes)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_functionPrototype)(struct AstVisitor* visitor, Ast* ast);

  /** TYPES **/

  void* (*visit_typeRef)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_tupleType)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_headerStackType)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeBoolean)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeInteger)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeBit)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeVarbit)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeString)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeVoid)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_baseTypeError)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_integerTypeSize)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_realTypeArg)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_typeArg)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_typeArgumentList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_typeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_derivedTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_headerTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_headerUnionDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_structTypeDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_structFieldList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_structField)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_enumDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_errorDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_matchKindDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_identifierList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_specifiedIdentifierList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_specifiedIdentifier)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_typedefDeclaration)(struct AstVisitor* visitor, Ast* ast);

  /** STATEMENTS **/

  void* (*visit_assignmentStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_functionCall)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_returnStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_exitStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_conditionalStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_directApplication)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_statement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_blockStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_statementOrDeclList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_switchStatement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_switchCases)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_switchCase)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_switchLabel)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_statementOrDeclaration)(struct AstVisitor* visitor, Ast* ast);

  /** TABLES **/

  void* (*visit_tableDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_tablePropertyList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_tableProperty)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_keyProperty)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_keyElementList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_keyElement)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_actionsProperty)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_actionList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_actionRef)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_entriesProperty)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_entriesList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_entry)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_simpleProperty)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_actionDeclaration)(struct AstVisitor* visitor, Ast* ast);

  /** VARIABLES **/

  void* (*visit_variableDeclaration)(struct AstVisitor* visitor, Ast* ast);

  /** EXPRESSIONS **/

  void* (*visit_functionDeclaration)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_argumentList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_argument)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_expressionList)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_lvalueExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_expression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_castExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_unaryExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_binaryExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_memberSelector)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_arraySubscript)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_indexExpression)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_booleanLiteral)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_integerLiteral)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_stringLiteral)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_default)(struct AstVisitor* visitor, Ast* ast);
  void* (*visit_dontcare)(struct AstVisitor* visitor, Ast* ast);
} AstVisitor;
