#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "arena.h"
#include "frontend.h"

typedef struct Lexeme {
  char* start;
  char* end;
} Lexeme;

internal Arena* lexeme_storage;
internal char* text;
internal int text_size;
internal Arena* tokens_storage;
internal UnboundedArray tokens_array = {};
internal int line_no;
internal char* line_start;
internal int state;
internal Lexeme lexeme[2];

internal char
char_lookahead(int pos)
{
  char* char_pos = lexeme->end + pos;
  assert(char_pos >= 0 && char_pos <= (text + text_size));
  return *char_pos;
}

internal char
char_advance(int pos)
{
  char* char_pos = lexeme->end + pos;
  assert(char_pos >= 0 && char_pos <= (text + text_size));
  lexeme->end = char_pos;
  return *char_pos;
}

internal char
char_retract()
{
  char result = *(--lexeme->end);
  assert(lexeme->end >= 0);
  return result;
}

internal void
lexeme_advance()
{
  lexeme->start = ++lexeme->end;
  assert (lexeme->start <= (text + text_size));
}

internal void
lexeme_copy(char* dest, Lexeme* lexeme)
{
  char* src = lexeme->start;
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
  while (src <= lexeme->end);
}

internal bool
lexeme_len(Lexeme* lexeme)
{
  int result = lexeme->end - lexeme->start + 1;
  return result;
}

internal bool
lexeme_match_cstr(Lexeme* lexeme, char* str)
{
  char* l = lexeme->start;
  char* s = str;
  while (*l == *s) {
    l++;
    s++;
    if (*s == '\0')
      break;
  }
  bool result = (s - str) == lexeme_len(lexeme) && (*s == '\0');
  return result;
}

internal char*
lexeme_to_cstring(Lexeme* lexeme)
{
  int len = lexeme_len(lexeme);
  char* string = arena_push(lexeme_storage, (len + 1)*sizeof(char));   // +1 the NULL terminator
  lexeme_copy(string, lexeme);
  string[len] = '\0';
  return string;
}

internal int
digit_to_integer(char c, int base)
{
  int digit_value = 0;
  if (base == 10 || base == 8 || base == 2) {
    digit_value = (int)(c - '0');
  } else if (base == 16) {
    if ('0' <= c && c <= '9') {
      digit_value = (int)(c - '0');
    } else if ('a' <= c && c <= 'f') {
      digit_value = 10 + (int)(c - 'a');
    } else if ('A' <= c && c <= 'F') {
      digit_value = 10 + (int)(c - 'A');
    } else assert(0);
  } else assert(0);
  return digit_value;
}

internal int
parse_integer(char* str, int base)
{
  int result = 0;
  char c = *str++;
  assert(cstr_is_digit(c, base) || c == '_');
  if (c != '_') {
    result = digit_to_integer(c, base);
  }
  for (c = *str++; c != '\0'; c = *str++) {
    if (cstr_is_digit(c, base)) {
      result = result*base + digit_to_integer(c, base);
    } else if (c == '_') {
      continue;
    } else assert(0);
  }
  return result;
}

internal void
token_install_integer(Token* token, Lexeme* lexeme, int base)
{
  char* string = lexeme_to_cstring(lexeme);
  if (cstr_is_digit(*string, base) || *string == '_') {
    token->i.value = parse_integer(string, base);
  } else {
    if (base == 10) {
      error("At %d:%d expected one or more digits, got '%s'.",
            token->line_no, token->column_no, string);
    } else if (base == 16) {
      error("At %d:%d expected one or more hexadecimal digits, got '%s'.",
            token->line_no, token->column_no, string);
    } else if (base == 8) {
      error("At %d:%d expected one or more octal digits, got '%s'.",
            token->line_no, token->column_no, string);
    } else if (base == 2) {
      error("At %d:%d expected one or more binary digits, got '%s'.",
            token->line_no, token->column_no, string);
    } else assert(0);
  }
}

internal void
next_token(Token* token)
{
  memset(token, 0, sizeof(*token));
  state = 1;
  while (state) {
    char c = char_lookahead(0);
    switch (state) {
      default: assert(0);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          lexeme_advance();
          if (c == '\n' || c == '\r') {
            char cc = char_lookahead(0);
            if (c + cc == '\n' + '\r') {
              lexeme_advance();
            }
            line_no += 1;
            line_start = lexeme->start;
          }
          state = 1;
        }
        else if (c == ';') {
          state = 100;
        } else if (c == '<') {
          state = 101;
        } else if (c == '>') {
          state = 102;
        } else if (c == '_') {
          state = 103;
        } else if (c == ':') {
          state = 104;
        } else if (c == '(') {
          state = 105;
        } else if (c == ')') {
          state = 106;
        } else if (c == '.') {
          state = 107;
        } else if (c == '{') {
          state = 108;
        } else if (c == '}') {
          state = 109;
        } else if (c == '[') {
          state = 110;
        } else if (c == ']') {
          state = 111;
        } else if (c == ',') {
          state = 112;
        } else if (c == '-') {
          state = 113;
        } else if (c == '+') {
          state = 114;
        } else if (c == '*') {
          state = 115;
        } else if (c == '/') {
          state = 116;
        } else if (c == '=') {
          state = 117;
        } else if (c == '!') {
          state = 118;
        } else if (c == '&') {
          state = 119;
        } else if (c == '|') {
          state = 120;
        } else if (c == '^') {
          state = 121;
        } else if (c == '~') {
          state = 122;
        } else if (c == '"') {
          state = 200;
        } else if (cstr_is_digit(c, 10)) {
          state = 400;
        } else if (cstr_is_letter(c)) {
          state = 500;
        } else if (c == '\0') {
          state = 2;
        } else {
          state = 3;
        }
      } break;

      case 2:
      {
        token->klass = TK_END_OF_INPUT;
        token->lexeme = "<end-of-input>";
        state = 0;
      } break;

      case 3:
      {
        token->klass = TK_UNKNOWN;
        token->lexeme = "<unknown>";
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 4:
      {
        token->klass = TK_LEXICAL_ERROR;
        token->lexeme = "<error>";
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 100:
      {
        token->klass = TK_SEMICOLON;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 101:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TK_ANGLE_OPEN_EQUAL;
        } else if (char_lookahead(1) == '<') {
          char_advance(1);
          token->klass = TK_DOUBLE_ANGLE_OPEN;
        } else {
          token->klass = TK_ANGLE_OPEN;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 102:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TK_ANGLE_CLOSE_EQUAL;
        } else if (char_lookahead(1) == '>') {
          char_advance(1);
          token->klass = TK_DOUBLE_ANGLE_CLOSE;
        } else {
          token->klass = TK_ANGLE_CLOSE;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 103:
      {
        char cc = char_lookahead(1);
        if (cstr_is_letter(cc) || cstr_is_digit(cc, 10) || cc == '_') {
          state = 500;
        } else {
          token->klass = TK_DONTCARE;
          token->lexeme = lexeme_to_cstring(lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        }
      } break;

      case 104:
      {
        token->klass = TK_COLON;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 105:
      {
        token->klass = TK_PARENTH_OPEN;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = TK_PARENTH_CLOSE;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 107:
      {
        token->klass = TK_DOTPREFIX;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 108:
      {
        token->klass = TK_BRACE_OPEN;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 109:
      {
        token->klass = TK_BRACE_CLOSE;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 110:
      {
        token->klass = TK_BRACKET_OPEN;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 111:
      {
        token->klass = TK_BRACKET_CLOSE;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 112:
      {
        token->klass = TK_COMMA;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 113:
      {
        Token* prev_token = array_get(&tokens_array, tokens_array.elem_count - 1);
        if (prev_token->klass == TK_PARENTH_OPEN) {
          token->klass = TK_UNARY_MINUS;
        } else {
          token->klass = TK_MINUS;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 114:
      {
        token->klass = TK_PLUS;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 115:
      {
        token->klass = TK_STAR;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 116:
      {
        if (char_lookahead(1) == '*') {
          char_advance(1);
          state = 310;
        } else if (char_lookahead(1) == '/') {
          state = 311;
        } else {
          token->klass = TK_SLASH;
          token->lexeme = lexeme_to_cstring(lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        }
      } break;

      case 117:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TK_DOUBLE_EQUAL;
        } else {
          token->klass = TK_EQUAL;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 118:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TK_EXCLAMATION_EQUAL;
        } else {
          token->klass = TK_EXCLAMATION;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 119:
      {
        if (char_lookahead(1) == '&') {
          char_advance(1);
          if (char_lookahead(1) == '&') {
            char_advance(1);
            token->klass = TK_TRIPLE_AMPERSAND;
          } else {
            token->klass = TK_DOUBLE_AMPERSAND;
          }
        } else {
          token->klass = TK_AMPERSAND;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 120:
      {
        if (char_lookahead(1) == '|') {
          char_advance(1);
          token->klass = TK_DOUBLE_PIPE;
        } else {
          token->klass = TK_PIPE;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 121:
      {
        token->klass = TK_CIRCUMFLEX;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 122:
      {
        token->klass = TK_TILDA;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 200:
      {
        do {
          c = char_advance(1);
          if (c == '\\') {
            state = 201;
            break;
          } else if (c == '\n' || c == '\r') {
            state = 4;
          }
        } while (c != '"');

        token->klass = TK_STRING_LITERAL;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 201:
      {
        c = char_advance(1);
        if (c == '\n' || c == '\r') {
          line_no += 1;
          line_start = lexeme->start;
          state = 200;
        } else if (c == '\\' || c =='"' || c == 'n' || c == 'r') {
          state = 200; // ok
        } else {
          state = 4;
        }
      } break;

      case 310:
      {
        do {
          c = char_advance(1);
          if (c == '\n' || c == '\r') {
            char cc = char_lookahead(1);
            if (c + cc == '\n' + '\r')
              c = char_advance(1);
            line_no += 1;
            line_start = lexeme->start;
          }
        } while (c != '*');

        if (char_lookahead(1) == '/') {
          char_advance(1);
          token->klass = TK_COMMENT;
          token->lexeme = lexeme_to_cstring(lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        } else {
          state = 310;
        }
      } break;

      case 311:
      {
        do {
          c = char_advance(1);
        } while (c != '\n' && c != '\r');
        line_no += 1;
        line_start = lexeme->start;
        token->klass = TK_COMMENT;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 400:
      {
        if (c == '0') {
          c = char_lookahead(1);
          if (c == 'x' || c == 'X') {
            state = 402;
            char_advance(2);
            break;
          } else if (c == 'o' || c == 'O') {
            state = 403;
            char_advance(2);
            break;
          } else if (c == 'b' || c == 'B') {
            state = 404;
            char_advance(2);
            break;
          }
        }
        state = 401;  // decimal
      } break;

      case 401:
      {
        // 99
        // ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 10));
        if (c == 'w' || c == 's') {
          token->klass = TK_INT_LITERAL;
          if (c == 's') {
            token->i.is_signed = true;
          }
          lexeme[1].end = lexeme->end - 1;  // omit w|s
          token->i.width = parse_integer(lexeme_to_cstring(&lexeme[1]), 10);
          char_advance(1);
          state = 405;
        } else {
          char_retract();
          lexeme[1].end = lexeme->end;
          token->klass = TK_INT_LITERAL;
          token->i.is_signed = true;
          token_install_integer(token, &lexeme[1], 10);
          token->lexeme = lexeme_to_cstring(lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        }
      } break;

      case 402:
      {
        // 0xFF
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 16) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TK_INT_LITERAL;
        token->i.is_signed = true;
        token_install_integer(token, &lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 403:
      {
        // 0o77
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 8) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TK_INT_LITERAL;
        token->i.is_signed = true;
        token_install_integer(token, &lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 404:
      {
        // 0b11
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 2) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TK_INT_LITERAL;
        token->i.is_signed = true;
        token_install_integer(token, &lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 405:
      {
        if (c == '0') {
          c = char_lookahead(1);
          if (c == 'x' || c == 'X') {
            state = 406;
            char_advance(2);
            break;
          } else if (c == 'o' || c == 'O') {
            state = 407;
            char_advance(2);
            break;
          } else if (c == 'b' || c == 'B') {
            state = 408;
            char_advance(2);
            break;
          }
        }
        state = 409;  // decimal
      } break;

      case 406:
      {
        // ..(w|s)0xFF
        //          ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 16) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 407:
      {
        // ..(w|s)0o77
        //          ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 8) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 408:
      {
        // ..(w|s)0b11
        //          ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 2) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 409:
      {
        // ..(w|s)99
        //        ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = char_advance(1);
        } while (cstr_is_digit(c, 10));
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 10);
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 500:
      {
        do {
          c = char_advance(1);
        } while (cstr_is_letter(c) || cstr_is_digit(c, 10) || c == '_');
        char_retract();
        token->klass = TK_IDENTIFIER;
        token->lexeme = lexeme_to_cstring(lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;
    }
  }
  token->line_no = line_no;
}

UnboundedArray*
lex_tokenize(char* text_, int text_size_, Arena* lexeme_storage_, Arena* tokens_storage_)
{
  text = text_;
  text_size = text_size_;
  lexeme_storage = lexeme_storage_;
  tokens_storage = tokens_storage_;

  lexeme->start = lexeme->end = text;
  line_start = text;
  line_no = 1;

  Token token = {};
  token.klass = TK_START_OF_INPUT;
  array_init(&tokens_array, sizeof(token), tokens_storage);
  array_append(&tokens_array, &token);

  next_token(&token);
  array_append(&tokens_array, &token);
  while (token.klass != TK_END_OF_INPUT) {
    if (token.klass == TK_UNKNOWN) {
      error("At %d:%d unknown token.", token.line_no, token.column_no);
    } else if (token.klass == TK_LEXICAL_ERROR) {
      error("At %d:%d lexical error.", token.line_no, token.column_no);
    }
    next_token(&token);
    array_append(&tokens_array, &token);
  }
  return &tokens_array;
}

