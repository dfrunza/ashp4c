#pragma once
#include "basic.h"

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
  AstUnOp_LogNot,
  AstUnOp_BitNot,
  AstUnOp_ArMinus,
};

enum AstParamDirKind {
  AstDir_None,
  AstDir_In,
  AstDir_Out,
  AstDir_InOut,
};

enum AstBaseTypeKind {
  AstBaseType_None,
  AstBaseType_Bool,
  AstBaseType_Error,
  AstBaseType_Int,
  AstBaseType_Bit,
  AstBaseType_Varbit,
};

enum AstKind {
  Ast_None,
  Ast_HeaderDecl,
  Ast_P4Program,
};

struct ListLink
{
  struct ListLink* prev;
  struct ListLink* next;
  void* object;
};

struct Ast {
  enum AstKind kind;
  int id;
  int line_nr;
};

struct Ast_HeaderDecl {
  struct Ast;
  char* name;
};

struct Ast_P4Program {
  struct Ast;
  struct ListLink decl_list;
  int decl_count;
};

struct AstTree {
  struct Arena* arena;
  struct Ast* p4program;
  int node_count;
};
