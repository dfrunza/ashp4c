#include <stdio.h>
#include <stdlib.h>
#include "basic.h"
#include "cstring.h"
#include "command_line.cpp"
#include "ashp4c.h"

void SourceText::read_source(char* filename)
{
  FILE* f_stream;
  char* text;

  f_stream = fopen(filename, "rb");
  if (!f_stream) {
    error("Could not open file '%s'.", filename);
  }
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  text = (char*)storage->malloc((text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  this->text = text;
  this->text_size = text_size;
  this->filename = filename;
}

int main(int arg_count, char* args[])
{
  CommandLineArg* cmdline_arg, *filename;
  Arena storage = {}, scratch_storage = {};
  SourceText source_text = {};
  Lexer lexer = {};
  Parser parser = {};
  TypeChecker type_checker = {};
  DryPass drypass = {};
  BuiltinMethodsPass builtin_methods = {};
  ScopeHierarchyPass scope_hierarchy = {};
  NameBindingPass name_binding = {};
  DeclaredTypesPass declared_types = {};
  PotentialTypesPass potential_types = {};
  SelectTypePass select_type = {};

  Arena::reserve_memory(500*KILOBYTE);

  cmdline_arg = CommandLineArg::parse_cmdline_args(&storage, arg_count, args);
  filename = find_unnamed_arg(cmdline_arg);
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  source_text.storage = &scratch_storage;
  source_text.read_source(filename->value);

  lexer.storage = &storage;
  lexer.tokenize(&source_text);

  parser.storage = &storage;
  parser.source_file = source_text.filename;
  parser.tokens = lexer.tokens;
  parser.parse();
  scratch_storage.free();

  drypass.do_pass(parser.p4program);

  builtin_methods.storage = &storage;
  builtin_methods.do_pass(parser.p4program);

  scope_hierarchy.storage = &storage;
  scope_hierarchy.root_scope = parser.root_scope;
  scope_hierarchy.p4program = parser.p4program;
  scope_hierarchy.do_pass();

  name_binding.storage = &storage;
  name_binding.p4program = parser.p4program;
  name_binding.root_scope = scope_hierarchy.root_scope;
  name_binding.scope_map = scope_hierarchy.scope_map;
  name_binding.do_pass();

  declared_types.storage = &storage;
  declared_types.source_file = source_text.filename;
  declared_types.p4program = parser.p4program;
  declared_types.root_scope = parser.root_scope;
  declared_types.scope_map = scope_hierarchy.scope_map;
  declared_types.decl_map = name_binding.decl_map;
  declared_types.type_array = name_binding.type_array;
  declared_types.do_pass();

  type_checker.type_equiv_pairs = declared_types.type_equiv_pairs;

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

