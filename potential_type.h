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

  void create(Arena* storage)
  {
    members.storage = storage;
  }

  void add(Type* ty) {
    members.insert(ty, 0, 0);
  };
};

struct PotentialType_Product {
  PotentialType** members;
  int arity;

  void create(Arena* storage, int arity) {
    this->arity = arity;
    members = 0;
    if (arity > 0) {
      members = storage->allocate<PotentialType*>(arity);
    }
  }

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

  static PotentialType* create(Arena* storage, enum PotentialTypeEnum kind)
  {
    PotentialType* potype = storage->allocate<PotentialType>();
    potype->kind = kind;
    return potype;
  }
};
