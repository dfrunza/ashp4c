#pragma once
#include "parse.h"

enum class TypeEnum : int {
  NONE = 0,
  VOID,
  BOOL,
  INT,
  BIT,
  VARBIT,
  STRING,
  ANY,
  ENUM,
  TYPEDEF,
  FUNCTION,
  EXTERN,
  PACKAGE,
  PARSER,
  CONTROL,
  TABLE,
  STRUCT,
  HEADER,
  UNION,
  STACK,
  STATE,
  FIELD,
  ERROR,
  MATCH_KIND,
  NAMEREF,
  TYPE,
  TUPLE,
  PRODUCT,
};
char* TypeEnum_to_string(enum TypeEnum type);

struct Type {
  enum TypeEnum ty_former;
  char* strname;
  Ast* ast;

  union {
    struct {
      int size;
    } basic;

    struct {
      Type* ref;
    } typedef_;

    struct {
      Type* fields;
      int field_count;
      int i;
    } struct_, enum_;

    struct {
      Type* params;
      Type* return_;
    } function;

    struct {
      Type* methods;
      Type* ctors;
    } extern_;

    struct {
      Type* params;
      Type* ctor_params;
      Type* methods;
    } parser, control;

    struct {
      Type* methods;
    } table;

    struct {
      Type* params;
    } package;

    struct {
      Type* element;
      int size;
    } header_stack;

    struct {
      Type* type;
    } field;

    struct {
      Ast* name;
      struct Scope* scope;
    } nameref;

    struct {
      Type* type;
    } type;

    struct {
      Type* left;
      Type* right;
    } tuple; /* 2-tuple */

    struct {
      Type** members;
      int count;
    } product;
  };

  Type* actual_type();
  Type* effective_type();
};
