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
internal int node_id = 0;
internal Scope* root_scope;
internal Scope* current_scope;

internal Ast* parse_expression(int priority_threshold);
internal Ast* parse_typeRef();
internal Ast* parse_baseType();
internal Ast* parse_blockStatement();
internal Ast* parse_statement(Ast* type_name);
internal Ast* parse_parserStatement();

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
    NamespaceEntry* ne = scope_lookup_name(current_scope, token->lexeme);
    if (ne->ns_keyword) {
      NameDecl* ndecl = ne->ns_keyword;
      token->klass = ndecl->token_class;
      return token;
    } else if (ne->ns_type) {
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
get_operator_priority(Token* token)
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

internal enum AstOperator
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

internal Ast*
parse_nonTypeName()
{
  if (token_is_nonTypeName(token)) {
    Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
    name->kind = AST_name;
    name->id = node_id++;
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
      next_token();
      Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
      type_name->kind = AST_name;
      type_name->id = node_id++;
      type_name->line_no = token->line_no;
      type_name->column_no = token->column_no;
      type_name->strname = token->lexeme;
      return (Ast*)type_name;
    } else assert(0);
  } else error("At line %d, column %d: name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_typeParameterList()
{
  Ast_List* params = arena_push_struct(ast_storage, Ast_List);
  params->kind = AST_typeParameterList;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  list_init(&params->members);
  if (token_is_typeParameterList(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&params->members, li, 1);
    Ast_Name* name = (Ast_Name*)parse_name();
    declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
    li->object = name;
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
      li->object = name;
      list_append_item(&params->members, li, 1);
    }
  }
  return (Ast*)params;
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
parse_typeArg()
{
  if (token_is_typeArg(token)) {
    Ast_TypeArgument* type_arg = arena_push_struct(ast_storage, Ast_TypeArgument);
    type_arg->kind = AST_typeArgument;
    type_arg->id = node_id++;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_push_struct(ast_storage, Ast);
      dontcare_arg->kind = AST_dontcareTypeArgument;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->arg = dontcare_arg;
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
parse_typeArgumentList()
{
  Ast_List* args = arena_push_struct(ast_storage, Ast_List);
  args->kind = AST_typeArgumentList;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_init(&args->members);
  if (token_is_typeArg(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&args->members, li, 1);
    li->object = parse_typeArg();
    while (token->klass == TK_COMMA) {
      next_token();
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_typeArg();
      list_append_item(&args->members, li, 1);
    }
  }
  return (Ast*)args;
}

internal enum AstParamDirection
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
parse_parameter()
{
  if (token_is_parameter(token)) {
    Ast_Parameter* param = arena_push_struct(ast_storage, Ast_Parameter);
    param->kind = AST_parameter;
    param->id = node_id++;
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

internal Ast*
parse_parameterList()
{
  Ast_List* params = arena_push_struct(ast_storage, Ast_List);
  params->kind = AST_parameterList;
  params->id = node_id++;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  list_init(&params->members);
  if (token_is_parameter(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&params->members, li, 1);
    li->object = parse_parameter();
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
parse_typeOrVoid()
{
  if (token_is_typeOrVoid(token)) {
    if (token_is_typeRef(token)) {
      Ast* type = parse_typeRef();
      return type;
    } else if (token->klass == TK_VOID) {
      return (Ast*)parse_baseType();
    } else if (token->klass == TK_IDENTIFIER) {
      next_token();
      Ast_Name* name = arena_push_struct(ast_storage, Ast_Name);
      name->kind = AST_name;
      name->id = node_id++;
      name->line_no = token->line_no;
      name->column_no = token->column_no;
      name->strname = token->lexeme;
      return (Ast*)name;
    } else assert(0);
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_functionPrototype(Ast* return_type)
{
  if (token_is_typeOrVoid(token) || return_type) {
    Ast_FunctionPrototype* proto = arena_push_struct(ast_storage, Ast_FunctionPrototype);
    proto->kind = AST_functionPrototype;
    proto->id = node_id++;
    proto->line_no = token->line_no;
    proto->column_no = token->column_no;
    if (return_type) {
      proto->return_type = return_type;
    } else {
      Ast* return_type = parse_typeOrVoid();
      if (return_type->kind == AST_name) {
        Ast_Name* name = (Ast_Name*)return_type;
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
        Ast_TypeRef* type_ref = arena_push_struct(ast_storage, Ast_TypeRef);
        type_ref->kind = AST_typeRef;
        type_ref->id = node_id++;
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
      proto->kind = AST_methodPrototype;
      proto->id = node_id++;
      proto->line_no = token->line_no;
      proto->column_no = token->column_no;
      proto->name = parse_name();
      proto->type_params = parse_optTypeParameters();
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

internal Ast*
parse_methodPrototypes()
{
  Ast_List* protos = arena_push_struct(ast_storage, Ast_List);
  protos->kind = AST_methodPrototypes;
  protos->id = node_id++;
  protos->line_no = token->line_no;
  protos->column_no = token->column_no;
  list_init(&protos->members);
  if (token_is_methodPrototype(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&protos->members, li, 1);
    li->object = parse_methodPrototype();
    while (token_is_methodPrototype(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_methodPrototype();
      list_append_item(&protos->members, li, 1);
    }
  }
  return (Ast*)protos;
}

internal Ast*
parse_externDeclaration()
{
  if (token->klass == TK_EXTERN) {
    next_token();
    Ast_ExternDeclaration* extern_decl = arena_push_struct(ast_storage, Ast_ExternDeclaration);
    extern_decl->kind = AST_externDeclaration;
    extern_decl->id = node_id++;
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
      Ast_ExternType* extern_type = arena_push_struct(ast_storage, Ast_ExternType);
      extern_type->kind = AST_externType;
      extern_type->id = node_id++;
      extern_type->line_no = token->line_no;
      extern_type->column_no = token->column_no;
      extern_type->name = parse_nonTypeName();
      Ast_Name* name = (Ast_Name*)extern_type->name;
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_integer()
{
  if (token->klass == TK_INTEGER_LITERAL) {
    next_token();
    Ast_IntegerLiteral* int_literal = arena_push_struct(ast_storage, Ast_IntegerLiteral);
    int_literal->kind = AST_integerLiteral;
    int_literal->id = node_id++;
    int_literal->line_no = token->line_no;
    int_literal->column_no = token->column_no;
    int_literal->is_signed = token->i.is_signed;
    int_literal->width = token->i.width;
    int_literal->value = token->i.value;
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
    next_token();
    Ast_BoolLiteral* bool_literal = arena_push_struct(ast_storage, Ast_BoolLiteral);
    bool_literal->kind = AST_booleanLiteral;
    bool_literal->id = node_id++;
    bool_literal->line_no = token->line_no;
    bool_literal->column_no = token->column_no;
    bool_literal->value = (token->klass == TK_TRUE);
    return (Ast*)bool_literal;
  } else error("At line %d, column %d: boolean was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_stringLiteral()
{
  if (token->klass == TK_STRING_LITERAL) {
    next_token();
    Ast_StringLiteral* string_literal = arena_push_struct(ast_storage, Ast_StringLiteral);
    string_literal->kind = AST_stringLiteral;
    string_literal->id = node_id++;
    string_literal->line_no = token->line_no;
    string_literal->column_no = token->column_no;
    string_literal->value = token->lexeme;
    return (Ast*)string_literal;
  } else error("At line %d, column %d: string was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_integerTypeSize()
{
  Ast_IntegerTypeSize* type_size = arena_push_struct(ast_storage, Ast_IntegerTypeSize);
  type_size->kind = AST_integerTypeSize;
  type_size->id = node_id++;
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
parse_baseType()
{
  if (token_is_baseType(token)) {
    Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
    type_name->kind = AST_name;
    type_name->id = node_id++;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    if (token->klass == TK_BOOL) {
      next_token();
      Ast_BoolType* bool_type = arena_push_struct(ast_storage, Ast_BoolType);
      bool_type->kind = AST_baseTypeBool;
      bool_type->id = node_id++;
      bool_type->line_no = token->line_no;
      bool_type->column_no = token->column_no;
      type_name->strname = "bool";
      bool_type->name = (Ast*)type_name;
      return (Ast*)bool_type;
    } else if (token->klass == TK_ERROR) {
      next_token();
      Ast_ErrorType* error_type = arena_push_struct(ast_storage, Ast_ErrorType);
      error_type->kind = AST_baseTypeError;
      error_type->id = node_id++;
      error_type->line_no = token->line_no;
      error_type->column_no = token->column_no;
      type_name->strname = "error";
      error_type->name = (Ast*)type_name;
      return (Ast*)error_type;
    } else if (token->klass == TK_INT) {
      next_token();
      Ast_IntegerType* int_type = arena_push_struct(ast_storage, Ast_IntegerType);
      int_type->kind = AST_baseTypeInteger;
      int_type->id = node_id++;
      int_type->line_no = token->line_no;
      int_type->column_no = token->column_no;
      type_name->strname = "int";
      int_type->name = (Ast*)type_name;
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
      next_token();
      Ast_BitType* bit_type = arena_push_struct(ast_storage, Ast_BitType);
      bit_type->kind = AST_baseTypeBit;
      bit_type->id = node_id++;
      bit_type->line_no = token->line_no;
      bit_type->column_no = token->column_no;
      type_name->strname = "bit";
      bit_type->name = (Ast*)type_name;
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
      next_token();
      Ast_VarbitType* varbit_type = arena_push_struct(ast_storage, Ast_VarbitType);
      varbit_type->kind = AST_baseTypeVarbit;
      varbit_type->id = node_id++;
      varbit_type->line_no = token->line_no;
      varbit_type->column_no = token->column_no;
      type_name->strname = "varbit";
      varbit_type->name = (Ast*)type_name;
      if (token->klass == TK_ANGLE_OPEN) {
        next_token();
        varbit_type->size = parse_integerTypeSize();
        if (token->klass == TK_ANGLE_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `>` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      }
      return (Ast*)varbit_type;
    } else if (token->klass == TK_STRING) {
      next_token();
      Ast_StringType* string_type = arena_push_struct(ast_storage, Ast_StringType);
      string_type->kind = AST_baseTypeString;
      string_type->id = node_id++;
      string_type->line_no = token->line_no;
      string_type->column_no = token->column_no;
      type_name->strname = "string";
      string_type->name = (Ast*)type_name;
      return (Ast*)string_type;
    } else if (token->klass == TK_VOID) {
      next_token();
      Ast_VoidType* void_type = arena_push_struct(ast_storage, Ast_VoidType);
      void_type->kind = AST_baseTypeVoid;
      void_type->id = node_id++;
      void_type->line_no = token->line_no;
      void_type->column_no = token->column_no;
      type_name->strname = "void";
      void_type->name = (Ast*)type_name;
      return (Ast*)void_type;
    } else assert(0);
  } else error("At line %d, column %d: base type was expected, got `%s`.",
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
    tuple->id = node_id++;
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
    stack->id = node_id++;
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
    type->id = node_id++;
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
parse_prefixedType()
{
  if (token->klass == TK_DOT) {
    next_token();
  }
  if (token->klass == TK_TYPE_IDENTIFIER) {
    next_token();
    Ast_Name* type_name = arena_push_struct(ast_storage, Ast_Name);
    type_name->kind = AST_name;
    type_name->id = node_id++;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    type_name->strname = token->lexeme;
    return (Ast*)type_name;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_namedType()
{
  if (token_is_typeName(token)) {
    Ast_NamedType* named_type = arena_push_struct(ast_storage, Ast_NamedType);
    named_type->kind = AST_namedType;
    named_type->id = node_id++;
    named_type->line_no = token->line_no;
    named_type->column_no = token->column_no;
    named_type->type = parse_prefixedType();
    if (token->klass == TK_ANGLE_OPEN) {
      Ast_SpecializedType* specd_type = (Ast_SpecializedType*)parse_specializedType();
      specd_type->name = named_type->type;
      named_type->type = (Ast*)specd_type;
    } else if (token->klass == TK_BRACKET_OPEN) {
      Ast_HeaderStackType* stack_type = (Ast_HeaderStackType*)parse_headerStackType();
      stack_type->name = named_type->type;
      named_type->type = (Ast*)stack_type;
    }
    return (Ast*)named_type;
  } else error("At line %d, column %d: type was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_typeRef()
{
  if (token_is_typeRef(token)) {
    Ast_TypeRef* type_ref = arena_push_struct(ast_storage, Ast_TypeRef);
    type_ref->kind = AST_typeRef;
    type_ref->id = node_id++;
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
parse_structField()
{
  if (token_is_structField(token)) {
    Ast_StructField* field = arena_push_struct(ast_storage, Ast_StructField);
    field->kind = AST_structField;
    field->id = node_id++;
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
parse_structFieldList()
{
  Ast_List* fields = arena_push_struct(ast_storage, Ast_List);
  fields->kind = AST_structFieldList;
  fields->id = node_id++;
  fields->line_no = token->line_no;
  fields->column_no = token->column_no;
  list_init(&fields->members);
  if (token_is_structField(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&fields->members, li, 1);
    li->object = parse_structField();
    while (token_is_structField(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_structField();
      list_append_item(&fields->members, li, 1);
    }
  }
  return (Ast*)fields;
}

internal Ast*
parse_headerTypeDeclaration()
{
  if (token->klass == TK_HEADER) {
    next_token();
    Ast_HeaderTypeDeclaration* header_decl = arena_push_struct(ast_storage, Ast_HeaderTypeDeclaration);
    header_decl->kind = AST_headerTypeDeclaration;
    header_decl->id = node_id++;
    header_decl->line_no = token->line_no;
    header_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
    union_decl->id = node_id++;
    union_decl->line_no = token->line_no;
    union_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
    struct_decl->id = node_id++;
    struct_decl->line_no = token->line_no;
    struct_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_initializer()
{
  return parse_expression(1);
}

internal Ast*
parse_optInitializer()
{
  if (token->klass == TK_EQUAL) {
    next_token();
    Ast* init_stmt = parse_initializer();
    return init_stmt;
  }
  return 0;
}

internal Ast*
parse_specifiedIdentifier()
{
  if (token_is_specifiedIdentifier(token)) {
    Ast_SpecifiedIdent* id = arena_push_struct(ast_storage, Ast_SpecifiedIdent);
    id->kind = AST_specifiedIdentifier;
    id->id = node_id++;
    id->line_no = token->line_no;
    id->column_no = token->column_no;
    id->name = parse_name();
    if (token->klass == TK_EQUAL) {
      next_token();
      if (token_is_expression(token)) {
        id->init_expr = parse_initializer();
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
parse_specifiedIdentifierList()
{
  Ast_List* ids = arena_push_struct(ast_storage, Ast_List);
  ids->kind = AST_specifiedIdentifierList;
  ids->id = node_id++;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  list_init(&ids->members);
  if (token_is_specifiedIdentifier(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&ids->members, li, 1);
    li->object = parse_specifiedIdentifier();
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
parse_enumDeclaration()
{
  if (token->klass == TK_ENUM) {
    next_token();
    Ast_EnumDeclaration* enum_decl = arena_push_struct(ast_storage, Ast_EnumDeclaration);
    enum_decl->kind = AST_enumDeclaration;
    enum_decl->id = node_id++;
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
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_derivedTypeDeclaration()
{
  if (token_is_derivedTypeDeclaration(token)) {
    Ast_DerivedTypeDeclaration* type_decl = arena_push_struct(ast_storage, Ast_DerivedTypeDeclaration);
    type_decl->kind = AST_derivedTypeDeclaration;
    type_decl->id = node_id++;
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
parse_parserTypeDeclaration()
{
  if (token->klass == TK_PARSER) {
    next_token();
    Ast_ParserPrototype* proto = arena_push_struct(ast_storage, Ast_ParserPrototype);
    proto->kind = AST_parserTypeDeclaration;
    proto->id = node_id++;
    proto->line_no = token->line_no; 
    proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_constantDeclaration()
{
  if (token->klass == TK_CONST) {
    next_token();
    Ast_ConstDeclaration* const_decl = arena_push_struct(ast_storage, Ast_ConstDeclaration);
    const_decl->kind = AST_constantDeclaration;
    const_decl->id = node_id++;
    const_decl->line_no = token->line_no;
    const_decl->column_no = token->column_no;
    if (token_is_typeRef(token)) {
      const_decl->type = parse_typeRef();
      if (token_is_name(token)) {
        const_decl->name = parse_name();
        if (token->klass == TK_EQUAL) {
          next_token();
          if (token_is_expression(token)) {
            const_decl->init_expr = parse_expression(1);
            if (token->klass == TK_SEMICOLON) {
              next_token();
            } else error("At line %d, column %d: `;` expected, got `%s`.",
                         token->line_no, token->column_no, token->lexeme);
          } else error("At line %d, column %d: expression was expected, got `%s`.",
                       token->line_no, token->column_no, token->lexeme);
        } else error("At line %d, column %d: `=` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
      } else error("At line %d, column %d: name was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: type was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)const_decl;
  } else error("At line %d, column %d: `const` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_argument()
{
  if (token_is_argument(token)) {
    Ast_Argument* arg = arena_push_struct(ast_storage, Ast_Argument);
    arg->kind = AST_argument;
    arg->id = node_id++;
    arg->line_no = token->line_no;
    arg->column_no = token->column_no;
    if (token_is_expression(token)) {
      arg->arg = parse_expression(1);
      return (Ast*)arg;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_push_struct(ast_storage, Ast);
      dontcare_arg->kind = AST_dontcareArgument;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      arg->arg = dontcare_arg;
      return (Ast*)arg;
    } else assert(0);
  } else error("At line %d, column %d: an argument was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_argumentList()
{
  Ast_List* args = arena_push_struct(ast_storage, Ast_List);
  args->kind = AST_argumentList;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_init(&args->members);
  if (token_is_argument(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&args->members, li, 1);
    li->object = parse_argument();
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
parse_optArguments()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    if (token_is_argument(token)) {
      Ast* args = parse_argumentList();
      if (token->klass == TK_PARENTH_CLOSE) {
        next_token();
      } else error("At line %d, column %d: `)` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
      return args;
    } else if (token->klass == TK_PARENTH_CLOSE) {
      next_token();
    } else error("At line %d, column %d: `)` was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
  }
  return 0;
}

internal Ast*
parse_variableDeclaration(Ast* type_ref)
{
  if (token_is_typeRef(token) || type_ref) {
    Ast_VarDeclaration* var_decl = arena_push_struct(ast_storage, Ast_VarDeclaration);
    var_decl->kind = AST_variableDeclaration;
    var_decl->id = node_id++;
    var_decl->line_no = token->line_no;
    var_decl->column_no = token->column_no;
    var_decl->type = type_ref ? type_ref : parse_typeRef();
    if (token_is_name(token)) {
      var_decl->name = parse_name();
      var_decl->init_expr = parse_optInitializer();
      if (token->klass == TK_SEMICOLON) {
        next_token();
      } else error("At line %d, column %d: `;` was expected, got `%s`.",
                   token->line_no, token->column_no, token->lexeme);
    } else error("At line %d, column %d: name was expected, got `%s`.",
                 token->line_no, token->column_no, token->lexeme);
    return (Ast*)var_decl;
  } else error("At line %d, column %d: type was expected, got `%s`.",
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
    inst_stmt->id = node_id++;
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

internal Ast*
parse_parserLocalElement()
{
  if (token_is_parserLocalElement(token)) {
    Ast_ParserLocalElement* local_element = arena_push_struct(ast_storage, Ast_ParserLocalElement);
    local_element->kind = AST_parserLocalElement;
    local_element->id = node_id++;
    local_element->line_no = token->line_no;
    local_element->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_element->element = parse_constantDeclaration();
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
parse_parserLocalElements()
{
  Ast_List* elems = arena_push_struct(ast_storage, Ast_List);
  elems->kind = AST_parserLocalElements;
  elems->id = node_id++;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  list_init(&elems->members);
  if (token_is_parserLocalElement(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&elems->members, li, 1);
    li->object = parse_parserLocalElement();
    while (token_is_parserLocalElement(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserLocalElement();
      list_append_item(&elems->members, li, 1);
    }
  }
  return (Ast*)elems;
}

internal Ast*
parse_directApplication(Ast* type_name)
{
  if (token_is_typeName(token) || type_name) {
    Ast_DirectApplication* apply_stmt = arena_push_struct(ast_storage, Ast_DirectApplication);
    apply_stmt->kind = AST_directApplication;
    apply_stmt->id = node_id++;
    apply_stmt->line_no = token->line_no;
    apply_stmt->column_no = token->column_no;
    apply_stmt->lhs_expr = type_name ? type_name : parse_namedType();
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
parse_arrayIndex()
{
  if (token_is_expression(token)) {
    Ast_ArrayIndex* index_expr = arena_push_struct(ast_storage, Ast_ArrayIndex);
    index_expr->kind = AST_arrayIndex;
    index_expr->id = node_id++;
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
parse_lvalue()
{
  if (token_is_lvalue(token)) {
    Ast_Expression* lvalue = arena_push_struct(ast_storage, Ast_Expression);
    lvalue->kind = AST_lvalue;
    lvalue->id = node_id++;
    lvalue->line_no = token->line_no;
    lvalue->column_no = token->column_no;
    lvalue->expr = parse_prefixedNonTypeName();
    while(token->klass == TK_DOT || token->klass == TK_BRACKET_OPEN) {
      if (token->klass == TK_DOT) {
        next_token();
        Ast_MemberSelector* member_expr = arena_push_struct(ast_storage, Ast_MemberSelector);
        member_expr->kind = AST_memberSelector;
        member_expr->id = node_id++;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->lhs_expr = (Ast*)lvalue;
        if (token_is_name(token)) {
          member_expr->member_name = parse_name();
        } else error("At line %d, column %d: name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_push_struct(ast_storage, Ast_Expression);
        lvalue->kind = AST_lvalue;
        lvalue->id = node_id++;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->expr = (Ast*)member_expr;
      }
      else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_ArraySubscript* subscript_expr = arena_push_struct(ast_storage, Ast_ArraySubscript);
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->lhs_expr = (Ast*)lvalue;
        subscript_expr->index_expr = parse_arrayIndex();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        lvalue = arena_push_struct(ast_storage, Ast_Expression);
        lvalue->kind = AST_lvalue;
        lvalue->id = node_id++;
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
parse_assignmentOrMethodCallStatement()
{
  if (token_is_lvalue(token)) {
    Ast_Expression* lvalue = (Ast_Expression*)parse_lvalue();
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
      call_stmt->id = node_id++;
      call_stmt->line_no = token->line_no;
      call_stmt->column_no = token->column_no;
      call_stmt->callee_expr = (Ast*)lvalue;
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
      Ast_AssignmentStmt* assign_stmt = arena_push_struct(ast_storage, Ast_AssignmentStmt);
      assign_stmt->kind = AST_assignmentStatement;
      assign_stmt->id = node_id++;
      assign_stmt->line_no = token->line_no;
      assign_stmt->column_no = token->column_no;
      assign_stmt->lvalue = (Ast*)lvalue;
      assign_stmt->expr = parse_expression(1);
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
parse_parserStatements()
{
  Ast_List* stmts = arena_push_struct(ast_storage, Ast_List);
  stmts->kind = AST_parserStatements;
  stmts->id = node_id++;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  list_init(&stmts->members);
  if (token_is_parserStatement(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&stmts->members, li, 1);
    li->object = parse_parserStatement();
    while (token_is_parserStatement(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserStatement();
      list_append_item(&stmts->members, li, 1);
    }
  }
  return (Ast*)stmts;
}

internal Ast*
parse_parserBlockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast_BlockStmt* stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    stmt->kind = AST_parserBlockStatement;
    stmt->id = node_id++;
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
parse_parserStatement()
{
  if (token_is_parserStatement(token)) {
    Ast_ParserStmt* parser_stmt = arena_push_struct(ast_storage, Ast_ParserStmt);
    parser_stmt->kind = AST_parserStatement;
    parser_stmt->id = node_id++;
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
      parser_stmt->stmt = parse_constantDeclaration();
      return (Ast*)parser_stmt;
    } else if (token->klass == TK_SEMICOLON) {
      Ast* stmt = arena_push_struct(ast_storage, Ast);
      stmt->kind = AST_emptyStatement;
      stmt->id = node_id++;
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
parse_expressionList()
{
  Ast_List* exprs = arena_push_struct(ast_storage, Ast_List);
  exprs->kind = AST_expressionList;
  exprs->id = node_id++;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  list_init(&exprs->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&exprs->members, li, 1);
    li->object = parse_expression(1);
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
parse_simpleKeysetExpression()
{
  if (token_is_simpleKeysetExpression(token)) {
    Ast_KeysetExpression* keyset_expr = arena_push_struct(ast_storage, Ast_KeysetExpression);
    keyset_expr->kind = AST_keysetExpression;
    keyset_expr->id = node_id++;
    keyset_expr->line_no = token->line_no;
    keyset_expr->column_no = token->column_no;
    if (token_is_expression(token)) {
      keyset_expr->expr = parse_expression(1);
      return (Ast*)keyset_expr;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast* default_set = arena_push_struct(ast_storage, Ast);
      default_set->kind = AST_defaultKeysetExpression;
      default_set->id = node_id++;
      default_set->line_no = token->line_no;
      default_set->column_no = token->column_no;
      keyset_expr->expr = default_set;
      return (Ast*)keyset_expr;
    } else if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_set = arena_push_struct(ast_storage, Ast);
      dontcare_set->kind = AST_dontcareKeysetExpression;
      dontcare_set->id = node_id++;
      dontcare_set->line_no = token->line_no;
      dontcare_set->column_no = token->column_no;
      keyset_expr->expr = dontcare_set;
      return (Ast*)keyset_expr;
    }
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_keysetExpressionList()
{
  Ast_List* exprs = arena_push_struct(ast_storage, Ast_List);
  exprs->kind = AST_keysetExpressionList;
  exprs->id = node_id++;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  list_init(&exprs->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&exprs->members, li, 1);
    li->object = parse_simpleKeysetExpression();
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
parse_tupleKeysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN) {
    next_token();
    Ast_TupleKeysetExpression* tuple_keyset = arena_push_struct(ast_storage, Ast_TupleKeysetExpression);
    tuple_keyset->kind = AST_tupleKeysetExpression;
    tuple_keyset->id = node_id++;
    tuple_keyset->line_no = token->line_no;
    tuple_keyset->column_no = token->column_no;
    tuple_keyset->expr_list = parse_keysetExpressionList();
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
parse_keysetExpression()
{
  if (token->klass == TK_PARENTH_OPEN || token_is_simpleKeysetExpression(token)) {
    if (token->klass == TK_PARENTH_OPEN) {
      Ast_KeysetExpression* keyset_expr = arena_push_struct(ast_storage, Ast_KeysetExpression);
      keyset_expr->kind = AST_keysetExpression;
      keyset_expr->id = node_id++;
      keyset_expr->line_no = token->line_no;
      keyset_expr->column_no = token->column_no;
      keyset_expr->expr = parse_tupleKeysetExpression();
      return (Ast*)keyset_expr;
    } else if (token_is_simpleKeysetExpression(token)) {
      return parse_simpleKeysetExpression();
    } else assert(0);
  } else error("At line %d, column %d: keyset expression was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_selectCase()
{
  if (token_is_keysetExpression(token)) {
    Ast_SelectCase* select_case = arena_push_struct(ast_storage, Ast_SelectCase);
    select_case->kind = AST_selectCase;
    select_case->id = node_id++;
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
parse_selectCaseList()
{
  Ast_List* cases = arena_push_struct(ast_storage, Ast_List);
  cases->kind = AST_selectCaseList;
  cases->id = node_id++;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  list_init(&cases->members);
  if (token_is_selectCase(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&cases->members, li, 1);
    li->object = parse_selectCase();
    while (token_is_selectCase(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_selectCase();
      list_append_item(&cases->members, li, 1);
    }
  }
  return (Ast*)cases;
}

internal Ast*
parse_selectExpression()
{
  if (token->klass == TK_SELECT) {
    next_token();
    Ast_SelectExpression* select_expr = arena_push_struct(ast_storage, Ast_SelectExpression);
    select_expr->kind = AST_selectExpression;
    select_expr->id = node_id++;
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
parse_stateExpression()
{
  if (token_is_name(token) || token->klass == TK_SELECT) {
    Ast_StateExpression* state_expr = arena_push_struct(ast_storage, Ast_StateExpression);
    state_expr->kind = AST_stateExpression;
    state_expr->id = node_id++;
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
parse_transitionStatement()
{
  if (token->klass == TK_TRANSITION) {
    next_token();
    Ast_TransitionStmt* transition = arena_push_struct(ast_storage, Ast_TransitionStmt);
    transition->kind = AST_transitionStatement;
    transition->id = node_id++;
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
parse_parserState()
{
  if (token->klass == TK_STATE) {
    next_token();
    Ast_ParserState* state = arena_push_struct(ast_storage, Ast_ParserState);
    state->kind = AST_parserState;
    state->id = node_id++;
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
parse_parserStates()
{
  Ast_List* states = arena_push_struct(ast_storage, Ast_List);
  states->kind = AST_parserStates;
  states->id = node_id++;
  states->line_no = token->line_no;
  states->column_no = token->column_no;
  list_init(&states->members);
  if (token->klass == TK_STATE) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&states->members, li, 1);
    li->object = parse_parserState();
    while (token->klass == TK_STATE) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_parserState();
      list_append_item(&states->members, li, 1);
    }
  }
  return (Ast*)states;
}

internal Ast*
parse_parserDeclaration()
{
  if (token->klass == TK_PARSER) {
    Ast* parser_proto = parse_parserTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <parserTypeDeclaration> */
      return parser_proto;
    } else {
      Ast_ParserDeclaration* parser_decl = arena_push_struct(ast_storage, Ast_ParserDeclaration);
      parser_decl->kind = AST_parserDeclaration;
      parser_decl->id = node_id++;
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
    }
  } else error("At line %d, column %d: `parser` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_controlTypeDeclaration()
{
  if (token->klass == TK_CONTROL) {
    next_token();
    Ast_ControlPrototype* proto = arena_push_struct(ast_storage, Ast_ControlPrototype);
    proto->kind = AST_controlTypeDeclaration;
    proto->id = node_id++;
    proto->line_no = token->line_no;
    proto->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_actionDeclaration()
{
  if (token->klass == TK_ACTION) {
    next_token();
    Ast_ActionDeclaration* action_decl = arena_push_struct(ast_storage, Ast_ActionDeclaration);
    action_decl->kind = AST_actionDeclaration;
    action_decl->id = node_id++;
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

internal Ast*
parse_keyElement()
{
  if (token_is_expression(token)) {
    Ast_KeyElement* key_elem = arena_push_struct(ast_storage, Ast_KeyElement);
    key_elem->kind = AST_keyElement;
    key_elem->id = node_id++;
    key_elem->line_no = token->line_no;
    key_elem->column_no = token->column_no;
    key_elem->expr = parse_expression(1);
    if (token->klass == TK_COLON) {
      next_token();
      key_elem->name = parse_name();
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
parse_keyElementList()
{
  Ast_List* elems = arena_push_struct(ast_storage, Ast_List);
  elems->kind = AST_keyElementList;
  elems->id = node_id++;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  list_init(&elems->members);
  if (token_is_expression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&elems->members, li, 1);
    li->object = parse_keyElement();
    while (token_is_expression(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_keyElement();
      list_append_item(&elems->members, li, 1);
    }
  }
  return (Ast*)elems;
}

internal Ast*
parse_actionRef()
{
  if (token_is_prefixedNonTypeName(token)) {
    Ast_ActionRef* action_ref = arena_push_struct(ast_storage, Ast_ActionRef);
    action_ref->kind = AST_actionRef;
    action_ref->id = node_id++;
    action_ref->line_no = token->line_no;
    action_ref->column_no = token->column_no;
    action_ref->name = parse_prefixedNonTypeName();
    action_ref->args = parse_optArguments();
    return (Ast*)action_ref;
  } else error("At line %d, column %d: non-type name was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_actionList()
{
  Ast_List* actions = arena_push_struct(ast_storage, Ast_List);
  actions->kind = AST_actionList;
  actions->id = node_id++;
  actions->line_no = token->line_no;
  actions->column_no = token->column_no;
  list_init(&actions->members);
  if (token_is_actionRef(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&actions->members, li, 1);
    li->object = parse_actionRef();
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
parse_entry()
{
  if (token_is_keysetExpression(token)) {
    Ast_Entry* entry = arena_push_struct(ast_storage, Ast_Entry);
    entry->kind = AST_entry;
    entry->id = node_id++;
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
parse_entriesList()
{
  Ast_List* entries = arena_push_struct(ast_storage, Ast_List);
  entries->kind = AST_entriesList;
  entries->id = node_id++;
  entries->line_no = token->line_no;
  entries->column_no = token->column_no;
  list_init(&entries->members);
  if (token_is_keysetExpression(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&entries->members, li, 1);
    li->object = parse_entry();
    while (token_is_keysetExpression(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_entry();
      list_append_item(&entries->members, li, 1);
    }
  }
  return (Ast*)entries;
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
    table_prop->id = node_id++;
    table_prop->line_no = token->line_no;
    table_prop->column_no = token->column_no;
    if (token->klass == TK_KEY) {
      next_token();
      Ast_TableKey* key_prop = arena_push_struct(ast_storage, Ast_TableKey);
      key_prop->kind = AST_tableKey;
      key_prop->id = node_id++;
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
      Ast_TableActions* actions_prop = arena_push_struct(ast_storage, Ast_TableActions);
      actions_prop->kind = AST_tableActions;
      actions_prop->id = node_id++;
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
      Ast_TableEntries* entries_prop = arena_push_struct(ast_storage, Ast_TableEntries);
      entries_prop->kind = AST_tableEntries;
      entries_prop->id = node_id++;
      entries_prop->line_no = token->line_no;
      entries_prop->column_no = token->column_no;
      entries_prop->is_const = is_const;
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
      Ast_SimpleTableProperty* simple_prop = arena_push_struct(ast_storage, Ast_SimpleTableProperty);
      simple_prop->kind = AST_simpleTableProperty;
      simple_prop->id = node_id++;
      simple_prop->line_no = token->line_no;
      simple_prop->column_no = token->column_no;
      simple_prop->is_const = is_const;
      simple_prop->name = parse_name();
      if (token->klass == TK_EQUAL) {
        next_token();
        simple_prop->init_expr = parse_initializer();
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
parse_tablePropertyList()
{
  Ast_List* props = arena_push_struct(ast_storage, Ast_List);
  props->kind = AST_tablePropertyList;
  props->id = node_id++;
  props->line_no = token->line_no;
  props->column_no = token->column_no;
  list_init(&props->members);
  if (token_is_tableProperty(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&props->members, li, 1);
    li->object = parse_tableProperty();
    while (token_is_tableProperty(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_tableProperty();
      list_append_item(&props->members, li, 1);
    }
  }
  return (Ast*)props;
}

internal Ast*
parse_tableDeclaration()
{
  if (token->klass == TK_TABLE) {
    next_token();
    Ast_TableDeclaration* table = arena_push_struct(ast_storage, Ast_TableDeclaration);
    table->kind = AST_tableDeclaration;
    table->id = node_id++;
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
parse_controlLocalDeclaration()
{
  if (token_is_controlLocalDeclaration(token)) {
    Ast_ControlLocalDeclaration* local_decl = arena_push_struct(ast_storage, Ast_ControlLocalDeclaration);
    local_decl->kind = AST_controlLocalDeclaration;
    local_decl->id = node_id++;
    local_decl->line_no = token->line_no;
    local_decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      local_decl->decl = parse_constantDeclaration();
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
  Ast_List* decls = arena_push_struct(ast_storage, Ast_List);
  decls->kind = AST_controlLocalDeclarations;
  decls->id = node_id++;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  list_init(&decls->members);
  if (token_is_controlLocalDeclaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&decls->members, li, 1);
    li->object = parse_controlLocalDeclaration();
    while (token_is_controlLocalDeclaration(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_controlLocalDeclaration();
      list_append_item(&decls->members, li, 1);
    }
  }
  return (Ast*)decls;
}

internal Ast*
parse_controlDeclaration()
{
  if (token->klass == TK_CONTROL) {
    Ast* control_proto = parse_controlTypeDeclaration();
    if (token->klass == TK_SEMICOLON) {
      next_token(); /* <controlTypeDeclaration> */
      return control_proto;
    } else {
      Ast_ControlDeclaration* control_decl = arena_push_struct(ast_storage, Ast_ControlDeclaration);
      control_decl->kind = AST_controlDeclaration;
      control_decl->id = node_id++;
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
    }
  } else error("At line %d, column %d: `control` was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_packageTypeDeclaration()
{
  if (token->klass == TK_PACKAGE) {
    next_token();
    Ast_PackageTypeDeclaration* package_decl = arena_push_struct(ast_storage, Ast_PackageTypeDeclaration);
    package_decl->kind = AST_packageTypeDeclaration;
    package_decl->id = node_id++;
    package_decl->line_no = token->line_no;
    package_decl->column_no = token->column_no;
    if (token_is_name(token)) {
      Ast_Name* name = (Ast_Name*)parse_name();
      declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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
parse_typedefDeclaration()
{
  if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
    next_token();
    if (token_is_typeRef(token) || token_is_derivedTypeDeclaration(token)) {
      Ast_TypedefDeclaration* type_decl = arena_push_struct(ast_storage, Ast_TypedefDeclaration);
      type_decl->kind = AST_typedefDeclaration;
      type_decl->id = node_id++;
      type_decl->line_no = token->line_no;
      type_decl->column_no = token->column_no;
      if (token_is_typeRef(token)) {
        type_decl->type_ref = parse_typeRef();
      } else if (token_is_derivedTypeDeclaration(token)) {
        type_decl->type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token_is_name(token)) {
        Ast_Name* name = (Ast_Name*)parse_name();
        declare_type_name(current_scope, name->strname, name->line_no, name->column_no, 0);
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

internal Ast*
parse_typeDeclaration()
{
  if (token_is_typeDeclaration(token)) {
    Ast_TypeDeclaration* type_decl = arena_push_struct(ast_storage, Ast_TypeDeclaration);
    type_decl->kind = AST_typeDeclaration;
    type_decl->id = node_id++;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token_is_derivedTypeDeclaration(token)) {
      type_decl->decl = parse_derivedTypeDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_TYPEDEF || token->klass == TK_TYPE) {
      type_decl->decl = parse_typedefDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_PARSER) {
      /* <parserTypeDeclaration> | <parserDeclaration> */
      type_decl->decl = parse_parserDeclaration();
      return (Ast*)type_decl;
    } else if (token->klass == TK_CONTROL) {
      /* <controlTypeDeclaration> | <controlDeclaration> */
      type_decl->decl = parse_controlDeclaration();
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
parse_conditionalStatement()
{
  if (token->klass == TK_IF) {
    next_token();
    Ast_ConditionalStmt* if_stmt = arena_push_struct(ast_storage, Ast_ConditionalStmt);
    if_stmt->kind = AST_conditionalStatement;
    if_stmt->id = node_id++;
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
parse_exitStatement()
{
  if (token->klass == TK_EXIT) {
    next_token();
    Ast* exit_stmt = arena_push_struct(ast_storage, Ast);
    exit_stmt->kind = AST_exitStatement;
    exit_stmt->id = node_id++;
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

internal Ast*
parse_returnStatement()
{
  if (token->klass == TK_RETURN) {
    next_token();
    Ast_ReturnStmt* return_stmt = arena_push_struct(ast_storage, Ast_ReturnStmt);
    return_stmt->kind = AST_returnStatement;
    return_stmt->id = node_id++;
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
parse_switchLabel()
{
  if (token_is_switchLabel(token)) {
    Ast_SwitchLabel* switch_label = arena_push_struct(ast_storage, Ast_SwitchLabel);
    switch_label->kind = AST_switchLabel;
    switch_label->id = node_id++;
    switch_label->line_no = token->line_no;
    switch_label->column_no = token->column_no;
    if (token_is_name(token)) {
      switch_label->label = parse_name();
      return (Ast*)switch_label;
    } else if (token->klass == TK_DEFAULT) {
      next_token();
      Ast* default_label = arena_push_struct(ast_storage, Ast);
      default_label->kind = AST_defaultSwitchLabel;
      default_label->id = node_id++;
      default_label->line_no = token->line_no;
      default_label->column_no = token->column_no;
      switch_label->label = default_label;
      return (Ast*)switch_label;
    } else assert(0);
  } else error("At line %d, column %d: switch label was expected, got `%s`.",
               token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

internal Ast*
parse_switchCase()
{
  if (token_is_switchLabel(token)) {
    Ast_SwitchCase* switch_case = arena_push_struct(ast_storage, Ast_SwitchCase);
    switch_case->kind = AST_switchCase;
    switch_case->id = node_id++;
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
parse_switchCases()
{
  Ast_List* cases = arena_push_struct(ast_storage, Ast_List);
  cases->kind = AST_switchCases;
  cases->id = node_id++;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  list_init(&cases->members);
  if (token_is_switchLabel(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&cases->members, li, 1);
    li->object = parse_switchCase();
    while (token_is_switchLabel(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_switchCase();
      list_append_item(&cases->members, li, 1);
    }
  }
  return (Ast*)cases;
}

internal Ast*
parse_switchStatement()
{
  if (token->klass == TK_SWITCH) {
    next_token();
    Ast_SwitchStmt* stmt = arena_push_struct(ast_storage, Ast_SwitchStmt);
    stmt->kind = AST_switchStatement;
    stmt->id = node_id++;
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
parse_statement(Ast* type_name)
{
  if (token_is_statement(token)) {
    Ast_Statement* stmt = arena_push_struct(ast_storage, Ast_Statement);
    stmt->kind = AST_statement;
    stmt->id = node_id++;
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
      empty_stmt->id = node_id++;
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
parse_statementOrDecl()
{
  if (token_is_statementOrDeclaration(token)) {
    Ast_Statement* stmt = arena_push_struct(ast_storage, Ast_Statement);
    stmt->kind = AST_statementOrDecl;
    stmt->id = node_id++;
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
      stmt->stmt = parse_constantDeclaration();
      return (Ast*)stmt;
    } else assert(0);
    assert(0);
  }
  assert(0);
  return 0;
}

internal Ast*
parse_statementOrDeclList()
{
  Ast_List* stmts = arena_push_struct(ast_storage, Ast_List);
  stmts->kind = AST_statementOrDeclList;
  stmts->id = node_id++;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  list_init(&stmts->members);
  if (token_is_statementOrDeclaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&stmts->members, li, 1);
    li->object = parse_statementOrDecl();
    while (token_is_statementOrDeclaration(token)) {
      li = arena_push_struct(ast_storage, ListItem);
      li->object = parse_statementOrDecl();
      list_append_item(&stmts->members, li, 1);
    }
  }
  return (Ast*)stmts;
}

internal Ast*
parse_blockStatement()
{
  if (token->klass == TK_BRACE_OPEN) {
    next_token();
    Ast_BlockStmt* block_stmt = arena_push_struct(ast_storage, Ast_BlockStmt);
    block_stmt->kind = AST_blockStatement;
    block_stmt->id = node_id++;
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
parse_identifierList()
{
  Ast_List* ids = arena_push_struct(ast_storage, Ast_List);
  ids->kind = AST_identifierList;
  ids->id = node_id++;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  list_init(&ids->members);
  if (token_is_name(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&ids->members, li, 1);
    li->object = parse_name();
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
parse_errorDeclaration()
{
  if (token->klass == TK_ERROR) {
    next_token();
    Ast_ErrorDeclaration* error_decl = arena_push_struct(ast_storage, Ast_ErrorDeclaration);
    error_decl->kind = AST_errorDeclaration;
    error_decl->id = node_id++;
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
    match_decl->id = node_id++;
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
parse_functionDeclaration(Ast* type_ref)
{
  if (token_is_typeOrVoid(token)) {
    Ast_FunctionDeclaration* func_decl = arena_push_struct(ast_storage, Ast_FunctionDeclaration);
    func_decl->kind = AST_functionDeclaration;
    func_decl->id = node_id++;
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
parse_declaration()
{
  if (token_is_declaration(token)) {
    Ast_Declaration* decl = arena_push_struct(ast_storage, Ast_Declaration);
    decl->kind = AST_declaration;
    decl->id = node_id++;
    decl->line_no = token->line_no;
    decl->column_no = token->column_no;
    if (token->klass == TK_CONST) {
      decl->decl = parse_constantDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_EXTERN) {
      decl->decl = parse_externDeclaration();
      return (Ast*)decl;
    } else if (token->klass == TK_ACTION) {
      decl->decl = parse_actionDeclaration();
      return (Ast*)decl;
    } else if (token_is_typeDeclaration(token)) {
      /* <parserDeclaration> | <controlDeclaration> */
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
parse_declarationList()
{
  Ast_List* decls = arena_push_struct(ast_storage, Ast_List);
  decls->kind = AST_declarationList;
  decls->id = node_id++;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  list_init(&decls->members);
  if (token_is_declaration(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&decls->members, li, 1);
    li->object = parse_declaration();
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
parse_p4program()
{
  Ast_P4Program* program = arena_push_struct(ast_storage, Ast_P4Program);
  program->kind = AST_p4program;
  program->id = node_id++;
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
parse_realTypeArg()
{
  if (token_is_realTypeArg(token)) {
    Ast_RealTypeArgument* type_arg = arena_push_struct(ast_storage, Ast_RealTypeArgument);
    type_arg->kind = AST_realTypeArgument;
    type_arg->id = node_id++;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TK_DONTCARE) {
      next_token();
      Ast* dontcare_arg = arena_push_struct(ast_storage, Ast);
      dontcare_arg->kind = AST_dontcareTypeArgument;
      dontcare_arg->id = node_id++;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->arg = dontcare_arg;
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
parse_realTypeArgumentList()
{
  Ast_List* args = arena_push_struct(ast_storage, Ast_List);
  args->kind = AST_realTypeArgumentList;
  args->id = node_id++;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  list_init(&args->members);
  if (token_is_realTypeArg(token)) {
    ListItem* li = arena_push_struct(ast_storage, ListItem);
    list_append_item(&args->members, li, 1);
    li->object = parse_realTypeArg();
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
parse_expressionPrimary()
{
  if (token_is_expression(token)) {
    Ast_Expression* primary = arena_push_struct(ast_storage, Ast_Expression);
    primary->kind = AST_expression;
    primary->id = node_id++;
    primary->line_no = token->line_no;
    primary->column_no = token->column_no;
    if (token->klass == TK_INTEGER_LITERAL) {
      primary->expr = parse_integer();
      return (Ast*)primary;
    } else if (token->klass == TK_TRUE || token->klass == TK_FALSE) {
      primary->expr = parse_boolean();
      return (Ast*)primary;
    } else if (token->klass == TK_STRING_LITERAL) {
      primary->expr = parse_stringLiteral();
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
        cast_expr->id = node_id++;
        cast_expr->line_no = token->line_no;
        cast_expr->column_no = token->column_no;
        cast_expr->to_type = parse_typeRef();
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
      unary_expr->id = node_id++;
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
      unary_expr->id = node_id++;
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
      unary_expr->id = node_id++;
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
      primary->expr = parse_baseType();
      return (Ast*)primary;
    } else assert(0);
    assert(0);
  } else error("At line %d, column %d: expression was expected, got `%s`.",
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
        member_expr->id = node_id++;
        member_expr->line_no = token->line_no;
        member_expr->column_no = token->column_no;
        member_expr->lhs_expr = (Ast*)expr;
        if (token_is_nonTypeName(token)) {
          member_expr->member_name = parse_nonTypeName();
        } else error("At line %d, column %d: non-type name was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->id = node_id++;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)member_expr;
      } else if (token->klass == TK_BRACKET_OPEN) {
        next_token();
        Ast_ArraySubscript* subscript_expr = arena_push_struct(ast_storage, Ast_ArraySubscript);
        subscript_expr->kind = AST_arraySubscript;
        subscript_expr->id = node_id++;
        subscript_expr->line_no = token->line_no;
        subscript_expr->column_no = token->column_no;
        subscript_expr->lhs_expr = (Ast*)expr;
        subscript_expr->index_expr = parse_arrayIndex();
        if (token->klass == TK_BRACKET_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `]` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->id = node_id++;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)subscript_expr;
      } else if (token->klass == TK_PARENTH_OPEN) {
        next_token();
        Ast_FunctionCall* call_expr = arena_push_struct(ast_storage, Ast_FunctionCall);
        call_expr->kind = AST_functionCall;
        call_expr->id = node_id++;
        call_expr->line_no = token->line_no;
        call_expr->column_no = token->column_no;
        call_expr->callee_expr = (Ast*)expr;
        call_expr->args = parse_argumentList();
        if (token->klass == TK_PARENTH_CLOSE) {
          next_token();
        } else error("At line %d, column %d: `)` was expected, got `%s`.",
                     token->line_no, token->column_no, token->lexeme);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->id = node_id++;
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
        Ast_KVPair* kv_pair = arena_push_struct(ast_storage, Ast_KVPair);
        kv_pair->kind = AST_kvPair;
        kv_pair->id = node_id++;
        kv_pair->line_no = token->line_no;
        kv_pair->column_no = token->column_no;
        kv_pair->name = (Ast*)expr;
        kv_pair->expr = parse_expression(1);
        expr = arena_push_struct(ast_storage, Ast_Expression);
        expr->kind = AST_expression;
        expr->id = node_id++;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->expr = (Ast*)kv_pair;
      } else if (token_is_binaryOperator(token)){
        int priority = get_operator_priority(token);
        if (priority >= priority_threshold) {
          next_token();
          Ast_BinaryExpression* binary_expr = arena_push_struct(ast_storage, Ast_BinaryExpression);
          binary_expr->kind = AST_binaryExpression;
          binary_expr->id = node_id++;
          binary_expr->line_no = token->line_no;
          binary_expr->column_no = token->column_no;
          binary_expr->left_operand = (Ast*)expr;
          binary_expr->op = token_to_binop(token);
          binary_expr->right_operand = parse_expression(priority + 1);
          expr = arena_push_struct(ast_storage, Ast_Expression);
          expr->kind = AST_expression;
          expr->id = node_id++;
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

Ast_P4Program*
parse_tokens(UnboundedArray* tokens_, Arena* ast_storage_)
{
  tokens = tokens_;
  ast_storage = ast_storage_;
  symbol_table_init(ast_storage);
  root_scope = current_scope = push_scope();

  declare_keyword(root_scope, "action", TK_ACTION);
  declare_keyword(root_scope, "actions", TK_ACTIONS);
  declare_keyword(root_scope, "entries", TK_ENTRIES);
  declare_keyword(root_scope, "enum", TK_ENUM);
  declare_keyword(root_scope, "in", TK_IN);
  declare_keyword(root_scope, "package", TK_PACKAGE);
  declare_keyword(root_scope, "select", TK_SELECT);
  declare_keyword(root_scope, "switch", TK_SWITCH);
  declare_keyword(root_scope, "tuple", TK_TUPLE);
  declare_keyword(root_scope, "control", TK_CONTROL);
  declare_keyword(root_scope, "error", TK_ERROR);
  declare_keyword(root_scope, "header", TK_HEADER);
  declare_keyword(root_scope, "inout", TK_INOUT);
  declare_keyword(root_scope, "parser", TK_PARSER);
  declare_keyword(root_scope, "state", TK_STATE);
  declare_keyword(root_scope, "table", TK_TABLE);
  declare_keyword(root_scope, "key", TK_KEY);
  declare_keyword(root_scope, "typedef", TK_TYPEDEF);
  declare_keyword(root_scope, "type", TK_TYPE);
  declare_keyword(root_scope, "default", TK_DEFAULT);
  declare_keyword(root_scope, "extern", TK_EXTERN);
  declare_keyword(root_scope, "header_union", TK_HEADER_UNION);
  declare_keyword(root_scope, "out", TK_OUT);
  declare_keyword(root_scope, "transition", TK_TRANSITION);
  declare_keyword(root_scope, "else", TK_ELSE);
  declare_keyword(root_scope, "exit", TK_EXIT);
  declare_keyword(root_scope, "if", TK_IF);
  declare_keyword(root_scope, "match_kind", TK_MATCH_KIND);
  declare_keyword(root_scope, "return", TK_RETURN);
  declare_keyword(root_scope, "struct", TK_STRUCT);
  declare_keyword(root_scope, "apply", TK_APPLY);
  declare_keyword(root_scope, "const", TK_CONST);
  declare_keyword(root_scope, "bool", TK_BOOL);
  declare_keyword(root_scope, "true", TK_TRUE);
  declare_keyword(root_scope, "false", TK_FALSE);
  declare_keyword(root_scope, "void", TK_VOID);
  declare_keyword(root_scope, "int", TK_INT);
  declare_keyword(root_scope, "bit", TK_BIT);
  declare_keyword(root_scope, "varbit", TK_VARBIT);
  declare_keyword(root_scope, "string", TK_STRING);

  token_at = 0;
  token = array_get(tokens, token_at);
  next_token();
  Ast_P4Program* p4program = (Ast_P4Program*)parse_p4program();
  p4program->last_node_id = node_id;
  current_scope = pop_scope();
  assert(current_scope == 0);
  return p4program;
}
