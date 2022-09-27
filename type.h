#pragma once
#include "basic.h"
#include "arena.h"

enum TypeEnum {
  TYPE_NAME = 1,
  TYPE_BASIC,
  TYPE_TYPEVAR,
  TYPE_TYPEDEF,
  TYPE_TYPENAME,
  TYPE_TYPEPARAM,
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
  struct Type* type_params;
  uint32_t equiv;
};

struct Type_Basic {
  struct Type;
  char* strname;
  enum BasicType basic_ty;
};

struct Type_Typevar {
  struct Type;
};

struct Type_Name {
  struct Type;
  char* strname;
  bool is_typedef;
};

struct Type_TypeParam {
  struct Type;
  char* strname;
};

struct Type_Product {
  struct Type;
  struct Type* lhs_ty;
  struct Type* rhs_ty;
};

struct Type_Function {
  struct Type;
  struct Type* params_ty;
  struct Type* return_ty;
};

struct Type_FunctionCall {
  struct Type;
  struct Type* function_ty;
  struct Type* args_ty;
  struct Type* return_ty;
};
