#pragma once
#include <array.h>
#include <type.h>
#include <potential_type.h>

struct TypeChecker {
  Array<Type>* type_equiv_pairs;

  bool match_type(PotentialType* potential_types, Type* required_ty)
  {
    int i = 0;
    for (MapEntry<Type, void>* m = potential_types->set.members.first; m != 0; m = m->next) {
      Type* ty = m->key->effective_type();
      if (type_equiv(ty, required_ty->actual_type())) {
        i += 1;
      }
    }
    return (i == 1);
  }

  bool match_params(PotentialType* potential_args, Type* params_ty)
  {
    int i = 0;
    if (params_ty->product.count != potential_args->product.count) return 0;
    for (i = 0; i < params_ty->product.count; i++) {
      if (!match_type(potential_args->product.members[i],
                      params_ty->product.members[i])) break;
    }
    return (i == params_ty->product.count);
  }

  void collect_matching_member(PotentialType* tau, Type* product_ty,
        char* strname, PotentialType* potential_args)
  {
    for (int i = 0; i < product_ty->product.count; i++) {
      Type* member_ty = product_ty->product.members[i];
      if (cstring::match(member_ty->strname, strname)) {
        if (member_ty->ty_former == TypeEnum::FUNCTION) {
          if (match_params(potential_args, member_ty->function.params)) {
            tau->set.members.insert(member_ty, 0, 1);
          }
        } else {
          tau->set.members.insert(member_ty, 0, 1);
        }
      }
    }
  }

  bool structural_type_equiv(Type* left, Type* right)
  {
    Type* type_pair;
    int i;

    if (left == 0 && right == 0) {
      return 1;
    } else if (left == 0 || right == 0) {
      return 0;
    }

    left = left->actual_type();
    right = right->actual_type();
    if (left == right) return 1;

    for (i = 0; i < type_equiv_pairs->elem_count; i++) {
      type_pair = type_equiv_pairs->get(i);
      assert(type_pair->ty_former == TypeEnum::TUPLE);
      if ((left == type_pair->tuple.left || left == type_pair->tuple.right) &&
          (right == type_pair->tuple.left || right == type_pair->tuple.right)) {
        return 1;
      }
    }

    type_pair = type_equiv_pairs->append();
    type_pair->ty_former = TypeEnum::TUPLE;
    type_pair->tuple.left = left;
    type_pair->tuple.right = right;

    if (left->ty_former == TypeEnum::VOID || left->ty_former == TypeEnum::STRING ||
        left->ty_former == TypeEnum::BOOL || left->ty_former == TypeEnum::INT ||
        left->ty_former == TypeEnum::BIT || left->ty_former == TypeEnum::VARBIT) {
      if (right->ty_former == left->ty_former) {
        return 1;
      }
      return 0;
    } else if (left->ty_former == TypeEnum::ANY) {
      return 1;
    } else if (left->ty_former == TypeEnum::ENUM) {
      if (right->ty_former == left->ty_former) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::EXTERN) {
      if (right->ty_former == left->ty_former) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::TABLE) {
      if (right->ty_former == left->ty_former) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::PRODUCT) {
      if (right->ty_former == left->ty_former) {
        if (left->product.count != right->product.count) {
          return 0;
        }
        for (int i = 0; i < left->product.count; i++) {
          if (!structural_type_equiv(left->product.members[i], left->product.members[i])) {
            return 0;
          }
        }
        return 1;
      }
      return 0;
    } else if (left->ty_former == TypeEnum::FUNCTION) {
      if (right->ty_former == left->ty_former) {
        if (!structural_type_equiv(left->function.return_, right->function.return_)) {
          return 0;
        }
        if (!structural_type_equiv(left->function.params, right->function.params)) {
          return 0;
        }
        return 1;
      }
      return 0;
    } else if (left->ty_former == TypeEnum::PACKAGE) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->package.params, right->package.params);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::PARSER) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->parser.params, right->parser.params);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::CONTROL) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->control.params, right->control.params);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::STRUCT) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->struct_.fields, right->struct_.fields);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::HEADER) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->struct_.fields, right->struct_.fields);
      }
      return 0;
    } else if (left->ty_former == TypeEnum::STACK) {
      if (right->ty_former == left->ty_former) {
        return structural_type_equiv(left->header_stack.element, right->header_stack.element);
      }
      return 0;
    } else assert(0);

    assert(0);
    return 0;
  }

  bool type_equiv(Type* left, Type* right)
  {
    this->type_equiv_pairs->elem_count = 0;
    return structural_type_equiv(left, right);
  }
};

