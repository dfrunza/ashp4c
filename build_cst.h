#pragma once
#include "token.h"
#include "cst.h"

struct CstTree {
  struct Arena* arena;
  struct Cst* p4program;
  int node_count;
  int size_in_bytes;
};

struct CstTree build_CstTree(struct TokenSequence* tksequence);
