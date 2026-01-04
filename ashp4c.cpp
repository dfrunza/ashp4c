#include <stdio.h>
#include <stdlib.h>
#include "basic.h"
#include "command_line.h"
#include "lexer.h"
#include "parser.h"
#include "passes/drypass.h"
#include "passes/builtin_methods.h"
#include "passes/scope_hierarchy.h"
#include "passes/name_binding.h"
#include "passes/declared_type.h"
#include "passes/potential_type.h"
#include "passes/select_type.h"

int main(int arg_count, char* args[])
{
  Arena storage = {}, scratch_storage = {};

  Memory::reserve(500 * KILOBYTE);

  CommandLineArg* cmdline_arg = CommandLineArg::parse_cmdline(&storage, arg_count, args);
  CommandLineArg* filename = cmdline_arg->find_unnamed_arg();
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  SourceText source_text = {};
  source_text.storage = &scratch_storage;
  source_text.read_source(filename->value);

  Lexer lexer = {};
  lexer.storage = &storage;
  lexer.tokenize(&source_text);

  Parser parser = {};
  parser.storage = &storage;
  parser.source_file = source_text.filename;
  parser.tokens = lexer.tokens;
  parser.parse();
  scratch_storage.free();

  DryPass drypass = {};
  drypass.do_pass(parser.p4program);

  BuiltinMethodsPass builtin_methods = {};
  builtin_methods.storage = &storage;
  builtin_methods.do_pass(parser.p4program);

  ScopeHierarchyPass scope_hierarchy = {};
  scope_hierarchy.storage = &storage;
  scope_hierarchy.root_scope = parser.root_scope;
  scope_hierarchy.p4program = parser.p4program;
  scope_hierarchy.do_pass();

  NameBindingPass name_binding = {};
  name_binding.storage = &storage;
  name_binding.p4program = parser.p4program;
  name_binding.root_scope = scope_hierarchy.root_scope;
  name_binding.scope_map = scope_hierarchy.scope_map;
  name_binding.do_pass();

  DeclaredTypePass declared_types = {};
  declared_types.storage = &storage;
  declared_types.source_file = source_text.filename;
  declared_types.p4program = parser.p4program;
  declared_types.root_scope = parser.root_scope;
  declared_types.scope_map = scope_hierarchy.scope_map;
  declared_types.decl_map = name_binding.decl_map;
  declared_types.type_array = name_binding.type_array;
  declared_types.do_pass();

  TypeChecker type_checker = {};
  type_checker.init(&storage);

  PotentialTypePass potential_types = {};
  potential_types.storage = &storage;
  potential_types.source_file = source_text.filename;
  potential_types.p4program = parser.p4program;
  potential_types.root_scope = parser.root_scope;
  potential_types.scope_map = scope_hierarchy.scope_map;
  potential_types.decl_map = name_binding.decl_map;
  potential_types.type_array = name_binding.type_array;
  potential_types.type_env = declared_types.type_env;
  potential_types.type_checker = &type_checker;
  potential_types.do_pass();

  SelectTypePass select_type = {};
  select_type.storage = &storage;
  select_type.source_file = source_text.filename;
  select_type.p4program = parser.p4program;
  select_type.root_scope = parser.root_scope;
  select_type.type_array = name_binding.type_array;
  select_type.scope_map = scope_hierarchy.scope_map;
  select_type.decl_map = name_binding.decl_map;
  select_type.type_env = declared_types.type_env;
  select_type.potype_map = potential_types.potype_map;
  select_type.type_checker = &type_checker;
  select_type.do_pass();

  storage.free();
  return 0;
}
