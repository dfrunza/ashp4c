#pragma once
#include <ast.h>

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

struct Type;

struct Type_Basic {
  int size;
};

struct Type_Typedef {
  Type* ref;
};

struct Type_Struct {
  Type* fields;
  int field_count;
  int i;
};

struct Type_Enum {
  Type* fields;
  int field_count;
  int i;
};

struct Type_Function {
  Type* params;
  Type* return_;
};

struct Type_Extern {
  Type* methods;
  Type* ctors;
};

struct Type_Parser {
  Type* params;
  Type* ctor_params;
  Type* methods;
};

struct Type_Control {
  Type* params;
  Type* ctor_params;
  Type* methods;
};

struct Type_Table {
  Type* methods;
};

struct Type_Package {
  Type* params;
};

struct Type_HeaderStack {
  Type* element;
  int size;
};

struct Type_Field {
  Type* type;
};

struct Type_Nameref {
  Ast* name;
  struct Scope* scope;
};

struct Type_Type {
  Type* type;
};

struct Type_Tuple {
  Type* left;
  Type* right;
}; /* 2-tuple */

struct Type_Product {
  Type** members;
  int count;
};

struct Type {
  enum TypeEnum ty_former;
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
    struct Type_Nameref nameref;
    struct Type_Type type;
    struct Type_Tuple tuple;
    struct Type_Product product;
  };

  Type* actual_type()
  {
      if (!this) { return 0; }
      if (ty_former == TypeEnum::TYPE) {
          return type.type;
      }
      return this;
  }

  Type* effective_type()
  {
      Type* applied_ty = actual_type();
      if (!applied_ty) { return 0; }
      if (ty_former == TypeEnum::FUNCTION) {
          return function.return_->actual_type();
      } else if (ty_former == TypeEnum::FIELD) {
          return field.type->actual_type();
      } else if (ty_former == TypeEnum::STACK) {
          return header_stack.element->actual_type();
      }
      return applied_ty;
  }
};
