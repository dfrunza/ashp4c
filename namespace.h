#pragma once
#include <lexer.h>
#include <type.h>

struct NameEntry;
struct NameDeclaration;

enum class NameSpace : int {
  VAR = 1 << 0,
  TYPE = 1 << 1,
  KEYWORD = 1 << 2,
};
#define NameSpace_COUNT 3
inline NameSpace operator | (NameSpace lhs, NameSpace rhs) {
  return (NameSpace)((int)lhs | (int)rhs);
}
inline NameSpace operator & (NameSpace lhs, NameSpace rhs) {
  return (NameSpace)((int)lhs & (int)rhs);
}
char* NameSpace_to_string(enum NameSpace ns);

struct NameDeclaration {
  char* strname;
  NameDeclaration* next_in_scope;

  union {
    Ast* ast;
    enum TokenClass token_class;
  };

  Type* type;
};

struct NameEntry {
  NameDeclaration* ns[NameSpace_COUNT];
};
