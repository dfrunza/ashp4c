#pragma once
#include <map.h>

enum class PotentialTypeEnum : int {
  NONE = 0,
  SET,
  PRODUCT,
};

struct PotentialType {
  enum PotentialTypeEnum kind;

  union {
    struct {
      Map<Type, void> members;

      void add(Type* ty) {
        members.insert(ty, 0, 0);
      };
    } set;

    struct {
      PotentialType** members;
      int count;
    } product;
  };

  static PotentialType* create(Arena* storage, enum PotentialTypeEnum kind)
  {
    PotentialType* potype = storage->allocate<PotentialType>();
    potype->kind = kind;
    if (potype->kind == PotentialTypeEnum::SET) {
      potype->set.members.storage = storage;
    }
    return potype;
  }
};
