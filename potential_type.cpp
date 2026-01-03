#include <potential_type.h>

PotentialType* PotentialType_Set::create(Arena* storage)
{
  PotentialType* potype = storage->allocate<PotentialType>();
  potype->kind = PotentialTypeEnum::Set;
  potype->set.members.storage = storage;
  return potype;
}

PotentialType* PotentialType_Product::create(Arena* storage, int arity)
{
  PotentialType* potype = storage->allocate<PotentialType>();
  potype->kind = PotentialTypeEnum::Product;
  potype->product.arity = arity;
  potype->product.members = 0;
  if (potype->product.arity > 0) {
    potype->product.members = storage->allocate<PotentialType*>(arity);
  }
  return potype;
}
