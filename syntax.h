#pragma once
#include "basic.h"
#include "arena.h"
#include "lex.h"

struct Cst;

enum IdentKind
{
  Ident_None,
  Ident_Keyword,
  Ident_Type,
  Ident_Ident,
};

struct Ident {
  enum IdentKind ident_kind;
  char* name;
  struct Cst* ast;
  int scope_level;
  struct Ident* next_in_scope;
};  

struct Ident_Keyword {
  struct Ident;
  enum TokenClass token_klass;
};

struct Namespace_Entry {
  char* name;
  struct Ident* ns_global;
  struct Ident* ns_type;
  struct Namespace_Entry* next;
};

#define cast(TYPE, EXPR) ({\
  if ((EXPR)) assert((EXPR)->kind == TYPE); \
  (struct TYPE*)(EXPR);})

enum CstKind {
  Cst_NonTypeName,
  Cst_TypeName,
  Cst_PrefixedTypeName,
  Cst_BaseType,
  Cst_DotPrefixedName,
  Cst_ConstDecl,
  Cst_ExternDecl,
  Cst_ExternFuncDecl,
  Cst_TypeParam,
  Cst_Constructor,
  Cst_FunctionProto,
  Cst_Action,
  Cst_HeaderDecl,
  Cst_HeaderUnionDecl,
  Cst_StructDecl,
  Cst_EnumDecl,
  Cst_Parser,
  Cst_Control,
  Cst_Package,
  Cst_Instantiation,
  Cst_Error,
  Cst_MatchKind,
  Cst_FunctionDecl,
  Cst_Dontcare,
  Cst_IntTypeSize,
  Cst_Int,
  Cst_StringLiteral,
  Cst_Tuple,
  Cst_HeaderStack,
  Cst_SpecdType,
  Cst_StructField,
  Cst_SpecdId,
  Cst_ParserType,
  Cst_Argument,
  Cst_VarDecl,
  Cst_DirectApplic,
  Cst_ArrayIndex,
  Cst_ParamDir,
  Cst_Parameter,
  Cst_Lvalue,
  Cst_AssignmentStmt,
  Cst_MethodCallStmt,
  Cst_EmptyStmt,
  Cst_Default,
  Cst_SelectCase,
  Cst_ParserState,
  Cst_ControlType,
  Cst_KeyElement,
  Cst_ActionRef,
  Cst_TableEntry,
  Cst_TableProperty,
  Cst_TableProp_Key,
  Cst_TableProp_Actions,
  Cst_TableProp_Entries,
  Cst_TableProp_SingleEntry,
  Cst_TableDecl,
  Cst_IfStmt,
  Cst_ExitStmt,
  Cst_ReturnStmt,
  Cst_SwitchLabel,
  Cst_SwitchCase,
  Cst_SwitchStmt,
  Cst_BlockStmt,
  Cst_ExpressionListExpr,
  Cst_CastExpr,
  Cst_UnaryExpr,
  Cst_BinaryExpr,
  Cst_MemberSelectExpr,
  Cst_IndexedArrayExpr,
  Cst_FunctionCallExpr,
  Cst_TypeArgsExpr,
  Cst_P4Program,
};

enum Cst_ParamDirKind {
  Cst_DirNone,
  Cst_DirIn,
  Cst_DirOut,
  Cst_DirInOut,
};

enum Cst_ExprOperator {
  Cst_Op_None,
  Cst_BinaryOp_ArithAdd,
  Cst_BinaryOp_ArithSub,
  Cst_BinaryOp_ArithMul,
  Cst_BinaryOp_ArithDiv,
  Cst_BinaryOp_LogicAnd,
  Cst_BinaryOp_LogicOr,
  Cst_BinaryOp_LogicEqual,
  Cst_BinaryOp_LogicNotEqual,
  Cst_BinaryOp_LogicLess,
  Cst_BinaryOp_LogicGreater,
  Cst_BinaryOp_LogicLessEqual,
  Cst_BinaryOp_LogicGreaterEqual,
  Cst_BinaryOp_BitwiseAnd,
  Cst_BinaryOp_BitwiseOr,
  Cst_BinaryOp_BitwiseXor,
  Cst_BinaryOp_BitshiftLeft,
  Cst_BinaryOp_BitshiftRight,
  Cst_UnaryOp_LogicNot,
  Cst_UnaryOp_BitwiseNot,
  Cst_UnaryOp_ArithMinus,
};

enum Cst_TypeParameterKind {
  Cst_TypeParamNone,
  Cst_TypeParamVar,
  Cst_TypeParamInt,
};

struct Cst {
  enum CstKind kind;
  int line_nr;
  struct List link;
};

struct Cst_NonTypeName {
  struct Cst;
  char* name;
};

struct Cst_TypeName {
  struct Cst;
  char* name;
};

struct Cst_DotPrefixedName {
  struct Cst;
  struct Cst* name;
};

struct Cst_PrefixedTypeName {
  struct Cst;
  struct Cst_TypeName* first_name;
  struct Cst_TypeName* second_name;
};

enum Cst_BaseTypeKind {
  Cst_BaseType_None,
  Cst_BaseType_Bool,
  Cst_BaseType_Error,
  Cst_BaseType_Int,
  Cst_BaseType_Bit,
  Cst_BaseType_Varbit,
};

struct Cst_BaseType {
  struct Cst;
  enum Cst_BaseTypeKind base_type;
  struct Cst* size;
};

struct Cst_ConstDecl {
  struct Cst;
  struct Cst* type;
  struct Cst* name;
  struct Cst* expr;
};

struct Cst_ExternDecl {
  struct Cst;
  struct Cst* name;
  struct Cst* type_params;
  struct Cst* method_protos;
};

struct Cst_TypeParam {
  struct Cst;
  struct Cst* name;
};

struct Cst_Constructor {
  struct Cst;
  struct Cst* params;
};

struct Cst_FunctionProto {
  struct Cst;
  struct Cst* return_type;
  struct Cst* name;
  struct Cst* type_params;
  struct Cst* params;
};

struct Cst_Action {
  struct Cst;
  struct Cst* name;
  struct Cst* params;
  struct Cst* stmt;
};

struct Cst_HeaderDecl {
  struct Cst;
  struct Cst* name;
  struct Cst* fields;
};

struct Cst_HeaderUnionDecl {
  struct Cst;
  struct Cst* name;
  struct Cst* fields;
};

struct Cst_StructDecl {
  struct Cst;
  struct Cst* name;
  struct Cst* fields;
};

struct Cst_EnumDecl {
  struct Cst;
  struct Cst* type_size;
  struct Cst* name;
  struct Cst* id_list;
};

struct Cst_Parser {
  struct Cst;
  struct Cst* type_decl;
  struct Cst* ctor_params;
  struct Cst* local_elements;
  struct Cst* states;
};

struct Cst_Control {
  struct Cst;
  struct Cst* type_decl;
  struct Cst* ctor_params;
  struct Cst* local_decls;
  struct Cst* apply_stmt;
};

struct Cst_Package {
  struct Cst;
  struct Cst* name;
  struct Cst* type_params;
  struct Cst* params;
};

struct Cst_Instantiation {
  struct Cst;
  struct Cst* type;
  struct Cst* args;
  struct Cst* name;
};

struct Cst_Error {
  struct Cst;
  struct Cst* id_list;
};

struct Cst_MatchKind {
  struct Cst;
  struct Cst* fields;
};

struct Cst_FunctionDecl {
  struct Cst;
  struct Cst* proto;
  struct Cst* stmt;
};

struct Cst_ParamDir {
  struct Cst;
  enum Cst_ParamDirKind dir;
};

struct Cst_Parameter {
  struct Cst;
  struct Cst* direction;
  struct Cst* type;
  struct Cst* name;
  struct Cst* init_expr;
};

struct Cst_Dontcare {
  struct Cst;
};

struct Cst_IntTypeSize {
  struct Cst;
  struct Cst* size;
};

struct Cst_Int {
  struct Cst;
  int value;
};

struct Cst_StringLiteral {
  struct Cst;
  char* value;
};

struct Cst_Tuple {
  struct Cst;
  struct Cst* type_args;
};

struct Cst_HeaderStack {
  struct Cst;
  struct Cst* expr;
};

struct Cst_SpecdType {
  struct Cst;
  struct Cst* type_args;
};

struct Cst_StructField {
  struct Cst;
  struct Cst* type;
  struct Cst* name;
};

struct Cst_SpecdId {
  struct Cst;
  struct Cst* name;
  struct Cst* init_expr;
};

struct Cst_ParserType {
  struct Cst;
  struct Cst* name;
  struct Cst* type_params;
  struct Cst* params;
};

struct Cst_Argument {
  struct Cst;
  struct Cst* name;
  struct Cst* init_expr;
};

struct Cst_VarDecl {
  struct Cst;
  struct Cst* type;
  struct Cst* name;
  struct Cst* init_expr;
};

struct Cst_DirectApplic {
  struct Cst;
  struct Cst* name;
  struct Cst* args;
};

struct Cst_Lvalue {
  struct Cst;
  struct Cst* name;
  struct Cst* array_index;
};

struct Cst_ArrayIndex {
  struct Cst;
  struct Cst* index;
  struct Cst* colon_index;
};

struct Cst_AssignmentStmt {
  struct Cst;
  struct Cst* lvalue;
  struct Cst* expr;
};

struct Cst_MethodCallStmt {
  struct Cst;
  struct Cst* lvalue;
  struct Cst* type_args;
  struct Cst* args;
};

struct Cst_EmptyStmt {
  struct Cst;
};

struct Cst_Default {
  struct Cst;
};

struct Cst_SelectCase {
  struct Cst;
  struct Cst* keyset;
  struct Cst* name;
};

struct Cst_ParserState {
  struct Cst;
  struct Cst* name;
  struct Cst* stmts;
  struct Cst* trans_stmt;
};

struct Cst_ControlType {
  struct Cst;
  struct Cst* name;
  struct Cst* type_params;
  struct Cst* params;
};

struct Cst_KeyElement {
  struct Cst;
  struct Cst* expr;
  struct Cst* name;
};

struct Cst_ActionRef {
  struct Cst;
  struct Cst* name;
  struct Cst* args;
};

struct Cst_TableEntry {
  struct Cst;
  struct Cst* keyset;
  struct Cst* action;
};

struct Cst_TableProp_Key {
  struct Cst;
  struct Cst* keyelem_list;
};

struct Cst_TableProp_Actions {
  struct Cst;
  struct Cst* action_list;
};

struct Cst_TableProp_Entries {
  struct Cst;
  bool is_const;
  struct Cst* entries;
};

struct Cst_TableProp_SingleEntry {
  struct Cst;
  struct Cst* name;
  struct Cst* init_expr;
};

struct Cst_TableDecl {
  struct Cst;
  struct Cst* name;
  struct Cst* prop_list;
};

struct Cst_IfStmt {
  struct Cst;
  struct Cst* cond_expr;
  struct Cst* stmt;
  struct Cst* else_stmt;
};

struct Cst_ExitStmt {
  struct Cst;
};

struct Cst_ReturnStmt {
  struct Cst;
  struct Cst* expr;
};

struct Cst_SwitchLabel {
  struct Cst;
  struct Cst* name;
};

struct Cst_SwitchCase {
  struct Cst;
  struct Cst* label;
  struct Cst* stmt;
};

struct Cst_SwitchStmt {
  struct Cst;
  struct Cst* expr;
  struct Cst* switch_cases;
};

struct Cst_BlockStmt {
  struct Cst;
  struct Cst* stmt_list;
};

struct Cst_ExpressionListExpr {
  struct Cst;
  struct Cst* expr_list;
};

struct Cst_CastExpr {
  struct Cst;
  struct Cst* to_type;
  struct Cst* expr;
};

struct Cst_UnaryExpr {
  struct Cst;
  enum Cst_ExprOperator op;
  struct Cst* expr;
};

struct Cst_BinaryExpr {
  struct Cst;
  enum Cst_ExprOperator op;
  struct Cst* left_operand;
  struct Cst* right_operand;
};

struct Cst_MemberSelectExpr {
  struct Cst;
  struct Cst* expr;
  struct Cst* member_name;
};

struct Cst_IndexedArrayExpr {
  struct Cst;
  struct Cst* expr;
  struct Cst* index_expr;
};

struct Cst_FunctionCallExpr {
  struct Cst;
  struct Cst* expr;
  struct Cst* args;
};

struct Cst_TypeArgsExpr {
  struct Cst;
  struct Cst* expr;
  struct Cst* type_args;
};

struct Cst_P4Program {
  struct Cst;
  struct Cst* decl_list;
};

