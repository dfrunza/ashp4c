#pragma once

#include "memory/arena.h"
#include "adt/array.h"
#include "adt/map.h"
#include "frontend/ast.h"
#include "frontend/scope.h"
#include "midend/potential_type.h"
#include "midend/type_checker.h"

struct PotentialTypePass {
  Arena* storage;
  char* source_file;
  Ast* p4program;
  Scope* root_scope;
  Map* scope_map;
  Map* decl_map;
  Array* type_array;
  Map* type_env;
  TypeChecker* type_checker;
  Map* potype_map;

/** PROGRAM **/

  void visit_p4program(Ast* p4program);
  void visit_declarationList(Ast* decl_list);
  void visit_declaration(Ast* decl);
  void visit_name(Ast* name, PotentialType* potential_args);
  void visit_parameterList(Ast* params);
  void visit_parameter(Ast* param);
  void visit_packageTypeDeclaration(Ast* package_decl);
  void visit_instantiation(Ast* inst);

/** PARSER **/

  void visit_parserDeclaration(Ast* parser_decl);
  void visit_parserTypeDeclaration(Ast* type_decl);
  void visit_parserLocalElements(Ast* local_elements);
  void visit_parserLocalElement(Ast* local_element);
  void visit_parserStates(Ast* states);
  void visit_parserState(Ast* state);
  void visit_parserStatements(Ast* stmts);
  void visit_parserStatement(Ast* stmt);
  void visit_parserBlockStatement(Ast* block_stmt);
  void visit_transitionStatement(Ast* transition_stmt);
  void visit_stateExpression(Ast* state_expr);
  void visit_selectExpression(Ast* select_expr);
  void visit_selectCaseList(Ast* case_list);
  void visit_selectCase(Ast* select_case);
  void visit_keysetExpression(Ast* keyset_expr);
  void visit_tupleKeysetExpression(Ast* tuple_expr);
  void visit_simpleKeysetExpression(Ast* simple_expr);
  void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

  void visit_controlDeclaration(Ast* control_decl);
  void visit_controlTypeDeclaration(Ast* type_decl);
  void visit_controlLocalDeclarations(Ast* local_decls);
  void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

  void visit_externDeclaration(Ast* extern_decl);
  void visit_externTypeDeclaration(Ast* type_decl);
  void visit_methodPrototypes(Ast* protos);
  void visit_functionPrototype(Ast* func_proto);

/** TYPES **/

  void visit_typeRef(Ast* type_ref);
  void visit_tupleType(Ast* type);
  void visit_headerStackType(Ast* type_decl);
  void visit_baseTypeBoolean(Ast* bool_type);
  void visit_baseTypeInteger(Ast* int_type);
  void visit_baseTypeBit(Ast* bit_type);
  void visit_baseTypeVarbit(Ast* varbit_type);
  void visit_baseTypeString(Ast* str_type);
  void visit_baseTypeVoid(Ast* void_type);
  void visit_baseTypeError(Ast* error_type);
  void visit_integerTypeSize(Ast* type_size);
  void visit_realTypeArg(Ast* type_arg);
  void visit_typeArg(Ast* type_arg);
  void visit_typeArgumentList(Ast* args);
  void visit_typeDeclaration(Ast* type_decl);
  void visit_derivedTypeDeclaration(Ast* type_decl);
  void visit_headerTypeDeclaration(Ast* header_decl);
  void visit_headerUnionDeclaration(Ast* union_decl);
  void visit_structTypeDeclaration(Ast* struct_decl);
  void visit_structFieldList(Ast* fields);
  void visit_structField(Ast* field);
  void visit_enumDeclaration(Ast* enum_decl);
  void visit_errorDeclaration(Ast* error_decl);
  void visit_matchKindDeclaration(Ast* match_decl);
  void visit_identifierList(Ast* ident_list);
  void visit_specifiedIdentifierList(Ast* ident_list);
  void visit_specifiedIdentifier(Ast* ident);
  void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

  void visit_assignmentStatement(Ast* assign_stmt);
  void visit_functionCall(Ast* func_call);
  void visit_returnStatement(Ast* return_stmt);
  void visit_exitStatement(Ast* exit_stmt);
  void visit_conditionalStatement(Ast* cond_stmt);
  void visit_directApplication(Ast* applic_stmt);
  void visit_statement(Ast* stmt);
  void visit_blockStatement(Ast* block_stmt);
  void visit_statementOrDeclList(Ast* stmt_list);
  void visit_switchStatement(Ast* switch_stmt);
  void visit_switchCases(Ast* switch_cases);
  void visit_switchCase(Ast* switch_case);
  void visit_switchLabel(Ast* label);
  void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

  void visit_tableDeclaration(Ast* table_decl);
  void visit_tablePropertyList(Ast* prop_list);
  void visit_tableProperty(Ast* table_prop);
  void visit_keyProperty(Ast* key_prop);
  void visit_keyElementList(Ast* element_list);
  void visit_keyElement(Ast* element);
  void visit_actionsProperty(Ast* actions_prop);
  void visit_actionList(Ast* action_list);
  void visit_actionRef(Ast* action_ref);
  void visit_entriesProperty(Ast* entries_prop);
  void visit_entriesList(Ast* entries_list);
  void visit_entry(Ast* entry);
  void visit_simpleProperty(Ast* simple_prop);
  void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

  void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

  void visit_functionDeclaration(Ast* func_decl);
  void visit_argumentList(Ast* args);
  void visit_argument(Ast* arg);
  void visit_expressionList(Ast* expr_list);
  void visit_lvalueExpression(Ast* lvalue_expr, PotentialType* potential_args);
  void visit_expression(Ast* expr, PotentialType* potential_args);
  void visit_castExpression(Ast* cast_expr);
  void visit_unaryExpression(Ast* unary_expr);
  void visit_binaryExpression(Ast* binary_expr);
  void visit_memberSelector(Ast* selector, PotentialType* potential_args);
  void visit_arraySubscript(Ast* subscript);
  void visit_indexExpression(Ast* index_expr);
  void visit_booleanLiteral(Ast* bool_literal);
  void visit_integerLiteral(Ast* int_literal);
  void visit_stringLiteral(Ast* str_literal);
  void visit_default(Ast* default_);
  void visit_dontcare(Ast* dontcare);

  void do_pass();
};
