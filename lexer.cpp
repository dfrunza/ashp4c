#include <memory.h>
#include <stdio.h>
#include <basic.h>
#include <cstring.h>
#include <lexer.h>

void SourceText::read_source(char* filename)
{
  FILE* f_stream;
  char* text;

  f_stream = fopen(filename, "rb");
  if (!f_stream) {
    error("Could not open file '%s'.", filename);
  }
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  text = (char*)storage->malloc((text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  this->text = text;
  this->text_size = text_size;
  this->filename = filename;
}

char Lexer::char_lookahead(int pos)
{
  char* char_pos;

  char_pos = lexeme->end + pos;
  assert(char_pos >= (char*)0 && char_pos <= (text + text_size));
  return *char_pos;
}

char Lexer::char_advance(int pos)
{
  char* char_pos;

  char_pos = lexeme->end + pos;
  assert(char_pos >= (char*)0 && char_pos <= (text + text_size));
  lexeme->end = char_pos;
  return *char_pos;
}

char Lexer::char_retract()
{
  char result;

  result = *(--lexeme->end);
  assert(lexeme->end >= (char*)0);
  return result;
}

void Lexer::lexeme_advance()
{
  lexeme->start = ++lexeme->end;
  assert(lexeme->start <= (text + text_size));
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
  assert(cstring::is_digit(c, base) || c == '_');
  if (c != '_') {
    result = digit_to_integer(c, base);
  }
  for (c = *str++; c != '\0'; c = *str++) {
    if (cstring::is_digit(c, base)) {
      result = result*base + digit_to_integer(c, base);
    } else if (c == '_') {
      continue;
    } else assert(0);
  }
  return result;
}

void Lexer::token_install_integer(Token* token, Lexeme* lexeme, int base)
{
  char* string;

  string = lexeme_to_cstring(storage, lexeme);
  if (cstring::is_digit(*string, base) || *string == '_') {
    token->integer.value = parse_integer(string, base);
  } else {
    if (base == 10) {
      error("%s:%d:%d: error: expected one or more digits, got '%s'.",
            filename, token->line_no, token->column_no, string);
    } else if (base == 16) {
      error("%s:%d:%d: error: expected one or more hexadecimal digits, got '%s'.",
            filename, token->line_no, token->column_no, string);
    } else if (base == 8) {
      error("%s:%d:%d: error: expected one or more octal digits, got '%s'.",
            filename, token->line_no, token->column_no, string);
    } else if (base == 2) {
      error("%s:%d:%d: error: expected one or more binary digits, got '%s'.",
            filename, token->line_no, token->column_no, string);
    } else assert(0);
  }
}

void Lexer::next_token(Token* token)
{
  char c, cc;
  Token* prev_token;

  memset(token, 0, sizeof(Token));
  state = 1;
  while (state) {
    c = char_lookahead(0);
    switch (state) {
      default: assert(0);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          lexeme_advance();
          if (c == '\n' || c == '\r') {
            cc = char_lookahead(0);
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
        } else if (cstring::is_digit(c, 10)) {
          state = 400;
        } else if (cstring::is_letter(c)) {
          state = 500;
        } else if (c == '\0') {
          state = 2;
        } else {
          state = 3;
        }
      } break;

      case 2:
      {
        token->klass = TokenClass::END_OF_INPUT;
        token->lexeme = "<end-of-input>";
        state = 0;
      } break;

      case 3:
      {
        token->klass = TokenClass::UNKNOWN;
        token->lexeme = "<unknown>";
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 4:
      {
        token->klass = TokenClass::LEXICAL_ERROR;
        token->lexeme = "<error>";
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 100:
      {
        token->klass = TokenClass::SEMICOLON;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 101:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TokenClass::ANGLE_OPEN_EQUAL;
        } else if (char_lookahead(1) == '<') {
          char_advance(1);
          token->klass = TokenClass::DOUBLE_ANGLE_OPEN;
        } else {
          token->klass = TokenClass::ANGLE_OPEN;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 102:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TokenClass::ANGLE_CLOSE_EQUAL;
        } else if (char_lookahead(1) == '>') {
          char_advance(1);
          token->klass = TokenClass::DOUBLE_ANGLE_CLOSE;
        } else {
          token->klass = TokenClass::ANGLE_CLOSE;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 103:
      {
        cc = char_lookahead(1);
        if (cstring::is_letter(cc) || cstring::is_digit(cc, 10) || cc == '_') {
          state = 500;
        } else {
          token->klass = TokenClass::DONTCARE;
          token->lexeme = lexeme_to_cstring(storage, lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        }
      } break;

      case 104:
      {
        token->klass = TokenClass::COLON;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 105:
      {
        token->klass = TokenClass::PARENTH_OPEN;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = TokenClass::PARENTH_CLOSE;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 107:
      {
        token->klass = TokenClass::DOT;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 108:
      {
        token->klass = TokenClass::BRACE_OPEN;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 109:
      {
        token->klass = TokenClass::BRACE_CLOSE;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 110:
      {
        token->klass = TokenClass::BRACKET_OPEN;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 111:
      {
        token->klass = TokenClass::BRACKET_CLOSE;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 112:
      {
        token->klass = TokenClass::COMMA;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 113:
      {
        prev_token = (Token*)tokens->get(tokens->elem_count - 1);
        if (prev_token->klass == TokenClass::PARENTH_OPEN) {
          token->klass = TokenClass::UNARY_MINUS;
        } else {
          token->klass = TokenClass::MINUS;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 114:
      {
        token->klass = TokenClass::PLUS;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 115:
      {
        token->klass = TokenClass::STAR;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
          token->klass = TokenClass::SLASH;
          token->lexeme = lexeme_to_cstring(storage, lexeme);
          token->column_no = lexeme->start - line_start + 1;
          lexeme_advance();
          state = 0;
        }
      } break;

      case 117:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TokenClass::DOUBLE_EQUAL;
        } else {
          token->klass = TokenClass::EQUAL;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 118:
      {
        if (char_lookahead(1) == '=') {
          char_advance(1);
          token->klass = TokenClass::EXCLAMATION_EQUAL;
        } else {
          token->klass = TokenClass::EXCLAMATION;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
            token->klass = TokenClass::TRIPLE_AMPERSAND;
          } else {
            token->klass = TokenClass::DOUBLE_AMPERSAND;
          }
        } else {
          token->klass = TokenClass::AMPERSAND;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 120:
      {
        if (char_lookahead(1) == '|') {
          char_advance(1);
          token->klass = TokenClass::DOUBLE_PIPE;
        } else {
          token->klass = TokenClass::PIPE;
        }
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 121:
      {
        token->klass = TokenClass::CIRCUMFLEX;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 122:
      {
        token->klass = TokenClass::TILDA;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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

        token->klass = TokenClass::STRING_LITERAL;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
            if (c + cc == '\n' + '\r') {
              c = char_advance(1);
            }
            line_no += 1;
          }
        } while (c != '*');

        if (char_lookahead(1) == '/') {
          char_advance(1);
          token->klass = TokenClass::COMMENT;
          token->lexeme = lexeme_to_cstring(storage, lexeme);
          lexeme_advance();
          line_start = lexeme->start;
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
        token->klass = TokenClass::COMMENT;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        lexeme_advance();
        line_start = lexeme->start;
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
        } while (cstring::is_digit(c, 10));
        if (c == 'w' || c == 's') {
          token->klass = TokenClass::INTEGER_LITERAL;
          if (c == 's') {
            token->integer.is_signed = 1;
          }
          lexeme[1].end = lexeme->end - 1;  // omit w|s
          token->integer.width = parse_integer(lexeme_to_cstring(storage,&lexeme[1]), 10);
          char_advance(1);
          state = 405;
        } else {
          char_retract();
          lexeme[1].end = lexeme->end;
          token->klass = TokenClass::INTEGER_LITERAL;
          token->integer.is_signed = 1;
          token_install_integer(token, &lexeme[1], 10);
          token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 16) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(token, &lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 8) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(token, &lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 2) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::INTEGER_LITERAL;
        token->integer.is_signed = 1;
        token_install_integer(token, &lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 16) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 16);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 8) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 8);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 2) || c == '_');
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 2);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
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
        } while (cstring::is_digit(c, 10));
        char_retract();
        lexeme[1].end = lexeme->end;
        token_install_integer(token, &lexeme[1], 10);
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;

      case 500:
      {
        do {
          c = char_advance(1);
        } while (cstring::is_letter(c) || cstring::is_digit(c, 10) || c == '_');
        char_retract();
        token->klass = TokenClass::IDENTIFIER;
        token->lexeme = lexeme_to_cstring(storage, lexeme);
        token->column_no = lexeme->start - line_start + 1;
        lexeme_advance();
        state = 0;
      } break;
    }
  }
  token->line_no = line_no;
}

void Lexer::tokenize(SourceText* source_text)
{
  Token token = {};

  filename = source_text->filename;
  text = source_text->text;
  text_size = source_text->text_size;
  lexeme->start = lexeme->end = text;
  line_start = text;
  line_no = 1;

  token.klass = TokenClass::START_OF_INPUT;
  tokens = Array::create(storage, sizeof(Token), 7);
  *(Token*)tokens->append() = token;

  next_token(&token);
  *(Token*)tokens->append() = token;
  while (token.klass != TokenClass::END_OF_INPUT) {
    if (token.klass == TokenClass::UNKNOWN) {
      error("%s:%d:%d: error: unknown token.", filename, token.line_no, token.column_no);
    } else if (token.klass == TokenClass::LEXICAL_ERROR) {
      error("%s:%d:%d: error: lexical error.", filename, token.line_no, token.column_no);
    }
    next_token(&token);
    *(Token*)tokens->append() = token;
  }
}

