#pragma once
#include "basic.h"
#include "arena.h"

struct Cst;

enum TokenClass {
  Token_None,
  Token_Semicolon,
  Token_Identifier,
  Token_TypeIdentifier,
  Token_String,
  Token_Integer,
  Token_ParenthOpen,
  Token_ParenthClose,
  Token_AngleOpen,
  Token_AngleClose,
  Token_BraceOpen,
  Token_BraceClose,
  Token_BracketOpen,
  Token_BracketClose,
  Token_Dontcare,
  Token_Colon,
  Token_Dotprefix,
  Token_Comma,
  Token_Minus,
  Token_UnaryMinus,
  Token_Plus,
  Token_Star,
  Token_Slash,
  Token_Equal,
  Token_LogicEqual,
  Token_LogicNotEqual,
  Token_LogicNot,
  Token_LogicAnd,
  Token_LogicOr,
  Token_LessEqual,
  Token_GreaterEqual,
  Token_BitwiseNot,
  Token_BitwiseAnd,
  Token_BitwiseOr,
  Token_BitwiseXor,
  Token_BitshiftLeft,
  Token_BitshiftRight,
  Token_Comment,

  Token_Action,
  Token_Actions,
  Token_Enum,
  Token_In,
  Token_Package,
  Token_Select,
  Token_Switch,
  Token_Tuple,
  Token_Void,
  Token_Apply,
  Token_Control,
  Token_Error,
  Token_Header,
  Token_InOut,
  Token_Parser,
  Token_State,
  Token_Table,
  Token_Entries,
  Token_Key,
  Token_Typedef,
  Token_Type,
  Token_Bool,
  Token_True,
  Token_False,
  Token_Default,
  Token_Extern,
  Token_HeaderUnion,
  Token_Int,
  Token_Bit,
  Token_Varbit,
  Token_Out,
  Token_StringLiteral,
  Token_Transition,
  Token_Else,
  Token_Exit,
  Token_If,
  Token_MatchKind,
  Token_Return,
  Token_Struct,
  Token_Const,
  Token_Var,

  Token_Unknown,
  Token_StartOfInput,
  Token_EndOfInput,
};

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

struct Token {
  enum TokenClass klass;
  char* lexeme;
  int line_nr;
};

#define cast(TYPE, EXPR) ({\
  if ((EXPR)) assert((EXPR)->kind == TYPE); \
  (struct TYPE*)(EXPR);})

enum AstKind {
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
  enum AstKind kind;
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

