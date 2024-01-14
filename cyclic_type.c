#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

void
resolve_type_idref(Hashmap* type_table, UnboundedArray* type_array)
{
  Type* ref_ty, *ty;

  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_elem(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_IDREF) {
      ref_ty = lookup_type_table(type_table, ty->idref.ref);
      assert(ref_ty);
      ty->ctor = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
}

void
resolve_type_nameref(Hashmap* type_table, UnboundedArray* type_array)
{
  Ast* name;
  Type* ref_ty, *ty;
  NameEntry* name_entry;
  NameDecl* name_decl;

  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_elem(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_NAMEREF) {
      name = ty->nameref.name;
      name_entry = scope_lookup_namespace(ty->nameref.scope, name->name.strname, NS_TYPE);
      if (name_entry && name_entry->ns[NS_TYPE]) {
        name_decl = name_entry->ns[NS_TYPE];
        ref_ty = lookup_type_table(type_table, name_decl->ast);
        assert(ref_ty);
        name_decl->type = ref_ty;
        ty->ctor = TYPE_TYPE;
        ty->type.type = ref_ty;
      } else error("Ar line %d, column %d: unresolved type `%s`.",
                   name->line_no, name->column_no, name->name.strname);
    }
  }
}

void
detect_type_cycle(UnboundedArray* type_array)
{

}
