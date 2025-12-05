#pragma once
#include <stdint.h>

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

  bool is_nonTypeName();
  bool is_name();
  bool is_typeName();
  bool is_nonTableKwName();
  bool is_baseType();
  bool is_typeRef();
  bool is_direction();
  bool is_parameter();
  bool is_derivedTypeDeclaration();
  bool is_typeDeclaration();
  bool is_typeArg();
  bool is_typeOrVoid();
  bool is_actionRef();
  bool is_tableProperty();
  bool is_switchLabel();
  bool is_expressionPrimary();
  bool is_expression();
  bool is_methodPrototype();
  bool is_structField();
  bool is_specifiedIdentifier();
  bool is_declaration();
  bool is_lvalue();
  bool is_assignmentOrMethodCallStatement();
  bool is_statement();
  bool is_statementOrDeclaration();
  bool is_argument();
  bool is_parserLocalElement();
  bool is_parserStatement();
  bool is_simpleKeysetExpression();
  bool is_keysetExpression();
  bool is_selectCase();
  bool is_controlLocalDeclaration();
  bool is_realTypeArg();
  bool is_binaryOperator();
  bool is_exprOperator();
};
