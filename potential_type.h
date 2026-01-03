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

  static PotentialType* create(Arena* storage);

  void add(Type* ty) {
    members.insert(ty, 0, 0);
  };
};

struct PotentialType_Product {
  PotentialType** members;
  int arity;

  static PotentialType* create(Arena* storage, int arity);

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
};
