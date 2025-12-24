#pragma once
#include <map.h>
#include <type.h>

enum class PotentialTypeEnum : int {
  None = 0,
  Set,
  Product,
};

struct PotentialType {
  enum PotentialTypeEnum kind;
  size_t size;
};

struct PotentialType_Set: PotentialType {
  Map<Type, void> members;

  static PotentialType_Set* create(Arena* storage)
  {
    PotentialType_Set* object = storage->allocate<PotentialType_Set>();
    object->kind = PotentialTypeEnum::Set;
    object->size = sizeof(PotentialType_Set);
    object->members.storage = storage;
    return object;
  }

  void add(Type* ty) {
    members.insert(ty, 0, 0);
  };
};

struct PotentialType_Product: PotentialType {
  PotentialType** members;
  int arity;

  static PotentialType_Product* create(Arena* storage, int arity) {
    PotentialType_Product* object = storage->allocate<PotentialType_Product>();
    object->kind = PotentialTypeEnum::Product;
    object->size = sizeof(PotentialType_Product);
    object->arity = arity;
    object->members = 0;
    if (arity > 0) {
      object->members = storage->allocate<PotentialType*>(arity);
    }
    return object;
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
