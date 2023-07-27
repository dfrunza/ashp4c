#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include <stdarg.h>   /* va_list, va_start, va_end */
#include "foundation.h"
#include "frontend.h"
#include "ashp4c.h"

static Arena main_storage = {};

typedef struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
} CmdlineArg;

static void
read_source(char** text_, int* text_size_, char* filename, Arena* text_storage)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  char* text = arena_malloc(text_storage, (text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  *text_ = text;
  *text_size_ = text_size;
}

static CmdlineArg*
find_unnamed_arg(CmdlineArg* args)
{
  CmdlineArg* unnamed_arg = 0;
  CmdlineArg* arg = args;
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
static CmdlineArg*
find_named_arg(char* name, CmdlineArg* args)
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

static CmdlineArg*
parse_cmdline_args(int arg_count, char* args[])
{
  CmdlineArg* arg_list = 0;
  if (arg_count <= 1) {
    return arg_list;
  }
  
  CmdlineArg sentinel_arg = {};
  CmdlineArg* prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    CmdlineArg* cmdline_arg = arena_malloc(&main_storage, sizeof(*cmdline_arg));
    if (cstr_start_with(args[i], "--")) {
      char* raw_arg = args[i] + 2;  /* skip the `--` prefix */
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

int
main(int arg_count, char* args[])
{
  reserve_page_memory(250*KILOBYTE);

  CmdlineArg* cmdline_args = parse_cmdline_args(arg_count, args);
  CmdlineArg* filename_arg = find_unnamed_arg(cmdline_args);
  if (!filename_arg) {
    printf("<filename> is required.\n");
    exit(1);
  }
  Arena text_storage = {};
  char* text = 0;
  int text_size = 0;
  read_source(&text, &text_size, filename_arg->value, &text_storage);
  UnboundedArray* tokens = tokenize_text(text, text_size, &main_storage, &text_storage);

  Scope* root_scope;
  Ast_P4Program* p4program = parse_tokens(tokens, &main_storage, &root_scope);
  assert(p4program->kind == AST_p4program);
  arena_free(&text_storage);

  pass_dry(p4program, root_scope);
  pass_name_decl(p4program, &main_storage, root_scope);
  Hashmap* type_table = pass_type_decl(p4program, &main_storage, root_scope);
  pass_potential_type(p4program, &main_storage, root_scope, type_table);

  /*
  select_type(p4program, root_scope, potential_type, &main_storage); */

  arena_free(&main_storage);
  return 0;
}

