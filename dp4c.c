#include "basic.h"
#include "arena.h"
#include "lex.h"
#include "build_cst.h"
#include "build_ast.h"
#include <sys/stat.h>

#define DEBUG_ENABLED 0

Arena arena = {};

struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
};

internal struct SourceText
read_source(char* filename)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  int input_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  char* input_text = arena_push_array(&arena, char, input_size + 1);
  fread(input_text, sizeof(char), input_size, f_stream);
  input_text[input_size] = '\0';
  fclose(f_stream);
  struct SourceText result = {
    .text = input_text,
    .size = input_size,  // char units, excluding NULL
  };
  return result;
}

internal struct CmdlineArg*
find_unnamed_arg(struct CmdlineArg* args)
{
  struct CmdlineArg* unnamed_arg = 0;
  struct CmdlineArg* arg = args;
  while (arg) {
    if (!arg->name) {
      unnamed_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return unnamed_arg;
}

internal struct CmdlineArg*
find_named_arg(char* name, struct CmdlineArg* args)
{
  struct CmdlineArg* named_arg = 0;
  struct CmdlineArg* arg = args;
  while (arg) {
    if (arg->name && cstr_match(name, arg->name)) {
      named_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return named_arg;
}

internal struct CmdlineArg*
parse_cmdline_args(int arg_count, char* args[])
{
  struct CmdlineArg* arg_list = 0;
  if (arg_count <= 1) {
    return arg_list;
  }
  
  struct CmdlineArg sentinel_arg = {};
  struct CmdlineArg* prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    struct CmdlineArg* cmdline_arg = arena_push_struct(&arena, CmdlineArg);
    zero_struct(cmdline_arg, CmdlineArg);
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
  arena_new(&arena, 192*KILOBYTE);

  struct CmdlineArg* cmdline_args = parse_cmdline_args(arg_count, args);
  struct CmdlineArg* filename_arg = find_unnamed_arg(cmdline_args);
  if (!filename_arg) {
    printf("<filename> argument is required.\n");
    exit(1);
  }
  struct SourceText source = read_source(filename_arg->value);
  if (DEBUG_ENABLED)
    arena_print_usage(&arena, "Memory (read_source): ");

  struct TokenSequence tksequence = lex_tokenize(&source);
  if (DEBUG_ENABLED)
    arena_print_usage(&arena, "Memory (lex): ");

  struct CstTree cst_tree = build_CstTree(&tksequence);
  struct Cst* cst_p4program = cst_tree.p4program;
  assert(cst_p4program->kind == Cst_P4Program);
  if (DEBUG_ENABLED)
    arena_print_usage(&arena, "Memory (syntax): ");

  if (find_named_arg("dump-cst", cmdline_args)) {
    dump_P4Program((struct Cst_P4Program*)cst_p4program);
  }

  struct Ast* ast_p4program = build_AstP4Program((struct Cst_P4Program*)cst_p4program);
  assert(ast_p4program->kind == Ast_P4Program);
  return 0;
}

