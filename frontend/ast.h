#pragma once
#include "memory/arena.h"
#include "adt/tree.h"
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

  static Ast* allocate(Arena* storage);
};

struct Ast_declarationList {
  static Ast* allocate(Arena* storage);
};

struct Ast_declaration {
  Ast* decl;
  static Ast* allocate(Arena* storage);
};

struct Ast_name {
  char* strname;

  static Ast* allocate(Arena* storage);
};

struct Ast_parameterList {
  static Ast* allocate(Arena* storage);
};

struct Ast_parameter {
  enum ParamDirection direction;
  Ast* name;
  Ast* type;
  Ast* init_expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_packageTypeDeclaration {
  Ast* name;
  Ast* params;

  static Ast* allocate(Arena* storage);
};

struct Ast_instantiation {
  Ast* name;
  Ast* type;
  Ast* args;

  static Ast* allocate(Arena* storage);
};

/** PARSER **/

struct Ast_parserDeclaration {
  Ast* proto;
  Ast* ctor_params;
  Ast* local_elements;
  Ast* states;

  static Ast* allocate(Arena* storage);
};

struct Ast_parserTypeDeclaration {
  Ast* name;
  Ast* params;
  Ast* method_protos;

  static Ast* allocate(Arena* storage);
};

struct Ast_parserLocalElements {
  static Ast* allocate(Arena* storage);
};

struct Ast_parserLocalElement {
  Ast* element;

  static Ast* allocate(Arena* storage);
};

struct Ast_parserStates {
  static Ast* allocate(Arena* storage);
};

struct Ast_parserState {
  Ast* name;
  Ast* stmt_list;
  Ast* transition_stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_parserStatements {
  static Ast* allocate(Arena* storage);
};

struct Ast_parserStatement {
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_parserBlockStatement {
  Ast* stmt_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_transitionStatement {
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_stateExpression {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_selectExpression {
  Ast* expr_list;
  Ast* case_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_selectCaseList {
  static Ast* allocate(Arena* storage);
};

struct Ast_selectCase {
  Ast* keyset_expr;
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_keysetExpression {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_tupleKeysetExpression {
  Ast* expr_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_simpleKeysetExpression {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_simpleExpressionList {

  static Ast* allocate(Arena* storage);
};

/** CONTROL **/

struct Ast_controlDeclaration {
  Ast* proto;
  Ast* ctor_params;
  Ast* local_decls;
  Ast* apply_stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_controlTypeDeclaration {
  Ast* name;
  Ast* params;
  Ast* method_protos;

  static Ast* allocate(Arena* storage);
};

struct Ast_controlLocalDeclarations {

  static Ast* allocate(Arena* storage);
};

struct Ast_controlLocalDeclaration {
  Ast* decl;

  static Ast* allocate(Arena* storage);
};

/** EXTERN **/

struct Ast_externDeclaration {
  Ast* decl;

  static Ast* allocate(Arena* storage);
};

struct Ast_externTypeDeclaration {
  Ast* name;
  Ast* method_protos;

  static Ast* allocate(Arena* storage);
};

struct Ast_methodPrototypes {
  static Ast* allocate(Arena* storage);
};

struct Ast_functionPrototype {
  Ast* return_type;
  Ast* name;
  Ast* params;

  static Ast* allocate(Arena* storage);
};

/** TYPES **/

struct Ast_typeRef {
  Ast* type;

  static Ast* allocate(Arena* storage);
};

struct Ast_tupleType {
  Ast* type_args;

  static Ast* allocate(Arena* storage);
};

struct Ast_headerStackType {
  Ast* type;
  Ast* stack_expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeBoolean {
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeInteger {
  Ast* name;
  Ast* size;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeBit {
  Ast* name;
  Ast* size;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeVarbit {
  Ast* name;
  Ast* size;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeString {
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeVoid {
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_baseTypeError {
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_integerTypeSize {
  Ast* size;

  static Ast* allocate(Arena* storage);
};

struct Ast_realTypeArg {
  Ast* arg;

  static Ast* allocate(Arena* storage);
};

struct Ast_typeArg {
  Ast* arg;

  static Ast* allocate(Arena* storage);
};

struct Ast_typeArgumentList {
  static Ast* allocate(Arena* storage);
};

struct Ast_typeDeclaration {
  Ast* decl;

  static Ast* allocate(Arena* storage);
};

struct Ast_derivedTypeDeclaration {
  Ast* decl;

  static Ast* allocate(Arena* storage);
};

struct Ast_headerTypeDeclaration {
  Ast* name;
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_headerUnionDeclaration {
  Ast* name;
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_structTypeDeclaration {
  Ast* name;
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_structFieldList {
  static Ast* allocate(Arena* storage);
};

struct Ast_structField {
  Ast* type;
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_enumDeclaration {
  Ast* type_size;
  Ast* name;
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_errorDeclaration {
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_matchKindDeclaration {
  Ast* fields;

  static Ast* allocate(Arena* storage);
};

struct Ast_identifierList {
  static Ast* allocate(Arena* storage);
};

struct Ast_specifiedIdentifierList {
  static Ast* allocate(Arena* storage);
};

struct Ast_specifiedIdentifier {
  Ast* name;
  Ast* init_expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_typedefDeclaration {
  Ast* type_ref;
  Ast* name;

  static Ast* allocate(Arena* storage);
};

/** STATEMENTS **/

struct Ast_assignmentStatement {
  Ast* lhs_expr;
  Ast* rhs_expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_emptyStatement {
  static Ast* allocate(Arena* storage);
};

struct Ast_functionCall {
  Ast* lhs_expr;
  Ast* args;

  static Ast* allocate(Arena* storage);
};

struct Ast_returnStatement {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_exitStatement {
  static Ast* allocate(Arena* storage);
};

struct Ast_conditionalStatement {
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_directApplication {
  Ast* name;
  Ast* args;

  static Ast* allocate(Arena* storage);
};

struct Ast_statement {
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_blockStatement {
  Ast* stmt_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_statementOrDeclList {
  static Ast* allocate(Arena* storage);
};

struct Ast_switchStatement {
  Ast* expr;
  Ast* switch_cases;

  static Ast* allocate(Arena* storage);
};

struct Ast_switchCases {
  static Ast* allocate(Arena* storage);
};

struct Ast_switchCase {
  Ast* label;
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_switchLabel {
  Ast* label;

  static Ast* allocate(Arena* storage);
};

struct Ast_statementOrDeclaration {
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

/** TABLES **/

struct Ast_tableDeclaration {
  Ast* name;
  Ast* prop_list;
  Ast* method_protos;

  static Ast* allocate(Arena* storage);
};

struct Ast_tablePropertyList {
  static Ast* allocate(Arena* storage);
};

struct Ast_tableProperty {
  Ast* prop;

  static Ast* allocate(Arena* storage);
};

struct Ast_keyProperty {
  Ast* keyelem_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_keyElementList {
  static Ast* allocate(Arena* storage);
};

struct Ast_keyElement {
  Ast* expr;
  Ast* match;

  static Ast* allocate(Arena* storage);
};

struct Ast_actionsProperty {
  Ast* action_list;

  static Ast* allocate(Arena* storage);
};

struct Ast_actionList {
  static Ast* allocate(Arena* storage);
};

struct Ast_actionRef {
  Ast* name;
  Ast* args;

  static Ast* allocate(Arena* storage);
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

  static Ast* allocate(Arena* storage);
};

/** VARIABLES **/

struct Ast_variableDeclaration {
  Ast* type;
  Ast* name;
  Ast* init_expr;
  bool is_const;

  static Ast* allocate(Arena* storage);
};

/** EXPRESSIONS **/

struct Ast_functionDeclaration {
  Ast* proto;
  Ast* stmt;

  static Ast* allocate(Arena* storage);
};

struct Ast_argumentList {
  static Ast* allocate(Arena* storage);
};

struct Ast_argument {
  Ast* arg;

  static Ast* allocate(Arena* storage);
};

struct Ast_expressionList {
  static Ast* allocate(Arena* storage);
};

struct Ast_lvalueExpression {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_expression {
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_castExpression {
  Ast* type;
  Ast* expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_unaryExpression {
  enum AstOperator op;
  char* strname;
  Ast* operand;

  static Ast* allocate(Arena* storage);
};

struct Ast_binaryExpression {
  enum AstOperator op;
  char* strname;
  Ast* left_operand;
  Ast* right_operand;

  static Ast* allocate(Arena* storage);
};

struct Ast_memberSelector {
  Ast* lhs_expr;
  Ast* name;

  static Ast* allocate(Arena* storage);
};

struct Ast_arraySubscript {
  Ast* lhs_expr;
  Ast* index_expr;

  static Ast* allocate(Arena* storage);
};

struct Ast_indexExpression {
  Ast* start_index;
  Ast* end_index;

  static Ast* allocate(Arena* storage);
};

struct Ast_integerLiteral {
  bool is_signed;
  int value;
  int width;

  static Ast* allocate(Arena* storage);
};

struct Ast_booleanLiteral {
  bool value;

  static Ast* allocate(Arena* storage);
};

struct Ast_stringLiteral {
  char* value;

  static Ast* allocate(Arena* storage);
};

struct Ast_default {
  static Ast* allocate(Arena* storage);
};

struct Ast_dontcare {
  static Ast* allocate(Arena* storage);
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
  Ast* clone(Arena* storage);
};
