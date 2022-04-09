#pragma once
#include "basic.h"
#include "arena.h"


enum TypeEnum {
  TYPE_NAME = 1,
  TYPE_TYPEVAR,
  TYPE_TYPEDEF,
  TYPE_TYPENAME,
  TYPE_PRODUCT,
  TYPE_FUNCTION,
  TYPE_FUNCTION_CALL,
};

enum BasicType {
  TYPE_INT = 1,
  TYPE_STRING,
  TYPE_VOID,
};

struct Type {
  enum TypeEnum ctor;
  char* strname;
  uint32_t id;
  uint32_t equiv;
};

struct Type_Basic {
  struct Type;
  enum BasicType basic_type;
};

struct Type_Typevar {
  struct Type;
};

struct Type_Name {
  struct Type;
};

struct Type_Typedef {
  struct Type;
  struct Type* type;
};

struct Type_Typename {
  struct Type;
  struct Type* type;
};

struct Type_Product {
  struct Type;
  struct Type* lhs;
  struct Type* rhs;
};

struct Type_Function {
  struct Type;
  struct Type* domain;
  struct Type* range;
};

struct Type_FunctionCall {
  struct Type;
  struct Type* function;
  struct Type* arg;
};
