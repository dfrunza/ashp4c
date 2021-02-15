#pragma once
#include "basic.h"
#include "arena.h"
#include "lex.h"
#include "ast.h"

enum CstKind {
  Cst_NonTypeName,
  Cst_TypeName,
  Cst_PrefixedTypeName,
  Cst_BaseType,
  Cst_DotPrefixedName,
  Cst_ConstDecl,
  Cst_ExternDecl,
  Cst_Constructor,
  Cst_FunctionProto,
  Cst_ActionDecl,
  Cst_HeaderDecl,
  Cst_HeaderUnionDecl,
  Cst_StructDecl,
  Cst_EnumDecl,
  Cst_TypeDecl,
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
  Cst_Bool,
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
  Cst_SelectExpr,
  Cst_SelectCase,
  Cst_ParserState,
  Cst_ControlType,
  Cst_KeyElement,
  Cst_ActionRef,
  Cst_TableEntry,
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

struct CstLink {
  struct Cst* prev_node;
  struct Cst* next_node;
};

struct Cst {
  enum CstKind kind;
  int id;
  int line_nr;
  struct CstLink link;
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

struct Cst_BaseType {
  struct Cst;
  enum AstBaseTypeKind base_type;
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

struct Cst_ActionDecl {
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

struct Cst_TypeDecl {
  struct Cst;
  bool is_typedef;
  struct Cst* type_ref;
  struct Cst* name;
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
  enum AstParamDirKind dir_kind;
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
  enum AstIntegerFlags flags;
  int width;
  int value;
};

struct Cst_Bool {
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
  struct Cst* name;
  struct Cst* stack_expr;
};

struct Cst_SpecdType {
  struct Cst;
  struct Cst* name;
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
  struct Cst* expr;
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

struct Cst_SelectExpr {
  struct Cst;
  struct Cst* expr_list;
  struct Cst* case_list;
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
  enum AstExprOperator op;
  struct Cst* expr;
};

struct Cst_BinaryExpr {
  struct Cst;
  enum AstExprOperator op;
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

struct Cst* build_cst();
