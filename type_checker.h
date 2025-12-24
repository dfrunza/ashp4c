#pragma once
#include <array.h>
#include <type.h>
#include <potential_type.h>

struct TypeChecker {
  Array<Type>* type_equiv_pairs;

  void init(Arena* storage)
  {
    type_equiv_pairs = Array<Type>::create(storage, 2);
  }

  bool match_type(PotentialType_Set* potential_types, Type* required_ty)
  {
    assert(potential_types->kind == PotentialTypeEnum::Set);

    int i = 0;
    for (MapEntry<Type, void>* m = potential_types->members.first; m != 0; m = m->next) {
      Type* ty = m->key->effective_type();
      if (type_equiv(ty, required_ty->actual_type())) {
        i += 1;
      }
    }
    return (i == 1);
  }

  bool match_params(PotentialType_Product* potential_args, Type* params_ty)
  {
    assert(potential_args->kind == PotentialTypeEnum::Product);

    int i = 0;
    if (params_ty->product.count != potential_args->arity) return 0;
    for (i = 0; i < params_ty->product.count; i++) {
      if (!match_type((PotentialType_Set*)potential_args->members[i],
                      params_ty->product.members[i])) break;
    }
    return (i == params_ty->product.count);
  }

  void collect_matching_member(PotentialType_Set* tau, Type* product_ty,
        char* strname, PotentialType_Product* potential_args)
  {
    assert(tau->kind == PotentialTypeEnum::Set);
    if (potential_args) { // FIXME
      assert(potential_args->kind == PotentialTypeEnum::Product);
    }

    for (int i = 0; i < product_ty->product.count; i++) {
      Type* member_ty = product_ty->product.members[i];
      if (cstring::match(member_ty->strname, strname)) {
        if (member_ty->kind == TypeEnum::Function) {
          if (match_params(potential_args, member_ty->function.params)) {
            tau->members.insert(member_ty, 0, 1);
          }
        } else {
          tau->members.insert(member_ty, 0, 1);
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

    for (i = 0; i < type_equiv_pairs->element_count; i++) {
      type_pair = type_equiv_pairs->get(i);
      assert(type_pair->kind == TypeEnum::Tuple);
      if ((left == type_pair->tuple.left || left == type_pair->tuple.right) &&
          (right == type_pair->tuple.left || right == type_pair->tuple.right)) {
        return 1;
      }
    }

    type_pair = type_equiv_pairs->append();
    type_pair->kind = TypeEnum::Tuple;
    type_pair->tuple.left = left;
    type_pair->tuple.right = right;

    if (left->kind == TypeEnum::Void || left->kind == TypeEnum::String ||
        left->kind == TypeEnum::Bool || left->kind == TypeEnum::Int ||
        left->kind == TypeEnum::Bit || left->kind == TypeEnum::Varbit) {
      if (right->kind == left->kind) {
        return 1;
      }
      return 0;
    } else if (left->kind == TypeEnum::Any) {
      return 1;
    } else if (left->kind == TypeEnum::Enum) {
      if (right->kind == left->kind) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->kind == TypeEnum::Extern) {
      if (right->kind == left->kind) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->kind == TypeEnum::Table) {
      if (right->kind == left->kind) {
        return cstring::match(left->strname, right->strname);
      }
      return 0;
    } else if (left->kind == TypeEnum::Product) {
      if (right->kind == left->kind) {
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
    } else if (left->kind == TypeEnum::Function) {
      if (right->kind == left->kind) {
        if (!structural_type_equiv(left->function.return_, right->function.return_)) {
          return 0;
        }
        if (!structural_type_equiv(left->function.params, right->function.params)) {
          return 0;
        }
        return 1;
      }
      return 0;
    } else if (left->kind == TypeEnum::Package) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->package.params, right->package.params);
      }
      return 0;
    } else if (left->kind == TypeEnum::Parser) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->parser.params, right->parser.params);
      }
      return 0;
    } else if (left->kind == TypeEnum::Control) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->control.params, right->control.params);
      }
      return 0;
    } else if (left->kind == TypeEnum::Struct) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->struct_.fields, right->struct_.fields);
      }
      return 0;
    } else if (left->kind == TypeEnum::Header) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->struct_.fields, right->struct_.fields);
      }
      return 0;
    } else if (left->kind == TypeEnum::Stack) {
      if (right->kind == left->kind) {
        return structural_type_equiv(left->header_stack.element, right->header_stack.element);
      }
      return 0;
    } else assert(0);

    assert(0);
    return 0;
  }

  bool type_equiv(Type* left, Type* right)
  {
    this->type_equiv_pairs->element_count = 0;
    return structural_type_equiv(left, right);
  }
};

