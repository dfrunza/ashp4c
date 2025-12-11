#include <basic.h>
#include <type.h>

char* TypeEnum_to_string(enum TypeEnum type)
{
  switch(type) {
    case TypeEnum::NONE: return "NONE";
    case TypeEnum::VOID: return "VOID";
    case TypeEnum::BOOL: return "BOOL";
    case TypeEnum::INT: return "INT";
    case TypeEnum::BIT: return "BIT";
    case TypeEnum::VARBIT: return "VARBIT";
    case TypeEnum::STRING: return "STRING";
    case TypeEnum::ANY: return "ANY";
    case TypeEnum::ENUM: return "ENUM";
    case TypeEnum::TYPEDEF: return "TYPEDEF";
    case TypeEnum::FUNCTION: return "FUNCTION";
    case TypeEnum::EXTERN: return "EXTERN";
    case TypeEnum::PACKAGE: return "PACKAGE";
    case TypeEnum::PARSER: return "PARSER";
    case TypeEnum::CONTROL: return "CONTROL";
    case TypeEnum::TABLE: return "TABLE";
    case TypeEnum::STRUCT: return "STRUCT";
    case TypeEnum::HEADER: return "HEADER";
    case TypeEnum::UNION: return "UNION";
    case TypeEnum::STACK: return "STACK";
    case TypeEnum::STATE: return "STATE";
    case TypeEnum::FIELD: return "FIELD";
    case TypeEnum::ERROR: return "ERROR";
    case TypeEnum::MATCH_KIND: return "MATCH_KIND";
    case TypeEnum::NAMEREF: return "NAMEREF";
    case TypeEnum::TYPE: return "TYPE";
    case TypeEnum::TUPLE: return "TUPLE";
    case TypeEnum::PRODUCT: return "TYPE_PRODUCT";

    default: return "?";
  }
  assert(0);
  return 0;
}
