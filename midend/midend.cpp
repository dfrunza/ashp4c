#include "midend/midend.h"

void Midend::do_analysis(Arena* storage, Arena* scratch,
       SourceText* source_text, Frontend* frontend) {
  type_checker.allocate(storage);

  builtin_methods.storage = storage;
  builtin_methods.p4program = frontend->p4program;
  builtin_methods.do_pass();

  scope_hierarchy.storage = storage;
  scope_hierarchy.p4program = frontend->p4program;
  scope_hierarchy.root_scope = frontend->root_scope;
  scope_hierarchy.do_pass();
  scope_map = scope_hierarchy.scope_map;

  name_binding.storage = storage;
  name_binding.p4program = frontend->p4program;
  name_binding.root_scope = frontend->root_scope;
  name_binding.scope_map = scope_map;
  name_binding.do_pass();
  decl_map = name_binding.decl_map;
  type_array = name_binding.type_array;

  declared_types.storage = storage;
  declared_types.source_file = source_text->filename;
  declared_types.p4program = frontend->p4program;
  declared_types.root_scope = frontend->root_scope;
  declared_types.scope_map = scope_map;
  declared_types.decl_map = decl_map;
  declared_types.type_array = type_array;
  declared_types.do_pass();
  type_env = declared_types.type_env;

  potential_types.storage = storage;
  potential_types.source_file = source_text->filename;
  potential_types.p4program = frontend->p4program;
  potential_types.root_scope = frontend->root_scope;
  potential_types.scope_map = scope_map;
  potential_types.type_env = type_env;
  potential_types.type_checker = &type_checker;
  potential_types.do_pass();
  po_type_map = potential_types.po_type_map;

  select_type.storage = storage;
  select_type.source_file = source_text->filename;
  select_type.p4program = frontend->p4program;
  select_type.root_scope = frontend->root_scope;
  select_type.scope_map = scope_map;
  select_type.type_array = type_array;
  select_type.type_env = type_env;
  select_type.po_type_map = po_type_map;
  select_type.type_checker = &type_checker;
  select_type.do_pass();

  scratch->free();
}