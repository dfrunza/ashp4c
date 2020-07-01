#include "basic.h"
#include "arena.h"
#include "lex.h"
#include "syntax.h"
#include "symtab.h"

int
main(int arg_count, char* args[])
{
  Arena main_arena = {};
  arena_new(&main_arena, 128*KILOBYTE);
  sym_init(&main_arena);
  lex_init(&main_arena, "test.p4");
  syn_init(&main_arena);
  Ast_P4Program* p4program = syn_parse();
  return 0;
}
