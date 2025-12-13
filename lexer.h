#pragma once
#include <arena.h>
#include <array.h>
#include <token.h>

struct SourceText {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;

  void read_source(char* filename)
  {
    FILE* f_stream = fopen(filename, "rb");
    if (!f_stream) {
      error("Could not open file '%s'.", filename);
    }
    fseek(f_stream, 0, SEEK_END);
    int text_size = ftell(f_stream);
    fseek(f_stream, 0, SEEK_SET);
    char* text = storage->allocate<char>(text_size + 1);
    fread(text, sizeof(char), text_size, f_stream);
    text[text_size] = '\0';
    fclose(f_stream);
    this->text = text;
    this->text_size = text_size;
    this->filename = filename;
  }
};

struct Lexeme {
  char* start;
  char* end;

  void copy_to(char* dest)
  {
    char* src = start;
    do {
      if (*src == '\\') {
        src++;
        if (*src == 'n') {
          *dest++ = '\n';
          src++;
        } else if (*src == 'r') {
          *dest++ = '\r';
          src++;
        } else if (*src == 't') {
          *dest++ = '\t';
          src++;
        } else {
          *dest++ = *src++;
        }
      } else {
        *dest++ = *src++;
      }
    }
    while (src <= end);
  }

  int len()
  {
    int result = end - start + 1;
    return result;
  }

  char* to_cstring(Arena* storage)
  {
    int len = this->len();
    char* string = storage->allocate<char>(len + 1);  // +1 the NULL terminator
    copy_to(string);
    string[len] = '\0';
    return string;
  }
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
  Array<Token>* tokens;

  char lookahead_char(int pos)
  {
    char* char_pos = lexeme->end + pos;
    assert(char_pos >= (char*)0 && char_pos <= (text + text_size));
    return *char_pos;
  }

  char advance_char(int pos)
  {
    char* char_pos = lexeme->end + pos;
    assert(char_pos >= (char*)0 && char_pos <= (text + text_size));
    lexeme->end = char_pos;
    return *char_pos;
  }

  char retract_char()
  {
    char result = *(--lexeme->end);
    assert(lexeme->end >= (char*)0);
    return result;
  }

  void advance_lexeme()
  {
    lexeme->start = ++lexeme->end;
    assert(lexeme->start <= (text + text_size));
  }

  void to_integer_token(Token* token, Lexeme* lexeme, int base);
  void next_token(Token* token);
  void tokenize(SourceText* source_text);
};
