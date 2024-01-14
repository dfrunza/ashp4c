#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Hashmap* type_table;

void
resolve_idref_type(Hashmap* type_table_)
{
  type_table = type_table_;
}
