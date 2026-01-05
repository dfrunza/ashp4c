#include "token.h"

bool Token::is_nonTypeName()
{
  bool result = klass == TokenClass::Identifier || klass == TokenClass::Apply || klass == TokenClass::Key
                || klass == TokenClass::Actions || klass == TokenClass::State || klass == TokenClass::Entries;
  return result;
}

bool Token::is_name()
{
  bool result = is_nonTypeName() || klass == TokenClass::TypeIdentifier;
  return result;
}

bool Token::is_typeName()
{
  return klass == TokenClass::TypeIdentifier;
}

bool Token::is_nonTableKwName()
{
  bool result = klass == TokenClass::Identifier || klass == TokenClass::TypeIdentifier
                || klass == TokenClass::Apply || klass == TokenClass::State;
  return result;
}

bool Token::is_baseType()
{
  bool result = klass == TokenClass::Bool || klass == TokenClass::Error || klass == TokenClass::Int
                || klass == TokenClass::Bit || klass == TokenClass::Varbit || klass == TokenClass::String
                || klass == TokenClass::Void;
  return result;
}

bool Token::is_typeRef()
{
  bool result = is_baseType() || klass == TokenClass::TypeIdentifier || klass == TokenClass::Tuple;
  return result;
}

bool Token::is_direction()
{
  bool result = klass == TokenClass::In || klass == TokenClass::Out || klass == TokenClass::InOut;
  return result;
}

bool Token::is_parameter()
{
  bool result = is_direction() || is_typeRef();
  return result;
}

bool Token::is_derivedTypeDeclaration()
{
  bool result = klass == TokenClass::Header || klass == TokenClass::Union || klass == TokenClass::Struct
                || klass == TokenClass::Enum;
  return result;
}

bool Token::is_typeDeclaration()
{
  bool result = is_derivedTypeDeclaration() || klass == TokenClass::Typedef
                || klass == TokenClass::Parser || klass == TokenClass::Control || klass == TokenClass::Package;
  return result;
}

bool Token::is_typeArg()
{
  bool result = klass == TokenClass::Dontcare || is_typeRef() || is_nonTypeName();
  return result;
}

bool Token::is_typeOrVoid()
{
  bool result = is_typeRef() || klass == TokenClass::Void || klass == TokenClass::Identifier;
  return result;
}

bool Token::is_actionRef()
{
  bool result = is_nonTypeName() || klass == TokenClass::ParenthOpen;
  return result;
}

bool Token::is_tableProperty()
{
  bool result = klass == TokenClass::Key || klass == TokenClass::Actions;
#if 0
  || klass == TokenClass::CONST || klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
  return result;
}

bool Token::is_switchLabel()
{
  bool result = is_name() || klass == TokenClass::Default;
  return result;
}

bool Token::is_expressionPrimary()
{
  bool result = klass == TokenClass::IntegerLiteral || klass == TokenClass::True || klass == TokenClass::False
                || klass == TokenClass::StringLiteral || is_nonTypeName()
                || klass == TokenClass::BraceOpen || klass == TokenClass::ParenthOpen || klass == TokenClass::Exclamation
                || klass == TokenClass::Tilda || klass == TokenClass::UnaryMinus || is_typeName()
                || klass == TokenClass::Error || klass == TokenClass::TypeIdentifier;
  return result;
}

bool Token::is_expression()
{
  return is_expressionPrimary();
}

bool Token::is_methodPrototype()
{
  return is_typeOrVoid() || klass == TokenClass::TypeIdentifier;
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
  bool result = klass == TokenClass::Const || klass == TokenClass::Extern || klass == TokenClass::Action
                || klass == TokenClass::Parser || is_typeDeclaration() || klass == TokenClass::Control
                || is_typeRef() || klass == TokenClass::Error || klass == TokenClass::MatchKind
                || is_typeOrVoid();
  return result;
}

bool Token::is_lvalue()
{
  bool result = is_nonTypeName() || (klass == TokenClass::Dot);
  return result;
}

bool Token::is_assignmentOrMethodCallStatement()
{
  bool result = is_lvalue() || klass == TokenClass::ParenthOpen || klass == TokenClass::AngleOpen
                || klass == TokenClass::Equal;
  return result;
}

bool Token::is_statement()
{
  bool result = is_assignmentOrMethodCallStatement() || is_typeName() || klass == TokenClass::If
                || klass == TokenClass::Semicolon || klass == TokenClass::BraceOpen || klass == TokenClass::Exit
                || klass == TokenClass::Return || klass == TokenClass::Switch;
  return result;
}

bool Token::is_statementOrDeclaration()
{
  bool result = is_typeRef() || klass == TokenClass::Const || is_statement();
  return result;
}

bool Token::is_argument()
{
  bool result = is_expression() || is_name() || klass == TokenClass::Dontcare;
  return result;
}

bool Token::is_parserLocalElement()
{
  bool result = klass == TokenClass::Const || is_typeRef();
  return result;
}

bool Token::is_parserStatement()
{
  bool result = is_assignmentOrMethodCallStatement() || is_typeName()
                || klass == TokenClass::BraceOpen || klass == TokenClass::Const || is_typeRef()
                || klass == TokenClass::Semicolon;
  return result;
}

bool Token::is_simpleKeysetExpression()
{
  bool result = is_expression() || klass == TokenClass::Default || klass == TokenClass::Dontcare;
  return result;
}

bool Token::is_keysetExpression()
{
  bool result = klass == TokenClass::Tuple || is_simpleKeysetExpression();
  return result;
}

bool Token::is_selectCase()
{
  return is_keysetExpression();
}

bool Token::is_controlLocalDeclaration()
{
  bool result = klass == TokenClass::Const || klass == TokenClass::Action
                || klass == TokenClass::Table || is_typeRef() || is_typeRef();
  return result;
}

bool Token::is_realTypeArg()
{
  bool result = klass == TokenClass::Dontcare || is_typeRef();
  return result;
}

bool Token::is_binaryOperator()
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

bool Token::is_exprOperator()
{
  bool result = is_binaryOperator() || klass == TokenClass::Dot
                || klass == TokenClass::BracketOpen || klass == TokenClass::ParenthOpen
                || klass == TokenClass::AngleOpen;
  return result;
}
