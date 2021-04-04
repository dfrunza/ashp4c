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
  AstBinOp_Mask,
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

enum AstKind {
  Ast_None,
  Ast_BaseType,
  Ast_TypeName,
  Ast_HeaderStack,
  Ast_SpecdType,
  Ast_Error,
  Ast_MatchKind,
  Ast_EnumDecl,
  Ast_ConstDecl,
  Ast_ActionDecl,
  Ast_TypeDecl,
  Ast_ExternDecl,
  Ast_FunctionProto,
  Ast_FunctionDecl,
  Ast_ControlType,
  Ast_Control,
  Ast_ParserType,
  Ast_Parser,
  Ast_HeaderDecl,
  Ast_HeaderUnionDecl,
  Ast_StructDecl,
  Ast_Instantiation,
  Ast_Package,
  Ast_P4Program,
};

struct ListLink
{
  struct ListLink* prev_lp;
  struct ListLink* next_lp;
  void* object;
};

struct List
{
  struct ListLink sentinel;
  struct ListLink* head_lp;
  struct ListLink* tail_lp;
  int link_count;
};

struct Ast {
  enum AstKind kind;
  int id;
  int line_nr;
};

struct Ast_BaseType {
  struct Ast;
  char* name;
};

struct Ast_TypeName {
  struct Ast;
  char* name;
};

struct Ast_HeaderStack {
  struct Ast;
  char* name;
  int size;
};

struct Ast_SpecdType {
  struct Ast;
  char* name;
};

struct Ast_Error {
  struct Ast;
};

struct Ast_MatchKind {
  struct Ast;
};

struct Ast_EnumDecl {
  struct Ast;
};

struct Ast_ConstDecl {
  struct Ast;
  char* name;
  struct Ast* type_ref;
};

struct Ast_ActionDecl {
  struct Ast;
  char* name;
};

struct Ast_TypeDecl {
  struct Ast;
  bool is_typedef;
  char* name;
  struct Ast* type_ref;
};

struct Ast_ExternDecl {
  struct Ast;
  char* name;
};

struct Ast_FunctionProto {
  struct Ast;
  char* name;
  struct Ast* return_type;
};

struct Ast_FunctionDecl {
  struct Ast;
  struct Ast* proto;
};

struct Ast_ControlType {
  struct Ast;
  char* name;
};

struct Ast_Control {
  struct Ast;
  struct Ast* control_type;
};

struct Ast_ParserType {
  struct Ast;
  char* name;
};

struct Ast_Parser {
  struct Ast;
  struct Ast* parser_type;
};

struct Ast_HeaderDecl {
  struct Ast;
  char* name;
};

struct Ast_HeaderUnionDecl {
  struct Ast;
  char* name;
};

struct Ast_StructDecl {
  struct Ast;
  char* name;
};

struct Ast_Instantiation {
  struct Ast;
  char* name;
  struct Ast* type_ref;
};

struct Ast_Package {
  struct Ast;
  char* name;
};

struct Ast_P4Program {
  struct Ast;
  struct List decl_list;
};

struct AstTree {
  struct Ast* p4program;
  int node_count;
  struct Arena* arena;
};

