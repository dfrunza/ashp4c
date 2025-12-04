#pragma once
#include <arena.h>
#include <array.h>
#include <token.h>

struct SourceText {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;

  void read_source(char* filename);
};

struct Lexeme {
  char* start;
  char* end;

  void copy_to(char* dest);
  int len();
  char* to_cstring(Arena* storage);
};

struct Lexer {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;
  int line_no;
  char* line_start;
  int state;
  Token token;
  Lexeme lexeme[2];
  Array* tokens;

  char lookahead_char(int pos);
  char advance_char(int pos);
  char retract_char();
  void advance_lexeme();
  void token_install_integer(Token* token, Lexeme* lexeme, int base);
  void next_token(Token* token);
  void tokenize(SourceText* source_text);
};
