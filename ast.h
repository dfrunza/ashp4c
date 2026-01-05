#pragma once
#include "arena.h"
#include "tree.h"
#include "ast.h"

enum class AstEnum {
  none = 0,

  /** PROGRAM **/

  p4program,
  declarationList,
  declaration,
  name,
  parameterList,
  parameter,
  paramDirection,
  packageTypeDeclaration,
  instantiation,

  /** PARSER **/

  parserDeclaration,
  parserTypeDeclaration,
  parserLocalElements,
  parserLocalElement,
  parserStates,
  parserState,
  parserStatements,
  parserStatement,
  parserBlockStatement,
  transitionStatement,
  stateExpression,
  selectExpression,
  selectCaseList,
  selectCase,
  keysetExpression,
  tupleKeysetExpression,
  simpleKeysetExpression,
  simpleExpressionList,

  /** CONTROL **/

  controlDeclaration,
  controlTypeDeclaration,
  controlLocalDeclarations,
  controlLocalDeclaration,

  /** EXTERN **/

  externDeclaration,
  externTypeDeclaration,
  methodPrototypes,
  functionPrototype,

  /** TYPES **/

  typeRef,
  tupleType,
  headerStackType,
  baseTypeBoolean,
  baseTypeInteger,
  baseTypeBit,
  baseTypeVarbit,
  baseTypeString,
  baseTypeVoid,
  baseTypeError,
  integerTypeSize,
  realTypeArg,
  typeArg,
  typeArgumentList,
  typeDeclaration,
  derivedTypeDeclaration,
  headerTypeDeclaration,
  headerUnionDeclaration,
  structTypeDeclaration,
  structFieldList,
  structField,
  enumDeclaration,
  errorDeclaration,
  matchKindDeclaration,
  identifierList,
  specifiedIdentifierList,
  specifiedIdentifier,
  typedefDeclaration,

  /** STATEMENTS **/

  assignmentStatement,
  emptyStatement,
  returnStatement,
  exitStatement,
  conditionalStatement,
  directApplication,
  statement,
  blockStatement,
  statementOrDeclaration,
  statementOrDeclList,
  switchStatement,
  switchCases,
  switchCase,
  switchLabel,

  /** TABLES **/

  tableDeclaration,
  tablePropertyList,
  tableProperty,
  keyProperty,
  keyElementList,
  keyElement,
  actionsProperty,
  actionList,
  actionRef,
#if 0
  entriesProperty,
  entriesList,
  entry,
  simpleProperty,
#endif
  actionDeclaration,

  /** VARIABLES **/

  variableDeclaration,

  /** EXPRESSIONS **/

  functionDeclaration,
  argumentList,
  argument,
  expressionList,
  expression,
  lvalueExpression,
  binaryExpression,
  unaryExpression,
  functionCall,
  memberSelector,
  castExpression,
  arraySubscript,
  indexExpression,
  integerLiteral,
  booleanLiteral,
  stringLiteral,
  default_,
  dontcare,
};
char* AstEnum_to_string(enum AstEnum ast);

enum class AstOperator : int {
  NONE = 0,

  /* Arithmetic */

  Add,
  Sub,
  Mul,
  Div,
  Neg,

  /* Logical */

  And,
  Or,
  Not,

  /* Relational */

  Eq,
  Neq,
  Less,
  Great,
  LessEq,
  GreatEq,

  /* Bitwise */

  BitwAnd,
  BitwOr,
  BitwXor,
  BitwNot,
  BitwShl,
  BitwShr,

  Mask,
};

enum class ParamDirection : int {
  None = 0,
  In = 1 << 1,
  Out = 1 << 2,
};
inline ParamDirection operator | (ParamDirection lhs, ParamDirection rhs) {
  return (ParamDirection)((int)lhs | (int)rhs);
}
inline ParamDirection operator & (ParamDirection lhs, ParamDirection rhs) {
  return (ParamDirection)((int)lhs & (int)rhs);
}

struct Ast;

/** PROGRAM **/

struct Ast_p4program {
  Ast* decl_list;
};

struct Ast_declarationList {
};

struct Ast_declaration {
  Ast* decl;
};

struct Ast_name {
  char* strname;
};

struct Ast_parameterList {
};

struct Ast_parameter {
  enum ParamDirection direction;
  Ast* name;
  Ast* type;
  Ast* init_expr;
};

struct Ast_packageTypeDeclaration {
  Ast* name;
  Ast* params;
};

struct Ast_instantiation {
  Ast* name;
  Ast* type;
  Ast* args;
};

/** PARSER **/

struct Ast_parserDeclaration {
  Ast* proto;
  Ast* ctor_params;
  Ast* local_elements;
  Ast* states;
};

struct Ast_parserTypeDeclaration {
  Ast* name;
  Ast* params;
  Ast* method_protos;
};

struct Ast_parserLocalElements {
};

struct Ast_parserLocalElement {
  Ast* element;
};

struct Ast_parserStates {
};

struct Ast_parserState {
  Ast* name;
  Ast* stmt_list;
  Ast* transition_stmt;
};

struct Ast_parserStatements {
};

struct Ast_parserStatement {
  Ast* stmt;
};

struct Ast_parserBlockStatement {
  Ast* stmt_list;
};

struct Ast_transitionStatement {
  Ast* stmt;
};

struct Ast_stateExpression {
  Ast* expr;
};

struct Ast_selectExpression {
  Ast* expr_list;
  Ast* case_list;
};

struct Ast_selectCaseList {
};

struct Ast_selectCase {
  Ast* keyset_expr;
  Ast* name;
};

struct Ast_keysetExpression {
  Ast* expr;
};

struct Ast_tupleKeysetExpression {
  Ast* expr_list;
};

struct Ast_simpleKeysetExpression {
  Ast* expr;
};

struct Ast_simpleExpressionList {
};

/** CONTROL **/

struct Ast_controlDeclaration {
  Ast* proto;
  Ast* ctor_params;
  Ast* local_decls;
  Ast* apply_stmt;
};

struct Ast_controlTypeDeclaration {
  Ast* name;
  Ast* params;
  Ast* method_protos;
};

struct Ast_controlLocalDeclarations {
};

struct Ast_controlLocalDeclaration {
  Ast* decl;
};

/** EXTERN **/

struct Ast_externDeclaration {
  Ast* decl;
};

struct Ast_externTypeDeclaration {
  Ast* name;
  Ast* method_protos;
};

struct Ast_methodPrototypes {
};

struct Ast_functionPrototype {
  Ast* return_type;
  Ast* name;
  Ast* params;
};

/** TYPES **/

struct Ast_typeRef {
  Ast* type;
};

struct Ast_tupleType {
  Ast* type_args;
};

struct Ast_headerStackType {
  Ast* type;
  Ast* stack_expr;
};

struct Ast_baseTypeBoolean {
  Ast* name;
};

struct Ast_baseTypeInteger {
  Ast* name;
  Ast* size;
};

struct Ast_baseTypeBit {
  Ast* name;
  Ast* size;
};

struct Ast_baseTypeVarbit {
  Ast* name;
  Ast* size;
};

struct Ast_baseTypeString {
  Ast* name;
};

struct Ast_baseTypeVoid {
  Ast* name;
};

struct Ast_baseTypeError {
  Ast* name;
};

struct Ast_integerTypeSize {
  Ast* size;
};

struct Ast_realTypeArg {
  Ast* arg;
};

struct Ast_typeArg {
  Ast* arg;
};

struct Ast_typeArgumentList {
};

struct Ast_typeDeclaration {
  Ast* decl;
};

struct Ast_derivedTypeDeclaration {
  Ast* decl;
};

struct Ast_headerTypeDeclaration {
  Ast* name;
  Ast* fields;
};

struct Ast_headerUnionDeclaration {
  Ast* name;
  Ast* fields;
};

struct Ast_structTypeDeclaration {
  Ast* name;
  Ast* fields;
};

struct Ast_structFieldList {
};

struct Ast_structField {
  Ast* type;
  Ast* name;
};

struct Ast_enumDeclaration {
  Ast* type_size;
  Ast* name;
  Ast* fields;
};

struct Ast_errorDeclaration {
  Ast* fields;
};

struct Ast_matchKindDeclaration {
  Ast* fields;
};

struct Ast_identifierList {
};

struct Ast_specifiedIdentifierList {
};

struct Ast_specifiedIdentifier {
  Ast* name;
  Ast* init_expr;
};

struct Ast_typedefDeclaration {
  Ast* type_ref;
  Ast* name;
};

/** STATEMENTS **/

struct Ast_assignmentStatement {
  Ast* lhs_expr;
  Ast* rhs_expr;
};

struct Ast_functionCall {
  Ast* lhs_expr;
  Ast* args;
};

struct Ast_returnStatement {
  Ast* expr;
};

struct Ast_exitStatement {
};

struct Ast_conditionalStatement {
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;
};

struct Ast_directApplication {
  Ast* name;
  Ast* args;
};

struct Ast_statement {
  Ast* stmt;
};

struct Ast_blockStatement {
  Ast* stmt_list;
};

struct Ast_statementOrDeclList {
};

struct Ast_switchStatement {
  Ast* expr;
  Ast* switch_cases;
};

struct Ast_switchCases {
};

struct Ast_switchCase {
  Ast* label;
  Ast* stmt;
};

struct Ast_switchLabel {
  Ast* label;
};

struct Ast_statementOrDeclaration {
  Ast* stmt;
};

/** TABLES **/

struct Ast_tableDeclaration {
  Ast* name;
  Ast* prop_list;
  Ast* method_protos;
};

struct Ast_tablePropertyList {
};

struct Ast_tableProperty {
  Ast* prop;
};

struct Ast_keyProperty {
  Ast* keyelem_list;
};

struct Ast_keyElementList {
};

struct Ast_keyElement {
  Ast* expr;
  Ast* match;
};

struct Ast_actionsProperty {
  Ast* action_list;
};

struct Ast_actionList {
};

struct Ast_actionRef {
  Ast* name;
  Ast* args;
};

#if 0
struct Ast_entriesProperty {
  Ast* entries_list;
};

struct Ast_entriesList {
};

struct Ast_entry {
  Ast* keyset;
  Ast* action;
};

struct Ast_simpleProperty {
  Ast* name;
  Ast* init_expr;
  bool is_const;
};
#endif

struct Ast_actionDeclaration {
  Ast* name;
  Ast* params;
  Ast* stmt;
};

/** VARIABLES **/

struct Ast_variableDeclaration {
  Ast* type;
  Ast* name;
  Ast* init_expr;
  bool is_const;
};

/** EXPRESSIONS **/

struct Ast_functionDeclaration {
  Ast* proto;
  Ast* stmt;
};

struct Ast_argumentList {
};

struct Ast_argument {
  Ast* arg;
};

struct Ast_expressionList {
};

struct Ast_lvalueExpression {
  Ast* expr;
};

struct Ast_expression {
  Ast* expr;
};

struct Ast_castExpression {
  Ast* type;
  Ast* expr;
};

struct Ast_unaryExpression {
  enum AstOperator op;
  char* strname;
  Ast* operand;
};

struct Ast_binaryExpression {
  enum AstOperator op;
  char* strname;
  Ast* left_operand;
  Ast* right_operand;
};

struct Ast_memberSelector {
  Ast* lhs_expr;
  Ast* name;
};

struct Ast_arraySubscript {
  Ast* lhs_expr;
  Ast* index_expr;
};

struct Ast_indexExpression {
  Ast* start_index;
  Ast* end_index;
};

struct Ast_integerLiteral {
  bool is_signed;
  int value;
  int width;
};

struct Ast_booleanLiteral {
  bool value;
};

struct Ast_stringLiteral {
  char* value;
};

struct Ast_default {
};

struct Ast_dontcare {
};

struct Ast {
  enum AstEnum kind;
  int line_no;
  int column_no;
  Tree tree;

  union {
/** PROGRAM **/
    struct Ast_p4program p4program;
    struct Ast_declarationList declarationList;
    struct Ast_declaration declaration;
    struct Ast_name name;
    struct Ast_parameterList parameterList;
    struct Ast_parameter parameter;
    struct Ast_packageTypeDeclaration packageTypeDeclaration;
    struct Ast_instantiation instantiation;
/** PARSER **/
    struct Ast_parserDeclaration parserDeclaration;
    struct Ast_parserTypeDeclaration parserTypeDeclaration;
    struct Ast_parserLocalElements parserLocalElements;
    struct Ast_parserLocalElement parserLocalElement;
    struct Ast_parserStates parserStates;
    struct Ast_parserState parserState;
    struct Ast_parserStatements parserStatements;
    struct Ast_parserStatement parserStatement;
    struct Ast_parserBlockStatement parserBlockStatement;
    struct Ast_transitionStatement transitionStatement;
    struct Ast_stateExpression stateExpression;
    struct Ast_selectExpression selectExpression;
    struct Ast_selectCaseList selectCaseList;
    struct Ast_selectCase selectCase;
    struct Ast_keysetExpression keysetExpression;
    struct Ast_tupleKeysetExpression tupleKeysetExpression;
    struct Ast_simpleKeysetExpression simpleKeysetExpression;
    struct Ast_simpleExpressionList simpleExpressionList;
/** CONTROL **/
    struct Ast_controlDeclaration controlDeclaration;
    struct Ast_controlTypeDeclaration controlTypeDeclaration;
    struct Ast_controlLocalDeclarations controlLocalDeclarations;
    struct Ast_controlLocalDeclaration controlLocalDeclaration;
/** EXTERN **/
    struct Ast_externDeclaration externDeclaration;
    struct Ast_externTypeDeclaration externTypeDeclaration;
    struct Ast_methodPrototypes methodPrototypes;
    struct Ast_functionPrototype functionPrototype;
/** TYPES **/
    struct Ast_typeRef typeRef;
    struct Ast_tupleType tupleType;
    struct Ast_headerStackType headerStackType;
    struct Ast_baseTypeBoolean baseTypeBoolean;
    struct Ast_baseTypeInteger baseTypeInteger;
    struct Ast_baseTypeBit baseTypeBit;
    struct Ast_baseTypeVarbit baseTypeVarbit;
    struct Ast_baseTypeString baseTypeString;
    struct Ast_baseTypeVoid baseTypeVoid;
    struct Ast_baseTypeError baseTypeError;
    struct Ast_integerTypeSize integerTypeSize;
    struct Ast_realTypeArg realTypeArg;
    struct Ast_typeArg typeArg;
    struct Ast_typeArgumentList typeArgumentList;
    struct Ast_typeDeclaration typeDeclaration;
    struct Ast_derivedTypeDeclaration derivedTypeDeclaration;
    struct Ast_headerTypeDeclaration headerTypeDeclaration;
    struct Ast_headerUnionDeclaration headerUnionDeclaration;
    struct Ast_structTypeDeclaration structTypeDeclaration;
    struct Ast_structFieldList structFieldList;
    struct Ast_structField structField;
    struct Ast_enumDeclaration enumDeclaration;
    struct Ast_errorDeclaration errorDeclaration;
    struct Ast_matchKindDeclaration matchKindDeclaration;
    struct Ast_identifierList identifierList;
    struct Ast_specifiedIdentifierList specifiedIdentifierList;
    struct Ast_specifiedIdentifier specifiedIdentifier;
    struct Ast_typedefDeclaration typedefDeclaration;
/** STATEMENTS **/
    struct Ast_assignmentStatement assignmentStatement;
    struct Ast_functionCall functionCall;
    struct Ast_returnStatement returnStatement;
    struct Ast_exitStatement exitStatement;
    struct Ast_conditionalStatement conditionalStatement;
    struct Ast_directApplication directApplication;
    struct Ast_statement statement;
    struct Ast_blockStatement blockStatement;
    struct Ast_statementOrDeclList statementOrDeclList;
    struct Ast_switchStatement switchStatement;
    struct Ast_switchCases switchCases;
    struct Ast_switchCase switchCase;
    struct Ast_switchLabel switchLabel;
    struct Ast_statementOrDeclaration statementOrDeclaration;
/** TABLES **/
    struct Ast_tableDeclaration tableDeclaration;
    struct Ast_tablePropertyList tablePropertyList;
    struct Ast_tableProperty tableProperty;
    struct Ast_keyProperty keyProperty;
    struct Ast_keyElementList keyElementList;
    struct Ast_keyElement keyElement;
    struct Ast_actionsProperty actionsProperty;
    struct Ast_actionList actionList;
    struct Ast_actionRef actionRef;
#if 0
    struct Ast_entriesProperty entriesProperty;
    struct Ast_entriesList entriesList;
    struct Ast_entry entry;
    struct Ast_simpleProperty simpleProperty;
#endif
    struct Ast_actionDeclaration actionDeclaration;
/** VARIABLES **/
    struct Ast_variableDeclaration variableDeclaration;
/** EXPRESSIONS **/
    struct Ast_functionDeclaration functionDeclaration;
    struct Ast_argumentList argumentList;
    struct Ast_argument argument;
    struct Ast_expressionList expressionList;
    struct Ast_lvalueExpression lvalueExpression;
    struct Ast_expression expression;
    struct Ast_castExpression castExpression;
    struct Ast_unaryExpression unaryExpression;
    struct Ast_binaryExpression binaryExpression;
    struct Ast_memberSelector memberSelector;
    struct Ast_arraySubscript arraySubscript;
    struct Ast_indexExpression indexExpression;
    struct Ast_integerLiteral integerLiteral;
    struct Ast_booleanLiteral booleanLiteral;
    struct Ast_stringLiteral stringLiteral;
    struct Ast_default default_;
    struct Ast_dontcare dontcare;
  };

  static Ast* owner_of(Tree* tree);
  static Ast* allocate(Arena* storage);
  Ast(enum AstEnum kind, int line_no, int column_no);
  void init(enum AstEnum kind, int line_no, int column_no);
  Ast* clone(Arena* storage);
};
