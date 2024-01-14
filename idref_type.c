#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Hashmap*        type_table;
static UnboundedArray* type_array;

void
resolve_idref_type(Hashmap* type_table_, UnboundedArray* type_array_)
{
  Type* idref_ty, *ty;
  HashmapKey hkey;

  type_table = type_table_;
  type_array = type_array_;

  for (int i = 0; i < type_array->elem_count; i++) {
    idref_ty = (Type*)array_get_elem(type_array, i, sizeof(Type));
    if (idref_ty->ctor == TYPE_IDREF) {
      hkey.u64_key = (uint64_t)idref_ty->idref.ref;
      ty = *(Type**)hashmap_lookup_entry(type_table, &hkey, HKEY_UINT64)->value;
      idref_ty->ctor = TYPE_PROXY;
      idref_ty->proxy.type = ty;
    }
  }
}
