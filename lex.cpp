#include <memory.h>
#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

static char char_lookahead(Lexer* lexer, int pos)
{
  char* char_pos;

  char_pos = lexer->lexeme->end + pos;
  assert(char_pos >= (char*)0 && char_pos <= (lexer->text + lexer->text_size));
  return *char_pos;
}

static char char_advance(Lexer* lexer, int pos)
{
  char* char_pos;

  char_pos = lexer->lexeme->end + pos;
  assert(char_pos >= (char*)0 && char_pos <= (lexer->text + lexer->text_size));
  lexer->lexeme->end = char_pos;
  return *char_pos;
}

static char char_retract(Lexer* lexer)
{
  char result;

  result = *(--lexer->lexeme->end);
  assert(lexer->lexeme->end >= (char*)0);
  return result;
}

static void lexeme_advance(Lexer* lexer)
{
  lexer->lexeme->start = ++lexer->lexeme->end;
  assert(lexer->lexeme->start <= (lexer->text + lexer->text_size));
}

static void lexeme_copy(char* dest, Lexeme* lexeme)
{
  char* src;

  src = lexeme->start;
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

static int lexeme_len(Lexeme* lexeme)
{
  int result;

  result = lexeme->end - lexeme->start + 1;
  return result;
}

static char* lexeme_to_cstring(Arena* storage, Lexeme* lexeme)
{
  int len;
  char* string;

  len = lexeme_len(lexeme);
  string = (char*)storage->malloc((len + 1)*sizeof(char));   // +1 the NULL terminator
  lexeme_copy(string, lexeme);
  string[len] = '\0';
  return string;
}

static int digit_to_integer(char c, int base)
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

static int parse_integer(char* str, int base)
{
  int result = 0;
  char c;

  c = *str++;
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

static void token_install_integer(Lexer* lexer, Token* token, Lexeme* lexeme, int base)
{
  char* string;

  string = lexeme_to_cstring(lexer->storage, lexeme);
  if (cstr_is_digit(*string, base) || *string == '_') {
    token->integer.value = parse_integer(string, base);
  } else {
    if (base == 10) {
      error("%s:%d:%d: error: expected one or more digits, got '%s'.",
            lexer->filename, token->line_no, token->column_no, string);
    } else if (base == 16) {
      error("%s:%d:%d: error: expected one or more hexadecimal digits, got '%s'.",
            lexer->filename, token->line_no, token->column_no, string);
    } else if (base == 8) {
      error("%s:%d:%d: error: expected one or more octal digits, got '%s'.",
            lexer->filename, token->line_no, token->column_no, string);
    } else if (base == 2) {
      error("%s:%d:%d: error: expected one or more binary digits, got '%s'.",
            lexer->filename, token->line_no, token->column_no, string);
    } else assert(0);
  }
}

static void next_token(Lexer* lexer, Token* token)
{
  char c, cc;
  Token* prev_token;

  memset(token, 0, sizeof(Token));
  lexer->state = 1;
  while (lexer->state) {
    c = char_lookahead(lexer, 0);
    switch (lexer->state) {
      default: assert(0);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          lexeme_advance(lexer);
          if (c == '\n' || c == '\r') {
            cc = char_lookahead(lexer, 0);
            if (c + cc == '\n' + '\r') {
              lexeme_advance(lexer);
            }
            lexer->line_no += 1;
            lexer->line_start = lexer->lexeme->start;
          }
          lexer->state = 1;
        }
        else if (c == ';') {
          lexer->state = 100;
        } else if (c == '<') {
          lexer->state = 101;
        } else if (c == '>') {
          lexer->state = 102;
        } else if (c == '_') {
          lexer->state = 103;
        } else if (c == ':') {
          lexer->state = 104;
        } else if (c == '(') {
          lexer->state = 105;
        } else if (c == ')') {
          lexer->state = 106;
        } else if (c == '.') {
          lexer->state = 107;
        } else if (c == '{') {
          lexer->state = 108;
        } else if (c == '}') {
          lexer->state = 109;
        } else if (c == '[') {
          lexer->state = 110;
        } else if (c == ']') {
          lexer->state = 111;
        } else if (c == ',') {
          lexer->state = 112;
        } else if (c == '-') {
          lexer->state = 113;
        } else if (c == '+') {
          lexer->state = 114;
        } else if (c == '*') {
          lexer->state = 115;
        } else if (c == '/') {
          lexer->state = 116;
        } else if (c == '=') {
          lexer->state = 117;
        } else if (c == '!') {
          lexer->state = 118;
        } else if (c == '&') {
          lexer->state = 119;
        } else if (c == '|') {
          lexer->state = 120;
        } else if (c == '^') {
          lexer->state = 121;
        } else if (c == '~') {
          lexer->state = 122;
        } else if (c == '"') {
          lexer->state = 200;
        } else if (cstr_is_digit(c, 10)) {
          lexer->state = 400;
        } else if (cstr_is_letter(c)) {
          lexer->state = 500;
        } else if (c == '\0') {
          lexer->state = 2;
        } else {
          lexer->state = 3;
        }
      } break;

      case 2:
      {
        token->klass = TokenClass::END_OF_INPUT;
        token->lexeme = "<end-of-input>";
        lexer->state = 0;
      } break;

      case 3:
      {
        token->klass = TokenClass::UNKNOWN;
        token->lexeme = "<unknown>";
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 4:
      {
        token->klass = TokenClass::LEXICAL_ERROR;
        token->lexeme = "<error>";
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 100:
      {
        token->klass = TokenClass::SEMICOLON;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 101:
      {
        if (char_lookahead(lexer, 1) == '=') {
          char_advance(lexer, 1);
          token->klass = TokenClass::ANGLE_OPEN_EQUAL;
        } else if (char_lookahead(lexer, 1) == '<') {
          char_advance(lexer, 1);
          token->klass = TokenClass::DOUBLE_ANGLE_OPEN;
        } else {
          token->klass = TokenClass::ANGLE_OPEN;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 102:
      {
        if (char_lookahead(lexer, 1) == '=') {
          char_advance(lexer, 1);
          token->klass = TokenClass::ANGLE_CLOSE_EQUAL;
        } else if (char_lookahead(lexer, 1) == '>') {
          char_advance(lexer, 1);
          token->klass = TokenClass::DOUBLE_ANGLE_CLOSE;
        } else {
          token->klass = TokenClass::ANGLE_CLOSE;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 103:
      {
        cc = char_lookahead(lexer, 1);
        if (cstr_is_letter(cc) || cstr_is_digit(cc, 10) || cc == '_') {
          lexer->state = 500;
        } else {
          token->klass = TokenClass::DONTCARE;
          token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
          token->column_no = lexer->lexeme->start - lexer->line_start + 1;
          lexeme_advance(lexer);
          lexer->state = 0;
        }
      } break;

      case 104:
      {
        token->klass = TokenClass::COLON;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 105:
      {
        token->klass = TokenClass::PARENTH_OPEN;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      }
      break;

      case 106:
      {
        token->klass = TokenClass::PARENTH_CLOSE;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 107:
      {
        token->klass = TokenClass::DOT;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 108:
      {
        token->klass = TokenClass::BRACE_OPEN;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 109:
      {
        token->klass = TokenClass::BRACE_CLOSE;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 110:
      {
        token->klass = TokenClass::BRACKET_OPEN;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 111:
      {
        token->klass = TokenClass::BRACKET_CLOSE;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 112:
      {
        token->klass = TokenClass::COMMA;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 113:
      {
        prev_token = (Token*)lexer->tokens->get(lexer->tokens->elem_count - 1, sizeof(Token));
        if (prev_token->klass == TokenClass::PARENTH_OPEN) {
          token->klass = TokenClass::UNARY_MINUS;
        } else {
          token->klass = TokenClass::MINUS;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 114:
      {
        token->klass = TokenClass::PLUS;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 115:
      {
        token->klass = TokenClass::STAR;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 116:
      {
        if (char_lookahead(lexer, 1) == '*') {
          char_advance(lexer, 1);
          lexer->state = 310;
        } else if (char_lookahead(lexer, 1) == '/') {
          lexer->state = 311;
        } else {
          token->klass = TokenClass::SLASH;
          token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
          token->column_no = lexer->lexeme->start - lexer->line_start + 1;
          lexeme_advance(lexer);
          lexer->state = 0;
        }
      } break;

      case 117:
      {
        if (char_lookahead(lexer, 1) == '=') {
          char_advance(lexer, 1);
          token->klass = TokenClass::DOUBLE_EQUAL;
        } else {
          token->klass = TokenClass::EQUAL;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 118:
      {
        if (char_lookahead(lexer, 1) == '=') {
          char_advance(lexer, 1);
          token->klass = TokenClass::EXCLAMATION_EQUAL;
        } else {
          token->klass = TokenClass::EXCLAMATION;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 119:
      {
        if (char_lookahead(lexer, 1) == '&') {
          char_advance(lexer, 1);
          if (char_lookahead(lexer, 1) == '&') {
            char_advance(lexer, 1);
            token->klass = TokenClass::TRIPLE_AMPERSAND;
          } else {
            token->klass = TokenClass::DOUBLE_AMPERSAND;
          }
        } else {
          token->klass = TokenClass::AMPERSAND;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 120:
      {
        if (char_lookahead(lexer, 1) == '|') {
          char_advance(lexer, 1);
          token->klass = TokenClass::DOUBLE_PIPE;
        } else {
          token->klass = TokenClass::PIPE;
        }
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 121:
      {
        token->klass = TokenClass::CIRCUMFLEX;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 122:
      {
        token->klass = TokenClass::TILDA;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 200:
      {
        do {
          c = char_advance(lexer, 1);
          if (c == '\\') {
            lexer->state = 201;
            break;
          } else if (c == '\n' || c == '\r') {
            lexer->state = 4;
          }
        } while (c != '"');

        token->klass = TokenClass::STRING_LITERAL;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 201:
      {
        c = char_advance(lexer, 1);
        if (c == '\n' || c == '\r') {
          lexer->line_no += 1;
          lexer->line_start = lexer->lexeme->start;
          lexer->state = 200;
        } else if (c == '\\' || c =='"' || c == 'n' || c == 'r') {
          lexer->state = 200; // ok
        } else {
          lexer->state = 4;
        }
      } break;

      case 310:
      {
        do {
          c = char_advance(lexer, 1);
          if (c == '\n' || c == '\r') {
            char cc = char_lookahead(lexer, 1);
            if (c + cc == '\n' + '\r') {
              c = char_advance(lexer, 1);
            }
            lexer->line_no += 1;
          }
        } while (c != '*');

        if (char_lookahead(lexer, 1) == '/') {
          char_advance(lexer, 1);
          token->klass = TokenClass::COMMENT;
          token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
          lexeme_advance(lexer);
          lexer->line_start = lexer->lexeme->start;
          lexer->state = 0;
        } else {
          lexer->state = 310;
        }
      } break;

      case 311:
      {
        do {
          c = char_advance(lexer, 1);
        } while (c != '\n' && c != '\r');

        lexer->line_no += 1;
        token->klass = TokenClass::COMMENT;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        lexeme_advance(lexer);
        lexer->line_start = lexer->lexeme->start;
        lexer->state = 0;
      } break;

      case 400:
      {
        if (c == '0') {
          c = char_lookahead(lexer, 1);
          if (c == 'x' || c == 'X') {
            lexer->state = 402;
            char_advance(lexer, 2);
            break;
          } else if (c == 'o' || c == 'O') {
            lexer->state = 403;
            char_advance(lexer, 2);
            break;
          } else if (c == 'b' || c == 'B') {
            lexer->state = 404;
            char_advance(lexer, 2);
            break;
          }
        }
        lexer->state = 401;  // decimal
      } break;

      case 401:
      {
        // 99
        // ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 10));
        if (c == 'w' || c == 's') {
          token->klass = TokenClass::INTEGER_LITERAL;
          if (c == 's') {
            token->integer.is_signed = 1;
          }
          lexer->lexeme[1].end = lexer->lexeme->end - 1;  // omit w|s
          token->integer.width = parse_integer(lexeme_to_cstring(lexer->storage,&lexer->lexeme[1]), 10);
          char_advance(lexer, 1);
          lexer->state = 405;
        } else {
          char_retract(lexer);
          lexer->lexeme[1].end = lexer->lexeme->end;
          token->klass = TokenClass::INTEGER_LITERAL;
          token->integer.is_signed = 1;
          token_install_integer(lexer, token, &lexer->lexeme[1], 10);
          token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
          token->column_no = lexer->lexeme->start - lexer->line_start + 1;
          lexeme_advance(lexer);
          lexer->state = 0;
        }
      } break;

      case 402:
      {
        // 0xFF
        //   ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 16) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(lexer, token, &lexer->lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 403:
      {
        // 0o77
        //   ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 8) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(lexer, token, &lexer->lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 404:
      {
        // 0b11
        //   ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 2) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(lexer, token, &lexer->lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 405:
      {
        if (c == '0') {
          c = char_lookahead(lexer, 1);
          if (c == 'x' || c == 'X') {
            lexer->state = 406;
            char_advance(lexer, 2);
            break;
          } else if (c == 'o' || c == 'O') {
            lexer->state = 407;
            char_advance(lexer, 2);
            break;
          } else if (c == 'b' || c == 'B') {
            lexer->state = 408;
            char_advance(lexer, 2);
            break;
          }
        }
        lexer->state = 409;  // decimal
      } break;

      case 406:
      {
        // ..(w|s)0xFF
        //          ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 16) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token_install_integer(lexer, token, &lexer->lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 407:
      {
        // ..(w|s)0o77
        //          ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 8) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token_install_integer(lexer, token, &lexer->lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 408:
      {
        // ..(w|s)0b11
        //          ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 2) || c == '_');
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token_install_integer(lexer, token, &lexer->lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 409:
      {
        // ..(w|s)99
        //        ^^
        lexer->lexeme[1].start = lexer->lexeme[1].end = lexer->lexeme->end;
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_digit(c, 10));
        char_retract(lexer);
        lexer->lexeme[1].end = lexer->lexeme->end;
        token_install_integer(lexer, token, &lexer->lexeme[1], 10);
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;

      case 500:
      {
        do {
          c = char_advance(lexer, 1);
        } while (cstr_is_letter(c) || cstr_is_digit(c, 10) || c == '_');
        char_retract(lexer);
        token->klass = TokenClass::IDENTIFIER;
        token->lexeme = lexeme_to_cstring(lexer->storage, lexer->lexeme);
        token->column_no = lexer->lexeme->start - lexer->line_start + 1;
        lexeme_advance(lexer);
        lexer->state = 0;
      } break;
    }
  }
  token->line_no = lexer->line_no;
}

void tokenize(Lexer* lexer, SourceText* source_text)
{
  Token token = {};

  lexer->filename = source_text->filename;
  lexer->text = source_text->text;
  lexer->text_size = source_text->text_size;
  lexer->lexeme->start = lexer->lexeme->end = lexer->text;
  lexer->line_start = lexer->text;
  lexer->line_no = 1;

  token.klass = TokenClass::START_OF_INPUT;
  lexer->tokens = Array::create(lexer->storage, sizeof(Token), 7);
  *(Token*)lexer->tokens->append(sizeof(Token)) = token;

  next_token(lexer, &token);
  *(Token*)lexer->tokens->append(sizeof(Token)) = token;
  while (token.klass != TokenClass::END_OF_INPUT) {
    if (token.klass == TokenClass::UNKNOWN) {
      error("%s:%d:%d: error: unknown token.", lexer->filename, token.line_no, token.column_no);
    } else if (token.klass == TokenClass::LEXICAL_ERROR) {
      error("%s:%d:%d: error: lexical error.", lexer->filename, token.line_no, token.column_no);
    }
    next_token(lexer, &token);
    *(Token*)lexer->tokens->append(sizeof(Token)) = token;
  }
}

