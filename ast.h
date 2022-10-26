#pragma once

enum TokenClass {
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

  /* Special */
  TK_UNKNOWN,
  TK_START_OF_INPUT,
  TK_END_OF_INPUT,
  TK_LEXICAL_ERROR,
};

struct Token {
  enum TokenClass klass;
  char* lexeme;
  int line_no;

  union {
    struct {
      bool is_signed;
      int width;
      int64_t value;
    } i;  /* integer */
    char* str;
  };
};

enum AstEnum {
  AST_NAME = 1,
  AST_DOTNAME = AST_NAME,
  AST_BOOL_TYPE,
  AST_ERROR_TYPE,
  AST_INT_TYPE,
  AST_BIT_TYPE,
  AST_VARBIT_TYPE,
  AST_STRING_TYPE,
  AST_VOID_TYPE,
  AST_CONST,
  AST_EXTERN,
  AST_FUNCTION_PROTO,
  AST_ACTION,
  AST_HEADER,
  AST_HEADER_UNION,
  AST_STRUCT,
  AST_ENUM,
  AST_ENUM_FIELD,
  AST_TYPE,
  AST_TYPE_PARAM,
  AST_PARSER,
  AST_CONTROL,
  AST_PACKAGE,
  AST_INSTANTIATION,
  AST_ERROR,
  AST_MATCH_KIND,
  AST_FUNCTION,
  AST_DONTCARE,
  AST_INT_TYPESIZE,
  AST_INT_LITERAL,
  AST_BOOL_LITERAL,
  AST_STRING_LITERAL,
  AST_TUPLE,
  AST_TUPLE_KEYSET,
  AST_HEADER_STACK,
  AST_SPECIALIZED_TYPE,
  AST_SPECIFIED_IDENT,
  AST_STRUCT_FIELD,
  AST_PARSER_PROTO,
  AST_ARGUMENT,
  AST_VAR,
  AST_PARAM,
  AST_ASSIGNMENT_STMT,
  AST_EMPTY_STMT,
  AST_DEFAULT_STMT,
  AST_SELECT_CASE,
  AST_PARSER_STATE,
  AST_CONTROL_PROTO,
  AST_KEY_ELEMENT,
  AST_ACTION_REF,
  AST_TABLE,
  AST_TABLE_ENTRY,
  AST_TABLE_KEY,
  AST_TABLE_ACTIONS,
  AST_TABLE_ENTRIES,
  AST_TABLE_SINGLE_ENTRY,
  AST_IF_STMT,
  AST_EXIT_STMT,
  AST_RETURN_STMT,
  AST_SWITCH_STMT,
  AST_SWITCH_CASE,
  AST_BLOCK_STMT,
  AST_P4PROGRAM,
  AST_KVPAIR,
  AST_SELECT_EXPR,
  AST_EXPRLIST,
  AST_CAST_EXPR,
  AST_UNARY_EXPR,
  AST_BINARY_EXPR,
  AST_MEMBER_SELECT,
  AST_SUBSCRIPT,
  AST_FUNCTION_CALL,
};

enum AstOperator {
  OP_ADD = 1,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_AND,
  OP_OR,
  OP_NOT,
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_LESS,
  OP_GREATER,
  OP_LESS_EQUAL,
  OP_GREATER_EQUAL,
  OP_BITWISE_AND,
  OP_BITWISE_OR,
  OP_BITWISE_XOR,
  OP_BITWISE_NOT,
  OP_BITWISE_SHIFT_LEFT,
  OP_BITWISE_SHIFT_RIGHT,
  OP_MASK,
  OP_NEG,
};

enum AstParamDirection {
  PARAMDIR_IN = 1,
  PARAMDIR_OUT,
  PARAMDIR_INOUT,
};

struct Ast {
  enum AstEnum kind;
  uint32_t id;
  int line_no;
};

struct Ast_Expression {
  struct Ast;
  struct DList* type_args;
};

struct Ast_Name {
  struct Ast_Expression;
  char* strname;
};

struct Ast_BasicType {
  struct Ast;
  struct Ast* name;
};

struct Ast_BoolType {
  struct Ast_BasicType;
};

struct Ast_ErrorType {
  struct Ast_BasicType;
};

struct Ast_IntType {
  struct Ast_BasicType;
  struct Ast* size;
};

struct Ast_BitType {
  struct Ast_BasicType;
  struct Ast* size;
};

struct Ast_VarbitType {
  struct Ast_BasicType;
  struct Ast* size;
};

struct Ast_StringType {
  struct Ast_BasicType;
};

struct Ast_VoidType {
  struct Ast_BasicType;
};

struct Ast_Const {
  struct Ast;
  struct Ast* name;
  struct Ast* type_ref;
  struct Ast* expr;
};

struct Ast_Extern {
  struct Ast;
  struct Ast* name;
  struct DList* type_params;
  struct DList* method_protos;
};

struct Ast_FunctionProto {
  struct Ast;
  bool is_extern;
  struct Ast* return_type;
  struct Ast* name;
  struct DList* type_params;
  struct DList* params;
};

struct Ast_Action {
  struct Ast;
  struct Ast* name;
  struct DList* params;
  struct Ast* stmt;
};

struct Ast_Header {
  struct Ast;
  struct Ast* name;
  struct DList* fields;
};

struct Ast_HeaderUnion {
  struct Ast;
  struct Ast* name;
  struct DList* fields;
};

struct Ast_Struct {
  struct Ast;
  struct Ast* name;
  struct DList* fields;
};

struct Ast_Enum {
  struct Ast;
  struct Ast* name;
  struct Ast* type_size;
  struct DList* id_list;
};

struct Ast_Type {
  struct Ast;
  struct Ast* name;
  bool is_typedef;
  struct Ast* type_ref;
};

struct Ast_Parser {
  struct Ast;
  struct Ast* type_decl;
  struct DList* ctor_params;
  struct DList* local_elements;
  struct DList* states;
};

struct Ast_Control {
  struct Ast;
  struct Ast* type_decl;
  struct DList* ctor_params;
  struct DList* local_decls;
  struct Ast* apply_stmt;
};

struct Ast_Package {
  struct Ast;
  struct Ast* name;
  struct DList* type_params;
  struct DList* params;
};

struct Ast_Instantiation {
  struct Ast;
  struct Ast* name;
  struct Ast* type_ref;
  struct DList* args;
};

struct Ast_Error {
  struct Ast;
  struct DList* id_list;
};

struct Ast_MatchKind {
  struct Ast;
  struct DList* id_list;
};

struct Ast_Function {
  struct Ast;
  struct Ast* proto;
  struct Ast* stmt;
};

struct Ast_Dontcare {
  struct Ast;
};

struct Ast_IntTypeSize {
  struct Ast;
  struct Ast* size;
};

struct Ast_IntLiteral {
  struct Ast_Expression;
  bool is_signed;
  int value;
  int width;
};

struct Ast_BoolLiteral {
  struct Ast_Expression;
  bool value;
};

struct Ast_StringLiteral {
  struct Ast_Expression;
  char* value;
};

struct Ast_Tuple {
  struct Ast;
  struct DList* type_args;
};

struct Ast_TupleKeyset {
  struct Ast;
  struct DList* expr_list;
};

struct Ast_HeaderStack {
  struct Ast;
  struct Ast* name;
  struct Ast* stack_expr;
};

struct Ast_SpecializedType {
  struct Ast;
  struct Ast* name;
  struct DList* type_args;
};

struct Ast_SpecifiedIdent {
  struct Ast;
  struct Ast* name;
  struct Ast* init_expr;
};

struct Ast_StructField {
  struct Ast;
  struct Ast* name;
  struct Ast* type;
};

struct Ast_ParserProto {
  struct Ast;
  struct Ast* name;
  struct DList* type_params;
  struct DList* params;
};

struct Ast_Argument {
  struct Ast;
  struct Ast* name;
  struct Ast* init_expr;
};

struct Ast_Var {
  struct Ast;
  struct Ast* name;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_Param {
  struct Ast;
  enum AstParamDirection direction;
  struct Ast* name;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_AssignmentStmt {
  struct Ast;
  struct Ast* lvalue;
  struct Ast* expr;
};

struct Ast_EmptyStmt {
  struct Ast;
};

struct Ast_DefaultStmt {
  struct Ast;
};

struct Ast_SelectCase {
  struct Ast;
  struct Ast* name;
  struct Ast* keyset;
};

struct Ast_ParserState {
  struct Ast;
  struct Ast* name;
  struct DList* stmt_list;
  struct Ast* trans_stmt;
};

struct Ast_ControlProto {
  struct Ast;
  struct Ast* name;
  struct DList* type_params;
  struct DList* params;
};

struct Ast_KeyElement {
  struct Ast;
  struct Ast* name;
  struct Ast* expr;
};

struct Ast_ActionRef {
  struct Ast;
  struct Ast* name;
  struct DList* args;
};

struct Ast_TableEntry {
  struct Ast;
  struct Ast* keyset;
  struct Ast* action;
};

struct Ast_TableKey {
  struct Ast;
  struct DList* keyelem_list;
};

struct Ast_TableActions {
  struct Ast;
  struct DList* action_list;
};

struct Ast_TableEntries {
  struct Ast;
  bool is_const;
  struct DList* entries;
};

struct Ast_TableSingleEntry {
  struct Ast;
  struct Ast* name;
  struct Ast* init_expr;
};

struct Ast_Table {
  struct Ast;
  struct Ast* name;
  struct DList* prop_list;
};

struct Ast_IfStmt {
  struct Ast;
  struct Ast* cond_expr;
  struct Ast* stmt;
  struct Ast* else_stmt;
};

struct Ast_ExitStmt {
  struct Ast;
};

struct Ast_ReturnStmt {
  struct Ast;
  struct Ast* expr;
};

struct Ast_SwitchCase {
  struct Ast;
  struct Ast* label;
  struct Ast* stmt;
};

struct Ast_SwitchStmt {
  struct Ast;
  struct Ast* expr;
  struct DList* switch_cases;
};

struct Ast_BlockStmt {
  struct Ast;
  struct DList* stmt_list;
};

struct Ast_KVPair {
  struct Ast_Expression;
  struct Ast* name;
  struct Ast* expr;
};

struct Ast_P4Program {
  struct Ast;
  struct DList* decl_list;
  int last_node_id;
};

struct Ast_SelectExpr {
  struct Ast_Expression;
  struct DList* expr_list;
  struct DList* case_list;
};

struct Ast_ExprList {
  struct Ast_Expression;
  struct DList* expr_list;
};

struct Ast_CastExpr {
  struct Ast_Expression;
  struct Ast* to_type;
  struct Ast* expr;
};

struct Ast_UnaryExpr {
  struct Ast_Expression;
  enum AstOperator op;
  struct Ast* operand;
};

struct Ast_BinaryExpr {
  struct Ast_Expression;
  enum AstOperator op;
  struct Ast* left_operand;
  struct Ast* right_operand;
};

struct Ast_MemberSelect {
  struct Ast_Expression;
  struct Ast* lhs_expr;
  struct Ast* member_name;
};

struct Ast_Subscript {
  struct Ast_Expression;
  struct Ast* index;
  struct Ast* colon_index;
};

struct Ast_FunctionCall {
  struct Ast_Expression;
  struct Ast* callee_expr;
  struct DList* args;
};

enum Namespace {
  NAMESPACE_TYPE = 1,
  NAMESPACE_VAR,
  NAMESPACE_KEYWORD,
};

struct NameDecl {
  union {
    struct Ast* ast;
    enum TokenClass token_class;
  };
  char* strname;
  int line_no;
  struct Scope* scope;
  struct NameDecl* nextdecl_in_scope;
};  

struct NameRef {
  struct Ast* ast;
  char* strname;
  int line_no;
  struct Scope* scope;
};

struct NameEntry {
  char* strname;
  struct NameDecl* ns_type;
  struct NameDecl* ns_var;
  struct NameDecl* ns_keyword;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  struct Hashmap decls;
};

void scope_init(struct Arena* scope_storage);
struct NameEntry* namedecl_get_or_create(struct Hashmap* decls, char* name);
struct NameEntry* namedecl_get(struct Hashmap* decls, char* name);
struct Scope* push_scope();
struct Scope* pop_scope();
struct NameEntry* scope_lookup_name(struct Scope* scope, char* name);
struct NameEntry* declare_name_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl);
struct NameRef* nameref_get(struct Hashmap* map, uint32_t id);
void nameref_add(struct Hashmap* map, struct NameRef* nameref, uint32_t id);

enum TypeEnum {
  TYPE_NAME = 1,
  TYPE_BASIC,
  TYPE_TYPEVAR,
  TYPE_TYPEDEF,
  TYPE_TYPENAME,
  TYPE_TYPEPARAM,
  TYPE_PRODUCT,
  TYPE_FUNCTION,
  TYPE_FUNCTION_CALL,
};

enum BasicType {
  TYPE_VOID = 1,
  TYPE_INT,
  TYPE_STRING,
};

struct Type {
  enum TypeEnum ctor;
  struct Type* type_params;
};

struct Type_Basic {
  struct Type;
  char* strname;
  enum BasicType basic_ty;
};

struct Type_TypeVar {
  struct Type;
};

struct Type_Name {
  struct Type;
  char* strname;
};

struct Type_TypeParam {
  struct Type;
  char* strname;
};

struct Type_Product {
  struct Type;
  struct Type* lhs_ty;
  struct Type* rhs_ty;
};

struct Type_Function {
  struct Type;
  struct Type* params_ty;
  struct Type* return_ty;
};

struct Type_FunctionCall {
  struct Type;
  struct Type* args_ty;
  struct Type* return_ty;
};

struct SList* type_get(struct Hashmap* map, uint32_t id);
struct SList* type_add(struct Hashmap* map, struct Type* type, uint32_t id);

