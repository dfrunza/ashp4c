#pragma once
#include "ast.h"
#include "adt/array.h"

enum class TypeEnum : int {
  None = 0,
  Void,
  Bool,
  Int,
  Bit,
  Varbit,
  String,
  Any,
  Enum,
  Typedef,
  Function,
  Extern,
  Package,
  Parser,
  Control,
  Table,
  Struct,
  Header,
  Union,
  HeaderStack,
  State,
  Field,
  Error,
  MatchKind,
  Nameref,
  Type,
  Tuple,
  Product,
};
char* TypeEnum_to_string(enum TypeEnum type);

struct Type;

struct Type_Basic {
  int size;
  enum TypeEnum basic_type;

  static Type* append(Array* array, enum TypeEnum basic_type);
};

struct Type_Typedef {
  Type* ref;

  static Type* append(Array* array);
};

struct Type_Struct {
  Type* fields;
  int field_count;
  int i;

  static Type* append(Array* array);
};

struct Type_Enum {
  Type* fields;
  int field_count;
  int i;

  static Type* append(Array* array);
};

struct Type_Function {
  Type* params;
  Type* return_;

  static Type* append(Array* array);
};

struct Type_Extern {
  Type* methods;
  Type* ctors;

  static Type* append(Array* array);
};

struct Type_Parser {
  Type* params;
  Type* ctor_params;
  Type* methods;

  static Type* append(Array* array);
};

struct Type_Control {
  Type* params;
  Type* ctor_params;
  Type* methods;

  static Type* append(Array* array);
};

struct Type_Table {
  Type* methods;

  static Type* append(Array* array);
};

struct Type_Package {
  Type* params;

  static Type* append(Array* array);
};

struct Type_HeaderStack {
  Type* element;
  int size;

  static Type* append(Array* array);
};

struct Type_Field {
  Type* type;

  static Type* append(Array* array);
};

struct Type_State {
  static Type* append(Array* array);
};

struct Type_Nameref {
  Ast* name;
  struct Scope* scope;

  static Type* append(Array* array);
};

struct Type_Type {
  Type* type;

  static Type* append(Array* array);
};

struct Type_Tuple {
  Type* left;
  Type* right;

  static Type* append(Array* array);
}; /* 2-tuple */

struct Type_Product {
  Type** members;
  int count;

  static Type* append(Array* array, Arena* storage, int count);
  void set(int i, Type* ty);
  Type* get(int i);
};

struct Type {
  enum TypeEnum kind;
  char* strname;
  Ast* ast;

  union {
    struct Type_Basic basic;
    struct Type_Typedef typedef_;
    struct Type_Struct struct_;
    struct Type_Enum enum_;
    struct Type_Function function;
    struct Type_Extern extern_;
    struct Type_Parser parser;
    struct Type_Control control;
    struct Type_Table table;
    struct Type_Package package;
    struct Type_HeaderStack header_stack;
    struct Type_Field field;
    struct Type_State state;
    struct Type_Nameref nameref;
    struct Type_Type type;
    struct Type_Tuple tuple;
    struct Type_Product product;
  };

  Type* actual_type();
  Type* effective_type();
};
