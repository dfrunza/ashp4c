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

  bool is_nonTypeName()
  {
    bool result = klass == TokenClass::Identifier || klass == TokenClass::Apply || klass == TokenClass::Key
                  || klass == TokenClass::Actions || klass == TokenClass::State || klass == TokenClass::Entries;
    return result;
  }

  bool is_name()
  {
    bool result = is_nonTypeName() || klass == TokenClass::TypeIdentifier;
    return result;
  }

  bool is_typeName()
  {
    return klass == TokenClass::TypeIdentifier;
  }

  bool is_nonTableKwName()
  {
    bool result = klass == TokenClass::Identifier || klass == TokenClass::TypeIdentifier
                  || klass == TokenClass::Apply || klass == TokenClass::State;
    return result;
  }

  bool is_baseType()
  {
    bool result = klass == TokenClass::Bool || klass == TokenClass::Error || klass == TokenClass::Int
                  || klass == TokenClass::Bit || klass == TokenClass::Varbit || klass == TokenClass::String
                  || klass == TokenClass::Void;
    return result;
  }

  bool is_typeRef()
  {
    bool result = is_baseType() || klass == TokenClass::TypeIdentifier || klass == TokenClass::Tuple;
    return result;
  }

  bool is_direction()
  {
    bool result = klass == TokenClass::In || klass == TokenClass::Out || klass == TokenClass::InOut;
    return result;
  }

  bool is_parameter()
  {
    bool result = is_direction() || is_typeRef();
    return result;
  }

  bool is_derivedTypeDeclaration()
  {
    bool result = klass == TokenClass::Header || klass == TokenClass::Union || klass == TokenClass::Struct
                  || klass == TokenClass::Enum;
    return result;
  }

  bool is_typeDeclaration()
  {
    bool result = is_derivedTypeDeclaration() || klass == TokenClass::Typedef
                  || klass == TokenClass::Parser || klass == TokenClass::Control || klass == TokenClass::Package;
    return result;
  }

  bool is_typeArg()
  {
    bool result = klass == TokenClass::Dontcare || is_typeRef() || is_nonTypeName();
    return result;
  }

  bool is_typeOrVoid()
  {
    bool result = is_typeRef() || klass == TokenClass::Void || klass == TokenClass::Identifier;
    return result;
  }

  bool is_actionRef()
  {
    bool result = is_nonTypeName() || klass == TokenClass::ParenthOpen;
    return result;
  }

  bool is_tableProperty()
  {
    bool result = klass == TokenClass::Key || klass == TokenClass::Actions;
#if 0
    || klass == TokenClass::CONST || klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
    return result;
  }

  bool is_switchLabel()
  {
    bool result = is_name() || klass == TokenClass::Default;
    return result;
  }

  bool is_expressionPrimary()
  {
    bool result = klass == TokenClass::IntegerLiteral || klass == TokenClass::True || klass == TokenClass::False
                  || klass == TokenClass::StringLiteral || is_nonTypeName()
                  || klass == TokenClass::BraceOpen || klass == TokenClass::ParenthOpen || klass == TokenClass::Exclamation
                  || klass == TokenClass::Tilda || klass == TokenClass::UnaryMinus || is_typeName()
                  || klass == TokenClass::Error || klass == TokenClass::TypeIdentifier;
    return result;
  }

  bool is_expression()
  {
    return is_expressionPrimary();
  }

  bool is_methodPrototype()
  {
    return is_typeOrVoid() || klass == TokenClass::TypeIdentifier;
  }

  bool is_structField()
  {
    bool result = is_typeRef();
    return result;
  }

  bool is_specifiedIdentifier()
  {
    return is_name();
  }

  bool is_declaration()
  {
    bool result = klass == TokenClass::Const || klass == TokenClass::Extern || klass == TokenClass::Action
                  || klass == TokenClass::Parser || is_typeDeclaration() || klass == TokenClass::Control
                  || is_typeRef() || klass == TokenClass::Error || klass == TokenClass::MatchKind
                  || is_typeOrVoid();
    return result;
  }

  bool is_lvalue()
  {
    bool result = is_nonTypeName() || (klass == TokenClass::Dot);
    return result;
  }

  bool is_assignmentOrMethodCallStatement()
  {
    bool result = is_lvalue() || klass == TokenClass::ParenthOpen || klass == TokenClass::AngleOpen
                  || klass == TokenClass::Equal;
    return result;
  }

  bool is_statement()
  {
    bool result = is_assignmentOrMethodCallStatement() || is_typeName() || klass == TokenClass::If
                  || klass == TokenClass::Semicolon || klass == TokenClass::BraceOpen || klass == TokenClass::Exit
                  || klass == TokenClass::Return || klass == TokenClass::Switch;
    return result;
  }

  bool is_statementOrDeclaration()
  {
    bool result = is_typeRef() || klass == TokenClass::Const || is_statement();
    return result;
  }

  bool is_argument()
  {
    bool result = is_expression() || is_name() || klass == TokenClass::Dontcare;
    return result;
  }

  bool is_parserLocalElement()
  {
    bool result = klass == TokenClass::Const || is_typeRef();
    return result;
  }

  bool is_parserStatement()
  {
    bool result = is_assignmentOrMethodCallStatement() || is_typeName()
                  || klass == TokenClass::BraceOpen || klass == TokenClass::Const || is_typeRef()
                  || klass == TokenClass::Semicolon;
    return result;
  }

  bool is_simpleKeysetExpression()
  {
    bool result = is_expression() || klass == TokenClass::Default || klass == TokenClass::Dontcare;
    return result;
  }

  bool is_keysetExpression()
  {
    bool result = klass == TokenClass::Tuple || is_simpleKeysetExpression();
    return result;
  }

  bool is_selectCase()
  {
    return is_keysetExpression();
  }

  bool is_controlLocalDeclaration()
  {
    bool result = klass == TokenClass::Const || klass == TokenClass::Action
                  || klass == TokenClass::Table || is_typeRef() || is_typeRef();
    return result;
  }

  bool is_realTypeArg()
  {
    bool result = klass == TokenClass::Dontcare || is_typeRef();
    return result;
  }

  bool is_binaryOperator()
  {
    bool result = klass == TokenClass::Star || klass == TokenClass::Slash
                  || klass == TokenClass::Plus || klass == TokenClass::Minus
                  || klass == TokenClass::AngleOpenEqual || klass == TokenClass::AngleCloseEqual
                  || klass == TokenClass::AngleOpen || klass == TokenClass::AngleClose
                  || klass == TokenClass::ExclamationEqual || klass == TokenClass::DoubleEqual
                  || klass == TokenClass::DoublePipe || klass == TokenClass::DoubleAmpersand
                  || klass == TokenClass::Pipe || klass == TokenClass::Ampersand
                  || klass == TokenClass::Circumflex || klass == TokenClass::DoubleAngleOpen
                  || klass == TokenClass::DoubleAngleClose || klass == TokenClass::TripleAmpersand
                  || klass == TokenClass::Equal;
    return result;
  }

  bool is_exprOperator()
  {
    bool result = is_binaryOperator() || klass == TokenClass::Dot
                  || klass == TokenClass::BracketOpen || klass == TokenClass::ParenthOpen
                  || klass == TokenClass::AngleOpen;
    return result;
  }
};
