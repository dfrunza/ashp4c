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
      Map members;
    } set;

    struct {
      PotentialType** members;
      int count;
    } product;
  };
};
