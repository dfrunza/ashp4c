#pragma once
#include "basic.h"
#include "arena.h"


enum AstKind {
  Ast_NONE_,
  Ast_Name,
  Ast_BaseType,
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
  Ast_Error,
  Ast_MatchKind,
  Ast_FunctionDecl,
  Ast_Dontcare,
  Ast_IntTypeSize,
  Ast_Int,
  Ast_Bool,
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
  Ast_ArrayIndex,
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
  Ast_TypeArgsExpr,
};

enum AstBaseType {
  AstBaseType_NONE_,
  AstBaseType_Bool,
  AstBaseType_Error,
  AstBaseType_Int,
  AstBaseType_Bit,
  AstBaseType_Varbit,
  AstBaseType_String,
};

enum AstIntegerFlags {
  AstInteger_NONE_,
  AstInteger_HasWidth,
  AstInteger_IsSigned,
};

enum AstExprOperator {
  AstExprOp_NONE_,
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
  AstExprOp_BitAnd,
  AstExprOp_BitOr,
  AstExprOp_BitXor,
  AstExprOp_BitShiftLeft,
  AstExprOp_BitShiftRight,
  AstExprOp_Mask,
  AstExprOp_Minus,
  AstExprOp_LogNot,
  AstExprOp_BitNot,
};

enum AstParamDirection {
  AstParamDir_NONE_,
  AstParamDir_In,
  AstParamDir_Out,
  AstParamDir_InOut,
};

enum AstAttributeType {
  AstAttr_NONE_,
  AstAttr_Ast,
  AstAttr_AstList,
  AstAttr_Integer,
  AstAttr_String,
  AstAttr_ExprOperator,
  AstAttr_BaseType,
  AstAttr_ParamDir,
  AstAttr_IntFlags,
};

struct AstAttribute {
  enum AstAttributeType type;
  char* name;
  void* value;
  struct AstAttribute* next_attr;
};

#define AST_ATTRTABLE_CAPACITY_LOG2  4
#define AST_ATTRTABLE_CAPACITY  ((1 << AST_ATTRTABLE_CAPACITY_LOG2) - 1)

struct Ast {
  enum AstKind kind;
  int id;
  int line_nr;
  struct Scope* scope;
  int attr_count;
  struct AstAttribute* attrs[AST_ATTRTABLE_CAPACITY];
};

struct AstAttributeIterator {
  struct Ast* ast;
  int table_i;
  struct AstAttribute* attr_at;
};

void ast_attr_set_storage(struct Arena* attr_storage);
void* ast_getattr(struct Ast* ast, char* attr_name);
void ast_setattr(struct Ast* ast, char* attr_name, void* attr_value, enum AstAttributeType attr_type);
struct AstAttribute* ast_attriter_init(struct AstAttributeIterator* iter, struct Ast* ast);
struct AstAttribute* ast_attriter_get_next(struct AstAttributeIterator* iter);

void print_ast(struct Ast* ast);
