#include <token.h>

bool Token::is_nonTypeName()
{
  bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::APPLY || klass == TokenClass::KEY
                || klass == TokenClass::ACTIONS || klass == TokenClass::STATE || klass == TokenClass::ENTRIES;
  return result;
}

bool Token::is_name()
{
  bool result = is_nonTypeName() || klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

bool Token::is_typeName()
{
  return klass == TokenClass::TYPE_IDENTIFIER;
}

bool Token::is_nonTableKwName()
{
  bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::TYPE_IDENTIFIER
                || klass == TokenClass::APPLY || klass == TokenClass::STATE;
  return result;
}

bool Token::is_baseType()
{
  bool result = klass == TokenClass::BOOL || klass == TokenClass::ERROR || klass == TokenClass::INT
                || klass == TokenClass::BIT || klass == TokenClass::VARBIT || klass == TokenClass::STRING
                || klass == TokenClass::VOID;
  return result;
}

bool Token::is_typeRef()
{
  bool result = is_baseType() || klass == TokenClass::TYPE_IDENTIFIER || klass == TokenClass::TUPLE;
  return result;
}

bool Token::is_direction()
{
  bool result = klass == TokenClass::IN || klass == TokenClass::OUT || klass == TokenClass::INOUT;
  return result;
}

bool Token::is_parameter()
{
  bool result = is_direction() || is_typeRef();
  return result;
}

bool Token::is_derivedTypeDeclaration()
{
  bool result = klass == TokenClass::HEADER || klass == TokenClass::UNION || klass == TokenClass::STRUCT
                || klass == TokenClass::ENUM;
  return result;
}

bool Token::is_typeDeclaration()
{
  bool result = is_derivedTypeDeclaration() || klass == TokenClass::TYPEDEF
                || klass == TokenClass::PARSER || klass == TokenClass::CONTROL || klass == TokenClass::PACKAGE;
  return result;
}

bool Token::is_typeArg()
{
  bool result = klass == TokenClass::DONTCARE || is_typeRef() || is_nonTypeName();
  return result;
}

bool Token::is_typeOrVoid()
{
  bool result = is_typeRef() || klass == TokenClass::VOID || klass == TokenClass::IDENTIFIER;
  return result;
}

bool Token::is_actionRef()
{
  bool result = is_nonTypeName() || klass == TokenClass::PARENTH_OPEN;
  return result;
}

bool Token::is_tableProperty()
{
  bool result = klass == TokenClass::KEY || klass == TokenClass::ACTIONS;
#if 0
  || klass == TokenClass::CONST || klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
  return result;
}

bool Token::is_switchLabel()
{
  bool result = is_name() || klass == TokenClass::DEFAULT;
  return result;
}

bool Token::is_expressionPrimary()
{
  bool result = klass == TokenClass::INTEGER_LITERAL || klass == TokenClass::TRUE || klass == TokenClass::FALSE
                || klass == TokenClass::STRING_LITERAL || is_nonTypeName()
                || klass == TokenClass::BRACE_OPEN || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::EXCLAMATION
                || klass == TokenClass::TILDA || klass == TokenClass::UNARY_MINUS || is_typeName()
                || klass == TokenClass::ERROR || klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

bool Token::is_expression()
{
  return is_expressionPrimary();
}

bool Token::is_methodPrototype()
{
  return is_typeOrVoid() || klass == TokenClass::TYPE_IDENTIFIER;
}

bool Token::is_structField()
{
  bool result = is_typeRef();
  return result;
}

bool Token::is_specifiedIdentifier()
{
  return is_name();
}

bool Token::is_declaration()
{
  bool result = klass == TokenClass::CONST || klass == TokenClass::EXTERN || klass == TokenClass::ACTION
                || klass == TokenClass::PARSER || is_typeDeclaration() || klass == TokenClass::CONTROL
                || is_typeRef() || klass == TokenClass::ERROR || klass == TokenClass::MATCH_KIND
                || is_typeOrVoid();
  return result;
}

bool Token::is_lvalue()
{
  bool result = is_nonTypeName() || (klass == TokenClass::DOT);
  return result;
}

bool Token::is_assignmentOrMethodCallStatement()
{
  bool result = is_lvalue() || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::ANGLE_OPEN
                || klass == TokenClass::EQUAL;
  return result;
}

bool Token::is_statement()
{
  bool result = is_assignmentOrMethodCallStatement() || is_typeName() || klass == TokenClass::IF
                || klass == TokenClass::SEMICOLON || klass == TokenClass::BRACE_OPEN || klass == TokenClass::EXIT
                || klass == TokenClass::RETURN || klass == TokenClass::SWITCH;
  return result;
}

bool Token::is_statementOrDeclaration()
{
  bool result = is_typeRef() || klass == TokenClass::CONST || is_statement();
  return result;
}

bool Token::is_argument()
{
  bool result = is_expression() || is_name() || klass == TokenClass::DONTCARE;
  return result;
}

bool Token::is_parserLocalElement()
{
  bool result = klass == TokenClass::CONST || is_typeRef();
  return result;
}

bool Token::is_parserStatement()
{
  bool result = is_assignmentOrMethodCallStatement() || is_typeName()
                || klass == TokenClass::BRACE_OPEN || klass == TokenClass::CONST || is_typeRef()
                || klass == TokenClass::SEMICOLON;
  return result;
}

bool Token::is_simpleKeysetExpression()
{
  bool result = is_expression() || klass == TokenClass::DEFAULT || klass == TokenClass::DONTCARE;
  return result;
}

bool Token::is_keysetExpression()
{
  bool result = klass == TokenClass::TUPLE || is_simpleKeysetExpression();
  return result;
}

bool Token::is_selectCase()
{
  return is_keysetExpression();
}

bool Token::is_controlLocalDeclaration()
{
  bool result = klass == TokenClass::CONST || klass == TokenClass::ACTION
                || klass == TokenClass::TABLE || is_typeRef() || is_typeRef();
  return result;
}

bool Token::is_realTypeArg()
{
  bool result = klass == TokenClass::DONTCARE || is_typeRef();
  return result;
}

bool Token::is_binaryOperator()
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

bool Token::is_exprOperator()
{
  bool result = is_binaryOperator() || klass == TokenClass::DOT
                || klass == TokenClass::BRACKET_OPEN || klass == TokenClass::PARENTH_OPEN
                || klass == TokenClass::ANGLE_OPEN;
  return result;
}

