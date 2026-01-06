#include "midend/potential_type.h"

PotentialType* PotentialType_Set::allocate(Arena* storage)
{
  PotentialType* po_type = (PotentialType*)storage->allocate(sizeof(PotentialType), 1);
  po_type->kind = PotentialTypeEnum::Set;
  po_type->set.members.storage = storage;
  return po_type;
}

void PotentialType_Set::add(Type* ty) {
  members.insert(ty, 0, 0);
};

PotentialType* PotentialType_Product::allocate(Arena* storage, int arity)
{
  PotentialType* po_type = (PotentialType*)storage->allocate(sizeof(PotentialType), 1);
  po_type->kind = PotentialTypeEnum::Product;
  po_type->product.arity = arity;
  po_type->product.members = 0;
  if (arity > 0) {
    po_type->product.members = (PotentialType**)storage->allocate(sizeof(PotentialType*), arity);
  }
  return po_type;
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
