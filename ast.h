#pragma once
#include "basic.h"
#include "ast.h"

enum AstKind {
  Ast_None,
  Ast_NonTypeName,
  Ast_TypeName,
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
  Ast_Parser,
  Ast_Control,
  Ast_Package,
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
  Ast_HeaderStack,
  Ast_SpecdType,
  Ast_StructField,
  Ast_SpecdId,
  Ast_ParserType,
  Ast_Argument,
  Ast_VarDecl,
  Ast_DirectApplic,
  Ast_ArrayIndex,
  Ast_Parameter,
  Ast_Lvalue,
  Ast_AssignmentStmt,
  Ast_MethodCallStmt,
  Ast_EmptyStmt,
  Ast_Default,
  Ast_SelectExpr,
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
  Ast_ExpressionListExpr,
  Ast_CastExpr,
  Ast_UnaryExpr,
  Ast_BinaryExpr,
  Ast_KvPair,
  Ast_MemberSelectExpr,
  Ast_IndexedArrayExpr,
  Ast_FunctionCallExpr,
  Ast_TypeArgsExpr,
  Ast_P4Program,
};

enum AstBaseTypeKind {
  AstBaseType_None,
  AstBaseType_Bool,
  AstBaseType_Error,
  AstBaseType_Int,
  AstBaseType_Bit,
  AstBaseType_Varbit,
  AstBaseType_String,
};

enum AstIntegerFlags
{
  AstInteger_None,
  AstInteger_HasWidth,
  AstInteger_IsSigned,
};

enum AstExprOperator {
  AstOp_None,
  AstBinOp_ArAdd,
  AstBinOp_ArSub,
  AstBinOp_ArMul,
  AstBinOp_ArDiv,
  AstBinOp_LogAnd,
  AstBinOp_LogOr,
  AstBinOp_LogEqual,
  AstBinOp_LogNotEqual,
  AstBinOp_LogLess,
  AstBinOp_LogGreater,
  AstBinOp_LogLessEqual,
  AstBinOp_LogGreaterEqual,
  AstBinOp_BitAnd,
  AstBinOp_BitOr,
  AstBinOp_BitXor,
  AstBinOp_BitShLeft,
  AstBinOp_BitShRight,
  AstBinOp_Mask,
  AstUnOp_LogNot,
  AstUnOp_BitNot,
  AstUnOp_ArMinus,
};

enum AstParamDirection {
  AstParamDir_None,
  AstParamDir_In,
  AstParamDir_Out,
  AstParamDir_InOut,
};

struct AstAttribute {
  char* name;
  void* value;
  struct AstAttribute* next_attr;
};

#define AST_ATTRTABLE_LEN 13

struct Ast {
  enum AstKind kind;
  int id;
  int line_nr;
  struct Ast* prev_node;
  struct Ast* next_node;
  struct AstAttribute* attrs[AST_ATTRTABLE_LEN];
};

struct AstTree {
  struct Arena* arena;
  struct Ast* p4program;
  int node_count;
};

void* ast_getattr(struct Ast* ast, char* attr_name);
void ast_setattr(struct Ast* ast, char* attr_name, void* attr_value);

