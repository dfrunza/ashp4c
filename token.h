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

  bool is_nonTypeName()
  {
    bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::APPLY || klass == TokenClass::KEY
                  || klass == TokenClass::ACTIONS || klass == TokenClass::STATE || klass == TokenClass::ENTRIES;
    return result;
  }

  bool is_name()
  {
    bool result = is_nonTypeName() || klass == TokenClass::TYPE_IDENTIFIER;
    return result;
  }

  bool is_typeName()
  {
    return klass == TokenClass::TYPE_IDENTIFIER;
  }

  bool is_nonTableKwName()
  {
    bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::TYPE_IDENTIFIER
                  || klass == TokenClass::APPLY || klass == TokenClass::STATE;
    return result;
  }

  bool is_baseType()
  {
    bool result = klass == TokenClass::BOOL || klass == TokenClass::ERROR || klass == TokenClass::INT
                  || klass == TokenClass::BIT || klass == TokenClass::VARBIT || klass == TokenClass::STRING
                  || klass == TokenClass::VOID;
    return result;
  }

  bool is_typeRef()
  {
    bool result = is_baseType() || klass == TokenClass::TYPE_IDENTIFIER || klass == TokenClass::TUPLE;
    return result;
  }

  bool is_direction()
  {
    bool result = klass == TokenClass::IN || klass == TokenClass::OUT || klass == TokenClass::INOUT;
    return result;
  }

  bool is_parameter()
  {
    bool result = is_direction() || is_typeRef();
    return result;
  }

  bool is_derivedTypeDeclaration()
  {
    bool result = klass == TokenClass::HEADER || klass == TokenClass::UNION || klass == TokenClass::STRUCT
                  || klass == TokenClass::ENUM;
    return result;
  }

  bool is_typeDeclaration()
  {
    bool result = is_derivedTypeDeclaration() || klass == TokenClass::TYPEDEF
                  || klass == TokenClass::PARSER || klass == TokenClass::CONTROL || klass == TokenClass::PACKAGE;
    return result;
  }

  bool is_typeArg()
  {
    bool result = klass == TokenClass::DONTCARE || is_typeRef() || is_nonTypeName();
    return result;
  }

  bool is_typeOrVoid()
  {
    bool result = is_typeRef() || klass == TokenClass::VOID || klass == TokenClass::IDENTIFIER;
    return result;
  }

  bool is_actionRef()
  {
    bool result = is_nonTypeName() || klass == TokenClass::PARENTH_OPEN;
    return result;
  }

  bool is_tableProperty()
  {
    bool result = klass == TokenClass::KEY || klass == TokenClass::ACTIONS;
#if 0
    || klass == TokenClass::CONST || klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
    return result;
  }

  bool is_switchLabel()
  {
    bool result = is_name() || klass == TokenClass::DEFAULT;
    return result;
  }

  bool is_expressionPrimary()
  {
    bool result = klass == TokenClass::INTEGER_LITERAL || klass == TokenClass::TRUE || klass == TokenClass::FALSE
                  || klass == TokenClass::STRING_LITERAL || is_nonTypeName()
                  || klass == TokenClass::BRACE_OPEN || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::EXCLAMATION
                  || klass == TokenClass::TILDA || klass == TokenClass::UNARY_MINUS || is_typeName()
                  || klass == TokenClass::ERROR || klass == TokenClass::TYPE_IDENTIFIER;
    return result;
  }

  bool is_expression()
  {
    return is_expressionPrimary();
  }

  bool is_methodPrototype()
  {
    return is_typeOrVoid() || klass == TokenClass::TYPE_IDENTIFIER;
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
    bool result = klass == TokenClass::CONST || klass == TokenClass::EXTERN || klass == TokenClass::ACTION
                  || klass == TokenClass::PARSER || is_typeDeclaration() || klass == TokenClass::CONTROL
                  || is_typeRef() || klass == TokenClass::ERROR || klass == TokenClass::MATCH_KIND
                  || is_typeOrVoid();
    return result;
  }

  bool is_lvalue()
  {
    bool result = is_nonTypeName() || (klass == TokenClass::DOT);
    return result;
  }

  bool is_assignmentOrMethodCallStatement()
  {
    bool result = is_lvalue() || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::ANGLE_OPEN
                  || klass == TokenClass::EQUAL;
    return result;
  }

  bool is_statement()
  {
    bool result = is_assignmentOrMethodCallStatement() || is_typeName() || klass == TokenClass::IF
                  || klass == TokenClass::SEMICOLON || klass == TokenClass::BRACE_OPEN || klass == TokenClass::EXIT
                  || klass == TokenClass::RETURN || klass == TokenClass::SWITCH;
    return result;
  }

  bool is_statementOrDeclaration()
  {
    bool result = is_typeRef() || klass == TokenClass::CONST || is_statement();
    return result;
  }

  bool is_argument()
  {
    bool result = is_expression() || is_name() || klass == TokenClass::DONTCARE;
    return result;
  }

  bool is_parserLocalElement()
  {
    bool result = klass == TokenClass::CONST || is_typeRef();
    return result;
  }

  bool is_parserStatement()
  {
    bool result = is_assignmentOrMethodCallStatement() || is_typeName()
                  || klass == TokenClass::BRACE_OPEN || klass == TokenClass::CONST || is_typeRef()
                  || klass == TokenClass::SEMICOLON;
    return result;
  }

  bool is_simpleKeysetExpression()
  {
    bool result = is_expression() || klass == TokenClass::DEFAULT || klass == TokenClass::DONTCARE;
    return result;
  }

  bool is_keysetExpression()
  {
    bool result = klass == TokenClass::TUPLE || is_simpleKeysetExpression();
    return result;
  }

  bool is_selectCase()
  {
    return is_keysetExpression();
  }

  bool is_controlLocalDeclaration()
  {
    bool result = klass == TokenClass::CONST || klass == TokenClass::ACTION
                  || klass == TokenClass::TABLE || is_typeRef() || is_typeRef();
    return result;
  }

  bool is_realTypeArg()
  {
    bool result = klass == TokenClass::DONTCARE || is_typeRef();
    return result;
  }

  bool is_binaryOperator()
  {
    bool result = klass == TokenClass::STAR || klass == TokenClass::SLASH
                  || klass == TokenClass::PLUS || klass == TokenClass::MINUS
                  || klass == TokenClass::ANGLE_OPEN_EQUAL || klass == TokenClass::ANGLE_CLOSE_EQUAL
                  || klass == TokenClass::ANGLE_OPEN || klass == TokenClass::ANGLE_CLOSE
                  || klass == TokenClass::EXCLAMATION_EQUAL || klass == TokenClass::DOUBLE_EQUAL
                  || klass == TokenClass::DOUBLE_PIPE || klass == TokenClass::DOUBLE_AMPERSAND
                  || klass == TokenClass::PIPE || klass == TokenClass::AMPERSAND
                  || klass == TokenClass::CIRCUMFLEX || klass == TokenClass::DOUBLE_ANGLE_OPEN
                  || klass == TokenClass::DOUBLE_ANGLE_CLOSE || klass == TokenClass::TRIPLE_AMPERSAND
                  || klass == TokenClass::EQUAL;
    return result;
  }

  bool is_exprOperator()
  {
    bool result = is_binaryOperator() || klass == TokenClass::DOT
                  || klass == TokenClass::BRACKET_OPEN || klass == TokenClass::PARENTH_OPEN
                  || klass == TokenClass::ANGLE_OPEN;
    return result;
  }
};
