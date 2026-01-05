#include "potential_type.h"

PotentialType* PotentialType::create(Arena* storage, enum PotentialTypeEnum kind)
{
  PotentialType* potype = (PotentialType*)storage->allocate(sizeof(PotentialType), 1);
  potype->kind = kind;
  return potype;
}

void PotentialType_Set::init(Arena* storage)
{
  members.storage = storage;
}

void PotentialType_Set::add(Type* ty) {
  members.insert(ty, 0, 0);
};

void PotentialType_Product::init(Arena* storage, int arity)
{
  this->arity = arity;
  members = 0;
  if (arity > 0) {
    members = (PotentialType**)storage->allocate(sizeof(PotentialType*), arity);
  }
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
