#pragma once
#include "arena.h"
#include "array.h"

struct SourceText {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;

  void read_source(char* filename);
};

enum class TokenClass {
  NONE = 0,

  /* Operators and syntactic structure */

  SEMICOLON,
  IDENTIFIER,
  TYPE_IDENTIFIER,
  INTEGER_LITERAL,
  STRING_LITERAL,
  PARENTH_OPEN,
  PARENTH_CLOSE,
  ANGLE_OPEN,
  ANGLE_CLOSE,
  BRACE_OPEN,
  BRACE_CLOSE,
  BRACKET_OPEN,
  BRACKET_CLOSE,
  DONTCARE,
  COLON,
  DOT,
  COMMA,
  MINUS,
  UNARY_MINUS,
  PLUS,
  STAR,
  SLASH,
  EQUAL,
  DOUBLE_EQUAL,
  EXCLAMATION_EQUAL,
  EXCLAMATION,
  DOUBLE_PIPE,
  ANGLE_OPEN_EQUAL,
  ANGLE_CLOSE_EQUAL,
  TILDA,
  AMPERSAND,
  DOUBLE_AMPERSAND,
  TRIPLE_AMPERSAND,
  PIPE,
  CIRCUMFLEX,
  DOUBLE_ANGLE_OPEN,
  DOUBLE_ANGLE_CLOSE,
  COMMENT,

  /* Keywords */

  ACTION,
  ACTIONS,
  ENUM,
  IN,
  PACKAGE,
  SELECT,
  SWITCH,
  TUPLE,
  VOID,
  APPLY,
  CONTROL,
  ERROR,
  HEADER,
  INOUT,
  PARSER,
  STATE,
  TABLE,
  ENTRIES,
  KEY,
  TYPEDEF,
  BOOL,
  TRUE,
  FALSE,
  DEFAULT,
  EXTERN,
  UNION,
  INT,
  BIT,
  VARBIT,
  STRING,
  OUT,
  TRANSITION,
  ELSE,
  EXIT,
  IF,
  MATCH_KIND,
  RETURN,
  STRUCT,
  CONST,

  /* Control */

  UNKNOWN,
  START_OF_INPUT,
  END_OF_INPUT,
  LEXICAL_ERROR,
};

struct Token {
  enum TokenClass klass;
  char* lexeme;
  int line_no;
  int column_no;

  union {
    struct {
      bool is_signed;
      int width;
      int64_t value;
    } integer;
    char* str;
  };

  bool token_is_nonTypeName();
  bool token_is_name();
  bool token_is_typeName();
  bool token_is_nonTableKwName();
  bool token_is_baseType();
  bool token_is_typeRef();
  bool token_is_direction();
  bool token_is_parameter();
  bool token_is_derivedTypeDeclaration();
  bool token_is_typeDeclaration();
  bool token_is_typeArg();
  bool token_is_typeOrVoid();
  bool token_is_actionRef();
  bool token_is_tableProperty();
  bool token_is_switchLabel();
  bool token_is_expressionPrimary();
  bool token_is_expression();
  bool token_is_methodPrototype();
  bool token_is_structField();
  bool token_is_specifiedIdentifier();
  bool token_is_declaration();
  bool token_is_lvalue();
  bool token_is_assignmentOrMethodCallStatement();
  bool token_is_statement();
  bool token_is_statementOrDeclaration();
  bool token_is_argument();
  bool token_is_parserLocalElement();
  bool token_is_parserStatement();
  bool token_is_simpleKeysetExpression();
  bool token_is_keysetExpression();
  bool token_is_selectCase();
  bool token_is_controlLocalDeclaration();
  bool token_is_realTypeArg();
  bool token_is_binaryOperator();
  bool token_is_exprOperator();
};

struct Lexeme {
  char* start;
  char* end;
};

struct Lexer {
  Arena* storage;
  char*  text;
  int    text_size;
  char*  filename;
  int    line_no;
  char*  line_start;
  int    state;
  Token  token;
  Lexeme lexeme[2];
  Array* tokens;

  char char_lookahead(int pos);
  char char_advance(int pos);
  char char_retract();
  void lexeme_advance();
  void token_install_integer(Token* token, Lexeme* lexeme, int base);
  void next_token(Token* token);
  void tokenize(SourceText* source_text);
};
