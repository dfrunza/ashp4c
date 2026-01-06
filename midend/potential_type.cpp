#include "midend/potential_type.h"

PotentialType* PotentialType_Set::allocate(Arena* storage)
{
  PotentialType* potype = (PotentialType*)storage->allocate(sizeof(PotentialType), 1);
  potype->kind = PotentialTypeEnum::Set;
  potype->set.members.storage = storage;
  return potype;
}

void PotentialType_Set::add(Type* ty) {
  members.insert(ty, 0, 0);
};

PotentialType* PotentialType_Product::allocate(Arena* storage, int arity)
{
  PotentialType* potype = (PotentialType*)storage->allocate(sizeof(PotentialType), 1);
  potype->kind = PotentialTypeEnum::Product;
  potype->product.arity = arity;
  potype->product.members = 0;
  if (arity > 0) {
    potype->product.members = (PotentialType**)storage->allocate(sizeof(PotentialType*), arity);
  }
  return potype;
}

PotentialType* PotentialType_Product::get(int i)
{
  assert(i >= 0 && i < arity);
  return members[i];
}

void PotentialType_Product::set(int i, PotentialType* m)
{
  assert(i >= 0 && i < arity);
  members[i] = m;
}
