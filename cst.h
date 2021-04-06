#pragma once
#include "basic.h"
#include "ast.h"

enum CstKind {
  Cst_None,
  Cst_NonTypeName,
  Cst_TypeName,
  Cst_BaseType,
  Cst_ConstDecl,
  Cst_ExternDecl,
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
  Cst_KvPair,
  Cst_MemberSelectExpr,
  Cst_IndexedArrayExpr,
  Cst_FunctionCallExpr,
  Cst_TypeArgsExpr,
  Cst_P4Program,
};

enum CstBaseTypeKind {
  CstBaseType_None,
  CstBaseType_Bool,
  CstBaseType_Error,
  CstBaseType_Int,
  CstBaseType_Bit,
  CstBaseType_Varbit,
  CstBaseType_String,
};

struct CstAttribute {
  char* name;
  void* value;
  struct CstAttribute* next_attr;
};

#define CST_ATTRTABLE_LEN 13

struct Cst {
  enum CstKind kind;
  int id;
  int line_nr;
  struct Cst* prev_node;
  struct Cst* next_node;
  struct CstAttribute* attrs[CST_ATTRTABLE_LEN];
};

struct CstTree {
  struct Arena* arena;
  struct Cst* p4program;
  int node_count;
};

void* cst_getattr(struct Cst* cst, char* attr_name);
void cst_setattr(struct Cst* cst, char* attr_name, void* attr_value);

