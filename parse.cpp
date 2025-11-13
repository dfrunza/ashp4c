#include "foundation.h"
#include "frontend.h"

struct Keyword {
  char* strname;
  enum TokenClass token_class;
};

static void define_keywords(Parser* parser, Scope* scope)
{
  struct Keyword keywords[] = {
    {"action",  TokenClass::ACTION},
    {"actions", TokenClass::ACTIONS},
    {"entries", TokenClass::ENTRIES},
    {"enum",    TokenClass::ENUM},
    {"in",      TokenClass::IN},
    {"package", TokenClass::PACKAGE},
    {"select",  TokenClass::SELECT},
    {"switch",  TokenClass::SWITCH},
    {"tuple",   TokenClass::TUPLE},
    {"control", TokenClass::CONTROL},
    {"error",   TokenClass::ERROR},
    {"header",  TokenClass::HEADER},
    {"inout",   TokenClass::INOUT},
    {"parser",  TokenClass::PARSER},
    {"state",   TokenClass::STATE},
    {"table",   TokenClass::TABLE},
    {"key",     TokenClass::KEY},
    {"typedef", TokenClass::TYPEDEF},
    {"default", TokenClass::DEFAULT},
    {"extern",  TokenClass::EXTERN},
    {"out",     TokenClass::OUT},
    {"else",    TokenClass::ELSE},
    {"exit",    TokenClass::EXIT},
    {"if",      TokenClass::IF},
    {"return",  TokenClass::RETURN},
    {"struct",  TokenClass::STRUCT},
    {"apply",   TokenClass::APPLY},
    {"const",   TokenClass::CONST},
    {"bool",    TokenClass::BOOL},
    {"true",    TokenClass::TRUE},
    {"false",   TokenClass::FALSE},
    {"void",    TokenClass::VOID},
    {"int",     TokenClass::INT},
    {"bit",     TokenClass::BIT},
    {"varbit",  TokenClass::VARBIT},
    {"string",  TokenClass::STRING},
    {"match_kind",   TokenClass::MATCH_KIND},
    {"transition",   TokenClass::TRANSITION},
    {"header_union", TokenClass::UNION},
  };
  NameDeclaration* name_decl;

  for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
    name_decl = scope->bind(parser->storage, keywords[i].strname, NameSpace::KEYWORD);
    name_decl->token_class = keywords[i].token_class;
  }
}

Token* Parser::next_token()
{
  assert(this->token_at < this->tokens->elem_count);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  this->prev_token = this->token;
  this->prev_token_at = this->token_at;
  this->token = (Token*)this->tokens->get(++this->token_at, sizeof(Token));
  while (this->token->klass == TokenClass::COMMENT) {
    this->token = (Token*)this->tokens->get(++this->token_at, sizeof(Token));
  }
  if (this->token->klass == TokenClass::IDENTIFIER) {
    name_entry = this->current_scope->lookup(this->token->lexeme, NameSpace::KEYWORD | NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::KEYWORD >> 1];
    if (name_decl) {
      this->token->klass = name_decl->token_class;
      return this->token;
    }
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    if (name_decl) {
      this->token->klass = TokenClass::TYPE_IDENTIFIER;
      return this->token;
    }
  }
  return this->token;
}

Token* Parser::peek_token()
{
  Token* peek_token;

  this->prev_token = this->token;
  this->prev_token_at = this->token_at;
  peek_token = this->next_token();
  this->token = this->prev_token;
  this->token_at = this->prev_token_at;
  return peek_token;
}

static bool token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TokenClass::IDENTIFIER || token->klass == TokenClass::APPLY || token->klass == TokenClass::KEY
    || token->klass == TokenClass::ACTIONS || token->klass == TokenClass::STATE || token->klass == TokenClass::ENTRIES;
  return result;
}

static bool token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

static bool token_is_typeName(Token* token)
{
  return token->klass == TokenClass::TYPE_IDENTIFIER;
}

static bool token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TokenClass::IDENTIFIER || token->klass == TokenClass::TYPE_IDENTIFIER
    || token->klass == TokenClass::APPLY || token->klass == TokenClass::STATE;
  return result;
}

static bool token_is_baseType(Token* token)
{
  bool result = token->klass == TokenClass::BOOL || token->klass == TokenClass::ERROR || token->klass == TokenClass::INT
    || token->klass == TokenClass::BIT || token->klass == TokenClass::VARBIT || token->klass == TokenClass::STRING
    || token->klass == TokenClass::VOID;
  return result;
}

static bool token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token->klass == TokenClass::TYPE_IDENTIFIER || token->klass == TokenClass::TUPLE;
  return result;
}

static bool token_is_direction(Token* token)
{
  bool result = token->klass == TokenClass::IN || token->klass == TokenClass::OUT || token->klass == TokenClass::INOUT;
  return result;
}

static bool token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

static bool token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TokenClass::HEADER || token->klass == TokenClass::UNION || token->klass == TokenClass::STRUCT
    || token->klass == TokenClass::ENUM;
  return result;
}

static bool token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TokenClass::TYPEDEF
    || token->klass == TokenClass::PARSER || token->klass == TokenClass::CONTROL || token->klass == TokenClass::PACKAGE;
  return result;
}

static bool token_is_typeArg(Token* token)
{
  bool result = token->klass == TokenClass::DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

static bool token_is_typeOrVoid(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TokenClass::VOID || token->klass == TokenClass::IDENTIFIER;
  return result;
}

static bool token_is_actionRef(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TokenClass::PARENTH_OPEN;
  return result;
}

static bool token_is_tableProperty(Token* token)
{
  bool result = token->klass == TokenClass::KEY || token->klass == TokenClass::ACTIONS;
#if 0
    || token->klass == TokenClass::CONST || token->klass == TokenClass::ENTRIES
    || token_is_nonTableKwName(token);
#endif
  return result;
}

static bool token_is_switchLabel(Token* token)
{
  bool result = token_is_name(token) || token->klass == TokenClass::DEFAULT;
  return result;
}

static bool token_is_expressionPrimary(Token* token)
{
  bool result = token->klass == TokenClass::INTEGER_LITERAL || token->klass == TokenClass::TRUE || token->klass == TokenClass::FALSE
    || token->klass == TokenClass::STRING_LITERAL || token_is_nonTypeName(token)
    || token->klass == TokenClass::BRACE_OPEN || token->klass == TokenClass::PARENTH_OPEN || token->klass == TokenClass::EXCLAMATION
    || token->klass == TokenClass::TILDA || token->klass == TokenClass::UNARY_MINUS || token_is_typeName(token)
    || token->klass == TokenClass::ERROR || token->klass == TokenClass::TYPE_IDENTIFIER;
  return result;
}

static bool token_is_expression(Token* token)
{
  return token_is_expressionPrimary(token);
}

static bool token_is_methodPrototype(Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TokenClass::TYPE_IDENTIFIER;
}

static bool token_is_structField(Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

static bool token_is_specifiedIdentifier(Token* token)
{
  return token_is_name(token);
}

static bool token_is_declaration(Token* token)
{
  bool result = token->klass == TokenClass::CONST || token->klass == TokenClass::EXTERN || token->klass == TokenClass::ACTION
    || token->klass == TokenClass::PARSER || token_is_typeDeclaration(token) || token->klass == TokenClass::CONTROL
    || token_is_typeRef(token) || token->klass == TokenClass::ERROR || token->klass == TokenClass::MATCH_KIND
    || token_is_typeOrVoid(token);
  return result;
}

static bool token_is_lvalue(Token* token)
{
  bool result = token_is_nonTypeName(token) || (token->klass == TokenClass::DOT);
  return result;
}

static bool token_is_assignmentOrMethodCallStatement(Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TokenClass::PARENTH_OPEN || token->klass == TokenClass::ANGLE_OPEN
    || token->klass == TokenClass::EQUAL;
  return result;
}

static bool token_is_statement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TokenClass::IF
    || token->klass == TokenClass::SEMICOLON || token->klass == TokenClass::BRACE_OPEN || token->klass == TokenClass::EXIT
    || token->klass == TokenClass::RETURN || token->klass == TokenClass::SWITCH;
  return result;
}

static bool token_is_statementOrDeclaration(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TokenClass::CONST || token_is_statement(token);
  return result;
}

static bool token_is_argument(Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TokenClass::DONTCARE;
  return result;
}

static bool token_is_parserLocalElement(Token* token)
{
  bool result = token->klass == TokenClass::CONST || token_is_typeRef(token);
  return result;
}

static bool token_is_parserStatement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TokenClass::BRACE_OPEN || token->klass == TokenClass::CONST || token_is_typeRef(token)
    || token->klass == TokenClass::SEMICOLON;
  return result;
}

static bool token_is_simpleKeysetExpression(Token* token) {
  bool result = token_is_expression(token) || token->klass == TokenClass::DEFAULT || token->klass == TokenClass::DONTCARE;
  return result;
}

static bool token_is_keysetExpression(Token* token)
{
  bool result = token->klass == TokenClass::TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

static bool token_is_selectCase(Token* token)
{
  return token_is_keysetExpression(token);
}

static bool token_is_controlLocalDeclaration(Token* token)
{
  bool result = token->klass == TokenClass::CONST || token->klass == TokenClass::ACTION
    || token->klass == TokenClass::TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

static bool token_is_realTypeArg(Token* token)
{
  bool result = token->klass == TokenClass::DONTCARE|| token_is_typeRef(token);
  return result;
}

static bool token_is_binaryOperator(Token* token)
{
  bool result = token->klass == TokenClass::STAR || token->klass == TokenClass::SLASH
    || token->klass == TokenClass::PLUS || token->klass == TokenClass::MINUS
    || token->klass == TokenClass::ANGLE_OPEN_EQUAL || token->klass == TokenClass::ANGLE_CLOSE_EQUAL
    || token->klass == TokenClass::ANGLE_OPEN || token->klass == TokenClass::ANGLE_CLOSE
    || token->klass == TokenClass::EXCLAMATION_EQUAL || token->klass == TokenClass::DOUBLE_EQUAL
    || token->klass == TokenClass::DOUBLE_PIPE || token->klass == TokenClass::DOUBLE_AMPERSAND
    || token->klass == TokenClass::PIPE || token->klass == TokenClass::AMPERSAND
    || token->klass == TokenClass::CIRCUMFLEX || token->klass == TokenClass::DOUBLE_ANGLE_OPEN
    || token->klass == TokenClass::DOUBLE_ANGLE_CLOSE || token->klass == TokenClass::TRIPLE_AMPERSAND
    || token->klass == TokenClass::EQUAL;
  return result;
}

static bool token_is_exprOperator(Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TokenClass::DOT
    || token->klass == TokenClass::BRACKET_OPEN || token->klass == TokenClass::PARENTH_OPEN
    || token->klass == TokenClass::ANGLE_OPEN;
  return result;
}

static int operator_priority(Token* token)
{
  if (token->klass == TokenClass::DOUBLE_AMPERSAND || token->klass == TokenClass::DOUBLE_PIPE) {
    /* Logical AND, OR */
    return 1;
  } else if (token->klass == TokenClass::DOUBLE_EQUAL || token->klass == TokenClass::EXCLAMATION_EQUAL
      || token->klass == TokenClass::ANGLE_OPEN /* < */ || token->klass == TokenClass::ANGLE_CLOSE /* > */
      || token->klass == TokenClass::ANGLE_OPEN_EQUAL /* <= */ || token->klass == TokenClass::ANGLE_CLOSE_EQUAL /* >= */) {
    /* Relational ops  */
    return 2;
  }
  else if (token->klass == TokenClass::PLUS || token->klass == TokenClass::MINUS
           || token->klass == TokenClass::AMPERSAND || token->klass == TokenClass::PIPE
           || token->klass == TokenClass::CIRCUMFLEX || token->klass == TokenClass::DOUBLE_ANGLE_OPEN /* << */
           || token->klass == TokenClass::DOUBLE_ANGLE_CLOSE /* >> */) {
    /* Addition and subtraction; bitwise ops */
    return 3;
  }
  else if (token->klass == TokenClass::STAR || token->klass == TokenClass::SLASH) {
    /* Multiplication and division */
    return 4;
  }
  else if (token->klass == TokenClass::TRIPLE_AMPERSAND) {
    /* Mask */
    return 5;
  }
  else assert(0);
  return 0;
}

static enum AstOperator token_to_binop(Token* token)
{
  switch (token->klass) {
    case TokenClass::DOUBLE_AMPERSAND:
      return AstOperator::AND;
    case TokenClass::DOUBLE_PIPE:
      return AstOperator::OR;
    case TokenClass::DOUBLE_EQUAL:
      return AstOperator::EQ;
    case TokenClass::EXCLAMATION_EQUAL:
      return AstOperator::NEQ;
    case TokenClass::ANGLE_OPEN:
      return AstOperator::LESS;
    case TokenClass::ANGLE_CLOSE:
      return AstOperator::GREAT;
    case TokenClass::ANGLE_OPEN_EQUAL:
      return AstOperator::LESS_EQ;
    case TokenClass::ANGLE_CLOSE_EQUAL:
      return AstOperator::GREAT_EQ;
    case TokenClass::PLUS:
      return AstOperator::ADD;
    case TokenClass::MINUS:
      return AstOperator::SUB;
    case TokenClass::STAR:
      return AstOperator::MUL;
    case TokenClass::SLASH:
      return AstOperator::DIV;
    case TokenClass::AMPERSAND:
      return AstOperator::BITW_AND;
    case TokenClass::PIPE:
      return AstOperator::BITW_OR;
    case TokenClass::CIRCUMFLEX:
      return AstOperator::BITW_XOR;
    case TokenClass::DOUBLE_ANGLE_OPEN:
      return AstOperator::BITW_SHL;
    case TokenClass::DOUBLE_ANGLE_CLOSE:
      return AstOperator::BITW_SHR;
    case TokenClass::TRIPLE_AMPERSAND:
      return AstOperator::MASK;
    default: return (AstOperator)0;
  }
}

char* AstEnum_to_string(enum AstEnum ast)
{
  switch (ast) {
    case AstEnum::none: return "none";

    /** PROGRAM **/

    case AstEnum::p4program: return "p4program";
    case AstEnum::declarationList: return "declarationList";
    case AstEnum::declaration: return "declaration";
    case AstEnum::name: return "name";
    case AstEnum::parameterList: return "parameterList";
    case AstEnum::parameter: return "parameter";
    case AstEnum::paramDirection: return "paramDirection";
    case AstEnum::packageTypeDeclaration: return "packageTypeDeclaration";
    case AstEnum::instantiation: return "instantiation";

    /** PARSER **/

    case AstEnum::parserDeclaration: return "parserDeclaration";
    case AstEnum::parserTypeDeclaration: return "parserTypeDeclaration";
    case AstEnum::parserLocalElements: return "parserLocalElements";
    case AstEnum::parserLocalElement: return "parserLocalElement";
    case AstEnum::parserStates: return "parserStates";
    case AstEnum::parserState: return "parserState";
    case AstEnum::parserStatements: return "parserStatements";
    case AstEnum::parserStatement: return "parserStatement";
    case AstEnum::parserBlockStatement: return "parserBlockStatement";
    case AstEnum::transitionStatement: return "transitionStatement";
    case AstEnum::stateExpression: return "stateExpression";
    case AstEnum::selectExpression: return "selectExpression";
    case AstEnum::selectCaseList: return "selectCaseList";
    case AstEnum::selectCase: return "selectCase";
    case AstEnum::keysetExpression: return "keysetExpression";
    case AstEnum::tupleKeysetExpression: return "tupleKeysetExpression";
    case AstEnum::simpleKeysetExpression: return "simpleKeysetExpression";
    case AstEnum::simpleExpressionList: return "simpleExpressionList";

    /** CONTROL **/

    case AstEnum::controlDeclaration: return "controlDeclaration";
    case AstEnum::controlTypeDeclaration: return "controlTypeDeclaration";
    case AstEnum::controlLocalDeclarations: return "controlLocalDeclarations";
    case AstEnum::controlLocalDeclaration: return "controlLocalDeclaration";

    /** TYPES **/

    case AstEnum::typeRef: return "typeRef";
    case AstEnum::tupleType: return "tupleType";
    case AstEnum::headerStackType: return "headerStackType";
    case AstEnum::baseTypeBoolean: return "baseTypeBoolean";
    case AstEnum::baseTypeInteger: return "baseTypeInteger";
    case AstEnum::baseTypeBit: return "baseTypeBit";
    case AstEnum::baseTypeVarbit: return "baseTypeVarbit";
    case AstEnum::baseTypeString: return "baseTypeString";
    case AstEnum::baseTypeVoid: return "baseTypeVoid";
    case AstEnum::baseTypeError: return "baseTypeError";
    case AstEnum::integerTypeSize: return "integerTypeSize";
    case AstEnum::realTypeArg: return "realTypeArg";
    case AstEnum::typeArg: return "typeArg";
    case AstEnum::typeArgumentList: return "typeArgumentList";
    case AstEnum::typeDeclaration: return "typeDeclaration";
    case AstEnum::derivedTypeDeclaration: return "derivedTypeDeclaration";
    case AstEnum::headerTypeDeclaration: return "headerTypeDeclaration";
    case AstEnum::headerUnionDeclaration: return "headerUnionDeclaration";
    case AstEnum::structTypeDeclaration: return "structTypeDeclaration";
    case AstEnum::structFieldList: return "structFieldList";
    case AstEnum::structField: return "structField";
    case AstEnum::enumDeclaration: return "enumDeclaration";
    case AstEnum::errorDeclaration: return "errorDeclaration";
    case AstEnum::matchKindDeclaration: return "matchKindDeclaration";
    case AstEnum::identifierList: return "identifierList";
    case AstEnum::specifiedIdentifierList: return "specifiedIdentifierList";
    case AstEnum::specifiedIdentifier: return "specifiedIdentifier";
    case AstEnum::typedefDeclaration: return "typedefDeclaration";

    /** STATEMENTS **/

    case AstEnum::assignmentStatement: return "assignmentStatement";
    case AstEnum::emptyStatement: return "emptyStatement";
    case AstEnum::returnStatement: return "returnStatement";
    case AstEnum::exitStatement: return "exitStatement";
    case AstEnum::conditionalStatement: return "conditionalStatement";
    case AstEnum::directApplication: return "directApplication";
    case AstEnum::statement: return "statement";
    case AstEnum::blockStatement: return "blockStatement";
    case AstEnum::statementOrDeclaration: return "statementOrDeclaration";
    case AstEnum::statementOrDeclList: return "statementOrDeclList";
    case AstEnum::switchStatement: return "switchStatement";
    case AstEnum::switchCases: return "switchCases";
    case AstEnum::switchCase: return "switchCase";
    case AstEnum::switchLabel: return "switchLabel";

    /** TABLES **/

    case AstEnum::tableDeclaration: return "tableDeclaration";
    case AstEnum::tablePropertyList: return "tablePropertyList";
    case AstEnum::tableProperty: return "tableProperty";
    case AstEnum::keyProperty: return "keyProperty";
    case AstEnum::keyElementList: return "keyElementList";
    case AstEnum::keyElement: return "keyElement";
    case AstEnum::actionsProperty: return "actionsProperty";
    case AstEnum::actionList: return "actionList";
    case AstEnum::actionRef: return "actionRef";
#if 0
    case AstEnum::entriesProperty: return "entriesProperty";
    case AstEnum::entriesList: return "entriesList";
    case AstEnum::entry: return "entry";
    case AstEnum::simpleProperty: return "simpleProperty";
#endif
    case AstEnum::actionDeclaration: return "actionDeclaration";

    /** VARIABLES **/

    case AstEnum::variableDeclaration: return "variableDeclaration";

    /** EXPRESSIONS **/

    case AstEnum::functionDeclaration: return "functionDeclaration";
    case AstEnum::argumentList: return "argumentList";
    case AstEnum::argument: return "argument";
    case AstEnum::expressionList: return "expressionList";
    case AstEnum::expression: return "expression";
    case AstEnum::lvalueExpression: return "lvalueExpression";
    case AstEnum::binaryExpression: return "binaryExpression";
    case AstEnum::unaryExpression: return "unaryExpression";
    case AstEnum::functionCall: return "functionCall";
    case AstEnum::memberSelector: return "memberSelector";
    case AstEnum::castExpression: return "castExpression";
    case AstEnum::arraySubscript: return "arraySubscript";
    case AstEnum::indexExpression: return "indexExpression";
    case AstEnum::integerLiteral: return "integerLiteral";
    case AstEnum::stringLiteral: return "stringLiteral";
    case AstEnum::dontcare: return "dontcare";
    case AstEnum::default_: return "default";

    default: return "?";
  }
  assert(0);
  return 0;
}

Ast* Ast::clone(Arena* storage)
{
  Ast* clone, *sibling_clone, *child_clone;

  if (this == 0) return (Ast*)0;
  clone = (Ast*)storage->malloc(sizeof(Ast));
  clone->kind = this->kind;
  clone->line_no = this->line_no;
  clone->column_no = this->column_no;
  if (this->tree.first_child) {
    child_clone = container_of(this->tree.first_child, Ast, tree)->clone(storage);
    clone->tree.first_child = &child_clone->tree;
  }
  if (this->tree.right_sibling) {
    sibling_clone = container_of(this->tree.right_sibling, Ast, tree)->clone(storage);
    clone->tree.right_sibling = &sibling_clone->tree;
  }

  /** PROGRAM **/
  if (this->kind == AstEnum::p4program) {
    clone->p4program.decl_list = this->p4program.decl_list->clone(storage);
  } else if (this->kind == AstEnum::declarationList) {
    ;
  } else if (this->kind == AstEnum::declaration) {
    clone->declaration.decl = this->declaration.decl->clone(storage);
  } else if (this->kind == AstEnum::name) {
    clone->name.strname = this->name.strname;
  } else if (this->kind == AstEnum::parameterList) {
    ;
  } else if (this->kind == AstEnum::parameter) {
    clone->parameter.direction = this->parameter.direction;
    clone->parameter.name = this->parameter.name->clone(storage);
    clone->parameter.type = this->parameter.type->clone(storage);
    clone->parameter.init_expr = this->parameter.init_expr->clone(storage);
  } else if (this->kind == AstEnum::packageTypeDeclaration) {
    clone->packageTypeDeclaration.name = this->packageTypeDeclaration.name->clone(storage);
    clone->packageTypeDeclaration.params = this->packageTypeDeclaration.params->clone(storage);
  } else if (this->kind == AstEnum::instantiation) {
    clone->instantiation.name = this->instantiation.name->clone(storage);
    clone->instantiation.type = this->instantiation.type->clone(storage);
    clone->instantiation.args = this->instantiation.args->clone(storage);
  }
  /** PARSER **/
  else if (this->kind == AstEnum::parserDeclaration) {
    clone->parserDeclaration.proto = this->parserDeclaration.proto->clone(storage);
    clone->parserDeclaration.ctor_params = this->parserDeclaration.ctor_params->clone(storage);
    clone->parserDeclaration.local_elements = this->parserDeclaration.local_elements->clone(storage);
    clone->parserDeclaration.states = this->parserDeclaration.states->clone(storage);
  } else if (this->kind == AstEnum::parserTypeDeclaration) {
    clone->parserTypeDeclaration.name = this->parserTypeDeclaration.name->clone(storage);
    clone->parserTypeDeclaration.params = this->parserTypeDeclaration.params->clone(storage);
    clone->parserTypeDeclaration.method_protos = this->parserTypeDeclaration.method_protos->clone(storage);
  } else if (this->kind == AstEnum::parserLocalElements) {
    ;
  } else if (this->kind == AstEnum::parserLocalElement) {
    clone->parserLocalElement.element = this->parserLocalElement.element->clone(storage);
  } else if (this->kind == AstEnum::parserStates) {
    ;
  } else if (this->kind == AstEnum::parserState) {
    clone->parserState.name = this->parserState.name->clone(storage);
    clone->parserState.stmt_list = this->parserState.stmt_list->clone(storage);
    clone->parserState.transition_stmt = this->parserState.transition_stmt->clone(storage);
  } else if (this->kind == AstEnum::parserStatements) {
    ;
  } else if (this->kind == AstEnum::parserStatement) {
    clone->parserStatement.stmt = this->parserStatement.stmt->clone(storage);
  } else if (this->kind == AstEnum::parserBlockStatement) {
    clone->parserBlockStatement.stmt_list = this->parserBlockStatement.stmt_list->clone(storage);
  } else if (this->kind == AstEnum::transitionStatement) {
    clone->transitionStatement.stmt = this->transitionStatement.stmt->clone(storage);
  } else if (this->kind == AstEnum::stateExpression) {
    clone->stateExpression.expr = this->stateExpression.expr->clone(storage);
  } else if (this->kind == AstEnum::selectExpression) {
    clone->selectExpression.expr_list = this->selectExpression.expr_list->clone(storage);
    clone->selectExpression.case_list = this->selectExpression.case_list->clone(storage);
  } else if (this->kind == AstEnum::selectCaseList) {
    ;
  } else if (this->kind == AstEnum::selectCase) {
    clone->selectCase.keyset_expr = this->selectCase.keyset_expr->clone(storage);
    clone->selectCase.name = this->selectCase.name->clone(storage);
  } else if (this->kind == AstEnum::keysetExpression) {
    clone->keysetExpression.expr = this->keysetExpression.expr->clone(storage);
  } else if (this->kind == AstEnum::tupleKeysetExpression) {
    clone->tupleKeysetExpression.expr_list = this->tupleKeysetExpression.expr_list->clone(storage);
  } else if (this->kind == AstEnum::simpleKeysetExpression) {
    clone->simpleKeysetExpression.expr = this->simpleKeysetExpression.expr->clone(storage);
  } else if (this->kind == AstEnum::simpleExpressionList) {
    ;
  } else if (this->kind == AstEnum::typeRef) {
    clone->typeRef.type = this->typeRef.type->clone(storage);
  } else if (this->kind == AstEnum::tupleType) {
    clone->tupleType.type_args = this->tupleType.type_args->clone(storage);
  }
  /** CONTROL **/
  else if (this->kind == AstEnum::controlDeclaration) {
    clone->controlDeclaration.proto = this->controlDeclaration.proto->clone(storage);
    clone->controlDeclaration.ctor_params = this->controlDeclaration.ctor_params->clone(storage);
    clone->controlDeclaration.local_decls = this->controlDeclaration.local_decls->clone(storage);
    clone->controlDeclaration.apply_stmt = this->controlDeclaration.apply_stmt->clone(storage);
  } else if (this->kind == AstEnum::controlTypeDeclaration) {
    clone->controlTypeDeclaration.name = this->controlTypeDeclaration.name->clone(storage);
    clone->controlTypeDeclaration.params = this->controlTypeDeclaration.params->clone(storage);
    clone->controlTypeDeclaration.method_protos = this->controlTypeDeclaration.params->clone(storage);
  } else if (this->kind == AstEnum::controlLocalDeclarations) {
    ;
  } else if (this->kind == AstEnum::controlLocalDeclaration) {
    clone->controlLocalDeclaration.decl = this->controlLocalDeclaration.decl->clone(storage);
  }
  /** EXTERN **/
  else if (this->kind == AstEnum::externDeclaration) {
    clone->externDeclaration.decl = this->externDeclaration.decl->clone(storage);
  } else if (this->kind == AstEnum::externTypeDeclaration) {
    clone->externTypeDeclaration.name = this->externTypeDeclaration.name->clone(storage);
    clone->externTypeDeclaration.method_protos = this->externTypeDeclaration.method_protos->clone(storage);
  } else if (this->kind == AstEnum::methodPrototypes) {
    ;
  } else if (this->kind == AstEnum::functionPrototype) {
    clone->functionPrototype.return_type = this->functionPrototype.return_type->clone(storage);
    clone->functionPrototype.name = this->functionPrototype.name->clone(storage);
    clone->functionPrototype.params = this->functionPrototype.params->clone(storage);
  }
  /** TYPES **/
  else if (this->kind == AstEnum::typeRef) {
    clone->typeRef.type = this->typeRef.type->clone(storage);
  } else if (this->kind == AstEnum::tupleType) {
    clone->tupleType.type_args = this->tupleType.type_args->clone(storage);
  } else if (this->kind == AstEnum::headerStackType) {
    clone->headerStackType.type = this->headerStackType.type->clone(storage);
    clone->headerStackType.stack_expr = this->headerStackType.stack_expr->clone(storage);
  } else if (this->kind == AstEnum::baseTypeBoolean) {
    clone->baseTypeBoolean.name = this->baseTypeBoolean.name->clone(storage);
  } else if (this->kind == AstEnum::baseTypeInteger) {
    clone->baseTypeInteger.name = this->baseTypeInteger.name->clone(storage);
    clone->baseTypeInteger.size = this->baseTypeInteger.size->clone(storage);
  } else if (this->kind == AstEnum::baseTypeBit) {
    clone->baseTypeBit.name = this->baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = this->baseTypeBit.size->clone(storage);
  } else if (this->kind == AstEnum::baseTypeBit) {
    clone->baseTypeBit.name = this->baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = this->baseTypeBit.size->clone(storage);
  } else if (this->kind == AstEnum::baseTypeString) {
    clone->baseTypeString.name = this->baseTypeString.name->clone(storage);
  } else if (this->kind == AstEnum::baseTypeVoid) {
    clone->baseTypeVoid.name = this->baseTypeVoid.name->clone(storage);
  } else if (this->kind == AstEnum::baseTypeError) {
    clone->baseTypeError.name = this->baseTypeError.name->clone(storage);
  } else if (this->kind == AstEnum::integerTypeSize) {
    clone->integerTypeSize.size = this->integerTypeSize.size->clone(storage);
  } else if (this->kind == AstEnum::realTypeArg) {
    clone->realTypeArg.arg = this->realTypeArg.arg->clone(storage);
  } else if (this->kind == AstEnum::typeArg) {
    clone->typeArg.arg = this->typeArg.arg->clone(storage);
  } else if (this->kind == AstEnum::typeArgumentList) {
    ;
  } else if (this->kind == AstEnum::typeDeclaration) {
    clone->typeDeclaration.decl = this->typeDeclaration.decl->clone(storage);
  } else if (this->kind == AstEnum::derivedTypeDeclaration) {
    clone->derivedTypeDeclaration.decl = this->derivedTypeDeclaration.decl->clone(storage);
  } else if (this->kind == AstEnum::headerTypeDeclaration) {
    clone->headerTypeDeclaration.name = this->headerTypeDeclaration.name->clone(storage);
    clone->headerTypeDeclaration.fields = this->headerTypeDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::headerUnionDeclaration) {
    clone->headerUnionDeclaration.name = this->headerUnionDeclaration.name->clone(storage);
    clone->headerUnionDeclaration.fields = this->headerUnionDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::structTypeDeclaration) {
    clone->structTypeDeclaration.name = this->structTypeDeclaration.name->clone(storage);
    clone->structTypeDeclaration.fields = this->structTypeDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::structFieldList) {
    ;
  } else if (this->kind == AstEnum::structField) {
    clone->structField.type = this->structField.type->clone(storage);
    clone->structField.name = this->structField.name->clone(storage);
  } else if (this->kind == AstEnum::enumDeclaration) {
    clone->enumDeclaration.type_size = this->enumDeclaration.type_size->clone(storage);
    clone->enumDeclaration.name = this->enumDeclaration.name->clone(storage);
    clone->enumDeclaration.fields = this->enumDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::errorDeclaration) {
    clone->errorDeclaration.fields = this->errorDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::matchKindDeclaration) {
    clone->matchKindDeclaration.fields = this->matchKindDeclaration.fields->clone(storage);
  } else if (this->kind == AstEnum::matchKindDeclaration) {
    ;
  } else if (this->kind == AstEnum::specifiedIdentifierList) {
    ;
  } else if (this->kind == AstEnum::specifiedIdentifier) {
    clone->specifiedIdentifier.name = this->specifiedIdentifier.name->clone(storage);
    clone->specifiedIdentifier.init_expr = this->specifiedIdentifier.init_expr->clone(storage);
  } else if (this->kind == AstEnum::typedefDeclaration) {
    clone->typedefDeclaration.type_ref = this->typedefDeclaration.type_ref->clone(storage);
    clone->typedefDeclaration.name = this->typedefDeclaration.name->clone(storage);
  }
  /** STATEMENTS **/
  else if (this->kind == AstEnum::assignmentStatement) {
    clone->assignmentStatement.lhs_expr = this->assignmentStatement.lhs_expr->clone(storage);
    clone->assignmentStatement.rhs_expr = this->assignmentStatement.rhs_expr->clone(storage);
  } else if (this->kind == AstEnum::emptyStatement) {
    ;
  } else if (this->kind == AstEnum::returnStatement) {
    clone->returnStatement.expr = this->returnStatement.expr->clone(storage);
  } else if (this->kind == AstEnum::returnStatement) {
    ;
  } else if (this->kind == AstEnum::conditionalStatement) {
    clone->conditionalStatement.cond_expr = this->conditionalStatement.cond_expr->clone(storage);
    clone->conditionalStatement.stmt = this->conditionalStatement.stmt->clone(storage);
    clone->conditionalStatement.else_stmt = this->conditionalStatement.else_stmt->clone(storage);
  } else if (this->kind == AstEnum::directApplication) {
    clone->directApplication.name = this->directApplication.name->clone(storage);
    clone->directApplication.args = this->directApplication.args->clone(storage);
  } else if (this->kind == AstEnum::statement) {
    clone->statement.stmt = this->statement.stmt->clone(storage);
  } else if (this->kind == AstEnum::blockStatement) {
    clone->blockStatement.stmt_list = this->blockStatement.stmt_list->clone(storage);
  } else if (this->kind == AstEnum::statementOrDeclaration) {
    clone->statementOrDeclaration.stmt = this->statementOrDeclaration.stmt->clone(storage);
  } else if (this->kind == AstEnum::statementOrDeclList) {
    ;
  } else if (this->kind == AstEnum::switchStatement) {
    clone->switchStatement.expr = this->switchStatement.expr->clone(storage);
    clone->switchStatement.switch_cases = this->switchStatement.switch_cases->clone(storage);
  } else if (this->kind == AstEnum::switchCases) {
    ;
  } else if (this->kind == AstEnum::switchCase) {
    clone->switchCase.label = this->switchCase.label->clone(storage);
    clone->switchCase.stmt = this->switchCase.stmt->clone(storage);
  } else if (this->kind == AstEnum::switchLabel) {
    clone->switchLabel.label = this->switchLabel.label->clone(storage);
  }
  /** TABLES **/
  else if (this->kind == AstEnum::tableDeclaration) {
    clone->tableDeclaration.name = this->tableDeclaration.name->clone(storage);
    clone->tableDeclaration.prop_list = this->tableDeclaration.prop_list->clone(storage);
  } else if (this->kind == AstEnum::tablePropertyList) {
    ;
  } else if (this->kind == AstEnum::tableProperty) {
    clone->tableProperty.prop = this->tableProperty.prop->clone(storage);
  } else if (this->kind == AstEnum::keyProperty) {
    clone->keyProperty.keyelem_list = this->keyProperty.keyelem_list->clone(storage);
  } else if (this->kind == AstEnum::keyElementList) {
    ;
  } else if (this->kind == AstEnum::keyElement) {
    clone->keyElement.expr = this->keyElement.expr->clone(storage);
    clone->keyElement.match = this->keyElement.match->clone(storage);
  } else if (this->kind == AstEnum::actionsProperty) {
    clone->actionsProperty.action_list = this->actionsProperty.action_list->clone(storage);
  } else if (this->kind == AstEnum::actionList) {
    ;
  } else if (this->kind == AstEnum::actionRef) {
    clone->actionRef.name = this->actionRef.name->clone(storage);
    clone->actionRef.args = this->actionRef.args->clone(storage);
  }
#if 0
  else if (this->kind == AstEnum::entriesProperty) {
    clone->entriesProperty.entries_list = this->entriesProperty.entries_list->clone(storage);
  } else if (this->kind == AstEnum::entriesList) {
    ;
  } else if (this->kind == AstEnum::entry) {
    clone->entry.keyset = this->entry.keyset->clone(storage);
    clone->entry.action = this->entry.action->clone(storage);
  } else if (this->kind == AstEnum::simpleProperty) {
    clone->simpleProperty.name = this->simpleProperty.name->clone(storage);
    clone->simpleProperty.init_expr = this->simpleProperty.init_expr->clone(storage);
    clone->simpleProperty.is_const = this->simpleProperty.is_const;
  }
#endif
  else if (this->kind == AstEnum::actionDeclaration) {
    clone->actionDeclaration.name = this->actionDeclaration.name->clone(storage);
    clone->actionDeclaration.params = this->actionDeclaration.params->clone(storage);
    clone->actionDeclaration.stmt = this->actionDeclaration.stmt->clone(storage);
  }
  /** VARIABLES **/
  else if (this->kind == AstEnum::variableDeclaration) {
    clone->variableDeclaration.type = this->variableDeclaration.type->clone(storage);
    clone->variableDeclaration.name = this->variableDeclaration.name->clone(storage);
    clone->variableDeclaration.init_expr = this->variableDeclaration.init_expr->clone(storage);
    clone->variableDeclaration.is_const = this->variableDeclaration.is_const;
  }
  /** EXPRESSIONS **/
  else if (this->kind == AstEnum::functionDeclaration) {
    clone->functionDeclaration.proto = this->functionDeclaration.proto->clone(storage);
    clone->functionDeclaration.stmt = this->functionDeclaration.stmt->clone(storage);
  } else if (this->kind == AstEnum::argumentList) {
    ;
  } else if (this->kind == AstEnum::argument) {
    clone->argument.arg = this->argument.arg->clone(storage);
  } else if (this->kind == AstEnum::expressionList) {
    ;
  } else if (this->kind == AstEnum::expression) {
    clone->expression.expr = this->expression.expr->clone(storage);
  } else if (this->kind == AstEnum::lvalueExpression) {
    clone->lvalueExpression.expr = this->lvalueExpression.expr->clone(storage);
  } else if (this->kind == AstEnum::binaryExpression) {
    clone->binaryExpression.op = this->binaryExpression.op;
    clone->binaryExpression.strname = this->binaryExpression.strname;
    clone->binaryExpression.left_operand = this->binaryExpression.left_operand->clone(storage);
    clone->binaryExpression.right_operand = this->binaryExpression.right_operand->clone(storage);
  } else if (this->kind == AstEnum::unaryExpression) {
    clone->unaryExpression.op = this->unaryExpression.op;
    clone->unaryExpression.strname = this->unaryExpression.strname;
    clone->unaryExpression.operand = this->unaryExpression.operand->clone(storage);
  } else if (this->kind == AstEnum::functionCall) {
    clone->functionCall.lhs_expr = this->functionCall.lhs_expr->clone(storage);
    clone->functionCall.args = this->functionCall.args->clone(storage);
  } else if (this->kind == AstEnum::memberSelector) {
    clone->memberSelector.lhs_expr = this->memberSelector.lhs_expr->clone(storage);
    clone->memberSelector.name = this->memberSelector.name->clone(storage);
  } else if (this->kind == AstEnum::castExpression) {
    clone->castExpression.type = this->castExpression.type->clone(storage);
    clone->castExpression.expr = this->castExpression.expr->clone(storage);
  } else if (this->kind == AstEnum::arraySubscript) {
    clone->arraySubscript.lhs_expr = this->arraySubscript.lhs_expr->clone(storage);
    clone->arraySubscript.index_expr = this->arraySubscript.index_expr->clone(storage);
  } else if (this->kind == AstEnum::indexExpression) {
    clone->indexExpression.start_index = this->indexExpression.start_index->clone(storage);
    clone->indexExpression.end_index = this->indexExpression.end_index->clone(storage);
  } else if (this->kind == AstEnum::integerLiteral) {
    clone->integerLiteral.is_signed = this->integerLiteral.is_signed;
    clone->integerLiteral.value = this->integerLiteral.value;
    clone->integerLiteral.width = this->integerLiteral.width;
  } else if (this->kind == AstEnum::booleanLiteral) {
    clone->booleanLiteral.value = this->booleanLiteral.value;
  } else if (this->kind == AstEnum::stringLiteral) {
    clone->stringLiteral.value = this->stringLiteral.value;
  } else if (this->kind == AstEnum::default_ || this->kind == AstEnum::dontcare) {
    ;
  }
  else assert(0);
  return clone;
}

void Parser::parse()
{
  this->root_scope = Scope::create(this->storage, 5);
  this->current_scope = this->root_scope;

  define_keywords(this, this->root_scope);
  this->token_at = 0;
  this->token = (Token*)this->tokens->get(this->token_at, sizeof(Token));
  this->next_token();
  this->p4program = parse_p4program(parser);
  assert(this->current_scope == this->root_scope);
}

/** PROGRAM **/

Ast* Parser::parse_p4program()
{
  Ast* p4program;
  Scope* scope;

  p4program = (Ast*)this->storage->malloc(sizeof(Ast));
  p4program->kind = AstEnum::p4program;
  p4program->line_no = this->token->line_no;
  p4program->column_no = this->token->column_no;
  while (this->token->klass == TokenClass::SEMICOLON) {
    this->next_token(); /* empty declaration */
  }
  scope = Scope::create(this->storage, 6);
  this->current_scope = scope->push(this->current_scope);
  p4program->p4program.decl_list = parse_declarationList(parser);
  this->current_scope = this->current_scope->pop();
  if (this->token->klass != TokenClass::END_OF_INPUT) {
    error("%s:%d:%d: error: unexpected token `%s`.",
          this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  }
  return p4program;
}

Ast* Parser::parse_declarationList()
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {};

  decls = (Ast*)this->storage->malloc(sizeof(Ast));
  decls->kind = AstEnum::declarationList;
  decls->line_no = this->token->line_no;
  decls->column_no = this->token->column_no;
  if (token_is_declaration(this->token)) {
    ast = parse_declaration(parser);
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token_is_declaration(this->token) || this->token->klass == TokenClass::SEMICOLON) {
      if (token_is_declaration(this->token)) {
        ast = parse_declaration(parser);
        tree_ctor.append_node(&decls->tree, &ast->tree);
      } else if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token(); /* empty declaration */
      }
    }
  }
  return decls;
}

Ast* Parser::parse_declaration()
{
  Ast* decl;

  if (token_is_declaration(this->token)) {
    decl = (Ast*)this->storage->malloc(sizeof(Ast));
    decl->kind = AstEnum::declaration;
    decl->line_no = this->token->line_no;
    decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::CONST) {
      decl->declaration.decl = parse_variableDeclaration(parser, 0);
      return decl;
    } else if (this->token->klass == TokenClass::EXTERN) {
      decl->declaration.decl = parse_externDeclaration(parser);
      return decl;
    } else if (this->token->klass == TokenClass::ACTION) {
      decl->declaration.decl = parse_actionDeclaration(parser);
      return decl;
    } else if (this->token->klass == TokenClass::PARSER) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else {
        decl->declaration.decl = parse_parserDeclaration(parser, decl->declaration.decl);
      }
      return decl;
    } else if (this->token->klass == TokenClass::CONTROL) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else {
        decl->declaration.decl = parse_controlDeclaration(parser, decl->declaration.decl);
      }
      return decl;
    } else if (token_is_typeDeclaration(this->token)) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      return decl;
    } else if (this->token->klass == TokenClass::ERROR) {
      decl->declaration.decl = parse_errorDeclaration(parser);
      return decl;
    } else if (this->token->klass == TokenClass::MATCH_KIND) {
      decl->declaration.decl = parse_matchKindDeclaration(parser);
      return decl;
    } else if (token_is_typeRef(this->token)) {
      Ast* type_ref = parse_typeRef(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        decl->declaration.decl = parse_instantiation(parser, type_ref);
        return decl;
      } else if (token_is_name(this->token)) {
        decl->declaration.decl = parse_functionDeclaration(parser, type_ref);
        return decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(this->token)) {
      decl->declaration.decl = parse_functionDeclaration(parser, parse_typeRef(parser));
      return decl;
    } else assert(0);
  } else error("%s:%d:%d: error: top-level declaration was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_nonTypeName()
{
  Ast* name;

  if (token_is_nonTypeName(this->token)) {
    name = (Ast*)this->storage->malloc(sizeof(Ast));
    name->kind = AstEnum::name;
    name->line_no = this->token->line_no;
    name->column_no = this->token->column_no;
    name->name.strname = this->token->lexeme;
    this->next_token();
    return name;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_name()
{
  Ast* type_name;

  if (token_is_name(this->token)) {
    if (token_is_nonTypeName(this->token)) {
      return parse_nonTypeName(parser);
    } else if (this->token->klass == TokenClass::TYPE_IDENTIFIER) {
      type_name = (Ast*)this->storage->malloc(sizeof(Ast));
      type_name->kind = AstEnum::name;
      type_name->line_no = this->token->line_no;
      type_name->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      this->next_token();
      return type_name;
    } else assert(0);
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parameterList()
{
  Ast* params, *ast;
  AstTreeCtor tree_ctor = {};

  params = (Ast*)this->storage->malloc(sizeof(Ast));
  params->kind = AstEnum::parameterList;
  params->line_no = this->token->line_no;
  params->column_no = this->token->column_no;
  if (token_is_parameter(this->token)) {
    ast = parse_parameter(parser);
    tree_ctor.append_node(&params->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_parameter(parser);
      tree_ctor.append_node(&params->tree, &ast->tree);
    }
  }
  return params;
}

Ast* Parser::parse_parameter()
{
  Ast* param;

  if (token_is_parameter(this->token)) {
    param = (Ast*)this->storage->malloc(sizeof(Ast));
    param->kind = AstEnum::parameter;
    param->line_no = this->token->line_no;
    param->column_no = this->token->column_no;
    param->parameter.direction = parse_direction(parser);
    param->parameter.type = parse_typeRef(parser);
    if (token_is_name(this->token)) {
      param->parameter.name = parse_name(parser);
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        if (token_is_expression(this->token)) {
          param->parameter.init_expr = parse_expression(parser, 1);
        } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      }
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return param;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

static enum ParamDirection parse_direction()
{
  if (token_is_direction(this->token)) {
    if (this->token->klass == TokenClass::IN) {
      this->next_token();
      return ParamDirection::IN;
    } else if (this->token->klass == TokenClass::OUT) {
      this->next_token();
      return ParamDirection::OUT;
    } else if (this->token->klass == TokenClass::INOUT) {
      this->next_token();
      return ParamDirection::IN | ParamDirection::OUT;
    } else assert(0);
  }
  return (ParamDirection)0;
}

Ast* Parser::parse_packageTypeDeclaration()
{
  Ast* package_decl, *name;

  if (this->token->klass == TokenClass::PACKAGE) {
    this->next_token();
    package_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    package_decl->kind = AstEnum::packageTypeDeclaration;
    package_decl->line_no = this->token->line_no;
    package_decl->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      package_decl->packageTypeDeclaration.name = name;
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        package_decl->packageTypeDeclaration.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return package_decl;
  } else error("%s:%d:%d: error: `package` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_instantiation(Ast* type_ref)
{
  Ast* inst_stmt;

  if (token_is_typeRef(this->token) || type_ref) {
    inst_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    inst_stmt->kind = AstEnum::instantiation;
    inst_stmt->line_no = this->token->line_no;
    inst_stmt->column_no = this->token->column_no;
    inst_stmt->instantiation.type = type_ref ? type_ref : parse_typeRef(parser);
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      inst_stmt->instantiation.args = parse_argumentList(parser);
      if (this->token->klass == TokenClass::PARENTH_CLOSE) {
        this->next_token();
        if (token_is_name(this->token)) {
          inst_stmt->instantiation.name = parse_name(parser);
          if (this->token->klass == TokenClass::SEMICOLON) {
            this->next_token();
          } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: instance name was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return inst_stmt;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** PARSER **/

Ast* Parser::parse_constructorParameters()
{
   Ast* params;

  if (this->token->klass == TokenClass::PARENTH_OPEN) {
    this->next_token();
    params = parse_parameterList(parser);
    if (this->token->klass == TokenClass::PARENTH_CLOSE) {
      this->next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return params;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  return 0;
}

Ast* Parser::parse_parserDeclaration(Ast* parser_proto)
{
  Ast* parser_decl;

  if (this->token->klass == TokenClass::PARENTH_OPEN || this->token->klass == TokenClass::BRACE_OPEN) {
    parser_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    parser_decl->kind = AstEnum::parserDeclaration;
    parser_decl->line_no = this->token->line_no;
    parser_decl->column_no = this->token->column_no;
    parser_decl->parserDeclaration.proto = parser_proto;
    parser_decl->parserDeclaration.ctor_params = parse_constructorParameters(parser);
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      parser_decl->parserDeclaration.local_elements = parse_parserLocalElements(parser);
      if (this->token->klass == TokenClass::STATE) {
        parser_decl->parserDeclaration.states = parse_parserStates(parser);
      } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      if (this->token->klass == TokenClass::BRACE_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return parser_decl;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserLocalElements()
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {};

  elems = (Ast*)this->storage->malloc(sizeof(Ast));
  elems->kind = AstEnum::parserLocalElements;
  elems->line_no = this->token->line_no;
  elems->column_no = this->token->column_no;
  if (token_is_parserLocalElement(this->token)) {
    ast = parse_parserLocalElement(parser);
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token_is_parserLocalElement(this->token)) {
      ast = parse_parserLocalElement(parser);
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_parserLocalElement()
{
  Ast* local_element, *type_ref;

  if (token_is_parserLocalElement(this->token)) {
    local_element = (Ast*)this->storage->malloc(sizeof(Ast));
    local_element->kind = AstEnum::parserLocalElement;
    local_element->line_no = this->token->line_no;
    local_element->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::CONST) {
      local_element->parserLocalElement.element = parse_variableDeclaration(parser, 0);
      return local_element;
    } else if (token_is_typeRef(this->token)) {
      type_ref = parse_typeRef(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        local_element->parserLocalElement.element = parse_instantiation(parser, type_ref);
        return local_element;
      } else if (token_is_name(this->token)) {
        local_element->parserLocalElement.element = parse_variableDeclaration(parser, type_ref);
        return local_element;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserTypeDeclaration()
{
  Ast* parser_proto, *name, *method_protos;

  if (this->token->klass == TokenClass::PARSER) {
    this->next_token();
    parser_proto = (Ast*)this->storage->malloc(sizeof(Ast));
    parser_proto->kind = AstEnum::parserTypeDeclaration;
    parser_proto->line_no = this->token->line_no;
    parser_proto->column_no = this->token->column_no;
    method_protos = (Ast*)this->storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = parser_proto->line_no;
    method_protos->column_no = parser_proto->column_no;
    parser_proto->parserTypeDeclaration.method_protos = method_protos;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      parser_proto->parserTypeDeclaration.name = name;
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        parser_proto->parserTypeDeclaration.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return parser_proto;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserStates()
{
  Ast* states, *ast;
  AstTreeCtor tree_ctor = {};

  states = (Ast*)this->storage->malloc(sizeof(Ast));
  states->kind = AstEnum::parserStates;
  states->line_no = this->token->line_no;
  states->column_no = this->token->column_no;
  if (this->token->klass == TokenClass::STATE) {
    ast = parse_parserState(parser);
    tree_ctor.append_node(&states->tree, &ast->tree);
    while (this->token->klass == TokenClass::STATE) {
      ast = parse_parserState(parser);
      tree_ctor.append_node(&states->tree, &ast->tree);
    }
  }
  return states;
}

Ast* Parser::parse_parserState()
{
  Ast* state;

  if (this->token->klass == TokenClass::STATE) {
    this->next_token();
    state = (Ast*)this->storage->malloc(sizeof(Ast));
    state->kind = AstEnum::parserState;
    state->line_no = this->token->line_no;
    state->column_no = this->token->column_no;
    state->parserState.name = parse_name(parser);
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      state->parserState.stmt_list = parse_parserStatements(parser);
      state->parserState.transition_stmt = parse_transitionStatement(parser);
      if (this->token->klass == TokenClass::BRACE_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return state;
  } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserStatements()
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {};

  stmts = (Ast*)this->storage->malloc(sizeof(Ast));
  stmts->kind = AstEnum::parserStatements;
  stmts->line_no = this->token->line_no;
  stmts->column_no = this->token->column_no;
  if (token_is_parserStatement(this->token)) {
    ast = parse_parserStatement(parser);
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token_is_parserStatement(this->token)) {
      ast = parse_parserStatement(parser);
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_parserStatement()
{
  Ast* parser_stmt, *type_ref;

  if (token_is_parserStatement(this->token)) {
    parser_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    parser_stmt->kind = AstEnum::parserStatement;
    parser_stmt->line_no = this->token->line_no;
    parser_stmt->column_no = this->token->column_no;
    if (token_is_typeRef(this->token)) {
      type_ref = parse_typeRef(parser);
      if (token_is_name(this->token)) {
        parser_stmt->parserStatement.stmt = parse_variableDeclaration(parser, type_ref);
        return parser_stmt;
      } else {
        parser_stmt->parserStatement.stmt = parse_directApplication(parser, type_ref);
        return parser_stmt;
      }
    } else if (token_is_assignmentOrMethodCallStatement(this->token)) {
      parser_stmt->parserStatement.stmt = parse_assignmentOrMethodCallStatement(parser);
      return parser_stmt;
    } else if (this->token->klass == TokenClass::BRACE_OPEN) {
      parser_stmt->parserStatement.stmt = parse_parserBlockStatement(parser);
      return parser_stmt;
    } else if (this->token->klass == TokenClass::CONST) {
      parser_stmt->parserStatement.stmt = parse_variableDeclaration(parser, 0);
      return parser_stmt;
    } else if (this->token->klass == TokenClass::SEMICOLON) {
      Ast* stmt = (Ast*)this->storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::emptyStatement;
      stmt->line_no = this->token->line_no;
      stmt->column_no = this->token->column_no;
      parser_stmt->parserStatement.stmt = stmt;
      this->next_token();
      return parser_stmt;
    } else assert(0);
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserBlockStatement()
{
  Ast* stmt;

  if (this->token->klass == TokenClass::BRACE_OPEN) {
    this->next_token();
    stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::parserBlockStatement;
    stmt->line_no = this->token->line_no;
    stmt->column_no = this->token->column_no;
    stmt->parserBlockStatement.stmt_list = parse_parserStatements(parser);
    if (this->token->klass == TokenClass::BRACE_CLOSE) {
      this->next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_transitionStatement()
{
  Ast* transition;

  if (this->token->klass == TokenClass::TRANSITION) {
    this->next_token();
    transition = (Ast*)this->storage->malloc(sizeof(Ast));
    transition->kind = AstEnum::transitionStatement;
    transition->line_no = this->token->line_no;
    transition->column_no = this->token->column_no;
    transition->transitionStatement.stmt = parse_stateExpression(parser);
    return transition;
  } else error("%s:%d:%d: error: `transition` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_stateExpression()
{
  Ast* state_expr;

  if (token_is_name(this->token) || this->token->klass == TokenClass::SELECT) {
    state_expr = (Ast*)this->storage->malloc(sizeof(Ast));
    state_expr->kind = AstEnum::stateExpression;
    state_expr->line_no = this->token->line_no;
    state_expr->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      state_expr->stateExpression.expr = parse_name(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                  this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return state_expr;
    } else if (this->token->klass == TokenClass::SELECT) {
      state_expr->stateExpression.expr = parse_selectExpression(parser);
      return state_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: state expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_selectExpression()
{
  Ast* select_expr;

  if (this->token->klass == TokenClass::SELECT) {
    this->next_token();
    select_expr = (Ast*)this->storage->malloc(sizeof(Ast));
    select_expr->kind = AstEnum::selectExpression;
    select_expr->line_no = this->token->line_no;
    select_expr->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      select_expr->selectExpression.expr_list = parse_expressionList(parser);
      if (this->token->klass == TokenClass::PARENTH_CLOSE) {
        this->next_token();
        if (this->token->klass == TokenClass::BRACE_OPEN) {
          this->next_token();
          select_expr->selectExpression.case_list = parse_selectCaseList(parser);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return select_expr;
  } else error("%s:%d:%d: error: `select` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_selectCaseList()
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {};

  cases = (Ast*)this->storage->malloc(sizeof(Ast));
  cases->kind = AstEnum::selectCaseList;
  cases->line_no = this->token->line_no;
  cases->column_no = this->token->column_no;
  if (token_is_selectCase(this->token)) {
    ast = parse_selectCase(parser);
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token_is_selectCase(this->token)) {
      ast = parse_selectCase(parser);
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_selectCase()
{
  Ast* select_case;

  if (token_is_keysetExpression(this->token)) {
    select_case = (Ast*)this->storage->malloc(sizeof(Ast));
    select_case->kind = AstEnum::selectCase;
    select_case->line_no = this->token->line_no;
    select_case->column_no = this->token->column_no;
    select_case->selectCase.keyset_expr = parse_keysetExpression(parser);
    if (this->token->klass == TokenClass::COLON) {
      this->next_token();
      if (token_is_name(this->token)) {
        select_case->selectCase.name = parse_name(parser);
        if (this->token->klass == TokenClass::SEMICOLON) {
          this->next_token();
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return select_case;
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_keysetExpression(Parser *parser)
{
  Ast* keyset_expr;

  if (this->token->klass == TokenClass::PARENTH_OPEN || token_is_simpleKeysetExpression(this->token)) {
    keyset_expr = (Ast*)this->storage->malloc(sizeof(Ast));
    keyset_expr->kind = AstEnum::keysetExpression;
    keyset_expr->line_no = this->token->line_no;
    keyset_expr->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      keyset_expr->keysetExpression.expr = parse_tupleKeysetExpression(parser);
      return keyset_expr;
    } else if (token_is_simpleKeysetExpression(this->token)) {
      keyset_expr->keysetExpression.expr = parse_simpleKeysetExpression(parser);
      return keyset_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_tupleKeysetExpression()
{
  Ast* tuple_keyset;

  if (this->token->klass == TokenClass::PARENTH_OPEN) {
    this->next_token();
    tuple_keyset = (Ast*)this->storage->malloc(sizeof(Ast));
    tuple_keyset->kind = AstEnum::tupleKeysetExpression;
    tuple_keyset->line_no = this->token->line_no;
    tuple_keyset->column_no = this->token->column_no;
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList(parser);
    if (this->token->klass == TokenClass::PARENTH_CLOSE) {
      this->next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return tuple_keyset;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_simpleExpressionList()
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {};

  exprs = (Ast*)this->storage->malloc(sizeof(Ast));
  exprs->kind = AstEnum::simpleExpressionList;
  exprs->line_no = this->token->line_no;
  exprs->column_no = this->token->column_no;
  if (token_is_expression(this->token)) {
    ast = parse_simpleKeysetExpression(parser);
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_simpleKeysetExpression(parser);
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_simpleKeysetExpression()
{
  Ast* simple_keyset, *default_keyset, *dontcare_keyset;

  if (token_is_simpleKeysetExpression(this->token)) {
    simple_keyset = (Ast*)this->storage->malloc(sizeof(Ast));
    simple_keyset->kind = AstEnum::simpleKeysetExpression;
    simple_keyset->line_no = this->token->line_no;
    simple_keyset->column_no = this->token->column_no;
    if (token_is_expression(this->token)) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(parser, 1);
      return simple_keyset;
    } else if (this->token->klass == TokenClass::DEFAULT) {
      this->next_token();
      default_keyset = (Ast*)this->storage->malloc(sizeof(Ast));
      default_keyset->kind = AstEnum::default_;
      default_keyset->line_no = this->token->line_no;
      default_keyset->column_no = this->token->column_no;
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (this->token->klass == TokenClass::DONTCARE) {
      this->next_token();
      dontcare_keyset = (Ast*)this->storage->malloc(sizeof(Ast));
      dontcare_keyset->kind = AstEnum::dontcare;
      dontcare_keyset->line_no = this->token->line_no;
      dontcare_keyset->column_no = this->token->column_no;
      simple_keyset->simpleKeysetExpression.expr = dontcare_keyset;
      return simple_keyset;
    }
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** CONTROL **/

Ast* Parser::parse_controlDeclaration(Ast* control_proto)
{
  Ast* control_decl;

  if (this->token->klass == TokenClass::PARENTH_OPEN || this->token->klass == TokenClass::BRACE_OPEN) {
    control_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    control_decl->kind = AstEnum::controlDeclaration;
    control_decl->line_no = this->token->line_no;
    control_decl->column_no = this->token->column_no;
    control_decl->controlDeclaration.proto = control_proto;
    control_decl->controlDeclaration.ctor_params = parse_constructorParameters(parser);
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      control_decl->controlDeclaration.local_decls = parse_controlLocalDeclarations(parser);
      if (this->token->klass == TokenClass::APPLY) {
        this->next_token();
        control_decl->controlDeclaration.apply_stmt = parse_blockStatement(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return control_decl;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_controlTypeDeclaration()
{
  Ast* control_proto, *name, *method_protos;

  if (this->token->klass == TokenClass::CONTROL) {
    this->next_token();
    control_proto = (Ast*)this->storage->malloc(sizeof(Ast));
    control_proto->kind = AstEnum::controlTypeDeclaration;
    control_proto->line_no = this->token->line_no;
    control_proto->column_no = this->token->column_no;
    method_protos = (Ast*)this->storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = control_proto->line_no;
    method_protos->column_no = control_proto->column_no;
    control_proto->controlTypeDeclaration.method_protos = method_protos;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      control_proto->controlTypeDeclaration.name = name;
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        control_proto->controlTypeDeclaration.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return control_proto;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_controlLocalDeclaration()
{
  Ast* local_decl, *type_ref;

  if (token_is_controlLocalDeclaration(this->token)) {
    local_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    local_decl->kind = AstEnum::controlLocalDeclaration;
    local_decl->line_no = this->token->line_no;
    local_decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::CONST) {
      local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(parser, 0);
      return local_decl;
    } else if (this->token->klass == TokenClass::ACTION) {
      local_decl->controlLocalDeclaration.decl = parse_actionDeclaration(parser);
      return local_decl;
    } else if (this->token->klass == TokenClass::TABLE) {
      local_decl->controlLocalDeclaration.decl = parse_tableDeclaration(parser);
      return local_decl;
    } else if (token_is_typeRef(this->token)) {
      type_ref = parse_typeRef(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(parser, type_ref);
        return local_decl;
      } else if (token_is_name(this->token)) {
        local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(parser, type_ref);
        return local_decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_controlLocalDeclarations()
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {};

  decls = (Ast*)this->storage->malloc(sizeof(Ast));
  decls->kind = AstEnum::controlLocalDeclarations;
  decls->line_no = this->token->line_no;
  decls->column_no = this->token->column_no;
  if (token_is_controlLocalDeclaration(this->token)) {
    ast = parse_controlLocalDeclaration(parser);
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token_is_controlLocalDeclaration(this->token)) {
      ast = parse_controlLocalDeclaration(parser);
      tree_ctor.append_node(&decls->tree, &ast->tree);
    }
  }
  return decls;
}

/** EXTERN **/

Ast* Parser::parse_externDeclaration()
{
  Ast* extern_decl, *extern_type;
  bool is_function_type = 0;
  Ast* name;

  if (this->token->klass == TokenClass::EXTERN) {
    this->next_token();
    extern_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    extern_decl->kind = AstEnum::externDeclaration;
    extern_decl->line_no = this->token->line_no;
    extern_decl->column_no = this->token->column_no;

    if (token_is_typeOrVoid(this->token) && token_is_nonTypeName(this->token)) {
      is_function_type = token_is_typeOrVoid(this->token) && token_is_name(this->peek_token());
    } else if (token_is_typeOrVoid(this->token)) {
      is_function_type = 1;
    } else if (token_is_nonTypeName(this->token)) {
      is_function_type = 0;
    } else error("%s:%d:%d: error: extern declaration was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);

    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(parser, 0);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return extern_decl;
    } else {
      extern_type = (Ast*)this->storage->malloc(sizeof(Ast));
      extern_type->kind = AstEnum::externTypeDeclaration;
      extern_type->line_no = this->token->line_no;
      extern_type->column_no = this->token->column_no;
      extern_type->externTypeDeclaration.name = parse_nonTypeName(parser);
      name = extern_type->externTypeDeclaration.name;
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        this->next_token();
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      extern_decl->externDeclaration.decl = extern_type;
      return extern_decl;
    }
  } else error("%s:%d:%d: error: `extern` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_methodPrototypes()
{
  Ast* protos, *ast;
  AstTreeCtor tree_ctor = {};

  protos = (Ast*)this->storage->malloc(sizeof(Ast));
  protos->kind = AstEnum::methodPrototypes;
  protos->line_no = this->token->line_no;
  protos->column_no = this->token->column_no;
  if (token_is_methodPrototype(this->token)) {
    ast = parse_methodPrototype(parser);
    tree_ctor.append_node(&protos->tree, &ast->tree);
    while (token_is_methodPrototype(this->token)) {
      ast = parse_methodPrototype(parser);
      tree_ctor.append_node(&protos->tree, &ast->tree);
    }
  }
  return protos;
}

Ast* Parser::parse_functionPrototype(Ast* return_type)
{
  Ast* func_proto, *type_ref;
  Ast* name;

  if (token_is_typeOrVoid(this->token) || return_type) {
    func_proto = (Ast*)this->storage->malloc(sizeof(Ast));
    func_proto->kind = AstEnum::functionPrototype;
    func_proto->line_no = this->token->line_no;
    func_proto->column_no = this->token->column_no;
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      return_type = parse_typeOrVoid(parser);
      if (return_type->kind == AstEnum::name) {
        name = return_type;
        this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
        type_ref = (Ast*)this->storage->malloc(sizeof(Ast));
        type_ref->kind = AstEnum::typeRef;
        type_ref->line_no = this->token->line_no;
        type_ref->column_no = this->token->column_no;
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token_is_name(this->token)) {
      func_proto->functionPrototype.name = parse_name(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        func_proto->functionPrototype.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: function name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return func_proto;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_methodPrototype()
{
  Ast* func_proto;

  if (token_is_methodPrototype(this->token)) {
    if (this->token->klass == TokenClass::TYPE_IDENTIFIER && this->peek_token()->klass == TokenClass::PARENTH_OPEN) {
      /* Constructor */
      func_proto = (Ast*)this->storage->malloc(sizeof(Ast));
      func_proto->kind = AstEnum::functionPrototype;
      func_proto->line_no = this->token->line_no;
      func_proto->column_no = this->token->column_no;
      func_proto->functionPrototype.name = parse_name(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        func_proto->functionPrototype.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return func_proto;
    } else if (token_is_typeOrVoid(this->token)) {
      func_proto = parse_functionPrototype(parser, 0);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return func_proto;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** TYPES **/

Ast* Parser::parse_typeRef()
{
  Ast* type_ref;

  if (token_is_typeRef(this->token)) {
    type_ref = (Ast*)this->storage->malloc(sizeof(Ast));
    type_ref->kind = AstEnum::typeRef;
    type_ref->line_no = this->token->line_no;
    type_ref->column_no = this->token->column_no;
    if (token_is_baseType(this->token)) {
      type_ref->typeRef.type = parse_baseType(parser);
      return type_ref;
    } else if (token_is_typeName(this->token)) {
      type_ref->typeRef.type = parse_namedType(parser);
      return type_ref;
    } else if (this->token->klass == TokenClass::TUPLE) {
      type_ref->typeRef.type = parse_tupleType(parser);
      return type_ref;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_namedType()
{
  Ast* named_type;

  if (token_is_typeName(this->token)) {
    named_type = parse_typeName(parser);
    if (this->token->klass == TokenClass::BRACKET_OPEN) {
      named_type = parse_headerStackType(parser, named_type);
      return named_type;
    }
    return named_type;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeName()
{
  Ast* type_name;

  if (this->token->klass == TokenClass::TYPE_IDENTIFIER) {
    type_name = (Ast*)this->storage->malloc(sizeof(Ast));
    type_name->kind = AstEnum::name;
    type_name->line_no = this->token->line_no;
    type_name->column_no = this->token->column_no;
    type_name->name.strname = this->token->lexeme;
    this->next_token();
    return type_name;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_tupleType()
{
  Ast* tuple;

  if (this->token->klass == TokenClass::TUPLE) {
    tuple = (Ast*)this->storage->malloc(sizeof(Ast));
    tuple->kind = AstEnum::tupleType;
    tuple->line_no = this->token->line_no;
    tuple->column_no = this->token->column_no;
    this->next_token();
    if (this->token->klass == TokenClass::ANGLE_OPEN) {
      this->next_token();
      tuple->tupleType.type_args = parse_typeArgumentList(parser);
      if (this->token->klass == TokenClass::ANGLE_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return tuple;
  } else error("%s:%d:%d: error: `tuple` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_headerStackType(Ast* named_type)
{
  Ast* type_ref, *type;

  if (this->token->klass == TokenClass::BRACKET_OPEN) {
    this->next_token();
    type_ref = (Ast*)this->storage->malloc(sizeof(Ast));
    type_ref->kind = AstEnum::typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    type = (Ast*)this->storage->malloc(sizeof(Ast));
    type->kind = AstEnum::headerStackType;
    type->line_no = named_type->line_no;
    type->column_no = named_type->column_no;
    type->headerStackType.type = type_ref;
    if (token_is_expression(this->token)) {
      type->headerStackType.stack_expr = parse_expression(parser, 1);
      if (this->token->klass == TokenClass::BRACKET_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: expression expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return type;
  } else error("%s:%d:%d: error: `[` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_baseType()
{
  Ast* type_name, *type;

  if (token_is_baseType(this->token)) {
    type_name = (Ast*)this->storage->malloc(sizeof(Ast));
    type_name->kind = AstEnum::name;
    type_name->line_no = this->token->line_no;
    type_name->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::BOOL) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeBoolean;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeBoolean.name = type_name;
      this->next_token();
      return type;
    } else if (this->token->klass == TokenClass::INT) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeInteger;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeInteger.name = type_name;
      this->next_token();
      if (this->token->klass == TokenClass::ANGLE_OPEN) {
        this->next_token();
        type->baseTypeInteger.size = parse_integerTypeSize(parser);
        if (this->token->klass == TokenClass::ANGLE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      }
      return type;
    } else if (this->token->klass == TokenClass::BIT) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeBit;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeBit.name = type_name;
      this->next_token();
      if (this->token->klass == TokenClass::ANGLE_OPEN) {
        this->next_token();
        type->baseTypeBit.size = parse_integerTypeSize(parser);
        if (this->token->klass == TokenClass::ANGLE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      }
      return type;
    } else if (this->token->klass == TokenClass::VARBIT) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeVarbit;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeVarbit.name = type_name;
      this->next_token();
      if (this->token->klass == TokenClass::ANGLE_OPEN) {
        this->next_token();
        type->baseTypeVarbit.size = parse_integerTypeSize(parser);
        if (this->token->klass == TokenClass::ANGLE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: '<' was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return type;
    } else if (this->token->klass == TokenClass::STRING) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeString;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeString.name = type_name;
      this->next_token();
      return type;
    } else if (this->token->klass == TokenClass::VOID) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeVoid;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeVoid.name = type_name;
      this->next_token();
      return type;
    } else if (this->token->klass == TokenClass::ERROR) {
      type = (Ast*)this->storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeError;
      type->line_no = this->token->line_no;
      type->column_no = this->token->column_no;
      type_name->name.strname = this->token->lexeme;
      type->baseTypeError.name = type_name;
      this->next_token();
      return type;
    } else assert(0);
  } else error("%s:%d:%d: error: base type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_integerTypeSize()
{
  Ast* type_size;

  type_size = (Ast*)this->storage->malloc(sizeof(Ast));
  type_size->kind = AstEnum::integerTypeSize;
  type_size->line_no = this->token->line_no;
  type_size->column_no = this->token->column_no;
  if (this->token->klass == TokenClass::INTEGER_LITERAL) {
    type_size->integerTypeSize.size = parse_integer(parser);
  } else if (this->token->klass == TokenClass::PARENTH_OPEN) {
#if 0
    type_size->size = parse_expression(parser, 1);
#endif
    error("%s:%d:%d: error: integer was expected, got `%s`.",
          this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  return type_size;
}

Ast* Parser::parse_typeOrVoid()
{
  Ast* type, *name;

  if (token_is_typeOrVoid(this->token)) {
    if (token_is_typeRef(this->token)) {
      type = parse_typeRef(parser);
      return type;
    } else if (this->token->klass == TokenClass::VOID) {
      return parse_baseType(parser);
    } else if (this->token->klass == TokenClass::IDENTIFIER) {
      name = (Ast*)this->storage->malloc(sizeof(Ast));
      name->kind = AstEnum::name;
      name->line_no = this->token->line_no;
      name->column_no = this->token->column_no;
      name->name.strname = this->token->lexeme;
      this->next_token();
      return name;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_realTypeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_realTypeArg(this->token)) {
    type_arg = (Ast*)this->storage->malloc(sizeof(Ast));
    type_arg->kind = AstEnum::realTypeArg;
    type_arg->line_no = this->token->line_no;
    type_arg->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::DONTCARE) {
      this->next_token();
      dontcare_arg = (Ast*)this->storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
      dontcare_arg->line_no = this->token->line_no;
      dontcare_arg->column_no = this->token->column_no;
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(this->token)) {
      type_arg->realTypeArg.arg = parse_typeRef(parser);
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_typeArg(this->token)) {
    type_arg = (Ast*)this->storage->malloc(sizeof(Ast));
    type_arg->kind = AstEnum::typeArg;
    type_arg->line_no = this->token->line_no;
    type_arg->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::DONTCARE) {
      this->next_token();
      dontcare_arg = (Ast*)this->storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
      dontcare_arg->line_no = this->token->line_no;
      dontcare_arg->column_no = this->token->column_no;
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(this->token)) {
      type_arg->typeArg.arg = parse_typeRef(parser);
      return type_arg;
    } else if (token_is_nonTypeName(this->token)) {
      type_arg->typeArg.arg = parse_nonTypeName(parser);
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeArgumentList()
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {};

  args = (Ast*)this->storage->malloc(sizeof(Ast));
  args->kind = AstEnum::typeArgumentList;
  args->line_no = this->token->line_no;
  args->column_no = this->token->column_no;
  if (token_is_typeArg(this->token)) {
    ast = parse_typeArg(parser);
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_typeArg(parser);
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_typeDeclaration()
{
  Ast* type_decl;

  if (token_is_typeDeclaration(this->token)) {
    type_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    type_decl->kind = AstEnum::typeDeclaration;
    type_decl->line_no = this->token->line_no;
    type_decl->column_no = this->token->column_no;
    if (token_is_derivedTypeDeclaration(this->token)) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::TYPEDEF) {
      type_decl->typeDeclaration.decl = parse_typedefDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::PARSER) {
      type_decl->typeDeclaration.decl = parse_parserTypeDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::CONTROL) {
      type_decl->typeDeclaration.decl = parse_controlTypeDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::PACKAGE) {
      type_decl->typeDeclaration.decl = parse_packageTypeDeclaration(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: type declaration was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_derivedTypeDeclaration()
{
  Ast* type_decl;

  if (token_is_derivedTypeDeclaration(this->token)) {
    type_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    type_decl->kind = AstEnum::derivedTypeDeclaration;
    type_decl->line_no = this->token->line_no;
    type_decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::HEADER) {
      type_decl->derivedTypeDeclaration.decl = parse_headerTypeDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::UNION) {
      type_decl->derivedTypeDeclaration.decl = parse_headerUnionDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::STRUCT) {
      type_decl->derivedTypeDeclaration.decl = parse_structTypeDeclaration(parser);
      return type_decl;
    } else if (this->token->klass == TokenClass::ENUM) {
      type_decl->derivedTypeDeclaration.decl = parse_enumDeclaration(parser);
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: structure declaration was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_headerTypeDeclaration()
{
  Ast* header_decl;
  Ast* name;

  if (this->token->klass == TokenClass::HEADER) {
    this->next_token();
    header_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    header_decl->kind = AstEnum::headerTypeDeclaration;
    header_decl->line_no = this->token->line_no;
    header_decl->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      header_decl->headerTypeDeclaration.name = name;
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        this->next_token();
        header_decl->headerTypeDeclaration.fields = parse_structFieldList(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return header_decl;
  } else error("%s:%d:%d: error: `header` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_headerUnionDeclaration()
{
  Ast* union_decl;
  Ast* name;

  if (this->token->klass == TokenClass::UNION) {
    this->next_token();
    union_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    union_decl->kind = AstEnum::headerUnionDeclaration;
    union_decl->line_no = this->token->line_no;
    union_decl->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      union_decl->headerUnionDeclaration.name = name;
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        this->next_token();
        union_decl->headerUnionDeclaration.fields = parse_structFieldList(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return union_decl;
  } else error("%s:%d:%d: error: `header_union` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_structTypeDeclaration()
{
  Ast* struct_decl;
  Ast* name;

  if (this->token->klass == TokenClass::STRUCT) {
    this->next_token();
    struct_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    struct_decl->kind = AstEnum::structTypeDeclaration;
    struct_decl->line_no = this->token->line_no;
    struct_decl->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      struct_decl->structTypeDeclaration.name = name;
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        this->next_token();
        struct_decl->structTypeDeclaration.fields = parse_structFieldList(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return struct_decl;
  } else error("%s:%d:%d: error: `struct` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_structFieldList()
{
  Ast* fields, *ast;
  AstTreeCtor tree_ctor = {};

  fields = (Ast*)this->storage->malloc(sizeof(Ast));
  fields->kind = AstEnum::structFieldList;
  fields->line_no = this->token->line_no;
  fields->column_no = this->token->column_no;
  if (token_is_structField(this->token)) {
    ast = parse_structField(parser);
    tree_ctor.append_node(&fields->tree, &ast->tree);
    while (token_is_structField(this->token)) {
      ast = parse_structField(parser);
      tree_ctor.append_node(&fields->tree, &ast->tree);
    }
  }
  return fields;
}

Ast* Parser::parse_structField()
{
  if (token_is_structField(this->token)) {
    Ast* field = (Ast*)this->storage->malloc(sizeof(Ast));
    field->kind = AstEnum::structField;
    field->line_no = this->token->line_no;
    field->column_no = this->token->column_no;
    field->structField.type = parse_typeRef(parser);
    if (token_is_name(this->token)) {
      field->structField.name = parse_name(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return field;
  } else error("%s:%d:%d: error: struct field was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_enumDeclaration()
{
  Ast* enum_decl;
  Ast* name;

  if (this->token->klass == TokenClass::ENUM) {
    this->next_token();
    enum_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    enum_decl->kind = AstEnum::enumDeclaration;
    enum_decl->line_no = this->token->line_no;
    enum_decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::BIT) {
      this->next_token();
      if (this->token->klass == TokenClass::ANGLE_OPEN) {
        this->next_token();
        if (this->token->klass == TokenClass::INTEGER_LITERAL) {
          enum_decl->enumDeclaration.type_size = parse_integer(parser);
          if (this->token->klass == TokenClass::ANGLE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: an integer was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    }
    if (token_is_name(this->token)) {
      name = parse_name(parser);
      this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
      enum_decl->enumDeclaration.name = name;
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        this->next_token();
        if (token_is_specifiedIdentifier(this->token)) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList(parser);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return enum_decl;
  } else error("%s:%d:%d: error: `enum` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_errorDeclaration()
{
  Ast* error_decl;

  if (this->token->klass == TokenClass::ERROR) {
    this->next_token();
    error_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    error_decl->kind = AstEnum::errorDeclaration;
    error_decl->line_no = this->token->line_no;
    error_decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      if (token_is_name(this->token)) {
        if (token_is_name(this->token)) {
          error_decl->errorDeclaration.fields = parse_identifierList(parser);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return error_decl;
  } else error("%s:%d:%d: error: `error` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_matchKindDeclaration()
{
  Ast* match_decl;

  if (this->token->klass == TokenClass::MATCH_KIND) {
    this->next_token();
    match_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    match_decl->kind = AstEnum::matchKindDeclaration;
    match_decl->line_no = this->token->line_no;
    match_decl->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      if (token_is_name(this->token)) {
        match_decl->matchKindDeclaration.fields = parse_identifierList(parser);
        if (this->token->klass == TokenClass::BRACE_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return match_decl;
  } else error("%s:%d:%d: error: `match_kind` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_identifierList()
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {};

  ids = (Ast*)this->storage->malloc(sizeof(Ast));
  ids->kind = AstEnum::identifierList;
  ids->line_no = this->token->line_no;
  ids->column_no = this->token->column_no;
  if (token_is_name(this->token)) {
    ast = parse_name(parser);
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_name(parser);
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifierList()
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {};

  ids = (Ast*)this->storage->malloc(sizeof(Ast));
  ids->kind = AstEnum::specifiedIdentifierList;
  ids->line_no = this->token->line_no;
  ids->column_no = this->token->column_no;
  if (token_is_specifiedIdentifier(this->token)) {
    ast = parse_specifiedIdentifier(parser);
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_specifiedIdentifier(parser);
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifier()
{
  Ast* id;

  if (token_is_specifiedIdentifier(this->token)) {
    id = (Ast*)this->storage->malloc(sizeof(Ast));
    id->kind = AstEnum::specifiedIdentifier;
    id->line_no = this->token->line_no;
    id->column_no = this->token->column_no;
    id->specifiedIdentifier.name = parse_name(parser);
    if (this->token->klass == TokenClass::EQUAL) {
      this->next_token();
      if (token_is_expression(this->token)) {
        id->specifiedIdentifier.init_expr = parse_expression(parser, 1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    }
    return id;
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typedefDeclaration()
{
  Ast* type_decl;
  Ast* name;

  if (this->token->klass == TokenClass::TYPEDEF) {
    this->next_token();
    if (token_is_typeRef(this->token) || token_is_derivedTypeDeclaration(this->token)) {
      type_decl = (Ast*)this->storage->malloc(sizeof(Ast));
      type_decl->kind = AstEnum::typedefDeclaration;
      type_decl->line_no = this->token->line_no;
      type_decl->column_no = this->token->column_no;
      if (token_is_typeRef(this->token)) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef(parser);
      } else if (token_is_derivedTypeDeclaration(this->token)) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration(parser);
      } else assert(0);
      if (token_is_name(this->token)) {
        name = parse_name(parser);
        this->current_scope->bind(this->storage, name->name.strname, NameSpace::TYPE);
        type_decl->typedefDeclaration.name = name;
        if (this->token->klass == TokenClass::SEMICOLON) {
          this->next_token();
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return type_decl;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  } else error("%s:%d:%d: error: type definition was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** STATEMENTS **/

Ast* Parser::parse_assignmentOrMethodCallStatement()
{
  Ast* lvalue, *stmt; 

  if (token_is_lvalue(this->token)) {
    lvalue = parse_lvalue(parser);
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      stmt = (Ast*)this->storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::functionCall;
      stmt->line_no = this->token->line_no;
      stmt->column_no = this->token->column_no;
      stmt->functionCall.lhs_expr = lvalue;
      stmt->functionCall.args = parse_argumentList(parser);
      if (this->token->klass == TokenClass::PARENTH_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return stmt;
    } else if (this->token->klass == TokenClass::EQUAL) {
      this->next_token();
      stmt = (Ast*)this->storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::assignmentStatement;
      stmt->line_no = this->token->line_no;
      stmt->column_no = this->token->column_no;
      stmt->assignmentStatement.lhs_expr = lvalue;
      stmt->assignmentStatement.rhs_expr = parse_expression(parser, 1);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return stmt;
    } else error("%s:%d:%d: error: assignment or function call was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_returnStatement()
{
  Ast* return_stmt;

  if (this->token->klass == TokenClass::RETURN) {
    this->next_token();
    return_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    return_stmt->kind = AstEnum::returnStatement;
    return_stmt->line_no = this->token->line_no;
    return_stmt->column_no = this->token->column_no;
    if (token_is_expression(this->token))
      return_stmt->returnStatement.expr = parse_expression(parser, 1);
    if (this->token->klass == TokenClass::SEMICOLON) {
      this->next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return return_stmt;
  } else error("%s:%d:%d: error: `return` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_exitStatement()
{
  Ast* exit_stmt;

  if (this->token->klass == TokenClass::EXIT) {
    this->next_token();
    exit_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    exit_stmt->kind = AstEnum::exitStatement;
    exit_stmt->line_no = this->token->line_no;
    exit_stmt->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::SEMICOLON) {
      this->next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return exit_stmt;
  } else error("%s:%d:%d: error: `exit` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_conditionalStatement()
{
  Ast* if_stmt;

  if (this->token->klass == TokenClass::IF) {
    this->next_token();
    if_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    if_stmt->kind = AstEnum::conditionalStatement;
    if_stmt->line_no = this->token->line_no;
    if_stmt->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      if (token_is_expression(this->token)) {
        if_stmt->conditionalStatement.cond_expr = parse_expression(parser, 1);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
          if (token_is_statement(this->token)) {
            if_stmt->conditionalStatement.stmt = parse_statement(parser, 0);
            if (this->token->klass == TokenClass::ELSE) {
              this->next_token();
              if (token_is_statement(this->token)) {
                if_stmt->conditionalStatement.else_stmt = parse_statement(parser, 0);
              } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                           this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
            }
          } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return if_stmt;
  } else error("%s:%d:%d: error: `if` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_directApplication(Ast* type_name)
{
  Ast* apply_stmt;

  if (token_is_typeName(this->token) || type_name) {
    apply_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    apply_stmt->kind = AstEnum::directApplication;
    apply_stmt->line_no = this->token->line_no;
    apply_stmt->column_no = this->token->column_no;
    apply_stmt->directApplication.name = type_name ? type_name : parse_typeName(parser);
    if (this->token->klass == TokenClass::DOT) {
      this->next_token();
      if (this->token->klass == TokenClass::APPLY) {
        this->next_token();
        if (this->token->klass == TokenClass::PARENTH_OPEN) {
          this->next_token();
          apply_stmt->directApplication.args = parse_argumentList(parser);
          if (this->token->klass == TokenClass::PARENTH_CLOSE) {
            this->next_token();
            if (this->token->klass == TokenClass::SEMICOLON) {
              this->next_token();
            } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                         this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
          } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `.` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return apply_stmt;
  } else error("%s:%d:%d: error: type name was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_statement(Ast* type_name)
{
  Ast* stmt, *empty_stmt;

  if (token_is_statement(this->token)) {
    stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::statement;
    stmt->line_no = this->token->line_no;
    stmt->column_no = this->token->column_no;
    if (token_is_typeName(this->token) || type_name) {
      stmt->statement.stmt = parse_directApplication(parser, type_name);
      return stmt;
    } else if (token_is_assignmentOrMethodCallStatement(this->token)) {
      stmt->statement.stmt = parse_assignmentOrMethodCallStatement(parser);
      return stmt;
    } else if (this->token->klass == TokenClass::IF) {
      stmt->statement.stmt = parse_conditionalStatement(parser);
      return stmt;
    } else if (this->token->klass == TokenClass::SEMICOLON) {
      empty_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
      empty_stmt->kind = AstEnum::emptyStatement;
      empty_stmt->line_no = this->token->line_no;
      empty_stmt->column_no = this->token->column_no;
      stmt->statement.stmt = empty_stmt;
      this->next_token();
      return stmt;
    } else if (this->token->klass == TokenClass::BRACE_OPEN) {
      stmt->statement.stmt = parse_blockStatement(parser);
      return stmt;
    } else if (this->token->klass == TokenClass::EXIT) {
      stmt->statement.stmt = parse_exitStatement(parser);
      return stmt;
    } else if (this->token->klass == TokenClass::RETURN) {
      stmt->statement.stmt = parse_returnStatement(parser);
      return stmt;
    } else if (this->token->klass == TokenClass::SWITCH) {
      stmt->statement.stmt = parse_switchStatement(parser);
      return stmt;
    }
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_blockStatement()
{
  Ast* block_stmt;

  if (this->token->klass == TokenClass::BRACE_OPEN) {
    this->next_token();
    block_stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    block_stmt->kind = AstEnum::blockStatement;
    block_stmt->line_no = this->token->line_no;
    block_stmt->column_no = this->token->column_no;
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList(parser);
    if (this->token->klass == TokenClass::BRACE_CLOSE) {
      this->next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return block_stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_statementOrDeclList()
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {};

  stmts = (Ast*)this->storage->malloc(sizeof(Ast));
  stmts->kind = AstEnum::statementOrDeclList;
  stmts->line_no = this->token->line_no;
  stmts->column_no = this->token->column_no;
  if (token_is_statementOrDeclaration(this->token)) {
    ast = parse_statementOrDeclaration(parser);
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token_is_statementOrDeclaration(this->token)) {
      ast = parse_statementOrDeclaration(parser);
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_switchStatement()
{
  Ast* stmt;

  if (this->token->klass == TokenClass::SWITCH) {
    this->next_token();
    stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::switchStatement;
    stmt->line_no = this->token->line_no;
    stmt->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      stmt->switchStatement.expr = parse_expression(parser, 1);
      if (this->token->klass == TokenClass::PARENTH_CLOSE) {
        this->next_token();
        if (this->token->klass == TokenClass::BRACE_OPEN) {
          this->next_token();
          stmt->switchStatement.switch_cases = parse_switchCases(parser);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `switch` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_switchCases()
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {};

  cases = (Ast*)this->storage->malloc(sizeof(Ast));
  cases->kind = AstEnum::switchCases;
  cases->line_no = this->token->line_no;
  cases->column_no = this->token->column_no;
  if (token_is_switchLabel(this->token)) {
    ast = parse_switchCase(parser);
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token_is_switchLabel(this->token)) {
      ast = parse_switchCase(parser);
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_switchCase()
{
  Ast* switch_case;

  if (token_is_switchLabel(this->token)) {
    switch_case = (Ast*)this->storage->malloc(sizeof(Ast));
    switch_case->kind = AstEnum::switchCase;
    switch_case->line_no = this->token->line_no;
    switch_case->column_no = this->token->column_no;
    switch_case->switchCase.label = parse_switchLabel(parser);
    if (this->token->klass == TokenClass::COLON) {
      this->next_token();
      if (this->token->klass == TokenClass::BRACE_OPEN) {
        switch_case->switchCase.stmt = parse_blockStatement(parser);
      }
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return switch_case;
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_switchLabel()
{
  Ast* switch_label, *default_label;

  if (token_is_switchLabel(this->token)) {
    switch_label = (Ast*)this->storage->malloc(sizeof(Ast));
    switch_label->kind = AstEnum::switchLabel;
    switch_label->line_no = this->token->line_no;
    switch_label->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      switch_label->switchLabel.label = parse_name(parser);
      return switch_label;
    } else if (this->token->klass == TokenClass::DEFAULT) {
      this->next_token();
      default_label = (Ast*)this->storage->malloc(sizeof(Ast));
      default_label->kind = AstEnum::default_;
      default_label->line_no = this->token->line_no;
      default_label->column_no = this->token->column_no;
      switch_label->switchLabel.label = default_label;
      return switch_label;
    } else assert(0);
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_statementOrDeclaration()
{
  Ast* stmt, *type_ref;

  if (token_is_statementOrDeclaration(this->token)) {
    stmt = (Ast*)this->storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::statementOrDeclaration;
    stmt->line_no = this->token->line_no;
    stmt->column_no = this->token->column_no;
    if (token_is_typeRef(this->token)) {
      type_ref = parse_typeRef(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        stmt->statementOrDeclaration.stmt = parse_instantiation(parser, type_ref);
        return stmt;
      } else if (token_is_name(this->token)) {
        stmt->statementOrDeclaration.stmt = parse_variableDeclaration(parser, type_ref);
        return stmt;
      } else {
        stmt->statementOrDeclaration.stmt = parse_statement(parser, type_ref);
        return stmt;
      }
    } else if (token_is_statement(this->token)) {
      stmt->statementOrDeclaration.stmt = parse_statement(parser, 0);
      return stmt;
    } else if (this->token->klass == TokenClass::CONST) {
      stmt->statementOrDeclaration.stmt = parse_variableDeclaration(parser, 0);
      return stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

/** TABLES **/ 

Ast* Parser::parse_tableDeclaration()
{
  Ast* table, *method_protos;

  if (this->token->klass == TokenClass::TABLE) {
    this->next_token();
    table = (Ast*)this->storage->malloc(sizeof(Ast));
    table->kind = AstEnum::tableDeclaration;
    table->line_no = this->token->line_no;
    table->column_no = this->token->column_no;
    table->tableDeclaration.name = parse_name(parser);
    method_protos = (Ast*)this->storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = table->line_no;
    method_protos->column_no = table->column_no;
    table->tableDeclaration.method_protos = method_protos;
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      if (token_is_tableProperty(this->token)) {
        table->tableDeclaration.prop_list = parse_tablePropertyList(parser);
      } else error("%s:%d:%d: error: table property was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      if (this->token->klass == TokenClass::BRACE_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return table;
  } else error("%s:%d:%d: error: `table` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_tablePropertyList()
{
  Ast* props, *ast;
  AstTreeCtor tree_ctor = {};

  props = (Ast*)this->storage->malloc(sizeof(Ast));
  props->kind = AstEnum::tablePropertyList;
  props->line_no = this->token->line_no;
  props->column_no = this->token->column_no;
  if (token_is_tableProperty(this->token)) {
    ast = parse_tableProperty(parser);
    tree_ctor.append_node(&props->tree, &ast->tree);
    while (token_is_tableProperty(this->token)) {
      ast = parse_tableProperty(parser);
      tree_ctor.append_node(&props->tree, &ast->tree);
    }
  }
  return props;
}

Ast* Parser::parse_tableProperty()
{
#if 0
  bool is_const = 0;
#endif
  Ast* table_prop, *prop;

  if (token_is_tableProperty(this->token)) {
#if 0
    if (this->token->klass == TokenClass::CONST) {
      this->next_token();
      is_const = 1;
    }
#endif
    table_prop = (Ast*)this->storage->malloc(sizeof(Ast));
    table_prop->kind = AstEnum::tableProperty;
    table_prop->line_no = this->token->line_no;
    table_prop->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::KEY) {
      this->next_token();
      prop = (Ast*)this->storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::keyProperty;
      prop->line_no = this->token->line_no;
      prop->column_no = this->token->column_no;
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        if (this->token->klass == TokenClass::BRACE_OPEN) {
          this->next_token();
          prop->keyProperty.keyelem_list = parse_keyElementList(parser);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (this->token->klass == TokenClass::ACTIONS) {
      this->next_token();
      prop = (Ast*)this->storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::actionsProperty;
      prop->line_no = this->token->line_no;
      prop->column_no = this->token->column_no;
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        if (this->token->klass == TokenClass::BRACE_OPEN) {
          this->next_token();
          prop->actionsProperty.action_list = parse_actionList(parser);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
#if 0
    else if (this->token->klass == TokenClass::ENTRIES) {
      this->next_token();
      prop = (Ast*)malloc(this->storage, sizeof(Ast));
      prop->kind = AstEnum::entriesProperty;
      prop->line_no = this->token->line_no;
      prop->column_no = this->token->column_no;
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        if (this->token->klass == TokenClass::BRACE_OPEN) {
          this->next_token();
          if (token_is_keysetExpression(this->token)) {
            prop->entriesProperty.entries_list = parse_entriesList(parser);
          } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
          if (this->token->klass == TokenClass::BRACE_CLOSE) {
            this->next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     this->source_file, parser->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
    else if (token_is_nonTableKwName(this->token)) {
      prop = (Ast*)malloc(this->storage, sizeof(Ast));
      prop->kind = AstEnum::simpleProperty;
      prop->line_no = this->token->line_no;
      prop->column_no = this->token->column_no;
      prop->simpleProperty.is_const = is_const;
      prop->simpleProperty.name = parse_name(parser);
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        prop->simpleProperty.init_expr = parse_expression(parser, 1);
        if (this->token->klass == TokenClass::SEMICOLON) {
          this->next_token();
        } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else assert(0);
#endif
    else error("%s:%d:%d: error: table property was expected, got `%s`.",
                this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  }
  else error("%s:%d:%d: error: table property was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_keyElementList()
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {};

  elems = (Ast*)this->storage->malloc(sizeof(Ast));
  elems->kind = AstEnum::keyElementList;
  elems->line_no = this->token->line_no;
  elems->column_no = this->token->column_no;
  if (token_is_expression(this->token)) {
    ast = parse_keyElement(parser);
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token_is_expression(this->token)) {
      ast = parse_keyElement(parser);
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_keyElement()
{
  Ast* key_elem;

  if (token_is_expression(this->token)) {
    key_elem = (Ast*)this->storage->malloc(sizeof(Ast));
    key_elem->kind = AstEnum::keyElement;
    key_elem->line_no = this->token->line_no;
    key_elem->column_no = this->token->column_no;
    key_elem->keyElement.expr = parse_expression(parser, 1);
    if (this->token->klass == TokenClass::COLON) {
      this->next_token();
      key_elem->keyElement.match = parse_name(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return key_elem;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_actionList()
{
  Ast* actions, *ast;
  AstTreeCtor tree_ctor = {};

  actions = (Ast*)this->storage->malloc(sizeof(Ast));
  actions->kind = AstEnum::actionList;
  actions->line_no = this->token->line_no;
  actions->column_no = this->token->column_no;
  if (token_is_actionRef(this->token)) {
    ast = parse_actionRef(parser);
    tree_ctor.append_node(&actions->tree, &ast->tree);
    if (this->token->klass == TokenClass::SEMICOLON) {
      this->next_token();
    } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    while (token_is_actionRef(this->token)) {
      ast = parse_actionRef(parser);
      tree_ctor.append_node(&actions->tree, &ast->tree);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    }
  }
  return actions;
}

Ast* Parser::parse_actionRef()
{
  Ast* action_ref;

  if (token_is_nonTypeName(this->token)) {
    action_ref = (Ast*)this->storage->malloc(sizeof(Ast));
    action_ref->kind = AstEnum::actionRef;
    action_ref->line_no = this->token->line_no;
    action_ref->column_no = this->token->column_no;
    action_ref->actionRef.name = parse_nonTypeName(parser);
    if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      if (token_is_argument(this->token)) {
        action_ref->actionRef.args = parse_argumentList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else if (this->token->klass == TokenClass::PARENTH_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    }
    return action_ref;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

#if 0
Ast* Parser::parse_entriesList()
{
  Ast* entries, *ast;
  AstTreeCtor tree_ctor = {0};

  entries = (Ast*)malloc(this->storage, sizeof(Ast));
  entries->kind = AstEnum::entriesList;
  entries->line_no = this->token->line_no;
  entries->column_no = this->token->column_no;
  if (token_is_keysetExpression(this->token)) {
    ast = parse_entry(parser);
    tree_ctor.append_node(&entries->tree, &ast->tree);
    while (token_is_keysetExpression(this->token)) {
      ast = parse_entry(parser);
      tree_ctor.append_node(&entries->tree, &ast->tree);
    }
  }
  return entries;
}

Ast* Parser::parse_entry()
{
  Ast* entry;

  if (token_is_keysetExpression(this->token)) {
    entry = (Ast*)malloc(this->storage, sizeof(Ast));
    entry->kind = AstEnum::entry;
    entry->line_no = this->token->line_no;
    entry->column_no = this->token->column_no;
    entry->entry.keyset = parse_keysetExpression(parser);
    if (this->token->klass == TokenClass::COLON) {
      this->next_token();
      entry->entry.action = parse_actionRef(parser);
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return entry;
  } else error("%s:%d:%d: error: keyset was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}
#endif

Ast* Parser::parse_actionDeclaration()
{
  Ast* action_decl;

  if (this->token->klass == TokenClass::ACTION) {
    this->next_token();
    action_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    action_decl->kind = AstEnum::actionDeclaration;
    action_decl->line_no = this->token->line_no;
    action_decl->column_no = this->token->column_no;
    if (token_is_name(this->token)) {
      action_decl->actionDeclaration.name = parse_name(parser);
      if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        action_decl->actionDeclaration.params = parse_parameterList(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
          if (this->token->klass == TokenClass::BRACE_OPEN) {
            action_decl->actionDeclaration.stmt = parse_blockStatement(parser);
          } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                       this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return action_decl;
  } else error("%s:%d:%d: error: `action` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** VARIABLES **/

Ast* Parser::parse_variableDeclaration(Ast* type_ref)
{
  bool is_const = 0;
  Ast* var_decl;

  if (this->token->klass == TokenClass::CONST) {
    this->next_token();
    is_const = 1;
  }
  if (token_is_typeRef(this->token) || type_ref) {
    var_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    var_decl->kind = AstEnum::variableDeclaration;
    var_decl->line_no = this->token->line_no;
    var_decl->column_no = this->token->column_no;
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef(parser);
    if (token_is_name(this->token)) {
      var_decl->variableDeclaration.name = parse_name(parser);
      if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        var_decl->variableDeclaration.init_expr = parse_expression(parser, 1);
      }
      if (this->token->klass == TokenClass::SEMICOLON) {
        this->next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    var_decl->variableDeclaration.is_const = is_const;
    return var_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

/** EXPRESSIONS **/

Ast* Parser::parse_functionDeclaration(Ast* type_ref)
{
  Ast* func_decl;

  if (token_is_typeOrVoid(this->token)) {
    func_decl = (Ast*)this->storage->malloc(sizeof(Ast));
    func_decl->kind = AstEnum::functionDeclaration;
    func_decl->line_no = this->token->line_no;
    func_decl->column_no = this->token->column_no;
    func_decl->functionDeclaration.proto = parse_functionPrototype(parser, type_ref);
    if (this->token->klass == TokenClass::BRACE_OPEN) {
      func_decl->functionDeclaration.stmt = parse_blockStatement(parser);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    return func_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_argumentList()
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {0};

  args = (Ast*)this->storage->malloc(sizeof(Ast));
  args->kind = AstEnum::argumentList;
  args->line_no = this->token->line_no;
  args->column_no = this->token->column_no;
  if (token_is_argument(this->token)) {
    ast = parse_argument(parser);
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_argument(parser);
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_argument()
{
  Ast* arg, *dontcare_arg;

  if (token_is_argument(this->token)) {
    arg = (Ast*)this->storage->malloc(sizeof(Ast));
    arg->kind = AstEnum::argument;
    arg->line_no = this->token->line_no;
    arg->column_no = this->token->column_no;
    if (token_is_expression(this->token)) {
      arg->argument.arg = parse_expression(parser, 1);
      return arg;
    } else if (this->token->klass == TokenClass::DONTCARE) {
      this->next_token();
      dontcare_arg = (Ast*)this->storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
      dontcare_arg->line_no = this->token->line_no;
      dontcare_arg->column_no = this->token->column_no;
      arg->argument.arg = dontcare_arg;
      return arg;
    } else assert(0);
  } else error("%s:%d:%d: error: an argument was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_expressionList()
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {0};
  
  exprs = (Ast*)this->storage->malloc(sizeof(Ast));
  exprs->kind = AstEnum::expressionList;
  exprs->line_no = this->token->line_no;
  exprs->column_no = this->token->column_no;
  if (token_is_expression(this->token)) {
    ast = parse_expression(parser, 1);
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (this->token->klass == TokenClass::COMMA) {
      this->next_token();
      ast = parse_expression(parser, 1);
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_lvalue()
{
  Ast* lvalue, *expr;

  if (token_is_lvalue(this->token)) {
    lvalue = (Ast*)this->storage->malloc(sizeof(Ast));
    lvalue->kind = AstEnum::lvalueExpression;
    lvalue->line_no = this->token->line_no;
    lvalue->column_no = this->token->column_no;
    lvalue->lvalueExpression.expr = parse_nonTypeName(parser);
    while(this->token->klass == TokenClass::DOT || this->token->klass == TokenClass::BRACKET_OPEN) {
      if (this->token->klass == TokenClass::DOT) {
        this->next_token();
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::memberSelector;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->memberSelector.lhs_expr = lvalue;
        if (token_is_name(this->token)) {
          expr->memberSelector.name = parse_name(parser);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        lvalue = (Ast*)this->storage->malloc(sizeof(Ast));
        lvalue->kind = AstEnum::lvalueExpression;
        lvalue->line_no = this->token->line_no;
        lvalue->column_no = this->token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
      else if (this->token->klass == TokenClass::BRACKET_OPEN) {
        this->next_token();
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::arraySubscript;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->arraySubscript.lhs_expr = lvalue;
        expr->arraySubscript.index_expr = parse_indexExpression(parser);
        if (this->token->klass == TokenClass::BRACKET_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        lvalue = (Ast*)this->storage->malloc(sizeof(Ast));
        lvalue->kind = AstEnum::lvalueExpression;
        lvalue->line_no = this->token->line_no;
        lvalue->column_no = this->token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
    }
    return lvalue;
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_expression(int priority_threshold)
{
  Ast* primary, *expr;

  if (token_is_expression(this->token)) {
    primary = parse_expressionPrimary(parser);
    while (token_is_exprOperator(this->token)) {
      if (this->token->klass == TokenClass::DOT) {
        this->next_token();
        Ast* expr;
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::memberSelector;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->memberSelector.lhs_expr = primary;
        if (token_is_nonTypeName(this->token)) {
          expr->memberSelector.name = parse_nonTypeName(parser);
        } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        primary = (Ast*)this->storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (this->token->klass == TokenClass::BRACKET_OPEN) {
        this->next_token();
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::arraySubscript;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->arraySubscript.lhs_expr = primary;
        expr->arraySubscript.index_expr = parse_indexExpression(parser);
        if (this->token->klass == TokenClass::BRACKET_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        primary = (Ast*)this->storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (this->token->klass == TokenClass::PARENTH_OPEN) {
        this->next_token();
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::functionCall;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->functionCall.lhs_expr = primary;
        expr->functionCall.args = parse_argumentList(parser);
        if (parser->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        primary = (Ast*)this->storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (this->token->klass == TokenClass::EQUAL) {
        this->next_token();
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::assignmentStatement;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->assignmentStatement.lhs_expr = primary;
        expr->assignmentStatement.rhs_expr = parse_expression(parser, 1);
        primary = (Ast*)this->storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token_is_binaryOperator(this->token)){
        int priority = operator_priority(this->token);
        if (priority >= priority_threshold) {
          expr = (Ast*)this->storage->malloc(sizeof(Ast));
          expr->kind = AstEnum::binaryExpression;
          expr->line_no = this->token->line_no;
          expr->column_no = this->token->column_no;
          expr->binaryExpression.left_operand = primary;
          expr->binaryExpression.op = token_to_binop(this->token);
          expr->binaryExpression.strname = this->token->lexeme;
          this->next_token();
          expr->binaryExpression.right_operand = parse_expression(parser, priority + 1);
          primary = (Ast*)this->storage->malloc(sizeof(Ast));
          primary->kind = AstEnum::expression;
          primary->line_no = expr->line_no;
          primary->column_no = expr->column_no;
          primary->expression.expr = expr;
        } else break;
      } else assert(0);
    }
    return primary;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_expressionPrimary()
{
  Ast* primary, *expr;

  if (token_is_expression(this->token)) {
    primary = (Ast*)this->storage->malloc(sizeof(Ast));
    primary->kind = AstEnum::expression;
    primary->line_no = this->token->line_no;
    primary->column_no = this->token->column_no;
    if (this->token->klass == TokenClass::INTEGER_LITERAL) {
      primary->expression.expr = parse_integer(parser);
      return primary;
    } else if (this->token->klass == TokenClass::TRUE || this->token->klass == TokenClass::FALSE) {
      primary->expression.expr = parse_boolean(parser);
      return primary;
    } else if (this->token->klass == TokenClass::STRING_LITERAL) {
      primary->expression.expr = parse_string(parser);
      return primary;
    } else if (this->token->klass == TokenClass::DOT) {
      this->next_token();
      if (this->token->klass == TokenClass::IDENTIFIER) {
        primary->expression.expr = parse_nonTypeName(parser);
        return primary;
      } else if (this->token->klass == TokenClass::TYPE_IDENTIFIER) {
        primary->expression.expr = parse_typeName(parser);
        return primary;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(this->token)) {
      primary->expression.expr = parse_nonTypeName(parser);
      return primary;
    } else if (this->token->klass == TokenClass::BRACE_OPEN) {
      this->next_token();
      primary->expression.expr = parse_expressionList(parser);
      if (this->token->klass == TokenClass::BRACE_CLOSE) {
        this->next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      return primary;
    } else if (this->token->klass == TokenClass::PARENTH_OPEN) {
      this->next_token();
      if (this->token->klass == TokenClass::TYPE_IDENTIFIER && this->peek_token()->klass == TokenClass::DOT) {
        /* (<typeName>.<name>) */
        primary->expression.expr = parse_expression(parser, 1);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        return primary;
      } else if (token_is_typeRef(this->token)) {
        expr = (Ast*)this->storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::castExpression;
        expr->line_no = this->token->line_no;
        expr->column_no = this->token->column_no;
        expr->castExpression.type = parse_typeRef(parser);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
          expr->castExpression.expr = parse_expression(parser, 10);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        primary->expression.expr = expr;
        return primary;
      } else if (token_is_expression(this->token)) {
        primary->expression.expr = parse_expression(parser, 1);
        if (this->token->klass == TokenClass::PARENTH_CLOSE) {
          this->next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
        return primary;
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
      assert(0);
    } else if (this->token->klass == TokenClass::EXCLAMATION) {
      this->next_token();
      expr = (Ast*)this->storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = this->token->line_no;
      expr->column_no = this->token->column_no;
      expr->unaryExpression.op = AstOperator::NOT;
      expr->unaryExpression.strname = this->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (this->token->klass == TokenClass::TILDA) {
      this->next_token();
      expr = (Ast*)this->storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = this->token->line_no;
      expr->column_no = this->token->column_no;
      expr->unaryExpression.op = AstOperator::BITW_NOT;
      expr->unaryExpression.strname = this->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (this->token->klass == TokenClass::UNARY_MINUS) {
      this->next_token();
      expr = (Ast*)this->storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = this->token->line_no;
      expr->column_no = this->token->column_no;
      expr->unaryExpression.op = AstOperator::NEG;
      expr->unaryExpression.strname = this->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (token_is_typeName(this->token)) {
      primary->expression.expr = parse_typeName(parser);
      return primary;
    } else if (this->token->klass == TokenClass::ERROR) {
      this->next_token();
      expr = (Ast*)this->storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::name;
      expr->line_no = this->token->line_no;
      expr->column_no = this->token->column_no;
      expr->name.strname = "error";
      primary->expression.expr = expr;
      return primary;
    } else assert(0);
    assert(0);
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_indexExpression()
{
  Ast* index_expr;

  if (token_is_expression(this->token)) {
    index_expr = (Ast*)this->storage->malloc(sizeof(Ast));
    index_expr->kind = AstEnum::indexExpression;
    index_expr->line_no = this->token->line_no;
    index_expr->column_no = this->token->column_no;
    index_expr->indexExpression.start_index = parse_expression(parser, 1);
    if (this->token->klass == TokenClass::COLON) {
      this->next_token();
      if (token_is_expression(this->token)) {
        index_expr->indexExpression.end_index = parse_expression(parser, 1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
    }
    return index_expr;
  } else error("%s:%d:%d: expression or `:` was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_integer()
{
  Ast* int_literal;

  if (this->token->klass == TokenClass::INTEGER_LITERAL) {
    int_literal = (Ast*)this->storage->malloc(sizeof(Ast));
    int_literal->kind = AstEnum::integerLiteral;
    int_literal->line_no = this->token->line_no;
    int_literal->column_no = this->token->column_no;
    int_literal->integerLiteral.is_signed = this->token->integer.is_signed;
    int_literal->integerLiteral.width = this->token->integer.width;
    int_literal->integerLiteral.value = this->token->integer.value;
    this->next_token();
    return int_literal;
  } else error("%s:%d:%d: error: integer was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_boolean()
{
  Ast* bool_literal;

  if (this->token->klass == TokenClass::TRUE || this->token->klass == TokenClass::FALSE) {
    bool_literal = (Ast*)this->storage->malloc(sizeof(Ast));
    bool_literal->kind = AstEnum::booleanLiteral;
    bool_literal->line_no = this->token->line_no;
    bool_literal->column_no = this->token->column_no;
    bool_literal->booleanLiteral.value = (this->token->klass == TokenClass::TRUE);
    this->next_token();
    return bool_literal;
  } else error("%s:%d:%d: error: boolean was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_string()
{
  Ast* string_literal;

  if (this->token->klass == TokenClass::STRING_LITERAL) {
    string_literal = (Ast*)this->storage->malloc(sizeof(Ast));
    string_literal->kind = AstEnum::stringLiteral;
    string_literal->line_no = this->token->line_no;
    string_literal->column_no = this->token->column_no;
    string_literal->stringLiteral.value = this->token->lexeme;
    this->next_token();
    return string_literal;
  } else error("%s:%d:%d: error: string was expected, got `%s`.",
               this->source_file, this->token->line_no, this->token->column_no, this->token->lexeme);
  assert(0);
  return 0;
}
