#pragma once
#include <map.h>
#include <type.h>

enum class PotentialTypeEnum : int {
  None = 0,
  Set,
  Product,
};

struct PotentialType;

struct PotentialType_Set {
  Map<Type, void> members;

  void add(Type* ty) {
    members.insert(ty, 0, 0);
  };
};

struct PotentialType_Product {
  PotentialType** members;
  int arity;

  PotentialType* get(int i)
  {
    assert(i >= 0 && i < arity);
    return members[i];
  }

  void set(int i, PotentialType* m)
  {
    assert(i >= 0 && i < arity);
    members[i] = m;
  }
};

struct PotentialType {
  enum PotentialTypeEnum kind;

  union {
    PotentialType_Set set;
    PotentialType_Product product;
  };

  static PotentialType* create_set(Arena* storage)
  {
    PotentialType* potype = storage->allocate<PotentialType>();
    potype->kind = PotentialTypeEnum::Set;
    potype->set.members.storage = storage;
    return potype;
  }

  static PotentialType* create_product(Arena* storage, int arity)
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
};
