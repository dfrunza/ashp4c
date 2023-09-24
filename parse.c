#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

static Arena  *storage;
static UnboundedArray* tokens;
static int    token_at = 0;
static Token* token = 0;
static int    prev_token_at = 0;
static Token* prev_token = 0;
static Scope* current_scope;
static Scope* root_scope;
static const int MAXLEN_ANONTYPE = 16;  /* type@9999:9999 */

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
static Ast* parse_prefixedType();
static Ast* parse_tupleType();
static Ast* parse_headerStackType(Ast* named_type);
static Ast* parse_specializedType(Ast* named_type);
static Ast* parse_baseType();
static Ast* parse_integerTypeSize();
static Ast* parse_typeOrVoid();
static Ast* parse_optTypeParameters();
static Ast* parse_typeParameterList();
static Ast* parse_realTypeArg();
static Ast* parse_typeArg();
static Ast* parse_realTypeArgumentList();
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
static Ast* parse_prefixedNonTypeName();
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
  prev_token = token;
  prev_token_at = token_at;
  token = array_get(tokens, ++token_at, sizeof(Token));
  while (token->klass == TK_COMMENT) {
    token = array_get(tokens, ++token_at, sizeof(Token));
  }
  if (token->klass == TK_IDENTIFIER) {
    NameEntry* name_entry = scope_lookup_any(current_scope, token->lexeme);
    if (name_entry) {
      if (name_entry->ns[NS_KEYWORD]) {
        NameDecl* namedecl = name_entry->ns[NS_KEYWORD];
        token->klass = namedecl->token_class;
        return token;
      } else if (name_entry->ns[NS_TYPE]) {
        token->klass = TK_TYPE_IDENTIFIER;
        return token;
      }
    }
  }
  return token;
}

static Token*
peek_token()
{
  prev_token = token;
  prev_token_at = token_at;
  Token* peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

static bool
token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES || token->klass == TK_TYPE;
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
  return token->klass == TK_DOT || token->klass == TK_TYPE_IDENTIFIER;
}

static bool
token_is_prefixedType(Token* token)
{
  return token->klass == TK_DOT || token->klass == TK_TYPE_IDENTIFIER;
}

static bool
token_is_prefixedNonTypeName(Token* token) {
  return token->klass == TK_DOT || token_is_nonTypeName(token);
}

static bool
token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE || token->klass == TK_TYPE;
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
  bool result = token_is_baseType(token) || token_is_prefixedType(token) || token->klass == TK_TUPLE;
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
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF || token->klass == TK_TYPE
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
token_is_typeParameterList(Token* token)
{
  return token_is_name(token);
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
  bool result = token->klass == TK_DOT || token_is_nonTypeName(token)
    || token->klass == TK_PARENTH_OPEN;
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
    || token->klass == TK_STRING_LITERAL || token->klass == TK_DOT || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token_is_prefixedType(token);
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
    || token_is_typeOrVoid(token) || token->klass == TK_DOT;
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
      || token->klass == TK_ANGLE_OPEN /* Less */ || token->klass == TK_ANGLE_CLOSE /* Greater */
      || token->klass == TK_ANGLE_OPEN_EQUAL /* Less-equal */ || token->klass == TK_ANGLE_CLOSE_EQUAL /* Greater-equal */) {
    /* Relational ops  */
    return 2;
  }
  else if (token->klass == TK_PLUS || token->klass == TK_MINUS
           || token->klass == TK_AMPERSAND || token->klass == TK_PIPE
           || token->klass == TK_CIRCUMFLEX || token->klass == TK_DOUBLE_ANGLE_OPEN /* BitshiftLeft */
           || token->klass == TK_DOUBLE_ANGLE_CLOSE /* BitshiftRight */) {
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

/** PROGRAM **/

static Ast*
parse_p4program()
{
  Ast* program = arena_malloc(storage, sizeof(Ast));
  program->kind = AST_p4program;
  program->line_no = token->line_no;
  program->column_no = token->column_no;
  while (token->klass == TK_SEMICOLON) {
    next_token(); /* empty declaration */
  }
  Scope* scope = scope_create(storage, 16, 1008);
  current_scope = scope_push(scope, current_scope);
  program->p4program.decl_list = parse_declarationList();
  current_scope = scope_pop(current_scope);
  if (token->klass != TK_END_OF_INPUT) {
    error("At line %d, column %d: unexpected token `%s`.",
          token->line_no, token->column_no, token->lexeme);
  }
  return program;
}

static Ast*
parse_declarationList()
{
  Ast* decls = arena_malloc(storage, sizeof(Ast));
  decls->kind = AST_declarationList;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token_is_declaration(token)) {
    Ast* ast = parse_declaration();
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
  if (token_is_declaration(token)) {
    Ast* decl = arena_malloc(storage, sizeof(Ast));
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
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(token)) {
      decl->declaration.decl = parse_functionDeclaration(parse_typeRef());
      return decl;
    } else assert(0);
  } else error("At line %d, column %d: top-level declaration as expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_nonTypeName()
{
  if (token_is_nonTypeName(token)) {
    Ast* name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = token->line_no;
    name->column_no = token->column_no;
    name->name.strname = token->lexeme;
    next_token();
    return name;
  } else error("At line %d, column %d: non-type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_name()
{
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      return parse_nonTypeName();
    } else if (token->klass == TK_TYPE_IDENTIFIER) {
      Ast* type_name = arena_malloc(storage, sizeof(Ast));
      type_name->kind = AST_name;
      type_name->line_no = token->line_no;
      type_name->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      next_token();
      return type_name;
    } else assert(0);
  } else error("At line %d, column %d: name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parameterList()
{
  Ast* params = arena_malloc(storage, sizeof(Ast));
  params->kind = AST_parameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  if (token_is_parameter(token)) {
    Ast* ast = parse_parameter();
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
  if (token_is_parameter(token)) {
    Ast* param = arena_malloc(storage, sizeof(Ast));
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
        } else error("At line %d, column %d: expression was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return param;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
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
  if (token->klass == TK_PACKAGE) {
    next_token();
    Ast* package_decl = arena_malloc(storage, sizeof(Ast));
    package_decl->kind = AST_packageTypeDeclaration;
    package_decl->line_no = token->line_no;
    package_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      package_decl->packageTypeDeclaration.name = name;
      package_decl->packageTypeDeclaration.type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        package_decl->packageTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return package_decl;
  } else error("At line %d, column %d: `package` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_instantiation(Ast* type_ref)
{
  if (token_is_typeRef(token) || type_ref) {
    Ast* inst_stmt = arena_malloc(storage, sizeof(Ast));
    inst_stmt->kind = AST_instantiation;
    inst_stmt->line_no = token->line_no;
    inst_stmt->column_no = token->column_no;
    inst_stmt->instantiation.type_ref = type_ref ? type_ref : parse_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      inst_stmt->instantiation.args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          inst_stmt->instantiation.name = parse_name();
          if (token->klass == TK_SEMICOLON) {
            next_token();
          } else error("At line %d, column %d: `;` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: instance name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `(` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return inst_stmt;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** PARSER **/

static Ast*
parse_optConstructorParameters()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    if (token_is_parameter(token)) {
      Ast* params = parse_parameterList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return params;
    } else if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `)` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  }
  return 0;
}

static Ast*
parse_parserDeclaration(Ast* parser_proto)
{
  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    Ast* parser_decl = arena_malloc(storage, sizeof(Ast));
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
      } else error("At line %d, column %d: `state` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    return parser_decl;
  } else error("At line %d, column %d: `parser` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserLocalElements()
{
  Ast* elems = arena_malloc(storage, sizeof(Ast));
  elems->kind = AST_parserLocalElements;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token_is_parserLocalElement(token)) {
    Ast* ast = parse_parserLocalElement();
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
  if (token_is_parserLocalElement(token)) {
    Ast* local_element = arena_malloc(storage, sizeof(Ast));
    local_element->kind = AST_parserLocalElement;
    local_element->line_no = token->line_no;
    local_element->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_element->parserLocalElement.element = parse_variableDeclaration(0);
      return local_element;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_element->parserLocalElement.element = parse_instantiation(type_ref);
        return local_element;
      } else if (token_is_name(token)) {
        local_element->parserLocalElement.element = parse_variableDeclaration(type_ref);
        return local_element;
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("At line %d, column %d: local declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserTypeDeclaration()
{
  if (token->klass == TK_PARSER) {
    next_token();
    Ast* parser_proto = arena_malloc(storage, sizeof(Ast));
    parser_proto->kind = AST_parserTypeDeclaration;
    parser_proto->line_no = token->line_no; 
    parser_proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      parser_proto->parserTypeDeclaration.name = name;
      parser_proto->parserTypeDeclaration.type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        parser_proto->parserTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return parser_proto;
  } else error("At line %d, column %d: `parser` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserStates()
{
  Ast* states = arena_malloc(storage, sizeof(Ast));
  states->kind = AST_parserStates;
  states->line_no = token->line_no;
  states->column_no = token->column_no;
  if (token->klass == TK_STATE) {
    Ast* ast = parse_parserState();
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
  if (token->klass == TK_STATE) {
    next_token();
    Ast* state = arena_malloc(storage, sizeof(Ast));
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
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return state;
  } else error("At line %d, column %d: `state` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserStatements()
{
  Ast* stmts = arena_malloc(storage, sizeof(Ast));
  stmts->kind = AST_parserStatements;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token_is_parserStatement(token)) {
    Ast* ast = parse_parserStatement();
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
  if (token_is_parserStatement(token)) {
    Ast* parser_stmt = arena_malloc(storage, sizeof(Ast));
    parser_stmt->kind = AST_parserStatement;
    parser_stmt->line_no = token->line_no;
    parser_stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
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
  } else error("At line %d, column %d: statement was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_parserBlockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast* stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_parserBlockStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    stmt->parserBlockStatement.stmt_list = parse_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `}` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return stmt;
  } else error("At line %d, column %d: `{` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_transitionStatement()
{
  if (token->klass == TK_TRANSITION) {
    next_token();
    Ast* transition = arena_malloc(storage, sizeof(Ast));
    transition->kind = AST_transitionStatement;
    transition->line_no = token->line_no;
    transition->column_no = token->column_no;
    transition->transitionStatement.stmt = parse_stateExpression();
    return transition;
  } else error("At line %d, column %d: `transition` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_stateExpression()
{
  if (token_is_name(token) || token->klass == TK_SELECT) {
    Ast* state_expr = arena_malloc(storage, sizeof(Ast));
    state_expr->kind = AST_stateExpression;
    state_expr->line_no = token->line_no;
    state_expr->column_no = token->column_no;
    if (token_is_name(token)) {
      state_expr->stateExpression.expr = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
      return state_expr;
    } else if (token->klass == TK_SELECT) {
      state_expr->stateExpression.expr = parse_selectExpression();
      return state_expr;
    } else assert(0);
  } else error("At line %d, column %d: state expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_selectExpression()
{
  if (token->klass == TK_SELECT) {
    next_token();
    Ast* select_expr = arena_malloc(storage, sizeof(Ast));
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
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `(` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return select_expr;
  } else error("At line %d, column %d: `select` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_selectCaseList()
{
  Ast* cases = arena_malloc(storage, sizeof(Ast));
  cases->kind = AST_selectCaseList;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token_is_selectCase(token)) {
    Ast* ast = parse_selectCase();
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
  if (token_is_keysetExpression(token)) {
    Ast* select_case = arena_malloc(storage, sizeof(Ast));
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
        } else error("At line %d, column %d: `;` expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return select_case;
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_keysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN || token_is_simpleKeysetExpression(token)) {
    Ast* keyset_expr = arena_malloc(storage, sizeof(Ast));
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
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tupleKeysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    Ast* tuple_keyset = arena_malloc(storage, sizeof(Ast));
    tuple_keyset->kind = AST_tupleKeysetExpression;
    tuple_keyset->line_no = token->line_no;
    tuple_keyset->column_no = token->column_no;
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `)` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return tuple_keyset;
  } else error("At line %d, column %d: `(` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_simpleExpressionList()
{
  Ast* exprs = arena_malloc(storage, sizeof(Ast));
  exprs->kind = AST_simpleExpressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  if (token_is_expression(token)) {
    Ast* ast = parse_simpleKeysetExpression();
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
  if (token_is_simpleKeysetExpression(token)) {
    Ast* simple_keyset = arena_malloc(storage, sizeof(Ast));
    simple_keyset->kind = AST_simpleKeysetExpression;
    simple_keyset->line_no = token->line_no;
    simple_keyset->column_no = token->column_no;
    if (token_is_expression(token)) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(1);
      return simple_keyset;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast* default_keyset = arena_malloc(storage, sizeof(Ast));
      default_keyset->kind = AST_default;
      default_keyset->line_no = token->line_no;
      default_keyset->column_no = token->column_no;
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_keyset = arena_malloc(storage, sizeof(Ast));
      dontcare_keyset->kind = AST_dontcare;
      dontcare_keyset->line_no = token->line_no;
      dontcare_keyset->column_no = token->column_no;
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = dontcare_keyset->line_no;
      name->column_no = dontcare_keyset->column_no;
      name->name.strname = "_";
      dontcare_keyset->dontcare.name = name;
      simple_keyset->simpleKeysetExpression.expr = dontcare_keyset;
      return simple_keyset;
    }
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** CONTROL **/

static Ast*
parse_controlDeclaration(Ast* control_proto)
{
  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    Ast* control_decl = arena_malloc(storage, sizeof(Ast));
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
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                      token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `apply` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    return control_decl;
  } else error("At line %d, column %d: `control` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlTypeDeclaration()
{
  if (token->klass == TK_CONTROL) {
    next_token();
    Ast* control_proto = arena_malloc(storage, sizeof(Ast));
    control_proto->kind = AST_controlTypeDeclaration;
    control_proto->line_no = token->line_no;
    control_proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      control_proto->controlTypeDeclaration.name = name;
      control_proto->controlTypeDeclaration.type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        control_proto->controlTypeDeclaration.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return control_proto;
  } else error("At line %d, column %d: `control` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlLocalDeclaration()
{
  if (token_is_controlLocalDeclaration(token)) {
    Ast* local_decl = arena_malloc(storage, sizeof(Ast));
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
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(type_ref);
        return local_decl;
      } else if (token_is_name(token)) {
        local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(type_ref);
        return local_decl;
      } else error("At line %d, column %d: unexpected token `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("At line %d, column %d: local declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_controlLocalDeclarations()
{
  Ast* decls = arena_malloc(storage, sizeof(Ast));
  decls->kind = AST_controlLocalDeclarations;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token_is_controlLocalDeclaration(token)) {
    Ast* ast = parse_controlLocalDeclaration();
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
  if (token->klass == TK_EXTERN) {
    next_token();
    Ast* extern_decl = arena_malloc(storage, sizeof(Ast));
    extern_decl->kind = AST_externDeclaration;
    extern_decl->line_no = token->line_no;
    extern_decl->column_no = token->column_no;

    bool is_function_type = false;
    if (token_is_typeOrVoid(token) && token_is_nonTypeName(token)) {
      is_function_type = token_is_typeOrVoid(token) && token_is_name(peek_token());
    } else if (token_is_typeOrVoid(token)) {
      is_function_type = true;
    } else if (token_is_nonTypeName(token)) {
      is_function_type = false;
    } else error("At line %d, column %d: extern declaration was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);

    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return extern_decl;
    } else {
      Ast* extern_type = arena_malloc(storage, sizeof(Ast));
      extern_type->kind = AST_externTypeDeclaration;
      extern_type->line_no = token->line_no;
      extern_type->column_no = token->column_no;
      extern_type->externTypeDeclaration.name = parse_nonTypeName();
      Ast* name = extern_type->externTypeDeclaration.name;
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      extern_type->externTypeDeclaration.type_params = parse_optTypeParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      extern_decl->externDeclaration.decl = extern_type;
      return extern_decl;
    }
  } else error("At line %d, column %d: `extern` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_methodPrototypes()
{
  Ast* protos = arena_malloc(storage, sizeof(Ast));
  protos->kind = AST_methodPrototypes;
  protos->line_no = token->line_no;
  protos->column_no = token->column_no;
  if (token_is_methodPrototype(token)) {
    Ast* ast = parse_methodPrototype();
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
  if (token_is_typeOrVoid(token) || return_type) {
    Ast* func_proto = arena_malloc(storage, sizeof(Ast));
    func_proto->kind = AST_functionPrototype;
    func_proto->line_no = token->line_no;
    func_proto->column_no = token->column_no;
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      Ast* return_type = parse_typeOrVoid();
      if (return_type->kind == AST_name) {
        Ast* name = return_type;
        NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
        namedecl->strname = name->name.strname;
        scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
        Ast* type_ref = arena_malloc(storage, sizeof(Ast));
        type_ref->kind = AST_typeRef;
        type_ref->line_no = token->line_no;
        type_ref->column_no = token->column_no;
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token_is_name(token)) {
      func_proto->functionPrototype.name = parse_name();
      func_proto->functionPrototype.type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: function name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return func_proto;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_methodPrototype()
{
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      Ast* func_proto = arena_malloc(storage, sizeof(Ast));
      func_proto->kind = AST_functionPrototype;
      func_proto->line_no = token->line_no;
      func_proto->column_no = token->column_no;
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` as expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else if (token_is_typeOrVoid(token)) {
      Ast* func_proto = parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else error("At line %d, column %d: type was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** TYPES **/

static Ast*
parse_typeRef()
{
  if (token_is_typeRef(token)) {
    Ast* type_ref = arena_malloc(storage, sizeof(Ast));
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
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_namedType()
{
  if (token_is_typeName(token)) {
    Ast* named_type = parse_prefixedType();
    if (token->klass == TK_ANGLE_OPEN) {
      Ast* specd_type = parse_specializedType(named_type);
      return specd_type;
    } else if (token->klass == TK_BRACKET_OPEN) {
      Ast* stack_type = parse_headerStackType(named_type);
      return stack_type;
    }
    return named_type;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_prefixedType()
{
  bool is_prefixed = false;
  if (token->klass == TK_DOT) {
    next_token();
    is_prefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    Ast* type_name = arena_malloc(storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    type_name->name.strname = token->lexeme;
    type_name->name.is_prefixed = is_prefixed;
    next_token();
    return type_name;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tupleType()
{
  if (token->klass == TK_TUPLE) {
    next_token();
    Ast* name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = token->line_no;
    name->column_no = token->column_no;
    name->name.strname = arena_malloc(storage, MAXLEN_ANONTYPE);
    int lexeme_len = sprintf(name->name.strname, "type@%d:%d", name->line_no, name->column_no);
    assert(lexeme_len <= MAXLEN_ANONTYPE);
    Ast* tuple = arena_malloc(storage, sizeof(Ast));
    tuple->kind = AST_tupleType;
    tuple->line_no = token->line_no;
    tuple->column_no = token->column_no;
    tuple->tupleType.name = name;
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      tuple->tupleType.type_args = parse_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `>` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `<` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return tuple;
  } else error("At line %d, column %d: `tuple` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerStackType(Ast* named_type)
{
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    Ast* name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = named_type->line_no;
    name->column_no = named_type->column_no;
    name->name.strname = arena_malloc(storage, MAXLEN_ANONTYPE);
    int lexeme_len = sprintf(name->name.strname, "type@%d:%d", name->line_no, name->column_no);
    assert(lexeme_len <= MAXLEN_ANONTYPE);
    Ast* type_ref = arena_malloc(storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    Ast* type = arena_malloc(storage, sizeof(Ast));
    type->kind = AST_headerStackType;
    type->line_no = token->line_no;
    type->column_no = token->column_no;
    type->headerStackType.name = name;
    type->headerStackType.type = type_ref;
    if (token_is_expression(token)) {
      type->headerStackType.stack_expr = parse_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `]` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: expression expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return type;
  } else error("At line %d, column %d: `[` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_specializedType(Ast* named_type)
{
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    Ast* name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->line_no = named_type->line_no;
    name->column_no = named_type->column_no;
    name->name.strname = arena_malloc(storage, MAXLEN_ANONTYPE);
    int lexeme_len = sprintf(name->name.strname, "type@%d:%d", name->line_no, name->column_no);
    assert(lexeme_len <= MAXLEN_ANONTYPE);
    Ast* type_ref = arena_malloc(storage, sizeof(Ast));
    type_ref->kind = AST_typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    Ast* type = arena_malloc(storage, sizeof(Ast));
    type->kind = AST_specializedType;
    type->line_no = token->line_no;
    type->column_no = token->column_no;
    type->specializedType.type_args = parse_typeArgumentList();
    type->specializedType.name = name;
    type->specializedType.type = type_ref;
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `>` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return type;
  } else error("At line %d, column %d: `<` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_baseType()
{
  if (token_is_baseType(token)) {
    Ast* type_name = arena_malloc(storage, sizeof(Ast));
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    if (token->klass == TK_BOOL) {
      Ast* bool_type = arena_malloc(storage, sizeof(Ast));
      bool_type->kind = AST_baseTypeBoolean;
      bool_type->line_no = token->line_no;
      bool_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      bool_type->baseTypeBoolean.name = type_name;
      next_token();
      return bool_type;
    } else if (token->klass == TK_INT) {
      Ast* int_type = arena_malloc(storage, sizeof(Ast));
      int_type->kind = AST_baseTypeInteger;
      int_type->line_no = token->line_no;
      int_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      int_type->baseTypeInteger.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->baseTypeInteger.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
      return int_type;
    } else if (token->klass == TK_BIT) {
      Ast* bit_type = arena_malloc(storage, sizeof(Ast));
      bit_type->kind = AST_baseTypeBit;
      bit_type->line_no = token->line_no;
      bit_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      bit_type->baseTypeBit.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->baseTypeBit.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
      return bit_type;
    } else if (token->klass == TK_VARBIT) {
      Ast* varbit_type = arena_malloc(storage, sizeof(Ast));
      varbit_type->kind = AST_baseTypeVarbit;
      varbit_type->line_no = token->line_no;
      varbit_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      varbit_type->baseTypeVarbit.name = type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->baseTypeVarbit.size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: '<' was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return varbit_type;
    } else if (token->klass == TK_STRING) {
      Ast* string_type = arena_malloc(storage, sizeof(Ast));
      string_type->kind = AST_baseTypeString;
      string_type->line_no = token->line_no;
      string_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      string_type->baseTypeString.name = type_name;
      next_token();
      return string_type;
    } else if (token->klass == TK_VOID) {
      Ast* void_type = arena_malloc(storage, sizeof(Ast));
      void_type->kind = AST_baseTypeVoid;
      void_type->line_no = token->line_no;
      void_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      void_type->baseTypeVoid.name = type_name;
      next_token();
      return void_type;
    } else if (token->klass == TK_ERROR) {
      Ast* error_type = arena_malloc(storage, sizeof(Ast));
      error_type->kind = AST_baseTypeError;
      error_type->line_no = token->line_no;
      error_type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      error_type->baseTypeError.name = type_name;
      next_token();
      return error_type;
    } else assert(0);
  } else error("At line %d, column %d: base type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_integerTypeSize()
{
  Ast* type_size = arena_malloc(storage, sizeof(Ast));
  type_size->kind = AST_integerTypeSize;
  type_size->line_no = token->line_no;
  type_size->column_no = token->column_no;
  if (token->klass == TK_INTEGER_LITERAL) {
    type_size->integerTypeSize.size = parse_integer();
  } else if (token->klass == TK_PARENTH_OPEN) {
    /* TODO
    type_size->size = parse_expression(1); */
    error("At line %d, column %d: integer was expected, got `%s`.",
          token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: `(` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  return type_size;
}

static Ast*
parse_typeOrVoid()
{
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      Ast* type = parse_typeRef();
      return type;
    } else if (token->klass == TK_VOID) {
      return parse_baseType();
    } else if (token->klass == TK_IDENTIFIER) {
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->name.strname = token->lexeme;
      next_token();
      return name;
    } else assert(0);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_optTypeParameters()
{
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    if (token_is_typeParameterList(token)) {
      Ast* params = parse_typeParameterList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `>` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return params;
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `>` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  }
  return 0;
}

static Ast*
parse_typeParameterList()
{
  Ast* params = arena_malloc(storage, sizeof(Ast));
  params->kind = AST_typeParameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  if (token_is_typeParameterList(token)) {
    Ast* name = parse_name();
    NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
    namedecl->strname = name->name.strname;
    scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
    params->typeParameterList.first_child = name;
    while (token->klass == TK_COMMA) {
      next_token();
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      name->right_sibling = name;
      name = name->right_sibling;
    }
  }
  return params;
}

static Ast*
parse_realTypeArg()
{
  if (token_is_realTypeArg(token)) {
    Ast* type_arg = arena_malloc(storage, sizeof(Ast));
    type_arg->kind = AST_realTypeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = dontcare_arg->line_no;
      name->column_no = dontcare_arg->column_no;
      name->name.strname = "_";
      dontcare_arg->dontcare.name = name;
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->realTypeArg.arg = parse_typeRef();
      return type_arg;
    } else assert(0);
  } else error("At line %d, column %d: type argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typeArg()
{
  if (token_is_typeArg(token)) {
    Ast* type_arg = arena_malloc(storage, sizeof(Ast));
    type_arg->kind = AST_typeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = dontcare_arg->line_no;
      name->column_no = dontcare_arg->column_no;
      name->name.strname = "_";
      dontcare_arg->dontcare.name = name;
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->typeArg.arg = parse_typeRef();
      return type_arg;
    } else if (token_is_nonTypeName(token)) {
      type_arg->typeArg.arg = parse_nonTypeName();
      return type_arg;
    } else assert(0);
  } else error("At line %d, column %d: type argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_realTypeArgumentList()
{
  Ast* args = arena_malloc(storage, sizeof(Ast));
  args->kind = AST_realTypeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token_is_realTypeArg(token)) {
    Ast* ast = parse_realTypeArg();
    args->realTypeArgumentList.first_child = ast;
    while (token->klass == TK_COMMA) {
      next_token();
      ast->right_sibling = parse_realTypeArg();
      ast = ast->right_sibling;
    }
  }
  return args;
}

static Ast*
parse_typeArgumentList()
{
  Ast* args = arena_malloc(storage, sizeof(Ast));
  args->kind = AST_typeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token_is_typeArg(token)) {
    Ast* ast = parse_typeArg();
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
  if (token_is_typeDeclaration(token)) {
    Ast* type_decl = arena_malloc(storage, sizeof(Ast));
    type_decl->kind = AST_typeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token_is_derivedTypeDeclaration(token)) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration();
      return type_decl;
    } else if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
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
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return type_decl;
    } else assert(0);
  } else error("At line %d, column %d: type declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme); 
  assert(0);
  return 0;
}

static Ast*
parse_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    Ast* type_decl = arena_malloc(storage, sizeof(Ast));
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
  } else error("At line %d, column %d: structure declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerTypeDeclaration()
{
  if (token->klass == TK_HEADER) {
    next_token();
    Ast* header_decl = arena_malloc(storage, sizeof(Ast));
    header_decl->kind = AST_headerTypeDeclaration;
    header_decl->line_no = token->line_no;
    header_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      header_decl->headerTypeDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        header_decl->headerTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return header_decl;
  } else error("At line %d, column %d: `header` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_headerUnionDeclaration()
{
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    Ast* union_decl = arena_malloc(storage, sizeof(Ast));
    union_decl->kind = AST_headerUnionDeclaration;
    union_decl->line_no = token->line_no;
    union_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      union_decl->headerUnionDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        union_decl->headerUnionDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return union_decl;
  } else error("At line %d, column %d: `header_union` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_structTypeDeclaration()
{
  if (token->klass == TK_STRUCT) {
    next_token();
    Ast* struct_decl = arena_malloc(storage, sizeof(Ast));
    struct_decl->kind = AST_structTypeDeclaration;
    struct_decl->line_no = token->line_no;
    struct_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      struct_decl->structTypeDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        struct_decl->structTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return struct_decl;
  } else error("At line %d, column %d: `struct` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_structFieldList()
{
  Ast* fields = arena_malloc(storage, sizeof(Ast));
  fields->kind = AST_structFieldList;
  fields->line_no = token->line_no;
  fields->column_no = token->column_no;
  if (token_is_structField(token)) {
    Ast* ast = parse_structField();
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
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return field;
  } else error("At line %d, column %d: struct field was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_enumDeclaration()
{
  if (token->klass == TK_ENUM) {
    next_token();
    Ast* enum_decl = arena_malloc(storage, sizeof(Ast));
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
          } else error("At line %d, column %d: `>` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: an integer was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `<` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
    if (token_is_name(token)) {
      Ast* name = parse_name();
      NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
      namedecl->strname = name->name.strname;
      scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
      enum_decl->enumDeclaration.name = name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return enum_decl;
  } else error("At line %d, column %d: `enum` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_errorDeclaration()
{
  if (token->klass == TK_ERROR) {
    next_token();
    Ast* error_decl = arena_malloc(storage, sizeof(Ast));
    error_decl->kind = AST_errorDeclaration;
    error_decl->line_no = token->line_no;
    error_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        if (token_is_name(token)) {
          error_decl->errorDeclaration.fields = parse_identifierList();
        } else error("At line %d, column %d: name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return error_decl;
  } else error("At line %d, column %d: `error` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_matchKindDeclaration()
{
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    Ast* match_decl = arena_malloc(storage, sizeof(Ast));
    match_decl->kind = AST_matchKindDeclaration;
    match_decl->line_no = token->line_no;
    match_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        match_decl->matchKindDeclaration.fields = parse_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return match_decl;
  } else error("At line %d, column %d: `match_kind` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_identifierList()
{
  Ast* ids = arena_malloc(storage, sizeof(Ast));
  ids->kind = AST_identifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token_is_name(token)) {
    Ast* ast = parse_name();
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
  Ast* ids = arena_malloc(storage, sizeof(Ast));
  ids->kind = AST_specifiedIdentifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token_is_specifiedIdentifier(token)) {
    Ast* ast = parse_specifiedIdentifier();
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
  if (token_is_specifiedIdentifier(token)) {
    Ast* id = arena_malloc(storage, sizeof(Ast));
    id->kind = AST_specifiedIdentifier;
    id->line_no = token->line_no;
    id->column_no = token->column_no;
    id->specifiedIdentifier.name = parse_name();
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->specifiedIdentifier.init_expr = parse_expression(1);
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
    return id;
  } else error("At line %d, column %d: name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_typedefDeclaration()
{
  if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
    next_token();
    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      Ast* type_decl = arena_malloc(storage, sizeof(Ast));
      type_decl->kind = AST_typedefDeclaration;
      type_decl->line_no = token->line_no;
      type_decl->column_no = token->column_no;
      if (token_is_typeRef(token)) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        Ast* name = parse_name();
        NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
        namedecl->strname = name->name.strname;
        scope_push_decl(current_scope, storage, namedecl, NS_TYPE);
        type_decl->typedefDeclaration.name = name;
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("At line %d, column %d: `;` expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return type_decl;
    } else error("At line %d, column %d: type was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: type definition was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** STATEMENTS **/

static Ast*
parse_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    Ast* lvalue = parse_lvalue();
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      lvalue->lvalueExpression.type_args = parse_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `>` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      Ast* call_stmt = arena_malloc(storage, sizeof(Ast));
      call_stmt->kind = AST_functionCall;
      call_stmt->line_no = token->line_no;
      call_stmt->column_no = token->column_no;
      call_stmt->functionCall.lhs_expr = lvalue;
      call_stmt->functionCall.args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return call_stmt;
    } else if (token->klass == TK_EQUAL) {
      next_token();
      Ast* assign_stmt = arena_malloc(storage, sizeof(Ast));
      assign_stmt->kind = AST_assignmentStatement;
      assign_stmt->line_no = token->line_no;
      assign_stmt->column_no = token->column_no;
      assign_stmt->assignmentStatement.lhs_expr = lvalue;
      assign_stmt->assignmentStatement.rhs_expr = parse_expression(1);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return assign_stmt;
    } else error("At line %d, column %d: assignment or function call was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: lvalue was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_returnStatement()
{
  if (token->klass == TK_RETURN) {
    next_token();
    Ast* return_stmt = arena_malloc(storage, sizeof(Ast));
    return_stmt->kind = AST_returnStatement;
    return_stmt->line_no = token->line_no;
    return_stmt->column_no = token->column_no;
    if (token_is_expression(token))
      return_stmt->returnStatement.expr = parse_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return return_stmt;
  } else error("At line %d, column %d: `return` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_exitStatement()
{
  if (token->klass == TK_EXIT) {
    next_token();
    Ast* exit_stmt = arena_malloc(storage, sizeof(Ast));
    exit_stmt->kind = AST_exitStatement;
    exit_stmt->line_no = token->line_no;
    exit_stmt->column_no = token->column_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return exit_stmt;
  } else error("At line %d, column %d: `exit` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_conditionalStatement()
{
  if (token->klass == TK_IF) {
    next_token();
    Ast* if_stmt = arena_malloc(storage, sizeof(Ast));
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
              } else error("At line %d, column %d: statement was expected, got `%s`.",
                           token->line_no, token->column_no, token->lexeme);
            }
          } else error("At line %d, column %d: statement was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `(` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return if_stmt;
  } else error("At line %d, column %d: `if` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_directApplication(Ast* type_name)
{
  if (token_is_typeName(token) || type_name) {
    Ast* apply_stmt = arena_malloc(storage, sizeof(Ast));
    apply_stmt->kind = AST_directApplication;
    apply_stmt->line_no = token->line_no;
    apply_stmt->column_no = token->column_no;
    apply_stmt->directApplication.name = type_name ? type_name : parse_prefixedType();
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
            } else error("At line %d, column %d: `;` was expected, got `%s`.",
                         token->line_no, token->column_no, token->lexeme);
          } else error("At line %d, column %d: `)` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `(` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `apply` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `.` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return apply_stmt;
  } else error("At line %d, column %d: type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statement(Ast* type_name)
{
  if (token_is_statement(token)) {
    Ast* stmt = arena_malloc(storage, sizeof(Ast));
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
      Ast* empty_stmt = arena_malloc(storage, sizeof(Ast));
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
  } else error("At line %d, column %d: statement was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_blockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast* block_stmt = arena_malloc(storage, sizeof(Ast));
    block_stmt->kind = AST_blockStatement;
    block_stmt->line_no = token->line_no;
    block_stmt->column_no = token->column_no;
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `}` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return block_stmt;
  } else error("At line %d, column %d: `{` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statementOrDeclList()
{
  Ast* stmts = arena_malloc(storage, sizeof(Ast));
  stmts->kind = AST_statementOrDeclList;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token_is_statementOrDeclaration(token)) {
    Ast* ast = parse_statementOrDeclaration();
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
  if (token->klass == TK_SWITCH) {
    next_token();
    Ast* stmt = arena_malloc(storage, sizeof(Ast));
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
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `(` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return stmt;
  } else error("At line %d, column %d: `switch` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_switchCases()
{
  Ast* cases = arena_malloc(storage, sizeof(Ast));
  cases->kind = AST_switchCases;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token_is_switchLabel(token)) {
    Ast* ast = parse_switchCase();
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
  if (token_is_switchLabel(token)) {
    Ast* switch_case = arena_malloc(storage, sizeof(Ast));
    switch_case->kind = AST_switchCase;
    switch_case->line_no = token->line_no;
    switch_case->column_no = token->column_no;
    switch_case->switchCase.label = parse_switchLabel();
    if (token->klass == TK_COLON) {
      next_token();
      if (token->klass == TK_BRACE_OPEN) {
        switch_case->switchCase.stmt = parse_blockStatement();
      }
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return switch_case;
  } else error("At line %d, column %d: switch label was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_switchLabel()
{
  if (token_is_switchLabel(token)) {
    Ast* switch_label = arena_malloc(storage, sizeof(Ast));
    switch_label->kind = AST_switchLabel;
    switch_label->line_no = token->line_no;
    switch_label->column_no = token->column_no;
    if (token_is_name(token)) {
      switch_label->switchLabel.label = parse_name();
      return switch_label;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast* default_label = arena_malloc(storage, sizeof(Ast));
      default_label->kind = AST_default;
      default_label->line_no = token->line_no;
      default_label->column_no = token->column_no;
      switch_label->switchLabel.label = default_label;
      return switch_label;
    } else assert(0);
  } else error("At line %d, column %d: switch label was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_statementOrDeclaration()
{
  if (token_is_statementOrDeclaration(token)) {
    Ast* stmt = arena_malloc(storage, sizeof(Ast));
    stmt->kind = AST_statementOrDeclaration;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
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
  if (token->klass == TK_TABLE) {
    next_token();
    Ast* table = arena_malloc(storage, sizeof(Ast));
    table->kind = AST_tableDeclaration;
    table->line_no = token->line_no;
    table->column_no = token->column_no;
    table->tableDeclaration.name = parse_name();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_tableProperty(token)) {
        table->tableDeclaration.prop_list = parse_tablePropertyList();
      } else error("At line %d, column %d: table property was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return table;
  } else error("At line %d, column %d: `table` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_tablePropertyList()
{
  Ast* props = arena_malloc(storage, sizeof(Ast));
  props->kind = AST_tablePropertyList;
  props->line_no = token->line_no;
  props->column_no = token->column_no;
  if (token_is_tableProperty(token)) {
    Ast* ast = parse_tableProperty();
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
  if (token_is_tableProperty(token)) {
    bool is_const = false;
    if (token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    Ast* table_prop = arena_malloc(storage, sizeof(Ast));
    table_prop->kind = AST_tableProperty;
    table_prop->line_no = token->line_no;
    table_prop->column_no = token->column_no;
    if (token->klass == TK_KEY) {
      next_token();
      Ast* key_prop = arena_malloc(storage, sizeof(Ast));
      key_prop->kind = AST_keyProperty;
      key_prop->line_no = token->line_no;
      key_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          key_prop->keyProperty.keyelem_list = parse_keyElementList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = key_prop;
      return table_prop;
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      Ast* actions_prop = arena_malloc(storage, sizeof(Ast));
      actions_prop->kind = AST_actionsProperty;
      actions_prop->line_no = token->line_no;
      actions_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          actions_prop->actionsProperty.action_list = parse_actionList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = actions_prop;
      return table_prop;
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      Ast* entries_prop = arena_malloc(storage, sizeof(Ast));
      entries_prop->kind = AST_entriesProperty;
      entries_prop->line_no = token->line_no;
      entries_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            entries_prop->entriesProperty.entries_list = parse_entriesList();
          } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = entries_prop;
      return table_prop;
    } else if (token_is_nonTableKwName(token)) {
      Ast* simple_prop = arena_malloc(storage, sizeof(Ast));
      simple_prop->kind = AST_simpleProperty;
      simple_prop->line_no = token->line_no;
      simple_prop->column_no = token->column_no;
      simple_prop->simpleProperty.is_const = is_const;
      simple_prop->simpleProperty.name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        simple_prop->simpleProperty.init_expr = parse_expression(1);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("At line %d, column %d: `;` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = simple_prop;
      return table_prop;
    } else assert(0);
  } else error("At line %d, column %d: table property was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_keyElementList()
{
  Ast* elems = arena_malloc(storage, sizeof(Ast));
  elems->kind = AST_keyElementList;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token_is_expression(token)) {
    Ast* ast = parse_keyElement();
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
  if (token_is_expression(token)) {
    Ast* key_elem = arena_malloc(storage, sizeof(Ast));
    key_elem->kind = AST_keyElement;
    key_elem->line_no = token->line_no;
    key_elem->column_no = token->column_no;
    key_elem->keyElement.expr = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->keyElement.match = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return key_elem;
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_actionList()
{
  Ast* actions = arena_malloc(storage, sizeof(Ast));
  actions->kind = AST_actionList;
  actions->line_no = token->line_no;
  actions->column_no = token->column_no;
  if (token_is_actionRef(token)) {
    Ast* ast = parse_actionRef();
    actions->actionList.first_child = ast;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    while (token_is_actionRef(token)) {
      ast->right_sibling = parse_actionRef();
      ast = ast->right_sibling;
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
  }
  return actions;
}

static Ast*
parse_actionRef()
{
  if (token_is_prefixedNonTypeName(token)) {
    Ast* action_ref = arena_malloc(storage, sizeof(Ast));
    action_ref->kind = AST_actionRef;
    action_ref->line_no = token->line_no;
    action_ref->column_no = token->column_no;
    action_ref->actionRef.name = parse_prefixedNonTypeName();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_argument(token)) {
        action_ref->actionRef.args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    }
    return action_ref;
  } else error("At line %d, column %d: non-type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_entriesList()
{
  Ast* entries = arena_malloc(storage, sizeof(Ast));
  entries->kind = AST_entriesList;
  entries->line_no = token->line_no;
  entries->column_no = token->column_no;
  if (token_is_keysetExpression(token)) {
    Ast* ast = parse_entry();
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
  if (token_is_keysetExpression(token)) {
    Ast* entry = arena_malloc(storage, sizeof(Ast));
    entry->kind = AST_entry;
    entry->line_no = token->line_no;
    entry->column_no = token->column_no;
    entry->entry.keyset = parse_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      entry->entry.action = parse_actionRef();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return entry;
  } else error("At line %d, column %d: keyset was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_actionDeclaration()
{
  if (token->klass == TK_ACTION) {
    next_token();
    Ast* action_decl = arena_malloc(storage, sizeof(Ast));
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
          } else error("At line %d, column %d: `{` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return action_decl;
  } else error("At line %d, column %d: `action` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** VARIABLES **/

static Ast*
parse_variableDeclaration(Ast* type_ref)
{
  bool is_const = false;
  if (token->klass == TK_CONST) {
    next_token();
    is_const = true;
  }
  if (token_is_typeRef(token) || type_ref) {
    Ast* var_decl = arena_malloc(storage, sizeof(Ast));
    var_decl->kind = AST_variableDeclaration;
    var_decl->line_no = token->line_no;
    var_decl->column_no = token->column_no;
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef();
    if (token_is_name(token)) {
      var_decl->variableDeclaration.name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        var_decl->variableDeclaration.init_expr = parse_expression(1);
      }
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    var_decl->variableDeclaration.is_const = is_const;
    return var_decl;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** EXPRESSIONS **/

static Ast*
parse_functionDeclaration(Ast* type_ref)
{
  if (token_is_typeOrVoid(token)) {
    Ast* func_decl = arena_malloc(storage, sizeof(Ast));
    func_decl->kind = AST_functionDeclaration;
    func_decl->line_no = token->line_no;
    func_decl->column_no = token->column_no;
    func_decl->functionDeclaration.proto = parse_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      func_decl->functionDeclaration.stmt = parse_blockStatement();
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return func_decl;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_argumentList()
{
  Ast* args = arena_malloc(storage, sizeof(Ast));
  args->kind = AST_argumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token_is_argument(token)) {
    Ast* ast = parse_argument();
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
  if (token_is_argument(token)) {
    Ast* arg = arena_malloc(storage, sizeof(Ast));
    arg->kind = AST_argument;
    arg->line_no = token->line_no;
    arg->column_no = token->column_no;
    if (token_is_expression(token)) {
      arg->argument.arg = parse_expression(1);
      return arg;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_malloc(storage, sizeof(Ast));
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = dontcare_arg->line_no;
      name->column_no = dontcare_arg->column_no;
      name->name.strname = "_";
      dontcare_arg->dontcare.name = name;
      arg->argument.arg = dontcare_arg;
      return arg;
    } else assert(0);
  } else error("At line %d, column %d: an argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
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
parse_prefixedNonTypeName()
{
  if (token->klass == TK_DOT) {
    next_token();
  }
  if (token_is_nonTypeName(token)) {
    return parse_nonTypeName();
  } else error("At line %d, column %d: non-type name was expected, ",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_lvalue()
{
  if (token_is_lvalue(token)) {
    Ast* lvalue = arena_malloc(storage, sizeof(Ast));
    lvalue->kind = AST_lvalueExpression;
    lvalue->line_no = token->line_no;
    lvalue->column_no = token->column_no;
    lvalue->lvalueExpression.expr = parse_prefixedNonTypeName();
    while(token->klass == TK_DOT || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast* member_expr = arena_malloc(storage, sizeof(Ast));
        member_expr->kind = AST_memberSelector;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->memberSelector.lhs_expr = lvalue;
        if (token_is_name(token)) {
          member_expr->memberSelector.name = parse_name();
        } else error("At line %d, column %d: name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_malloc(storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->lvalueExpression.expr = member_expr;
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast* subscript_expr = arena_malloc(storage, sizeof(Ast));
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->arraySubscript.lhs_expr = lvalue;
        subscript_expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_malloc(storage, sizeof(Ast));
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->lvalueExpression.expr = subscript_expr;
      }
    }
    return lvalue;
  } else error("At line %d, column %d: lvalue was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_expression(int priority_threshold)
{
  if (token_is_expression(token)) {
    Ast* expr = parse_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast* member_expr = arena_malloc(storage, sizeof(Ast));
        member_expr->kind = AST_memberSelector;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->memberSelector.lhs_expr = expr;
        if (token_is_nonTypeName(token)) {
          member_expr->memberSelector.name = parse_nonTypeName();
        } else error("At line %d, column %d: non-type name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expression.expr = member_expr;
      } else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast* subscript_expr = arena_malloc(storage, sizeof(Ast));
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->arraySubscript.lhs_expr = expr;
        subscript_expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expression.expr = subscript_expr;
      } else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        Ast* call_expr = arena_malloc(storage, sizeof(Ast));
        call_expr->kind = AST_functionCall;
        call_expr->line_no = token->line_no;
        call_expr->column_no = token->column_no;
        call_expr->functionCall.lhs_expr = expr;
        call_expr->functionCall.args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expression.expr = call_expr;
      } else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        expr->expression.type_args = parse_realTypeArgumentList();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        Ast* assign_stmt = arena_malloc(storage, sizeof(Ast));
        assign_stmt->kind = AST_assignmentStatement;
        assign_stmt->line_no = token->line_no;
        assign_stmt->column_no = token->column_no;
        assign_stmt->assignmentStatement.lhs_expr = expr;
        assign_stmt->assignmentStatement.rhs_expr = parse_expression(1);
        expr = arena_malloc(storage, sizeof(Ast));
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expression.expr = assign_stmt;
      } else if (token_is_binaryOperator(token)){
        int priority = operator_priority(token);
        if (priority >= priority_threshold) {
          next_token();
          Ast* binary_expr = arena_malloc(storage, sizeof(Ast));
          binary_expr->kind = AST_binaryExpression;
          binary_expr->line_no = token->line_no;
          binary_expr->column_no = token->column_no;
          binary_expr->binaryExpression.left_operand = expr;
          binary_expr->binaryExpression.op = token_to_binop(token);
          binary_expr->binaryExpression.right_operand = parse_expression(priority + 1);
          expr = arena_malloc(storage, sizeof(Ast));
          expr->kind = AST_expression;
          expr->line_no = token->line_no;
          expr->column_no = token->column_no;
          expr->expression.expr = binary_expr;
        } else break;
      } else assert(0);
    }
    return expr;
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_expressionPrimary()
{
  if (token_is_expression(token)) {
    Ast* primary = arena_malloc(storage, sizeof(Ast));
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
        primary->expression.expr = parse_namedType();
        return primary;
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(token)) {
      primary->expression.expr = parse_nonTypeName();
      return primary;
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      primary->expression.expr = parse_expressionList();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return primary;
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        Ast* cast_expr = arena_malloc(storage, sizeof(Ast));
        cast_expr->kind = AST_castExpression;
        cast_expr->line_no = token->line_no;
        cast_expr->column_no = token->column_no;
        cast_expr->castExpression.type = parse_typeRef();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->castExpression.expr = parse_expression(1);
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        primary->expression.expr = cast_expr;
        return primary;
      } else if (token_is_expression(token)) {
        primary->expression.expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        return primary;
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      Ast* unary_expr = arena_malloc(storage, sizeof(Ast));
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->unaryExpression.op = OP_NOT;
      unary_expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = unary_expr;
      return primary;
    } else if (token->klass == TK_TILDA) {
      next_token();
      Ast* unary_expr = arena_malloc(storage, sizeof(Ast));
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->unaryExpression.op = OP_BITW_NOT;
      unary_expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = unary_expr;
      return primary;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      Ast* unary_expr = arena_malloc(storage, sizeof(Ast));
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->unaryExpression.op = OP_NEG;
      unary_expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = unary_expr;
      return primary;
    } else if (token_is_typeName(token)) {
      primary->expression.expr = parse_namedType();
      return primary;
    } else if (token->klass == TK_ERROR) {
      next_token();
      Ast* name = arena_malloc(storage, sizeof(Ast));
      name->kind = AST_name;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->name.strname = "error";
      primary->expression.expr = name;
      return primary;
    } else assert(0);
    assert(0);
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_indexExpression()
{
  if (token_is_expression(token)) {
    Ast* index_expr = arena_malloc(storage, sizeof(Ast));
    index_expr->kind = AST_indexExpression;
    index_expr->line_no = token->line_no;
    index_expr->column_no = token->column_no;
    index_expr->indexExpression.start_index = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_expression(token)) {
        index_expr->indexExpression.end_index = parse_expression(1);
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    }
    return index_expr;
  } else error("At line %d, column %d: expression or `:` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_integer()
{
  if (token->klass == TK_INTEGER_LITERAL) {
    Ast* int_literal = arena_malloc(storage, sizeof(Ast));
    int_literal->kind = AST_integerLiteral;
    int_literal->line_no = token->line_no;
    int_literal->column_no = token->column_no;
    int_literal->integerLiteral.is_signed = token->integer.is_signed;
    int_literal->integerLiteral.width = token->integer.width;
    int_literal->integerLiteral.value = token->integer.value;
    next_token();
    return int_literal;
  } else error("At line %d, column %d: integer was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_boolean()
{
  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    Ast* bool_literal = arena_malloc(storage, sizeof(Ast));
    bool_literal->kind = AST_booleanLiteral;
    bool_literal->line_no = token->line_no;
    bool_literal->column_no = token->column_no;
    bool_literal->booleanLiteral.value = (token->klass == TK_TRUE);
    next_token();
    return bool_literal;
  } else error("At line %d, column %d: boolean was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

static Ast*
parse_string()
{
  if (token->klass == TK_STRING_LITERAL) {
    Ast* string_literal = arena_malloc(storage, sizeof(Ast));
    string_literal->kind = AST_stringLiteral;
    string_literal->line_no = token->line_no;
    string_literal->column_no = token->column_no;
    string_literal->stringLiteral.value = token->lexeme;
    next_token();
    return string_literal;
  } else error("At line %d, column %d: string was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast*
parse_program(UnboundedArray* _tokens, Scope** _root_scope, Arena* _storage)
{
  tokens = _tokens;
  storage = _storage;
  root_scope = scope_create(storage, 16, 1008);
  current_scope = root_scope;

  struct Keyword {
    char* strname;
    enum TokenClass token_class;
  };
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
    {"type",    TK_TYPE},
    {"default", TK_DEFAULT},
    {"extern",  TK_EXTERN},
    {"header_union", TK_HEADER_UNION},
    {"out",     TK_OUT},
    {"transition", TK_TRANSITION},
    {"else",    TK_ELSE},
    {"exit",    TK_EXIT},
    {"if",      TK_IF},
    {"match_kind", TK_MATCH_KIND},
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
  };
  for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
    NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
    namedecl->strname = keywords[i].strname;
    namedecl->token_class = keywords[i].token_class;
    scope_push_decl(current_scope, storage, namedecl, NS_KEYWORD);
  }

  struct BuiltinName {
    char* strname;
    enum NameSpace ns;
  };
  struct BuiltinName builtin_names[] = {
    {"bool",   NS_TYPE},
    {"int",    NS_TYPE},
    {"bit",    NS_TYPE},
    {"varbit", NS_TYPE},
    {"string", NS_TYPE},
    {"void",   NS_TYPE},
    {"error",  NS_TYPE},
    {"match_kind", NS_TYPE},
    {"accept", NS_VAR},
    {"reject", NS_VAR},
  };
  for (int i = 0; i < sizeof(builtin_names)/sizeof(builtin_names[0]); i++) {
    Ast* name = arena_malloc(storage, sizeof(Ast));
    name->kind = AST_name;
    name->name.strname = builtin_names[i].strname;
    NameDecl* namedecl = arena_malloc(storage, sizeof(NameDecl));
    namedecl->strname = name->name.strname;
    namedecl->ast = name;
    scope_push_decl(root_scope, storage, namedecl, builtin_names[i].ns);
  }

  token_at = 0;
  token = array_get(tokens, token_at, sizeof(Token));
  next_token();
  Ast* ast = parse_p4program();
  assert(current_scope == root_scope);
  *_root_scope = root_scope;
  return ast;
}
