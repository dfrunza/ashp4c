#include <cstring.h>
#include <type_checker.h>

bool TypeChecker::match_type(PotentialType* potential_types, Type* required_ty)
{
  Type* ty;
  MapEntry* m;
  int i;

  i = 0;
  for (m = potential_types->set.members.first; m != 0; m = m->next) {
    ty = ((Type*)m->key)->effective_type();
    if (((TypeChecker*)this)->type_equiv(ty, required_ty->actual_type())) {
      i += 1;
    }
  }
  return (i == 1);
}

bool TypeChecker::match_params(PotentialType* potential_args, Type* params_ty)
{
  int i;

  if (params_ty->product.count != potential_args->product.count) return 0;
  for (i = 0; i < params_ty->product.count; i++) {
    if (!match_type(potential_args->product.members[i],
                    params_ty->product.members[i])) break;
  }
  return (i == params_ty->product.count);
}

void TypeChecker::collect_matching_member(PotentialType* tau, Type* product_ty,
                                          char* strname, PotentialType* potential_args)
{
  Type* member_ty;

  for (int i = 0; i < product_ty->product.count; i++) {
    member_ty = product_ty->product.members[i];
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

