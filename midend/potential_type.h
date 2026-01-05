#pragma once
#include "adt/map.h"
#include "frontend/type.h"

enum class PotentialTypeEnum : int {
  None = 0,
  Set,
  Product,
};

struct PotentialType;

struct PotentialType_Set {
  Map members;

  static PotentialType* allocate(Arena* storage);
  void add(Type* ty);
};

struct PotentialType_Product {
  PotentialType** members;
  int arity;

  static PotentialType* allocate(Arena* storage, int arity);
  PotentialType* get(int i);
  void set(int i, PotentialType* m);
};

struct PotentialType {
  enum PotentialTypeEnum kind;

  union {
    PotentialType_Set set;
    PotentialType_Product product;
  };
};
