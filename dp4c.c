#include "dp4c.h"
#include "lex.h"
#include "syntax.h"

Arena arena = {};
char* input_text = 0;
int input_size = 0;
Token* tokenized_input = 0;
int tokenized_input_len = 0;
int max_tokenized_input_len = 1000;
int max_symtab_len = 997;
NamespaceInfo** symtab = 0;
int scope_level = 0;
uint64_t* typetable = 0;
int typetable_len = 0;
int max_typetable_len = 1000;
Ast_P4Program* p4program = 0;

internal void
read_input(char* filename)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  input_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  input_text = arena_push_array(&arena, char, input_size + 1);
  fread(input_text, sizeof(char), input_size, f_stream);
  input_text[input_size] = '\0';
  fclose(f_stream);
  arena_print_usage(&arena, "Memory (read_input): ");
}

int
main(int arg_count, char* args[])
{
  arena_new(&arena, 128*KILOBYTE);

  tokenized_input = arena_push_array(&arena, Token, max_tokenized_input_len);
  symtab = arena_push_array(&arena, NamespaceInfo*, max_symtab_len);
  int i = 0;
  while (i < max_symtab_len)
    symtab[i++] = 0;
  typetable = arena_push_array(&arena, uint64_t, max_typetable_len);

  read_input("test.p4");
  lex_input_init(input_text);
  lex_tokenize_input();
  sym_init();
  syn_parse();
  return 0;
}

