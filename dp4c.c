#include "dp4c.h"
#include "syntax.h"
#include "symtab.h"

Arena arena = {};
Token token_at = {};
char* input_text = 0;
uint32_t input_size = 0;
int symtab_len = 997;
NamespaceInfo** symtab = 0;
int scope_level = 0;
int typetable_len = 1000;
uint64_t* typetable = 0;

internal char*
read_input(char* filename)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  uint32_t f_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  char* data = arena_push_array(&arena, char, f_size + 1);
  fread(data, sizeof(char), f_size, f_stream);
  data[f_size] = '\0';
  input_size = f_size;
  fclose(f_stream);
  arena_print_usage(&arena, "Main arena (read_input): ");
  return data;
}

int
main(int arg_count, char* args[])
{
  arena_new(&arena, 128*KILOBYTE);

  symtab = arena_push_array(&arena, NamespaceInfo*, symtab_len);
  int i = 0;
  while (i < symtab_len)
    symtab[i++] = 0;
  typetable = arena_push_array(&arena, uint64_t, typetable_len);

  input_text = read_input("test.p4");
  sym_init();
  lex_lexeme_init(input_text);
  Ast_P4Program* p4program = syn_parse();
  return 0;
}
