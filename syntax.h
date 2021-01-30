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
  Cst_PrefixedType,
  Cst_BaseType,
  Cst_DotPrefix,
  Cst_ConstDecl,
  Cst_ExternDecl,
  Cst_ExternFuncDecl,
  Cst_TypeParam,
  Cst_MethodProto,
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
  Cst_Tuple,
  Cst_HeaderStack,
  Cst_SpecdType,
  Cst_StructField,
  Cst_SpecdId,
  Cst_ParserType,
  Cst_Argument,
  Cst_VarDecl,
  Cst_DirectApplic,
  Cst_SlicedIndex,
  Cst_Parameter,
  Cst_Lvalue,
};

enum Cst_ParameterDirection {
  Cst_DirNone,
  Cst_DirIn,
  Cst_DirOut,
  Cst_DirInOut,
};

enum Cst_ExprOperator {
  Cst_OpNone,
  Cst_OpLogicEqual,
  Cst_OpAssign,
  Cst_OpAddition,
  Cst_OpSubtract,
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

struct Cst_Name {
  struct Cst;
  char* name;
};

struct Cst_NonTypeName {
  struct Cst_Name;
};

struct Cst_TypeName {
  struct Cst_Name;
};

struct Cst_DotPrefix {
  struct Cst;
};

struct Cst_PrefixedType {
  struct Cst;
  struct Cst_TypeName* first_name;
  struct Cst_TypeName* second_name;
};

enum BaseTypeKind {
  BASETYPE_NONE,
  BASETYPE_BOOL,
  BASETYPE_ERROR,
  BASETYPE_INT,
  BASETYPE_BIT,
  BASETYPE_VARBIT,
};

struct Cst_BaseType {
  struct Cst;
  enum BaseTypeKind base_type;
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

struct Cst_MethodProto {
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

struct Cst_SlicedIndex {
  struct Cst;
  struct Cst* idx_left;
  struct Cst* idx_right;
};

struct Cst_P4Program {
  struct Cst;
};

