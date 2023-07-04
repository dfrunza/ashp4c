#pragma once

enum TokenClass {
  /* Operators and syntactic structure */
  TK_SEMICOLON = 1,
  TK_IDENTIFIER,
  TK_TYPE_IDENTIFIER,
  TK_INTEGER_LITERAL,
  TK_STRING_LITERAL,
  TK_PARENTH_OPEN,
  TK_PARENTH_CLOSE,
  TK_ANGLE_OPEN,
  TK_ANGLE_CLOSE,
  TK_BRACE_OPEN,
  TK_BRACE_CLOSE,
  TK_BRACKET_OPEN,
  TK_BRACKET_CLOSE,
  TK_DONTCARE,
  TK_COLON,
  TK_DOT,
  TK_COMMA,
  TK_MINUS,
  TK_UNARY_MINUS,
  TK_PLUS,
  TK_STAR,
  TK_SLASH,
  TK_EQUAL,
  TK_DOUBLE_EQUAL,
  TK_EXCLAMATION_EQUAL,
  TK_EXCLAMATION,
  TK_DOUBLE_PIPE,
  TK_ANGLE_OPEN_EQUAL,
  TK_ANGLE_CLOSE_EQUAL,
  TK_TILDA,
  TK_AMPERSAND,
  TK_DOUBLE_AMPERSAND,
  TK_TRIPLE_AMPERSAND,
  TK_PIPE,
  TK_CIRCUMFLEX,
  TK_DOUBLE_ANGLE_OPEN,
  TK_DOUBLE_ANGLE_CLOSE,
  TK_COMMENT,

  /* Keywords */
  TK_ACTION,
  TK_ACTIONS,
  TK_ENUM,
  TK_IN,
  TK_PACKAGE,
  TK_SELECT,
  TK_SWITCH,
  TK_TUPLE,
  TK_VOID,
  TK_APPLY,
  TK_CONTROL,
  TK_ERROR,
  TK_HEADER,
  TK_INOUT,
  TK_PARSER,
  TK_STATE,
  TK_TABLE,
  TK_ENTRIES,
  TK_KEY,
  TK_TYPEDEF,
  TK_TYPE,
  TK_BOOL,
  TK_TRUE,
  TK_FALSE,
  TK_DEFAULT,
  TK_EXTERN,
  TK_HEADER_UNION,
  TK_INT,
  TK_BIT,
  TK_VARBIT,
  TK_STRING,
  TK_OUT,
  TK_TRANSITION,
  TK_ELSE,
  TK_EXIT,
  TK_IF,
  TK_MATCH_KIND,
  TK_RETURN,
  TK_STRUCT,
  TK_CONST,

  /* Control */
  TK_UNKNOWN,
  TK_START_OF_INPUT,
  TK_END_OF_INPUT,
  TK_LEXICAL_ERROR,
};

typedef struct Token {
  enum TokenClass klass;
  char* lexeme;
  int line_no;
  int column_no;

  union {
    struct {
      bool is_signed;
      int width;
      int64_t value;
    } i;  /* integer */
    char* str;
  };
} Token;

enum AstEnum {

  /** PROGRAM **/

  AST_p4program = 1,
  AST_declarationList,
  AST_declaration,
  AST_name,
  AST_parameterList,
  AST_parameter,
  AST_paramDirection,
  AST_packageTypeDeclaration,
  AST_instantiation,

  /** PARSER **/

  AST_parserDeclaration,
  AST_parserTypeDeclaration,
  AST_parserLocalElements,
  AST_parserLocalElement,
  AST_parserStates,
  AST_parserState,
  AST_parserStatements,
  AST_parserStatement,
  AST_parserBlockStatement,
  AST_transitionStatement,
  AST_stateExpression,
  AST_selectExpression,
  AST_selectCaseList,
  AST_selectCase,
  AST_keysetExpression,
  AST_tupleKeysetExpression,
  AST_simpleKeysetExpression,
  AST_simpleExpressionList,

  /** CONTROL **/

  AST_controlDeclaration,
  AST_controlTypeDeclaration,
  AST_controlLocalDeclarations,
  AST_controlLocalDeclaration,

  /** EXTERN **/

  AST_externDeclaration,
  AST_externTypeDeclaration,
  AST_methodPrototypes,
  AST_functionPrototype,

  /** TYPES **/

  AST_typeRef,
  AST_namedType,
  AST_tupleType,
  AST_headerStackType,
  AST_specializedType,
  AST_baseTypeBoolean,
  AST_baseTypeInteger,
  AST_baseTypeBit,
  AST_baseTypeVarbit,
  AST_baseTypeString,
  AST_baseTypeVoid,
  AST_baseTypeError,
  AST_integerTypeSize,
  AST_typeParameterList,
  AST_realTypeArg,
  AST_typeArg,
  AST_realTypeArgumentList,
  AST_typeArgumentList,
  AST_typeDeclaration,
  AST_derivedTypeDeclaration,
  AST_headerTypeDeclaration,
  AST_headerUnionDeclaration,
  AST_structTypeDeclaration,
  AST_structFieldList,
  AST_structField,
  AST_enumDeclaration,
  AST_errorDeclaration,
  AST_matchKindDeclaration,
  AST_identifierList,
  AST_specifiedIdentifierList,
  AST_specifiedIdentifier,
  AST_typedefDeclaration,

  /** STATEMENTS **/

  AST_assignmentStatement,
  AST_emptyStatement,
  AST_returnStatement,
  AST_exitStatement,
  AST_conditionalStatement,
  AST_directApplication,
  AST_statement,
  AST_blockStatement,
  AST_statementOrDeclaration,
  AST_statementOrDeclList,
  AST_switchStatement,
  AST_switchCases,
  AST_switchCase,
  AST_switchLabel,

  /** TABLES **/

  AST_tableDeclaration,
  AST_tablePropertyList,
  AST_tableProperty,
  AST_keyProperty,
  AST_keyElementList,
  AST_keyElement,
  AST_actionsProperty,
  AST_actionList,
  AST_actionRef,
  AST_entriesProperty,
  AST_entriesList,
  AST_entry,
  AST_simpleProperty,
  AST_actionDeclaration,

  /** VARIABLES **/

  AST_variableDeclaration,
  AST_constantDeclaration,

  /** EXPRESSIONS **/

  AST_functionDeclaration,
  AST_argumentList,
  AST_argument,
  AST_expressionList,
  AST_expression,
  AST_lvalueExpression,
  AST_binaryExpression,
  AST_unaryExpression,
  AST_functionCall,
  AST_memberSelector,
  AST_castExpression,
  AST_arraySubscript,
  AST_indexExpression,
  AST_integerLiteral,
  AST_booleanLiteral,
  AST_stringLiteral,
  AST_dontcare,
  AST_default,
};

enum Ast_Operator {
  /* Arithmetic */
  OP_ADD = 1,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_NEG,

  /* Logical */
  OP_AND,
  OP_OR,
  OP_NOT,

  /* Relational */
  OP_EQ,
  OP_NEQ,
  OP_LESS,
  OP_GREAT,
  OP_LESS_EQ,
  OP_GREAT_EQ,

  /* Bitwise */
  OP_BITW_AND,
  OP_BITW_OR,
  OP_BITW_XOR,
  OP_BITW_NOT,
  OP_BITW_SHL,
  OP_BITW_SHR,
  OP_MASK,
};

enum Ast_ParamDirection {
  PARAMDIR_IN = 1,
  PARAMDIR_OUT,
  PARAMDIR_INOUT,
};

typedef struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Hashmap decls;
} Scope;

typedef struct Ast {
  enum AstEnum kind;
  uint32_t id;
  int line_no;
  int column_no;
} Ast;

/** PROGRAM **/

typedef struct Ast_P4Program {
  Ast;
  Ast* decl_list;
} Ast_P4Program;

typedef struct Ast_DeclarationList {
  Ast;
  List members;
} Ast_DeclarationList;

typedef struct Ast_Declaration {
  Ast;
  Ast* decl;
} Ast_Declaration;

typedef struct Ast_Name {
  Ast;
  char* strname;
  bool is_prefixed;

  struct {
    Scope* scope;
  } attr;
} Ast_Name;

typedef struct Ast_ParameterList {
  Ast;
  List members;
} Ast_ParameterList;

typedef struct Ast_Parameter {
  Ast;
  enum Ast_ParamDirection direction;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_Parameter;

typedef struct Ast_PackageTypeDeclaration {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_PackageTypeDeclaration;

typedef struct Ast_Instantiation {
  Ast;
  Ast* name;
  Ast* type_ref;
  Ast* args;
} Ast_Instantiation;

/** PARSER **/

typedef struct Ast_ParserDeclaration {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_elements;
  Ast* states;
} Ast_ParserDeclaration;

typedef struct Ast_ParserLocalElements {
  Ast;
  List members;
} Ast_ParserLocalElements;

typedef struct Ast_ParserTypeDeclaration {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ParserTypeDeclaration;

typedef struct Ast_ParserStates {
  Ast;
  List members;
} Ast_ParserStates;

typedef struct Ast_ParserLocalElement {
  Ast;
  Ast* element;
} Ast_ParserLocalElement;

typedef struct Ast_ParserState {
  Ast;
  Ast* name;
  Ast* stmt_list;
  Ast* transition_stmt;
} Ast_ParserState;

typedef struct Ast_ParserStatements {
  Ast;
  List members;
} Ast_ParserStatements;

typedef struct Ast_ParserStatement {
  Ast;
  Ast* stmt;
} Ast_ParserStatement;

typedef struct Ast_ParserBlockStatement {
  Ast;
  Ast* stmt_list;
} Ast_ParserBlockStatement;

typedef struct Ast_TransitionStatement {
  Ast;
  Ast* stmt;
} Ast_TransitionStatement;

typedef struct Ast_StateExpression {
  Ast;
  Ast* expr;
} Ast_StateExpression;

typedef struct Ast_SelectExpression {
  Ast;
  Ast* expr_list;
  Ast* case_list;
} Ast_SelectExpression;

typedef struct Ast_SelectCaseList {
  Ast;
  List members;
} Ast_SelectCaseList;

typedef struct Ast_SelectCase {
  Ast;
  Ast* keyset_expr;
  Ast* name;
} Ast_SelectCase;

typedef struct Ast_KeysetExpression {
  Ast;
  Ast* expr;
} Ast_KeysetExpression;

typedef struct Ast_TupleKeysetExpression {
  Ast;
  Ast* expr_list;
} Ast_TupleKeysetExpression;

typedef struct Ast_SimpleKeysetExpression {
  Ast;
  Ast* expr;
} Ast_SimpleKeysetExpression;

typedef struct Ast_SimpleExpressionList {
  Ast;
  List members;
} Ast_SimpleExpressionList;

/** CONTROL **/

typedef struct Ast_ControlDeclaration {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_decls;
  Ast* apply_stmt;
} Ast_ControlDeclaration;

typedef struct Ast_ControlTypeDeclaration {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ControlTypeDeclaration;

typedef struct Ast_ControlLocalDeclaration {
  Ast;
  Ast* decl;
} Ast_ControlLocalDeclaration;

typedef struct Ast_ControlLocalDeclarations {
  Ast;
  List members;
} Ast_ControlLocalDeclarations;

/** EXTERN **/

typedef struct Ast_ExternDeclaration {
  Ast;
  Ast* decl;
} Ast_ExternDeclaration;

typedef struct Ast_MethodPrototypes {
  Ast;
  List members;
} Ast_MethodPrototypes;

typedef struct Ast_ExternTypeDeclaration {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* method_protos;
} Ast_ExternTypeDeclaration;

typedef struct Ast_FunctionPrototype {
  Ast;
  Ast* name;
  Ast* return_type;
  Ast* type_params;
  Ast* params;
} Ast_FunctionPrototype;

/** TYPES **/

typedef struct Ast_TypeRef {
  Ast;
  Ast* type;
} Ast_TypeRef;

typedef struct Ast_TupleType {
  Ast;
  Ast* type_args;
} Ast_TupleType;

typedef struct Ast_HeaderStackType {
  Ast;
  Ast* name;
  Ast* stack_expr;
} Ast_HeaderStackType;

typedef struct Ast_SpecializedType {
  Ast;
  Ast* name;
  Ast* type_args;
} Ast_SpecializedType;

typedef struct Ast_BooleanType {
  Ast;
  Ast* name;
} Ast_BooleanType;

typedef struct Ast_IntegerType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_IntegerType;

typedef struct Ast_BitType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_BitType;

typedef struct Ast_VarbitType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_VarbitType;

typedef struct Ast_StringType {
  Ast;
  Ast* name;
} Ast_StringType;

typedef struct Ast_VoidType {
  Ast;
  Ast* name;
} Ast_VoidType;

typedef struct Ast_ErrorType {
  Ast;
  Ast* name;
} Ast_ErrorType;

typedef struct Ast_IntegerTypeSize {
  Ast;
  Ast* size;
} Ast_IntegerTypeSize;

typedef struct Ast_TypeParameterList {
  Ast;
  List members;
} Ast_TypeParameterList;

typedef struct Ast_RealTypeArg {
  Ast;
  Ast* arg;
} Ast_RealTypeArg;

typedef struct Ast_TypeArg {
  Ast;
  Ast* arg;
} Ast_TypeArg;

typedef struct Ast_RealTypeArgumentList {
  Ast;
  List members;
} Ast_RealTypeArgumentList;

typedef struct Ast_TypeArgumentList {
  Ast;
  List members;
} Ast_TypeArgumentList;

typedef struct Ast_TypeDeclaration {
  Ast;
  Ast* decl;
} Ast_TypeDeclaration;

typedef struct Ast_DerivedTypeDeclaration {
  Ast;
  Ast* decl;
} Ast_DerivedTypeDeclaration;

typedef struct Ast_HeaderTypeDeclaration {
  Ast;
  Ast* name;
  Ast* fields;

  struct {
    int field_count;
  } attr;
} Ast_HeaderTypeDeclaration;

typedef struct Ast_HeaderUnionDeclaration {
  Ast;
  Ast* name;
  Ast* fields;

  struct {
    int field_count;
  } attr;
} Ast_HeaderUnionDeclaration;

typedef struct Ast_StructTypeDeclaration {
  Ast;
  Ast* name;
  Ast* fields;

  struct {
    int field_count;
  } attr;
} Ast_StructTypeDeclaration;

typedef struct Ast_StructFieldList {
  Ast;
  List members;
} Ast_StructFieldList;

typedef struct Ast_StructField {
  Ast;
  Ast* type;
  Ast* name;
} Ast_StructField;

typedef struct Ast_EnumDeclaration {
  Ast;
  Ast* type_size;
  Ast* name;
  Ast* fields;

  struct {
    int field_count;
    Hashmap fields;
  } attr;
} Ast_EnumDeclaration;

typedef struct Ast_ErrorDeclaration {
  Ast;
  Ast* fields;

  struct {
    int field_count;
  } attr;
} Ast_ErrorDeclaration;

typedef struct Ast_MatchKindDeclaration {
  Ast;
  Ast* fields;

  struct {
    int field_count;
  } attr;
} Ast_MatchKindDeclaration;

typedef struct Ast_IdentifierList {
  Ast;
  List members;
} Ast_IdentifierList;

typedef struct Ast_SpecifiedIdentifierList {
  Ast;
  List members;
} Ast_SpecifiedIdentifierList;

typedef struct Ast_SpecifiedIdentifier {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_SpecifiedIdentifier;

typedef struct Ast_TypedefDeclaration {
  Ast;
  Ast* name;
  Ast* type_ref;
} Ast_TypedefDeclaration;

/** STATEMENTS **/

typedef struct Ast_AssignmentStatement {
  Ast;
  Ast* lhs_expr;
  Ast* rhs_expr;
} Ast_AssignmentStatement;

typedef struct Ast_FunctionCall {
  Ast;
  Ast* lhs_expr;
  Ast* args;
} Ast_FunctionCall;

typedef struct Ast_ReturnStatement {
  Ast;
  Ast* expr;
} Ast_ReturnStatement;

typedef struct Ast_ExitStatement {
  Ast;
} Ast_ExitStatement;

typedef struct Ast_ConditionalStatement {
  Ast;
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;
} Ast_ConditionalStatement;

typedef struct Ast_DirectApplication {
  Ast;
  Ast* name;
  Ast* args;
} Ast_DirectApplication;

typedef struct Ast_Statement {
  Ast;
  Ast* stmt;
} Ast_Statement;

typedef struct Ast_BlockStatement {
  Ast;
  Ast* stmt_list;
} Ast_BlockStatement;

typedef struct Ast_StatementOrDeclList {
  Ast;
  List members;
} Ast_StatementOrDeclList;

typedef struct Ast_SwitchStatement {
  Ast;
  Ast* expr;
  Ast* switch_cases;
} Ast_SwitchStatement;

typedef struct Ast_SwitchCases {
  Ast;
  List members;
} Ast_SwitchCases;

typedef struct Ast_SwitchCase {
  Ast;
  Ast* label;
  Ast* stmt;
} Ast_SwitchCase;

typedef struct Ast_SwitchLabel {
  Ast;
  Ast* label;
} Ast_SwitchLabel;

typedef struct Ast_StatementOrDeclaration {
  Ast;
  Ast* stmt;
} Ast_StatementOrDeclaration;

/** TABLES **/

typedef struct Ast_TableDeclaration {
  Ast;
  Ast* name;
  Ast* prop_list;
} Ast_TableDeclaration;

typedef struct Ast_TablePropertyList {
  Ast;
  List members;
} Ast_TablePropertyList;

typedef struct Ast_TableProperty {
  Ast;
  Ast* prop;
} Ast_TableProperty;

typedef struct Ast_KeyProperty {
  Ast;
  Ast* keyelem_list;
} Ast_KeyProperty;

typedef struct Ast_KeyElementList {
  Ast;
  List members;
} Ast_KeyElementList;

typedef struct Ast_KeyElement {
  Ast;
  Ast* expr;
  Ast* match;
} Ast_KeyElement;

typedef struct Ast_ActionsProperty {
  Ast;
  Ast* action_list;
} Ast_ActionsProperty;

typedef struct Ast_ActionList {
  Ast;
  List members;
} Ast_ActionList;

typedef struct Ast_ActionRef {
  Ast;
  Ast* name;
  Ast* args;
} Ast_ActionRef;

typedef struct Ast_EntriesProperty {
  Ast;
  Ast* entries_list;
} Ast_EntriesProperty;

typedef struct Ast_EntriesList {
  Ast;
  List members;
} Ast_EntriesList;

typedef struct Ast_Entry {
  Ast;
  Ast* keyset;
  Ast* action;
} Ast_Entry;

typedef struct Ast_SimpleProperty {
  Ast;
  Ast* name;
  Ast* init_expr;
  bool is_const;
} Ast_SimpleProperty;

typedef struct Ast_ActionDeclaration {
  Ast;
  Ast* name;
  Ast* params;
  Ast* stmt;
} Ast_ActionDeclaration;

/** VARIABLES **/

typedef struct Ast_VarDeclaration {
  Ast;
  Ast* name;
  Ast* type;
  Ast* init_expr;
  bool is_const;
} Ast_VarDeclaration;

/** EXPRESSIONS **/

typedef struct Ast_FunctionDeclaration {
  Ast;
  Ast* proto;
  Ast* stmt;
} Ast_FunctionDeclaration;

typedef struct Ast_ArgumentList {
  Ast;
  List members;
} Ast_ArgumentList;

typedef struct Ast_Argument {
  Ast;
  Ast* arg;
} Ast_Argument;

typedef struct Ast_ExpressionList {
  Ast;
  List members;
} Ast_ExpressionList;

typedef struct Ast_LvalueExpression {
  Ast;
  Ast* expr;
  Ast* type_args;
} Ast_LvalueExpression;

typedef struct Ast_Expression {
  Ast;
  Ast* expr;
  Ast* type_args;
} Ast_Expression;

typedef struct Ast_CastExpression {
  Ast;
  Ast* type;
  Ast* expr;
} Ast_CastExpression;

typedef struct Ast_UnaryExpression {
  Ast;
  enum Ast_Operator op;
  Ast* operand;
} Ast_UnaryExpression;

typedef struct Ast_BinaryExpression {
  Ast;
  enum Ast_Operator op;
  Ast* left_operand;
  Ast* right_operand;
} Ast_BinaryExpression;

typedef struct Ast_MemberSelector {
  Ast;
  Ast* lhs_expr;
  Ast* name;
} Ast_MemberSelector;

typedef struct Ast_ArraySubscript {
  Ast;
  Ast* lhs_expr;
  Ast* index_expr;
} Ast_ArraySubscript;

typedef struct Ast_IndexExpression {
  Ast;
  Ast* start_index;
  Ast* end_index;
} Ast_IndexExpression;

typedef struct Ast_IntegerLiteral {
  Ast;
  bool is_signed;
  int value;
  int width;
} Ast_IntegerLiteral;

typedef struct Ast_BooleanLiteral {
  Ast;
  bool value;
} Ast_BooleanLiteral;

typedef struct Ast_StringLiteral {
  Ast;
  char* value;
} Ast_StringLiteral;

typedef struct Ast_Dontcare {
  Ast;
} Ast_Dontcare;

typedef struct Ast_Default {
  Ast;
} Ast_Default;

typedef struct NameDecl {
  union {
    Ast* ast;
    enum TokenClass token_class;
  };
  char* strname;
  int line_no;
  int column_no;
  struct NameDecl* next_in_scope;
} NameDecl;

typedef enum Namespace {
  NS_TYPE = 0,
  NS_VAR,
  NS_KEYWORD,
} Namespace;

typedef struct NamespaceEntry {
  char* strname;
  NameDecl* decls[3];
} NamespaceEntry;

Scope* push_scope(Arena* storage, Scope* parent_scope);
Scope* pop_scope(Scope* scope);
NamespaceEntry* scope_lookup_name(Scope* scope, char* name);
NameDecl* declare_name(Arena* storage, Hashmap* decls, char* strname, enum Namespace ns,
            int line_no, int column_no);
NameDecl* declare_struct_field(Arena* storage, Hashmap* fields, char* strname,
            int line_no, int column_no);

enum TypeEnum {
  TYPE_VOID = 1,
  TYPE_BOOL,
  TYPE_INT,
  TYPE_BIT,
  TYPE_VARBIT,
  TYPE_STRING,
  TYPE_TYPE,
  TYPE_TYPEDEF,
  TYPE_PRODUCT,
  TYPE_UNION,
  TYPE_FUNCTION,
  TYPE_ARRAY,
};

typedef struct Type {
  enum TypeEnum ctor;
  char* strname;
} Type;

typedef struct TypeSet {
  List members;
} TypeSet;

typedef struct Type_Type {
  Type;
} Type_Type;

typedef struct Type_Basic {
  Type;
} Type_Basic;

typedef struct Type_Typedef {
  Type;
  Type* ref_ty;
} Type_Typedef;

typedef struct Type_Product {
  Type;
  List members;
} Type_Product;

typedef struct Type_Union {
  Type;
  List members;
} Type_Union;

typedef struct Type_Function {
  Type;
  Type* params_ty;
  Type* return_ty;
} Type_Function;

typedef struct Type_Array {
  Type;
  Type* element_ty;
  int size;
} Type_Array;

void tyset_add_type(Arena *type_storage, TypeSet* set, Type* type);
void tyset_import_set(Arena *type_storage, TypeSet* to_set, TypeSet* from_set);
bool tyset_contains_type(TypeSet* set, Type* target_type);

