#include <memory.h>
#include <basic.h>
#include <cstring.h>
#include <lexer.h>

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

  char c = *str++;
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

void Lexer::to_integer_token(Token* token, Lexeme* lexeme, int base)
{
  char* string = lexeme->to_cstring(storage);
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
  memset(token, 0, sizeof(Token));
  state = 1;
  while (state) {
    char c = lookahead_char(0);
    switch (state) {
      default: assert(0);

      case 1:
      {
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
          advance_lexeme();
          if (c == '\n' || c == '\r') {
            char cc = lookahead_char(0);
            if (c + cc == '\n' + '\r') {
              advance_lexeme();
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
        token->klass = TokenClass::EndOfInput;
        token->lexeme = "<end-of-input>";
        state = 0;
      } break;

      case 3:
      {
        token->klass = TokenClass::Unknown;
        token->lexeme = "<unknown>";
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 4:
      {
        token->klass = TokenClass::LexicalError;
        token->lexeme = "<error>";
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 100:
      {
        token->klass = TokenClass::Semicolon;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 101:
      {
        if (lookahead_char(1) == '=') {
          advance_char(1);
          token->klass = TokenClass::AngleOpenEqual;
        } else if (lookahead_char(1) == '<') {
          advance_char(1);
          token->klass = TokenClass::DoubleAngleOpen;
        } else {
          token->klass = TokenClass::AngleOpen;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 102:
      {
        if (lookahead_char(1) == '=') {
          advance_char(1);
          token->klass = TokenClass::AngleCloseEqual;
        } else if (lookahead_char(1) == '>') {
          advance_char(1);
          token->klass = TokenClass::DoubleAngleClose;
        } else {
          token->klass = TokenClass::AngleClose;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 103:
      {
        char cc = lookahead_char(1);
        if (cstring::is_letter(cc) || cstring::is_digit(cc, 10) || cc == '_') {
          state = 500;
        } else {
          token->klass = TokenClass::Dontcare;
          token->lexeme = lexeme->to_cstring(storage);
          token->column_no = lexeme->start - line_start + 1;
          advance_lexeme();
          state = 0;
        }
      } break;

      case 104:
      {
        token->klass = TokenClass::Colon;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 105:
      {
        token->klass = TokenClass::ParenthOpen;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      }
      break;

      case 106:
      {
        token->klass = TokenClass::ParenthClose;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 107:
      {
        token->klass = TokenClass::Dot;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 108:
      {
        token->klass = TokenClass::BraceOpen;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 109:
      {
        token->klass = TokenClass::BraceClose;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 110:
      {
        token->klass = TokenClass::BracketOpen;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 111:
      {
        token->klass = TokenClass::BracketClose;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 112:
      {
        token->klass = TokenClass::Comma;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 113:
      {
        Token* prev_token = tokens->get(tokens->element_count - 1);
        if (prev_token->klass == TokenClass::ParenthOpen) {
          token->klass = TokenClass::UnaryMinus;
        } else {
          token->klass = TokenClass::Minus;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 114:
      {
        token->klass = TokenClass::Plus;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 115:
      {
        token->klass = TokenClass::Star;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 116:
      {
        if (lookahead_char(1) == '*') {
          advance_char(1);
          state = 310;
        } else if (lookahead_char(1) == '/') {
          state = 311;
        } else {
          token->klass = TokenClass::Slash;
          token->lexeme = lexeme->to_cstring(storage);
          token->column_no = lexeme->start - line_start + 1;
          advance_lexeme();
          state = 0;
        }
      } break;

      case 117:
      {
        if (lookahead_char(1) == '=') {
          advance_char(1);
          token->klass = TokenClass::DoubleEqual;
        } else {
          token->klass = TokenClass::Equal;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 118:
      {
        if (lookahead_char(1) == '=') {
          advance_char(1);
          token->klass = TokenClass::ExclamationEqual;
        } else {
          token->klass = TokenClass::Exclamation;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 119:
      {
        if (lookahead_char(1) == '&') {
          advance_char(1);
          if (lookahead_char(1) == '&') {
            advance_char(1);
            token->klass = TokenClass::TripleAmpersand;
          } else {
            token->klass = TokenClass::DoubleAmpersand;
          }
        } else {
          token->klass = TokenClass::Ampersand;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 120:
      {
        if (lookahead_char(1) == '|') {
          advance_char(1);
          token->klass = TokenClass::DoublePipe;
        } else {
          token->klass = TokenClass::Pipe;
        }
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 121:
      {
        token->klass = TokenClass::Circumflex;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 122:
      {
        token->klass = TokenClass::Tilda;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 200:
      {
        do {
          c = advance_char(1);
          if (c == '\\') {
            state = 201;
            break;
          } else if (c == '\n' || c == '\r') {
            state = 4;
          }
        } while (c != '"');

        token->klass = TokenClass::StringLiteral;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 201:
      {
        c = advance_char(1);
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
          c = advance_char(1);
          if (c == '\n' || c == '\r') {
            char cc = lookahead_char(1);
            if (c + cc == '\n' + '\r') {
              c = advance_char(1);
            }
            line_no += 1;
          }
        } while (c != '*');

        if (lookahead_char(1) == '/') {
          advance_char(1);
          token->klass = TokenClass::Comment;
          token->lexeme = lexeme->to_cstring(storage);
          advance_lexeme();
          line_start = lexeme->start;
          state = 0;
        } else {
          state = 310;
        }
      } break;

      case 311:
      {
        do {
          c = advance_char(1);
        } while (c != '\n' && c != '\r');

        line_no += 1;
        token->klass = TokenClass::Comment;
        token->lexeme = lexeme->to_cstring(storage);
        advance_lexeme();
        line_start = lexeme->start;
        state = 0;
      } break;

      case 400:
      {
        if (c == '0') {
          c = lookahead_char(1);
          if (c == 'x' || c == 'X') {
            state = 402;
            advance_char(2);
            break;
          } else if (c == 'o' || c == 'O') {
            state = 403;
            advance_char(2);
            break;
          } else if (c == 'b' || c == 'B') {
            state = 404;
            advance_char(2);
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
          c = advance_char(1);
        } while (cstring::is_digit(c, 10));
        if (c == 'w' || c == 's') {
          token->klass = TokenClass::IntegerLiteral;
          if (c == 's') {
            token->integer.is_signed = 1;
          }
          lexeme[1].end = lexeme->end - 1;  // omit w|s
          token->integer.width = parse_integer(lexeme[1].to_cstring(storage), 10);
          advance_char(1);
          state = 405;
        } else {
          retract_char();
          lexeme[1].end = lexeme->end;
          token->klass = TokenClass::IntegerLiteral;
          token->integer.is_signed = 1;
          to_integer_token(token, &lexeme[1], 10);
          token->lexeme = lexeme->to_cstring(storage);
          token->column_no = lexeme->start - line_start + 1;
          advance_lexeme();
          state = 0;
        }
      } break;

      case 402:
      {
        // 0xFF
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 16) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::IntegerLiteral;
        token->integer.is_signed = 1;
        to_integer_token(token, &lexeme[1], 16);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 403:
      {
        // 0o77
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 8) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::IntegerLiteral;
        token->integer.is_signed = 1;
        to_integer_token(token, &lexeme[1], 8);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 404:
      {
        // 0b11
        //   ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 2) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        token->klass = TokenClass::IntegerLiteral;
        token->integer.is_signed = 1;
        to_integer_token(token, &lexeme[1], 2);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 405:
      {
        if (c == '0') {
          c = lookahead_char(1);
          if (c == 'x' || c == 'X') {
            state = 406;
            advance_char(2);
            break;
          } else if (c == 'o' || c == 'O') {
            state = 407;
            advance_char(2);
            break;
          } else if (c == 'b' || c == 'B') {
            state = 408;
            advance_char(2);
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
          c = advance_char(1);
        } while (cstring::is_digit(c, 16) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        to_integer_token(token, &lexeme[1], 16);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 407:
      {
        // ..(w|s)0o77
        //          ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 8) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        to_integer_token(token, &lexeme[1], 8);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 408:
      {
        // ..(w|s)0b11
        //          ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 2) || c == '_');
        retract_char();
        lexeme[1].end = lexeme->end;
        to_integer_token(token, &lexeme[1], 2);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 409:
      {
        // ..(w|s)99
        //        ^^
        lexeme[1].start = lexeme[1].end = lexeme->end;
        do {
          c = advance_char(1);
        } while (cstring::is_digit(c, 10));
        retract_char();
        lexeme[1].end = lexeme->end;
        to_integer_token(token, &lexeme[1], 10);
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
        state = 0;
      } break;

      case 500:
      {
        do {
          c = advance_char(1);
        } while (cstring::is_letter(c) || cstring::is_digit(c, 10) || c == '_');
        retract_char();
        token->klass = TokenClass::Identifier;
        token->lexeme = lexeme->to_cstring(storage);
        token->column_no = lexeme->start - line_start + 1;
        advance_lexeme();
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

  token.klass = TokenClass::StartOfInput;
  tokens = Array<Token>::create(storage, 7);
  *tokens->append() = token;

  next_token(&token);
  *tokens->append() = token;
  while (token.klass != TokenClass::EndOfInput) {
    if (token.klass == TokenClass::Unknown) {
      error("%s:%d:%d: error: unknown token.", filename, token.line_no, token.column_no);
    } else if (token.klass == TokenClass::LexicalError) {
      error("%s:%d:%d: error: lexical error.", filename, token.line_no, token.column_no);
    }
    next_token(&token);
    *tokens->append() = token;
  }
}
