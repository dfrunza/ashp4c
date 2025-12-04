#pragma once
#include "arena.h"
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

  ADD,
  SUB,
  MUL,
  DIV,
  NEG,

  /* Logical */

  AND,
  OR,
  NOT,

  /* Relational */

  EQ,
  NEQ,
  LESS,
  GREAT,
  LESS_EQ,
  GREAT_EQ,

  /* Bitwise */

  BITW_AND,
  BITW_OR,
  BITW_XOR,
  BITW_NOT,
  BITW_SHL,
  BITW_SHR,

  MASK,
};

enum class ParamDirection : int {
  NONE = 0,
  IN = 1 << 1,
  OUT = 1 << 2,
};
inline ParamDirection operator | (ParamDirection lhs, ParamDirection rhs) {
  return (ParamDirection)((int)lhs | (int)rhs);
}
inline ParamDirection operator & (ParamDirection lhs, ParamDirection rhs) {
  return (ParamDirection)((int)lhs & (int)rhs);
}

struct AstTree {
  AstTree* first_child;
  AstTree* right_sibling;
};

struct AstTreeCtor {
  AstTree* last_sibling;

  void append_node(AstTree* tree, AstTree* node);
};

struct Ast {
  enum AstEnum kind;
  int line_no;
  int column_no;
  AstTree tree;

  union {

    /** PROGRAM **/

    struct {
      Ast* decl_list;
    } p4program;

    struct {
    } declarationList;

    struct {
      Ast* decl;
    } declaration;

    struct {
      char* strname;
    } name;

    struct {
    } parameterList;

    struct {
      enum ParamDirection direction;
      Ast* name;
      Ast* type;
      Ast* init_expr;
    } parameter;

    struct {
      Ast* name;
      Ast* params;
    } packageTypeDeclaration;

    struct {
      Ast* name;
      Ast* type;
      Ast* args;
    } instantiation;

    /** PARSER **/

    struct {
      Ast* proto;
      Ast* ctor_params;
      Ast* local_elements;
      Ast* states;
    } parserDeclaration;

    struct {
      Ast* name;
      Ast* params;
      Ast* method_protos;
    } parserTypeDeclaration;

    struct {
    } parserLocalElements;

    struct {
      Ast* element;
    } parserLocalElement;

    struct {
    } parserStates;

    struct {
      Ast* name;
      Ast* stmt_list;
      Ast* transition_stmt;
    } parserState;

    struct {
    } parserStatements;

    struct {
      Ast* stmt;
    } parserStatement;

    struct {
      Ast* stmt_list;
    } parserBlockStatement;

    struct {
      Ast* stmt;
    } transitionStatement;

    struct {
      Ast* expr;
    } stateExpression;

    struct {
      Ast* expr_list;
      Ast* case_list;
    } selectExpression;

    struct {
    } selectCaseList;

    struct {
      Ast* keyset_expr;
      Ast* name;
    } selectCase;

    struct {
      Ast* expr;
    } keysetExpression;

    struct {
      Ast* expr_list;
    } tupleKeysetExpression;

    struct {
      Ast* expr;
    } simpleKeysetExpression;

    struct {
    } simpleExpressionList;

    /** CONTROL **/

    struct {
      Ast* proto;
      Ast* ctor_params;
      Ast* local_decls;
      Ast* apply_stmt;
    } controlDeclaration;

    struct {
      Ast* name;
      Ast* params;
      Ast* method_protos;
    } controlTypeDeclaration;

    struct {
    } controlLocalDeclarations;

    struct {
      Ast* decl;
    } controlLocalDeclaration;

    /** EXTERN **/

    struct {
      Ast* decl;
    } externDeclaration;

    struct {
      Ast* name;
      Ast* method_protos;
    } externTypeDeclaration;

    struct {
    } methodPrototypes;

    struct {
      Ast* return_type;
      Ast* name;
      Ast* params;
    } functionPrototype;

    /** TYPES **/

    struct {
      Ast* type;
    } typeRef;

    struct {
      Ast* type_args;
    } tupleType;

    struct {
      Ast* type;
      Ast* stack_expr;
    } headerStackType;

    struct {
      Ast* name;
    } baseTypeBoolean;

    struct {
      Ast* name;
      Ast* size;
    } baseTypeInteger;

    struct {
      Ast* name;
      Ast* size;
    } baseTypeBit;

    struct {
      Ast* name;
      Ast* size;
    } baseTypeVarbit;

    struct {
      Ast* name;
    } baseTypeString;

    struct {
      Ast* name;
    } baseTypeVoid;

    struct {
      Ast* name;
    } baseTypeError;

    struct {
      Ast* size;
    } integerTypeSize;

    struct {
      Ast* arg;
    } realTypeArg;

    struct {
      Ast* arg;
    } typeArg;

    struct {
    } typeArgumentList;

    struct {
      Ast* decl;
    } typeDeclaration;

    struct {
      Ast* decl;
    } derivedTypeDeclaration;

    struct {
      Ast* name;
      Ast* fields;
    } headerTypeDeclaration;

    struct {
      Ast* name;
      Ast* fields;
    } headerUnionDeclaration;

    struct {
      Ast* name;
      Ast* fields;
    } structTypeDeclaration;

    struct {
    } structFieldList;

    struct {
      Ast* type;
      Ast* name;
    } structField;

    struct {
      Ast* type_size;
      Ast* name;
      Ast* fields;
    } enumDeclaration;

    struct {
      Ast* fields;
    } errorDeclaration;

    struct {
      Ast* fields;
    } matchKindDeclaration;

    struct {
    } identifierList;

    struct {
    } specifiedIdentifierList;

    struct {
      Ast* name;
      Ast* init_expr;
    } specifiedIdentifier;

    struct {
      Ast* type_ref;
      Ast* name;
    } typedefDeclaration;

    /** STATEMENTS **/

    struct {
      Ast* lhs_expr;
      Ast* rhs_expr;
    } assignmentStatement;

    struct {
      Ast* lhs_expr;
      Ast* args;
    } functionCall;

    struct {
      Ast* expr;
    } returnStatement;

    struct {
    } exitStatement;

    struct {
      Ast* cond_expr;
      Ast* stmt;
      Ast* else_stmt;
    } conditionalStatement;

    struct {
      Ast* name;
      Ast* args;
    } directApplication;

    struct {
      Ast* stmt;
    } statement;

    struct {
      Ast* stmt_list;
    } blockStatement;

    struct {
    } statementOrDeclList;

    struct {
      Ast* expr;
      Ast* switch_cases;
    } switchStatement;

    struct {
    } switchCases;

    struct {
      Ast* label;
      Ast* stmt;
    } switchCase;

    struct {
      Ast* label;
    } switchLabel;

    struct {
      Ast* stmt;
    } statementOrDeclaration;

    /** TABLES **/

    struct {
      Ast* name;
      Ast* prop_list;
      Ast* method_protos;
    } tableDeclaration;

    struct {
    } tablePropertyList;

    struct {
      Ast* prop;
    } tableProperty;

    struct {
      Ast* keyelem_list;
    } keyProperty;

    struct {
    } keyElementList;

    struct {
      Ast* expr;
      Ast* match;
    } keyElement;

    struct {
      Ast* action_list;
    } actionsProperty;

    struct {
    } actionList;

    struct {
      Ast* name;
      Ast* args;
    } actionRef;

#if 0
    struct {
      Ast* entries_list;
    } entriesProperty;

    struct {
    } entriesList;

    struct {
      Ast* keyset;
      Ast* action;
    } entry;

    struct {
      Ast* name;
      Ast* init_expr;
      bool is_const;
    } simpleProperty;
#endif

    struct {
      Ast* name;
      Ast* params;
      Ast* stmt;
    } actionDeclaration;

    /** VARIABLES **/

    struct {
      Ast* type;
      Ast* name;
      Ast* init_expr;
      bool is_const;
    } variableDeclaration;

    /** EXPRESSIONS **/

    struct {
      Ast* proto;
      Ast* stmt;
    } functionDeclaration;

    struct {
    } argumentList;

    struct {
      Ast* arg;
    } argument;

    struct {
    } expressionList;

    struct {
      Ast* expr;
    } lvalueExpression;

    struct {
      Ast* expr;
    } expression;

    struct {
      Ast* type;
      Ast* expr;
    } castExpression;

    struct {
      enum AstOperator op;
      char* strname;
      Ast* operand;
    } unaryExpression;

    struct {
      enum AstOperator op;
      char* strname;
      Ast* left_operand;
      Ast* right_operand;
    } binaryExpression;

    struct {
      Ast* lhs_expr;
      Ast* name;
    } memberSelector;

    struct {
      Ast* lhs_expr;
      Ast* index_expr;
    } arraySubscript;

    struct {
      Ast* start_index;
      Ast* end_index;
    } indexExpression;

    struct {
      bool is_signed;
      int value;
      int width;
    } integerLiteral;

    struct {
      bool value;
    } booleanLiteral;

    struct {
      char* value;
    } stringLiteral;

    struct {
    } default_, dontcare;
  };

  Ast* clone(Arena* storage);
};
