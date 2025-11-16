#include <stdio.h>
#include <stdlib.h>
#include "ashp4c.h"

struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
};

static void read_source(SourceText* source_text, char* filename)
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
  text = (char*)source_text->storage->malloc((text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  source_text->text = text;
  source_text->text_size = text_size;
  source_text->filename = filename;
}

static CmdlineArg* find_unnamed_arg(CmdlineArg* args)
{
  CmdlineArg* unnamed_arg = 0;
  CmdlineArg* arg;
  
  arg = args;
  while (arg) {
    if (!arg->name) {
      unnamed_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return unnamed_arg;
}

#if 0
static CmdlineArg* find_named_arg(char* name, CmdlineArg* args)
{
  CmdlineArg* named_arg = 0;
  CmdlineArg* arg = args;
  while (arg) {
    if (arg->name && cstr_match(name, arg->name)) {
      named_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return named_arg;
}
#endif

static CmdlineArg* parse_cmdline_args(Arena* storage, int arg_count, char* args[])
{
  CmdlineArg* arg_list = 0;
  CmdlineArg *prev_arg, *cmdline_arg;
  CmdlineArg sentinel_arg = {};
  char* raw_arg;

  if (arg_count <= 1) {
    return arg_list;
  }
  prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    cmdline_arg = (CmdlineArg*)storage->malloc(sizeof(CmdlineArg));
    if (cstr_start_with(args[i], "-")) {
      raw_arg = args[i] + 1;  /* skip the `-` prefix */
      cmdline_arg->name = raw_arg;
    } else {
      cmdline_arg->value = args[i];
    }
    prev_arg->next_arg = cmdline_arg;
    prev_arg = cmdline_arg;
    i += 1;
  }
  arg_list = sentinel_arg.next_arg;
  return arg_list;
}

int main(int arg_count, char* args[])
{
  CmdlineArg* cmdline_arg, *filename;
  Arena storage = {}, scratch_storage = {};
  SourceText source_text = {};
  Lexer lexer = {};
  Parser parser = {};
  DryPass drypass = {};
  BuiltinMethodBuilder method_builder = {};
  ScopeBuilder scope_builder = {};
  NameBinder name_binder = {};
  TypeChecker type_checker = {};
  DeclaredTypesPass declared_types = {};
  PotentialTypesPass potential_types = {};

  Arena::reserve_memory(500*KILOBYTE);

  cmdline_arg = parse_cmdline_args(&storage, arg_count, args);
  filename = find_unnamed_arg(cmdline_arg);
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  source_text.storage = &scratch_storage;
  read_source(&source_text, filename->value);

  lexer.storage = &storage;
  lexer.tokenize(&source_text);

  parser.storage = &storage;
  parser.source_file = source_text.filename;
  parser.tokens = lexer.tokens;
  parser.parse();
  scratch_storage.free();

  drypass.do_pass(parser.p4program);

  method_builder.storage = &storage;
  builtin_methods(&method_builder, parser.p4program);

  scope_builder.storage = &storage;
  scope_builder.root_scope = parser.root_scope;
  scope_builder.p4program = parser.p4program;
  scope_builder.scope_hierarchy();

  name_binder.storage = &storage;
  name_binder.p4program = parser.p4program;
  name_binder.root_scope = scope_builder.root_scope;
  name_binder.scope_map = scope_builder.scope_map;
  name_binder.name_bind();

  type_checker.storage = &storage;
  type_checker.source_file = source_text.filename;
  type_checker.p4program = parser.p4program;
  type_checker.root_scope = parser.root_scope;
  type_checker.type_array = name_binder.type_array;
  type_checker.scope_map = scope_builder.scope_map;
  type_checker.decl_map = name_binder.decl_map;

  *(TypeChecker*)&declared_types = type_checker;
  declared_types.declared_types();
  type_checker = *(TypeChecker*)&declared_types;

  *(TypeChecker*)&potential_types = type_checker;
  potential_types.potential_types();
  type_checker = *(TypeChecker*)&potential_types;

  select_type(&type_checker);
  storage.free();

  return 0;
}

