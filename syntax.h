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

struct Cst_Declaration {
  struct Cst;
};

struct Cst_P4Program {
  struct Cst;
};

void build_cst();
