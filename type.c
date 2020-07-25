#include "type.h"

internal Arena* arena = 0;
internal int typetable_len = 1000;
internal uint64_t* typetable = 0;

void
typ_init(Arena* arena_)
{
  arena = arena_;
  typetable = arena_push_array(arena, uint64_t, typetable_len);
}

