#include <memory.h>  // memset
#include <stdint.h>
#include <stdio.h>
#include "foundation.h"
#include "frontend.h"

internal Arena *ast_storage;
internal UnboundedArray* tokens;
internal int token_at = 0;
internal Token* token = 0;
internal int prev_token_at = 0;
internal Token* prev_token = 0;
internal Scope* root_scope;
internal Scope* current_scope;

/** PROGRAM **/

internal Ast* parse_p4program();
internal Ast* parse_declarationList();
internal Ast* parse_declaration();
internal Ast* parse_nonTypeName();
internal Ast* parse_name();
internal Ast* parse_parameterList();
internal Ast* parse_parameter();
internal enum Ast_ParamDirection parse_direction();
internal Ast* parse_packageTypeDeclaration();
internal Ast* parse_instantiation(Ast* type_ref);
internal Ast* parse_optConstructorParameters();

/** PARSER **/

internal Ast* parse_parserDeclaration(Ast* parser_proto);
internal Ast* parse_parserLocalElements();
internal Ast* parse_parserLocalElement();
internal Ast* parse_parserTypeDeclaration();
internal Ast* parse_parserStates();
internal Ast* parse_parserState();
internal Ast* parse_parserStatements();
internal Ast* parse_parserStatement();
internal Ast* parse_parserBlockStatement();
internal Ast* parse_transitionStatement();
internal Ast* parse_stateExpression();
internal Ast* parse_selectExpression();
internal Ast* parse_selectCaseList();
internal Ast* parse_selectCase();
internal Ast* parse_keysetExpression();
internal Ast* parse_tupleKeysetExpression();
internal Ast* parse_simpleExpressionList();
internal Ast* parse_simpleKeysetExpression();

/** CONTROL **/

internal Ast* parse_controlDeclaration(Ast* control_proto);
internal Ast* parse_controlTypeDeclaration();
internal Ast* parse_controlLocalDeclaration();
internal Ast* parse_controlLocalDeclarations();

/** EXTERN **/

internal Ast* parse_externDeclaration();
internal Ast* parse_methodPrototypes();
internal Ast* parse_functionPrototype(Ast* return_type);
internal Ast* parse_methodPrototype();

/** TYPES **/

internal Ast* parse_typeRef();
internal Ast* parse_namedType();
internal Ast* parse_prefixedType();
internal Ast* parse_tupleType();
internal Ast* parse_headerStackType();
internal Ast* parse_specializedType();
internal Ast* parse_baseType();
internal Ast* parse_integerTypeSize();
internal Ast* parse_typeOrVoid();
internal Ast* parse_optTypeParameters();
internal Ast* parse_typeParameterList();
internal Ast* parse_realTypeArg();
internal Ast* parse_typeArg();
internal Ast* parse_realTypeArgumentList();
internal Ast* parse_typeArgumentList();
internal Ast* parse_typeDeclaration();
internal Ast* parse_derivedTypeDeclaration();
internal Ast* parse_headerTypeDeclaration();
internal Ast* parse_headerUnionDeclaration();
internal Ast* parse_structTypeDeclaration();
internal Ast* parse_structFieldList();
internal Ast* parse_structField();
internal Ast* parse_enumDeclaration();
internal Ast* parse_errorDeclaration();
internal Ast* parse_matchKindDeclaration();
internal Ast* parse_identifierList();
internal Ast* parse_specifiedIdentifierList();
internal Ast* parse_specifiedIdentifier();
internal Ast* parse_typedefDeclaration();

/** STATEMENTS **/

internal Ast* parse_assignmentOrMethodCallStatement();
internal Ast* parse_returnStatement();
internal Ast* parse_exitStatement();
internal Ast* parse_conditionalStatement();
internal Ast* parse_directApplication(Ast* type_name);
internal Ast* parse_statement(Ast* type_name);
internal Ast* parse_blockStatement();
internal Ast* parse_statementOrDeclList();
internal Ast* parse_switchStatement();
internal Ast* parse_switchCases();
internal Ast* parse_switchCase();
internal Ast* parse_switchLabel();
internal Ast* parse_statementOrDeclaration();

/** TABLES **/

internal Ast* parse_tableDeclaration();
internal Ast* parse_tablePropertyList();
internal Ast* parse_tableProperty();
internal Ast* parse_keyElementList();
internal Ast* parse_keyElement();
internal Ast* parse_actionList();
internal Ast* parse_actionRef();
internal Ast* parse_entriesList();
internal Ast* parse_entry();
internal Ast* parse_actionDeclaration();

/** VARIABLES **/

internal Ast* parse_variableDeclaration(Ast* type_ref);

/** EXPRESSIONS **/

internal Ast* parse_functionDeclaration(Ast* type_ref);
internal Ast* parse_argumentList();
internal Ast* parse_argument();
internal Ast* parse_expressionList();
internal Ast* parse_prefixedNonTypeName();
internal Ast* parse_lvalue();
internal Ast* parse_expression(int priority_threshold);
internal Ast* parse_expressionPrimary();
internal Ast* parse_indexExpression();
internal Ast* parse_integer();
internal Ast* parse_boolean();
internal Ast* parse_string();

internal Token*
next_token()
{
  assert (token_at < tokens->elem_count);
  prev_token = token;
  prev_token_at = token_at;
  token = array_get(tokens, ++token_at);
  while (token->klass == TK_COMMENT) {
    token = array_get(tokens, ++token_at);
  }
  if (token->klass == TK_IDENTIFIER) {
    NamespaceEntry* ns = scope_lookup_name(current_scope, token->lexeme);
    if (ns && ns->ns_keyword) {
      NameDecl* ndecl = ns->ns_keyword;
      token->klass = ndecl->token_class;
      return token;
    } else if (ns && ns->ns_type.item_count > 0) {
      token->klass = TK_TYPE_IDENTIFIER;
      return token;
    }
  }
  return token;
}

internal Token*
peek_token()
{
  prev_token = token;
  prev_token_at = token_at;
  Token* peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

internal bool
token_is_nonTypeName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_APPLY || token->klass == TK_KEY
    || token->klass == TK_ACTIONS || token->klass == TK_STATE || token->klass == TK_ENTRIES || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_name(Token* token)
{
  bool result = token_is_nonTypeName(token) || token->klass == TK_TYPE_IDENTIFIER;
  return result;
}

internal bool
token_is_typeName(Token* token)
{
  return token->klass == TK_DOT || token->klass == TK_TYPE_IDENTIFIER;
}

internal bool
token_is_prefixedType(Token* token)
{
  return token->klass == TK_DOT || token->klass == TK_TYPE_IDENTIFIER;
}

internal bool
token_is_prefixedNonTypeName(Token* token) {
  return token->klass == TK_DOT || token_is_nonTypeName(token);
}

internal bool
token_is_nonTableKwName(Token* token)
{
  bool result = token->klass == TK_IDENTIFIER || token->klass == TK_TYPE_IDENTIFIER
    || token->klass == TK_APPLY || token->klass == TK_STATE || token->klass == TK_TYPE;
  return result;
}

internal bool
token_is_baseType(Token* token)
{
  bool result = token->klass == TK_BOOL || token->klass == TK_ERROR || token->klass == TK_INT
    || token->klass == TK_BIT || token->klass == TK_VARBIT || token->klass == TK_STRING
    || token->klass == TK_VOID;
  return result;
}

internal bool
token_is_typeRef(Token* token)
{
  bool result = token_is_baseType(token) || token_is_prefixedType(token) || token->klass == TK_TUPLE;
  return result;
}

internal bool
token_is_direction(Token* token)
{
  bool result = token->klass == TK_IN || token->klass == TK_OUT || token->klass == TK_INOUT;
  return result;
}

internal bool
token_is_parameter(Token* token)
{
  bool result = token_is_direction(token) || token_is_typeRef(token);
  return result;
}

internal bool
token_is_derivedTypeDeclaration(Token* token)
{
  bool result = token->klass == TK_HEADER || token->klass == TK_HEADER_UNION || token->klass == TK_STRUCT
    || token->klass == TK_ENUM;
  return result;
}

internal bool
token_is_typeDeclaration(Token* token)
{
  bool result = token_is_derivedTypeDeclaration(token) || token->klass == TK_TYPEDEF || token->klass == TK_TYPE
    || token->klass == TK_PARSER || token->klass == TK_CONTROL || token->klass == TK_PACKAGE;
  return result;
}

internal bool
token_is_typeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE || token_is_typeRef(token) || token_is_nonTypeName(token);
  return result;
}

internal bool
token_is_typeParameterList(Token* token)
{
  return token_is_name(token);
}

internal bool
token_is_typeOrVoid(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_VOID || token->klass == TK_IDENTIFIER;
  return result;
}

internal bool
token_is_actionRef(Token* token)
{
  bool result = token->klass == TK_DOT || token_is_nonTypeName(token)
    || token->klass == TK_PARENTH_OPEN;
  return result;
}

internal bool
token_is_tableProperty(Token* token)
{
  bool result = token->klass == TK_KEY || token->klass == TK_ACTIONS
    || token->klass == TK_CONST || token->klass == TK_ENTRIES
    || token_is_nonTableKwName(token);
  return result;
}

internal bool
token_is_switchLabel(Token* token)
{
  bool result = token_is_name(token) || token->klass == TK_DEFAULT;
  return result;
}

internal bool
token_is_expressionPrimary(Token* token)
{
  bool result = token->klass == TK_INTEGER_LITERAL || token->klass == TK_TRUE || token->klass == TK_FALSE
    || token->klass == TK_STRING_LITERAL || token->klass == TK_DOT || token_is_nonTypeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_PARENTH_OPEN || token->klass == TK_EXCLAMATION
    || token->klass == TK_TILDA || token->klass == TK_UNARY_MINUS || token_is_typeName(token)
    || token->klass == TK_ERROR || token_is_prefixedType(token);
  return result;
}

internal bool
token_is_expression(Token* token)
{
  return token_is_expressionPrimary(token);
}

internal bool
token_is_methodPrototype(Token* token)
{
  return token_is_typeOrVoid(token) || token->klass == TK_TYPE_IDENTIFIER;
}

internal bool
token_is_structField(Token* token)
{
  bool result = token_is_typeRef(token);
  return result;
}

internal bool
token_is_specifiedIdentifier(Token* token)
{
  return token_is_name(token);
}

internal bool
token_is_declaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_EXTERN || token->klass == TK_ACTION
    || token->klass == TK_PARSER || token_is_typeDeclaration(token) || token->klass == TK_CONTROL
    || token_is_typeRef(token) || token->klass == TK_ERROR || token->klass == TK_MATCH_KIND
    || token_is_typeOrVoid(token) || token->klass == TK_DOT;
  return result;
}

internal bool
token_is_lvalue(Token* token)
{
  bool result = token_is_nonTypeName(token) | (token->klass == TK_DOT);
  return result;
}

internal bool
token_is_assignmentOrMethodCallStatement(Token* token)
{
  bool result = token_is_lvalue(token) || token->klass == TK_PARENTH_OPEN || token->klass == TK_ANGLE_OPEN
    || token->klass == TK_EQUAL;
  return result;
}

internal bool
token_is_statement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token) || token->klass == TK_IF
    || token->klass == TK_SEMICOLON || token->klass == TK_BRACE_OPEN || token->klass == TK_EXIT
    || token->klass == TK_RETURN || token->klass == TK_SWITCH;
  return result;
}

internal bool
token_is_statementOrDeclaration(Token* token)
{
  bool result = token_is_typeRef(token) || token->klass == TK_CONST || token_is_statement(token);
  return result;
}

internal bool
token_is_argument(Token* token)
{
  bool result = token_is_expression(token) || token_is_name(token) || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_parserLocalElement(Token* token)
{
  bool result = token->klass == TK_CONST || token_is_typeRef(token);
  return result;
}

internal bool
token_is_parserStatement(Token* token)
{
  bool result = token_is_assignmentOrMethodCallStatement(token) || token_is_typeName(token)
    || token->klass == TK_BRACE_OPEN || token->klass == TK_CONST || token_is_typeRef(token)
    || token->klass == TK_SEMICOLON;
  return result;
}

internal bool
token_is_simpleKeysetExpression(Token* token) {
  bool result = token_is_expression(token) || token->klass == TK_DEFAULT || token->klass == TK_DONTCARE;
  return result;
}

internal bool
token_is_keysetExpression(Token* token)
{
  bool result = token->klass == TK_TUPLE || token_is_simpleKeysetExpression(token);
  return result;
}

internal bool
token_is_selectCase(Token* token)
{
  return token_is_keysetExpression(token);
}

internal bool
token_is_controlLocalDeclaration(Token* token)
{
  bool result = token->klass == TK_CONST || token->klass == TK_ACTION
    || token->klass == TK_TABLE || token_is_typeRef(token) || token_is_typeRef(token);
  return result;
}

internal bool
token_is_realTypeArg(Token* token)
{
  bool result = token->klass == TK_DONTCARE|| token_is_typeRef(token);
  return result;
}

internal bool
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

internal bool
token_is_exprOperator(Token* token)
{
  bool result = token_is_binaryOperator(token) || token->klass == TK_DOT
    || token->klass == TK_BRACKET_OPEN || token->klass == TK_PARENTH_OPEN
    || token->klass == TK_ANGLE_OPEN;
  return result;
}

internal int
operator_priority_of(Token* token)
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

internal enum Ast_Operator
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

internal Ast*
parse_p4program()
{
  Ast_P4Program* program = arena_push_struct(ast_storage, Ast_P4Program);
  program->kind = AST_p4program;
  program->line_no = token->line_no;
  program->column_no = token->column_no;
  while (token->klass == TK_SEMICOLON) {
    next_token(); /* empty declaration */
  }
  program->decl_list = parse_declarationList();
  if (token->klass != TK_END_OF_INPUT) {
    error("At line %d, column %d: unexpected token `%s`.",
          token->line_no, token->column_no, token->lexeme);
  }
  return (Ast*)program;
}

internal Ast*
parse_declarationList()
{
  Ast_DeclarationList* decls = arena_push_struct(ast_storage, Ast_DeclarationList);
  decls->kind = AST_declarationList;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  list_reset(&decls->members);
  if (token_is_declaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_declaration();
    list_append_item(&decls->members, li, 1);
    while (token_is_declaration(token) || token->klass == TK_SEMICOLON) {
      if (token_is_declaration(token)) {
        li = arena_push_struct(ast_storage, ListItem);
        li->object = parse_declaration();
        list_append_item(&decls->members, li, 1);
      } else if (token->klass == TK_SEMICOLON) {
        next_token(); /* empty declaration */
      }
    }
  }
  return (Ast*)decls;
}

internal Ast*
parse_declaration()
{
  if (token_is_declaration(token)) {
    Ast_Declaration* decl = arena_push_struct(ast_storage, Ast_Declaration);
    decl->kind = AST_declaration;
    decl->line_no = token->line_no;
    decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      decl->decl = parse_variableDeclaration(0);
      return (Ast*)decl;
    } else if (token->klass == TK_EXTERN) {
      decl->decl = parse_externDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_ACTION) {
      decl->decl = parse_actionDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_PARSER) {
      decl->decl = parse_typeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else {
        decl->decl = parse_parserDeclaration(decl->decl);
      }
      return (Ast*)decl;
    } else if (token->klass == TK_CONTROL) {
      decl->decl = parse_typeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else {
        decl->decl = parse_controlDeclaration(decl->decl);
      }
      return (Ast*)decl;
    } else if (token_is_typeDeclaration(token)) {
      decl->decl = parse_typeDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_ERROR) {
      decl->decl = parse_errorDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_MATCH_KIND) {
      decl->decl = parse_matchKindDeclaration();
      return (Ast*)decl;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        decl->decl = parse_instantiation(type_ref);
        return (Ast*)decl;
      } else if (token_is_name(token)) {
        decl->decl = parse_functionDeclaration(type_ref);
        return (Ast*)decl;
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_typeOrVoid(token)) {
      decl->decl = parse_functionDeclaration(parse_typeRef());
      return (Ast*)decl;
    } else assert(0);
  } else error("At line %d, column %d: top-level declaration as expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_nonTypeName()
{
  if (token_is_nonTypeName(token)) {
    Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
    name->kind = AST_name;
    name->line_no = token->line_no;
    name->column_no = token->column_no;
    name->strname = token->lexeme;
    next_token();
    return (Ast*)name;
  } else error("At line %d, column %d: non-type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_name()
{
  if (token_is_name(token)) {
    if (token_is_nonTypeName(token)) {
      return parse_nonTypeName();
    } else if (token->klass == TK_TYPE_IDENTIFIER) {
      Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
      type_name->kind = AST_name;
      type_name->line_no = token->line_no;
      type_name->column_no = token->column_no;
      type_name->strname = token->lexeme;
      next_token();
      return (Ast*)type_name;
    } else assert(0);
  } else error("At line %d, column %d: name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parameterList()
{
  Ast_ParameterList* params = arena_push_struct(ast_storage, Ast_ParameterList);
  params->kind = AST_parameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  list_reset(&params->members);
  if (token_is_parameter(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_parameter();
    list_append_item(&params->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parameter();
      list_append_item(&params->members, li, 1);
    }
  }
  return (Ast*)params;
}

internal Ast*
parse_parameter()
{
  if (token_is_parameter(token)) {
    Ast_Parameter* param = arena_push_struct(ast_storage, Ast_Parameter);
    param->kind = AST_parameter;
    param->line_no = token->line_no;
    param->column_no = token->column_no;
    param->direction = parse_direction();
    param->type = parse_typeRef();
    if (token_is_name(token)) {
      param->name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token_is_expression(token)) {
          param->init_expr = parse_expression(1);
        } else error("At line %d, column %d: expression was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)param;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal enum Ast_ParamDirection
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

internal Ast*
parse_packageTypeDeclaration()
{
  if (token->klass == TK_PACKAGE) {
    next_token();
    Ast_PackageTypeDeclaration* package_decl = arena_push_struct(ast_storage, Ast_PackageTypeDeclaration);
    package_decl->kind = AST_packageTypeDeclaration;
    package_decl->line_no = token->line_no;
    package_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      package_decl->name = (Ast*)name;
      package_decl->type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        package_decl->params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)package_decl;
  } else error("At line %d, column %d: `package` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_instantiation(Ast* type_ref)
{
  if (token_is_typeRef(token) || type_ref) {
    Ast_Instantiation* inst_stmt = arena_push_struct(ast_storage, Ast_Instantiation);
    inst_stmt->kind = AST_instantiation;
    inst_stmt->line_no = token->line_no;
    inst_stmt->column_no = token->column_no;
    inst_stmt->type_ref = type_ref ? type_ref : parse_typeRef();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      inst_stmt->args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token_is_name(token)) {
          inst_stmt->name = parse_name();
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
    return (Ast*)inst_stmt;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** PARSER **/

internal Ast*
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

internal Ast*
parse_parserDeclaration(Ast* parser_proto)
{
  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    Ast_ParserDeclaration* parser_decl = arena_push_struct(ast_storage, Ast_ParserDeclaration);
    parser_decl->kind = AST_parserDeclaration;
    parser_decl->line_no = token->line_no;
    parser_decl->column_no = token->column_no;
    parser_decl->proto = parser_proto;
    parser_decl->ctor_params = parse_optConstructorParameters();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      parser_decl->local_elements = parse_parserLocalElements();
      if (token->klass == TK_STATE) {
        parser_decl->states = parse_parserStates();
      } else error("At line %d, column %d: `state` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    return (Ast*)parser_decl;
  } else error("At line %d, column %d: `parser` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parserLocalElements()
{
  Ast_ParserLocalElements* elems = arena_push_struct(ast_storage, Ast_ParserLocalElements);
  elems->kind = AST_parserLocalElements;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  list_reset(&elems->members);
  if (token_is_parserLocalElement(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_parserLocalElement();
    list_append_item(&elems->members, li, 1);
    while (token_is_parserLocalElement(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserLocalElement();
      list_append_item(&elems->members, li, 1);
    }
  }
  return (Ast*)elems;
}

internal Ast*
parse_parserLocalElement()
{
  if (token_is_parserLocalElement(token)) {
    Ast_ParserLocalElement* local_element = arena_push_struct(ast_storage, Ast_ParserLocalElement);
    local_element->kind = AST_parserLocalElement;
    local_element->line_no = token->line_no;
    local_element->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_element->element = parse_variableDeclaration(0);
      return (Ast*)local_element;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_element->element = parse_instantiation(type_ref);
        return (Ast*)local_element;
      } else if (token_is_name(token)) {
        local_element->element = parse_variableDeclaration(type_ref);
        return (Ast*)local_element;
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("At line %d, column %d: local declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parserTypeDeclaration()
{
  if (token->klass == TK_PARSER) {
    next_token();
    Ast_ParserTypeDeclaration* proto = arena_push_struct(ast_storage, Ast_ParserTypeDeclaration);
    proto->kind = AST_parserTypeDeclaration;
    proto->line_no = token->line_no; 
    proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      proto->name = (Ast*)name;
      proto->type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)proto;
  } else error("At line %d, column %d: `parser` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parserStates()
{
  Ast_ParserStates* states = arena_push_struct(ast_storage, Ast_ParserStates);
  states->kind = AST_parserStates;
  states->line_no = token->line_no;
  states->column_no = token->column_no;
  list_reset(&states->members);
  if (token->klass == TK_STATE) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_parserState();
    list_append_item(&states->members, li, 1);
    while (token->klass == TK_STATE) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserState();
      list_append_item(&states->members, li, 1);
    }
  }
  return (Ast*)states;
}

internal Ast*
parse_parserState()
{
  if (token->klass == TK_STATE) {
    next_token();
    Ast_ParserState* state = arena_push_struct(ast_storage, Ast_ParserState);
    state->kind = AST_parserState;
    state->line_no = token->line_no;
    state->column_no = token->column_no;
    state->name = parse_name();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      state->stmt_list = parse_parserStatements();
      state->transition_stmt = parse_transitionStatement();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)state;
  } else error("At line %d, column %d: `state` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parserStatements()
{
  Ast_ParserStatements* stmts = arena_push_struct(ast_storage, Ast_ParserStatements);
  stmts->kind = AST_parserStatements;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  list_reset(&stmts->members);
  if (token_is_parserStatement(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_parserStatement();
    list_append_item(&stmts->members, li, 1);
    while (token_is_parserStatement(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserStatement();
      list_append_item(&stmts->members, li, 1);
    }
  }
  return (Ast*)stmts;
}

internal Ast*
parse_parserStatement()
{
  if (token_is_parserStatement(token)) {
    Ast_ParserStatement* parser_stmt = arena_push_struct(ast_storage, Ast_ParserStatement);
    parser_stmt->kind = AST_parserStatement;
    parser_stmt->line_no = token->line_no;
    parser_stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token_is_name(token)) {
        parser_stmt->stmt = parse_variableDeclaration(type_ref);
        return (Ast*)parser_stmt;
      } else {
        parser_stmt->stmt = parse_directApplication(type_ref);
        return (Ast*)parser_stmt;
      }
    } else if (token_is_assignmentOrMethodCallStatement(token)) {
      parser_stmt->stmt = parse_assignmentOrMethodCallStatement();
      return (Ast*)parser_stmt;
    } else if (token->klass == TK_BRACE_OPEN) {
      parser_stmt->stmt = parse_parserBlockStatement();
      return (Ast*)parser_stmt;
    } else if (token->klass == TK_CONST) {
      parser_stmt->stmt = parse_variableDeclaration(0);
      return (Ast*)parser_stmt;
    } else if (token->klass == TK_SEMICOLON) {
      Ast* stmt = arena_push_struct(ast_storage, Ast);
      stmt->kind = AST_emptyStatement;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      parser_stmt->stmt = stmt;
      return (Ast*)parser_stmt;
    } else assert(0);
  } else error("At line %d, column %d: statement was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_parserBlockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast_ParserBlockStatement* stmt = arena_push_struct(ast_storage, Ast_ParserBlockStatement);
    stmt->kind = AST_parserBlockStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    stmt->stmt_list = parse_parserStatements();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `}` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)stmt;
  } else error("At line %d, column %d: `{` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_transitionStatement()
{
  if (token->klass == TK_TRANSITION) {
    next_token();
    Ast_TransitionStatement* transition = arena_push_struct(ast_storage, Ast_TransitionStatement);
    transition->kind = AST_transitionStatement;
    transition->line_no = token->line_no;
    transition->column_no = token->column_no;
    transition->stmt = parse_stateExpression();
    return (Ast*)transition;
  } else error("At line %d, column %d: `transition` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_stateExpression()
{
  if (token_is_name(token) || token->klass == TK_SELECT) {
    Ast_StateExpression* state_expr = arena_push_struct(ast_storage, Ast_StateExpression);
    state_expr->kind = AST_stateExpression;
    state_expr->line_no = token->line_no;
    state_expr->column_no = token->column_no;
    if (token_is_name(token)) {
      state_expr->expr = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
      return (Ast*)state_expr;
    } else if (token->klass == TK_SELECT) {
      state_expr->expr = parse_selectExpression();
      return (Ast*)state_expr;
    } else assert(0);
  } else error("At line %d, column %d: state expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_selectExpression()
{
  if (token->klass == TK_SELECT) {
    next_token();
    Ast_SelectExpression* select_expr = arena_push_struct(ast_storage, Ast_SelectExpression);
    select_expr->kind = AST_selectExpression;
    select_expr->line_no = token->line_no;
    select_expr->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      select_expr->expr_list = parse_expressionList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          select_expr->case_list = parse_selectCaseList();
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
    return (Ast*)select_expr;
  } else error("At line %d, column %d: `select` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_selectCaseList()
{
  Ast_SelectCaseList* cases = arena_push_struct(ast_storage, Ast_SelectCaseList);
  cases->kind = AST_selectCaseList;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  list_reset(&cases->members);
  if (token_is_selectCase(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_selectCase();
    list_append_item(&cases->members, li, 1);
    while (token_is_selectCase(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_selectCase();
      list_append_item(&cases->members, li, 1);
    }
  }
  return (Ast*)cases;
}

internal Ast*
parse_selectCase()
{
  if (token_is_keysetExpression(token)) {
    Ast_SelectCase* select_case = arena_push_struct(ast_storage, Ast_SelectCase);
    select_case->kind = AST_selectCase;
    select_case->line_no = token->line_no;
    select_case->column_no = token->column_no;
    select_case->keyset_expr = parse_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_name(token)) {
        select_case->name = parse_name();
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("At line %d, column %d: `;` expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)select_case;
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_keysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN || token_is_simpleKeysetExpression(token)) {
    Ast_KeysetExpression* keyset_expr = arena_push_struct(ast_storage, Ast_KeysetExpression);
    keyset_expr->kind = AST_keysetExpression;
    keyset_expr->line_no = token->line_no;
    keyset_expr->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      keyset_expr->expr = parse_tupleKeysetExpression();
      return (Ast*)keyset_expr;
    } else if (token_is_simpleKeysetExpression(token)) {
      keyset_expr->expr = parse_simpleKeysetExpression();
      return (Ast*)keyset_expr;
    } else assert(0);
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_tupleKeysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    Ast_TupleKeysetExpression* tuple_keyset = arena_push_struct(ast_storage, Ast_TupleKeysetExpression);
    tuple_keyset->kind = AST_tupleKeysetExpression;
    tuple_keyset->line_no = token->line_no;
    tuple_keyset->column_no = token->column_no;
    tuple_keyset->expr_list = parse_simpleExpressionList();
    if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `)` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)tuple_keyset;
  } else error("At line %d, column %d: `(` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_simpleExpressionList()
{
  Ast_SimpleExpressionList* exprs = arena_push_struct(ast_storage, Ast_SimpleExpressionList);
  exprs->kind = AST_simpleExpressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  list_reset(&exprs->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_simpleKeysetExpression();
    list_append_item(&exprs->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_simpleKeysetExpression();
      list_append_item(&exprs->members, li, 1);
    }
  }
  return (Ast*)exprs;
}

internal Ast*
parse_simpleKeysetExpression()
{
  if (token_is_simpleKeysetExpression(token)) {
    Ast_SimpleKeysetExpression* simple_keyset = arena_push_struct(ast_storage, Ast_SimpleKeysetExpression);
    simple_keyset->kind = AST_simpleKeysetExpression;
    simple_keyset->line_no = token->line_no;
    simple_keyset->column_no = token->column_no;
    if (token_is_expression(token)) {
      simple_keyset->expr = parse_expression(1);
      return (Ast*)simple_keyset;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast_Default* default_keyset = arena_push_struct(ast_storage, Ast_Default);
      default_keyset->kind = AST_default;
      default_keyset->line_no = token->line_no;
      default_keyset->column_no = token->column_no;
      simple_keyset->expr = (Ast*)default_keyset;
      return (Ast*)simple_keyset;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast_Dontcare* dontcare_keyset = arena_push_struct(ast_storage, Ast_Dontcare);
      dontcare_keyset->kind = AST_dontcare;
      dontcare_keyset->line_no = token->line_no;
      dontcare_keyset->column_no = token->column_no;
      simple_keyset->expr = (Ast*)dontcare_keyset;
      return (Ast*)simple_keyset;
    }
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** CONTROL **/

internal Ast*
parse_controlDeclaration(Ast* control_proto)
{
  if (token->klass == TK_PARENTH_OPEN || token->klass == TK_BRACE_OPEN) {
    Ast_ControlDeclaration* control_decl = arena_push_struct(ast_storage, Ast_ControlDeclaration);
    control_decl->kind = AST_controlDeclaration;
    control_decl->line_no = token->line_no;
    control_decl->column_no = token->column_no;
    control_decl->proto = control_proto;
    control_decl->ctor_params = parse_optConstructorParameters();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      control_decl->local_decls = parse_controlLocalDeclarations();
      if (token->klass == TK_APPLY) {
        next_token();
        control_decl->apply_stmt = parse_blockStatement();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                      token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `apply` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    return (Ast*)control_decl;
  } else error("At line %d, column %d: `control` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_controlTypeDeclaration()
{
  if (token->klass == TK_CONTROL) {
    next_token();
    Ast_ControlTypeDeclaration* proto = arena_push_struct(ast_storage, Ast_ControlTypeDeclaration);
    proto->kind = AST_controlTypeDeclaration;
    proto->line_no = token->line_no;
    proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      proto->name = (Ast*)name;
      proto->type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)proto;
  } else error("At line %d, column %d: `control` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_controlLocalDeclaration()
{
  if (token_is_controlLocalDeclaration(token)) {
    Ast_ControlLocalDeclaration* local_decl = arena_push_struct(ast_storage, Ast_ControlLocalDeclaration);
    local_decl->kind = AST_controlLocalDeclaration;
    local_decl->line_no = token->line_no;
    local_decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_decl->decl = parse_variableDeclaration(0);
      return (Ast*)local_decl;
    } else if (token->klass == TK_ACTION) {
      local_decl->decl = parse_actionDeclaration();
      return (Ast*)local_decl;
    } else if (token->klass == TK_TABLE) {
      local_decl->decl = parse_tableDeclaration();
      return (Ast*)local_decl;
    } else if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        local_decl->decl = parse_instantiation(type_ref);
        return (Ast*)local_decl;
      } else if (token_is_name(token)) {
        local_decl->decl = parse_variableDeclaration(type_ref);
        return (Ast*)local_decl;
      } else error("At line %d, column %d: unexpected token `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    } else assert(0);
  } else error("At line %d, column %d: local declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_controlLocalDeclarations()
{
  Ast_ControlLocalDeclarations* decls = arena_push_struct(ast_storage, Ast_ControlLocalDeclarations);
  decls->kind = AST_controlLocalDeclarations;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  list_reset(&decls->members);
  if (token_is_controlLocalDeclaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_controlLocalDeclaration();
    list_append_item(&decls->members, li, 1);
    while (token_is_controlLocalDeclaration(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_controlLocalDeclaration();
      list_append_item(&decls->members, li, 1);
    }
  }
  return (Ast*)decls;
}

/** EXTERN **/

internal Ast*
parse_externDeclaration()
{
  if (token->klass == TK_EXTERN) {
    next_token();
    Ast_ExternDeclaration* extern_decl = arena_push_struct(ast_storage, Ast_ExternDeclaration);
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
      extern_decl->decl = parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)extern_decl;
    } else {
      Ast_ExternTypeDeclaration* extern_type = arena_push_struct(ast_storage, Ast_ExternTypeDeclaration);
      extern_type->kind = AST_externTypeDeclaration;
      extern_type->line_no = token->line_no;
      extern_type->column_no = token->column_no;
      extern_type->name = parse_nonTypeName();
      Ast_Name* name = (Ast_Name*)extern_type->name;
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      extern_type->type_params = parse_optTypeParameters();
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        extern_type->method_protos = parse_methodPrototypes();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      extern_decl->decl = (Ast*)extern_type;
      return (Ast*)extern_decl;
    }
  } else error("At line %d, column %d: `extern` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_methodPrototypes()
{
  Ast_MethodPrototypes* protos = arena_push_struct(ast_storage, Ast_MethodPrototypes);
  protos->kind = AST_methodPrototypes;
  protos->line_no = token->line_no;
  protos->column_no = token->column_no;
  list_reset(&protos->members);
  if (token_is_methodPrototype(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_methodPrototype();
    list_append_item(&protos->members, li, 1);
    while (token_is_methodPrototype(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_methodPrototype();
      list_append_item(&protos->members, li, 1);
    }
  }
  return (Ast*)protos;
}

internal Ast*
parse_functionPrototype(Ast* return_type)
{
  if (token_is_typeOrVoid(token) || return_type) {
    Ast_FunctionPrototype* proto = arena_push_struct(ast_storage, Ast_FunctionPrototype);
    proto->kind = AST_functionPrototype;
    proto->line_no = token->line_no;
    proto->column_no = token->column_no;
    if (return_type) {
      proto->return_type = return_type;
    } else {
      Ast* return_type = parse_typeOrVoid();
      if (return_type->kind == AST_name) {
        Ast_Name* name = (Ast_Name*)return_type;
        declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
        Ast_TypeRef* type_ref = arena_push_struct(ast_storage, Ast_TypeRef);
        type_ref->kind = AST_typeRef;
        type_ref->line_no = token->line_no;
        type_ref->column_no = token->column_no;
        type_ref->type = (Ast*)name;
        return_type = (Ast*)type_ref;
      }
      proto->return_type = return_type;
    }
    if (token_is_name(token)) {
      proto->name = parse_name();
      proto->type_params = parse_optTypeParameters();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: function name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)proto;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_methodPrototype()
{
  if (token_is_methodPrototype(token)) {
    if (token->klass == TK_TYPE_IDENTIFIER && peek_token()->klass == TK_PARENTH_OPEN) {
      /* Constructor */
      Ast_FunctionPrototype* proto = arena_push_struct(ast_storage, Ast_FunctionPrototype);
      proto->kind = AST_functionPrototype;
      proto->line_no = token->line_no;
      proto->column_no = token->column_no;
      proto->name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        proto->params = parse_parameterList();
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
      return (Ast*)proto;
    } else if (token_is_typeOrVoid(token)) {
      Ast_FunctionPrototype* proto = (Ast_FunctionPrototype*)parse_functionPrototype(0);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)proto;
    } else error("At line %d, column %d: type was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** TYPES **/

internal Ast*
parse_typeRef()
{
  if (token_is_typeRef(token)) {
    Ast_TypeRef* type_ref = arena_push_struct(ast_storage, Ast_TypeRef);
    type_ref->kind = AST_typeRef;
    type_ref->line_no = token->line_no;
    type_ref->column_no = token->column_no;
    if (token_is_baseType(token)) {
      type_ref->type = parse_baseType();
      return (Ast*)type_ref;
    } else if (token_is_typeName(token)) {
      type_ref->type = parse_namedType();
      return (Ast*)type_ref;
    } else if (token->klass == TK_TUPLE) {
      type_ref->type = parse_tupleType();
      return (Ast*)type_ref;
    } else assert(0);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_namedType()
{
  if (token_is_typeName(token)) {
    Ast* named_type = parse_prefixedType();
    if (token->klass == TK_ANGLE_OPEN) {
      Ast_SpecializedType* specd_type = (Ast_SpecializedType*)parse_specializedType();
      specd_type->name = named_type;
      return (Ast*)specd_type;
    } else if (token->klass == TK_BRACKET_OPEN) {
      Ast_HeaderStackType* stack_type = (Ast_HeaderStackType*)parse_headerStackType();
      stack_type->name = named_type;
      return (Ast*)stack_type;
    }
    return (Ast*)named_type;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_prefixedType()
{
  bool is_prefixed = false;
  if (token->klass == TK_DOT) {
    next_token();
    is_prefixed = true;
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    type_name->strname = token->lexeme;
    type_name->is_prefixed = is_prefixed;
    next_token();
    return (Ast*)type_name;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_tupleType()
{
  if (token->klass == TK_TUPLE) {
    next_token();
    Ast_TupleType* tuple = arena_push_struct(ast_storage, Ast_TupleType);
    tuple->kind = AST_tupleType;
    tuple->line_no = token->line_no;
    tuple->column_no = token->column_no;
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      tuple->type_args = parse_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `>` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `<` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)tuple;
  } else error("At line %d, column %d: `tuple` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_headerStackType()
{
  if (token->klass == TK_BRACKET_OPEN) {
    next_token();
    Ast_HeaderStackType* stack = arena_push_struct(ast_storage, Ast_HeaderStackType);
    stack->kind = AST_headerStackType;
    stack->line_no = token->line_no;
    stack->column_no = token->column_no;
    if (token_is_expression(token)) {
      stack->stack_expr = parse_expression(1);
      if (token->klass == TK_BRACKET_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `]` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: expression expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)stack;
  } else error("At line %d, column %d: `[` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_specializedType()
{
  if (token->klass == TK_ANGLE_OPEN) {
    next_token();
    Ast_SpecializedType* type = arena_push_struct(ast_storage, Ast_SpecializedType);
    type->kind = AST_specializedType;
    type->line_no = token->line_no;
    type->column_no = token->column_no;
    type->type_args = parse_typeArgumentList();
    if (token->klass == TK_ANGLE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `>` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)type;
  } else error("At line %d, column %d: `<` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_baseType()
{
  if (token_is_baseType(token)) {
    Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
    type_name->kind = AST_name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    if (token->klass == TK_BOOL) {
      Ast_BooleanType* bool_type = arena_push_struct(ast_storage, Ast_BooleanType);
      bool_type->kind = AST_baseTypeBoolean;
      bool_type->line_no = token->line_no;
      bool_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      bool_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)bool_type;
    } else if (token->klass == TK_ERROR) {
      Ast_ErrorType* error_type = arena_push_struct(ast_storage, Ast_ErrorType);
      error_type->kind = AST_baseTypeError;
      error_type->line_no = token->line_no;
      error_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      error_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)error_type;
    } else if (token->klass == TK_INT) {
      Ast_IntegerType* int_type = arena_push_struct(ast_storage, Ast_IntegerType);
      int_type->kind = AST_baseTypeInteger;
      int_type->line_no = token->line_no;
      int_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      int_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        int_type->size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
      return (Ast*)int_type;
    } else if (token->klass == TK_BIT) {
      Ast_BitType* bit_type = arena_push_struct(ast_storage, Ast_BitType);
      bit_type->kind = AST_baseTypeBit;
      bit_type->line_no = token->line_no;
      bit_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      bit_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        bit_type->size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
      return (Ast*)bit_type;
    } else if (token->klass == TK_VARBIT) {
      Ast_VarbitType* varbit_type = arena_push_struct(ast_storage, Ast_VarbitType);
      varbit_type->kind = AST_baseTypeVarbit;
      varbit_type->line_no = token->line_no;
      varbit_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      varbit_type->name = (Ast*)type_name;
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: '<' was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)varbit_type;
    } else if (token->klass == TK_STRING) {
      Ast_StringType* string_type = arena_push_struct(ast_storage, Ast_StringType);
      string_type->kind = AST_baseTypeString;
      string_type->line_no = token->line_no;
      string_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      string_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)string_type;
    } else if (token->klass == TK_VOID) {
      Ast_VoidType* void_type = arena_push_struct(ast_storage, Ast_VoidType);
      void_type->kind = AST_baseTypeVoid;
      void_type->line_no = token->line_no;
      void_type->column_no = token->column_no;
      type_name->strname = token->lexeme;
      void_type->name = (Ast*)type_name;
      next_token();
      return (Ast*)void_type;
    } else assert(0);
  } else error("At line %d, column %d: base type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_integerTypeSize()
{
  Ast_IntegerTypeSize* type_size = arena_push_struct(ast_storage, Ast_IntegerTypeSize);
  type_size->kind = AST_integerTypeSize;
  type_size->line_no = token->line_no;
  type_size->column_no = token->column_no;
  if (token->klass == TK_INTEGER_LITERAL) {
    type_size->size = parse_integer();
  } else if (token->klass == TK_PARENTH_OPEN) {
    /* TODO
    type_size->size = parse_expression(1); */
    error("At line %d, column %d: integer was expected, got `%s`.",
          token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: `(` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  return (Ast*)type_size;
}

internal Ast*
parse_typeOrVoid()
{
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      Ast* type = parse_typeRef();
      return type;
    } else if (token->klass == TK_VOID) {
      return (Ast*)parse_baseType();
    } else if (token->klass == TK_IDENTIFIER) {
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_name;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->strname = token->lexeme;
      next_token();
      return (Ast*)name;
    } else assert(0);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
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

internal Ast*
parse_typeParameterList()
{
  Ast_TypeParameterList* params = arena_push_struct(ast_storage, Ast_TypeParameterList);
  params->kind = AST_typeParameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  list_reset(&params->members);
  if (token_is_typeParameterList(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    Ast_Name* name = (Ast_Name*)parse_name();
    declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
    li->object = name;
    list_append_item(&params->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      li->object = name;
      list_append_item(&params->members, li, 1);
    }
  }
  return (Ast*)params;
}

internal Ast*
parse_realTypeArg()
{
  if (token_is_realTypeArg(token)) {
    Ast_RealTypeArg* type_arg = arena_push_struct(ast_storage, Ast_RealTypeArg);
    type_arg->kind = AST_realTypeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast_Dontcare* dontcare_arg = arena_push_struct(ast_storage, Ast_Dontcare);
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->arg = (Ast*)dontcare_arg;
      return (Ast*)type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->arg = parse_typeRef();
      return (Ast*)type_arg;
    } else assert(0);
  } else error("At line %d, column %d: type argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_typeArg()
{
  if (token_is_typeArg(token)) {
    Ast_TypeArg* type_arg = arena_push_struct(ast_storage, Ast_TypeArg);
    type_arg->kind = AST_typeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast_Dontcare* dontcare_arg = arena_push_struct(ast_storage, Ast_Dontcare);
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->arg = (Ast*)dontcare_arg;
      return (Ast*)type_arg;
    } else if (token_is_typeRef(token)) {
      type_arg->arg = parse_typeRef();
      return (Ast*)type_arg;
    } else if (token_is_nonTypeName(token)) {
      type_arg->arg = parse_nonTypeName();
      return (Ast*)type_arg;
    } else assert(0);
  } else error("At line %d, column %d: type argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_realTypeArgumentList()
{
  Ast_RealTypeArgumentList* args = arena_push_struct(ast_storage, Ast_RealTypeArgumentList);
  args->kind = AST_realTypeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_reset(&args->members);
  if (token_is_realTypeArg(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_realTypeArg();
    list_append_item(&args->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_realTypeArg();
      list_append_item(&args->members, li, 1);
    }
  }
  return (Ast*)args;
}

internal Ast*
parse_typeArgumentList()
{
  Ast_TypeArgumentList* args = arena_push_struct(ast_storage, Ast_TypeArgumentList);
  args->kind = AST_typeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_reset(&args->members);
  if (token_is_typeArg(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_typeArg();
    list_append_item(&args->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_typeArg();
      list_append_item(&args->members, li, 1);
    }
  }
  return (Ast*)args;
}

internal Ast*
parse_typeDeclaration()
{
  if (token_is_typeDeclaration(token)) {
    Ast_TypeDeclaration* type_decl = arena_push_struct(ast_storage, Ast_TypeDeclaration);
    type_decl->kind = AST_typeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token_is_derivedTypeDeclaration(token)) {
      type_decl->decl = parse_derivedTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
      type_decl->decl = parse_typedefDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_PARSER) {
      type_decl->decl = parse_parserTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_CONTROL) {
      type_decl->decl = parse_controlTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_PACKAGE) {
      type_decl->decl = parse_packageTypeDeclaration();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)type_decl;
    } else assert(0);
  } else error("At line %d, column %d: type declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme); 
  assert(0);
  return 0;
}

internal Ast*
parse_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    Ast_DerivedTypeDeclaration* type_decl = arena_push_struct(ast_storage, Ast_DerivedTypeDeclaration);
    type_decl->kind = AST_derivedTypeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token->klass == TK_HEADER) {
      type_decl->decl = parse_headerTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_HEADER_UNION) {
      type_decl->decl = parse_headerUnionDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_STRUCT) {
      type_decl->decl = parse_structTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_ENUM) {
      type_decl->decl = parse_enumDeclaration();
      return (Ast*)type_decl;
    } else assert(0);
  } else error("At line %d, column %d: structure declaration was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_headerTypeDeclaration()
{
  if (token->klass == TK_HEADER) {
    next_token();
    Ast_HeaderTypeDeclaration* header_decl = arena_push_struct(ast_storage, Ast_HeaderTypeDeclaration);
    header_decl->kind = AST_headerTypeDeclaration;
    header_decl->line_no = token->line_no;
    header_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      header_decl->name = (Ast*)name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        header_decl->fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token(token);
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)header_decl;
  } else error("At line %d, column %d: `header` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_headerUnionDeclaration()
{
  if (token->klass == TK_HEADER_UNION) {
    next_token();
    Ast_HeaderUnionDeclaration* union_decl = arena_push_struct(ast_storage, Ast_HeaderUnionDeclaration);
    union_decl->kind = AST_headerUnionDeclaration;
    union_decl->line_no = token->line_no;
    union_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      union_decl->name = (Ast*)name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        union_decl->fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)union_decl;
  } else error("At line %d, column %d: `header_union` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_structTypeDeclaration()
{
  if (token->klass == TK_STRUCT) {
    next_token();
    Ast_StructTypeDeclaration* struct_decl = arena_push_struct(ast_storage, Ast_StructTypeDeclaration);
    struct_decl->kind = AST_structTypeDeclaration;
    struct_decl->line_no = token->line_no;
    struct_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      struct_decl->name = (Ast*)name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        struct_decl->fields = parse_structFieldList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `{` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)struct_decl;
  } else error("At line %d, column %d: `struct` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_structFieldList()
{
  Ast_StructFieldList* fields = arena_push_struct(ast_storage, Ast_StructFieldList);
  fields->kind = AST_structFieldList;
  fields->line_no = token->line_no;
  fields->column_no = token->column_no;
  list_reset(&fields->members);
  if (token_is_structField(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_structField();
    list_append_item(&fields->members, li, 1);
    while (token_is_structField(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_structField();
      list_append_item(&fields->members, li, 1);
    }
  }
  return (Ast*)fields;
}

internal Ast*
parse_structField()
{
  if (token_is_structField(token)) {
    Ast_StructField* field = arena_push_struct(ast_storage, Ast_StructField);
    field->kind = AST_structField;
    field->line_no = token->line_no;
    field->column_no = token->column_no;
    field->type = parse_typeRef();
    if (token_is_name(token)) {
      field->name = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)field;
  } else error("At line %d, column %d: struct field was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_enumDeclaration()
{
  if (token->klass == TK_ENUM) {
    next_token();
    Ast_EnumDeclaration* enum_decl = arena_push_struct(ast_storage, Ast_EnumDeclaration);
    enum_decl->kind = AST_enumDeclaration;
    enum_decl->line_no = token->line_no;
    enum_decl->column_no = token->column_no;
    if (token->klass == TK_BIT) {
      next_token();
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        if (token->klass == TK_INTEGER_LITERAL) {
          enum_decl->type_size = parse_integer();
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
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
      enum_decl->name = (Ast*)name;
      if (token->klass == TK_BRACE_OPEN) {
        next_token();
        if (token_is_specifiedIdentifier(token)) {
          enum_decl->fields = parse_specifiedIdentifierList();
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
    return (Ast*)enum_decl;
  } else error("At line %d, column %d: `enum` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_errorDeclaration()
{
  if (token->klass == TK_ERROR) {
    next_token();
    Ast_ErrorDeclaration* error_decl = arena_push_struct(ast_storage, Ast_ErrorDeclaration);
    error_decl->kind = AST_errorDeclaration;
    error_decl->line_no = token->line_no;
    error_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        if (token_is_name(token)) {
          error_decl->fields = parse_identifierList();
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
    return (Ast*)error_decl;
  } else error("At line %d, column %d: `error` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_matchKindDeclaration()
{
  if (token->klass == TK_MATCH_KIND) {
    next_token();
    Ast_MatchKindDeclaration* match_decl = arena_push_struct(ast_storage, Ast_MatchKindDeclaration);
    match_decl->kind = AST_matchKindDeclaration;
    match_decl->line_no = token->line_no;
    match_decl->column_no = token->column_no;
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_name(token)) {
        match_decl->fields = parse_identifierList();
        if (token->klass == TK_BRACE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)match_decl;
  } else error("At line %d, column %d: `match_kind` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_identifierList()
{
  Ast_IdentifierList* ids = arena_push_struct(ast_storage, Ast_IdentifierList);
  ids->kind = AST_identifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  list_reset(&ids->members);
  if (token_is_name(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_name();
    list_append_item(&ids->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_name();
      list_append_item(&ids->members, li, 1);
    }
  }
  return (Ast*)ids;
}

internal Ast*
parse_specifiedIdentifierList()
{
  Ast_SpecifiedIdentifierList* ids = arena_push_struct(ast_storage, Ast_SpecifiedIdentifierList);
  ids->kind = AST_specifiedIdentifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  list_reset(&ids->members);
  if (token_is_specifiedIdentifier(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_specifiedIdentifier();
    list_append_item(&ids->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_specifiedIdentifier();
      list_append_item(&ids->members, li, 1);
    }
  }
  return (Ast*)ids;
}

internal Ast*
parse_specifiedIdentifier()
{
  if (token_is_specifiedIdentifier(token)) {
    Ast_SpecifiedIdentifier* id = arena_push_struct(ast_storage, Ast_SpecifiedIdentifier);
    id->kind = AST_specifiedIdentifier;
    id->line_no = token->line_no;
    id->column_no = token->column_no;
    id->name = parse_name();
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->init_expr = parse_expression(1);
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
    return (Ast*)id;
  } else error("At line %d, column %d: name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_typedefDeclaration()
{
  if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
    next_token();
    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      Ast_TypedefDeclaration* type_decl = arena_push_struct(ast_storage, Ast_TypedefDeclaration);
      type_decl->kind = AST_typedefDeclaration;
      type_decl->line_no = token->line_no;
      type_decl->column_no = token->column_no;
      if (token_is_typeRef(token)) {
        type_decl->type_ref = parse_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        Ast_Name* name = (Ast_Name*)parse_name();
        declare_type_name(ast_storage, current_scope, name->strname, name->line_no, name->column_no);
        type_decl->name = (Ast*)name;
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("At line %d, column %d: `;` expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)type_decl;
    } else error("At line %d, column %d: type was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: type definition was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** STATEMENTS **/

internal Ast*
parse_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    Ast_LvalueExpression* lvalue = (Ast_LvalueExpression*)parse_lvalue();
    if (token->klass == TK_ANGLE_OPEN) {
      next_token();
      lvalue->type_args = parse_typeArgumentList();
      if (token->klass == TK_ANGLE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `>` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      Ast_FunctionCall* call_stmt = arena_push_struct(ast_storage, Ast_FunctionCall);
      call_stmt->kind = AST_functionCall;
      call_stmt->line_no = token->line_no;
      call_stmt->column_no = token->column_no;
      call_stmt->lhs_expr = (Ast*)lvalue;
      call_stmt->args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)call_stmt;
    } else if (token->klass == TK_EQUAL) {
      next_token();
      Ast_AssignmentStatement* assign_stmt = arena_push_struct(ast_storage, Ast_AssignmentStatement);
      assign_stmt->kind = AST_assignmentStatement;
      assign_stmt->line_no = token->line_no;
      assign_stmt->column_no = token->column_no;
      assign_stmt->lhs_expr = (Ast*)lvalue;
      assign_stmt->rhs_expr = parse_expression(1);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)assign_stmt;
    } else error("At line %d, column %d: assignment or function call was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  } else error("At line %d, column %d: lvalue was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_returnStatement()
{
  if (token->klass == TK_RETURN) {
    next_token();
    Ast_ReturnStatement* return_stmt = arena_push_struct(ast_storage, Ast_ReturnStatement);
    return_stmt->kind = AST_returnStatement;
    return_stmt->line_no = token->line_no;
    return_stmt->column_no = token->column_no;
    if (token_is_expression(token))
      return_stmt->expr = parse_expression(1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)return_stmt;
  } else error("At line %d, column %d: `return` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_exitStatement()
{
  if (token->klass == TK_EXIT) {
    next_token();
    Ast_ExitStatement* exit_stmt = arena_push_struct(ast_storage, Ast_ExitStatement);
    exit_stmt->kind = AST_exitStatement;
    exit_stmt->line_no = token->line_no;
    exit_stmt->column_no = token->column_no;
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)exit_stmt;
  } else error("At line %d, column %d: `exit` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_conditionalStatement()
{
  if (token->klass == TK_IF) {
    next_token();
    Ast_ConditionalStatement* if_stmt = arena_push_struct(ast_storage, Ast_ConditionalStatement);
    if_stmt->kind = AST_conditionalStatement;
    if_stmt->line_no = token->line_no;
    if_stmt->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_expression(token)) {
        if_stmt->cond_expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token_is_statement(token)) {
            if_stmt->stmt = parse_statement(0);
            if (token->klass == TK_ELSE) {
              next_token();
              if (token_is_statement(token)) {
                if_stmt->else_stmt = parse_statement(0);
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
    return (Ast*)if_stmt;
  } else error("At line %d, column %d: `if` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_directApplication(Ast* type_name)
{
  if (token_is_typeName(token) || type_name) {
    Ast_DirectApplication* apply_stmt = arena_push_struct(ast_storage, Ast_DirectApplication);
    apply_stmt->kind = AST_directApplication;
    apply_stmt->line_no = token->line_no;
    apply_stmt->column_no = token->column_no;
    apply_stmt->name = type_name ? type_name : parse_prefixedType();
    if (token->klass == TK_DOT) {
      next_token();
      if (token->klass == TK_APPLY) {
        next_token();
        if (token->klass == TK_PARENTH_OPEN) {
          next_token();
          apply_stmt->args = parse_argumentList();
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
    return (Ast*)apply_stmt;
  } else error("At line %d, column %d: type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_statement(Ast* type_name)
{
  if (token_is_statement(token)) {
    Ast_Statement* stmt = arena_push_struct(ast_storage, Ast_Statement);
    stmt->kind = AST_statement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token_is_typeName(token) || type_name) {
      stmt->stmt = parse_directApplication(type_name);
      return (Ast*)stmt;
    } else if (token_is_assignmentOrMethodCallStatement(token)) {
      stmt->stmt = parse_assignmentOrMethodCallStatement();
      return (Ast*)stmt;
    } else if (token->klass == TK_IF) {
      stmt->stmt = parse_conditionalStatement();
      return (Ast*)stmt;
    } else if (token->klass == TK_SEMICOLON) {
      next_token();
      Ast* empty_stmt = arena_push_struct(ast_storage, Ast);
      empty_stmt->kind = AST_emptyStatement;
      empty_stmt->line_no = token->line_no;
      empty_stmt->column_no = token->column_no;
      stmt->stmt = empty_stmt;
      return (Ast*)stmt;
    } else if (token->klass == TK_BRACE_OPEN) {
      stmt->stmt = parse_blockStatement();
      return (Ast*)stmt;
    } else if (token->klass == TK_EXIT) {
      stmt->stmt = parse_exitStatement();
      return (Ast*)stmt;
    } else if (token->klass == TK_RETURN) {
      stmt->stmt = parse_returnStatement();
      return (Ast*)stmt;
    } else if (token->klass == TK_SWITCH) {
      stmt->stmt = parse_switchStatement();
      return (Ast*)stmt;
    }
  } else error("At line %d, column %d: statement was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_blockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast_BlockStatement* block_stmt = arena_push_struct(ast_storage, Ast_BlockStatement);
    block_stmt->kind = AST_blockStatement;
    block_stmt->line_no = token->line_no;
    block_stmt->column_no = token->column_no;
    block_stmt->stmt_list = parse_statementOrDeclList();
    if (token->klass == TK_BRACE_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `}` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)block_stmt;
  } else error("At line %d, column %d: `{` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_statementOrDeclList()
{
  Ast_StatementOrDeclList* stmts = arena_push_struct(ast_storage, Ast_StatementOrDeclList);
  stmts->kind = AST_statementOrDeclList;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  list_reset(&stmts->members);
  if (token_is_statementOrDeclaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_statementOrDeclaration();
    list_append_item(&stmts->members, li, 1);
    while (token_is_statementOrDeclaration(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_statementOrDeclaration();
      list_append_item(&stmts->members, li, 1);
    }
  }
  return (Ast*)stmts;
}

internal Ast*
parse_switchStatement()
{
  if (token->klass == TK_SWITCH) {
    next_token();
    Ast_SwitchStatement* stmt = arena_push_struct(ast_storage, Ast_SwitchStatement);
    stmt->kind = AST_switchStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      stmt->expr = parse_expression(1);
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          stmt->switch_cases = parse_switchCases();
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
    return (Ast*)stmt;
  } else error("At line %d, column %d: `switch` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_switchCases()
{
  Ast_SwitchCases* cases = arena_push_struct(ast_storage, Ast_SwitchCases);
  cases->kind = AST_switchCases;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  list_reset(&cases->members);
  if (token_is_switchLabel(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_switchCase();
    list_append_item(&cases->members, li, 1);
    while (token_is_switchLabel(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_switchCase();
      list_append_item(&cases->members, li, 1);
    }
  }
  return (Ast*)cases;
}

internal Ast*
parse_switchCase()
{
  if (token_is_switchLabel(token)) {
    Ast_SwitchCase* switch_case = arena_push_struct(ast_storage, Ast_SwitchCase);
    switch_case->kind = AST_switchCase;
    switch_case->line_no = token->line_no;
    switch_case->column_no = token->column_no;
    switch_case->label = parse_switchLabel();
    if (token->klass == TK_COLON) {
      next_token();
      if (token->klass == TK_BRACE_OPEN) {
        switch_case->stmt = parse_blockStatement();
      }
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)switch_case;
  } else error("At line %d, column %d: switch label was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_switchLabel()
{
  if (token_is_switchLabel(token)) {
    Ast_SwitchLabel* switch_label = arena_push_struct(ast_storage, Ast_SwitchLabel);
    switch_label->kind = AST_switchLabel;
    switch_label->line_no = token->line_no;
    switch_label->column_no = token->column_no;
    if (token_is_name(token)) {
      switch_label->label = parse_name();
      return (Ast*)switch_label;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast_Default* default_label = arena_push_struct(ast_storage, Ast_Default);
      default_label->kind = AST_default;
      default_label->line_no = token->line_no;
      default_label->column_no = token->column_no;
      switch_label->label = (Ast*)default_label;
      return (Ast*)switch_label;
    } else assert(0);
  } else error("At line %d, column %d: switch label was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_statementOrDeclaration()
{
  if (token_is_statementOrDeclaration(token)) {
    Ast_StatementOrDeclaration* stmt = arena_push_struct(ast_storage, Ast_StatementOrDeclaration);
    stmt->kind = AST_statementOrDeclaration;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TK_PARENTH_OPEN) {
        stmt->stmt = parse_instantiation(type_ref);
        return (Ast*)stmt;
      } else if (token_is_name(token)) {
        stmt->stmt = parse_variableDeclaration(type_ref);
        return (Ast*)stmt;
      } else {
        stmt->stmt = parse_statement(type_ref);
        return (Ast*)stmt;
      }
    } else if (token_is_statement(token)) {
      stmt->stmt = parse_statement(0);
      return (Ast*)stmt;
    } else if (token->klass == TK_CONST) {
      stmt->stmt = parse_variableDeclaration(0);
      return (Ast*)stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

/** TABLES **/ 

internal Ast*
parse_tableDeclaration()
{
  if (token->klass == TK_TABLE) {
    next_token();
    Ast_TableDeclaration* table = arena_push_struct(ast_storage, Ast_TableDeclaration);
    table->kind = AST_tableDeclaration;
    table->line_no = token->line_no;
    table->column_no = token->column_no;
    table->name = parse_name();
    if (token->klass == TK_BRACE_OPEN) {
      next_token();
      if (token_is_tableProperty(token)) {
        table->prop_list = parse_tablePropertyList();
      } else error("At line %d, column %d: table property was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)table;
  } else error("At line %d, column %d: `table` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_tablePropertyList()
{
  Ast_TablePropertyList* props = arena_push_struct(ast_storage, Ast_TablePropertyList);
  props->kind = AST_tablePropertyList;
  props->line_no = token->line_no;
  props->column_no = token->column_no;
  list_reset(&props->members);
  if (token_is_tableProperty(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_tableProperty();
    list_append_item(&props->members, li, 1);
    while (token_is_tableProperty(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_tableProperty();
      list_append_item(&props->members, li, 1);
    }
  }
  return (Ast*)props;
}

internal Ast*
parse_tableProperty()
{
  if (token_is_tableProperty(token)) {
    bool is_const = false;
    if (token->klass == TK_CONST) {
      next_token();
      is_const = true;
    }
    Ast_TableProperty* table_prop = arena_push_struct(ast_storage, Ast_TableProperty);
    table_prop->kind = AST_tableProperty;
    table_prop->line_no = token->line_no;
    table_prop->column_no = token->column_no;
    if (token->klass == TK_KEY) {
      next_token();
      Ast_KeyProperty* key_prop = arena_push_struct(ast_storage, Ast_KeyProperty);
      key_prop->kind = AST_keyProperty;
      key_prop->line_no = token->line_no;
      key_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          key_prop->keyelem_list = parse_keyElementList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->prop = (Ast*)key_prop;
      return (Ast*)table_prop;
    } else if (token->klass == TK_ACTIONS) {
      next_token();
      Ast_ActionsProperty* actions_prop = arena_push_struct(ast_storage, Ast_ActionsProperty);
      actions_prop->kind = AST_actionsProperty;
      actions_prop->line_no = token->line_no;
      actions_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          actions_prop->action_list = parse_actionList();
          if (token->klass == TK_BRACE_CLOSE) {
            next_token();
          } else error("At line %d, column %d: `}` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `{` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->prop = (Ast*)actions_prop;
      return (Ast*)table_prop;
    } else if (token->klass == TK_ENTRIES) {
      next_token();
      Ast_EntriesProperty* entries_prop = arena_push_struct(ast_storage, Ast_EntriesProperty);
      entries_prop->kind = AST_entriesProperty;
      entries_prop->line_no = token->line_no;
      entries_prop->column_no = token->column_no;
      if (token->klass == TK_EQUAL) {
        next_token();
        if (token->klass == TK_BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            entries_prop->entries_list = parse_entriesList();
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
      table_prop->prop = (Ast*)entries_prop;
      return (Ast*)table_prop;
    } else if (token_is_nonTableKwName(token)) {
      Ast_SimpleProperty* simple_prop = arena_push_struct(ast_storage, Ast_SimpleProperty);
      simple_prop->kind = AST_simpleProperty;
      simple_prop->line_no = token->line_no;
      simple_prop->column_no = token->column_no;
      simple_prop->is_const = is_const;
      simple_prop->name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        simple_prop->init_expr = parse_expression(1);
        if (token->klass == TK_SEMICOLON) {
          next_token();
        } else error("At line %d, column %d: `;` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `=` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      table_prop->prop = (Ast*)simple_prop;
      return (Ast*)table_prop;
    } else assert(0);
  } else error("At line %d, column %d: table property was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_keyElementList()
{
  Ast_KeyElementList* elems = arena_push_struct(ast_storage, Ast_KeyElementList);
  elems->kind = AST_keyElementList;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  list_reset(&elems->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_keyElement();
    list_append_item(&elems->members, li, 1);
    while (token_is_expression(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_keyElement();
      list_append_item(&elems->members, li, 1);
    }
  }
  return (Ast*)elems;
}

internal Ast*
parse_keyElement()
{
  if (token_is_expression(token)) {
    Ast_KeyElement* key_elem = arena_push_struct(ast_storage, Ast_KeyElement);
    key_elem->kind = AST_keyElement;
    key_elem->line_no = token->line_no;
    key_elem->column_no = token->column_no;
    key_elem->expr = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->match = parse_name();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)key_elem;
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_actionList()
{
  Ast_ActionList* actions = arena_push_struct(ast_storage, Ast_ActionList);
  actions->kind = AST_actionList;
  actions->line_no = token->line_no;
  actions->column_no = token->column_no;
  list_reset(&actions->members);
  if (token_is_actionRef(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_actionRef();
    list_append_item(&actions->members, li, 1);
    if (token->klass == TK_SEMICOLON) {
      next_token();
    } else error("At line %d, column %d: `;` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    while (token_is_actionRef(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_actionRef();
      list_append_item(&actions->members, li, 1);
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    }
  }
  return (Ast*)actions;
}

internal Ast*
parse_actionRef()
{
  if (token_is_prefixedNonTypeName(token)) {
    Ast_ActionRef* action_ref = arena_push_struct(ast_storage, Ast_ActionRef);
    action_ref->kind = AST_actionRef;
    action_ref->line_no = token->line_no;
    action_ref->column_no = token->column_no;
    action_ref->name = parse_prefixedNonTypeName();
    if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_argument(token)) {
        action_ref->args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                    token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    }
    return (Ast*)action_ref;
  } else error("At line %d, column %d: non-type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_entriesList()
{
  Ast_EntriesList* entries = arena_push_struct(ast_storage, Ast_EntriesList);
  entries->kind = AST_entriesList;
  entries->line_no = token->line_no;
  entries->column_no = token->column_no;
  list_reset(&entries->members);
  if (token_is_keysetExpression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_entry();
    list_append_item(&entries->members, li, 1);
    while (token_is_keysetExpression(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_entry();
      list_append_item(&entries->members, li, 1);
    }
  }
  return (Ast*)entries;
}

internal Ast*
parse_entry()
{
  if (token_is_keysetExpression(token)) {
    Ast_Entry* entry = arena_push_struct(ast_storage, Ast_Entry);
    entry->kind = AST_entry;
    entry->line_no = token->line_no;
    entry->column_no = token->column_no;
    entry->keyset = parse_keysetExpression();
    if (token->klass == TK_COLON) {
      next_token();
      entry->action = parse_actionRef();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: `:` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)entry;
  } else error("At line %d, column %d: keyset was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_actionDeclaration()
{
  if (token->klass == TK_ACTION) {
    next_token();
    Ast_ActionDeclaration* action_decl = arena_push_struct(ast_storage, Ast_ActionDeclaration);
    action_decl->kind = AST_actionDeclaration;
    action_decl->line_no = token->line_no;
    action_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      action_decl->name = parse_name();
      if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        action_decl->params = parse_parameterList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          if (token->klass == TK_BRACE_OPEN) {
            action_decl->stmt = parse_blockStatement();
          } else error("At line %d, column %d: `{` was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `}` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: `(` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)action_decl;
  } else error("At line %d, column %d: `action` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** VARIABLES **/

internal Ast*
parse_variableDeclaration(Ast* type_ref)
{
  bool is_const = false;
  if (token->klass == TK_CONST) {
    next_token();
    is_const = true;
  }
  if (token_is_typeRef(token) || type_ref) {
    Ast_VarDeclaration* var_decl = arena_push_struct(ast_storage, Ast_VarDeclaration);
    var_decl->kind = AST_variableDeclaration;
    var_decl->line_no = token->line_no;
    var_decl->column_no = token->column_no;
    var_decl->type = type_ref ? type_ref : parse_typeRef();
    if (token_is_name(token)) {
      var_decl->name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        var_decl->init_expr = parse_expression(1);
      }
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    var_decl->is_const = is_const;
    return (Ast*)var_decl;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

/** EXPRESSIONS **/

internal Ast*
parse_functionDeclaration(Ast* type_ref)
{
  if (token_is_typeOrVoid(token)) {
    Ast_FunctionDeclaration* func_decl = arena_push_struct(ast_storage, Ast_FunctionDeclaration);
    func_decl->kind = AST_functionDeclaration;
    func_decl->line_no = token->line_no;
    func_decl->column_no = token->column_no;
    func_decl->proto = parse_functionPrototype(type_ref);
    if (token->klass == TK_BRACE_OPEN) {
      func_decl->stmt = parse_blockStatement();
    } else error("At line %d, column %d: `{` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)func_decl;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_argumentList()
{
  Ast_ArgumentList* args = arena_push_struct(ast_storage, Ast_ArgumentList);
  args->kind = AST_argumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_reset(&args->members);
  if (token_is_argument(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_argument();
    list_append_item(&args->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_argument();
      list_append_item(&args->members, li, 1);
    }
  }
  return (Ast*)args;
}

internal Ast*
parse_argument()
{
  if (token_is_argument(token)) {
    Ast_Argument* arg = arena_push_struct(ast_storage, Ast_Argument);
    arg->kind = AST_argument;
    arg->line_no = token->line_no;
    arg->column_no = token->column_no;
    if (token_is_expression(token)) {
      arg->arg = parse_expression(1);
      return (Ast*)arg;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast_Dontcare* dontcare_arg = arena_push_struct(ast_storage, Ast_Dontcare);
      dontcare_arg->kind = AST_dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      arg->arg = (Ast*)dontcare_arg;
      return (Ast*)arg;
    } else assert(0);
  } else error("At line %d, column %d: an argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_expressionList()
{
  Ast_ExpressionList* exprs = arena_push_struct(ast_storage, Ast_ExpressionList);
  exprs->kind = AST_expressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  list_reset(&exprs->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    li->object = parse_expression(1);
    list_append_item(&exprs->members, li, 1);
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_expression(1);
      list_append_item(&exprs->members, li, 1);
    }
  }
  return (Ast*)exprs;
}

internal Ast*
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

internal Ast*
parse_lvalue()
{
  if (token_is_lvalue(token)) {
    Ast_LvalueExpression* lvalue = arena_push_struct(ast_storage, Ast_LvalueExpression);
    lvalue->kind = AST_lvalueExpression;
    lvalue->line_no = token->line_no;
    lvalue->column_no = token->column_no;
    lvalue->expr = parse_prefixedNonTypeName();
    while(token->klass == TK_DOT || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast_MemberSelector* member_expr = arena_push_struct(ast_storage, Ast_MemberSelector);
        member_expr->kind = AST_memberSelector;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->lhs_expr = (Ast*)lvalue;
        if (token_is_name(token)) {
          member_expr->name = parse_name();
        } else error("At line %d, column %d: name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_push_struct(ast_storage, Ast_LvalueExpression);
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->expr = (Ast*)member_expr;
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_ArraySubscript* subscript_expr = arena_push_struct(ast_storage, Ast_ArraySubscript);
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->lhs_expr = (Ast*)lvalue;
        subscript_expr->index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_push_struct(ast_storage, Ast_LvalueExpression);
        lvalue->kind = AST_lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->expr = (Ast*)subscript_expr;
      }
    }
    return (Ast*)lvalue;
  } else error("At line %d, column %d: lvalue was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_expression(int priority_threshold)
{
  if (token_is_expression(token)) {
    Ast_Expression* expr = (Ast_Expression*)parse_expressionPrimary();
    while (token_is_exprOperator(token)) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast_MemberSelector* member_expr = arena_push_struct(ast_storage, Ast_MemberSelector);
        member_expr->kind = AST_memberSelector;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->lhs_expr = (Ast*)expr;
        if (token_is_nonTypeName(token)) {
          member_expr->name = parse_nonTypeName();
        } else error("At line %d, column %d: non-type name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)member_expr;
      } else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_ArraySubscript* subscript_expr = arena_push_struct(ast_storage, Ast_ArraySubscript);
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->lhs_expr = (Ast*)expr;
        subscript_expr->index_expr = parse_indexExpression();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)subscript_expr;
      } else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        Ast_FunctionCall* call_expr = arena_push_struct(ast_storage, Ast_FunctionCall);
        call_expr->kind = AST_functionCall;
        call_expr->line_no = token->line_no;
        call_expr->column_no = token->column_no;
        call_expr->lhs_expr = (Ast*)expr;
        call_expr->args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)call_expr;
      } else if (token->klass == TK_ANGLE_OPEN && token_is_realTypeArg(peek_token())) {
        next_token();
        expr->type_args = parse_realTypeArgumentList();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TK_EQUAL) {
        next_token();
        Ast_AssignmentStatement* assign_stmt = arena_push_struct(ast_storage, Ast_AssignmentStatement);
        assign_stmt->kind = AST_assignmentStatement;
        assign_stmt->line_no = token->line_no;
        assign_stmt->column_no = token->column_no;
        assign_stmt->lhs_expr = (Ast*)expr;
        assign_stmt->rhs_expr = parse_expression(1);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)assign_stmt;
      } else if (token_is_binaryOperator(token)){
        int priority = operator_priority_of(token);
        if (priority >= priority_threshold) {
          next_token();
          Ast_BinaryExpression* binary_expr = arena_push_struct(ast_storage, Ast_BinaryExpression);
          binary_expr->kind = AST_binaryExpression;
          binary_expr->line_no = token->line_no;
          binary_expr->column_no = token->column_no;
          binary_expr->left_operand = (Ast*)expr;
          binary_expr->op = token_to_binop(token);
          binary_expr->right_operand = parse_expression(priority + 1);
          expr = arena_push_struct(ast_storage, Ast_Expression);
          expr->kind = AST_expression;
          expr->line_no = token->line_no;
          expr->column_no = token->column_no;
          expr->expr = (Ast*)binary_expr;
        } else break;
      } else assert(0);
    }
    return (Ast*)expr;
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_expressionPrimary()
{
  if (token_is_expression(token)) {
    Ast_Expression* primary = arena_push_struct(ast_storage, Ast_Expression);
    primary->kind = AST_expression;
    primary->line_no = token->line_no;
    primary->column_no = token->column_no;
    if (token->klass == TK_INTEGER_LITERAL) {
      primary->expr = parse_integer();
      return (Ast*)primary;
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      primary->expr = parse_boolean();
      return (Ast*)primary;
    } else if (token->klass == TK_STRING_LITERAL) {
      primary->expr = parse_string();
      return (Ast*)primary;
    } else if (token->klass == TK_DOT) {
      next_token();
      if (token->klass == TK_IDENTIFIER) {
        primary->expr = parse_nonTypeName();
        return (Ast*)primary;
      } else if (token->klass == TK_TYPE_IDENTIFIER) {
        primary->expr = parse_namedType();
        return (Ast*)primary;
      } else error("At line %d, column %d: unexpected token `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token_is_nonTypeName(token)) {
      primary->expr = parse_nonTypeName();
      return (Ast*)primary;
    } else if (token->klass == TK_BRACE_OPEN) {
      next_token();
      primary->expr = parse_expressionList();
      if (token->klass == TK_BRACE_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `}` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return (Ast*)primary;
    } else if (token->klass == TK_PARENTH_OPEN) {
      next_token();
      if (token_is_typeRef(token)) {
        Ast_CastExpression* cast_expr = arena_push_struct(ast_storage, Ast_CastExpression);
        cast_expr->kind = AST_castExpression;
        cast_expr->line_no = token->line_no;
        cast_expr->column_no = token->column_no;
        cast_expr->type = parse_typeRef();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
          cast_expr->expr = parse_expression(1);
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        primary->expr = (Ast*)cast_expr;
        return (Ast*)primary;
      } else if (token_is_expression(token)) {
        primary->expr = parse_expression(1);
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        return (Ast*)primary;
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->klass == TK_EXCLAMATION) {
      next_token();
      Ast_UnaryExpression* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpression);
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->op = OP_NOT;
      unary_expr->operand = parse_expression(1);
      primary->expr = (Ast*)unary_expr;
      return (Ast*)primary;
    } else if (token->klass == TK_TILDA) {
      next_token();
      Ast_UnaryExpression* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpression);
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->op = OP_BITW_NOT;
      unary_expr->operand = parse_expression(1);
      primary->expr = (Ast*)unary_expr;
      return (Ast*)primary;
    } else if (token->klass == TK_UNARY_MINUS) {
      next_token();
      Ast_UnaryExpression* unary_expr = arena_push_struct(ast_storage, Ast_UnaryExpression);
      unary_expr->kind = AST_unaryExpression;
      unary_expr->line_no = token->line_no;
      unary_expr->column_no = token->column_no;
      unary_expr->op = OP_NEG;
      unary_expr->operand = parse_expression(1);
      primary->expr = (Ast*)unary_expr;
      return (Ast*)primary;
    } else if (token_is_typeName(token)) {
      primary->expr = parse_namedType();
      return (Ast*)primary;
    } else if (token->klass == TK_ERROR) {
      next_token();
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_name;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->strname = "error";
      primary->expr = (Ast*)name;
      return (Ast*)primary;
    } else assert(0);
    assert(0);
  } else error("At line %d, column %d: expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_indexExpression()
{
  if (token_is_expression(token)) {
    Ast_IndexExpression* index_expr = arena_push_struct(ast_storage, Ast_IndexExpression);
    index_expr->kind = AST_indexExpression;
    index_expr->line_no = token->line_no;
    index_expr->column_no = token->column_no;
    index_expr->start_index = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      if (token_is_expression(token)) {
        index_expr->end_index = parse_expression(1);
      } else error("At line %d, column %d: expression was expected, got `%s`.",
                  token->line_no, token->column_no, token->lexeme);
    }
    return (Ast*)index_expr;
  } else error("At line %d, column %d: expression or `:` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_integer()
{
  if (token->klass == TK_INTEGER_LITERAL) {
    Ast_IntegerLiteral* int_literal = arena_push_struct(ast_storage, Ast_IntegerLiteral);
    int_literal->kind = AST_integerLiteral;
    int_literal->line_no = token->line_no;
    int_literal->column_no = token->column_no;
    int_literal->is_signed = token->i.is_signed;
    int_literal->width = token->i.width;
    int_literal->value = token->i.value;
    next_token();
    return (Ast*)int_literal;
  } else error("At line %d, column %d: integer was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_boolean()
{
  if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
    Ast_BooleanLiteral* bool_literal = arena_push_struct(ast_storage, Ast_BooleanLiteral);
    bool_literal->kind = AST_booleanLiteral;
    bool_literal->line_no = token->line_no;
    bool_literal->column_no = token->column_no;
    bool_literal->value = (token->klass == TK_TRUE);
    next_token();
    return (Ast*)bool_literal;
  } else error("At line %d, column %d: boolean was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_string()
{
  if (token->klass == TK_STRING_LITERAL) {
    Ast_StringLiteral* string_literal = arena_push_struct(ast_storage, Ast_StringLiteral);
    string_literal->kind = AST_stringLiteral;
    string_literal->line_no = token->line_no;
    string_literal->column_no = token->column_no;
    string_literal->value = token->lexeme;
    next_token();
    return (Ast*)string_literal;
  } else error("At line %d, column %d: string was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast_P4Program*
parse_tokens(UnboundedArray* tokens_, Arena* ast_storage_)
{
  tokens = tokens_;
  ast_storage = ast_storage_;
  root_scope = arena_push_struct(ast_storage, Scope);
  hashmap_create(&root_scope->decls, HASHMAP_KEY_STRING, 3, ast_storage);
  root_scope->scope_level = 0;
  current_scope = root_scope;

  NameDecl* kw_decl;
  kw_decl = declare_keyword(ast_storage, root_scope, "action");
  kw_decl->token_class = TK_ACTION;
  kw_decl = declare_keyword(ast_storage, root_scope, "actions");
  kw_decl->token_class = TK_ACTIONS;
  kw_decl = declare_keyword(ast_storage, root_scope, "entries");
  kw_decl->token_class = TK_ENTRIES;
  kw_decl = declare_keyword(ast_storage, root_scope, "enum");
  kw_decl->token_class = TK_ENUM;
  kw_decl = declare_keyword(ast_storage, root_scope, "in");
  kw_decl->token_class = TK_IN;
  kw_decl = declare_keyword(ast_storage, root_scope, "package");
  kw_decl->token_class = TK_PACKAGE;
  kw_decl = declare_keyword(ast_storage, root_scope, "select");
  kw_decl->token_class = TK_SELECT;
  kw_decl = declare_keyword(ast_storage, root_scope, "switch");
  kw_decl->token_class = TK_SWITCH;
  kw_decl = declare_keyword(ast_storage, root_scope, "tuple");
  kw_decl->token_class = TK_TUPLE;
  kw_decl = declare_keyword(ast_storage, root_scope, "control");
  kw_decl->token_class = TK_CONTROL;
  kw_decl = declare_keyword(ast_storage, root_scope, "error");
  kw_decl->token_class = TK_ERROR;
  kw_decl = declare_keyword(ast_storage, root_scope, "header");
  kw_decl->token_class = TK_HEADER;
  kw_decl = declare_keyword(ast_storage, root_scope, "inout");
  kw_decl->token_class = TK_INOUT;
  kw_decl = declare_keyword(ast_storage, root_scope, "parser");
  kw_decl->token_class = TK_PARSER;
  kw_decl = declare_keyword(ast_storage, root_scope, "state");
  kw_decl->token_class = TK_STATE;
  kw_decl = declare_keyword(ast_storage, root_scope, "table");
  kw_decl->token_class = TK_TABLE;
  kw_decl = declare_keyword(ast_storage, root_scope, "key");
  kw_decl->token_class = TK_KEY;
  kw_decl = declare_keyword(ast_storage, root_scope, "typedef");
  kw_decl->token_class = TK_TYPEDEF;
  kw_decl = declare_keyword(ast_storage, root_scope, "type");
  kw_decl->token_class = TK_TYPE;
  kw_decl = declare_keyword(ast_storage, root_scope, "default");
  kw_decl->token_class = TK_DEFAULT;
  kw_decl = declare_keyword(ast_storage, root_scope, "extern");
  kw_decl->token_class = TK_EXTERN;
  kw_decl = declare_keyword(ast_storage, root_scope, "header_union");
  kw_decl->token_class = TK_HEADER_UNION;
  kw_decl = declare_keyword(ast_storage, root_scope, "out");
  kw_decl->token_class = TK_OUT;
  kw_decl = declare_keyword(ast_storage, root_scope, "transition");
  kw_decl->token_class = TK_TRANSITION;
  kw_decl = declare_keyword(ast_storage, root_scope, "else");
  kw_decl->token_class = TK_ELSE;
  kw_decl = declare_keyword(ast_storage, root_scope, "exit");
  kw_decl->token_class = TK_EXIT;
  kw_decl = declare_keyword(ast_storage, root_scope, "if");
  kw_decl->token_class = TK_IF;
  kw_decl = declare_keyword(ast_storage, root_scope, "match_kind");
  kw_decl->token_class = TK_MATCH_KIND;
  kw_decl = declare_keyword(ast_storage, root_scope, "return");
  kw_decl->token_class = TK_RETURN;
  kw_decl = declare_keyword(ast_storage, root_scope, "struct");
  kw_decl->token_class = TK_STRUCT;
  kw_decl = declare_keyword(ast_storage, root_scope, "apply");
  kw_decl->token_class = TK_APPLY;
  kw_decl = declare_keyword(ast_storage, root_scope, "const");
  kw_decl->token_class = TK_CONST;
  kw_decl = declare_keyword(ast_storage, root_scope, "bool");
  kw_decl->token_class = TK_BOOL;
  kw_decl = declare_keyword(ast_storage, root_scope, "true");
  kw_decl->token_class = TK_TRUE;
  kw_decl = declare_keyword(ast_storage, root_scope, "false");
  kw_decl->token_class = TK_FALSE;
  kw_decl = declare_keyword(ast_storage, root_scope, "void");
  kw_decl->token_class = TK_VOID;
  kw_decl = declare_keyword(ast_storage, root_scope, "int");
  kw_decl->token_class = TK_INT;
  kw_decl = declare_keyword(ast_storage, root_scope, "bit");
  kw_decl->token_class = TK_BIT;
  kw_decl = declare_keyword(ast_storage, root_scope, "varbit");
  kw_decl->token_class = TK_VARBIT;
  kw_decl = declare_keyword(ast_storage, root_scope, "string");
  kw_decl->token_class = TK_STRING;

  token_at = 0;
  token = array_get(tokens, token_at);
  next_token();
  Ast_P4Program* p4program = (Ast_P4Program*)parse_p4program();
  current_scope = pop_scope(current_scope);
  assert(current_scope == 0);
  return p4program;
}
