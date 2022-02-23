#include "arena.h"
#include "ast.h"
#include "symtable.h"


void
resolve_nameref_type(struct Hashmap* nameref_table, struct UnboundedArray* type_names)
{
  for (int i = 0; i < type_names->elem_count; ++i) {
    struct NameRef* nameref = *(struct NameRef**)array_get(type_names, i);
    assert(nameref->kind == NAMEREF_TYPE);
    printf("%s:%d -> ", nameref->strname, nameref->line_nr, nameref->kind);
    struct SymtableEntry* symtable_entry = scope_lookup_name(nameref->scope, NAMESPACE_TYPE, nameref->strname);
    if (symtable_entry) {
      if (symtable_entry->ns_type) {
        struct NamedObject* descriptor = symtable_entry->ns_type;
        printf("%d\n", descriptor->line_nr);
        nameref->descriptor = descriptor;
      } else error("at line %d: name `%s` not found.", nameref->line_nr, nameref->strname);
    } else error("at line %d: name `%s` not found.", nameref->line_nr, nameref->strname);
  }
}

void
resolve_nameref_var(struct Hashmap* nameref_table, struct UnboundedArray* var_names)
{
  for (int i = 0; i < var_names->elem_count; ++i) {
    struct NameRef* nameref = *(struct NameRef**)array_get(var_names, i);
    assert(nameref->kind == NAMEREF_VAR);
    printf("%s:%d -> ", nameref->strname, nameref->line_nr, nameref->kind);
    struct SymtableEntry* symtable_entry = scope_lookup_name(nameref->scope, NAMESPACE_VAR, nameref->strname);
    if (symtable_entry) {
      if (symtable_entry->ns_var) {
        struct NamedObject* descriptor = symtable_entry->ns_var;
        printf("%d\n", descriptor->line_nr);
        nameref->descriptor = descriptor;
      } else error("at line %d: name `%s` not found.", nameref->line_nr, nameref->strname);
    } else error("at line %d: name `%s` not found.", nameref->line_nr, nameref->strname);
  }
}
