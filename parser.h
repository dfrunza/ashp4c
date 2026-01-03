#pragma once
#include <arena.h>
#include <array.h>
#include <lexer.h>
#include <scope.h>

struct Parser {
  Arena* storage;
  Ast* p4program;
  char* source_file;
  Array* tokens;
  int token_at;
  int prev_token_at;
  Token* token;
  Token* prev_token;
  Scope* current_scope;
  Scope* root_scope;

/** PROGRAM **/

  Ast* parse_p4program();
  Ast* parse_declarationList();
  Ast* parse_declaration();
  Ast* parse_nonTypeName();
  Ast* parse_name();
  Ast* parse_parameterList();
  Ast* parse_parameter();
  enum ParamDirection parse_direction();
  Ast* parse_packageTypeDeclaration();
  Ast* parse_instantiation(Ast* type_ref);
  Ast* parse_constructorParameters();

/** PARSER **/

  Ast* parse_parserDeclaration(Ast* parser_proto);
  Ast* parse_parserLocalElements();
  Ast* parse_parserLocalElement();
  Ast* parse_parserTypeDeclaration();
  Ast* parse_parserStates();
  Ast* parse_parserState();
  Ast* parse_parserStatements();
  Ast* parse_parserStatement();
  Ast* parse_parserBlockStatement();
  Ast* parse_transitionStatement();
  Ast* parse_stateExpression();
  Ast* parse_selectExpression();
  Ast* parse_selectCaseList();
  Ast* parse_selectCase();
  Ast* parse_keysetExpression();
  Ast* parse_tupleKeysetExpression();
  Ast* parse_simpleExpressionList();
  Ast* parse_simpleKeysetExpression();

/** CONTROL **/

  Ast* parse_controlDeclaration(Ast* control_proto);
  Ast* parse_controlTypeDeclaration();
  Ast* parse_controlLocalDeclaration();
  Ast* parse_controlLocalDeclarations();

/** EXTERN **/

  Ast* parse_externDeclaration();
  Ast* parse_methodPrototypes();
  Ast* parse_functionPrototype(Ast* return_type);
  Ast* parse_methodPrototype();

/** TYPES **/

  Ast* parse_typeRef();
  Ast* parse_namedType();
  Ast* parse_typeName();
  Ast* parse_tupleType();
  Ast* parse_headerStackType(Ast* named_type);
  Ast* parse_baseType();
  Ast* parse_integerTypeSize();
  Ast* parse_typeOrVoid();
  Ast* parse_realTypeArg();
  Ast* parse_typeArg();
  Ast* parse_typeArgumentList();
  Ast* parse_typeDeclaration();
  Ast* parse_derivedTypeDeclaration();
  Ast* parse_headerTypeDeclaration();
  Ast* parse_headerUnionDeclaration();
  Ast* parse_structTypeDeclaration();
  Ast* parse_structFieldList();
  Ast* parse_structField();
  Ast* parse_enumDeclaration();
  Ast* parse_errorDeclaration();
  Ast* parse_matchKindDeclaration();
  Ast* parse_identifierList();
  Ast* parse_specifiedIdentifierList();
  Ast* parse_specifiedIdentifier();
  Ast* parse_typedefDeclaration();

/** STATEMENTS **/

  Ast* parse_assignmentOrMethodCallStatement();
  Ast* parse_returnStatement();
  Ast* parse_exitStatement();
  Ast* parse_conditionalStatement();
  Ast* parse_directApplication(Ast* type_name);
  Ast* parse_statement(Ast* type_name);
  Ast* parse_blockStatement();
  Ast* parse_statementOrDeclList();
  Ast* parse_switchStatement();
  Ast* parse_switchCases();
  Ast* parse_switchCase();
  Ast* parse_switchLabel();
  Ast* parse_statementOrDeclaration();

/** TABLES **/

  Ast* parse_tableDeclaration();
  Ast* parse_tablePropertyList();
  Ast* parse_tableProperty();
  Ast* parse_keyElementList();
  Ast* parse_keyElement();
  Ast* parse_actionList();
  Ast* parse_actionRef();
  Ast* parse_entriesList();
  Ast* parse_entry();
  Ast* parse_actionDeclaration();

/** VARIABLES **/

  Ast* parse_variableDeclaration(Ast* type_ref);

/** EXPRESSIONS **/

  Ast* parse_functionDeclaration(Ast* type_ref);
  Ast* parse_argumentList();
  Ast* parse_argument();
  Ast* parse_expressionList();
  Ast* parse_lvalue();
  Ast* parse_expression(int priority_threshold);
  Ast* parse_expressionPrimary();
  Ast* parse_indexExpression();
  Ast* parse_integer();
  Ast* parse_boolean();
  Ast* parse_string();

  void parse();
  Token* next_token();
  Token* peek_token();
  void define_keywords(Scope* scope);
};
