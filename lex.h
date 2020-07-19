#pragma once
#include <stdio.h>
#include "basic.h"
#include "arena.h"

enum TokenClass
{
  TOK_NONE,
  TOK_SEMICOLON,
  TOK_IDENT,
  TOK_TYPE_IDENT,
  TOK_STRING,
  TOK_INTEGER,
  TOK_WINTEGER,
  TOK_SINTEGER,
  TOK_INTEGER_HEX,
  TOK_WINTEGER_HEX,
  TOK_SINTEGER_HEX,
  TOK_INTEGER_OCT,
  TOK_WINTEGER_OCT,
  TOK_SINTEGER_OCT,
  TOK_INTEGER_BIN,
  TOK_WINTEGER_BIN,
  TOK_SINTEGER_BIN,
  TOK_PARENTH_OPEN,
  TOK_PARENTH_CLOSE,
  TOK_ANGLE_OPEN,
  TOK_ANGLE_CLOSE,
  TOK_BRACE_OPEN,
  TOK_BRACE_CLOSE,
  TOK_DONTCARE,
  TOK_COLON,
  TOK_PERIOD,
  TOK_COMMA,
  TOK_MINUS,
  TOK_PLUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_EQUAL,
  TOK_EQUAL_EQUAL,

  TOK_KW_ACTION,
  TOK_KW_CONST,
  TOK_KW_ENUM,
  TOK_KW_IN,
  TOK_KW_PACKAGE,
  TOK_KW_SELECT,
  TOK_KW_SWITCH,
  TOK_KW_TUPLE,
  TOK_KW_VOID,
  TOK_KW_APPLY,
  TOK_KW_CONTROL,
  TOK_KW_ERROR,
  TOK_KW_HEADER,
  TOK_KW_INOUT,
  TOK_KW_PARSER,
  TOK_KW_STATE,
  TOK_KW_TABLE,
  TOK_KW_TYPEDEF,
  TOK_KW_BIT,
  TOK_KW_DEFAULT,
  TOK_KW_EXTERN,
  TOK_KW_HEADER_UNION,
  TOK_KW_INT,
  TOK_KW_OUT,
  TOK_KW_STRING,
  TOK_KW_TRANSITION,
  TOK_KW_VARBIT,
  TOK_KW_ELSE,
  TOK_KW_EXIT,
  TOK_KW_IF,
  TOK_KW_MATCH_KIND,
  TOK_KW_RETURN,
  TOK_KW_STRUCT,
  //TOK_KW_BOOL,
  //TOK_KW_FALSE,
  //TOK_KW_TRUE,
  //TOK_KW_VERIFY,
  //TOK_KW_VAR,

  TOK_UNKNOWN,
  TOK_EOI,    // End Of Input
};

enum IdentObjectKind
{
  IDOBJ_NONE,
  IDOBJ_KEYWORD,
  IDOBJ_TYPE,
  IDOBJ_TYPEVAR,
  IDOBJ_VAR,
  IDOBJ_FIELD,
};

typedef struct IdentInfo
{
  enum IdentObjectKind object_kind;
  char* name;
  int scope_level;
  struct IdentInfo* next_in_scope;
}
IdentInfo;

typedef struct IdentInfo_Selector
{
  IdentInfo;
  struct IdentInfo_Selector* next_selector;
}
IdentInfo_Selector;

typedef struct
{
  IdentInfo;
  IdentInfo_Selector* selector;
}
IdentInfo_Type;

typedef struct
{
  IdentInfo;
  enum TokenClass token_klass;
}
IdentInfo_Keyword;

typedef struct
{
  IdentInfo;
  IdentInfo_Type* type;
}
IdentInfo_Var;

typedef struct NamespaceInfo
{
  char* name;
  IdentInfo* ns_global;
  IdentInfo* ns_type;
  struct NamespaceInfo* next;
}
NamespaceInfo;

#include "syntax.h"
#include "symtab.h"

typedef struct
{
  enum TokenClass klass;
  char* lexeme;
  struct IdentInfo* ident;
}
Token;

void lex_init(Arena* arena, char* filename);
void lex_next_token(Token* token);
int lex_line_nr();


