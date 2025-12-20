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
    return potype;
  }
};
