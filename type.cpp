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
    case TypeEnum::HeaderStack: return "HEADER_STACK";
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

#if 0
void Type::init(enum TypeEnum kind, char* strname)
{
  this->kind = kind;
  this->strname = strname;
}
#endif

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
  } else if (kind == TypeEnum::HeaderStack) {
    return header_stack.element->actual_type();
  }
  return applied_ty;
}

Type* Type_Basic::append(Array* array, enum TypeEnum basic_type)
{
  Type* ty = (Type*)array->append();
  ty->kind = basic_type;
  return ty;
}

Type* Type_Typedef::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Typedef;
  return ty;
}

Type* Type_Struct::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Struct;
  return ty;
}

Type* Type_Enum::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Enum;
  return ty;
}

Type* Type_Function::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Function;
  return ty;
}

Type* Type_Extern::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Extern;
  return ty;
}

Type* Type_Parser::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Parser;
  return ty;
}

Type* Type_Control::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Control;
  return ty;
}

Type* Type_Table::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Table;
  return ty;
}

Type* Type_Package::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Package;
  return ty;
}

Type* Type_HeaderStack::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Header;
  return ty;
}

Type* Type_Field::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Field;
  return ty;
}

Type* Type_State::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::State;
  return ty;
}

Type* Type_Nameref::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Nameref;
  return ty;
}

Type* Type_Type::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Type;
  return ty;
}

Type* Type_Tuple::append(Array* array)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Tuple;
  return ty;
}

Type* Type_Product::append(Array* array, Arena* storage, int count)
{
  Type* ty = (Type*)array->append();
  ty->kind = TypeEnum::Product;
  ty->product.count = count;
  if (count > 0) {
    ty->product.members = (Type**)storage->allocate(sizeof(Type*), count);
  }
  return ty;
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
