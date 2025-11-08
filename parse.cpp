#include <stdint.h>
#include "foundation.h"
#include "frontend.h"

struct Keyword {
  char* strname;
  enum TokenClass token_class;
};

/** PROGRAM **/

static Ast* parse_p4program(Parser* parser);
static Ast* parse_declarationList(Parser* parser);
static Ast* parse_declaration(Parser* parser);
static Ast* parse_nonTypeName(Parser* parser);
static Ast* parse_name(Parser* parser);
static Ast* parse_parameterList(Parser* parser);
static Ast* parse_parameter(Parser* parser);
static enum ParamDirection parse_direction(Parser* parser);
static Ast* parse_packageTypeDeclaration(Parser* parser);
static Ast* parse_instantiation(Parser* parser, Ast* type_ref);
static Ast* parse_constructorParameters(Parser* parser);

/** PARSER **/

static Ast* parse_parserDeclaration(Parser* parser, Ast* parser_proto);
static Ast* parse_parserLocalElements(Parser* parser);
static Ast* parse_parserLocalElement(Parser* parser);
static Ast* parse_parserTypeDeclaration(Parser* parser);
static Ast* parse_parserStates(Parser* parser);
static Ast* parse_parserState(Parser* parser);
static Ast* parse_parserStatements(Parser* parser);
static Ast* parse_parserStatement(Parser* parser);
static Ast* parse_parserBlockStatement(Parser* parser);
static Ast* parse_transitionStatement(Parser* parser);
static Ast* parse_stateExpression(Parser* parser);
static Ast* parse_selectExpression(Parser* parser);
static Ast* parse_selectCaseList(Parser* parser);
static Ast* parse_selectCase(Parser* parser);
static Ast* parse_keysetExpression(Parser* parser);
static Ast* parse_tupleKeysetExpression(Parser* parser);
static Ast* parse_simpleExpressionList(Parser* parser);
static Ast* parse_simpleKeysetExpression(Parser* parser);

/** CONTROL **/

static Ast* parse_controlDeclaration(Parser* parser, Ast* control_proto);
static Ast* parse_controlTypeDeclaration(Parser* parser);
static Ast* parse_controlLocalDeclaration(Parser* parser);
static Ast* parse_controlLocalDeclarations(Parser* parser);

/** EXTERN **/

static Ast* parse_externDeclaration(Parser* parser);
static Ast* parse_methodPrototypes(Parser* parser);
static Ast* parse_functionPrototype(Parser* parser, Ast* return_type);
static Ast* parse_methodPrototype(Parser* parser);

/** TYPES **/

static Ast* parse_typeRef(Parser* parser);
static Ast* parse_namedType(Parser* parser);
static Ast* parse_typeName(Parser* parser);
static Ast* parse_tupleType(Parser* parser);
static Ast* parse_headerStackType(Parser* parser, Ast* named_type);
static Ast* parse_baseType(Parser* parser);
static Ast* parse_integerTypeSize(Parser* parser);
static Ast* parse_typeOrVoid(Parser* parser);
static Ast* parse_realTypeArg(Parser* parser);
static Ast* parse_typeArg(Parser* parser);
static Ast* parse_typeArgumentList(Parser* parser);
static Ast* parse_typeDeclaration(Parser* parser);
static Ast* parse_derivedTypeDeclaration(Parser* parser);
static Ast* parse_headerTypeDeclaration(Parser* parser);
static Ast* parse_headerUnionDeclaration(Parser* parser);
static Ast* parse_structTypeDeclaration(Parser* parser);
static Ast* parse_structFieldList(Parser* parser);
static Ast* parse_structField(Parser* parser);
static Ast* parse_enumDeclaration(Parser* parser);
static Ast* parse_errorDeclaration(Parser* parser);
static Ast* parse_matchKindDeclaration(Parser* parser);
static Ast* parse_identifierList(Parser* parser);
static Ast* parse_specifiedIdentifierList(Parser* parser);
static Ast* parse_specifiedIdentifier(Parser* parser);
static Ast* parse_typedefDeclaration(Parser* parser);

/** STATEMENTS **/

static Ast* parse_assignmentOrMethodCallStatement(Parser* parser);
static Ast* parse_returnStatement(Parser* parser);
static Ast* parse_exitStatement(Parser* parser);
static Ast* parse_conditionalStatement(Parser* parser);
static Ast* parse_directApplication(Parser* parser, Ast* type_name);
static Ast* parse_statement(Parser* parser, Ast* type_name);
static Ast* parse_blockStatement(Parser* parser);
static Ast* parse_statementOrDeclList(Parser* parser);
static Ast* parse_switchStatement(Parser* parser);
static Ast* parse_switchCases(Parser* parser);
static Ast* parse_switchCase(Parser* parser);
static Ast* parse_switchLabel(Parser* parser);
static Ast* parse_statementOrDeclaration(Parser* parser);

/** TABLES **/

static Ast* parse_tableDeclaration(Parser* parser);
static Ast* parse_tablePropertyList(Parser* parser);
static Ast* parse_tableProperty(Parser* parser);
static Ast* parse_keyElementList(Parser* parser);
static Ast* parse_keyElement(Parser* parser);
static Ast* parse_actionList(Parser* parser);
static Ast* parse_actionRef(Parser* parser);
static Ast* parse_entriesList(Parser* parser);
static Ast* parse_entry(Parser* parser);
static Ast* parse_actionDeclaration(Parser* parser);

/** VARIABLES **/

static Ast* parse_variableDeclaration(Parser* parser, Ast* type_ref);

/** EXPRESSIONS **/

static Ast* parse_functionDeclaration(Parser* parser, Ast* type_ref);
static Ast* parse_argumentList(Parser* parser);
static Ast* parse_argument(Parser* parser);
static Ast* parse_expressionList(Parser* parser);
static Ast* parse_lvalue(Parser* parser);
static Ast* parse_expression(Parser* parser, int priority_threshold);
static Ast* parse_expressionPrimary(Parser* parser);
static Ast* parse_indexExpression(Parser* parser);
static Ast* parse_integer(Parser* parser);
static Ast* parse_boolean(Parser* parser);
static Ast* parse_string(Parser* parser);

static void define_keywords(Parser* parser, Scope* scope)
{
  struct Keyword keywords[] = {
    {"action",  TK_ACTION},
    {"actions", TK_ACTIONS},
    {"entries", TK_ENTRIES},
    {"enum",    TK_ENUM},
    {"in",      TK_IN},
    {"package", TK_PACKAGE},
    {"select",  TK_SELECT},
    {"switch",  TK_SWITCH},
    {"tuple",   TK_TUPLE},
    {"control", TK_CONTROL},
    {"error",   TK_ERROR},
    {"header",  TK_HEADER},
    {"inout",   TK_INOUT},
    {"parser",  TK_PARSER},
    {"state",   TK_STATE},
    {"table",   TK_TABLE},
    {"key",     TK_KEY},
    {"typedef", TK_TYPEDEF},
    {"default", TK_DEFAULT},
    {"extern",  TK_EXTERN},
    {"out",     TK_OUT},
    {"else",    TK_ELSE},
    {"exit",    TK_EXIT},
    {"if",      TK_IF},
    {"return",  TK_RETURN},
    {"struct",  TK_STRUCT},
    {"apply",   TK_APPLY},
    {"const",   TK_CONST},
    {"bool",    TK_BOOL},
    {"true",    TK_TRUE},
    {"false",   TK_FALSE},
    {"void",    TK_VOID},
    {"int",     TK_INT},
    {"bit",     TK_BIT},
    {"varbit",  TK_VARBIT},
    {"string",  TK_STRING},
    {"match_kind",   TK_MATCH_KIND},
    {"transition",   TK_TRANSITION},
    {"header_union", TK_UNION},
  };
  NameDeclaration* name_decl;

  for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
    name_decl = scope->bind(parser->storage, keywords[i].strname, NameSpace::KEYWORD);
    name_decl->token_class = keywords[i].token_class;
  }
}

static Token* next_token(Parser* parser)
{
  assert(parser->token_at < parser->tokens->elem_count);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  parser->prev_token = parser->token;
  parser->prev_token_at = parser->token_at;
  parser->token = (Token*)array_get(parser->tokens, ++parser->token_at, sizeof(Token));
  while (parser->token->klass == TK_COMMENT) {
    parser->token = (Token*)array_get(parser->tokens, ++parser->token_at, sizeof(Token));
  }
  if (parser->token->klass == TK_IDENTIFIER) {
    name_entry = parser->current_scope->lookup(parser->token->lexeme, NameSpace::KEYWORD | NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::KEYWORD >> 1];
    if (name_decl) {
      parser->token->klass = name_decl->token_class;
      return parser->token;
    }
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    if (name_decl) {
      parser->token->klass = TK_TYPE_IDENTIFIER;
      return parser->token;
    }
  }
  return parser->token;
}

static Token* peek_token(Parser* parser)
{
  Token* peek_token;

  parser->prev_token = parser->token;
  parser->prev_token_at = parser->token_at;
  peek_token = next_token(parser);
  parser->token = parser->prev_token;
  parser->token_at = parser->prev_token_at;
  return peek_token;
}

static bool token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES;
  return result;
}

static bool token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

static bool token_is_typeName(Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER;
}

static bool token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE;
  return result;
}

static bool token_is_baseType(Token* token)
{
  bool result = token->klass == TK_BOOL || token->klass == TK_ERROR || token->klass == TK_INT
    || token->klass == TK_BIT || token->klass == TK_VARBIT || token->klass == TK_STRING
    || token->klass == TK_VOID;
  return result;
}

static bool token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_TUPLE;
  return result;
}

static bool token_is_direction(Token* token)
{
  bool result = token->klass == TK_IN || token->klass == TK_OUT || token->klass == TK_INOUT;
  return result;
}

static bool token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

static bool token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TK_HEADER || token->klass == TK_UNION || token->klass == TK_STRUCT
    || token->klass == TK_ENUM;
  return result;
}

static bool token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF
    || token->klass == TK_PARSER || token->klass == TK_CONTROL || token->klass == TK_PACKAGE;
  return result;
}

static bool token_is_typeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

static bool token_is_typeOrVoid(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_VOID || token->klass == TK_IDENTIFIER;
  return result;
}

static bool token_is_actionRef(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_PARENTH_OPEN;
  return result;
}

static bool token_is_tableProperty(Token* token)
{
  bool result = token->klass == TK_KEY || token->klass == TK_ACTIONS;
#if 0
    || token->klass == TK_CONST || token->klass == TK_ENTRIES
    || token_is_nonTableKwName(token);
#endif
  return result;
}

static bool token_is_switchLabel(Token* token)
{
  bool result = token_is_name(token) || token->klass == TK_DEFAULT;
  return result;
}

static bool token_is_expressionPrimary(Token* token)
{
  bool result = token->klass == TK_INTEGER_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

static bool token_is_expression(Token* token)
{
  return token_is_expressionPrimary(token);
}

static bool token_is_methodPrototype(Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
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
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token);
  return result;
}

static bool token_is_lvalue(Token* token)
{
  bool result = token_is_nonTypeName(token) || (token->klass == TK_DOT);
  return result;
}

static bool token_is_assignmentOrMethodCallStatement(Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TK_PARENTH_OPEN || token->klass == TK_ANGLE_OPEN
    || token->klass == TK_EQUAL;
  return result;
}

static bool token_is_statement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TK_IF
    || token->klass == TK_SEMICOLON || token->klass == TK_BRACE_OPEN || token->klass == TK_EXIT
    || token->klass == TK_RETURN || token->klass == TK_SWITCH;
  return result;
}

static bool token_is_statementOrDeclaration(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_CONST || token_is_statement(token);
  return result;
}

static bool token_is_argument(Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TK_DONTCARE;
  return result;
}

static bool token_is_parserLocalElement(Token* token)
{
  bool result = token->klass == TK_CONST || token_is_typeRef(token);
  return result;
}

static bool token_is_parserStatement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_CONST || token_is_typeRef(token)
    || token->klass == TK_SEMICOLON;
  return result;
}

static bool token_is_simpleKeysetExpression(Token* token) {
  bool result = token_is_expression(token) || token->klass == TK_DEFAULT || token->klass == TK_DONTCARE;
  return result;
}

static bool token_is_keysetExpression(Token* token)
{
  bool result = token->klass == TK_TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

static bool token_is_selectCase(Token* token)
{
  return token_is_keysetExpression(token);
}

static bool token_is_controlLocalDeclaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_ACTION
    || token->klass == TK_TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

static bool token_is_realTypeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE|| token_is_typeRef(token);
  return result;
}

static bool token_is_binaryOperator(Token* token)
{
  bool result = token->klass == TK_STAR || token->klass == TK_SLASH
    || token->klass == TK_PLUS || token->klass == TK_MINUS
    || token->klass == TK_ANGLE_OPEN_EQUAL || token->klass == TK_ANGLE_CLOSE_EQUAL
    || token->klass == TK_ANGLE_OPEN || token->klass == TK_ANGLE_CLOSE
    || token->klass == TK_EXCLAMATION_EQUAL || token->klass == TK_DOUBLE_EQUAL
    || token->klass == TK_DOUBLE_PIPE || token->klass == TK_DOUBLE_AMPERSAND
    || token->klass == TK_PIPE || token->klass == TK_AMPERSAND
    || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN
    || token->klass == TK_DOUBLE_ANGLE_CLOSE || token->klass == TK_TRIPLE_AMPERSAND
    || token->klass == TK_EQUAL;
  return result;
}

static bool token_is_exprOperator(Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOT
    || token->klass == TK_BRACKET_OPEN || token->klass == TK_PARENTH_OPEN
    || token->klass == TK_ANGLE_OPEN;
  return result;
}

static int operator_priority(Token* token)
{
  if (token->klass == TK_DOUBLE_AMPERSAND || token->klass == TK_DOUBLE_PIPE) {
    /* Logical AND, OR */
    return 1;
  } else if (token->klass == TK_DOUBLE_EQUAL || token->klass == TK_EXCLAMATION_EQUAL
      || token->klass == TK_ANGLE_OPEN /* < */ || token->klass == TK_ANGLE_CLOSE /* > */
      || token->klass == TK_ANGLE_OPEN_EQUAL /* <= */ || token->klass == TK_ANGLE_CLOSE_EQUAL /* >= */) {
    /* Relational ops  */
    return 2;
  }
  else if (token->klass == TK_PLUS || token->klass == TK_MINUS
           || token->klass == TK_AMPERSAND || token->klass == TK_PIPE
           || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN /* << */
           || token->klass == TK_DOUBLE_ANGLE_CLOSE /* >> */) {
    /* Addition and subtraction; bitwise ops */
    return 3;
  }
  else if (token->klass == TK_STAR || token->klass == TK_SLASH) {
    /* Multiplication and division */
    return 4;
  }
  else if (token->klass == TK_TRIPLE_AMPERSAND) {
    /* Mask */
    return 5;
  }
  else assert(0);
  return 0;
}

static enum AstOperator token_to_binop(Token* token)
{
  switch (token->klass) {
    case TK_DOUBLE_AMPERSAND:
      return OP_AND;
    case TK_DOUBLE_PIPE:
      return OP_OR;
    case TK_DOUBLE_EQUAL:
      return OP_EQ;
    case TK_EXCLAMATION_EQUAL:
      return OP_NEQ;
    case TK_ANGLE_OPEN:
      return OP_LESS;
    case TK_ANGLE_CLOSE:
      return OP_GREAT;
    case TK_ANGLE_OPEN_EQUAL:
      return OP_LESS_EQ;
    case TK_ANGLE_CLOSE_EQUAL:
      return OP_GREAT_EQ;
    case TK_PLUS:
      return OP_ADD;
    case TK_MINUS:
      return OP_SUB;
    case TK_STAR:
      return OP_MUL;
    case TK_SLASH:
      return OP_DIV;
    case TK_AMPERSAND:
      return OP_BITW_AND;
    case TK_PIPE:
      return OP_BITW_OR;
    case TK_CIRCUMFLEX:
      return OP_BITW_XOR;
    case TK_DOUBLE_ANGLE_OPEN:
      return OP_BITW_SHL;
    case TK_DOUBLE_ANGLE_CLOSE:
      return OP_BITW_SHR;
    case TK_TRIPLE_AMPERSAND:
      return OP_MASK;
    default: return (AstOperator)0;
  }
}

char* AstEnum_to_string(enum AstEnum ast)
{
  switch (ast) {
    case AST_none: return "AST_none";

    /** PROGRAM **/

    case AST_p4program: return "AST_p4program";
    case AST_declarationList: return "AST_declarationList";
    case AST_declaration: return "AST_declaration";
    case AST_name: return "AST_name";
    case AST_parameterList: return "AST_parameterList";
    case AST_parameter: return "AST_parameter";
    case AST_paramDirection: return "AST_paramDirection";
    case AST_packageTypeDeclaration: return "AST_packageTypeDeclaration";
    case AST_instantiation: return "AST_instantiation";

    /** PARSER **/

    case AST_parserDeclaration: return "AST_parserDeclaration";
    case AST_parserTypeDeclaration: return "AST_parserTypeDeclaration";
    case AST_parserLocalElements: return "AST_parserLocalElements";
    case AST_parserLocalElement: return "AST_parserLocalElement";
    case AST_parserStates: return "AST_parserStates";
    case AST_parserState: return "AST_parserState";
    case AST_parserStatements: return "AST_parserStatements";
    case AST_parserStatement: return "AST_parserStatement";
    case AST_parserBlockStatement: return "AST_parserBlockStatement";
    case AST_transitionStatement: return "AST_transitionStatement";
    case AST_stateExpression: return "AST_stateExpression";
    case AST_selectExpression: return "AST_selectExpression";
    case AST_selectCaseList: return "AST_selectCaseList";
    case AST_selectCase: return "AST_selectCase";
    case AST_keysetExpression: return "AST_keysetExpression";
    case AST_tupleKeysetExpression: return "AST_tupleKeysetExpression";
    case AST_simpleKeysetExpression: return "AST_simpleKeysetExpression";
    case AST_simpleExpressionList: return "AST_simpleExpressionList";

    /** CONTROL **/

    case AST_controlDeclaration: return "AST_controlDeclaration";
    case AST_controlTypeDeclaration: return "AST_controlTypeDeclaration";
    case AST_controlLocalDeclarations: return "AST_controlLocalDeclarations";
    case AST_controlLocalDeclaration: return "AST_controlLocalDeclaration";

    /** TYPES **/

    case AST_typeRef: return "AST_typeRef";
    case AST_tupleType: return "AST_tupleType";
    case AST_headerStackType: return "AST_headerStackType";
    case AST_baseTypeBoolean: return "AST_baseTypeBoolean";
    case AST_baseTypeInteger: return "AST_baseTypeInteger";
    case AST_baseTypeBit: return "AST_baseTypeBit";
    case AST_baseTypeVarbit: return "AST_baseTypeVarbit";
    case AST_baseTypeString: return "AST_baseTypeString";
    case AST_baseTypeVoid: return "AST_baseTypeVoid";
    case AST_baseTypeError: return "AST_baseTypeError";
    case AST_integerTypeSize: return "AST_integerTypeSize";
    case AST_realTypeArg: return "AST_realTypeArg";
    case AST_typeArg: return "AST_typeArg";
    case AST_typeArgumentList: return "AST_typeArgumentList";
    case AST_typeDeclaration: return "AST_typeDeclaration";
    case AST_derivedTypeDeclaration: return "AST_derivedTypeDeclaration";
    case AST_headerTypeDeclaration: return "AST_headerTypeDeclaration";
    case AST_headerUnionDeclaration: return "AST_headerUnionDeclaration";
    case AST_structTypeDeclaration: return "AST_structTypeDeclaration";
    case AST_structFieldList: return "AST_structFieldList";
    case AST_structField: return "AST_structField";
    case AST_enumDeclaration: return "AST_enumDeclaration";
    case AST_errorDeclaration: return "AST_errorDeclaration";
    case AST_matchKindDeclaration: return "AST_matchKindDeclaration";
    case AST_identifierList: return "AST_identifierList";
    case AST_specifiedIdentifierList: return "AST_specifiedIdentifierList";
    case AST_specifiedIdentifier: return "AST_specifiedIdentifier";
    case AST_typedefDeclaration: return "AST_typedefDeclaration";

    /** STATEMENTS **/

    case AST_assignmentStatement: return "AST_assignmentStatement";
    case AST_emptyStatement: return "AST_emptyStatement";
    case AST_returnStatement: return "AST_returnStatement";
    case AST_exitStatement: return "AST_exitStatement";
    case AST_conditionalStatement: return "AST_conditionalStatement";
    case AST_directApplication: return "AST_directApplication";
    case AST_statement: return "AST_statement";
    case AST_blockStatement: return "AST_blockStatement";
    case AST_statementOrDeclaration: return "AST_statementOrDeclaration";
    case AST_statementOrDeclList: return "AST_statementOrDeclList";
    case AST_switchStatement: return "AST_switchStatement";
    case AST_switchCases: return "AST_switchCases";
    case AST_switchCase: return "AST_switchCase";
    case AST_switchLabel: return "AST_switchLabel";

    /** TABLES **/

    case AST_tableDeclaration: return "AST_tableDeclaration";
    case AST_tablePropertyList: return "AST_tablePropertyList";
    case AST_tableProperty: return "AST_tableProperty";
    case AST_keyProperty: return "AST_keyProperty";
    case AST_keyElementList: return "AST_keyElementList";
    case AST_keyElement: return "AST_keyElement";
    case AST_actionsProperty: return "AST_actionsProperty";
    case AST_actionList: return "AST_actionList";
    case AST_actionRef: return "AST_actionRef";
#if 0
    case AST_entriesProperty: return "AST_entriesProperty";
    case AST_entriesList: return "AST_entriesList";
    case AST_entry: return "AST_entry";
    case AST_simpleProperty: return "AST_simpleProperty";
#endif
    case AST_actionDeclaration: return "AST_actionDeclaration";

    /** VARIABLES **/

    case AST_variableDeclaration: return "AST_variableDeclaration";

    /** EXPRESSIONS **/

    case AST_functionDeclaration: return "AST_functionDeclaration";
    case AST_argumentList: return "AST_argumentList";
    case AST_argument: return "AST_argument";
    case AST_expressionList: return "AST_expressionList";
    case AST_expression: return "AST_expression";
    case AST_lvalueExpression: return "AST_lvalueExpression";
    case AST_binaryExpression: return "AST_binaryExpression";
    case AST_unaryExpression: return "AST_unaryExpression";
    case AST_functionCall: return "AST_functionCall";
    case AST_memberSelector: return "";
    case AST_castExpression: return "AST_castExpression";
    case AST_arraySubscript: return "AST_arraySubscript";
    case AST_indexExpression: return "AST_indexExpression";
    case AST_integerLiteral: return "AST_integerLiteral";
    case AST_stringLiteral: return "AST_stringLiteral";
    case AST_dontcare: return "AST_dontcare";
    case AST_default: return "AST_default";

    default: return "?";
  }
  assert(0);
  return 0;
}

Ast* Ast::clone(Arena* storage)
{
  Ast* clone, *sibling_clone, *child_clone;

  if (this == 0) return (Ast*)0;
  clone = (Ast*)arena_malloc(storage, sizeof(Ast));
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
  if (this->kind == AST_p4program) {
    clone->p4program.decl_list = this->p4program.decl_list->clone(storage);
  } else if (this->kind == AST_declarationList) {
    ;
  } else if (this->kind == AST_declaration) {
    clone->declaration.decl = this->declaration.decl->clone(storage);
  } else if (this->kind == AST_name) {
    clone->name.strname = this->name.strname;
  } else if (this->kind == AST_parameterList) {
    ;
  } else if (this->kind == AST_parameter) {
    clone->parameter.direction = this->parameter.direction;
    clone->parameter.name = this->parameter.name->clone(storage);
    clone->parameter.type = this->parameter.type->clone(storage);
    clone->parameter.init_expr = this->parameter.init_expr->clone(storage);
  } else if (this->kind == AST_packageTypeDeclaration) {
    clone->packageTypeDeclaration.name = this->packageTypeDeclaration.name->clone(storage);
    clone->packageTypeDeclaration.params = this->packageTypeDeclaration.params->clone(storage);
  } else if (this->kind == AST_instantiation) {
    clone->instantiation.name = this->instantiation.name->clone(storage);
    clone->instantiation.type = this->instantiation.type->clone(storage);
    clone->instantiation.args = this->instantiation.args->clone(storage);
  }
  /** PARSER **/
  else if (this->kind == AST_parserDeclaration) {
    clone->parserDeclaration.proto = this->parserDeclaration.proto->clone(storage);
    clone->parserDeclaration.ctor_params = this->parserDeclaration.ctor_params->clone(storage);
    clone->parserDeclaration.local_elements = this->parserDeclaration.local_elements->clone(storage);
    clone->parserDeclaration.states = this->parserDeclaration.states->clone(storage);
  } else if (this->kind == AST_parserTypeDeclaration) {
    clone->parserTypeDeclaration.name = this->parserTypeDeclaration.name->clone(storage);
    clone->parserTypeDeclaration.params = this->parserTypeDeclaration.params->clone(storage);
    clone->parserTypeDeclaration.method_protos = this->parserTypeDeclaration.method_protos->clone(storage);
  } else if (this->kind == AST_parserLocalElements) {
    ;
  } else if (this->kind == AST_parserLocalElement) {
    clone->parserLocalElement.element = this->parserLocalElement.element->clone(storage);
  } else if (this->kind == AST_parserStates) {
    ;
  } else if (this->kind == AST_parserState) {
    clone->parserState.name = this->parserState.name->clone(storage);
    clone->parserState.stmt_list = this->parserState.stmt_list->clone(storage);
    clone->parserState.transition_stmt = this->parserState.transition_stmt->clone(storage);
  } else if (this->kind == AST_parserStatements) {
    ;
  } else if (this->kind == AST_parserStatement) {
    clone->parserStatement.stmt = this->parserStatement.stmt->clone(storage);
  } else if (this->kind == AST_parserBlockStatement) {
    clone->parserBlockStatement.stmt_list = this->parserBlockStatement.stmt_list->clone(storage);
  } else if (this->kind == AST_transitionStatement) {
    clone->transitionStatement.stmt = this->transitionStatement.stmt->clone(storage);
  } else if (this->kind == AST_stateExpression) {
    clone->stateExpression.expr = this->stateExpression.expr->clone(storage);
  } else if (this->kind == AST_selectExpression) {
    clone->selectExpression.expr_list = this->selectExpression.expr_list->clone(storage);
    clone->selectExpression.case_list = this->selectExpression.case_list->clone(storage);
  } else if (this->kind == AST_selectCaseList) {
    ;
  } else if (this->kind == AST_selectCase) {
    clone->selectCase.keyset_expr = this->selectCase.keyset_expr->clone(storage);
    clone->selectCase.name = this->selectCase.name->clone(storage);
  } else if (this->kind == AST_keysetExpression) {
    clone->keysetExpression.expr = this->keysetExpression.expr->clone(storage);
  } else if (this->kind == AST_tupleKeysetExpression) {
    clone->tupleKeysetExpression.expr_list = this->tupleKeysetExpression.expr_list->clone(storage);
  } else if (this->kind == AST_simpleKeysetExpression) {
    clone->simpleKeysetExpression.expr = this->simpleKeysetExpression.expr->clone(storage);
  } else if (this->kind == AST_simpleExpressionList) {
    ;
  } else if (this->kind == AST_typeRef) {
    clone->typeRef.type = this->typeRef.type->clone(storage);
  } else if (this->kind == AST_tupleType) {
    clone->tupleType.type_args = this->tupleType.type_args->clone(storage);
  }
  /** CONTROL **/
  else if (this->kind == AST_controlDeclaration) {
    clone->controlDeclaration.proto = this->controlDeclaration.proto->clone(storage);
    clone->controlDeclaration.ctor_params = this->controlDeclaration.ctor_params->clone(storage);
    clone->controlDeclaration.local_decls = this->controlDeclaration.local_decls->clone(storage);
    clone->controlDeclaration.apply_stmt = this->controlDeclaration.apply_stmt->clone(storage);
  } else if (this->kind == AST_controlTypeDeclaration) {
    clone->controlTypeDeclaration.name = this->controlTypeDeclaration.name->clone(storage);
    clone->controlTypeDeclaration.params = this->controlTypeDeclaration.params->clone(storage);
    clone->controlTypeDeclaration.method_protos = this->controlTypeDeclaration.params->clone(storage);
  } else if (this->kind == AST_controlLocalDeclarations) {
    ;
  } else if (this->kind == AST_controlLocalDeclaration) {
    clone->controlLocalDeclaration.decl = this->controlLocalDeclaration.decl->clone(storage);
  }
  /** EXTERN **/
  else if (this->kind == AST_externDeclaration) {
    clone->externDeclaration.decl = this->externDeclaration.decl->clone(storage);
  } else if (this->kind == AST_externTypeDeclaration) {
    clone->externTypeDeclaration.name = this->externTypeDeclaration.name->clone(storage);
    clone->externTypeDeclaration.method_protos = this->externTypeDeclaration.method_protos->clone(storage);
  } else if (this->kind == AST_methodPrototypes) {
    ;
  } else if (this->kind == AST_functionPrototype) {
    clone->functionPrototype.return_type = this->functionPrototype.return_type->clone(storage);
    clone->functionPrototype.name = this->functionPrototype.name->clone(storage);
    clone->functionPrototype.params = this->functionPrototype.params->clone(storage);
  }
  /** TYPES **/
  else if (this->kind == AST_typeRef) {
    clone->typeRef.type = this->typeRef.type->clone(storage);
  } else if (this->kind == AST_tupleType) {
    clone->tupleType.type_args = this->tupleType.type_args->clone(storage);
  } else if (this->kind == AST_headerStackType) {
    clone->headerStackType.type = this->headerStackType.type->clone(storage);
    clone->headerStackType.stack_expr = this->headerStackType.stack_expr->clone(storage);
  } else if (this->kind == AST_baseTypeBoolean) {
    clone->baseTypeBoolean.name = this->baseTypeBoolean.name->clone(storage);
  } else if (this->kind == AST_baseTypeInteger) {
    clone->baseTypeInteger.name = this->baseTypeInteger.name->clone(storage);
    clone->baseTypeInteger.size = this->baseTypeInteger.size->clone(storage);
  } else if (this->kind == AST_baseTypeBit) {
    clone->baseTypeBit.name = this->baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = this->baseTypeBit.size->clone(storage);
  } else if (this->kind == AST_baseTypeBit) {
    clone->baseTypeBit.name = this->baseTypeBit.name->clone(storage);
    clone->baseTypeBit.size = this->baseTypeBit.size->clone(storage);
  } else if (this->kind == AST_baseTypeString) {
    clone->baseTypeString.name = this->baseTypeString.name->clone(storage);
  } else if (this->kind == AST_baseTypeVoid) {
    clone->baseTypeVoid.name = this->baseTypeVoid.name->clone(storage);
  } else if (this->kind == AST_baseTypeError) {
    clone->baseTypeError.name = this->baseTypeError.name->clone(storage);
  } else if (this->kind == AST_integerTypeSize) {
    clone->integerTypeSize.size = this->integerTypeSize.size->clone(storage);
  } else if (this->kind == AST_realTypeArg) {
    clone->realTypeArg.arg = this->realTypeArg.arg->clone(storage);
  } else if (this->kind == AST_typeArg) {
    clone->typeArg.arg = this->typeArg.arg->clone(storage);
  } else if (this->kind == AST_typeArgumentList) {
    ;
  } else if (this->kind == AST_typeDeclaration) {
    clone->typeDeclaration.decl = this->typeDeclaration.decl->clone(storage);
  } else if (this->kind == AST_derivedTypeDeclaration) {
    clone->derivedTypeDeclaration.decl = this->derivedTypeDeclaration.decl->clone(storage);
  } else if (this->kind == AST_headerTypeDeclaration) {
    clone->headerTypeDeclaration.name = this->headerTypeDeclaration.name->clone(storage);
    clone->headerTypeDeclaration.fields = this->headerTypeDeclaration.fields->clone(storage);
  } else if (this->kind == AST_headerUnionDeclaration) {
    clone->headerUnionDeclaration.name = this->headerUnionDeclaration.name->clone(storage);
    clone->headerUnionDeclaration.fields = this->headerUnionDeclaration.fields->clone(storage);
  } else if (this->kind == AST_structTypeDeclaration) {
    clone->structTypeDeclaration.name = this->structTypeDeclaration.name->clone(storage);
    clone->structTypeDeclaration.fields = this->structTypeDeclaration.fields->clone(storage);
  } else if (this->kind == AST_structFieldList) {
    ;
  } else if (this->kind == AST_structField) {
    clone->structField.type = this->structField.type->clone(storage);
    clone->structField.name = this->structField.name->clone(storage);
  } else if (this->kind == AST_enumDeclaration) {
    clone->enumDeclaration.type_size = this->enumDeclaration.type_size->clone(storage);
    clone->enumDeclaration.name = this->enumDeclaration.name->clone(storage);
    clone->enumDeclaration.fields = this->enumDeclaration.fields->clone(storage);
  } else if (this->kind == AST_errorDeclaration) {
    clone->errorDeclaration.fields = this->errorDeclaration.fields->clone(storage);
  } else if (this->kind == AST_matchKindDeclaration) {
    clone->matchKindDeclaration.fields = this->matchKindDeclaration.fields->clone(storage);
  } else if (this->kind == AST_matchKindDeclaration) {
    ;
  } else if (this->kind == AST_specifiedIdentifierList) {
    ;
  } else if (this->kind == AST_specifiedIdentifier) {
    clone->specifiedIdentifier.name = this->specifiedIdentifier.name->clone(storage);
    clone->specifiedIdentifier.init_expr = this->specifiedIdentifier.init_expr->clone(storage);
  } else if (this->kind == AST_typedefDeclaration) {
    clone->typedefDeclaration.type_ref = this->typedefDeclaration.type_ref->clone(storage);
    clone->typedefDeclaration.name = this->typedefDeclaration.name->clone(storage);
  }
  /** STATEMENTS **/
  else if (this->kind == AST_assignmentStatement) {
    clone->assignmentStatement.lhs_expr = this->assignmentStatement.lhs_expr->clone(storage);
    clone->assignmentStatement.rhs_expr = this->assignmentStatement.rhs_expr->clone(storage);
  } else if (this->kind == AST_emptyStatement) {
    ;
  } else if (this->kind == AST_returnStatement) {
    clone->returnStatement.expr = this->returnStatement.expr->clone(storage);
  } else if (this->kind == AST_returnStatement) {
    ;
  } else if (this->kind == AST_conditionalStatement) {
    clone->conditionalStatement.cond_expr = this->conditionalStatement.cond_expr->clone(storage);
    clone->conditionalStatement.stmt = this->conditionalStatement.stmt->clone(storage);
    clone->conditionalStatement.else_stmt = this->conditionalStatement.else_stmt->clone(storage);
  } else if (this->kind == AST_directApplication) {
    clone->directApplication.name = this->directApplication.name->clone(storage);
    clone->directApplication.args = this->directApplication.args->clone(storage);
  } else if (this->kind == AST_statement) {
    clone->statement.stmt = this->statement.stmt->clone(storage);
  } else if (this->kind == AST_blockStatement) {
    clone->blockStatement.stmt_list = this->blockStatement.stmt_list->clone(storage);
  } else if (this->kind == AST_statementOrDeclaration) {
    clone->statementOrDeclaration.stmt = this->statementOrDeclaration.stmt->clone(storage);
  } else if (this->kind == AST_statementOrDeclList) {
    ;
  } else if (this->kind == AST_switchStatement) {
    clone->switchStatement.expr = this->switchStatement.expr->clone(storage);
    clone->switchStatement.switch_cases = this->switchStatement.switch_cases->clone(storage);
  } else if (this->kind == AST_switchCases) {
    ;
  } else if (this->kind == AST_switchCase) {
    clone->switchCase.label = this->switchCase.label->clone(storage);
    clone->switchCase.stmt = this->switchCase.stmt->clone(storage);
  } else if (this->kind == AST_switchLabel) {
    clone->switchLabel.label = this->switchLabel.label->clone(storage);
  }
  /** TABLES **/
  else if (this->kind == AST_tableDeclaration) {
    clone->tableDeclaration.name = this->tableDeclaration.name->clone(storage);
    clone->tableDeclaration.prop_list = this->tableDeclaration.prop_list->clone(storage);
  } else if (this->kind == AST_tablePropertyList) {
    ;
  } else if (this->kind == AST_tableProperty) {
    clone->tableProperty.prop = this->tableProperty.prop->clone(storage);
  } else if (this->kind == AST_keyProperty) {
    clone->keyProperty.keyelem_list = this->keyProperty.keyelem_list->clone(storage);
  } else if (this->kind == AST_keyElementList) {
    ;
  } else if (this->kind == AST_keyElement) {
    clone->keyElement.expr = this->keyElement.expr->clone(storage);
    clone->keyElement.match = this->keyElement.match->clone(storage);
  } else if (this->kind == AST_actionsProperty) {
    clone->actionsProperty.action_list = this->actionsProperty.action_list->clone(storage);
  } else if (this->kind == AST_actionList) {
    ;
  } else if (this->kind == AST_actionRef) {
    clone->actionRef.name = this->actionRef.name->clone(storage);
    clone->actionRef.args = this->actionRef.args->clone(storage);
  }
#if 0
  else if (this->kind == AST_entriesProperty) {
    clone->entriesProperty.entries_list = this->entriesProperty.entries_list->clone(storage);
  } else if (this->kind == AST_entriesList) {
    ;
  } else if (this->kind == AST_entry) {
    clone->entry.keyset = this->entry.keyset->clone(storage);
    clone->entry.action = this->entry.action->clone(storage);
  } else if (this->kind == AST_simpleProperty) {
    clone->simpleProperty.name = this->simpleProperty.name->clone(storage);
    clone->simpleProperty.init_expr = this->simpleProperty.init_expr->clone(storage);
    clone->simpleProperty.is_const = this->simpleProperty.is_const;
  }
#endif
  else if (this->kind == AST_actionDeclaration) {
    clone->actionDeclaration.name = this->actionDeclaration.name->clone(storage);
    clone->actionDeclaration.params = this->actionDeclaration.params->clone(storage);
    clone->actionDeclaration.stmt = this->actionDeclaration.stmt->clone(storage);
  }
  /** VARIABLES **/
  else if (this->kind == AST_variableDeclaration) {
    clone->variableDeclaration.type = this->variableDeclaration.type->clone(storage);
    clone->variableDeclaration.name = this->variableDeclaration.name->clone(storage);
    clone->variableDeclaration.init_expr = this->variableDeclaration.init_expr->clone(storage);
    clone->variableDeclaration.is_const = this->variableDeclaration.is_const;
  }
  /** EXPRESSIONS **/
  else if (this->kind == AST_functionDeclaration) {
    clone->functionDeclaration.proto = this->functionDeclaration.proto->clone(storage);
    clone->functionDeclaration.stmt = this->functionDeclaration.stmt->clone(storage);
  } else if (this->kind == AST_argumentList) {
    ;
  } else if (this->kind == AST_argument) {
    clone->argument.arg = this->argument.arg->clone(storage);
  } else if (this->kind == AST_expressionList) {
    ;
  } else if (this->kind == AST_expression) {
    clone->expression.expr = this->expression.expr->clone(storage);
  } else if (this->kind == AST_lvalueExpression) {
    clone->lvalueExpression.expr = this->lvalueExpression.expr->clone(storage);
  } else if (this->kind == AST_binaryExpression) {
    clone->binaryExpression.op = this->binaryExpression.op;
    clone->binaryExpression.strname = this->binaryExpression.strname;
    clone->binaryExpression.left_operand = this->binaryExpression.left_operand->clone(storage);
    clone->binaryExpression.right_operand = this->binaryExpression.right_operand->clone(storage);
  } else if (this->kind == AST_unaryExpression) {
    clone->unaryExpression.op = this->unaryExpression.op;
    clone->unaryExpression.strname = this->unaryExpression.strname;
    clone->unaryExpression.operand = this->unaryExpression.operand->clone(storage);
  } else if (this->kind == AST_functionCall) {
    clone->functionCall.lhs_expr = this->functionCall.lhs_expr->clone(storage);
    clone->functionCall.args = this->functionCall.args->clone(storage);
  } else if (this->kind == AST_memberSelector) {
    clone->memberSelector.lhs_expr = this->memberSelector.lhs_expr->clone(storage);
    clone->memberSelector.name = this->memberSelector.name->clone(storage);
  } else if (this->kind == AST_castExpression) {
    clone->castExpression.type = this->castExpression.type->clone(storage);
    clone->castExpression.expr = this->castExpression.expr->clone(storage);
  } else if (this->kind == AST_arraySubscript) {
    clone->arraySubscript.lhs_expr = this->arraySubscript.lhs_expr->clone(storage);
    clone->arraySubscript.index_expr = this->arraySubscript.index_expr->clone(storage);
  } else if (this->kind == AST_indexExpression) {
    clone->indexExpression.start_index = this->indexExpression.start_index->clone(storage);
    clone->indexExpression.end_index = this->indexExpression.end_index->clone(storage);
  } else if (this->kind == AST_integerLiteral) {
    clone->integerLiteral.is_signed = this->integerLiteral.is_signed;
    clone->integerLiteral.value = this->integerLiteral.value;
    clone->integerLiteral.width = this->integerLiteral.width;
  } else if (this->kind == AST_booleanLiteral) {
    clone->booleanLiteral.value = this->booleanLiteral.value;
  } else if (this->kind == AST_stringLiteral) {
    clone->stringLiteral.value = this->stringLiteral.value;
  } else if (this->kind == AST_default || this->kind == AST_dontcare) {
    ;
  }
  else assert(0);
  return clone;
}

void parse(Parser* parser)
{
  parser->root_scope = Scope::create(parser->storage, 5);
  parser->current_scope = parser->root_scope;

  define_keywords(parser, parser->root_scope);
  parser->token_at = 0;
  parser->token = (Token*)array_get(parser->tokens, parser->token_at, sizeof(Token));
  next_token(parser);
  parser->p4program = parse_p4program(parser);
  assert(parser->current_scope == parser->root_scope);
}

/** PROGRAM **/

static Ast* parse_p4program(Parser* parser)
{
  Ast* p4program;
  Scope* scope;

  p4program = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  p4program->kind = AST_p4program;
  p4program->line_no = parser->token->line_no;
  p4program->column_no = parser->token->column_no;
  while (parser->token->klass == TK_SEMICOLON) {
    next_token(parser); /* empty declaration */
  }
  scope = Scope::create(parser->storage, 6);
  parser->current_scope = scope->push(parser->current_scope);
  p4program->p4program.decl_list = parse_declarationList(parser);
  parser->current_scope = parser->current_scope->pop();
  if (parser->token->klass != TK_END_OF_INPUT) {
    error("%s:%d:%d: error: unexpected token `%s`.",
          parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  }
  return p4program;
}

static Ast* parse_declarationList(Parser* parser)
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {0};

  decls = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  decls->kind = AST_declarationList;
  decls->line_no = parser->token->line_no;
  decls->column_no = parser->token->column_no;
  if (token_is_declaration(parser->token)) {
    ast = parse_declaration(parser);
    ast_tree_append_node(&decls->tree, &tree_ctor, &ast->tree);
    while (token_is_declaration(parser->token) || parser->token->klass == TK_SEMICOLON) {
      if (token_is_declaration(parser->token)) {
        ast = parse_declaration(parser);
        ast_tree_append_node(&decls->tree, &tree_ctor, &ast->tree);
      } else if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser); /* empty declaration */
      }
    }
  }
  return decls;
}

static Ast* parse_declaration(Parser* parser)
{
  Ast* decl;

  if (token_is_declaration(parser->token)) {
    decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    decl->kind = AST_declaration;
    decl->line_no = parser->token->line_no;
    decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_CONST) {
      decl->declaration.decl = parse_variableDeclaration(parser, 0);
      return decl;
    } else if (parser->token->klass == TK_EXTERN) {
      decl->declaration.decl = parse_externDeclaration(parser);
      return decl;
    } else if (parser->token->klass == TK_ACTION) {
      decl->declaration.decl = parse_actionDeclaration(parser);
      return decl;
    } else if (parser->token->klass == TK_PARSER) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else {
        decl->declaration.decl = parse_parserDeclaration(parser, decl->declaration.decl);
      }
      return decl;
    } else if (parser->token->klass == TK_CONTROL) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else {
        decl->declaration.decl = parse_controlDeclaration(parser, decl->declaration.decl);
      }
      return decl;
    } else if (token_is_typeDeclaration(parser->token)) {
      decl->declaration.decl = parse_typeDeclaration(parser);
      return decl;
    } else if (parser->token->klass == TK_ERROR) {
      decl->declaration.decl = parse_errorDeclaration(parser);
      return decl;
    } else if (parser->token->klass == TK_MATCH_KIND) {
      decl->declaration.decl = parse_matchKindDeclaration(parser);
      return decl;
    } else if (token_is_typeRef(parser->token)) {
      Ast* type_ref = parse_typeRef(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        decl->declaration.decl = parse_instantiation(parser, type_ref);
        return decl;
      } else if (token_is_name(parser->token)) {
        decl->declaration.decl = parse_functionDeclaration(parser, type_ref);
        return decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(parser->token)) {
      decl->declaration.decl = parse_functionDeclaration(parser, parse_typeRef(parser));
      return decl;
    } else assert(0);
  } else error("%s:%d:%d: error: top-level declaration was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_nonTypeName(Parser* parser)
{
  Ast* name;

  if (token_is_nonTypeName(parser->token)) {
    name = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = parser->token->line_no;
    name->column_no = parser->token->column_no;
    name->name.strname = parser->token->lexeme;
    next_token(parser);
    return name;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_name(Parser* parser)
{
  Ast* type_name;

  if (token_is_name(parser->token)) {
    if (token_is_nonTypeName(parser->token)) {
      return parse_nonTypeName(parser);
    } else if (parser->token->klass == TK_TYPE_IDENTIFIER) {
      type_name = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type_name->kind = AST_name;
      type_name->line_no = parser->token->line_no;
      type_name->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      next_token(parser);
      return type_name;
    } else assert(0);
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parameterList(Parser* parser)
{
  Ast* params, *ast;
  AstTreeCtor tree_ctor = {0};

  params = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  params->kind = AST_parameterList;
  params->line_no = parser->token->line_no;
  params->column_no = parser->token->column_no;
  if (token_is_parameter(parser->token)) {
    ast = parse_parameter(parser);
    ast_tree_append_node(&params->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_parameter(parser);
      ast_tree_append_node(&params->tree, &tree_ctor, &ast->tree);
    }
  }
  return params;
}

static Ast* parse_parameter(Parser* parser)
{
  Ast* param;

  if (token_is_parameter(parser->token)) {
    param = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    param->kind = AST_parameter;
    param->line_no = parser->token->line_no;
    param->column_no = parser->token->column_no;
    param->parameter.direction = parse_direction(parser);
    param->parameter.type = parse_typeRef(parser);
    if (token_is_name(parser->token)) {
      param->parameter.name = parse_name(parser);
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        if (token_is_expression(parser->token)) {
          param->parameter.init_expr = parse_expression(parser, 1);
        } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      }
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return param;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static enum ParamDirection parse_direction(Parser* parser)
{
  if (token_is_direction(parser->token)) {
    if (parser->token->klass == TK_IN) {
      next_token(parser);
      return ParamDirection::IN;
    } else if (parser->token->klass == TK_OUT) {
      next_token(parser);
      return ParamDirection::OUT;
    } else if (parser->token->klass == TK_INOUT) {
      next_token(parser);
      return ParamDirection::IN | ParamDirection::OUT;
    } else assert(0);
  }
  return (ParamDirection)0;
}

static Ast* parse_packageTypeDeclaration(Parser* parser)
{
  Ast* package_decl, *name;

  if (parser->token->klass == TK_PACKAGE) {
    next_token(parser);
    package_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    package_decl->kind = AST_packageTypeDeclaration;
    package_decl->line_no = parser->token->line_no;
    package_decl->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      package_decl->packageTypeDeclaration.name = name;
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        package_decl->packageTypeDeclaration.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return package_decl;
  } else error("%s:%d:%d: error: `package` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_instantiation(Parser* parser, Ast* type_ref)
{
  Ast* inst_stmt;

  if (token_is_typeRef(parser->token) || type_ref) {
    inst_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    inst_stmt->kind = AST_instantiation;
    inst_stmt->line_no = parser->token->line_no;
    inst_stmt->column_no = parser->token->column_no;
    inst_stmt->instantiation.type = type_ref ? type_ref : parse_typeRef(parser);
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      inst_stmt->instantiation.args = parse_argumentList(parser);
      if (parser->token->klass == TK_PARENTH_CLOSE) {
        next_token(parser);
        if (token_is_name(parser->token)) {
          inst_stmt->instantiation.name = parse_name(parser);
          if (parser->token->klass == TK_SEMICOLON) {
            next_token(parser);
          } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: instance name was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return inst_stmt;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** PARSER **/

static Ast* parse_constructorParameters(Parser* parser)
{
   Ast* params;

  if (parser->token->klass == TK_PARENTH_OPEN) {
    next_token(parser);
    params = parse_parameterList(parser);
    if (parser->token->klass == TK_PARENTH_CLOSE) {
      next_token(parser);
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return params;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  return 0;
}

static Ast* parse_parserDeclaration(Parser* parser, Ast* parser_proto)
{
  Ast* parser_decl;

  if (parser->token->klass == TK_PARENTH_OPEN || parser->token->klass == TK_BRACE_OPEN) {
    parser_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    parser_decl->kind = AST_parserDeclaration;
    parser_decl->line_no = parser->token->line_no;
    parser_decl->column_no = parser->token->column_no;
    parser_decl->parserDeclaration.proto = parser_proto;
    parser_decl->parserDeclaration.ctor_params = parse_constructorParameters(parser);
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      parser_decl->parserDeclaration.local_elements = parse_parserLocalElements(parser);
      if (parser->token->klass == TK_STATE) {
        parser_decl->parserDeclaration.states = parse_parserStates(parser);
      } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      if (parser->token->klass == TK_BRACE_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return parser_decl;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parserLocalElements(Parser* parser)
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {0};

  elems = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  elems->kind = AST_parserLocalElements;
  elems->line_no = parser->token->line_no;
  elems->column_no = parser->token->column_no;
  if (token_is_parserLocalElement(parser->token)) {
    ast = parse_parserLocalElement(parser);
    ast_tree_append_node(&elems->tree, &tree_ctor, &ast->tree);
    while (token_is_parserLocalElement(parser->token)) {
      ast = parse_parserLocalElement(parser);
      ast_tree_append_node(&elems->tree, &tree_ctor, &ast->tree);
    }
  }
  return elems;
}

static Ast* parse_parserLocalElement(Parser* parser)
{
  Ast* local_element, *type_ref;

  if (token_is_parserLocalElement(parser->token)) {
    local_element = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    local_element->kind = AST_parserLocalElement;
    local_element->line_no = parser->token->line_no;
    local_element->column_no = parser->token->column_no;
    if (parser->token->klass == TK_CONST) {
      local_element->parserLocalElement.element = parse_variableDeclaration(parser, 0);
      return local_element;
    } else if (token_is_typeRef(parser->token)) {
      type_ref = parse_typeRef(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        local_element->parserLocalElement.element = parse_instantiation(parser, type_ref);
        return local_element;
      } else if (token_is_name(parser->token)) {
        local_element->parserLocalElement.element = parse_variableDeclaration(parser, type_ref);
        return local_element;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parserTypeDeclaration(Parser* parser)
{
  Ast* parser_proto, *name, *method_protos;

  if (parser->token->klass == TK_PARSER) {
    next_token(parser);
    parser_proto = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    parser_proto->kind = AST_parserTypeDeclaration;
    parser_proto->line_no = parser->token->line_no; 
    parser_proto->column_no = parser->token->column_no;
    method_protos = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    method_protos->kind = AST_methodPrototypes;
    method_protos->line_no = parser_proto->line_no;
    method_protos->column_no = parser_proto->column_no;
    parser_proto->parserTypeDeclaration.method_protos = method_protos;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      parser_proto->parserTypeDeclaration.name = name;
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        parser_proto->parserTypeDeclaration.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return parser_proto;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parserStates(Parser* parser)
{
  Ast* states, *ast;
  AstTreeCtor tree_ctor = {0};

  states = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  states->kind = AST_parserStates;
  states->line_no = parser->token->line_no;
  states->column_no = parser->token->column_no;
  if (parser->token->klass == TK_STATE) {
    ast = parse_parserState(parser);
    ast_tree_append_node(&states->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_STATE) {
      ast = parse_parserState(parser);
      ast_tree_append_node(&states->tree, &tree_ctor, &ast->tree);
    }
  }
  return states;
}

static Ast* parse_parserState(Parser* parser)
{
  Ast* state;

  if (parser->token->klass == TK_STATE) {
    next_token(parser);
    state = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    state->kind = AST_parserState;
    state->line_no = parser->token->line_no;
    state->column_no = parser->token->column_no;
    state->parserState.name = parse_name(parser);
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      state->parserState.stmt_list = parse_parserStatements(parser);
      state->parserState.transition_stmt = parse_transitionStatement(parser);
      if (parser->token->klass == TK_BRACE_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return state;
  } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parserStatements(Parser* parser)
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {0};

  stmts = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  stmts->kind = AST_parserStatements;
  stmts->line_no = parser->token->line_no;
  stmts->column_no = parser->token->column_no;
  if (token_is_parserStatement(parser->token)) {
    ast = parse_parserStatement(parser);
    ast_tree_append_node(&stmts->tree, &tree_ctor, &ast->tree);
    while (token_is_parserStatement(parser->token)) {
      ast = parse_parserStatement(parser);
      ast_tree_append_node(&stmts->tree, &tree_ctor, &ast->tree);
    }
  }
  return stmts;
}

static Ast* parse_parserStatement(Parser* parser)
{
  Ast* parser_stmt, *type_ref;

  if (token_is_parserStatement(parser->token)) {
    parser_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    parser_stmt->kind = AST_parserStatement;
    parser_stmt->line_no = parser->token->line_no;
    parser_stmt->column_no = parser->token->column_no;
    if (token_is_typeRef(parser->token)) {
      type_ref = parse_typeRef(parser);
      if (token_is_name(parser->token)) {
        parser_stmt->parserStatement.stmt = parse_variableDeclaration(parser, type_ref);
        return parser_stmt;
      } else {
        parser_stmt->parserStatement.stmt = parse_directApplication(parser, type_ref);
        return parser_stmt;
      }
    } else if (token_is_assignmentOrMethodCallStatement(parser->token)) {
      parser_stmt->parserStatement.stmt = parse_assignmentOrMethodCallStatement(parser);
      return parser_stmt;
    } else if (parser->token->klass == TK_BRACE_OPEN) {
      parser_stmt->parserStatement.stmt = parse_parserBlockStatement(parser);
      return parser_stmt;
    } else if (parser->token->klass == TK_CONST) {
      parser_stmt->parserStatement.stmt = parse_variableDeclaration(parser, 0);
      return parser_stmt;
    } else if (parser->token->klass == TK_SEMICOLON) {
      Ast* stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      stmt->kind = AST_emptyStatement;
      stmt->line_no = parser->token->line_no;
      stmt->column_no = parser->token->column_no;
      parser_stmt->parserStatement.stmt = stmt;
      next_token(parser);
      return parser_stmt;
    } else assert(0);
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_parserBlockStatement(Parser* parser)
{
  Ast* stmt;

  if (parser->token->klass == TK_BRACE_OPEN) {
    next_token(parser);
    stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    stmt->kind = AST_parserBlockStatement;
    stmt->line_no = parser->token->line_no;
    stmt->column_no = parser->token->column_no;
    stmt->parserBlockStatement.stmt_list = parse_parserStatements(parser);
    if (parser->token->klass == TK_BRACE_CLOSE) {
      next_token(parser);
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_transitionStatement(Parser* parser)
{
  Ast* transition;

  if (parser->token->klass == TK_TRANSITION) {
    next_token(parser);
    transition = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    transition->kind = AST_transitionStatement;
    transition->line_no = parser->token->line_no;
    transition->column_no = parser->token->column_no;
    transition->transitionStatement.stmt = parse_stateExpression(parser);
    return transition;
  } else error("%s:%d:%d: error: `transition` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_stateExpression(Parser* parser)
{
  Ast* state_expr;

  if (token_is_name(parser->token) || parser->token->klass == TK_SELECT) {
    state_expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    state_expr->kind = AST_stateExpression;
    state_expr->line_no = parser->token->line_no;
    state_expr->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      state_expr->stateExpression.expr = parse_name(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                  parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return state_expr;
    } else if (parser->token->klass == TK_SELECT) {
      state_expr->stateExpression.expr = parse_selectExpression(parser);
      return state_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: state expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_selectExpression(Parser* parser)
{
  Ast* select_expr;

  if (parser->token->klass == TK_SELECT) {
    next_token(parser);
    select_expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    select_expr->kind = AST_selectExpression;
    select_expr->line_no = parser->token->line_no;
    select_expr->column_no = parser->token->column_no;
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      select_expr->selectExpression.expr_list = parse_expressionList(parser);
      if (parser->token->klass == TK_PARENTH_CLOSE) {
        next_token(parser);
        if (parser->token->klass == TK_BRACE_OPEN) {
          next_token(parser);
          select_expr->selectExpression.case_list = parse_selectCaseList(parser);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return select_expr;
  } else error("%s:%d:%d: error: `select` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_selectCaseList(Parser* parser)
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {0};

  cases = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  cases->kind = AST_selectCaseList;
  cases->line_no = parser->token->line_no;
  cases->column_no = parser->token->column_no;
  if (token_is_selectCase(parser->token)) {
    ast = parse_selectCase(parser);
    ast_tree_append_node(&cases->tree, &tree_ctor, &ast->tree);
    while (token_is_selectCase(parser->token)) {
      ast = parse_selectCase(parser);
      ast_tree_append_node(&cases->tree, &tree_ctor, &ast->tree);
    }
  }
  return cases;
}

static Ast* parse_selectCase(Parser* parser)
{
  Ast* select_case;

  if (token_is_keysetExpression(parser->token)) {
    select_case = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    select_case->kind = AST_selectCase;
    select_case->line_no = parser->token->line_no;
    select_case->column_no = parser->token->column_no;
    select_case->selectCase.keyset_expr = parse_keysetExpression(parser);
    if (parser->token->klass == TK_COLON) {
      next_token(parser);
      if (token_is_name(parser->token)) {
        select_case->selectCase.name = parse_name(parser);
        if (parser->token->klass == TK_SEMICOLON) {
          next_token(parser);
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return select_case;
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_keysetExpression(Parser *parser)
{
  Ast* keyset_expr;

  if (parser->token->klass == TK_PARENTH_OPEN || token_is_simpleKeysetExpression(parser->token)) {
    keyset_expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    keyset_expr->kind = AST_keysetExpression;
    keyset_expr->line_no = parser->token->line_no;
    keyset_expr->column_no = parser->token->column_no;
    if (parser->token->klass == TK_PARENTH_OPEN) {
      keyset_expr->keysetExpression.expr = parse_tupleKeysetExpression(parser);
      return keyset_expr;
    } else if (token_is_simpleKeysetExpression(parser->token)) {
      keyset_expr->keysetExpression.expr = parse_simpleKeysetExpression(parser);
      return keyset_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_tupleKeysetExpression(Parser* parser)
{
  Ast* tuple_keyset;

  if (parser->token->klass == TK_PARENTH_OPEN) {
    next_token(parser);
    tuple_keyset = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    tuple_keyset->kind = AST_tupleKeysetExpression;
    tuple_keyset->line_no = parser->token->line_no;
    tuple_keyset->column_no = parser->token->column_no;
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList(parser);
    if (parser->token->klass == TK_PARENTH_CLOSE) {
      next_token(parser);
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return tuple_keyset;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_simpleExpressionList(Parser* parser)
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {0};

  exprs = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  exprs->kind = AST_simpleExpressionList;
  exprs->line_no = parser->token->line_no;
  exprs->column_no = parser->token->column_no;
  if (token_is_expression(parser->token)) {
    ast = parse_simpleKeysetExpression(parser);
    ast_tree_append_node(&exprs->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_simpleKeysetExpression(parser);
      ast_tree_append_node(&exprs->tree, &tree_ctor, &ast->tree);
    }
  }
  return exprs;
}

static Ast* parse_simpleKeysetExpression(Parser* parser)
{
  Ast* simple_keyset, *default_keyset, *dontcare_keyset;

  if (token_is_simpleKeysetExpression(parser->token)) {
    simple_keyset = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    simple_keyset->kind = AST_simpleKeysetExpression;
    simple_keyset->line_no = parser->token->line_no;
    simple_keyset->column_no = parser->token->column_no;
    if (token_is_expression(parser->token)) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(parser, 1);
      return simple_keyset;
    } else if (parser->token->klass == TK_DEFAULT) {
      next_token(parser);
      default_keyset = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      default_keyset->kind = AST_default;
      default_keyset->line_no = parser->token->line_no;
      default_keyset->column_no = parser->token->column_no;
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (parser->token->klass == TK_DONTCARE) {
      next_token(parser);
      dontcare_keyset = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      dontcare_keyset->kind = AST_dontcare;
      dontcare_keyset->line_no = parser->token->line_no;
      dontcare_keyset->column_no = parser->token->column_no;
      simple_keyset->simpleKeysetExpression.expr = dontcare_keyset;
      return simple_keyset;
    }
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** CONTROL **/

static Ast* parse_controlDeclaration(Parser* parser, Ast* control_proto)
{
  Ast* control_decl;

  if (parser->token->klass == TK_PARENTH_OPEN || parser->token->klass == TK_BRACE_OPEN) {
    control_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    control_decl->kind = AST_controlDeclaration;
    control_decl->line_no = parser->token->line_no;
    control_decl->column_no = parser->token->column_no;
    control_decl->controlDeclaration.proto = control_proto;
    control_decl->controlDeclaration.ctor_params = parse_constructorParameters(parser);
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      control_decl->controlDeclaration.local_decls = parse_controlLocalDeclarations(parser);
      if (parser->token->klass == TK_APPLY) {
        next_token(parser);
        control_decl->controlDeclaration.apply_stmt = parse_blockStatement(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return control_decl;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_controlTypeDeclaration(Parser* parser)
{
  Ast* control_proto, *name, *method_protos;

  if (parser->token->klass == TK_CONTROL) {
    next_token(parser);
    control_proto = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    control_proto->kind = AST_controlTypeDeclaration;
    control_proto->line_no = parser->token->line_no;
    control_proto->column_no = parser->token->column_no;
    method_protos = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    method_protos->kind = AST_methodPrototypes;
    method_protos->line_no = control_proto->line_no;
    method_protos->column_no = control_proto->column_no;
    control_proto->controlTypeDeclaration.method_protos = method_protos;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      control_proto->controlTypeDeclaration.name = name;
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        control_proto->controlTypeDeclaration.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return control_proto;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_controlLocalDeclaration(Parser* parser)
{
  Ast* local_decl, *type_ref;

  if (token_is_controlLocalDeclaration(parser->token)) {
    local_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    local_decl->kind = AST_controlLocalDeclaration;
    local_decl->line_no = parser->token->line_no;
    local_decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_CONST) {
      local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(parser, 0);
      return local_decl;
    } else if (parser->token->klass == TK_ACTION) {
      local_decl->controlLocalDeclaration.decl = parse_actionDeclaration(parser);
      return local_decl;
    } else if (parser->token->klass == TK_TABLE) {
      local_decl->controlLocalDeclaration.decl = parse_tableDeclaration(parser);
      return local_decl;
    } else if (token_is_typeRef(parser->token)) {
      type_ref = parse_typeRef(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(parser, type_ref);
        return local_decl;
      } else if (token_is_name(parser->token)) {
        local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(parser, type_ref);
        return local_decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_controlLocalDeclarations(Parser* parser)
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {0};

  decls = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  decls->kind = AST_controlLocalDeclarations;
  decls->line_no = parser->token->line_no;
  decls->column_no = parser->token->column_no;
  if (token_is_controlLocalDeclaration(parser->token)) {
    ast = parse_controlLocalDeclaration(parser);
    ast_tree_append_node(&decls->tree, &tree_ctor, &ast->tree);
    while (token_is_controlLocalDeclaration(parser->token)) {
      ast = parse_controlLocalDeclaration(parser);
      ast_tree_append_node(&decls->tree, &tree_ctor, &ast->tree);
    }
  }
  return decls;
}

/** EXTERN **/

static Ast* parse_externDeclaration(Parser* parser)
{
  Ast* extern_decl, *extern_type;
  bool is_function_type = 0;
  Ast* name;

  if (parser->token->klass == TK_EXTERN) {
    next_token(parser);
    extern_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    extern_decl->kind = AST_externDeclaration;
    extern_decl->line_no = parser->token->line_no;
    extern_decl->column_no = parser->token->column_no;

    if (token_is_typeOrVoid(parser->token) && token_is_nonTypeName(parser->token)) {
      is_function_type = token_is_typeOrVoid(parser->token) && token_is_name(peek_token(parser));
    } else if (token_is_typeOrVoid(parser->token)) {
      is_function_type = 1;
    } else if (token_is_nonTypeName(parser->token)) {
      is_function_type = 0;
    } else error("%s:%d:%d: error: extern declaration was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);

    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(parser, 0);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return extern_decl;
    } else {
      extern_type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      extern_type->kind = AST_externTypeDeclaration;
      extern_type->line_no = parser->token->line_no;
      extern_type->column_no = parser->token->column_no;
      extern_type->externTypeDeclaration.name = parse_nonTypeName(parser);
      name = extern_type->externTypeDeclaration.name;
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      if (parser->token->klass == TK_BRACE_OPEN) {
        next_token(parser);
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      extern_decl->externDeclaration.decl = extern_type;
      return extern_decl;
    }
  } else error("%s:%d:%d: error: `extern` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_methodPrototypes(Parser* parser)
{
  Ast* protos, *ast;
  AstTreeCtor tree_ctor = {0};

  protos = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  protos->kind = AST_methodPrototypes;
  protos->line_no = parser->token->line_no;
  protos->column_no = parser->token->column_no;
  if (token_is_methodPrototype(parser->token)) {
    ast = parse_methodPrototype(parser);
    ast_tree_append_node(&protos->tree, &tree_ctor, &ast->tree);
    while (token_is_methodPrototype(parser->token)) {
      ast = parse_methodPrototype(parser);
      ast_tree_append_node(&protos->tree, &tree_ctor, &ast->tree);
    }
  }
  return protos;
}

static Ast* parse_functionPrototype(Parser* parser, Ast* return_type)
{
  Ast* func_proto, *type_ref;
  Ast* name;

  if (token_is_typeOrVoid(parser->token) || return_type) {
    func_proto = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    func_proto->kind = AST_functionPrototype;
    func_proto->line_no = parser->token->line_no;
    func_proto->column_no = parser->token->column_no;
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      return_type = parse_typeOrVoid(parser);
      if (return_type->kind == AST_name) {
        name = return_type;
        parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
        type_ref = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        type_ref->kind = AST_typeRef;
        type_ref->line_no = parser->token->line_no;
        type_ref->column_no = parser->token->column_no;
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token_is_name(parser->token)) {
      func_proto->functionPrototype.name = parse_name(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        func_proto->functionPrototype.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: function name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return func_proto;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_methodPrototype(Parser* parser)
{
  Ast* func_proto;

  if (token_is_methodPrototype(parser->token)) {
    if (parser->token->klass == TK_TYPE_IDENTIFIER && peek_token(parser)->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      func_proto = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      func_proto->kind = AST_functionPrototype;
      func_proto->line_no = parser->token->line_no;
      func_proto->column_no = parser->token->column_no;
      func_proto->functionPrototype.name = parse_name(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        func_proto->functionPrototype.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return func_proto;
    } else if (token_is_typeOrVoid(parser->token)) {
      func_proto = parse_functionPrototype(parser, 0);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return func_proto;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** TYPES **/

static Ast* parse_typeRef(Parser* parser)
{
  Ast* type_ref;

  if (token_is_typeRef(parser->token)) {
    type_ref = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = parser->token->line_no;
    type_ref->column_no = parser->token->column_no;
    if (token_is_baseType(parser->token)) {
      type_ref->typeRef.type = parse_baseType(parser);
      return type_ref;
    } else if (token_is_typeName(parser->token)) {
      type_ref->typeRef.type = parse_namedType(parser);
      return type_ref;
    } else if (parser->token->klass == TK_TUPLE) {
      type_ref->typeRef.type = parse_tupleType(parser);
      return type_ref;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_namedType(Parser* parser)
{
  Ast* named_type;

  if (token_is_typeName(parser->token)) {
    named_type = parse_typeName(parser);
    if (parser->token->klass == TK_BRACKET_OPEN) {
      named_type = parse_headerStackType(parser, named_type);
      return named_type;
    }
    return named_type;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_typeName(Parser* parser)
{
  Ast* type_name;

  if (parser->token->klass == TK_TYPE_IDENTIFIER) {
    type_name = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = parser->token->line_no;
    type_name->column_no = parser->token->column_no;
    type_name->name.strname = parser->token->lexeme;
    next_token(parser);
    return type_name;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_tupleType(Parser* parser)
{
  Ast* tuple;

  if (parser->token->klass == TK_TUPLE) {
    tuple = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    tuple->kind = AST_tupleType;
    tuple->line_no = parser->token->line_no;
    tuple->column_no = parser->token->column_no;
    next_token(parser);
    if (parser->token->klass == TK_ANGLE_OPEN) {
      next_token(parser);
      tuple->tupleType.type_args = parse_typeArgumentList(parser);
      if (parser->token->klass == TK_ANGLE_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return tuple;
  } else error("%s:%d:%d: error: `tuple` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_headerStackType(Parser* parser, Ast* named_type)
{
  Ast* type_ref, *type;

  if (parser->token->klass == TK_BRACKET_OPEN) {
    next_token(parser);
    type_ref = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type->kind = AST_headerStackType;
    type->line_no = named_type->line_no;
    type->column_no = named_type->column_no;
    type->headerStackType.type = type_ref;
    if (token_is_expression(parser->token)) {
      type->headerStackType.stack_expr = parse_expression(parser, 1);
      if (parser->token->klass == TK_BRACKET_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: expression expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return type;
  } else error("%s:%d:%d: error: `[` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_baseType(Parser* parser)
{
  Ast* type_name, *type;

  if (token_is_baseType(parser->token)) {
    type_name = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = parser->token->line_no;
    type_name->column_no = parser->token->column_no;
    if (parser->token->klass == TK_BOOL) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeBoolean;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeBoolean.name = type_name;
      next_token(parser);
      return type;
    } else if (parser->token->klass == TK_INT) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeInteger;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeInteger.name = type_name;
      next_token(parser);
      if (parser->token->klass == TK_ANGLE_OPEN) {
        next_token(parser);
        type->baseTypeInteger.size = parse_integerTypeSize(parser);
        if (parser->token->klass == TK_ANGLE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      }
      return type;
    } else if (parser->token->klass == TK_BIT) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeBit;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeBit.name = type_name;
      next_token(parser);
      if (parser->token->klass == TK_ANGLE_OPEN) {
        next_token(parser);
        type->baseTypeBit.size = parse_integerTypeSize(parser);
        if (parser->token->klass == TK_ANGLE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      }
      return type;
    } else if (parser->token->klass == TK_VARBIT) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeVarbit;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeVarbit.name = type_name;
      next_token(parser);
      if (parser->token->klass == TK_ANGLE_OPEN) {
        next_token(parser);
        type->baseTypeVarbit.size = parse_integerTypeSize(parser);
        if (parser->token->klass == TK_ANGLE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: '<' was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return type;
    } else if (parser->token->klass == TK_STRING) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeString;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeString.name = type_name;
      next_token(parser);
      return type;
    } else if (parser->token->klass == TK_VOID) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeVoid;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeVoid.name = type_name;
      next_token(parser);
      return type;
    } else if (parser->token->klass == TK_ERROR) {
      type = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type->kind = AST_baseTypeError;
      type->line_no = parser->token->line_no;
      type->column_no = parser->token->column_no;
      type_name->name.strname = parser->token->lexeme;
      type->baseTypeError.name = type_name;
      next_token(parser);
      return type;
    } else assert(0);
  } else error("%s:%d:%d: error: base type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_integerTypeSize(Parser* parser)
{
  Ast* type_size;

  type_size = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  type_size->kind = AST_integerTypeSize;
  type_size->line_no = parser->token->line_no;
  type_size->column_no = parser->token->column_no;
  if (parser->token->klass == TK_INTEGER_LITERAL) {
    type_size->integerTypeSize.size = parse_integer(parser);
  } else if (parser->token->klass == TK_PARENTH_OPEN) {
#if 0
    type_size->size = parse_expression(parser, 1);
#endif
    error("%s:%d:%d: error: integer was expected, got `%s`.",
          parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  return type_size;
}

static Ast* parse_typeOrVoid(Parser* parser)
{
  Ast* type, *name;

  if (token_is_typeOrVoid(parser->token)) {
    if (token_is_typeRef(parser->token)) {
      type = parse_typeRef(parser);
      return type;
    } else if (parser->token->klass == TK_VOID) {
      return parse_baseType(parser);
    } else if (parser->token->klass == TK_IDENTIFIER) {
      name = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = parser->token->line_no;
      name->column_no = parser->token->column_no;
      name->name.strname = parser->token->lexeme;
      next_token(parser);
      return name;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_realTypeArg(Parser* parser)
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_realTypeArg(parser->token)) {
    type_arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_arg->kind = AST_realTypeArg;
    type_arg->line_no = parser->token->line_no;
    type_arg->column_no = parser->token->column_no;
    if (parser->token->klass == TK_DONTCARE) {
      next_token(parser);
      dontcare_arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = parser->token->line_no;
      dontcare_arg->column_no = parser->token->column_no;
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(parser->token)) {
      type_arg->realTypeArg.arg = parse_typeRef(parser);
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_typeArg(Parser* parser)
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_typeArg(parser->token)) {
    type_arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_arg->kind = AST_typeArg;
    type_arg->line_no = parser->token->line_no;
    type_arg->column_no = parser->token->column_no;
    if (parser->token->klass == TK_DONTCARE) {
      next_token(parser);
      dontcare_arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = parser->token->line_no;
      dontcare_arg->column_no = parser->token->column_no;
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(parser->token)) {
      type_arg->typeArg.arg = parse_typeRef(parser);
      return type_arg;
    } else if (token_is_nonTypeName(parser->token)) {
      type_arg->typeArg.arg = parse_nonTypeName(parser);
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_typeArgumentList(Parser* parser)
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {0};

  args = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  args->kind = AST_typeArgumentList;
  args->line_no = parser->token->line_no;
  args->column_no = parser->token->column_no;
  if (token_is_typeArg(parser->token)) {
    ast = parse_typeArg(parser);
    ast_tree_append_node(&args->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_typeArg(parser);
      ast_tree_append_node(&args->tree, &tree_ctor, &ast->tree);
    }
  }
  return args;
}

static Ast* parse_typeDeclaration(Parser* parser)
{
  Ast* type_decl;

  if (token_is_typeDeclaration(parser->token)) {
    type_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_decl->kind = AST_typeDeclaration;
    type_decl->line_no = parser->token->line_no;
    type_decl->column_no = parser->token->column_no;
    if (token_is_derivedTypeDeclaration(parser->token)) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_TYPEDEF) {
      type_decl->typeDeclaration.decl = parse_typedefDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_PARSER) {
      type_decl->typeDeclaration.decl = parse_parserTypeDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_CONTROL) {
      type_decl->typeDeclaration.decl = parse_controlTypeDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_PACKAGE) {
      type_decl->typeDeclaration.decl = parse_packageTypeDeclaration(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: type declaration was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme); 
  assert(0);
  return 0;
}

static Ast* parse_derivedTypeDeclaration(Parser* parser)
{
  Ast* type_decl;

  if (token_is_derivedTypeDeclaration(parser->token)) {
    type_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    type_decl->kind = AST_derivedTypeDeclaration;
    type_decl->line_no = parser->token->line_no;
    type_decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_HEADER) {
      type_decl->derivedTypeDeclaration.decl = parse_headerTypeDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_UNION) {
      type_decl->derivedTypeDeclaration.decl = parse_headerUnionDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_STRUCT) {
      type_decl->derivedTypeDeclaration.decl = parse_structTypeDeclaration(parser);
      return type_decl;
    } else if (parser->token->klass == TK_ENUM) {
      type_decl->derivedTypeDeclaration.decl = parse_enumDeclaration(parser);
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: structure declaration was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_headerTypeDeclaration(Parser* parser)
{
  Ast* header_decl;
  Ast* name;

  if (parser->token->klass == TK_HEADER) {
    next_token(parser);
    header_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    header_decl->kind = AST_headerTypeDeclaration;
    header_decl->line_no = parser->token->line_no;
    header_decl->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      header_decl->headerTypeDeclaration.name = name;
      if (parser->token->klass == TK_BRACE_OPEN) {
        next_token(parser);
        header_decl->headerTypeDeclaration.fields = parse_structFieldList(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return header_decl;
  } else error("%s:%d:%d: error: `header` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_headerUnionDeclaration(Parser* parser)
{
  Ast* union_decl;
  Ast* name;

  if (parser->token->klass == TK_UNION) {
    next_token(parser);
    union_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    union_decl->kind = AST_headerUnionDeclaration;
    union_decl->line_no = parser->token->line_no;
    union_decl->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      union_decl->headerUnionDeclaration.name = name;
      if (parser->token->klass == TK_BRACE_OPEN) {
        next_token(parser);
        union_decl->headerUnionDeclaration.fields = parse_structFieldList(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return union_decl;
  } else error("%s:%d:%d: error: `header_union` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_structTypeDeclaration(Parser* parser)
{
  Ast* struct_decl;
  Ast* name;

  if (parser->token->klass == TK_STRUCT) {
    next_token(parser);
    struct_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    struct_decl->kind = AST_structTypeDeclaration;
    struct_decl->line_no = parser->token->line_no;
    struct_decl->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      struct_decl->structTypeDeclaration.name = name;
      if (parser->token->klass == TK_BRACE_OPEN) {
        next_token(parser);
        struct_decl->structTypeDeclaration.fields = parse_structFieldList(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return struct_decl;
  } else error("%s:%d:%d: error: `struct` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_structFieldList(Parser* parser)
{
  Ast* fields, *ast;
  AstTreeCtor tree_ctor = {0};

  fields = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  fields->kind = AST_structFieldList;
  fields->line_no = parser->token->line_no;
  fields->column_no = parser->token->column_no;
  if (token_is_structField(parser->token)) {
    ast = parse_structField(parser);
    ast_tree_append_node(&fields->tree, &tree_ctor, &ast->tree);
    while (token_is_structField(parser->token)) {
      ast = parse_structField(parser);
      ast_tree_append_node(&fields->tree, &tree_ctor, &ast->tree);
    }
  }
  return fields;
}

static Ast* parse_structField(Parser* parser)
{
  if (token_is_structField(parser->token)) {
    Ast* field = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    field->kind = AST_structField;
    field->line_no = parser->token->line_no;
    field->column_no = parser->token->column_no;
    field->structField.type = parse_typeRef(parser);
    if (token_is_name(parser->token)) {
      field->structField.name = parse_name(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return field;
  } else error("%s:%d:%d: error: struct field was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_enumDeclaration(Parser* parser)
{
  Ast* enum_decl;
  Ast* name;

  if (parser->token->klass == TK_ENUM) {
    next_token(parser);
    enum_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    enum_decl->kind = AST_enumDeclaration;
    enum_decl->line_no = parser->token->line_no;
    enum_decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_BIT) {
      next_token(parser);
      if (parser->token->klass == TK_ANGLE_OPEN) {
        next_token(parser);
        if (parser->token->klass == TK_INTEGER_LITERAL) {
          enum_decl->enumDeclaration.type_size = parse_integer(parser);
          if (parser->token->klass == TK_ANGLE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: an integer was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    }
    if (token_is_name(parser->token)) {
      name = parse_name(parser);
      parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
      enum_decl->enumDeclaration.name = name;
      if (parser->token->klass == TK_BRACE_OPEN) {
        next_token(parser);
        if (token_is_specifiedIdentifier(parser->token)) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList(parser);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return enum_decl;
  } else error("%s:%d:%d: error: `enum` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_errorDeclaration(Parser* parser)
{
  Ast* error_decl;

  if (parser->token->klass == TK_ERROR) {
    next_token(parser);
    error_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    error_decl->kind = AST_errorDeclaration;
    error_decl->line_no = parser->token->line_no;
    error_decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      if (token_is_name(parser->token)) {
        if (token_is_name(parser->token)) {
          error_decl->errorDeclaration.fields = parse_identifierList(parser);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return error_decl;
  } else error("%s:%d:%d: error: `error` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_matchKindDeclaration(Parser* parser)
{
  Ast* match_decl;

  if (parser->token->klass == TK_MATCH_KIND) {
    next_token(parser);
    match_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    match_decl->kind = AST_matchKindDeclaration;
    match_decl->line_no = parser->token->line_no;
    match_decl->column_no = parser->token->column_no;
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      if (token_is_name(parser->token)) {
        match_decl->matchKindDeclaration.fields = parse_identifierList(parser);
        if (parser->token->klass == TK_BRACE_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return match_decl;
  } else error("%s:%d:%d: error: `match_kind` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_identifierList(Parser* parser)
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {0};

  ids = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  ids->kind = AST_identifierList;
  ids->line_no = parser->token->line_no;
  ids->column_no = parser->token->column_no;
  if (token_is_name(parser->token)) {
    ast = parse_name(parser);
    ast_tree_append_node(&ids->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_name(parser);
      ast_tree_append_node(&ids->tree, &tree_ctor, &ast->tree);
    }
  }
  return ids;
}

static Ast* parse_specifiedIdentifierList(Parser* parser)
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {0};

  ids = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  ids->kind = AST_specifiedIdentifierList;
  ids->line_no = parser->token->line_no;
  ids->column_no = parser->token->column_no;
  if (token_is_specifiedIdentifier(parser->token)) {
    ast = parse_specifiedIdentifier(parser);
    ast_tree_append_node(&ids->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_specifiedIdentifier(parser);
      ast_tree_append_node(&ids->tree, &tree_ctor, &ast->tree);
    }
  }
  return ids;
}

static Ast* parse_specifiedIdentifier(Parser* parser)
{
  Ast* id;

  if (token_is_specifiedIdentifier(parser->token)) {
    id = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    id->kind = AST_specifiedIdentifier;
    id->line_no = parser->token->line_no;
    id->column_no = parser->token->column_no;
    id->specifiedIdentifier.name = parse_name(parser);
    if (parser->token->klass == TK_EQUAL) {
      next_token(parser);
      if (token_is_expression(parser->token)) {
        id->specifiedIdentifier.init_expr = parse_expression(parser, 1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    }
    return id;
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_typedefDeclaration(Parser* parser)
{
  Ast* type_decl;
  Ast* name;

  if (parser->token->klass == TK_TYPEDEF) {
    next_token(parser);
    if (token_is_typeRef(parser->token) || token_is_derivedTypeDeclaration(parser->token)) {
      type_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      type_decl->kind = AST_typedefDeclaration;
      type_decl->line_no = parser->token->line_no;
      type_decl->column_no = parser->token->column_no;
      if (token_is_typeRef(parser->token)) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef(parser);
      } else if (token_is_derivedTypeDeclaration(parser->token)) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration(parser);
      } else assert(0);
      if (token_is_name(parser->token)) {
        name = parse_name(parser);
        parser->current_scope->bind(parser->storage, name->name.strname, NameSpace::TYPE);
        type_decl->typedefDeclaration.name = name;
        if (parser->token->klass == TK_SEMICOLON) {
          next_token(parser);
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return type_decl;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  } else error("%s:%d:%d: error: type definition was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** STATEMENTS **/

static Ast* parse_assignmentOrMethodCallStatement(Parser* parser)
{
  Ast* lvalue, *stmt; 

  if (token_is_lvalue(parser->token)) {
    lvalue = parse_lvalue(parser);
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      stmt->kind = AST_functionCall;
      stmt->line_no = parser->token->line_no;
      stmt->column_no = parser->token->column_no;
      stmt->functionCall.lhs_expr = lvalue;
      stmt->functionCall.args = parse_argumentList(parser);
      if (parser->token->klass == TK_PARENTH_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return stmt;
    } else if (parser->token->klass == TK_EQUAL) {
      next_token(parser);
      stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      stmt->kind = AST_assignmentStatement;
      stmt->line_no = parser->token->line_no;
      stmt->column_no = parser->token->column_no;
      stmt->assignmentStatement.lhs_expr = lvalue;
      stmt->assignmentStatement.rhs_expr = parse_expression(parser, 1);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return stmt;
    } else error("%s:%d:%d: error: assignment or function call was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_returnStatement(Parser* parser)
{
  Ast* return_stmt;

  if (parser->token->klass == TK_RETURN) {
    next_token(parser);
    return_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    return_stmt->kind = AST_returnStatement;
    return_stmt->line_no = parser->token->line_no;
    return_stmt->column_no = parser->token->column_no;
    if (token_is_expression(parser->token))
      return_stmt->returnStatement.expr = parse_expression(parser, 1);
    if (parser->token->klass == TK_SEMICOLON) {
      next_token(parser);
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return return_stmt;
  } else error("%s:%d:%d: error: `return` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_exitStatement(Parser* parser)
{
  Ast* exit_stmt;

  if (parser->token->klass == TK_EXIT) {
    next_token(parser);
    exit_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    exit_stmt->kind = AST_exitStatement;
    exit_stmt->line_no = parser->token->line_no;
    exit_stmt->column_no = parser->token->column_no;
    if (parser->token->klass == TK_SEMICOLON) {
      next_token(parser);
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return exit_stmt;
  } else error("%s:%d:%d: error: `exit` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_conditionalStatement(Parser* parser)
{
  Ast* if_stmt;

  if (parser->token->klass == TK_IF) {
    next_token(parser);
    if_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    if_stmt->kind = AST_conditionalStatement;
    if_stmt->line_no = parser->token->line_no;
    if_stmt->column_no = parser->token->column_no;
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      if (token_is_expression(parser->token)) {
        if_stmt->conditionalStatement.cond_expr = parse_expression(parser, 1);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
          if (token_is_statement(parser->token)) {
            if_stmt->conditionalStatement.stmt = parse_statement(parser, 0);
            if (parser->token->klass == TK_ELSE) {
              next_token(parser);
              if (token_is_statement(parser->token)) {
                if_stmt->conditionalStatement.else_stmt = parse_statement(parser, 0);
              } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                           parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
            }
          } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return if_stmt;
  } else error("%s:%d:%d: error: `if` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_directApplication(Parser* parser, Ast* type_name)
{
  Ast* apply_stmt;

  if (token_is_typeName(parser->token) || type_name) {
    apply_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    apply_stmt->kind = AST_directApplication;
    apply_stmt->line_no = parser->token->line_no;
    apply_stmt->column_no = parser->token->column_no;
    apply_stmt->directApplication.name = type_name ? type_name : parse_typeName(parser);
    if (parser->token->klass == TK_DOT) {
      next_token(parser);
      if (parser->token->klass == TK_APPLY) {
        next_token(parser);
        if (parser->token->klass == TK_PARENTH_OPEN) {
          next_token(parser);
          apply_stmt->directApplication.args = parse_argumentList(parser);
          if (parser->token->klass == TK_PARENTH_CLOSE) {
            next_token(parser);
            if (parser->token->klass == TK_SEMICOLON) {
              next_token(parser);
            } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                         parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
          } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `.` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return apply_stmt;
  } else error("%s:%d:%d: error: type name was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_statement(Parser* parser, Ast* type_name)
{
  Ast* stmt, *empty_stmt;

  if (token_is_statement(parser->token)) {
    stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    stmt->kind = AST_statement;
    stmt->line_no = parser->token->line_no;
    stmt->column_no = parser->token->column_no;
    if (token_is_typeName(parser->token) || type_name) {
      stmt->statement.stmt = parse_directApplication(parser, type_name);
      return stmt;
    } else if (token_is_assignmentOrMethodCallStatement(parser->token)) {
      stmt->statement.stmt = parse_assignmentOrMethodCallStatement(parser);
      return stmt;
    } else if (parser->token->klass == TK_IF) {
      stmt->statement.stmt = parse_conditionalStatement(parser);
      return stmt;
    } else if (parser->token->klass == TK_SEMICOLON) {
      empty_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      empty_stmt->kind = AST_emptyStatement;
      empty_stmt->line_no = parser->token->line_no;
      empty_stmt->column_no = parser->token->column_no;
      stmt->statement.stmt = empty_stmt;
      next_token(parser);
      return stmt;
    } else if (parser->token->klass == TK_BRACE_OPEN) {
      stmt->statement.stmt = parse_blockStatement(parser);
      return stmt;
    } else if (parser->token->klass == TK_EXIT) {
      stmt->statement.stmt = parse_exitStatement(parser);
      return stmt;
    } else if (parser->token->klass == TK_RETURN) {
      stmt->statement.stmt = parse_returnStatement(parser);
      return stmt;
    } else if (parser->token->klass == TK_SWITCH) {
      stmt->statement.stmt = parse_switchStatement(parser);
      return stmt;
    }
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_blockStatement(Parser* parser)
{
  Ast* block_stmt;

  if (parser->token->klass == TK_BRACE_OPEN) {
    next_token(parser);
    block_stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    block_stmt->kind = AST_blockStatement;
    block_stmt->line_no = parser->token->line_no;
    block_stmt->column_no = parser->token->column_no;
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList(parser);
    if (parser->token->klass == TK_BRACE_CLOSE) {
      next_token(parser);
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return block_stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_statementOrDeclList(Parser* parser)
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {0};

  stmts = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  stmts->kind = AST_statementOrDeclList;
  stmts->line_no = parser->token->line_no;
  stmts->column_no = parser->token->column_no;
  if (token_is_statementOrDeclaration(parser->token)) {
    ast = parse_statementOrDeclaration(parser);
    ast_tree_append_node(&stmts->tree, &tree_ctor, &ast->tree);
    while (token_is_statementOrDeclaration(parser->token)) {
      ast = parse_statementOrDeclaration(parser);
      ast_tree_append_node(&stmts->tree, &tree_ctor, &ast->tree);
    }
  }
  return stmts;
}

static Ast* parse_switchStatement(Parser* parser)
{
  Ast* stmt;

  if (parser->token->klass == TK_SWITCH) {
    next_token(parser);
    stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    stmt->kind = AST_switchStatement;
    stmt->line_no = parser->token->line_no;
    stmt->column_no = parser->token->column_no;
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      stmt->switchStatement.expr = parse_expression(parser, 1);
      if (parser->token->klass == TK_PARENTH_CLOSE) {
        next_token(parser);
        if (parser->token->klass == TK_BRACE_OPEN) {
          next_token(parser);
          stmt->switchStatement.switch_cases = parse_switchCases(parser);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `switch` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_switchCases(Parser* parser)
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {0};

  cases = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  cases->kind = AST_switchCases;
  cases->line_no = parser->token->line_no;
  cases->column_no = parser->token->column_no;
  if (token_is_switchLabel(parser->token)) {
    ast = parse_switchCase(parser);
    ast_tree_append_node(&cases->tree, &tree_ctor, &ast->tree);
    while (token_is_switchLabel(parser->token)) {
      ast = parse_switchCase(parser);
      ast_tree_append_node(&cases->tree, &tree_ctor, &ast->tree);
    }
  }
  return cases;
}

static Ast* parse_switchCase(Parser* parser)
{
  Ast* switch_case;

  if (token_is_switchLabel(parser->token)) {
    switch_case = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    switch_case->kind = AST_switchCase;
    switch_case->line_no = parser->token->line_no;
    switch_case->column_no = parser->token->column_no;
    switch_case->switchCase.label = parse_switchLabel(parser);
    if (parser->token->klass == TK_COLON) {
      next_token(parser);
      if (parser->token->klass == TK_BRACE_OPEN) {
        switch_case->switchCase.stmt = parse_blockStatement(parser);
      }
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return switch_case;
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_switchLabel(Parser* parser)
{
  Ast* switch_label, *default_label;

  if (token_is_switchLabel(parser->token)) {
    switch_label = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    switch_label->kind = AST_switchLabel;
    switch_label->line_no = parser->token->line_no;
    switch_label->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      switch_label->switchLabel.label = parse_name(parser);
      return switch_label;
    } else if (parser->token->klass == TK_DEFAULT) {
      next_token(parser);
      default_label = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      default_label->kind = AST_default;
      default_label->line_no = parser->token->line_no;
      default_label->column_no = parser->token->column_no;
      switch_label->switchLabel.label = default_label;
      return switch_label;
    } else assert(0);
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_statementOrDeclaration(Parser* parser)
{
  Ast* stmt, *type_ref;

  if (token_is_statementOrDeclaration(parser->token)) {
    stmt = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    stmt->kind = AST_statementOrDeclaration;
    stmt->line_no = parser->token->line_no;
    stmt->column_no = parser->token->column_no;
    if (token_is_typeRef(parser->token)) {
      type_ref = parse_typeRef(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        stmt->statementOrDeclaration.stmt = parse_instantiation(parser, type_ref);
        return stmt;
      } else if (token_is_name(parser->token)) {
        stmt->statementOrDeclaration.stmt = parse_variableDeclaration(parser, type_ref);
        return stmt;
      } else {
        stmt->statementOrDeclaration.stmt = parse_statement(parser, type_ref);
        return stmt;
      }
    } else if (token_is_statement(parser->token)) {
      stmt->statementOrDeclaration.stmt = parse_statement(parser, 0);
      return stmt;
    } else if (parser->token->klass == TK_CONST) {
      stmt->statementOrDeclaration.stmt = parse_variableDeclaration(parser, 0);
      return stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

/** TABLES **/ 

static Ast* parse_tableDeclaration(Parser* parser)
{
  Ast* table, *method_protos;

  if (parser->token->klass == TK_TABLE) {
    next_token(parser);
    table = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    table->kind = AST_tableDeclaration;
    table->line_no = parser->token->line_no;
    table->column_no = parser->token->column_no;
    table->tableDeclaration.name = parse_name(parser);
    method_protos = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    method_protos->kind = AST_methodPrototypes;
    method_protos->line_no = table->line_no;
    method_protos->column_no = table->column_no;
    table->tableDeclaration.method_protos = method_protos;
    if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      if (token_is_tableProperty(parser->token)) {
        table->tableDeclaration.prop_list = parse_tablePropertyList(parser);
      } else error("%s:%d:%d: error: table property was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      if (parser->token->klass == TK_BRACE_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return table;
  } else error("%s:%d:%d: error: `table` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_tablePropertyList(Parser* parser)
{
  Ast* props, *ast;
  AstTreeCtor tree_ctor = {0};

  props = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  props->kind = AST_tablePropertyList;
  props->line_no = parser->token->line_no;
  props->column_no = parser->token->column_no;
  if (token_is_tableProperty(parser->token)) {
    ast = parse_tableProperty(parser);
    ast_tree_append_node(&props->tree, &tree_ctor, &ast->tree);
    while (token_is_tableProperty(parser->token)) {
      ast = parse_tableProperty(parser);
      ast_tree_append_node(&props->tree, &tree_ctor, &ast->tree);
    }
  }
  return props;
}

static Ast* parse_tableProperty(Parser* parser)
{
#if 0
  bool is_const = 0;
#endif
  Ast* table_prop, *prop;

  if (token_is_tableProperty(parser->token)) {
#if 0
    if (parser->token->klass == TK_CONST) {
      next_token(parser);
      is_const = 1;
    }
#endif
    table_prop = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    table_prop->kind = AST_tableProperty;
    table_prop->line_no = parser->token->line_no;
    table_prop->column_no = parser->token->column_no;
    if (parser->token->klass == TK_KEY) {
      next_token(parser);
      prop = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      prop->kind = AST_keyProperty;
      prop->line_no = parser->token->line_no;
      prop->column_no = parser->token->column_no;
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        if (parser->token->klass == TK_BRACE_OPEN) {
          next_token(parser);
          prop->keyProperty.keyelem_list = parse_keyElementList(parser);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (parser->token->klass == TK_ACTIONS) {
      next_token(parser);
      prop = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      prop->kind = AST_actionsProperty;
      prop->line_no = parser->token->line_no;
      prop->column_no = parser->token->column_no;
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        if (parser->token->klass == TK_BRACE_OPEN) {
          next_token(parser);
          prop->actionsProperty.action_list = parse_actionList(parser);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
#if 0
    else if (parser->token->klass == TK_ENTRIES) {
      next_token(parser);
      prop = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      prop->kind = AST_entriesProperty;
      prop->line_no = parser->token->line_no;
      prop->column_no = parser->token->column_no;
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        if (parser->token->klass == TK_BRACE_OPEN) {
          next_token(parser);
          if (token_is_keysetExpression(parser->token)) {
            prop->entriesProperty.entries_list = parse_entriesList(parser);
          } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
          if (parser->token->klass == TK_BRACE_CLOSE) {
            next_token(parser);
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
    else if (token_is_nonTableKwName(parser->token)) {
      prop = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      prop->kind = AST_simpleProperty;
      prop->line_no = parser->token->line_no;
      prop->column_no = parser->token->column_no;
      prop->simpleProperty.is_const = is_const;
      prop->simpleProperty.name = parse_name(parser);
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        prop->simpleProperty.init_expr = parse_expression(parser, 1);
        if (parser->token->klass == TK_SEMICOLON) {
          next_token(parser);
        } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else assert(0);
#endif
    else error("%s:%d:%d: error: table property was expected, got `%s`.",
                parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  }
  else error("%s:%d:%d: error: table property was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_keyElementList(Parser* parser)
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {0};

  elems = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  elems->kind = AST_keyElementList;
  elems->line_no = parser->token->line_no;
  elems->column_no = parser->token->column_no;
  if (token_is_expression(parser->token)) {
    ast = parse_keyElement(parser);
    ast_tree_append_node(&elems->tree, &tree_ctor, &ast->tree);
    while (token_is_expression(parser->token)) {
      ast = parse_keyElement(parser);
      ast_tree_append_node(&elems->tree, &tree_ctor, &ast->tree);
    }
  }
  return elems;
}

static Ast* parse_keyElement(Parser* parser)
{
  Ast* key_elem;

  if (token_is_expression(parser->token)) {
    key_elem = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    key_elem->kind = AST_keyElement;
    key_elem->line_no = parser->token->line_no;
    key_elem->column_no = parser->token->column_no;
    key_elem->keyElement.expr = parse_expression(parser, 1);
    if (parser->token->klass == TK_COLON) {
      next_token(parser);
      key_elem->keyElement.match = parse_name(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return key_elem;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_actionList(Parser* parser)
{
  Ast* actions, *ast;
  AstTreeCtor tree_ctor = {0};

  actions = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  actions->kind = AST_actionList;
  actions->line_no = parser->token->line_no;
  actions->column_no = parser->token->column_no;
  if (token_is_actionRef(parser->token)) {
    ast = parse_actionRef(parser);
    ast_tree_append_node(&actions->tree, &tree_ctor, &ast->tree);
    if (parser->token->klass == TK_SEMICOLON) {
      next_token(parser);
    } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    while (token_is_actionRef(parser->token)) {
      ast = parse_actionRef(parser);
      ast_tree_append_node(&actions->tree, &tree_ctor, &ast->tree);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    }
  }
  return actions;
}

static Ast* parse_actionRef(Parser* parser)
{
  Ast* action_ref;

  if (token_is_nonTypeName(parser->token)) {
    action_ref = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    action_ref->kind = AST_actionRef;
    action_ref->line_no = parser->token->line_no;
    action_ref->column_no = parser->token->column_no;
    action_ref->actionRef.name = parse_nonTypeName(parser);
    if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      if (token_is_argument(parser->token)) {
        action_ref->actionRef.args = parse_argumentList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else if (parser->token->klass == TK_PARENTH_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    }
    return action_ref;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

#if 0
static Ast* parse_entriesList(Parser* parser)
{
  Ast* entries, *ast;
  AstTreeCtor tree_ctor = {0};

  entries = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  entries->kind = AST_entriesList;
  entries->line_no = parser->token->line_no;
  entries->column_no = parser->token->column_no;
  if (token_is_keysetExpression(parser->token)) {
    ast = parse_entry(parser);
    ast_tree_append_node(&entries->tree, &tree_ctor, &ast->tree);
    while (token_is_keysetExpression(parser->token)) {
      ast = parse_entry(parser);
      ast_tree_append_node(&entries->tree, &tree_ctor, &ast->tree);
    }
  }
  return entries;
}

static Ast* parse_entry(Parser* parser)
{
  Ast* entry;

  if (token_is_keysetExpression(parser->token)) {
    entry = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    entry->kind = AST_entry;
    entry->line_no = parser->token->line_no;
    entry->column_no = parser->token->column_no;
    entry->entry.keyset = parse_keysetExpression(parser);
    if (parser->token->klass == TK_COLON) {
      next_token(parser);
      entry->entry.action = parse_actionRef(parser);
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return entry;
  } else error("%s:%d:%d: error: keyset was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}
#endif

static Ast* parse_actionDeclaration(Parser* parser)
{
  Ast* action_decl;

  if (parser->token->klass == TK_ACTION) {
    next_token(parser);
    action_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    action_decl->kind = AST_actionDeclaration;
    action_decl->line_no = parser->token->line_no;
    action_decl->column_no = parser->token->column_no;
    if (token_is_name(parser->token)) {
      action_decl->actionDeclaration.name = parse_name(parser);
      if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        action_decl->actionDeclaration.params = parse_parameterList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
          if (parser->token->klass == TK_BRACE_OPEN) {
            action_decl->actionDeclaration.stmt = parse_blockStatement(parser);
          } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                       parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return action_decl;
  } else error("%s:%d:%d: error: `action` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** VARIABLES **/

static Ast* parse_variableDeclaration(Parser* parser, Ast* type_ref)
{
  bool is_const = 0;
  Ast* var_decl;

  if (parser->token->klass == TK_CONST) {
    next_token(parser);
    is_const = 1;
  }
  if (token_is_typeRef(parser->token) || type_ref) {
    var_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    var_decl->kind = AST_variableDeclaration;
    var_decl->line_no = parser->token->line_no;
    var_decl->column_no = parser->token->column_no;
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef(parser);
    if (token_is_name(parser->token)) {
      var_decl->variableDeclaration.name = parse_name(parser);
      if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        var_decl->variableDeclaration.init_expr = parse_expression(parser, 1);
      }
      if (parser->token->klass == TK_SEMICOLON) {
        next_token(parser);
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    var_decl->variableDeclaration.is_const = is_const;
    return var_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

/** EXPRESSIONS **/

static Ast* parse_functionDeclaration(Parser* parser, Ast* type_ref)
{
  Ast* func_decl;

  if (token_is_typeOrVoid(parser->token)) {
    func_decl = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    func_decl->kind = AST_functionDeclaration;
    func_decl->line_no = parser->token->line_no;
    func_decl->column_no = parser->token->column_no;
    func_decl->functionDeclaration.proto = parse_functionPrototype(parser, type_ref);
    if (parser->token->klass == TK_BRACE_OPEN) {
      func_decl->functionDeclaration.stmt = parse_blockStatement(parser);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    return func_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_argumentList(Parser* parser)
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {0};

  args = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  args->kind = AST_argumentList;
  args->line_no = parser->token->line_no;
  args->column_no = parser->token->column_no;
  if (token_is_argument(parser->token)) {
    ast = parse_argument(parser);
    ast_tree_append_node(&args->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_argument(parser);
      ast_tree_append_node(&args->tree, &tree_ctor, &ast->tree);
    }
  }
  return args;
}

static Ast* parse_argument(Parser* parser)
{
  Ast* arg, *dontcare_arg;

  if (token_is_argument(parser->token)) {
    arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    arg->kind = AST_argument;
    arg->line_no = parser->token->line_no;
    arg->column_no = parser->token->column_no;
    if (token_is_expression(parser->token)) {
      arg->argument.arg = parse_expression(parser, 1);
      return arg;
    } else if (parser->token->klass == TK_DONTCARE) {
      next_token(parser);
      dontcare_arg = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = parser->token->line_no;
      dontcare_arg->column_no = parser->token->column_no;
      arg->argument.arg = dontcare_arg;
      return arg;
    } else assert(0);
  } else error("%s:%d:%d: error: an argument was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_expressionList(Parser* parser)
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {0};
  
  exprs = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
  exprs->kind = AST_expressionList;
  exprs->line_no = parser->token->line_no;
  exprs->column_no = parser->token->column_no;
  if (token_is_expression(parser->token)) {
    ast = parse_expression(parser, 1);
    ast_tree_append_node(&exprs->tree, &tree_ctor, &ast->tree);
    while (parser->token->klass == TK_COMMA) {
      next_token(parser);
      ast = parse_expression(parser, 1);
      ast_tree_append_node(&exprs->tree, &tree_ctor, &ast->tree);
    }
  }
  return exprs;
}

static Ast* parse_lvalue(Parser* parser)
{
  Ast* lvalue, *expr;

  if (token_is_lvalue(parser->token)) {
    lvalue = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    lvalue->kind = AST_lvalueExpression;
    lvalue->line_no = parser->token->line_no;
    lvalue->column_no = parser->token->column_no;
    lvalue->lvalueExpression.expr = parse_nonTypeName(parser);
    while(parser->token->klass == TK_DOT || parser->token->klass == TK_BRACKET_OPEN) {
      if (parser->token->klass == TK_DOT) {
        next_token(parser);
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_memberSelector;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->memberSelector.lhs_expr = lvalue;
        if (token_is_name(parser->token)) {
          expr->memberSelector.name = parse_name(parser);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        lvalue = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = parser->token->line_no;
        lvalue->column_no = parser->token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
      else if (parser->token->klass == TK_BRACKET_OPEN) {
        next_token(parser);
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_arraySubscript;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->arraySubscript.lhs_expr = lvalue;
        expr->arraySubscript.index_expr = parse_indexExpression(parser);
        if (parser->token->klass == TK_BRACKET_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        lvalue = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = parser->token->line_no;
        lvalue->column_no = parser->token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
    }
    return lvalue;
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_expression(Parser* parser, int priority_threshold)
{
  Ast* primary, *expr;

  if (token_is_expression(parser->token)) {
    primary = parse_expressionPrimary(parser);
    while (token_is_exprOperator(parser->token)) {
      if (parser->token->klass == TK_DOT) {
        next_token(parser);
        Ast* expr;
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_memberSelector;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->memberSelector.lhs_expr = primary;
        if (token_is_nonTypeName(parser->token)) {
          expr->memberSelector.name = parse_nonTypeName(parser);
        } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (parser->token->klass == TK_BRACKET_OPEN) {
        next_token(parser);
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_arraySubscript;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->arraySubscript.lhs_expr = primary;
        expr->arraySubscript.index_expr = parse_indexExpression(parser);
        if (parser->token->klass == TK_BRACKET_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (parser->token->klass == TK_PARENTH_OPEN) {
        next_token(parser);
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_functionCall;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->functionCall.lhs_expr = primary;
        expr->functionCall.args = parse_argumentList(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (parser->token->klass == TK_EQUAL) {
        next_token(parser);
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_assignmentStatement;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->assignmentStatement.lhs_expr = primary;
        expr->assignmentStatement.rhs_expr = parse_expression(parser, 1);
        primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token_is_binaryOperator(parser->token)){
        int priority = operator_priority(parser->token);
        if (priority >= priority_threshold) {
          expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
          expr->kind = AST_binaryExpression;
          expr->line_no = parser->token->line_no;
          expr->column_no = parser->token->column_no;
          expr->binaryExpression.left_operand = primary;
          expr->binaryExpression.op = token_to_binop(parser->token);
          expr->binaryExpression.strname = parser->token->lexeme;
          next_token(parser);
          expr->binaryExpression.right_operand = parse_expression(parser, priority + 1);
          primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
          primary->kind = AST_expression;
          primary->line_no = expr->line_no;
          primary->column_no = expr->column_no;
          primary->expression.expr = expr;
        } else break;
      } else assert(0);
    }
    return primary;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_expressionPrimary(Parser* parser)
{
  Ast* primary, *expr;

  if (token_is_expression(parser->token)) {
    primary = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    primary->kind = AST_expression;
    primary->line_no = parser->token->line_no;
    primary->column_no = parser->token->column_no;
    if (parser->token->klass == TK_INTEGER_LITERAL) {
      primary->expression.expr = parse_integer(parser);
      return primary;
    } else if (parser->token->klass == TK_TRUE || parser->token->klass == TK_FALSE) {
      primary->expression.expr = parse_boolean(parser);
      return primary;
    } else if (parser->token->klass == TK_STRING_LITERAL) {
      primary->expression.expr = parse_string(parser);
      return primary;
    } else if (parser->token->klass == TK_DOT) {
      next_token(parser);
      if (parser->token->klass == TK_IDENTIFIER) {
        primary->expression.expr = parse_nonTypeName(parser);
        return primary;
      } else if (parser->token->klass == TK_TYPE_IDENTIFIER) {
        primary->expression.expr = parse_typeName(parser);
        return primary;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(parser->token)) {
      primary->expression.expr = parse_nonTypeName(parser);
      return primary;
    } else if (parser->token->klass == TK_BRACE_OPEN) {
      next_token(parser);
      primary->expression.expr = parse_expressionList(parser);
      if (parser->token->klass == TK_BRACE_CLOSE) {
        next_token(parser);
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      return primary;
    } else if (parser->token->klass == TK_PARENTH_OPEN) {
      next_token(parser);
      if (parser->token->klass == TK_TYPE_IDENTIFIER && peek_token(parser)->klass == TK_DOT) {
        /* (<typeName>.<name>) */
        primary->expression.expr = parse_expression(parser, 1);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        return primary;
      } else if (token_is_typeRef(parser->token)) {
        expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
        expr->kind = AST_castExpression;
        expr->line_no = parser->token->line_no;
        expr->column_no = parser->token->column_no;
        expr->castExpression.type = parse_typeRef(parser);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
          expr->castExpression.expr = parse_expression(parser, 10);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        primary->expression.expr = expr;
        return primary;
      } else if (token_is_expression(parser->token)) {
        primary->expression.expr = parse_expression(parser, 1);
        if (parser->token->klass == TK_PARENTH_CLOSE) {
          next_token(parser);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
        return primary;
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
      assert(0);
    } else if (parser->token->klass == TK_EXCLAMATION) {
      next_token(parser);
      expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = parser->token->line_no;
      expr->column_no = parser->token->column_no;
      expr->unaryExpression.op = OP_NOT;
      expr->unaryExpression.strname = parser->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (parser->token->klass == TK_TILDA) {
      next_token(parser);
      expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = parser->token->line_no;
      expr->column_no = parser->token->column_no;
      expr->unaryExpression.op = OP_BITW_NOT;
      expr->unaryExpression.strname = parser->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (parser->token->klass == TK_UNARY_MINUS) {
      next_token(parser);
      expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = parser->token->line_no;
      expr->column_no = parser->token->column_no;
      expr->unaryExpression.op = OP_NEG;
      expr->unaryExpression.strname = parser->token->lexeme;
      expr->unaryExpression.operand = parse_expression(parser, 1);
      primary->expression.expr = expr;
      return primary;
    } else if (token_is_typeName(parser->token)) {
      primary->expression.expr = parse_typeName(parser);
      return primary;
    } else if (parser->token->klass == TK_ERROR) {
      next_token(parser);
      expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
      expr->kind = AST_name;
      expr->line_no = parser->token->line_no;
      expr->column_no = parser->token->column_no;
      expr->name.strname = "error";
      primary->expression.expr = expr;
      return primary;
    } else assert(0);
    assert(0);
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_indexExpression(Parser* parser)
{
  Ast* index_expr;

  if (token_is_expression(parser->token)) {
    index_expr = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    index_expr->kind = AST_indexExpression;
    index_expr->line_no = parser->token->line_no;
    index_expr->column_no = parser->token->column_no;
    index_expr->indexExpression.start_index = parse_expression(parser, 1);
    if (parser->token->klass == TK_COLON) {
      next_token(parser);
      if (token_is_expression(parser->token)) {
        index_expr->indexExpression.end_index = parse_expression(parser, 1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
    }
    return index_expr;
  } else error("%s:%d:%d: expression or `:` was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_integer(Parser* parser)
{
  Ast* int_literal;

  if (parser->token->klass == TK_INTEGER_LITERAL) {
    int_literal = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    int_literal->kind = AST_integerLiteral;
    int_literal->line_no = parser->token->line_no;
    int_literal->column_no = parser->token->column_no;
    int_literal->integerLiteral.is_signed = parser->token->integer.is_signed;
    int_literal->integerLiteral.width = parser->token->integer.width;
    int_literal->integerLiteral.value = parser->token->integer.value;
    next_token(parser);
    return int_literal;
  } else error("%s:%d:%d: error: integer was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_boolean(Parser* parser)
{
  Ast* bool_literal;

  if (parser->token->klass == TK_TRUE || parser->token->klass == TK_FALSE) {
    bool_literal = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    bool_literal->kind = AST_booleanLiteral;
    bool_literal->line_no = parser->token->line_no;
    bool_literal->column_no = parser->token->column_no;
    bool_literal->booleanLiteral.value = (parser->token->klass == TK_TRUE);
    next_token(parser);
    return bool_literal;
  } else error("%s:%d:%d: error: boolean was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}

static Ast* parse_string(Parser* parser)
{
  Ast* string_literal;

  if (parser->token->klass == TK_STRING_LITERAL) {
    string_literal = (Ast*)arena_malloc(parser->storage, sizeof(Ast));
    string_literal->kind = AST_stringLiteral;
    string_literal->line_no = parser->token->line_no;
    string_literal->column_no = parser->token->column_no;
    string_literal->stringLiteral.value = parser->token->lexeme;
    next_token(parser);
    return string_literal;
  } else error("%s:%d:%d: error: string was expected, got `%s`.",
               parser->source_file, parser->token->line_no, parser->token->column_no, parser->token->lexeme);
  assert(0);
  return 0;
}
