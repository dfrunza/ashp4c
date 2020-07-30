#include "dp4c.h"

external Arena arena;
external char* input_text;
external uint32_t input_size;
external Token* tokenized_input;
external int tokenized_input_len;

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
  do
  {
    if (*src == '\\')
    {
      src++;
      if (*src == 'n')
      {
        *dest++ = '\n';
        src++;
      }
      else if (*src == 'r')
      {
        *dest++ = '\r';
        src++;
      }
      else if (*src == 't')
      {
        *dest++ = '\t';
        src++;
      }
      else
        *dest++ = *src++;
    }
    else
      *dest++ = *src++;
  }
  while (src <= end);
}

bool
lexeme_len(char* lexeme_start, char* lexeme_end)
{
  int result = lexeme_end - lexeme_start + 1;
  return result;
}

bool
lexeme_match_cstr(char* lexeme_start, char* lexeme_end, char* str)
{
  char* l = lexeme_start;
  char* s = str;
  while (*l == *s)
  {
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

internal void
next_token(Token* token_at)
{
  zero_struct(token_at, Token);
  state = 1;
  while (state)
  {
    char c = char_lookahead(0);
    switch (state)
    {
      default:
        assert (false);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
          lexeme_advance();
          if (c == '\n' || c == '\r')
          {
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
        else if (c == ',')
          state = 110;
        else if (c == '-')
          state = 111;
        else if (c == '+')
          state = 112;
        else if (c == '*')
          state = 113;
        else if (c == '/')
          state = 114;
        else if (c == '=')
          state = 115;
        else if (cstr_is_digit(c))
          state = 400;
        else if (cstr_is_letter(c))
          state = 500;
        else if (c == '\0')
          state = 2;
        else
          state = 3;
      }
      break;

      case 100:
      {
        token_at->klass = TOK_SEMICOLON;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 101:
      {
        token_at->klass = TOK_ANGLE_OPEN;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 102:
      {
        token_at->klass = TOK_ANGLE_CLOSE;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 103:
      {
        token_at->klass = TOK_DONTCARE;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 104:
      {
        token_at->klass = TOK_COLON;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 105:
      {
        token_at->klass = TOK_PARENTH_OPEN;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token_at->klass = TOK_PARENTH_CLOSE;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 107:
      {
        token_at->klass = TOK_PERIOD;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 108:
      {
        token_at->klass = TOK_BRACE_OPEN;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 109:
      {
        token_at->klass = TOK_BRACE_CLOSE;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 110:
      {
        token_at->klass = TOK_COMMA;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 111:
      {
        token_at->klass = TOK_MINUS;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 112:
      {
        token_at->klass = TOK_PLUS;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 113:
      {
        token_at->klass = TOK_STAR;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 114:
      {
        if (char_lookahead(1) == '*')
        {
          char_advance();
          state = 314;
        }
        else
        {
          token_at->klass = TOK_SLASH;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      }
      break;

      case 115:
      {
        if (char_lookahead(1) == '=')
        {
          char_advance();
          token_at->klass = TOK_EQUAL_EQUAL;
        }
        else
          token_at->klass = TOK_EQUAL;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 314:
      {
        do
          c = char_advance();
        while (c != '*');
        if (char_lookahead(1) == '/')
        {
          char_advance();
          token_at->klass = TOK_COMMENT;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
        else
          state = 314;
      }
      break;

      case 400:
      {
        if (c == '0')
        {
          c = char_lookahead(1);
          if (c == 'x' || c == 'X')
          {
            state = 402;
            lexeme_advance();
            break;
          }
          else if (c == 'o' || c == 'O')
          {
            state = 403;
            lexeme_advance();
            break;
          }
          else if (c == 'b' || c == 'B')
          {
            state = 404;
            lexeme_advance();
            break;
          }
        }
        state = 401;
      }
      break;

      case 401:
      {
        do
          c = char_advance();
        while (cstr_is_digit(c));
        if (c == 'w')
          state = 405;
        else if (c == 's')
          state = 406;
        else
        {
          char_retract();
          token_at->klass = TOK_INTEGER;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      }
      break;

      case 402:
      {
        do
          c = char_advance();
        while (cstr_is_hex_digit(c));
        if (c == 'w')
          state = 407;
        else if (c == 's')
          state = 408;
        else
        {
          char_retract();
          token_at->klass = TOK_INTEGER_HEX;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      }
      break;

      case 403:
      {
        do
          c = char_advance();
        while (cstr_is_oct_digit(c));
        if (c == 'w')
          state = 409;
        else if (c == 's')
          state = 410;
        else
        {
          char_retract();
          token_at->klass = TOK_INTEGER_OCT;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      }
      break;

      case 404:
      {
        do
          c = char_advance();
        while (cstr_is_bin_digit(c));
        if (c == 'w')
          state = 411;
        else if (c == 's')
          state = 412;
        else
        {
          char_retract();
          token_at->klass = TOK_INTEGER_BIN;
          token_at->lexeme = lexeme_to_cstring();
          lexeme_advance();
          state = 0;
        }
      }
      break;

      case 405:
      {
        do
          c = char_advance();
        while (cstr_is_digit(c));
        char_retract();
        token_at->klass = TOK_WINTEGER;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 406:
      {
        do
          c = char_advance();
        while (cstr_is_digit(c));
        char_retract();
        token_at->klass = TOK_SINTEGER;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 407:
      {
        do
          c = char_advance();
        while (cstr_is_hex_digit(c));
        char_retract();
        token_at->klass = TOK_WINTEGER_HEX;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 408:
      {
        do
          c = char_advance();
        while (cstr_is_hex_digit(c));
        char_retract();
        token_at->klass = TOK_SINTEGER_HEX;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 409:
      {
        do
          c = char_advance();
        while (cstr_is_oct_digit(c));
        char_retract();
        token_at->klass = TOK_WINTEGER_OCT;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 410:
      {
        do
          c = char_advance();
        while (cstr_is_oct_digit(c));
        char_retract();
        token_at->klass = TOK_SINTEGER_OCT;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 411:
      {
        do
          c = char_advance();
        while (cstr_is_bin_digit(c) || c == '_');
        char_retract();
        token_at->klass = TOK_WINTEGER_BIN;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 412:
      {
        do
          c = char_advance();
        while (cstr_is_bin_digit(c) || c == '_');
        char_retract();
        token_at->klass = TOK_SINTEGER_BIN;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 500:
      {
        do
          c = char_advance();
        while (cstr_is_letter(c) || cstr_is_digit(c) || c == '_');
        char_retract();
        token_at->klass = TOK_IDENT;
        token_at->lexeme = lexeme_to_cstring();
        lexeme_advance();
        state = 0;
      }
      break;

      case 2:
      {
        token_at->klass = TOK_EOI;
        token_at->lexeme = "<end-of-input>";
        state = 0;
      }
      break;

      case 3:
      {
        token_at->klass = TOK_UNKNOWN;
        token_at->lexeme = "<unknown>";
        lexeme_advance();
        state = 0;
      }
      break;
    }
  }
  token_at->line_nr = line_nr;
}

void
lex_tokenize_input()
{
  Token* token_at = tokenized_input;
  token_at->klass = TOK_SOI;
  token_at++;
  tokenized_input_len++;

  next_token(token_at);
  tokenized_input_len++;
  while (token_at->klass != TOK_EOI)
  {
    if (token_at->klass == TOK_UNKNOWN)
      error("at line %d: unknown token", token_at->line_nr);
    token_at++;
    next_token(token_at);
    tokenized_input_len++;
  }
  arena_print_usage(&arena, "Memory (lex_tokenize_input): ");
}

