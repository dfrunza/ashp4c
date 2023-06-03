#pragma once

enum TokenClass {
  /* Operators and syntactic structure */
  TK_SEMICOLON = 1,
  TK_IDENTIFIER,
  TK_TYPE_IDENTIFIER,
  TK_INT_LITERAL,
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
  TK_DOTPREFIX,
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
  AST_name = 1,
  AST_baseTypeBool,
  AST_baseTypeInt,
  AST_baseTypeBit,
  AST_baseTypeVarbit,
  AST_baseTypeString,
  AST_baseTypeVoid,
  AST_baseTypeError,
  AST_constantDeclaration,
  AST_externDeclaration,
  AST_actionDeclaration,
  AST_headerTypeDeclaration,
  AST_headerUnionDeclaration,
  AST_headerStackType,
  AST_structTypeDeclaration,
  AST_structField,
  AST_enumDeclaration,
  AST_typedefDeclaration,
  AST_parserDeclaration,
  AST_parserTypeDeclaration,
  AST_parserState,
  AST_parserStates,
  AST_parserBlockStatement,
  AST_parserLocalElement,
  AST_parserLocalElements,
  AST_parserStatements,
  AST_parserTransition,
  AST_controlDeclaration,
  AST_controlTypeDeclaration,
  AST_packageTypeDeclaration,
  AST_instantiation,
  AST_errorDeclaration,
  AST_matchKindDeclaration,
  AST_functionDeclaration,
  AST_functionPrototype,
  AST_dontcareArgument,
  AST_dontcareTypeArgument,
  AST_integerTypeSize,
  AST_integerLiteral,
  AST_booleanLiteral,
  AST_stringLiteral,
  AST_tupleType,
  AST_tupleKeysetExpression,
  AST_specializedType,
  AST_specifiedIdentifier,
  AST_argument,
  AST_variableDeclaration,
  AST_parameter,
  AST_assignmentStatement,
  AST_emptyStatement,
  AST_defaultKeysetExpression,
  AST_dontcareKeysetExpression,
  AST_selectCase,
  AST_selectCaseList,
  AST_selectExpression,
  AST_selectKeyset,
  AST_keyElement,
  AST_actionRef,
  AST_tableDeclaration,
  AST_tableEntry,
  AST_tableKey,
  AST_tableActions,
  AST_tableEntries,
  AST_tableProperty,
  AST_tablePropertyList,
  AST_conditionalStatement,
  AST_exitStatement,
  AST_returnStatement,
  AST_switchStatement,
  AST_switchCase,
  AST_switchCases,
  AST_switchLabel,
  AST_statement,
  AST_blockStatement,
  AST_kvPairExpression,
  AST_exprListExpression,
  AST_castExpression,
  AST_unaryExpression,
  AST_binaryExpression,
  AST_memberSelectExpression,
  AST_arraySubscript,
  AST_functionCall,
  AST_p4program,
  AST_typeParameter,
  AST_typeParameterList,
  AST_parameterList,
  AST_methodPrototypes,
  AST_typeArgumentList,
  AST_structFieldList,
  AST_specifiedIdentifierList,
  AST_argumentList,
  AST_directApplication,
  AST_expression,
  AST_expressionList,
  AST_keysetExpr,
  AST_keysetExpressionList,
  AST_keyElementList,
  AST_actionList,
  AST_entriesList,
  AST_controlLocalDeclarations,
  AST_statementOrDeclList,
  AST_identifierList,
  AST_declarationList,
  AST_realTypeArgumentList,
  AST_typeRef,
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

enum AstWalkBranch {
  BRANCH_args = 1,
};

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

typedef struct Ast_List {
  Ast;
  DList members;
} Ast_List;

typedef struct Ast_Expression {
  Ast;
  Ast* type_args;
} Ast_Expression;

typedef struct Ast_Name {
  Ast_Expression;
  char* strname;
  Scope* scope;
} Ast_Name;

typedef struct Ast_BoolType {
  Ast;
  Ast* name;
} Ast_BoolType;

typedef struct Ast_ErrorType {
  Ast;
  Ast* name;
} Ast_ErrorType;

typedef struct Ast_IntType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_IntType;

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

typedef struct Ast_Const {
  Ast;
  Ast* name;
  Ast* type;
  Ast* expr;
} Ast_Const;

typedef struct Ast_Extern {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* method_protos;
} Ast_Extern;

typedef struct Ast_FunctionProto {
  Ast;
  Ast* name;
  Ast* return_type;
  Ast* type_params;
  Ast* params;
  bool is_extern;
} Ast_FunctionProto;

typedef struct Ast_Action {
  Ast;
  Ast* name;
  Ast* params;
  Ast* stmt;
} Ast_Action;

typedef struct Ast_Header {
  Ast;
  Ast* name;
  Ast* fields;
} Ast_Header;

typedef struct Ast_HeaderUnion {
  Ast;
  Ast* name;
  Ast* fields;
} Ast_HeaderUnion;

typedef struct Ast_Struct {
  Ast;
  Ast* name;
  Ast* fields;
} Ast_Struct;

typedef struct Ast_Enum {
  Ast;
  Ast* name;
  Ast* type_size;
  Ast* fields;
} Ast_Enum;

typedef struct Ast_TypeDef {
  Ast;
  Ast* name;
  Ast* type_ref;
} Ast_TypeDef;

typedef struct Ast_Parser {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_elements;
  Ast* states;
} Ast_Parser;

typedef struct Ast_Control {
  Ast;
  Ast* proto;
  Ast* ctor_params;
  Ast* local_decls;
  Ast* apply_stmt;
} Ast_Control;

typedef struct Ast_Package {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_Package;

typedef struct Ast_Instantiation {
  Ast;
  Ast* name;
  Ast* type;
  Ast* args;
} Ast_Instantiation;

typedef struct Ast_ErrorEnum {
  Ast;
  Ast* fields;
} Ast_ErrorEnum;

typedef struct Ast_MatchKind {
  Ast;
  Ast* fields;
} Ast_MatchKind;

typedef struct Ast_Function {
  Ast;
  Ast* proto;
  Ast* stmt;
} Ast_Function;

typedef struct Ast_IntTypeSize {
  Ast;
  Ast* size;
} Ast_IntTypeSize;

typedef struct Ast_IntLiteral {
  Ast_Expression;
  bool is_signed;
  int value;
  int width;
} Ast_IntLiteral;

typedef struct Ast_BoolLiteral {
  Ast_Expression;
  bool value;
} Ast_BoolLiteral;

typedef struct Ast_StringLiteral {
  Ast_Expression;
  char* value;
} Ast_StringLiteral;

typedef struct Ast_Tuple {
  Ast;
  Ast* type_args;
} Ast_Tuple;

typedef struct Ast_TupleKeyset {
  Ast;
  Ast* expr_list;
} Ast_TupleKeyset;

typedef struct Ast_HeaderStack {
  Ast;
  Ast* name;
  Ast* stack_expr;
} Ast_HeaderStack;

typedef struct Ast_SpecializedType {
  Ast;
  Ast* name;
  Ast* type_args;
} Ast_SpecializedType;

typedef struct Ast_SpecifiedIdent {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_SpecifiedIdent;

typedef struct Ast_StructField {
  Ast;
  Ast* name;
  Ast* type;
} Ast_StructField;

typedef struct Ast_ParserProto {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ParserProto;

typedef struct Ast_Argument {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_Argument;

typedef struct Ast_Var {
  Ast;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_Var;

typedef struct Ast_Param {
  Ast;
  enum AstParamDirection direction;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_Param;

typedef struct Ast_AssignmentStmt {
  Ast;
  Ast* lvalue;
  Ast* expr;
} Ast_AssignmentStmt;

typedef struct Ast_SelectCase {
  Ast;
  Ast* name;
  Ast* keyset;
} Ast_SelectCase;

typedef struct Ast_ParserState {
  Ast;
  Ast* name;
  Ast* stmt_list;
  Ast* trans_stmt;
} Ast_ParserState;

typedef struct Ast_ControlProto {
  Ast;
  Ast* name;
  Ast* type_params;
  Ast* params;
} Ast_ControlProto;

typedef struct Ast_KeyElement {
  Ast;
  Ast* name;
  Ast* expr;
} Ast_KeyElement;

typedef struct Ast_ActionRef {
  Ast;
  Ast* name;
  Ast* args;
} Ast_ActionRef;

typedef struct Ast_TableEntry {
  Ast;
  Ast* keyset;
  Ast* action;
} Ast_TableEntry;

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
  Ast* entries;
} Ast_TableEntries;

typedef struct Ast_TableProperty {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_TableProperty;

typedef struct Ast_Table {
  Ast;
  Ast* name;
  Ast* prop_list;
} Ast_Table;

typedef struct Ast_IfStmt {
  Ast;
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;
} Ast_IfStmt;

typedef struct Ast_ReturnStmt {
  Ast;
  Ast* expr;
} Ast_ReturnStmt;

typedef struct Ast_SwitchCase {
  Ast;
  Ast* label;
  Ast* stmt;
} Ast_SwitchCase;

typedef struct Ast_SwitchStmt {
  Ast;
  Ast* expr;
  Ast* switch_cases;
} Ast_SwitchStmt;

typedef struct Ast_BlockStmt {
  Ast;
  Ast* stmt_list;
} Ast_BlockStmt;

typedef struct Ast_DirectApplyStmt {
  Ast;
  Ast* lhs_expr;
  Ast* args;
} Ast_DirectApplyStmt;

typedef struct Ast_KVPairExpr {
  Ast_Expression;
  Ast* name;
  Ast* expr;
} Ast_KVPairExpr;

typedef struct Ast_P4Program {
  Ast;
  Ast* decl_list;
  int last_node_id;
} Ast_P4Program;

typedef struct Ast_SelectExpr {
  Ast_Expression;
  Ast* expr_list;
  Ast* case_list;
} Ast_SelectExpr;

typedef struct Ast_ExprListExpression {
  Ast_Expression;
  Ast* expr_list;
} Ast_ExprListExpression;

typedef struct Ast_CastExpr {
  Ast_Expression;
  Ast* to_type;
  Ast* expr;
} Ast_CastExpr;

typedef struct Ast_UnaryExpr {
  Ast_Expression;
  enum AstOperator op;
  Ast* operand;
} Ast_UnaryExpr;

typedef struct Ast_BinaryExpr {
  Ast_Expression;
  enum AstOperator op;
  Ast* left_operand;
  Ast* right_operand;
} Ast_BinaryExpr;

typedef struct Ast_MemberSelect {
  Ast_Expression;
  Ast* lhs_expr;
  Ast* member_name;
} Ast_MemberSelect;

typedef struct Ast_ArraySubscript {
  Ast_Expression;
  Ast* index;
  Ast* end_index;
} Ast_ArraySubscript;

typedef struct Ast_FunctionCall {
  Ast_Expression;
  Ast* callee_expr;
  Ast* args;
} Ast_FunctionCall;

typedef void (*AstVisitor)(Ast*, Ast*, enum AstWalkDirection);
void traverse_p4program(Ast_P4Program* p4program, AstVisitor visitor);

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
  DList members;
} TypeSet;

typedef struct Type_Type {
  Type;
} Type_Type;

typedef struct Type_Vector {
  Type;
  DList members;
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

