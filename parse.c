#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static char*  source_file;
static Arena* storage;
static UnboundedArray* tokens;
static int    token_at = 0, prev_token_at = 0;
static Token* token = 0, *prev_token = 0;
static Scope* current_scope;

/** PROGRAM **/

static Ast* parse_p4program();
static Ast* parse_declarationList();
static Ast* parse_declaration();
static Ast* parse_nonTypeName();
static Ast* parse_name();
static Ast* parse_parameterList();
static Ast* parse_parameter();
static enum Ast_ParamDirection parse_direction();
static Ast* parse_packageTypeDeclaration();
static Ast* parse_instantiation(Ast* type_ref);
static Ast* parse_optConstructorParameters();

/** PARSER **/

static Ast* parse_parserDeclaration(Ast* parser_proto);
static Ast* parse_parserLocalElements();
static Ast* parse_parserLocalElement();
static Ast* parse_parserTypeDeclaration();
static Ast* parse_parserStates();
static Ast* parse_parserState();
static Ast* parse_parserStatements();
static Ast* parse_parserStatement();
static Ast* parse_parserBlockStatement();
static Ast* parse_transitionStatement();
static Ast* parse_stateExpression();
static Ast* parse_selectExpression();
static Ast* parse_selectCaseList();
static Ast* parse_selectCase();
static Ast* parse_keysetExpression();
static Ast* parse_tupleKeysetExpression();
static Ast* parse_simpleExpressionList();
static Ast* parse_simpleKeysetExpression();

/** CONTROL **/

static Ast* parse_controlDeclaration(Ast* control_proto);
static Ast* parse_controlTypeDeclaration();
static Ast* parse_controlLocalDeclaration();
static Ast* parse_controlLocalDeclarations();

/** EXTERN **/

static Ast* parse_externDeclaration();
static Ast* parse_methodPrototypes();
static Ast* parse_functionPrototype(Ast* return_type);
static Ast* parse_methodPrototype();

/** TYPES **/

static Ast* parse_typeRef();
static Ast* parse_namedType();
static Ast* parse_typeName();
static Ast* parse_tupleType();
static Ast* parse_headerStackType(Ast* named_type);
static Ast* parse_baseType();
static Ast* parse_integerTypeSize();
static Ast* parse_typeOrVoid();
static Ast* parse_realTypeArg();
static Ast* parse_typeArg();
static Ast* parse_typeArgumentList();
static Ast* parse_typeDeclaration();
static Ast* parse_derivedTypeDeclaration();
static Ast* parse_headerTypeDeclaration();
static Ast* parse_headerUnionDeclaration();
static Ast* parse_structTypeDeclaration();
static Ast* parse_structFieldList();
static Ast* parse_structField();
static Ast* parse_enumDeclaration();
static Ast* parse_errorDeclaration();
static Ast* parse_matchKindDeclaration();
static Ast* parse_identifierList();
static Ast* parse_specifiedIdentifierList();
static Ast* parse_specifiedIdentifier();
static Ast* parse_typedefDeclaration();

/** STATEMENTS **/

static Ast* parse_assignmentOrMethodCallStatement();
static Ast* parse_returnStatement();
static Ast* parse_exitStatement();
static Ast* parse_conditionalStatement();
static Ast* parse_directApplication(Ast* type_name);
static Ast* parse_statement(Ast* type_name);
static Ast* parse_blockStatement();
static Ast* parse_statementOrDeclList();
static Ast* parse_switchStatement();
static Ast* parse_switchCases();
static Ast* parse_switchCase();
static Ast* parse_switchLabel();
static Ast* parse_statementOrDeclaration();

/** TABLES **/

static Ast* parse_tableDeclaration();
static Ast* parse_tablePropertyList();
static Ast* parse_tableProperty();
static Ast* parse_keyElementList();
static Ast* parse_keyElement();
static Ast* parse_actionList();
static Ast* parse_actionRef();
static Ast* parse_entriesList();
static Ast* parse_entry();
static Ast* parse_actionDeclaration();

/** VARIABLES **/

static Ast* parse_variableDeclaration(Ast* type_ref);

/** EXPRESSIONS **/

static Ast* parse_functionDeclaration(Ast* type_ref);
static Ast* parse_argumentList();
static Ast* parse_argument();
static Ast* parse_expressionList();
static Ast* parse_lvalue();
static Ast* parse_expression(int priority_threshold);
static Ast* parse_expressionPrimary();
static Ast* parse_indexExpression();
static Ast* parse_integer();
static Ast* parse_boolean();
static Ast* parse_string();

static Token*
next_token()
{
  assert(token_at < tokens->elem_count);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  prev_token = token;
  prev_token_at = token_at;
  token = array_get_element(tokens, ++token_at, sizeof(Token));
  while (token->klass == TK_COMMENT) {
    token = array_get_element(tokens, ++token_at, sizeof(Token));
  }
  if (token->klass == TK_IDENTIFIER) {
    name_entry = scope_lookup(current_scope, token->lexeme);
    if (name_entry->ns[NAMESPACE_KEYWORD]) {
      name_decl = name_entry->ns[NAMESPACE_KEYWORD];
      token->klass = name_decl->token_class;
      return token;
    } else if (name_entry->ns[NAMESPACE_TYPE]) {
      token->klass = TK_TYPE_IDENTIFIER;
      return token;
    }
  }
  return token;
}

static Token*
peek_token()
{
  Token* peek_token;

  prev_token = token;
  prev_token_at = token_at;
  peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

static bool
token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES;
  return result;
}

static bool
token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

static bool
token_is_typeName(Token* token)
{
  return token->klass == TK_TYPE_IDENTIFIER;
}

static bool
token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE;
  return result;
}

static bool
token_is_baseType(Token* token)
{
  bool result = token->klass == TK_BOOL || token->klass == TK_ERROR || token->klass == TK_INT
    || token->klass == TK_BIT || token->klass == TK_VARBIT || token->klass == TK_STRING
    || token->klass == TK_VOID;
  return result;
}

static bool
token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token->klass == TK_TYPE_IDENTIFIER || token->klass == TK_TUPLE;
  return result;
}

static bool
token_is_direction(Token* token)
{
  bool result = token->klass == TK_IN || token->klass == TK_OUT || token->klass == TK_INOUT;
  return result;
}

static bool
token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

static bool
token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TK_HEADER || token->klass == TK_HEADER_UNION || token->klass == TK_STRUCT
    || token->klass == TK_ENUM;
  return result;
}

static bool
token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF
    || token->klass == TK_PARSER || token->klass == TK_CONTROL || token->klass == TK_PACKAGE;
  return result;
}

static bool
token_is_typeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

static bool
token_is_typeOrVoid(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_VOID || token->klass == TK_IDENTIFIER;
  return result;
}

static bool
token_is_actionRef(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_PARENTH_OPEN;
  return result;
}

static bool
token_is_tableProperty(Token* token)
{
  bool result = token->klass == TK_KEY || token->klass == TK_ACTIONS
    || token->klass == TK_CONST || token->klass == TK_ENTRIES
    || token_is_nonTableKwName(token);
  return result;
}

static bool
token_is_switchLabel(Token* token)
{
  bool result = token_is_name(token) || token->klass == TK_DEFAULT;
  return result;
}

static bool
token_is_expressionPrimary(Token* token)
{
  bool result = token->klass == TK_INTEGER_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

static bool
token_is_expression(Token* token)
{
  return token_is_expressionPrimary(token);
}

static bool
token_is_methodPrototype(Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

static bool
token_is_structField(Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

static bool
token_is_specifiedIdentifier(Token* token)
{
  return token_is_name(token);
}

static bool
token_is_declaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token);
  return result;
}

static bool
token_is_lvalue(Token* token)
{
  bool result = token_is_nonTypeName(token) || (token->klass == TK_DOT);
  return result;
}

static bool
token_is_assignmentOrMethodCallStatement(Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TK_PARENTH_OPEN || token->klass == TK_ANGLE_OPEN
    || token->klass == TK_EQUAL;
  return result;
}

static bool
token_is_statement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TK_IF
    || token->klass == TK_SEMICOLON || token->klass == TK_BRACE_OPEN || token->klass == TK_EXIT
    || token->klass == TK_RETURN || token->klass == TK_SWITCH;
  return result;
}

static bool
token_is_statementOrDeclaration(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_CONST || token_is_statement(token);
  return result;
}

static bool
token_is_argument(Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TK_DONTCARE;
  return result;
}

static bool
token_is_parserLocalElement(Token* token)
{
  bool result = token->klass == TK_CONST || token_is_typeRef(token);
  return result;
}

static bool
token_is_parserStatement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_CONST || token_is_typeRef(token)
    || token->klass == TK_SEMICOLON;
  return result;
}

static bool
token_is_simpleKeysetExpression(Token* token) {
  bool result = token_is_expression(token) || token->klass == TK_DEFAULT || token->klass == TK_DONTCARE;
  return result;
}

static bool
token_is_keysetExpression(Token* token)
{
  bool result = token->klass == TK_TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

static bool
token_is_selectCase(Token* token)
{
  return token_is_keysetExpression(token);
}

static bool
token_is_controlLocalDeclaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_ACTION
    || token->klass == TK_TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

static bool
token_is_realTypeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE|| token_is_typeRef(token);
  return result;
}

static bool
token_is_binaryOperator(Token* token)
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

static bool
token_is_exprOperator(Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOT
    || token->klass == TK_BRACKET_OPEN || token->klass == TK_PARENTH_OPEN
    || token->klass == TK_ANGLE_OPEN;
  return result;
}

static int
operator_priority(Token* token)
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

static enum Ast_Operator
token_to_binop(Token* token)
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
    default: return 0;
  }
}

char*
Debug_AstEnum_to_string(enum AstEnum ast)
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
    case AST_namedType: return "AST_namedType";
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
    case AST_entriesProperty: return "AST_entriesProperty";
    case AST_entriesList: return "AST_entriesList";
    case AST_entry: return "AST_entry";
    case AST_simpleProperty: return "AST_simpleProperty";
    case AST_actionDeclaration: return "AST_actionDeclaration";

    /** VARIABLES **/

    case AST_variableDeclaration: return "AST_variableDeclaration";
    case AST_constantDeclaration: return "AST_constantDeclaration";

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

Ast*
parse_program(char* source_file_, UnboundedArray* tokens_, Arena* storage_, Scope* root_scope)
{
  Ast *program;

  source_file = source_file_;
  tokens = tokens_;
  storage = storage_;
  current_scope = root_scope;

  token_at = 0;
  token = array_get_element(tokens, token_at, sizeof(Token));
  next_token();
  program = parse_p4program();
  assert(current_scope == root_scope);
  return program;
}

/** PROGRAM **/

static Ast*
parse_p4program()
{
  Ast* program;
  Scope* scope;

  program = arena_malloc(storage, sizeof(Ast));
  program->kind = AST_p4program;
  program->line_no = token->line_no;
  program->column_no = token->column_no;
  while (token->klass == TK_SEMICOLON) {
    next_token(); /* empty declaration */
  }
  scope = scope_create(storage, 1008);
  current_scope = scope_push(scope, current_scope);
  program->p4program.decl_list = parse_declarationList();
  current_scope = scope_pop(current_scope);
  if (token->klass != TK_END_OF_INPUT) {
    error("%s:%d:%d: error: unexpected token `%s`.",
          source_file, token->line_no, token->column_no, token->lexeme);
  }
  return program;
}

static Ast*
parse_declarationList()
{
  Ast* decls, *ast;

  decls = arena_malloc(storage, sizeof(Ast));
  decls->kind = AST_declarationList;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token_is_declaration(token)) {
    ast = parse_declaration();
    decls->declarationList.first_child = ast;
    while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
      if (token_is_declaration(token)) {
        ast->right_sibling = parse_declaration();
        ast = ast->right_sibling;
      } else if (token->klass == TK_SEMICOLON) {
        next_token(); /* empty declaration */
      }
    }
  }
  return decls;
}

static Ast*
parse_declaration()
{
  Ast* decl;

  if (token_is_declaration(token)) {
    decl = arena_malloc(storage, sizeof(Ast));
    decl->kind = AST_declaration;
    decl->line_no = token->line_no;
    decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      decl->declaration.decl = parse_variableDeclaration(0);
      return decl;
    } else if (token->klass == TK_EXTERN) {
      decl->declaration.decl = parse_externDeclaration();
      return decl;
    } else if (token->klass == TK_ACTION) {
      decl->declaration.decl = parse_actionDeclaration();
      return decl;
    } else if (token->klass == TK_PARSER) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else {
        decl->declaration.decl = parse_parserDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token->klass == TK_CONTROL) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else {
        decl->declaration.decl = parse_controlDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token_is_typeDeclaration(token)) {
      decl->declaration.decl = parse_typeDeclaration();
      return decl;
    } else if (token->klass == TK_ERROR) {
      decl->declaration.decl = parse_errorDeclaration();
      return decl;
    } else if (token->klass == TK_MATCH_KIND) {
      decl->declaration.decl = parse_matchKindDeclaration();
      return decl;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        decl->declaration.decl = parse_instantiation(type_ref);
        return decl;
      } else if (token_is_name(token)) {
        decl->declaration.decl = parse_functionDeclaration(type_ref);
        return decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(token)) {
      decl->declaration.decl = parse_functionDeclaration(parse_typeRef());
      return decl;
    } else assert(0);
  } else error("%s:%d:%d: error: top-level declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_nonTypeName()
{
  Ast* name;

  if (token_is_nonTypeName(token)) {
    name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = token->line_no;
    name->column_no = token->column_no;
    name->name.strname = token->lexeme;
    next_token();
    return name;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_name()
{
  Ast* type_name;

  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      return parse_nonTypeName();
    } else if (token->klass == TK_TYPE_IDENTIFIER) {
      type_name = arena_malloc(storage, sizeof(Ast));
      type_name->kind = AST_name;
      type_name->line_no = token->line_no;
      type_name->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      next_token();
      return type_name;
    } else assert(0);
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parameterList()
{
  Ast* params, *ast;

  params = arena_malloc(storage, sizeof(Ast));
  params->kind = AST_parameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  if (token_is_parameter(token)) {
    ast = parse_parameter();
    params->parameterList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_parameter();
      ast = ast->right_sibling;
    }
  }
  return params;
}

static Ast*
parse_parameter()
{
  Ast* param;

  if (token_is_parameter(token)) {
    param = arena_malloc(storage, sizeof(Ast));
    param->kind = AST_parameter;
    param->line_no = token->line_no;
    param->column_no = token->column_no;
    param->parameter.direction = parse_direction();
    param->parameter.type = parse_typeRef();
    if (token_is_name(token)) {
      param->parameter.name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          param->parameter.init_expr = parse_expression(1);
        } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return param;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static enum Ast_ParamDirection
parse_direction()
{
  if (token_is_direction(token)) {
    if (token->klass == TK_IN) {
      next_token();
      return PARAMDIR_IN;
    } else if (token->klass == TK_OUT) {
      next_token();
      return PARAMDIR_OUT;
    } else if (token->klass == TK_INOUT) {
      next_token();
      return PARAMDIR_INOUT;
    } else assert(0);
  }
  return 0;
}

static Ast*
parse_packageTypeDeclaration()
{
  Ast* package_decl, *name;

  if (token->klass == TK_PACKAGE) {
    next_token();
    package_decl = arena_malloc(storage, sizeof(Ast));
    package_decl->kind = AST_packageTypeDeclaration;
    package_decl->line_no = token->line_no;
    package_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      package_decl->packageTypeDeclaration.name = name;
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        package_decl->packageTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return package_decl;
  } else error("%s:%d:%d: error: `package` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_instantiation(Ast* type_ref)
{
  Ast* inst_stmt;

  if (token_is_typeRef(token) || type_ref) {
    inst_stmt = arena_malloc(storage, sizeof(Ast));
    inst_stmt->kind = AST_instantiation;
    inst_stmt->instantiation.type = type_ref ? type_ref : parse_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      inst_stmt->instantiation.args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          inst_stmt->line_no = token->line_no;
          inst_stmt->column_no = token->column_no;
          inst_stmt->instantiation.name = parse_name();
          if (token->klass == TK_SEMICOLON) {
            next_token();
          } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: instance name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return inst_stmt;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** PARSER **/

static Ast*
parse_optConstructorParameters()
{
   Ast* params;

  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    if (token_is_parameter(token)) {
      params = parse_parameterList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return params;
    } else if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
  }
  return 0;
}

static Ast*
parse_parserDeclaration(Ast* parser_proto)
{
  Ast* parser_decl;

  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    parser_decl = arena_malloc(storage, sizeof(Ast));
    parser_decl->kind = AST_parserDeclaration;
    parser_decl->line_no = token->line_no;
    parser_decl->column_no = token->column_no;
    parser_decl->parserDeclaration.proto = parser_proto;
    parser_decl->parserDeclaration.ctor_params = parse_optConstructorParameters();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      parser_decl->parserDeclaration.local_elements = parse_parserLocalElements();
      if (token->klass == TK_STATE) {
        parser_decl->parserDeclaration.states = parse_parserStates();
      } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return parser_decl;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserLocalElements()
{
  Ast* elems, *ast;

  elems = arena_malloc(storage, sizeof(Ast));
  elems->kind = AST_parserLocalElements;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token_is_parserLocalElement(token)) {
    ast = parse_parserLocalElement();
    elems->parserLocalElements.first_child = ast;
    while (token_is_parserLocalElement(token)) {
      ast->right_sibling = parse_parserLocalElement();
      ast = ast->right_sibling;
    }
  }
  return elems;
}

static Ast*
parse_parserLocalElement()
{
  Ast* local_element, *type_ref;

  if (token_is_parserLocalElement(token)) {
    local_element = arena_malloc(storage, sizeof(Ast));
    local_element->kind = AST_parserLocalElement;
    local_element->line_no = token->line_no;
    local_element->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_element->parserLocalElement.element = parse_variableDeclaration(0);
      return local_element;
    } else if (token_is_typeRef(token)) {
      type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_element->parserLocalElement.element = parse_instantiation(type_ref);
        return local_element;
      } else if (token_is_name(token)) {
        local_element->parserLocalElement.element = parse_variableDeclaration(type_ref);
        return local_element;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserTypeDeclaration()
{
  Ast* parser_proto, *name;

  if (token->klass == TK_PARSER) {
    next_token();
    parser_proto = arena_malloc(storage, sizeof(Ast));
    parser_proto->kind = AST_parserTypeDeclaration;
    parser_proto->line_no = token->line_no; 
    parser_proto->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      parser_proto->parserTypeDeclaration.name = name;
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        parser_proto->parserTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return parser_proto;
  } else error("%s:%d:%d: error: `parser` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserStates()
{
  Ast* states, *ast;

  states = arena_malloc(storage, sizeof(Ast));
  states->kind = AST_parserStates;
  states->line_no = token->line_no;
  states->column_no = token->column_no;
  if (token->klass == TK_STATE) {
    ast = parse_parserState();
    states->parserStates.first_child = ast;
    while (token->klass == TK_STATE) {
      ast->right_sibling = parse_parserState();
      ast = ast->right_sibling;
    }
  }
  return states;
}

static Ast*
parse_parserState()
{
  Ast* state;

  if (token->klass == TK_STATE) {
    next_token();
    state = arena_malloc(storage, sizeof(Ast));
    state->kind = AST_parserState;
    state->line_no = token->line_no;
    state->column_no = token->column_no;
    state->parserState.name = parse_name();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      state->parserState.stmt_list = parse_parserStatements();
      state->parserState.transition_stmt = parse_transitionStatement();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return state;
  } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserStatements()
{
  Ast* stmts, *ast;

  stmts = arena_malloc(storage, sizeof(Ast));
  stmts->kind = AST_parserStatements;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token_is_parserStatement(token)) {
    ast = parse_parserStatement();
    stmts->parserStatements.first_child = ast;
    while (token_is_parserStatement(token)) {
      ast->right_sibling = parse_parserStatement();
      ast = ast->right_sibling;
    }
  }
  return stmts;
}

static Ast*
parse_parserStatement()
{
  Ast* parser_stmt, *type_ref;

  if (token_is_parserStatement(token)) {
    parser_stmt = arena_malloc(storage, sizeof(Ast));
    parser_stmt->kind = AST_parserStatement;
    parser_stmt->line_no = token->line_no;
    parser_stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      type_ref = parse_typeRef();
      if (token_is_name(token)) {
        parser_stmt->parserStatement.stmt = parse_variableDeclaration(type_ref);
        return parser_stmt;
      } else {
        parser_stmt->parserStatement.stmt = parse_directApplication(type_ref);
        return parser_stmt;
      }
    } else if (token_is_assignmentOrMethodCallStatement(token)) {
      parser_stmt->parserStatement.stmt = parse_assignmentOrMethodCallStatement();
      return parser_stmt;
    } else if (token->klass == TK_BRACE_OPEN) {
      parser_stmt->parserStatement.stmt = parse_parserBlockStatement();
      return parser_stmt;
    } else if (token->klass == TK_CONST) {
      parser_stmt->parserStatement.stmt = parse_variableDeclaration(0);
      return parser_stmt;
    } else if (token->klass == TK_SEMICOLON) {
      Ast* stmt = arena_malloc(storage, sizeof(Ast));
      stmt->kind = AST_emptyStatement;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      parser_stmt->parserStatement.stmt = stmt;
      return parser_stmt;
    } else assert(0);
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserBlockStatement()
{
  Ast* stmt;

  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_parserBlockStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    stmt->parserBlockStatement.stmt_list = parse_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_transitionStatement()
{
  Ast* transition;

  if (token->klass == TK_TRANSITION) {
    next_token();
    transition = arena_malloc(storage, sizeof(Ast));
    transition->kind = AST_transitionStatement;
    transition->line_no = token->line_no;
    transition->column_no = token->column_no;
    transition->transitionStatement.stmt = parse_stateExpression();
    return transition;
  } else error("%s:%d:%d: error: `transition` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_stateExpression()
{
  Ast* state_expr;

  if (token_is_name(token) || token->klass == TK_SELECT) {
    state_expr = arena_malloc(storage, sizeof(Ast));
    state_expr->kind = AST_stateExpression;
    state_expr->line_no = token->line_no;
    state_expr->column_no = token->column_no;
    if (token_is_name(token)) {
      state_expr->stateExpression.expr = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                  source_file, token->line_no, token->column_no, token->lexeme);
      return state_expr;
    } else if (token->klass == TK_SELECT) {
      state_expr->stateExpression.expr = parse_selectExpression();
      return state_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: state expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_selectExpression()
{
  Ast* select_expr;

  if (token->klass == TK_SELECT) {
    next_token();
    select_expr = arena_malloc(storage, sizeof(Ast));
    select_expr->kind = AST_selectExpression;
    select_expr->line_no = token->line_no;
    select_expr->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      select_expr->selectExpression.expr_list = parse_expressionList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          select_expr->selectExpression.case_list = parse_selectCaseList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return select_expr;
  } else error("%s:%d:%d: error: `select` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_selectCaseList()
{
  Ast* cases, *ast;

  cases = arena_malloc(storage, sizeof(Ast));
  cases->kind = AST_selectCaseList;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token_is_selectCase(token)) {
    ast = parse_selectCase();
    cases->selectCaseList.first_child = ast;
    while (token_is_selectCase(token)) {
      ast->right_sibling = parse_selectCase();
      ast = ast->right_sibling;
    }
  }
  return cases;
}

static Ast*
parse_selectCase()
{
  Ast* select_case;

  if (token_is_keysetExpression(token)) {
    select_case = arena_malloc(storage, sizeof(Ast));
    select_case->kind = AST_selectCase;
    select_case->line_no = token->line_no;
    select_case->column_no = token->column_no;
    select_case->selectCase.keyset_expr = parse_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_name(token)) {
        select_case->selectCase.name = parse_name();
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return select_case;
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_keysetExpression()
{
  Ast* keyset_expr;

  if (token->klass == TK_PARENTH_OPEN || token_is_simpleKeysetExpression(token)) {
    keyset_expr = arena_malloc(storage, sizeof(Ast));
    keyset_expr->kind = AST_keysetExpression;
    keyset_expr->line_no = token->line_no;
    keyset_expr->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      keyset_expr->keysetExpression.expr = parse_tupleKeysetExpression();
      return keyset_expr;
    } else if (token_is_simpleKeysetExpression(token)) {
      keyset_expr->keysetExpression.expr = parse_simpleKeysetExpression();
      return keyset_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tupleKeysetExpression()
{
  Ast* tuple_keyset;

  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    tuple_keyset = arena_malloc(storage, sizeof(Ast));
    tuple_keyset->kind = AST_tupleKeysetExpression;
    tuple_keyset->line_no = token->line_no;
    tuple_keyset->column_no = token->column_no;
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return tuple_keyset;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_simpleExpressionList()
{
  Ast* exprs, *ast;

  exprs = arena_malloc(storage, sizeof(Ast));
  exprs->kind = AST_simpleExpressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  if (token_is_expression(token)) {
    ast = parse_simpleKeysetExpression();
    exprs->simpleExpressionList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_simpleKeysetExpression();
      ast = ast->right_sibling;
    }
  }
  return exprs;
}

static Ast*
parse_simpleKeysetExpression()
{
  Ast* simple_keyset, *default_keyset, *dontcare_keyset;

  if (token_is_simpleKeysetExpression(token)) {
    simple_keyset = arena_malloc(storage, sizeof(Ast));
    simple_keyset->kind = AST_simpleKeysetExpression;
    simple_keyset->line_no = token->line_no;
    simple_keyset->column_no = token->column_no;
    if (token_is_expression(token)) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(1);
      return simple_keyset;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      default_keyset = arena_malloc(storage, sizeof(Ast));
      default_keyset->kind = AST_default;
      default_keyset->line_no = token->line_no;
      default_keyset->column_no = token->column_no;
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      dontcare_keyset = arena_malloc(storage, sizeof(Ast));
      dontcare_keyset->kind = AST_dontcare;
      dontcare_keyset->line_no = token->line_no;
      dontcare_keyset->column_no = token->column_no;
      simple_keyset->simpleKeysetExpression.expr = dontcare_keyset;
      return simple_keyset;
    }
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** CONTROL **/

static Ast*
parse_controlDeclaration(Ast* control_proto)
{
  Ast* control_decl;

  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    control_decl = arena_malloc(storage, sizeof(Ast));
    control_decl->kind = AST_controlDeclaration;
    control_decl->line_no = token->line_no;
    control_decl->column_no = token->column_no;
    control_decl->controlDeclaration.proto = control_proto;
    control_decl->controlDeclaration.ctor_params = parse_optConstructorParameters();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      control_decl->controlDeclaration.local_decls = parse_controlLocalDeclarations();
      if (token->klass == TK_APPLY) {
        next_token();
        control_decl->controlDeclaration.apply_stmt = parse_blockStatement();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return control_decl;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlTypeDeclaration()
{
  Ast* control_proto, *name;

  if (token->klass == TK_CONTROL) {
    next_token();
    control_proto = arena_malloc(storage, sizeof(Ast));
    control_proto->kind = AST_controlTypeDeclaration;
    control_proto->line_no = token->line_no;
    control_proto->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      control_proto->controlTypeDeclaration.name = name;
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        control_proto->controlTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return control_proto;
  } else error("%s:%d:%d: error: `control` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlLocalDeclaration()
{
  Ast* local_decl, *type_ref;

  if (token_is_controlLocalDeclaration(token)) {
    local_decl = arena_malloc(storage, sizeof(Ast));
    local_decl->kind = AST_controlLocalDeclaration;
    local_decl->line_no = token->line_no;
    local_decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(0);
      return local_decl;
    } else if (token->klass == TK_ACTION) {
      local_decl->controlLocalDeclaration.decl = parse_actionDeclaration();
      return local_decl;
    } else if (token->klass == TK_TABLE) {
      local_decl->controlLocalDeclaration.decl = parse_tableDeclaration();
      return local_decl;
    } else if (token_is_typeRef(token)) {
      type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(type_ref);
        return local_decl;
      } else if (token_is_name(token)) {
        local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(type_ref);
        return local_decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("%s:%d:%d: error: local declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlLocalDeclarations()
{
  Ast* decls, *ast;

  decls = arena_malloc(storage, sizeof(Ast));
  decls->kind = AST_controlLocalDeclarations;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token_is_controlLocalDeclaration(token)) {
    ast = parse_controlLocalDeclaration();
    decls->controlLocalDeclarations.first_child = ast;
    while (token_is_controlLocalDeclaration(token)) {
      ast->right_sibling = parse_controlLocalDeclaration();
      ast = ast->right_sibling;
    }
  }
  return decls;
}

/** EXTERN **/

static Ast*
parse_externDeclaration()
{
  Ast* extern_decl, *extern_type;
  bool is_function_type = false;
  Ast* name;

  if (token->klass == TK_EXTERN) {
    next_token();
    extern_decl = arena_malloc(storage, sizeof(Ast));
    extern_decl->kind = AST_externDeclaration;
    extern_decl->line_no = token->line_no;
    extern_decl->column_no = token->column_no;

    if (token_is_typeOrVoid(token) && token_is_nonTypeName(token)) {
      is_function_type = token_is_typeOrVoid(token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(token)) {
      is_function_type = true;
    } else if (token_is_nonTypeName(token)) {
      is_function_type = false;
    } else error("%s:%d:%d: error: extern declaration was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);

    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return extern_decl;
    } else {
      extern_type = arena_malloc(storage, sizeof(Ast));
      extern_type->kind = AST_externTypeDeclaration;
      extern_type->line_no = token->line_no;
      extern_type->column_no = token->column_no;
      extern_type->externTypeDeclaration.name = parse_nonTypeName();
      name = extern_type->externTypeDeclaration.name;
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      extern_decl->externDeclaration.decl = extern_type;
      return extern_decl;
    }
  } else error("%s:%d:%d: error: `extern` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_methodPrototypes()
{
  Ast* protos, *ast;

  protos = arena_malloc(storage, sizeof(Ast));
  protos->kind = AST_methodPrototypes;
  protos->line_no = token->line_no;
  protos->column_no = token->column_no;
  if (token_is_methodPrototype(token)) {
    ast = parse_methodPrototype();
    protos->methodPrototypes.first_child = ast;
    while (token_is_methodPrototype(token)) {
      ast->right_sibling = parse_methodPrototype();
      ast = ast->right_sibling;
    }
  }
  return protos;
}

static Ast*
parse_functionPrototype(Ast* return_type)
{
  Ast* func_proto, *type_ref;
  Ast* name;

  if (token_is_typeOrVoid(token) || return_type) {
    func_proto = arena_malloc(storage, sizeof(Ast));
    func_proto->kind = AST_functionPrototype;
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      return_type = parse_typeOrVoid();
      if (return_type->kind == AST_name) {
        name = return_type;
        scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
        type_ref = arena_malloc(storage, sizeof(Ast));
        type_ref->kind = AST_typeRef;
        type_ref->line_no = token->line_no;
        type_ref->column_no = token->column_no;
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token_is_name(token)) {
      func_proto->line_no = token->line_no;
      func_proto->column_no = token->column_no;
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: function name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return func_proto;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_methodPrototype()
{
  Ast* func_proto;

  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      func_proto = arena_malloc(storage, sizeof(Ast));
      func_proto->kind = AST_functionPrototype;
      func_proto->line_no = token->line_no;
      func_proto->column_no = token->column_no;
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else if (token_is_typeOrVoid(token)) {
      func_proto = parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** TYPES **/

static Ast*
parse_typeRef()
{
  Ast* type_ref;

  if (token_is_typeRef(token)) {
    type_ref = arena_malloc(storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = token->line_no;
    type_ref->column_no = token->column_no;
    if (token_is_baseType(token)) {
      type_ref->typeRef.type = parse_baseType();
      return type_ref;
    } else if (token_is_typeName(token)) {
      type_ref->typeRef.type = parse_namedType();
      return type_ref;
    } else if (token->klass == TK_TUPLE) {
      type_ref->typeRef.type = parse_tupleType();
      return type_ref;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_namedType()
{
  Ast* named_type;

  if (token_is_typeName(token)) {
    named_type = parse_typeName();
    if (token->klass == TK_BRACKET_OPEN) {
      named_type = parse_headerStackType(named_type);
      return named_type;
    }
    return named_type;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typeName()
{
  Ast* type_name;

  if (token->klass == TK_TYPE_IDENTIFIER) {
    type_name = arena_malloc(storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    type_name->name.strname = token->lexeme;
    next_token();
    return type_name;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tupleType()
{
  Ast* tuple;

  if (token->klass == TK_TUPLE) {
    tuple = arena_malloc(storage, sizeof(Ast));
    tuple->kind = AST_tupleType;
    tuple->line_no = token->line_no;
    tuple->column_no = token->column_no;
    next_token();
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      tuple->tupleType.type_args = parse_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return tuple;
  } else error("%s:%d:%d: error: `tuple` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerStackType(Ast* named_type)
{
  Ast* type_ref, *type;

  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    type_ref = arena_malloc(storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    type = arena_malloc(storage, sizeof(Ast));
    type->kind = AST_headerStackType;
    type->line_no = named_type->line_no;
    type->column_no = named_type->column_no;
    type->headerStackType.type = type_ref;
    if (token_is_expression(token)) {
      type->headerStackType.stack_expr = parse_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: expression expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return type;
  } else error("%s:%d:%d: error: `[` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_baseType()
{
  Ast* type_name, *type;

  if (token_is_baseType(token)) {
    type_name = arena_malloc(storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    if (token->klass == TK_BOOL) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeBoolean;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeBoolean.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TK_INT) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeInteger;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeInteger.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        type->baseTypeInteger.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TK_BIT) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeBit;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeBit.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        type->baseTypeBit.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TK_VARBIT) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeVarbit;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeVarbit.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        type->baseTypeVarbit.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: '<' was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return type;
    } else if (token->klass == TK_STRING) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeString;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeString.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TK_VOID) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeVoid;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeVoid.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TK_ERROR) {
      type = arena_malloc(storage, sizeof(Ast));
      type->kind = AST_baseTypeError;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeError.name = type_name;
      next_token();
      return type;
    } else assert(0);
  } else error("%s:%d:%d: error: base type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_integerTypeSize()
{
  Ast* type_size;

  type_size = arena_malloc(storage, sizeof(Ast));
  type_size->kind = AST_integerTypeSize;
  type_size->line_no = token->line_no;
  type_size->column_no = token->column_no;
  if (token->klass == TK_INTEGER_LITERAL) {
    type_size->integerTypeSize.size = parse_integer();
  } else if (token->klass == TK_PARENTH_OPEN) {
    /* TODO
    type_size->size = parse_expression(1); */
    error("%s:%d:%d: error: integer was expected, got `%s`.",
          source_file, token->line_no, token->column_no, token->lexeme);
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  return type_size;
}

static Ast*
parse_typeOrVoid()
{
  Ast* type, *name;

  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      type = parse_typeRef();
      return type;
    } else if (token->klass == TK_VOID) {
      return parse_baseType();
    } else if (token->klass == TK_IDENTIFIER) {
      name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->name.strname = token->lexeme;
      next_token();
      return name;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_realTypeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_realTypeArg(token)) {
    type_arg = arena_malloc(storage, sizeof(Ast));
    type_arg->kind = AST_realTypeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->realTypeArg.arg = parse_typeRef();
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token_is_typeArg(token)) {
    type_arg = arena_malloc(storage, sizeof(Ast));
    type_arg->kind = AST_typeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->typeArg.arg = parse_typeRef();
      return type_arg;
    } else if (token_is_nonTypeName(token)) {
      type_arg->typeArg.arg = parse_nonTypeName();
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typeArgumentList()
{
  Ast* args, *ast;

  args = arena_malloc(storage, sizeof(Ast));
  args->kind = AST_typeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token_is_typeArg(token)) {
    ast = parse_typeArg();
    args->typeArgumentList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_typeArg();
      ast = ast->right_sibling;
    }
  }
  return args;
}

static Ast*
parse_typeDeclaration()
{
  Ast* type_decl;

  if (token_is_typeDeclaration(token)) {
    type_decl = arena_malloc(storage, sizeof(Ast));
    type_decl->kind = AST_typeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token_is_derivedTypeDeclaration(token)) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_TYPEDEF) {
      type_decl->typeDeclaration.decl = parse_typedefDeclaration();
      return type_decl;
    } else if (token->klass == TK_PARSER) {
      type_decl->typeDeclaration.decl = parse_parserTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_CONTROL) {
      type_decl->typeDeclaration.decl = parse_controlTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_PACKAGE) {
      type_decl->typeDeclaration.decl = parse_packageTypeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: type declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme); 
  assert(0);
  return 0;
}

static Ast*
parse_derivedTypeDeclaration()
{
  Ast* type_decl;

  if (token_is_derivedTypeDeclaration(token)) {
    type_decl = arena_malloc(storage, sizeof(Ast));
    type_decl->kind = AST_derivedTypeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token->klass == TK_HEADER) {
      type_decl->derivedTypeDeclaration.decl = parse_headerTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_HEADER_UNION) {
      type_decl->derivedTypeDeclaration.decl = parse_headerUnionDeclaration();
      return type_decl;
    } else if (token->klass == TK_STRUCT) {
      type_decl->derivedTypeDeclaration.decl = parse_structTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_ENUM) {
      type_decl->derivedTypeDeclaration.decl = parse_enumDeclaration();
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: structure declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerTypeDeclaration()
{
  Ast* header_decl;
  Ast* name;

  if (token->klass == TK_HEADER) {
    next_token();
    header_decl = arena_malloc(storage, sizeof(Ast));
    header_decl->kind = AST_headerTypeDeclaration;
    header_decl->line_no = token->line_no;
    header_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      header_decl->headerTypeDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        header_decl->headerTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return header_decl;
  } else error("%s:%d:%d: error: `header` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerUnionDeclaration()
{
  Ast* union_decl;
  Ast* name;

  if (token->klass == TK_HEADER_UNION) {
    next_token();
    union_decl = arena_malloc(storage, sizeof(Ast));
    union_decl->kind = AST_headerUnionDeclaration;
    union_decl->line_no = token->line_no;
    union_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      union_decl->headerUnionDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        union_decl->headerUnionDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return union_decl;
  } else error("%s:%d:%d: error: `header_union` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_structTypeDeclaration()
{
  Ast* struct_decl;
  Ast* name;

  if (token->klass == TK_STRUCT) {
    next_token();
    struct_decl = arena_malloc(storage, sizeof(Ast));
    struct_decl->kind = AST_structTypeDeclaration;
    struct_decl->line_no = token->line_no;
    struct_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      struct_decl->structTypeDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        struct_decl->structTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return struct_decl;
  } else error("%s:%d:%d: error: `struct` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_structFieldList()
{
  Ast* fields, *ast;

  fields = arena_malloc(storage, sizeof(Ast));
  fields->kind = AST_structFieldList;
  fields->line_no = token->line_no;
  fields->column_no = token->column_no;
  if (token_is_structField(token)) {
    ast = parse_structField();
    fields->structFieldList.first_child = ast;
    while (token_is_structField(token)) {
      ast->right_sibling = parse_structField();
      ast = ast->right_sibling;
    }
  }
  return fields;
}

static Ast*
parse_structField()
{
  if (token_is_structField(token)) {
    Ast* field = arena_malloc(storage, sizeof(Ast));
    field->kind = AST_structField;
    field->line_no = token->line_no;
    field->column_no = token->column_no;
    field->structField.type = parse_typeRef();
    if (token_is_name(token)) {
      field->structField.name = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return field;
  } else error("%s:%d:%d: error: struct field was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_enumDeclaration()
{
  Ast* enum_decl;
  Ast* name;

  if (token->klass == TK_ENUM) {
    next_token();
    enum_decl = arena_malloc(storage, sizeof(Ast));
    enum_decl->kind = AST_enumDeclaration;
    enum_decl->line_no = token->line_no;
    enum_decl->column_no = token->column_no;
    if (token->klass == TK_BIT) {
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        if (token->klass == TK_INTEGER_LITERAL) {
          enum_decl->enumDeclaration.type_size = parse_integer();
          if (token->klass == TK_ANGLE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: an integer was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    if (token_is_name(token)) {
      name = parse_name();
      scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
      enum_decl->enumDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return enum_decl;
  } else error("%s:%d:%d: error: `enum` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_errorDeclaration()
{
  Ast* error_decl;

  if (token->klass == TK_ERROR) {
    next_token();
    error_decl = arena_malloc(storage, sizeof(Ast));
    error_decl->kind = AST_errorDeclaration;
    error_decl->line_no = token->line_no;
    error_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        if (token_is_name(token)) {
          error_decl->errorDeclaration.fields = parse_identifierList();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return error_decl;
  } else error("%s:%d:%d: error: `error` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_matchKindDeclaration()
{
  Ast* match_decl;

  if (token->klass == TK_MATCH_KIND) {
    next_token();
    match_decl = arena_malloc(storage, sizeof(Ast));
    match_decl->kind = AST_matchKindDeclaration;
    match_decl->line_no = token->line_no;
    match_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        match_decl->matchKindDeclaration.fields = parse_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return match_decl;
  } else error("%s:%d:%d: error: `match_kind` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_identifierList()
{
  Ast* ids, *ast;

  ids = arena_malloc(storage, sizeof(Ast));
  ids->kind = AST_identifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token_is_name(token)) {
    ast = parse_name();
    ids->identifierList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_name();
      ast = ast->right_sibling;
    }
  }
  return ids;
}

static Ast*
parse_specifiedIdentifierList()
{
  Ast* ids, *ast;

  ids = arena_malloc(storage, sizeof(Ast));
  ids->kind = AST_specifiedIdentifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token_is_specifiedIdentifier(token)) {
    ast = parse_specifiedIdentifier();
    ids->specifiedIdentifierList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_specifiedIdentifier();
      ast = ast->right_sibling;
    }
  }
  return ids;
}

static Ast*
parse_specifiedIdentifier()
{
  Ast* id;

  if (token_is_specifiedIdentifier(token)) {
    id = arena_malloc(storage, sizeof(Ast));
    id->kind = AST_specifiedIdentifier;
    id->line_no = token->line_no;
    id->column_no = token->column_no;
    id->specifiedIdentifier.name = parse_name();
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->specifiedIdentifier.init_expr = parse_expression(1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    return id;
  } else error("%s:%d:%d: error: name was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typedefDeclaration()
{
  Ast* type_decl;
  Ast* name;

  if (token->klass == TK_TYPEDEF) {
    next_token();
    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      type_decl = arena_malloc(storage, sizeof(Ast));
      type_decl->kind = AST_typedefDeclaration;
      if (token_is_typeRef(token)) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        type_decl->line_no = token->line_no;
        type_decl->column_no = token->column_no;
        name = parse_name();
        scope_bind(current_scope, storage, name->name.strname, NAMESPACE_TYPE);
        type_decl->typedefDeclaration.name = name;
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: name was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return type_decl;
    } else error("%s:%d:%d: error: type was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
  } else error("%s:%d:%d: error: type definition was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** STATEMENTS **/

static Ast*
parse_assignmentOrMethodCallStatement()
{
  Ast* lvalue, *stmt; 

  if (token_is_lvalue(token)) {
    lvalue = parse_lvalue();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      stmt = arena_malloc(storage, sizeof(Ast));
      stmt->kind = AST_functionCall;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      stmt->functionCall.lhs_expr = lvalue;
      stmt->functionCall.args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return stmt;
    } else if (token->klass == TK_EQUAL) {
      next_token();
      stmt = arena_malloc(storage, sizeof(Ast));
      stmt->kind = AST_assignmentStatement;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      stmt->assignmentStatement.lhs_expr = lvalue;
      stmt->assignmentStatement.rhs_expr = parse_expression(1);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return stmt;
    } else error("%s:%d:%d: error: assignment or function call was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_returnStatement()
{
  Ast* return_stmt;

  if (token->klass == TK_RETURN) {
    next_token();
    return_stmt = arena_malloc(storage, sizeof(Ast));
    return_stmt->kind = AST_returnStatement;
    return_stmt->line_no = token->line_no;
    return_stmt->column_no = token->column_no;
    if (token_is_expression(token))
      return_stmt->returnStatement.expr = parse_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return return_stmt;
  } else error("%s:%d:%d: error: `return` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_exitStatement()
{
  Ast* exit_stmt;

  if (token->klass == TK_EXIT) {
    next_token();
    exit_stmt = arena_malloc(storage, sizeof(Ast));
    exit_stmt->kind = AST_exitStatement;
    exit_stmt->line_no = token->line_no;
    exit_stmt->column_no = token->column_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return exit_stmt;
  } else error("%s:%d:%d: error: `exit` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_conditionalStatement()
{
  Ast* if_stmt;

  if (token->klass == TK_IF) {
    next_token();
    if_stmt = arena_malloc(storage, sizeof(Ast));
    if_stmt->kind = AST_conditionalStatement;
    if_stmt->line_no = token->line_no;
    if_stmt->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(token)) {
        if_stmt->conditionalStatement.cond_expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token_is_statement(token)) {
            if_stmt->conditionalStatement.stmt = parse_statement(0);
            if (token->klass == TK_ELSE) {
              next_token();
              if (token_is_statement(token)) {
                if_stmt->conditionalStatement.else_stmt = parse_statement(0);
              } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                           source_file, token->line_no, token->column_no, token->lexeme);
            }
          } else error("%s:%d:%d: error: statement was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return if_stmt;
  } else error("%s:%d:%d: error: `if` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_directApplication(Ast* type_name)
{
  Ast* apply_stmt;

  if (token_is_typeName(token) || type_name) {
    apply_stmt = arena_malloc(storage, sizeof(Ast));
    apply_stmt->kind = AST_directApplication;
    apply_stmt->line_no = token->line_no;
    apply_stmt->column_no = token->column_no;
    apply_stmt->directApplication.name = type_name ? type_name : parse_typeName();
    if (token->klass == TK_DOT) {
      next_token();
      if (token->klass == TK_APPLY) {
        next_token();
        if (token->klass == TK_PARENTH_OPEN) {
          next_token();
          apply_stmt->directApplication.args = parse_argumentList();
          if (token->klass == TK_PARENTH_CLOSE) {
            next_token();
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                         source_file, token->line_no, token->column_no, token->lexeme);
          } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `apply` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `.` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return apply_stmt;
  } else error("%s:%d:%d: error: type name was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statement(Ast* type_name)
{
  Ast* stmt, *empty_stmt;

  if (token_is_statement(token)) {
    stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_statement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token_is_typeName(token) || type_name) {
      stmt->statement.stmt = parse_directApplication(type_name);
      return stmt;
    } else if (token_is_assignmentOrMethodCallStatement(token)) {
      stmt->statement.stmt = parse_assignmentOrMethodCallStatement();
      return stmt;
    } else if (token->klass == TK_IF) {
      stmt->statement.stmt = parse_conditionalStatement();
      return stmt;
    } else if (token->klass == TK_SEMICOLON) {
      next_token();
      empty_stmt = arena_malloc(storage, sizeof(Ast));
      empty_stmt->kind = AST_emptyStatement;
      empty_stmt->line_no = token->line_no;
      empty_stmt->column_no = token->column_no;
      stmt->statement.stmt = empty_stmt;
      return stmt;
    } else if (token->klass == TK_BRACE_OPEN) {
      stmt->statement.stmt = parse_blockStatement();
      return stmt;
    } else if (token->klass == TK_EXIT) {
      stmt->statement.stmt = parse_exitStatement();
      return stmt;
    } else if (token->klass == TK_RETURN) {
      stmt->statement.stmt = parse_returnStatement();
      return stmt;
    } else if (token->klass == TK_SWITCH) {
      stmt->statement.stmt = parse_switchStatement();
      return stmt;
    }
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_blockStatement()
{
  Ast* block_stmt;

  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    block_stmt = arena_malloc(storage, sizeof(Ast));
    block_stmt->kind = AST_blockStatement;
    block_stmt->line_no = token->line_no;
    block_stmt->column_no = token->column_no;
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return block_stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statementOrDeclList()
{
  Ast* stmts, *ast;

  stmts = arena_malloc(storage, sizeof(Ast));
  stmts->kind = AST_statementOrDeclList;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token_is_statementOrDeclaration(token)) {
    ast = parse_statementOrDeclaration();
    stmts->statementOrDeclList.first_child = ast;
    while (token_is_statementOrDeclaration(token)) {
      ast->right_sibling = parse_statementOrDeclaration();
      ast = ast->right_sibling;
    }
  }
  return stmts;
}

static Ast*
parse_switchStatement()
{
  Ast* stmt;

  if (token->klass == TK_SWITCH) {
    next_token();
    stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_switchStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      stmt->switchStatement.expr = parse_expression(1);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          stmt->switchStatement.switch_cases = parse_switchCases();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `switch` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_switchCases()
{
  Ast* cases, *ast;

  cases = arena_malloc(storage, sizeof(Ast));
  cases->kind = AST_switchCases;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token_is_switchLabel(token)) {
    ast = parse_switchCase();
    cases->switchCases.first_child = ast;
    while (token_is_switchLabel(token)) {
      ast->right_sibling = parse_switchCase();
      ast = ast->right_sibling;
    }
  }
  return cases;
}

static Ast*
parse_switchCase()
{
  Ast* switch_case;

  if (token_is_switchLabel(token)) {
    switch_case = arena_malloc(storage, sizeof(Ast));
    switch_case->kind = AST_switchCase;
    switch_case->line_no = token->line_no;
    switch_case->column_no = token->column_no;
    switch_case->switchCase.label = parse_switchLabel();
    if (token->klass == TK_COLON) {
      next_token();
      if (token->klass == TK_BRACE_OPEN) {
        switch_case->switchCase.stmt = parse_blockStatement();
      }
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return switch_case;
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_switchLabel()
{
  Ast* switch_label, *default_label;

  if (token_is_switchLabel(token)) {
    switch_label = arena_malloc(storage, sizeof(Ast));
    switch_label->kind = AST_switchLabel;
    switch_label->line_no = token->line_no;
    switch_label->column_no = token->column_no;
    if (token_is_name(token)) {
      switch_label->switchLabel.label = parse_name();
      return switch_label;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      default_label = arena_malloc(storage, sizeof(Ast));
      default_label->kind = AST_default;
      default_label->line_no = token->line_no;
      default_label->column_no = token->column_no;
      switch_label->switchLabel.label = default_label;
      return switch_label;
    } else assert(0);
  } else error("%s:%d:%d: error: switch label was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statementOrDeclaration()
{
  Ast* stmt, *type_ref;

  if (token_is_statementOrDeclaration(token)) {
    stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_statementOrDeclaration;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        stmt->statementOrDeclaration.stmt = parse_instantiation(type_ref);
        return stmt;
      } else if (token_is_name(token)) {
        stmt->statementOrDeclaration.stmt = parse_variableDeclaration(type_ref);
        return stmt;
      } else {
        stmt->statementOrDeclaration.stmt = parse_statement(type_ref);
        return stmt;
      }
    } else if (token_is_statement(token)) {
      stmt->statementOrDeclaration.stmt = parse_statement(0);
      return stmt;
    } else if (token->klass == TK_CONST) {
      stmt->statementOrDeclaration.stmt = parse_variableDeclaration(0);
      return stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

/** TABLES **/ 

static Ast*
parse_tableDeclaration()
{
  Ast* table;

  if (token->klass == TK_TABLE) {
    next_token();
    table = arena_malloc(storage, sizeof(Ast));
    table->kind = AST_tableDeclaration;
    table->line_no = token->line_no;
    table->column_no = token->column_no;
    table->tableDeclaration.name = parse_name();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_tableProperty(token)) {
        table->tableDeclaration.prop_list = parse_tablePropertyList();
      } else error("%s:%d:%d: error: table property was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return table;
  } else error("%s:%d:%d: error: `table` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tablePropertyList()
{
  Ast* props, *ast;

  props = arena_malloc(storage, sizeof(Ast));
  props->kind = AST_tablePropertyList;
  props->line_no = token->line_no;
  props->column_no = token->column_no;
  if (token_is_tableProperty(token)) {
    ast = parse_tableProperty();
    props->tablePropertyList.first_child = ast;
    while (token_is_tableProperty(token)) {
      ast->right_sibling = parse_tableProperty();
      ast = ast->right_sibling;
    }
  }
  return props;
}

static Ast*
parse_tableProperty()
{
  bool is_const = false;
  Ast* table_prop, *prop;

  if (token_is_tableProperty(token)) {
    if (token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    table_prop = arena_malloc(storage, sizeof(Ast));
    table_prop->kind = AST_tableProperty;
    table_prop->line_no = token->line_no;
    table_prop->column_no = token->column_no;
    if (token->klass == TK_KEY) {
      next_token();
      prop = arena_malloc(storage, sizeof(Ast));
      prop->kind = AST_keyProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          prop->keyProperty.keyelem_list = parse_keyElementList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      prop = arena_malloc(storage, sizeof(Ast));
      prop->kind = AST_actionsProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          prop->actionsProperty.action_list = parse_actionList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      prop = arena_malloc(storage, sizeof(Ast));
      prop->kind = AST_entriesProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            prop->entriesProperty.entries_list = parse_entriesList();
          } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (token_is_nonTableKwName(token)) {
      prop = arena_malloc(storage, sizeof(Ast));
      prop->kind = AST_simpleProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      prop->simpleProperty.is_const = is_const;
      prop->simpleProperty.name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        prop->simpleProperty.init_expr = parse_expression(1);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else assert(0);
  } else error("%s:%d:%d: error: table property was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_keyElementList()
{
  Ast* elems, *ast;

  elems = arena_malloc(storage, sizeof(Ast));
  elems->kind = AST_keyElementList;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token_is_expression(token)) {
    ast = parse_keyElement();
    elems->keyElementList.first_child = ast;
    while (token_is_expression(token)) {
      ast->right_sibling = parse_keyElement();
      ast = ast->right_sibling;
    }
  }
  return elems;
}

static Ast*
parse_keyElement()
{
  Ast* key_elem;

  if (token_is_expression(token)) {
    key_elem = arena_malloc(storage, sizeof(Ast));
    key_elem->kind = AST_keyElement;
    key_elem->line_no = token->line_no;
    key_elem->column_no = token->column_no;
    key_elem->keyElement.expr = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->keyElement.match = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return key_elem;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_actionList()
{
  Ast* actions, *ast;

  actions = arena_malloc(storage, sizeof(Ast));
  actions->kind = AST_actionList;
  actions->line_no = token->line_no;
  actions->column_no = token->column_no;
  if (token_is_actionRef(token)) {
    ast = parse_actionRef();
    actions->actionList.first_child = ast;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    while (token_is_actionRef(token)) {
      ast->right_sibling = parse_actionRef();
      ast = ast->right_sibling;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
  }
  return actions;
}

static Ast*
parse_actionRef()
{
  Ast* action_ref;

  if (token_is_nonTypeName(token)) {
    action_ref = arena_malloc(storage, sizeof(Ast));
    action_ref->kind = AST_actionRef;
    action_ref->line_no = token->line_no;
    action_ref->column_no = token->column_no;
    action_ref->actionRef.name = parse_nonTypeName(token);
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_argument(token)) {
        action_ref->actionRef.args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    return action_ref;
  } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_entriesList()
{
  Ast* entries, *ast;

  entries = arena_malloc(storage, sizeof(Ast));
  entries->kind = AST_entriesList;
  entries->line_no = token->line_no;
  entries->column_no = token->column_no;
  if (token_is_keysetExpression(token)) {
    ast = parse_entry();
    entries->entriesList.first_child = ast;
    while (token_is_keysetExpression(token)) {
      ast->right_sibling = parse_entry();
      ast = ast->right_sibling;
    }
  }
  return entries;
}

static Ast*
parse_entry()
{
  Ast* entry;

  if (token_is_keysetExpression(token)) {
    entry = arena_malloc(storage, sizeof(Ast));
    entry->kind = AST_entry;
    entry->line_no = token->line_no;
    entry->column_no = token->column_no;
    entry->entry.keyset = parse_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      entry->entry.action = parse_actionRef();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: `:` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return entry;
  } else error("%s:%d:%d: error: keyset was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_actionDeclaration()
{
  Ast* action_decl;

  if (token->klass == TK_ACTION) {
    next_token();
    action_decl = arena_malloc(storage, sizeof(Ast));
    action_decl->kind = AST_actionDeclaration;
    action_decl->line_no = token->line_no;
    action_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      action_decl->actionDeclaration.name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        action_decl->actionDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TK_BRACE_OPEN) {
            action_decl->actionDeclaration.stmt = parse_blockStatement();
          } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return action_decl;
  } else error("%s:%d:%d: error: `action` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** VARIABLES **/

static Ast*
parse_variableDeclaration(Ast* type_ref)
{
  bool is_const = false;
  Ast* var_decl;

  if (token->klass == TK_CONST) {
    next_token();
    is_const = true;
  }
  if (token_is_typeRef(token) || type_ref) {
    var_decl = arena_malloc(storage, sizeof(Ast));
    var_decl->kind = AST_variableDeclaration;
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef();
    if (token_is_name(token)) {
      var_decl->line_no = token->line_no;
      var_decl->column_no = token->column_no;
      var_decl->variableDeclaration.name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        var_decl->variableDeclaration.init_expr = parse_expression(1);
      }
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    } else error("%s:%d:%d: error: name was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    var_decl->variableDeclaration.is_const = is_const;
    return var_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** EXPRESSIONS **/

static Ast*
parse_functionDeclaration(Ast* type_ref)
{
  Ast* func_decl;

  if (token_is_typeOrVoid(token)) {
    func_decl = arena_malloc(storage, sizeof(Ast));
    func_decl->kind = AST_functionDeclaration;
    func_decl->line_no = token->line_no;
    func_decl->column_no = token->column_no;
    func_decl->functionDeclaration.proto = parse_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      func_decl->functionDeclaration.stmt = parse_blockStatement();
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return func_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_argumentList()
{
  Ast* args, *ast;

  args = arena_malloc(storage, sizeof(Ast));
  args->kind = AST_argumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token_is_argument(token)) {
    ast = parse_argument();
    args->argumentList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_argument();
      ast = ast->right_sibling;
    }
  }
  return args;
}

static Ast*
parse_argument()
{
  Ast* arg, *dontcare_arg;

  if (token_is_argument(token)) {
    arg = arena_malloc(storage, sizeof(Ast));
    arg->kind = AST_argument;
    arg->line_no = token->line_no;
    arg->column_no = token->column_no;
    if (token_is_expression(token)) {
      arg->argument.arg = parse_expression(1);
      return arg;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      arg->argument.arg = dontcare_arg;
      return arg;
    } else assert(0);
  } else error("%s:%d:%d: error: an argument was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_expressionList()
{
  Ast* exprs = arena_malloc(storage, sizeof(Ast));
  exprs->kind = AST_expressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  if (token_is_expression(token)) {
    Ast* ast = parse_expression(1);
    exprs->expressionList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_expression(1);
      ast = ast->right_sibling;
    }
  }
  return exprs;
}

static Ast*
parse_lvalue()
{
  Ast* lvalue, *expr;

  if (token_is_lvalue(token)) {
    lvalue = arena_malloc(storage, sizeof(Ast));
    lvalue->kind = AST_lvalueExpression;
    lvalue->line_no = token->line_no;
    lvalue->column_no = token->column_no;
    lvalue->lvalueExpression.expr = parse_nonTypeName(token);
    while(token->klass == TK_DOT || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOT) {
        next_token();
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_memberSelector;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->memberSelector.lhs_expr = lvalue;
        if (token_is_name(token)) {
          expr->memberSelector.name = parse_name();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = arena_malloc(storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_arraySubscript;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->arraySubscript.lhs_expr = lvalue;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = arena_malloc(storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
    }
    return lvalue;
  } else error("%s:%d:%d: error: lvalue was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_expression(int priority_threshold)
{
  Ast* primary, *expr;

  if (token_is_expression(token)) {
    primary = parse_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast* expr;
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_memberSelector;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->memberSelector.lhs_expr = primary;
        if (token_is_nonTypeName(token)) {
          expr->memberSelector.name = parse_nonTypeName();
        } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = arena_malloc(storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = token->line_no;
        primary->column_no = token->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_arraySubscript;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->arraySubscript.lhs_expr = primary;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = arena_malloc(storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = token->line_no;
        primary->column_no = token->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_functionCall;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->functionCall.lhs_expr = primary;
        expr->functionCall.args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = arena_malloc(storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = token->line_no;
        primary->column_no = token->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TK_EQUAL) {
        next_token();
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_assignmentStatement;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->assignmentStatement.lhs_expr = primary;
        expr->assignmentStatement.rhs_expr = parse_expression(1);
        primary = arena_malloc(storage, sizeof(Ast));
        primary->kind = AST_expression;
        primary->line_no = token->line_no;
        primary->column_no = token->column_no;
        primary->expression.expr = expr;
      } else if (token_is_binaryOperator(token)){
        int priority = operator_priority(token);
        if (priority >= priority_threshold) {
          expr = arena_malloc(storage, sizeof(Ast));
          expr->kind = AST_binaryExpression;
          expr->line_no = token->line_no;
          expr->column_no = token->column_no;
          expr->binaryExpression.left_operand = primary;
          expr->binaryExpression.op = token_to_binop(token);
          next_token();
          expr->binaryExpression.right_operand = parse_expression(priority + 1);
          primary = arena_malloc(storage, sizeof(Ast));
          primary->kind = AST_expression;
          primary->line_no = token->line_no;
          primary->column_no = token->column_no;
          primary->expression.expr = expr;
        } else break;
      } else assert(0);
    }
    return primary;
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_expressionPrimary()
{
  Ast* primary, *expr;

  if (token_is_expression(token)) {
    primary = arena_malloc(storage, sizeof(Ast));
    primary->kind = AST_expression;
    primary->line_no = token->line_no;
    primary->column_no = token->column_no;
    if (token->klass == TK_INTEGER_LITERAL) {
      primary->expression.expr = parse_integer();
      return primary;
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      primary->expression.expr = parse_boolean();
      return primary;
    } else if (token->klass == TK_STRING_LITERAL) {
      primary->expression.expr = parse_string();
      return primary;
    } else if (token->klass == TK_DOT) {
      next_token();
      if (token->klass == TK_IDENTIFIER) {
        primary->expression.expr = parse_nonTypeName();
        return primary;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        primary->expression.expr = parse_typeName();
        return primary;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(token)) {
      primary->expression.expr = parse_nonTypeName();
      return primary;
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      primary->expression.expr = parse_expressionList();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return primary;
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_DOT) {
        /* (<typeName>.<name>) */
        primary->expression.expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else if (token_is_typeRef(token)) {
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_castExpression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->castExpression.type = parse_typeRef();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          expr->castExpression.expr = parse_expression(10);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary->expression.expr = expr;
        return primary;
      } else if (token_is_expression(token)) {
        primary->expression.expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      expr = arena_malloc(storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = OP_NOT;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TK_TILDA) {
      next_token();
      expr = arena_malloc(storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = OP_BITW_NOT;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      expr = arena_malloc(storage, sizeof(Ast));
      expr->kind = AST_unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = OP_NEG;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token_is_typeName(token)) {
      primary->expression.expr = parse_typeName();
      return primary;
    } else if (token->klass == TK_ERROR) {
      next_token();
      expr = arena_malloc(storage, sizeof(Ast));
      expr->kind = AST_name;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->name.strname = "error";
      primary->expression.expr = expr;
      return primary;
    } else assert(0);
    assert(0);
  } else error("%s:%d:%d: error: expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_indexExpression()
{
  Ast* index_expr;

  if (token_is_expression(token)) {
    index_expr = arena_malloc(storage, sizeof(Ast));
    index_expr->kind = AST_indexExpression;
    index_expr->line_no = token->line_no;
    index_expr->column_no = token->column_no;
    index_expr->indexExpression.start_index = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_expression(token)) {
        index_expr->indexExpression.end_index = parse_expression(1);
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    return index_expr;
  } else error("%s:%d:%d: expression or `:` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_integer()
{
  Ast* int_literal;

  if (token->klass == TK_INTEGER_LITERAL) {
    int_literal = arena_malloc(storage, sizeof(Ast));
    int_literal->kind = AST_integerLiteral;
    int_literal->line_no = token->line_no;
    int_literal->column_no = token->column_no;
    int_literal->integerLiteral.is_signed = token->integer.is_signed;
    int_literal->integerLiteral.width = token->integer.width;
    int_literal->integerLiteral.value = token->integer.value;
    next_token();
    return int_literal;
  } else error("%s:%d:%d: error: integer was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_boolean()
{
  Ast* bool_literal;

  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    bool_literal = arena_malloc(storage, sizeof(Ast));
    bool_literal->kind = AST_booleanLiteral;
    bool_literal->line_no = token->line_no;
    bool_literal->column_no = token->column_no;
    bool_literal->booleanLiteral.value = (token->klass == TK_TRUE);
    next_token();
    return bool_literal;
  } else error("%s:%d:%d: error: boolean was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_string()
{
  Ast* string_literal;

  if (token->klass == TK_STRING_LITERAL) {
    string_literal = arena_malloc(storage, sizeof(Ast));
    string_literal->kind = AST_stringLiteral;
    string_literal->line_no = token->line_no;
    string_literal->column_no = token->column_no;
    string_literal->stringLiteral.value = token->lexeme;
    next_token();
    return string_literal;
  } else error("%s:%d:%d: error: string was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}
