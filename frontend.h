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
  AST_packageTypeDeclaration,

  /** PARSER **/
  AST_instantiation,
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
  AST_keysetExpressionList,
  AST_tupleKeysetExpression,
  AST_defaultKeysetExpression,
  AST_dontcareKeysetExpression,

  /** CONTROL **/
  AST_controlDeclaration,
  AST_controlTypeDeclaration,
  AST_controlLocalDeclarations,
  AST_controlLocalDeclaration,

  /** EXTERN **/
  AST_externDeclaration,
  AST_externType,
  AST_methodPrototypes,
  AST_functionPrototype,
  AST_methodPrototype,

  /** TYPES **/
  AST_typeRef,
  AST_namedType,
  AST_tupleType,
  AST_headerStackType,
  AST_specializedType,
  AST_baseTypeBool,
  AST_baseTypeInteger,
  AST_baseTypeBit,
  AST_baseTypeVarbit,
  AST_baseTypeString,
  AST_baseTypeVoid,
  AST_baseTypeError,
  AST_integerTypeSize,
  AST_typeParameterList,
  AST_realTypeArgument,
  AST_typeArgument,
  AST_dontcareTypeArgument,
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
  AST_statementOrDecl,
  AST_statementOrDeclList,
  AST_switchStatement,
  AST_switchCases,
  AST_switchCase,
  AST_switchLabel,
  AST_defaultSwitchLabel,

  /** TABLES **/
  AST_tableDeclaration,
  AST_tablePropertyList,
  AST_tableProperty,
  AST_tableKey,
  AST_tableActions,
  AST_tableEntries,
  AST_simpleTableProperty,
  AST_keyElementList,
  AST_keyElement,
  AST_actionList,
  AST_actionRef,
  AST_entriesList,
  AST_entry,
  AST_actionDeclaration,

  /** VARIABLES **/
  AST_variableDeclaration,
  AST_constantDeclaration,

  /** EXPRESSIONS **/
  AST_functionDeclaration,
  AST_argumentList,
  AST_argument,
  AST_dontcareArgument,
  AST_kvPair,
  AST_expressionList,
  AST_lvalue,
  AST_expression,
  AST_binaryExpression,
  AST_unaryExpression,
  AST_functionCall,
  AST_memberSelector,
  AST_castExpression,
  AST_arraySubscript,
  AST_arrayIndex,
  AST_integerLiteral,
  AST_booleanLiteral,
  AST_stringLiteral,
};

enum AstOperator {
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

enum AstParamDirection {
  PARAMDIR_IN = 1,
  PARAMDIR_OUT,
  PARAMDIR_INOUT,
};

typedef struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Hashmap sym_table;
} Scope;

enum AstWalkDirection {
  WALK_IN = 1,
  WALK_OUT,
};

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
  int last_node_id;
} Ast_P4Program;

typedef struct Ast_List {
  Ast;
  List members;
} Ast_List;

typedef struct Ast_Declaration {
  Ast;
  Ast* decl;
} Ast_Declaration;

typedef struct Ast_Name {
  Ast;
  char* strname;
  Scope* scope;
} Ast_Name;

typedef struct Ast_Parameter {
  Ast;
  enum AstParamDirection direction;
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

/** PARSER **/

typedef struct Ast_Instantiation {
  Ast;
  Ast* name;
  Ast* type_ref;
  Ast* args;
} Ast_Instantiation;

typedef struct Ast_ParserDeclaration {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_elements;
  Ast* states;
} Ast_ParserDeclaration;

typedef struct Ast_ParserPrototype {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ParserPrototype;

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

typedef struct Ast_ParserStmt {
  Ast;
  Ast* stmt;
} Ast_ParserStmt;

typedef struct Ast_TransitionStmt {
  Ast;
  Ast* stmt;
} Ast_TransitionStmt;

typedef struct Ast_StateExpression {
  Ast;
  Ast* expr;
} Ast_StateExpression;

typedef struct Ast_SelectExpression {
  Ast;
  Ast* expr_list;
  Ast* case_list;
} Ast_SelectExpression;

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

/** CONTROL **/

typedef struct Ast_ControlDeclaration {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_decls;
  Ast* apply_stmt;
} Ast_ControlDeclaration;

typedef struct Ast_ControlPrototype {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ControlPrototype;

typedef struct Ast_ControlLocalDeclaration {
  Ast;
  Ast* decl;
} Ast_ControlLocalDeclaration;

/** EXTERN **/

typedef struct Ast_ExternDeclaration {
  Ast;
  Ast* decl;
} Ast_ExternDeclaration;

typedef struct Ast_ExternType {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* method_protos;
} Ast_ExternType;

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

typedef struct Ast_NamedType {
  Ast;
  Ast* type;
} Ast_NamedType;

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

typedef struct Ast_BoolType {
  Ast;
  Ast* name;
} Ast_BoolType;

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

typedef struct Ast_RealTypeArgument {
  Ast;
  Ast* arg;
} Ast_RealTypeArgument;

typedef struct Ast_TypeArgument {
  Ast;
  Ast* arg;
} Ast_TypeArgument;

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
} Ast_HeaderTypeDeclaration;

typedef struct Ast_HeaderUnionDeclaration {
  Ast;
  Ast* name;
  Ast* fields;
} Ast_HeaderUnionDeclaration;

typedef struct Ast_StructTypeDeclaration {
  Ast;
  Ast* name;
  Ast* fields;
} Ast_StructTypeDeclaration;

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
} Ast_EnumDeclaration;

typedef struct Ast_ErrorDeclaration {
  Ast;
  Ast* fields;
} Ast_ErrorDeclaration;

typedef struct Ast_MatchKindDeclaration {
  Ast;
  Ast* fields;
} Ast_MatchKindDeclaration;

typedef struct Ast_SpecifiedIdent {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_SpecifiedIdent;

typedef struct Ast_TypedefDeclaration {
  Ast;
  Ast* name;
  Ast* type_ref;
} Ast_TypedefDeclaration;

/** STATEMENTS **/

typedef struct Ast_AssignmentStmt {
  Ast;
  Ast* lvalue;
  Ast* expr;
} Ast_AssignmentStmt;

typedef struct Ast_ReturnStmt {
  Ast;
  Ast* expr;
} Ast_ReturnStmt;

typedef struct Ast_ConditionalStmt {
  Ast;
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;
} Ast_ConditionalStmt;

typedef struct Ast_DirectApplication {
  Ast;
  Ast* lhs_expr;
  Ast* args;
} Ast_DirectApplication;

typedef struct Ast_Statement {
  Ast;
  Ast* stmt;
} Ast_Statement;

typedef struct Ast_BlockStmt {
  Ast;
  Ast* stmt_list;
} Ast_BlockStmt;

typedef struct Ast_SwitchStmt {
  Ast;
  Ast* expr;
  Ast* switch_cases;
} Ast_SwitchStmt;

typedef struct Ast_SwitchCase {
  Ast;
  Ast* label;
  Ast* stmt;
} Ast_SwitchCase;

typedef struct Ast_SwitchLabel {
  Ast;
  Ast* label;
} Ast_SwitchLabel;

/** TABLES **/

typedef struct Ast_TableDeclaration {
  Ast;
  Ast* name;
  Ast* prop_list;
} Ast_TableDeclaration;

typedef struct Ast_TableProperty {
  Ast;
  Ast* prop;
} Ast_TableProperty;

typedef struct Ast_TableKey {
  Ast;
  Ast* keyelem_list;
} Ast_TableKey;

typedef struct Ast_TableActions {
  Ast;
  Ast* action_list;
} Ast_TableActions;

typedef struct Ast_TableEntries {
  Ast;
  bool is_const;
  Ast* entries_list;
} Ast_TableEntries;

typedef struct Ast_SimpleTableProperty {
  Ast;
  bool is_const;
  Ast* name;
  Ast* init_expr;
} Ast_SimpleTableProperty;

typedef struct Ast_KeyElement {
  Ast;
  Ast* expr;
  Ast* name;
} Ast_KeyElement;

typedef struct Ast_ActionRef {
  Ast;
  Ast* name;
  Ast* args;
} Ast_ActionRef;

typedef struct Ast_Entry {
  Ast;
  Ast* keyset;
  Ast* action;
} Ast_Entry;

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
} Ast_VarDeclaration;

typedef struct Ast_ConstDeclaration {
  Ast;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_ConstDeclaration;

/** EXPRESSIONS **/

typedef struct Ast_FunctionDeclaration {
  Ast;
  Ast* proto;
  Ast* stmt;
} Ast_FunctionDeclaration;

typedef struct Ast_Argument {
  Ast;
  Ast* arg;
} Ast_Argument;

typedef struct Ast_KVPair {
  Ast;
  Ast* name;
  Ast* expr;
} Ast_KVPair;

typedef struct Ast_Expression {
  Ast;
  Ast* expr;
  Ast* type_args;
} Ast_Expression;

typedef struct Ast_BinaryExpression {
  Ast;
  enum AstOperator op;
  Ast* left_operand;
  Ast* right_operand;
} Ast_BinaryExpression;

typedef struct Ast_UnaryExpression {
  Ast;
  enum AstOperator op;
  Ast* operand;
} Ast_UnaryExpression;

typedef struct Ast_FunctionCall {
  Ast;
  Ast* callee_expr;
  Ast* args;
} Ast_FunctionCall;

typedef struct Ast_MemberSelector {
  Ast;
  Ast* lhs_expr;
  Ast* member_name;
} Ast_MemberSelector;

typedef struct Ast_CastExpression {
  Ast;
  Ast* to_type;
  Ast* expr;
} Ast_CastExpression;

typedef struct Ast_ArraySubscript {
  Ast;
  Ast* lhs_expr;
  Ast* index_expr;
} Ast_ArraySubscript;

typedef struct Ast_ArrayIndex {
  Ast;
  Ast* start_index;
  Ast* end_index;
} Ast_ArrayIndex;

typedef struct Ast_IntegerLiteral {
  Ast_Expression;
  bool is_signed;
  int value;
  int width;
} Ast_IntegerLiteral;

typedef struct Ast_BoolLiteral {
  Ast_Expression;
  bool value;
} Ast_BoolLiteral;

typedef struct Ast_StringLiteral {
  Ast_Expression;
  char* value;
} Ast_StringLiteral;

typedef void AstVisitor(Ast*, enum AstWalkDirection);
void traverse_p4program(Ast_P4Program* p4program, AstVisitor* visitor);

typedef struct NameDecl {
  union {
    Ast* ast;
    enum TokenClass token_class;
  };
  char* strname;
  int line_no;
  int column_no;
  struct NameDecl* next_decl;
} NameDecl;

typedef struct NamespaceEntry {
  char* strname;
  NameDecl* ns_type;
  NameDecl* ns_var;
  NameDecl* ns_keyword;
} NamespaceEntry;

void symbol_table_init(Arena* scope_storage);
Scope* push_scope();
Scope* pop_scope();
NamespaceEntry* scope_lookup_name(Scope* scope, char* name);
void declare_type_name(Scope* scope, char* strname, int line_no, int column_no, Ast* ast);
void declare_var_name(Scope* scope, char* strname, int line_no, int column_no, Ast* ast);
void declare_keyword(Scope* scope, char* strname, enum TokenClass token_class);

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
  Ast* ast;
  List members;
} TypeSet;

typedef struct Type_Type {
  Type;
} Type_Type;

typedef struct Type_Vector {
  Type;
  List members;
} Type_Vector;

typedef struct Type_TypeDef {
  Type;
  Type* ref_ty;
} Type_TypeDef;

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

