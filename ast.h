#pragma once
#include "basic.h"
#include "arena.h"


enum AstKind {
  AST_NONE,
  AST_NAME,
  AST_BASETYPE_BOOL,
  AST_BASETYPE_ERROR,
  AST_BASETYPE_INT,
  AST_BASETYPE_BIT,
  AST_BASETYPE_VARBIT,
  AST_BASETYPE_STRING,
  AST_BASETYPE_VOID,
  AST_CONST_DECL,
  AST_EXTERN_DECL,
  AST_FUNCTION_PROTO,
  AST_ACTION_DECL,
  AST_HEADER_DECL,
  AST_HEADER_UNION_DECL,
  AST_STRUCT_DECL,
  AST_ENUM_DECL,
  AST_TYPE_DECL,
  AST_PARSER_DECL,
  AST_CONTROL_DECL,
  AST_PACKAGE_DECL,
  AST_INSTANTIATION,
  AST_ERROR_DECL,
  AST_MATCH_KIND_DECL,
  AST_FUNCTION_DECL,
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
  AST_VAR_DECL,
  AST_DIRECT_APPLICATION,
  AST_PARAM,
  AST_LVALUE,
  AST_ASSIGNMENT_STMT,
  AST_METHODCALL_STMT,
  AST_EMPTY_STMT,
  AST_DEFAULT_STMT,
  AST_SELECT_CASE,
  AST_PARSER_STATE,
  AST_CONTROL_PROTO,
  AST_KEY_ELEMENT,
  AST_ACTION_REF,
  AST_TABLE_DECL,
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
  AST_KEYVALUE_PAIR_EXPR,
  AST_SELECT_EXPR,
  AST_EXPRLIST_EXPR,
  AST_CAST_EXPR,
  AST_UNARY_EXPR,
  AST_BINARY_EXPR,
  AST_MEMBERSELECT_EXPR,
  AST_INDEXEDARRAY_EXPR,
  AST_FUNCTIONCALL_EXPR,
};

enum AstIntegerFlags {
  INTFLAGS_NONE,
  INTFLAGS_HAS_WIDTH,
  INTFLAGS_IS_SIGNED,
};

enum AstExprOperator {
  OP_NONE,
  OP_ADD,
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
  OP_MINUS,
};

enum AstParamDirection {
  PARAMDIR_NONE,
  PARAMDIR_IN,
  PARAMDIR_OUT,
  PARAMDIR_INOUT,
};

struct Ast {
  enum AstKind kind;
  struct Ast* parent;
  uint32_t id;
  int line_nr;
};

struct Ast_Expression {
  struct Ast;
  struct List* type_args;
};

struct Ast_Name {
  struct Ast_Expression;
  char* strname;
  bool is_dotprefixed;
};

struct Ast_BaseType {
  struct Ast;
  struct Ast* name;
};

struct Ast_BaseType_Bool {
  struct Ast_BaseType;
};

struct Ast_BaseType_Error {
  struct Ast_BaseType;
};

struct Ast_BaseType_Int {
  struct Ast_BaseType;
  struct Ast* size;
};

struct Ast_BaseType_Bit {
  struct Ast_BaseType;
  struct Ast* size;
};

struct Ast_BaseType_Varbit {
  struct Ast_BaseType;
  struct Ast* size;
};

struct Ast_BaseType_String {
  struct Ast_BaseType;
};

struct Ast_BaseType_Void {
  struct Ast_BaseType;
};

struct Ast_ConstDecl {
  struct Ast;
  struct Ast* name;
  struct Ast* type_ref;
  struct Ast* expr;
};

struct Ast_ExternDecl {
  struct Ast;
  struct Ast* name;
  struct List* type_params;
  struct List* method_protos;
};

struct Ast_FunctionProto {
  struct Ast;
  bool is_extern;
  struct Ast* return_type;
  struct Ast* name;
  struct List* type_params;
  struct List* params;
};

struct Ast_ActionDecl {
  struct Ast;
  struct Ast* name;
  struct List* params;
  struct Ast* stmt;
};

struct Ast_HeaderDecl {
  struct Ast;
  struct Ast* name;
  struct List* fields;
};

struct Ast_HeaderUnionDecl {
  struct Ast;
  struct Ast* name;
  struct List* fields;
};

struct Ast_StructDecl {
  struct Ast;
  struct Ast* name;
  struct List* fields;
};

struct Ast_EnumDecl {
  struct Ast;
  struct Ast* name;
  struct Ast* type_size;
  struct List* id_list;
};

struct Ast_TypeDecl {
  struct Ast;
  struct Ast* name;
  bool is_typedef;
  struct Ast* type_ref;
};

struct Ast_ParserDecl {
  struct Ast;
  struct Ast* type_decl;
  struct List* ctor_params;
  struct List* local_elements;
  struct List* states;
};

struct Ast_ControlDecl {
  struct Ast;
  struct Ast* type_decl;
  struct List* ctor_params;
  struct List* local_decls;
  struct Ast* apply_stmt;
};

struct Ast_PackageDecl {
  struct Ast;
  struct Ast* name;
  struct List* type_params;
  struct List* params;
};

struct Ast_Instantiation {
  struct Ast;
  struct Ast* name;
  struct Ast* type_ref;
  struct List* args;
};

struct Ast_ErrorDecl {
  struct Ast;
  struct List* id_list;
};

struct Ast_MatchKindDecl {
  struct Ast;
  struct List* id_list;
};

struct Ast_FunctionDecl {
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
  enum AstIntegerFlags flags;
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
  struct List* type_args;
};

struct Ast_TupleKeyset {
  struct Ast;
  struct List* expr_list;
};

struct Ast_HeaderStack {
  struct Ast;
  struct Ast* name;
  struct Ast* stack_expr;
};

struct Ast_SpecializedType {
  struct Ast;
  struct Ast* name;
  struct List* type_args;
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
  struct List* type_params;
  struct List* params;
};

struct Ast_Argument {
  struct Ast;
  struct Ast* name;
  struct Ast* init_expr;
};

struct Ast_VarDecl {
  struct Ast;
  struct Ast* name;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_DirectApplication {
  struct Ast;
  struct Ast* name;
  struct List* args;
};

struct Ast_Param {
  struct Ast;
  enum AstParamDirection direction;
  struct Ast* name;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_Lvalue {
  struct Ast;
  struct Ast* name;
  struct List* expr;
};

struct Ast_AssignmentStmt {
  struct Ast;
  struct Ast* lvalue;
  struct Ast* expr;
};

struct Ast_MethodCallStmt {
  struct Ast;
  struct Ast* lvalue;
  struct List* args;
  struct List* type_args;
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
  struct List* stmt_list;
  struct Ast* trans_stmt;
};

struct Ast_ControlProto {
  struct Ast;
  struct Ast* name;
  struct List* type_params;
  struct List* params;
};

struct Ast_KeyElement {
  struct Ast;
  struct Ast* name;
  struct Ast* expr;
};

struct Ast_ActionRef {
  struct Ast;
  struct Ast* name;
  struct List* args;
};

struct Ast_TableEntry {
  struct Ast;
  struct Ast* keyset;
  struct Ast* action;
};

struct Ast_TableKey {
  struct Ast;
  struct List* keyelem_list;
};

struct Ast_TableActions {
  struct Ast;
  struct List* action_list;
};

struct Ast_TableEntries {
  struct Ast;
  bool is_const;
  struct List* entries;
};

struct Ast_TableSingleEntry {
  struct Ast;
  struct Ast* name;
  struct Ast* init_expr;
};

struct Ast_TableDecl {
  struct Ast;
  struct Ast* name;
  struct List* prop_list;
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
  struct List* switch_cases;
};

struct Ast_BlockStmt {
  struct Ast;
  struct List* stmt_list;
};

struct Ast_KeyValuePairExpr {
  struct Ast_Expression;
  struct Ast* name;
  struct Ast* expr;
};

struct Ast_P4Program {
  struct Ast;
  struct List* decl_list;
};

struct Ast_SelectExpr {
  struct Ast_Expression;
  struct List* expr_list;
  struct List* case_list;
};

struct Ast_ExprListExpr {
  struct Ast_Expression;
  struct List* expr_list;
};

struct Ast_CastExpr {
  struct Ast_Expression;
  struct Ast* to_type;
  struct Ast* expr;
};

struct Ast_UnaryExpr {
  struct Ast_Expression;
  enum AstExprOperator op;
  struct Ast* operand;
};

struct Ast_BinaryExpr {
  struct Ast_Expression;
  enum AstExprOperator op;
  struct Ast* left_operand;
  struct Ast* right_operand;
};

struct Ast_MemberSelectExpr {
  struct Ast_Expression;
  struct Ast* expr;
  struct Ast* member_name;
};

struct Ast_IndexedArrayExpr {
  struct Ast_Expression;
  struct Ast* index;
  struct Ast* colon_index;
};

struct Ast_FunctionCallExpr {
  struct Ast_Expression;
  struct Ast* callee_expr;
  struct List* args;
};

void print_ast(struct Ast* ast);
