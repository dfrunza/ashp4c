#pragma once
#include <map.h>

enum class PotentialTypeEnum : int {
  NONE = 0,
  SET,
  PRODUCT,
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

  void create(Arena* storage, int arity) {
    this->arity = arity;
    if (arity > 0) {
      members = storage->allocate<PotentialType*>(arity);
    }
  }

  PotentialType* get(int i)
  {
    assert(i > 0 && i < arity);
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

  static PotentialType* create(Arena* storage, enum PotentialTypeEnum kind)
  {
    PotentialType* potype = storage->allocate<PotentialType>();
    potype->kind = kind;
    if (potype->kind == PotentialTypeEnum::SET) {
      potype->set.members.storage = storage;
    } else if (potype->kind == PotentialTypeEnum::PRODUCT) {
      potype->product.members = 0;
      potype->product.arity = 0;
    }
    return potype;
  }
};
