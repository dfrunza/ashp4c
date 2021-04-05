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

struct Cst_NonTypeName {
  struct Cst;
};

struct Cst_TypeName {
  struct Cst;
};

struct Cst_BaseType {
  struct Cst;
};

struct Cst_ConstDecl {
  struct Cst;
};

struct Cst_ExternDecl {
  struct Cst;
};

struct Cst_FunctionProto {
  struct Cst;
};

struct Cst_ActionDecl {
  struct Cst;
};

struct Cst_HeaderDecl {
  struct Cst;
};

struct Cst_HeaderUnionDecl {
  struct Cst;
};

struct Cst_StructDecl {
  struct Cst;
};

struct Cst_EnumDecl {
  struct Cst;
};

struct Cst_TypeDecl {
  struct Cst;
};

struct Cst_Parser {
  struct Cst;
};

struct Cst_Control {
  struct Cst;
};

struct Cst_Package {
  struct Cst;
};

struct Cst_Instantiation {
  struct Cst;
};

struct Cst_Error {
  struct Cst;
};

struct Cst_MatchKind {
  struct Cst;
};

struct Cst_FunctionDecl {
  struct Cst;
};

struct Cst_Parameter {
  struct Cst;
};

struct Cst_Dontcare {
  struct Cst;
};

struct Cst_IntTypeSize {
  struct Cst;
};

struct Cst_Int {
  struct Cst;
};

struct Cst_Bool {
  struct Cst;
};

struct Cst_StringLiteral {
  struct Cst;
};

struct Cst_Tuple {
  struct Cst;
};

struct Cst_HeaderStack {
  struct Cst;
};

struct Cst_SpecdType {
  struct Cst;
};

struct Cst_StructField {
  struct Cst;
};

struct Cst_SpecdId {
  struct Cst;
};

struct Cst_ParserType {
  struct Cst;
};

struct Cst_Argument {
  struct Cst;
};

struct Cst_VarDecl {
  struct Cst;
};

struct Cst_DirectApplic {
  struct Cst;
};

struct Cst_Lvalue {
  struct Cst;
};

struct Cst_ArrayIndex {
  struct Cst;
};

struct Cst_AssignmentStmt {
  struct Cst;
};

struct Cst_MethodCallStmt {
  struct Cst;
};

struct Cst_EmptyStmt {
  struct Cst;
};

struct Cst_Default {
  struct Cst;
};

struct Cst_SelectExpr {
  struct Cst;
};

struct Cst_SelectCase {
  struct Cst;
};

struct Cst_ParserState {
  struct Cst;
};

struct Cst_ControlType {
  struct Cst;
};

struct Cst_KeyElement {
  struct Cst;
};

struct Cst_ActionRef {
  struct Cst;
};

struct Cst_TableEntry {
  struct Cst;
};

struct Cst_TableProp_Key {
  struct Cst;
};

struct Cst_TableProp_Actions {
  struct Cst;
};

struct Cst_TableProp_Entries {
  struct Cst;
};

struct Cst_TableProp_SingleEntry {
  struct Cst;
};

struct Cst_TableDecl {
  struct Cst;
};

struct Cst_IfStmt {
  struct Cst;
};

struct Cst_ExitStmt {
  struct Cst;
};

struct Cst_ReturnStmt {
  struct Cst;
};

struct Cst_SwitchLabel {
  struct Cst;
};

struct Cst_SwitchCase {
  struct Cst;
};

struct Cst_SwitchStmt {
  struct Cst;
};

struct Cst_BlockStmt {
  struct Cst;
};

struct Cst_ExpressionListExpr {
  struct Cst;
};

struct Cst_CastExpr {
  struct Cst;
};

struct Cst_UnaryExpr {
  struct Cst;
};

struct Cst_BinaryExpr {
  struct Cst;
};

struct Cst_KvPair {
  struct Cst;
};

struct Cst_MemberSelectExpr {
  struct Cst;
};

struct Cst_IndexedArrayExpr {
  struct Cst;
};

struct Cst_FunctionCallExpr {
  struct Cst;
};

struct Cst_TypeArgsExpr {
  struct Cst;
};

struct Cst_P4Program {
  struct Cst;
};

struct CstTree {
  struct Arena* arena;
  struct Cst* p4program;
  int node_count;
};

void* cst_getattr(struct Cst* cst, char* attr_name);
void cst_setattr(struct Cst* cst, char* attr_name, void* attr_value);

