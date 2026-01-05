#pragma once
#include "array.h"
#include "type.h"
#include "potential_type.h"

struct TypeChecker {
  Array* type_equiv_pairs;

  void allocate(Arena* storage);
  bool match_type(PotentialType* potential_types, Type* required_ty);
  bool match_params(PotentialType* potential_args, Type* params_ty);
  void collect_matching_member(PotentialType* tau, Type* product_ty,
           char* strname, PotentialType* potential_args);
  bool structural_type_equiv(Type* left, Type* right);
  bool type_equiv(Type* left, Type* right);
};
