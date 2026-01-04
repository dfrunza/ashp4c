#include "basic.h"
#include "type.h"

char* TypeEnum_to_string(enum TypeEnum type)
{
  switch(type) {
    case TypeEnum::None: return "NONE";
    case TypeEnum::Void: return "VOID";
    case TypeEnum::Bool: return "BOOL";
    case TypeEnum::Int: return "INT";
    case TypeEnum::Bit: return "BIT";
    case TypeEnum::Varbit: return "VARBIT";
    case TypeEnum::String: return "STRING";
    case TypeEnum::Any: return "ANY";
    case TypeEnum::Enum: return "ENUM";
    case TypeEnum::Typedef: return "TYPEDEF";
    case TypeEnum::Function: return "FUNCTION";
    case TypeEnum::Extern: return "EXTERN";
    case TypeEnum::Package: return "PACKAGE";
    case TypeEnum::Parser: return "PARSER";
    case TypeEnum::Control: return "CONTROL";
    case TypeEnum::Table: return "TABLE";
    case TypeEnum::Struct: return "STRUCT";
    case TypeEnum::Header: return "HEADER";
    case TypeEnum::Union: return "UNION";
    case TypeEnum::Stack: return "STACK";
    case TypeEnum::State: return "STATE";
    case TypeEnum::Field: return "FIELD";
    case TypeEnum::Error: return "ERROR";
    case TypeEnum::MatchKind: return "MATCH_KIND";
    case TypeEnum::Nameref: return "NAMEREF";
    case TypeEnum::Type: return "TYPE";
    case TypeEnum::Tuple: return "TUPLE";
    case TypeEnum::Product: return "PRODUCT";

    default: return "?";
  }
  assert(0);
  return 0;
}

void Type::create(enum TypeEnum kind, char* strname)
{
  this->kind = kind;
  this->strname = strname;
}

Type* Type::actual_type()
{
  if (!this) { return 0; }
  if (kind == TypeEnum::Type) {
    return type.type;
  }
  return this;
}

Type* Type::effective_type()
{
  Type* applied_ty = actual_type();
  if (!applied_ty) { return 0; }
  if (kind == TypeEnum::Function) {
    return function.return_->actual_type();
  } else if (kind == TypeEnum::Field) {
    return field.type->actual_type();
  } else if (kind == TypeEnum::Stack) {
    return header_stack.element->actual_type();
  }
  return applied_ty;
}

void Type_Product::create(Arena* storage, int count)
{
  this->count = count;
  members = (Type**)storage->allocate(sizeof(Type*), count);
}

void Type_Product::set(int i, Type* ty)
{
  assert(i >= 0 && i < count);
  members[i] = ty;
}

Type* Type_Product::get(int i)
{
  assert(i >= 0 && i < count);
  return members[i];
}
