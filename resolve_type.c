#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static void
resolve_TYPE_IDREF(Set* type_table, UnboundedArray* type_array)
{
  Type* ref_ty, *ty;

  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_element(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_IDREF) {
      ref_ty = lookup_type_table(type_table, ty->idref.ref);
      assert(ref_ty);
      ty->ctor = TYPE_TYPE;
      ty->type.type = ref_ty;
    }
  }
}

static void
resolve_TYPE_NAMEREF(Set* type_table, UnboundedArray* type_array)
{
  Ast* name;
  Type* ref_ty, *ty;
  NameEntry* name_entry;
  NameDecl* name_decl;

  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_element(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_NAMEREF) {
      name = ty->nameref.name;
      name_entry = scope_lookup_namespace(ty->nameref.scope, name->name.strname, NS_TYPE);
      if (name_entry && name_entry->ns[NS_TYPE]) {
        name_decl = name_entry->ns[NS_TYPE];
        if (!name_decl->next_in_scope) {
          ref_ty = lookup_type_table(type_table, name_decl->ast);
          assert(ref_ty);
          name_decl->type = ref_ty;
          ty->ctor = TYPE_TYPE;
          ty->type.type = ref_ty;
        } else error("At line %d, column %d: ambiguous type reference `%s`.",
                     name->line_no, name->column_no, name->name.strname);
      } else error("At line %d, column %d: unresolved type reference `%s`.",
                   name->line_no, name->column_no, name->name.strname);
    }
  }
}

void
resolve_TYPE_TYPE(UnboundedArray* type_array)
{
  Type* ref_ty, *ty;

  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_element(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_TYPE) {
      ref_ty = ty->type.type;
      while (ref_ty->ctor == TYPE_TYPE) {
        ref_ty = ref_ty->type.type;
      }
      ty->type.type = ref_ty;
    }
  }
}

void
resolve_type_xref(Set* type_table, UnboundedArray* type_array)
{
  resolve_TYPE_IDREF(type_table, type_array);
  resolve_TYPE_NAMEREF(type_table, type_array);
  resolve_TYPE_TYPE(type_array);

#if 0
  /* Test */
  Type* ty;
  for (int i = 0; i < type_array->elem_count; i++) {
    ty = (Type*)array_get_element(type_array, i, sizeof(Type));
    if (ty->ctor == TYPE_TYPE) {
      ty = ty->type.type;
      assert(ty->ctor != TYPE_TYPE);
    }
  }
#endif
}

