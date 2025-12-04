#include <token.h>

bool Token::token_is_nonTypeName()
{
  bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::APPLY || klass == TokenClass::KEY
                || klass == TokenClass::ACTIONS || klass == TokenClass::STATE || klass == TokenClass::ENTRIES;
  return result;
}

bool Token::token_is_name()
{
  bool result = token_is_nonTypeName() || klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

bool Token::token_is_typeName()
{
  return klass == TokenClass::TYPE_IDENTIFIER;
}

bool Token::token_is_nonTableKwName()
{
  bool result = klass == TokenClass::IDENTIFIER || klass == TokenClass::TYPE_IDENTIFIER
                || klass == TokenClass::APPLY || klass == TokenClass::STATE;
  return result;
}

bool Token::token_is_baseType()
{
  bool result = klass == TokenClass::BOOL || klass == TokenClass::ERROR || klass == TokenClass::INT
                || klass == TokenClass::BIT || klass == TokenClass::VARBIT || klass == TokenClass::STRING
                || klass == TokenClass::VOID;
  return result;
}

bool Token::token_is_typeRef()
{
  bool result = token_is_baseType() || klass == TokenClass::TYPE_IDENTIFIER || klass == TokenClass::TUPLE;
  return result;
}

bool Token::token_is_direction()
{
  bool result = klass == TokenClass::IN || klass == TokenClass::OUT || klass == TokenClass::INOUT;
  return result;
}

bool Token::token_is_parameter()
{
  bool result = token_is_direction() || token_is_typeRef();
  return result;
}

bool Token::token_is_derivedTypeDeclaration()
{
  bool result = klass == TokenClass::HEADER || klass == TokenClass::UNION || klass == TokenClass::STRUCT
                || klass == TokenClass::ENUM;
  return result;
}

bool Token::token_is_typeDeclaration()
{
  bool result = token_is_derivedTypeDeclaration() || klass == TokenClass::TYPEDEF
                || klass == TokenClass::PARSER || klass == TokenClass::CONTROL || klass == TokenClass::PACKAGE;
  return result;
}

bool Token::token_is_typeArg()
{
  bool result = klass == TokenClass::DONTCARE || token_is_typeRef() || token_is_nonTypeName();
  return result;
}

bool Token::token_is_typeOrVoid()
{
  bool result = token_is_typeRef() || klass == TokenClass::VOID || klass == TokenClass::IDENTIFIER;
  return result;
}

bool Token::token_is_actionRef()
{
  bool result = token_is_nonTypeName() || klass == TokenClass::PARENTH_OPEN;
  return result;
}

bool Token::token_is_tableProperty()
{
  bool result = klass == TokenClass::KEY || klass == TokenClass::ACTIONS;
#if 0
  || klass == TokenClass::CONST || klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
  return result;
}

bool Token::token_is_switchLabel()
{
  bool result = token_is_name() || klass == TokenClass::DEFAULT;
  return result;
}

bool Token::token_is_expressionPrimary()
{
  bool result = klass == TokenClass::INTEGER_LITERAL || klass == TokenClass::TRUE || klass == TokenClass::FALSE
                || klass == TokenClass::STRING_LITERAL || token_is_nonTypeName()
                || klass == TokenClass::BRACE_OPEN || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::EXCLAMATION
                || klass == TokenClass::TILDA || klass == TokenClass::UNARY_MINUS || token_is_typeName()
                || klass == TokenClass::ERROR || klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

bool Token::token_is_expression()
{
  return token_is_expressionPrimary();
}

bool Token::token_is_methodPrototype()
{
  return token_is_typeOrVoid() || klass == TokenClass::TYPE_IDENTIFIER;
}

bool Token::token_is_structField()
{
  bool result = token_is_typeRef();
  return result;
}

bool Token::token_is_specifiedIdentifier()
{
  return token_is_name();
}

bool Token::token_is_declaration()
{
  bool result = klass == TokenClass::CONST || klass == TokenClass::EXTERN || klass == TokenClass::ACTION
                || klass == TokenClass::PARSER || token_is_typeDeclaration() || klass == TokenClass::CONTROL
                || token_is_typeRef() || klass == TokenClass::ERROR || klass == TokenClass::MATCH_KIND
                || token_is_typeOrVoid();
  return result;
}

bool Token::token_is_lvalue()
{
  bool result = token_is_nonTypeName() || (klass == TokenClass::DOT);
  return result;
}

bool Token::token_is_assignmentOrMethodCallStatement()
{
  bool result = token_is_lvalue() || klass == TokenClass::PARENTH_OPEN || klass == TokenClass::ANGLE_OPEN
                || klass == TokenClass::EQUAL;
  return result;
}

bool Token::token_is_statement()
{
  bool result = token_is_assignmentOrMethodCallStatement() || token_is_typeName() || klass == TokenClass::IF
                || klass == TokenClass::SEMICOLON || klass == TokenClass::BRACE_OPEN || klass == TokenClass::EXIT
                || klass == TokenClass::RETURN || klass == TokenClass::SWITCH;
  return result;
}

bool Token::token_is_statementOrDeclaration()
{
  bool result = token_is_typeRef() || klass == TokenClass::CONST || token_is_statement();
  return result;
}

bool Token::token_is_argument()
{
  bool result = token_is_expression() || token_is_name() || klass == TokenClass::DONTCARE;
  return result;
}

bool Token::token_is_parserLocalElement()
{
  bool result = klass == TokenClass::CONST || token_is_typeRef();
  return result;
}

bool Token::token_is_parserStatement()
{
  bool result = token_is_assignmentOrMethodCallStatement() || token_is_typeName()
                || klass == TokenClass::BRACE_OPEN || klass == TokenClass::CONST || token_is_typeRef()
                || klass == TokenClass::SEMICOLON;
  return result;
}

bool Token::token_is_simpleKeysetExpression()
{
  bool result = token_is_expression() || klass == TokenClass::DEFAULT || klass == TokenClass::DONTCARE;
  return result;
}

bool Token::token_is_keysetExpression()
{
  bool result = klass == TokenClass::TUPLE || token_is_simpleKeysetExpression();
  return result;
}

bool Token::token_is_selectCase()
{
  return token_is_keysetExpression();
}

bool Token::token_is_controlLocalDeclaration()
{
  bool result = klass == TokenClass::CONST || klass == TokenClass::ACTION
                || klass == TokenClass::TABLE || token_is_typeRef() || token_is_typeRef();
  return result;
}

bool Token::token_is_realTypeArg()
{
  bool result = klass == TokenClass::DONTCARE|| token_is_typeRef();
  return result;
}

bool Token::token_is_binaryOperator()
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

bool Token::token_is_exprOperator()
{
  bool result = token_is_binaryOperator() || klass == TokenClass::DOT
                || klass == TokenClass::BRACKET_OPEN || klass == TokenClass::PARENTH_OPEN
                || klass == TokenClass::ANGLE_OPEN;
  return result;
}

