#pragma once

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
      struct PotentialType** members;
      int count;
    } product;
  };
};
