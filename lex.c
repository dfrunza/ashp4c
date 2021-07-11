#include "arena.h"
#include "lex.h"
#include <memory.h>  // memset

internal struct Arena* lexeme_storage;
internal char* text;
internal int text_size;

internal struct Arena* tokens_storage;
internal struct UnboundedArray* tokens_array;
internal int line_nr = 1;
internal int state = 0;

struct Lexeme {
  char* start;
  char* end;
} lexeme[2];

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
lexeme_copy(char* dest, struct Lexeme* lexeme)
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
lexeme_len(struct Lexeme* lexeme)
{
  int result = lexeme->end - lexeme->start + 1;
  return result;
}

internal bool
lexeme_match_cstr(struct Lexeme* lexeme, char* str)
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
lexeme_to_cstring(struct Lexeme* lexeme)
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
token_install_integer(struct Token* token, struct Lexeme* lexeme, int base)
{
  char* string = lexeme_to_cstring(lexeme);
  if (cstr_is_digit(*string, base) || *string == '_') {
    token->i.value = parse_integer(string, base);
  } else {
    if (base == 10) {
      error("at line %d: expected one or more digits, got '%s'.", token->line_nr, string);
    } else if (base == 16) {
      error("at line %d: expected one or more hexadecimal digits, got '%s'.", token->line_nr, string);
    } else if (base == 8) {
      error("at line %d: expected one or more octal digits, got '%s'.", token->line_nr, string);
    } else if (base == 2) {
      error("at line %d: expected one or more binary digits, got '%s'.", token->line_nr, string);
    } else assert(0);
  }
}

internal void
next_token(struct Token* token)
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
            line_nr++;
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
        token->klass = Token_EndOfInput_;
        token->lexeme = "<end-of-input>";
        state = 0;
      } break;

      case 3:
      {
        token->klass = Token_Unknown_;
        token->lexeme = "<unknown>";
        lexeme_advance();
        state = 0;
      } break;

      case 4:
      {
        token->klass = Token_LexicalError_;
        token->lexeme = "<error>";
        lexeme_advance();
        state = 0;
      } break;

      case 100:
      {
        token->klass = Token_Semicolon;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 101:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = Token_AngleOpenEqual;
        } else if (char_lookahead(1) == '<') {
          char_advance(1);
          token->klass = Token_TwoAngleOpen;
        } else {
          token->klass = Token_AngleOpen;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 102:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = Token_AngleCloseEqual;
        } else if (char_lookahead(1) == '>') {
          char_advance(1);
          token->klass = Token_TwoAngleClose;
        } else {
          token->klass = Token_AngleClose;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 103:
      {
        char cc = char_lookahead(1);
        if (cstr_is_letter(cc) || cstr_is_digit(cc, 10) || cc == '_') {
          state = 500;
        } else {
          token->klass = Token_Dontcare;
          token->lexeme = lexeme_to_cstring(lexeme);
          lexeme_advance();
          state = 0;
        }
      } break;

      case 104:
      {
        token->klass = Token_Colon;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 105:
      {
        token->klass = Token_ParenthOpen;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = Token_ParenthClose;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 107:
      {
        token->klass = Token_DotPrefix;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 108:
      {
        token->klass = Token_BraceOpen;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 109:
      {
        token->klass = Token_BraceClose;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 110:
      {
        token->klass = Token_BracketOpen;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 111:
      {
        token->klass = Token_BracketClose;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 112:
      {
        token->klass = Token_Comma;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 113:
      {
        struct Token* prev_token = array_get(tokens_array, tokens_array->elem_count - 1);
        if (prev_token->klass == Token_ParenthOpen) {
          token->klass = Token_UnaryMinus;
        } else {
          token->klass = Token_Minus;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 114:
      {
        token->klass = Token_Plus;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 115:
      {
        token->klass = Token_Star;
        token->lexeme = lexeme_to_cstring(lexeme);
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
          token->klass = Token_Slash;
          token->lexeme = lexeme_to_cstring(lexeme);
          lexeme_advance();
          state = 0;
        }
      } break;

      case 117:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = Token_TwoEqual;
        } else {
          token->klass = Token_Equal;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 118:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = Token_ExclamationEqual;
        } else {
          token->klass = Token_Exclamation;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 119:
      {
        if (char_lookahead(1) == '&') {
          char_advance(1);
          if (char_lookahead(1) == '&') {
            char_advance(1);
            token->klass = Token_ThreeAmpersand;
          } else {
            token->klass = Token_TwoAmpersand;
          }
        } else {
          token->klass = Token_Ampersand;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 120:
      {
        if (char_lookahead(1) == '|') {
          char_advance(1);
          token->klass = Token_TwoPipe;
        } else {
          token->klass = Token_Pipe;
        }
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 121:
      {
        token->klass = Token_Circumflex;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 122:
      {
        token->klass = Token_Tilda;
        token->lexeme = lexeme_to_cstring(lexeme);
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

        token->klass = Token_StringLiteral;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;

      case 201:
      {
        c = char_advance(1);
        if (c == '\n' || c == '\r') {
          line_nr++;
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
            line_nr++;
          }
        } while (c != '*');

        if (char_lookahead(1) == '/') {
          char_advance(1);
          token->klass = Token_Comment;
          token->lexeme = lexeme_to_cstring(lexeme);
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
        line_nr++;
        token->klass = Token_Comment;
        token->lexeme = lexeme_to_cstring(lexeme);
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
          token->klass = Token_Integer;
          token->i.flags |= AstInteger_HasWidth;
          if (c == 's') {
            token->i.flags |= AstInteger_IsSigned;
          }
          lexeme[1].end = lexeme->end - 1;  // omit w|s
          token->i.width = parse_integer(lexeme_to_cstring(&lexeme[1]), 10);
          char_advance(1);
          state = 405;
        } else {
          char_retract();
          lexeme[1].end = lexeme->end;
          token->klass = Token_Integer;
          token->i.flags |= AstInteger_IsSigned;
          token_install_integer(token, &lexeme[1], 10);
          token->lexeme = lexeme_to_cstring(lexeme);
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
        token->klass = Token_Integer;
        token->i.flags |= AstInteger_IsSigned;
        token_install_integer(token, &lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(lexeme);
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
        token->klass = Token_Integer;
        token->i.flags |= AstInteger_IsSigned;
        token_install_integer(token, &lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(lexeme);
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
        token->klass = Token_Integer;
        token->i.flags |= AstInteger_IsSigned;
        token_install_integer(token, &lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(lexeme);
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
        lexeme_advance();
        state = 0;
      } break;

      case 500:
      {
        do {
          c = char_advance(1);
        } while (cstr_is_letter(c) || cstr_is_digit(c, 10) || c == '_');
        char_retract();
        token->klass = Token_Identifier;
        token->lexeme = lexeme_to_cstring(lexeme);
        lexeme_advance();
        state = 0;
      } break;
    }
  }
  token->line_nr = line_nr;
}

void
lex_tokenize(char* text_, int text_size_, struct UnboundedArray* tokens_array_)
{
  text = text_;
  text_size = text_size_;
  tokens_array = tokens_array_;

  lexeme->start = lexeme->end = text;

  struct Token token = {};
  token.klass = Token_StartOfInput_;
  array_init(tokens_array, sizeof(token), tokens_storage);
  array_append(tokens_array, &token);

  next_token(&token);
  array_append(tokens_array, &token);
  while (token.klass != Token_EndOfInput_) {
    if (token.klass == Token_Unknown_) {
      error("at line %d: unknown token.", token.line_nr);
    } else if (token.klass == Token_LexicalError_) {
      error("at line %d: lexical error.", token.line_nr);
    }
    next_token(&token);
    array_append(tokens_array, &token);
  }
}

void
lex_set_storage(struct Arena* lexeme_storage_, struct Arena* tokens_storage_)
{
  lexeme_storage = lexeme_storage_;
  tokens_storage = tokens_storage_;
}
