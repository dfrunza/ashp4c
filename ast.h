#pragma once
#include "basic.h"
#include "arena.h"


enum AstKind {
  Ast_NONE,
  Ast_Name,
  Ast_BaseType_Bool,
  Ast_BaseType_Error,
  Ast_BaseType_Int,
  Ast_BaseType_Bit,
  Ast_BaseType_Varbit,
  Ast_BaseType_String,
  Ast_BaseType_Void,
  Ast_ConstDecl,
  Ast_ExternDecl,
  Ast_FunctionProto,
  Ast_ActionDecl,
  Ast_HeaderDecl,
  Ast_HeaderUnionDecl,
  Ast_StructDecl,
  Ast_EnumDecl,
  Ast_TypeDecl,
  Ast_ParserDecl,
  Ast_ControlDecl,
  Ast_PackageDecl,
  Ast_Instantiation,
  Ast_ErrorDecl,
  Ast_MatchKindDecl,
  Ast_FunctionDecl,
  Ast_Dontcare,
  Ast_IntTypeSize,
  Ast_IntLiteral,
  Ast_BoolLiteral,
  Ast_StringLiteral,
  Ast_Tuple,
  Ast_TupleKeyset,
  Ast_HeaderStack,
  Ast_SpecializedType,
  Ast_SpecifiedIdent,
  Ast_StructField,
  Ast_ParserType,
  Ast_Argument,
  Ast_VarDecl,
  Ast_DirectApplication,
  Ast_Parameter,
  Ast_Lvalue,
  Ast_AssignmentStmt,
  Ast_MethodCallStmt,
  Ast_EmptyStmt,
  Ast_Default,
  Ast_SelectCase,
  Ast_ParserState,
  Ast_ControlType,
  Ast_KeyElement,
  Ast_ActionRef,
  Ast_TableEntry,
  Ast_TableProp_Key,
  Ast_TableProp_Actions,
  Ast_TableProp_Entries,
  Ast_TableProp_SingleEntry,
  Ast_TableDecl,
  Ast_IfStmt,
  Ast_ExitStmt,
  Ast_ReturnStmt,
  Ast_SwitchLabel,
  Ast_SwitchCase,
  Ast_SwitchStmt,
  Ast_BlockStmt,
  Ast_KvPair,
  Ast_P4Program,
  Ast_SelectExpr,
  Ast_ExpressionListExpr,
  Ast_CastExpr,
  Ast_UnaryExpr,
  Ast_BinaryExpr,
  Ast_MemberSelectExpr,
  Ast_IndexedArrayExpr,
  Ast_FunctionCallExpr,
};

enum AstIntegerFlags {
  AstInteger_NONE,
  AstInteger_HasWidth,
  AstInteger_IsSigned,
};

enum AstExprOperator {
  AstExprOp_NONE,
  AstExprOp_Add,
  AstExprOp_Sub,
  AstExprOp_Mul,
  AstExprOp_Div,
  AstExprOp_And,
  AstExprOp_Or,
  AstExprOp_Equal,
  AstExprOp_NotEqual,
  AstExprOp_Less,
  AstExprOp_Greater,
  AstExprOp_LessEqual,
  AstExprOp_GreaterEqual,
  AstExprOp_BitwiseAnd,
  AstExprOp_BitwiseOr,
  AstExprOp_BitwiseXor,
  AstExprOp_BitwiseShiftLeft,
  AstExprOp_BitwiseShiftRight,
  AstExprOp_Mask,
  AstExprOp_Minus,
  AstExprOp_LogicNot,
  AstExprOp_BitwiseNot,
};

enum AstParamDirection {
  AstParamDir_NONE,
  AstParamDir_In,
  AstParamDir_Out,
  AstParamDir_InOut,
};

enum AstAttributeType {
  AstAttr_NONE,
  AstAttr_Ast,
  AstAttr_AstList,
  AstAttr_Integer,
  AstAttr_String,
  AstAttr_ExprOperator,
  AstAttr_BaseType,
  AstAttr_ParamDir,
  AstAttr_IntFlags,
};

#define AST_ATTRTABLE_CAPACITY_LOG2  4
#define AST_ATTRTABLE_CAPACITY  ((1 << AST_ATTRTABLE_CAPACITY_LOG2) - 1)

struct Ast {
  enum AstKind kind;
  int id;
  int line_nr;
  struct Ast* name;
  struct List* type_args;
  struct Scope* scope;
};

struct Ast_Name {
  struct Ast;
  char* strname;
  bool is_dotprefixed;
  struct SymtableEntry* symtable_entry;
};

struct Ast_BaseType_Bool {
  struct Ast;
};

struct Ast_BaseType_Error {
  struct Ast;
};

struct Ast_BaseType_Int {
  struct Ast;
  struct Ast* size;
};

struct Ast_BaseType_Bit {
  struct Ast;
  struct Ast* size;
};

struct Ast_BaseType_Varbit {
  struct Ast;
  struct Ast* size;
};

struct Ast_BaseType_String {
  struct Ast;
};

struct Ast_BaseType_Void {
  struct Ast;
};

struct Ast_ConstDecl {
  struct Ast;
  struct Ast* type_ref;
  struct Ast* expr;
};

struct Ast_ExternDecl {
  struct Ast;
  struct List* type_params;
  struct List* method_protos;
};

struct Ast_FunctionProto {
  struct Ast;
  bool is_extern;
  struct Ast* return_type;
  struct List* type_params;
  struct List* params;
};

struct Ast_ActionDecl {
  struct Ast;
  struct List* params;
  struct Ast* stmt;
};

struct Ast_HeaderDecl {
  struct Ast;
  struct List* fields;
};

struct Ast_HeaderUnionDecl {
  struct Ast;
  struct List* fields;
};

struct Ast_StructDecl {
  struct Ast;
  struct List* fields;
};

struct Ast_EnumDecl {
  struct Ast;
  struct Ast* type_size;
  struct List* id_list;
};

struct Ast_TypeDecl {
  struct Ast;
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
  struct List* type_params;
  struct List* params;
};

struct Ast_Instantiation {
  struct Ast;
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
  struct Ast;
  enum AstIntegerFlags flags;
  int value;
  int width;
};

struct Ast_BoolLiteral {
  struct Ast;
  bool value;
};

struct Ast_StringLiteral {
  struct Ast;
  char* value;
};

struct Ast_Tuple {
  struct Ast;
};

struct Ast_TupleKeyset {
  struct Ast;
  struct List* expr_list;
};

struct Ast_HeaderStack {
  struct Ast;
  struct Ast* stack_expr;
};

struct Ast_SpecializedType {
  struct Ast;
};

struct Ast_SpecifiedIdent {
  struct Ast;
  struct Ast* init_expr;
};

struct Ast_StructField {
  struct Ast;
  struct Ast* type;
};

struct Ast_ParserType {
  struct Ast;
  struct List* type_params;
  struct List* params;
};

struct Ast_Argument {
  struct Ast;
  struct Ast* init_expr;
};

struct Ast_VarDecl {
  struct Ast;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_DirectApplication {
  struct Ast;
  struct List* args;
};

struct Ast_Parameter {
  struct Ast;
  enum AstParamDirection direction;
  struct Ast* type;
  struct Ast* init_expr;
};

struct Ast_Lvalue {
  struct Ast;
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
};

struct Ast_EmptyStmt {
  struct Ast;
};

struct Ast_Default {
  struct Ast;
};

struct Ast_SelectCase {
  struct Ast;
  struct Ast* keyset;
};

struct Ast_ParserState {
  struct Ast;
  struct List* stmt_list;
  struct Ast* trans_stmt;
};

struct Ast_ControlType {
  struct Ast;
  struct List* type_params;
  struct List* params;
};

struct Ast_KeyElement {
  struct Ast;
  struct Ast* expr;
};

struct Ast_ActionRef {
  struct Ast;
  struct List* args;
};

struct Ast_TableEntry {
  struct Ast;
  struct Ast* keyset;
  struct Ast* action;
};

struct Ast_TableProp_Key {
  struct Ast;
  struct List* keyelem_list;
};

struct Ast_TableProp_Actions {
  struct Ast;
  struct List* action_list;
};

struct Ast_TableProp_Entries {
  struct Ast;
  bool is_const;
  struct List* entries;
};

struct Ast_TableProp_SingleEntry {
  struct Ast;
  struct Ast* init_expr;
};

struct Ast_TableDecl {
  struct Ast;
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

struct Ast_SwitchLabel {
  struct Ast;
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

struct Ast_KvPair {
  struct Ast;
  struct Ast* expr;
};

struct Ast_P4Program {
  struct Ast;
  struct List* decl_list;
};

struct Ast_SelectExpr {
  struct Ast;
  struct List* expr_list;
  struct List* case_list;
};

struct Ast_ExpressionListExpr {
  struct Ast;
  struct List* expr_list;
};

struct Ast_CastExpr {
  struct Ast;
  struct Ast* to_type;
  struct Ast* expr;
};

struct Ast_UnaryExpr {
  struct Ast;
  enum AstExprOperator op;
  struct Ast* operand;
};

struct Ast_BinaryExpr {
  struct Ast;
  enum AstExprOperator op;
  struct Ast* left_operand;
  struct Ast* right_operand;
};

struct Ast_MemberSelectExpr {
  struct Ast;
  struct Ast* expr;
  struct Ast* member_name;
};

struct Ast_IndexedArrayExpr {
  struct Ast;
  struct Ast* index;
  struct Ast* colon_index;
};

struct Ast_FunctionCallExpr {
  struct Ast;
  struct Ast* expr;
  struct List* args;
};

void print_ast(struct Ast* ast);
