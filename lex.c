#include "lex.h"

internal Arena* arena = 0;
char* input_text = 0;
uint32_t input_size = 0;
internal char* lexeme_start = 0;
internal char* lexeme_end = 0;
internal int state = 0;
internal int line_nr = 1;

internal char*
lex_read_input(char* filename)
{
  FILE* f_stream = fopen(filename, "rb");
  fseek(f_stream, 0, SEEK_END);
  uint32_t f_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  char* data = arena_push_array(arena, char, f_size + 1);
  fread(data, sizeof(char), f_size, f_stream);
  data[f_size] = '\0';
  input_size = f_size;
  fclose(f_stream);
  arena_print_usage(arena, "Main arena (lex_read_input): ");
  return data;
}

internal char
lex_char_lookahead(int pos)
{
  char* char_pos = lexeme_end + pos;
  assert (char_pos >= 0 && char_pos <= (input_text + input_size));
  char result = *char_pos;
  return result;
}

internal char
lex_char_advance()
{
  char result = *(++lexeme_end);
  assert (lexeme_end <= (input_text + input_size));
  return result;
}

internal char
lex_char_retract()
{
  char result = *(--lexeme_end);
  assert (lexeme_end >= 0);
  return result;
}

internal void
lex_lexeme_init(char* input_text)
{
  lexeme_start = input_text;
  lexeme_end = lexeme_start;
}

internal void
lex_lexeme_advance()
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
lex_lexeme_to_cstring()
{
  int lexeme_len = lexeme_end - lexeme_start + 1;   // not counting the NULL terminator
  char* lexeme = arena_push_array(arena, char, lexeme_len + 1);   // +1 the NULL terminator
  lexeme_copy(lexeme, lexeme_start, lexeme_end);
  lexeme[lexeme_len] = '\0';
  return lexeme;
}

void
lex_init(Arena* arena_, char* filename)
{
  arena = arena_;
  input_text = lex_read_input(filename);
  lex_lexeme_init(input_text);
}

int
lex_line_nr()
{
  return line_nr;
}

void
lex_next_token(Token* token)
{
  zero_struct(token, Token);
  bool stop = false;
  state = 1;
  while (state)
  {
    char c = lex_char_lookahead(0);
    switch (state)
    {
      default:
        assert (false);
      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r')
        {
          lex_lexeme_advance();
          if (c == '\n' || c == '\r')
          {
            char cc = lex_char_lookahead(0);
            if (c + cc == '\n' + '\r')
              lex_lexeme_advance();
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
        token->klass = TOK_SEMICOLON;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 101:
      {
        token->klass = TOK_ANGLE_OPEN;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 102:
      {
        token->klass = TOK_ANGLE_CLOSE;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 103:
      {
        token->klass = TOK_DONTCARE;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 104:
      {
        token->klass = TOK_COLON;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 105:
      {
        token->klass = TOK_PARENTH_OPEN;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = TOK_PARENTH_CLOSE;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 107:
      {
        token->klass = TOK_PERIOD;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 108:
      {
        token->klass = TOK_BRACE_OPEN;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 109:
      {
        token->klass = TOK_BRACE_CLOSE;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 110:
      {
        token->klass = TOK_COMMA;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 111:
      {
        token->klass = TOK_MINUS;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 112:
      {
        token->klass = TOK_PLUS;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 113:
      {
        token->klass = TOK_STAR;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 114:
      {
        token->klass = TOK_SLASH;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 115:
      {
        if (lex_char_lookahead(1) == '=')
        {
          lex_char_advance();
          token->klass = TOK_EQUAL_EQUAL;
        }
        else
          token->klass = TOK_EQUAL;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 400:
      {
        if (c == '0')
        {
          c = lex_char_lookahead(1);
          if (c == 'x' || c == 'X')
          {
            state = 402;
            lex_lexeme_advance();
            break;
          }
          else if (c == 'o' || c == 'O')
          {
            state = 403;
            lex_lexeme_advance();
            break;
          }
          else if (c == 'b' || c == 'B')
          {
            state = 404;
            lex_lexeme_advance();
            break;
          }
        }
        state = 401;
      }
      break;

      case 401:
      {
        do
          c = lex_char_advance();
        while (cstr_is_digit(c));
        if (c == 'w')
          state = 405;
        else if (c == 's')
          state = 406;
        else
        {
          lex_char_retract();
          token->klass = TOK_INTEGER;
          token->lexeme = lex_lexeme_to_cstring();
          lex_lexeme_advance();
          state = 0;
        }
      }
      break;

      case 402:
      {
        do
          c = lex_char_advance();
        while (cstr_is_hex_digit(c));
        if (c == 'w')
          state = 407;
        else if (c == 's')
          state = 408;
        else
        {
          lex_char_retract();
          token->klass = TOK_INTEGER_HEX;
          token->lexeme = lex_lexeme_to_cstring();
          lex_lexeme_advance();
          state = 0;
        }
      }
      break;

      case 403:
      {
        do
          c = lex_char_advance();
        while (cstr_is_oct_digit(c));
        if (c == 'w')
          state = 409;
        else if (c == 's')
          state = 410;
        else
        {
          lex_char_retract();
          token->klass = TOK_INTEGER_OCT;
          token->lexeme = lex_lexeme_to_cstring();
          lex_lexeme_advance();
          state = 0;
        }
      }
      break;

      case 404:
      {
        do
          c = lex_char_advance();
        while (cstr_is_bin_digit(c));
        if (c == 'w')
          state = 411;
        else if (c == 's')
          state = 412;
        else
        {
          lex_char_retract();
          token->klass = TOK_INTEGER_BIN;
          token->lexeme = lex_lexeme_to_cstring();
          lex_lexeme_advance();
          state = 0;
        }
      }
      break;

      case 405:
      {
        do
          c = lex_char_advance();
        while (cstr_is_digit(c));
        lex_char_retract();
        token->klass = TOK_WINTEGER;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 406:
      {
        do
          c = lex_char_advance();
        while (cstr_is_digit(c));
        lex_char_retract();
        token->klass = TOK_SINTEGER;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 407:
      {
        do
          c = lex_char_advance();
        while (cstr_is_hex_digit(c));
        lex_char_retract();
        token->klass = TOK_WINTEGER_HEX;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 408:
      {
        do
          c = lex_char_advance();
        while (cstr_is_hex_digit(c));
        lex_char_retract();
        token->klass = TOK_SINTEGER_HEX;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 409:
      {
        do
          c = lex_char_advance();
        while (cstr_is_oct_digit(c));
        lex_char_retract();
        token->klass = TOK_WINTEGER_OCT;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 410:
      {
        do
          c = lex_char_advance();
        while (cstr_is_oct_digit(c));
        lex_char_retract();
        token->klass = TOK_SINTEGER_OCT;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 411:
      {
        do
          c = lex_char_advance();
        while (cstr_is_bin_digit(c) || c == '_');
        lex_char_retract();
        token->klass = TOK_WINTEGER_BIN;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 412:
      {
        do
          c = lex_char_advance();
        while (cstr_is_bin_digit(c) || c == '_');
        lex_char_retract();
        token->klass = TOK_SINTEGER_BIN;
        token->lexeme = lex_lexeme_to_cstring();
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 500:
      {
        do
          c = lex_char_advance();
        while (cstr_is_letter(c) || cstr_is_digit(c) || c == '_');
        lex_char_retract();
        token->klass = TOK_IDENT;
        token->lexeme = lex_lexeme_to_cstring();
        NamespaceInfo* ns = sym_get_namespace(token->lexeme);
        if (ns->ns_global)
        {
          IdentInfo* id_info = ns->ns_global;
          if (id_info->object_kind == IDOBJ_KEYWORD)
            token->klass = ((IdentInfo_Keyword*)id_info)->token_klass;
        }
        else if (ns->ns_type)
        {
          IdentInfo* id_info = ns->ns_type;
          if (id_info->object_kind == IDOBJ_TYPE || id_info->object_kind == IDOBJ_TYPEVAR)
            token->klass = TOK_TYPE_IDENT;
        }
        lex_lexeme_advance();
        state = 0;
      }
      break;

      case 2:
      {
        token->klass = TOK_EOI;
        token->lexeme = "<end-of-input>";
        state = 0;
      }
      break;

      case 3:
      {
        token->klass = TOK_UNKNOWN;
        token->lexeme = "<unknown>";
        lex_lexeme_advance();
        state = 0;
      }
      break;
    }
  }
}
