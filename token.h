#pragma once
#include <stdint.h>

enum class TokenClass {
  NONE = 0,

  /* Operators and syntactic structure */

  Semicolon,
  Identifier,
  TypeIdentifier,
  IntegerLiteral,
  StringLiteral,
  ParenthOpen,
  ParenthClose,
  AngleOpen,
  AngleClose,
  BraceOpen,
  BraceClose,
  BracketOpen,
  BracketClose,
  Dontcare,
  Colon,
  Dot,
  Comma,
  Minus,
  UnaryMinus,
  Plus,
  Star,
  Slash,
  Equal,
  DoubleEqual,
  ExclamationEqual,
  Exclamation,
  DoublePipe,
  AngleOpenEqual,
  AngleCloseEqual,
  Tilda,
  Ampersand,
  DoubleAmpersand,
  TripleAmpersand,
  Pipe,
  Circumflex,
  DoubleAngleOpen,
  DoubleAngleClose,
  Comment,

  /* Keywords */

  Action,
  Actions,
  Enum,
  In,
  Package,
  Select,
  Switch,
  Tuple,
  Void,
  Apply,
  Control,
  Error,
  Header,
  InOut,
  Parser,
  State,
  Table,
  Entries,
  Key,
  Typedef,
  Bool,
  True,
  False,
  Default,
  Extern,
  Union,
  Int,
  Bit,
  Varbit,
  String,
  Out,
  Transition,
  Else,
  Exit,
  If,
  MatchKind,
  Return,
  Struct,
  Const,

  /* Control */

  Unknown,
  StartOfInput,
  EndOfInput,
  LexicalError,
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
