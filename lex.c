#include "lex.h"

external Arena arena;
external char* input_text;
external uint32_t input_size;
external struct Token* tokenized_input;
external int tokenized_input_len;

internal struct Token* prev_token;
internal int line_nr = 1;
internal char* lexeme_start = 0;
internal char* lexeme_end = 0;
internal int state = 0;

internal char
char_lookahead(int pos)
{
  char* char_pos = lexeme_end + pos;
  assert (char_pos >= 0 && char_pos <= (input_text + input_size));
  char result = *char_pos;
  return result;
}

internal char
char_advance()
{
  char result = *(++lexeme_end);
  assert (lexeme_end <= (input_text + input_size));
  return result;
}

internal char
char_retract()
{
  char result = *(--lexeme_end);
  assert (lexeme_end >= 0);
  return result;
}

void
lex_input_init(char* input_text)
{
  lexeme_start = input_text;
  lexeme_end = lexeme_start;
}

internal void
lexeme_advance()
{
  lexeme_start = ++lexeme_end;
  assert (lexeme_start <= (input_text + input_size));
}

internal void
lexeme_copy(char* dest, char* begin, char* end)
{
  char* src = begin;
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

internal bool
lexeme_len(char* lexeme_start, char* lexeme_end)
{
  int result = lexeme_end - lexeme_start + 1;
  return result;
}

internal bool
lexeme_match_cstr(char* lexeme_start, char* lexeme_end, char* str)
{
  char* l = lexeme_start;
  char* s = str;
  while (*l == *s) {
    l++;
    s++;
    if (*s == '\0')
      break;
  }
  bool result = (s - str) == lexeme_len(lexeme_start, lexeme_end) && (*s == '\0');
  return result;
}

internal char*
lexeme_to_cstring()
{
  int lexeme_len = lexeme_end - lexeme_start + 1;   // not counting the NULL terminator
  char* lexeme = arena_push_array(&arena, char, lexeme_len + 1);   // +1 the NULL terminator
  lexeme_copy(lexeme, lexeme_start, lexeme_end);
  lexeme[lexeme_len] = '\0';
  return lexeme;
}

internal int
parse_integer(char* str)
{
  int result = 0;
  char c = *str++;
  assert(cstr_is_digit(c));
  result = (int)(c - '0');
  for (c = *str++; c != '\0'; c = *str++) {
    if (cstr_is_digit(c)) {
      int digit = (int)(c - '0');
      result = result*10 + digit;
    } else {
      break;
    }
  }
  return result;
}

internal void
next_token(struct Token* token)
{
  zero_struct(token, Token);
  state = 1;
  while (state) {
    char c = char_lookahead(0);
    switch (state) {
      default:
        assert (false);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          lexeme_advance();
          if (c == '\n' || c == '\r') {
            char cc = char_lookahead(0);
            if (c + cc == '\n' + '\r')
              lexeme_advance();
            line_nr++;
          }
          state = 1;
        }
        else if (c == ';')
          state = 100;
        else if (c == '<')
          state = 101;
        else if (c == '>')
          state = 102;
        else if (c == '_')
          state = 103;
        else if (c == ':')
          state = 104;
        else if (c == '(')
          state = 105;
        else if (c == ')')
          state = 106;
        else if (c == '.')
          state = 107;
        else if (c == '{')
          state = 108;
        else if (c == '}')
          state = 109;
        else if (c == '[')
          state = 110;
        else if (c == ']')
          state = 111;
        else if (c == ',')
          state = 112;
        else if (c == '-')
          state = 113;
        else if (c == '+')
          state = 114;
        else if (c == '*')
          state = 115;
        else if (c == '/')
          state = 116;
        else if (c == '=')
          state = 117;
        else if (c == '!')
          state = 118;
        else if (c == '&')
          state = 119;
        else if (c == '|')
          state = 120;
        else if (c == '^')
          state = 121;
        else if (c == '~')
          state = 122;
        else if (cstr_is_digit(c))
          state = 400;
        else if (cstr_is_letter(c))
          state = 500;
        else if (c == '\0')
          state = 2;
        else
          state = 3;
      } break;

      case 2:
      {
        token->klass = Token_EndOfInput;
        token->lexeme = "<end-of-input>";
        state = 0;
      } break;

      case 3:
      {
        token->klass = Token_Unknown;
        token->lexeme = "<unknown>";
        lexeme_advance();
        state = 0;
      } break;

      case 100:
      {
        token->klass = Token_Semicolon;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 101:
      {
        if (char_lookahead(1) == '=') {
          char_advance();
          token->klass = Token_LessEqual;
        } else if (char_lookahead(1) == '<') {
          char_advance();
          token->klass = Token_BitshiftLeft;
        } else {
          token->klass = Token_AngleOpen;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 102:
      {
        if (char_lookahead(1) == '=') {
          char_advance();
          token->klass = Token_GreaterEqual;
        } else if (char_lookahead(1) == '>') {
          char_advance();
          token->klass = Token_BitshiftRight;
        } else {
          token->klass = Token_AngleClose;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 103:
      {
        char cc = char_lookahead(1);
        if (cstr_is_letter(cc) || cstr_is_digit(cc)) {
          state = 500;
        } else {
          token->klass = Token_Dontcare;
          token->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      } break;

      case 104:
      {
        token->klass = Token_Colon;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 105:
      {
        token->klass = Token_ParenthOpen;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = Token_ParenthClose;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 107:
      {
        token->klass = Token_DotPrefix;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 108:
      {
        token->klass = Token_BraceOpen;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 109:
      {
        token->klass = Token_BraceClose;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 110:
      {
        token->klass = Token_BracketOpen;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 111:
      {
        token->klass = Token_BracketClose;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 112:
      {
        token->klass = Token_Comma;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 113:
      {
        if (prev_token->klass == Token_ParenthOpen) {
          token->klass = Token_UnaryMinus;
        } else {
          token->klass = Token_Minus;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 114:
      {
        token->klass = Token_Plus;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 115:
      {
        token->klass = Token_Star;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 116:
      {
        if (char_lookahead(1) == '*') {
          char_advance();
          state = 310;
        } else if (char_lookahead(1) == '/') {
          state = 311;
        } else {
          token->klass = Token_Slash;
          token->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      } break;

      case 117:
      {
        if (char_lookahead(1) == '=') {
          char_advance();
          token->klass = Token_LogicEqual;
        } else {
          token->klass = Token_Equal;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 118:
      {
        if (char_lookahead(1) == '=') {
          char_advance();
          token->klass = Token_LogicNotEqual;
        } else {
          token->klass = Token_LogicNot;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 119:
      {
        if (char_lookahead(1) == '&') {
          char_advance();
          token->klass = Token_LogicAnd;
        } else {
          token->klass = Token_BitwiseAnd;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 120:
      {
        if (char_lookahead(1) == '|') {
          char_advance();
          token->klass = Token_LogicOr;
        } else {
          token->klass = Token_BitwiseOr;
        }
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 121:
      {
        token->klass = Token_BitwiseXor;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 122:
      {
        token->klass = Token_BitwiseNot;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 310:
      {
        do {
          c = char_advance();
          if (c == '\n' || c == '\r') {
            char cc = char_lookahead(1);
            if (c + cc == '\n' + '\r')
              c = char_advance();
            line_nr++;
          }
        } while (c != '*');

        if (char_lookahead(1) == '/') {
          char_advance();
          token->klass = Token_Comment;
          token->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        } else {
          state = 310;
        }
      } break;

      case 311:
      {
        do {
          c = char_advance();
        } while (c != '\n' && c != '\r');
        line_nr++;
        token->klass = Token_Comment;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 400:
      {
        if (c == '0') {
          c = char_lookahead(1);
          if (c == 'x' || c == 'X') {
            state = 402;
            lexeme_advance();
            break;
          }
          else if (c == 'o' || c == 'O') {
            state = 403;
            lexeme_advance();
            break;
          }
          else if (c == 'b' || c == 'B') {
            state = 404;
            lexeme_advance();
            break;
          }
        }
        state = 401;
      } break;

      case 401:
      {
        do
          c = char_advance();
        while (cstr_is_digit(c));
        if (c == 'w' || c == 's') {
          token->klass = Token_Integer;
          token->i.flags |= IntFlags_HasWidth;
          if (c == 's') {
            token->i.flags |= IntFlags_Signed;
          }
          char* lexeme = lexeme_to_cstring();
          token->i.width = parse_integer(lexeme);
          char_advance();
          state = 405;
        }
        else {
          char_retract();
          token->klass = Token_Integer;
          token->lexeme = lexeme_to_cstring();
          token->i.flags |= IntFlags_Signed;
          token->i.value = parse_integer(token->lexeme);
          lexeme_advance();
          state = 0;
        }
      } break;

      case 402:
      {
        do
          c = char_advance();
        while (cstr_is_hex_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 403:
      {
        do
          c = char_advance();
        while (cstr_is_oct_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 404:
      {
        do
          c = char_advance();
        while (cstr_is_bin_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 405:
      {
        if (c == '0') {
          c = char_lookahead(1);
          if (c == 'x' || c == 'X') {
            state = 406;
            lexeme_advance();
            break;
          }
          else if (c == 'o' || c == 'O') {
            state = 407;
            lexeme_advance();
            break;
          }
          else if (c == 'b' || c == 'B') {
            state = 408;
            lexeme_advance();
            break;
          }
        }
        state = 409;
      } break;

      case 406:
      {
        /* 32w0xAA */
        do
          c = char_advance();
        while (cstr_is_hex_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 407:
      {
        /* 32w0o77 */
        do
          c = char_advance();
        while (cstr_is_oct_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 408:
      {
        /* 32w0b11 */
        do
          c = char_advance();
        while (cstr_is_bin_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 409:
      {
        /* 32w99 */
        do
          c = char_advance();
        while (cstr_is_digit(c));
        char_retract();
        token->klass = Token_Integer;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;

      case 500:
      {
        do
          c = char_advance();
        while (cstr_is_letter(c) || cstr_is_digit(c) || c == '_');
        char_retract();
        token->klass = Token_Identifier;
        token->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      } break;
    }
  }
  token->line_nr = line_nr;
  prev_token = token;
}

void
lex_tokenize_input()
{
  struct Token* token = tokenized_input;
  token->klass = Token_StartOfInput;
  token++;
  tokenized_input_len++;

  next_token(token);
  tokenized_input_len++;
  while (token->klass != Token_EndOfInput) {
    if (token->klass == Token_Unknown)
      error("at line %d: unknown token", token->line_nr);
    token++;
    next_token(token);
    tokenized_input_len++;
  }
}

