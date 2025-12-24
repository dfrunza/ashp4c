#pragma once
#include <arena.h>
#include <tree.h>
#include <ast.h>

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
  Tree<Ast> tree;

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

  static inline Ast* owner_of(Tree<Ast>* tree)
  {
    return ::owner_of(tree, &Ast::tree);
  }

  static Ast* create(Arena* storage, enum AstEnum kind)
  {
    Ast* ast = storage->allocate<Ast>();
    ast->kind = kind;
    return ast;
  }

  Ast* clone(Arena* storage)
  {
    Ast* clone, *sibling_clone, *child_clone;

    if (this == 0) return (Ast*)0;
    clone = storage->allocate<Ast>();
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
};
