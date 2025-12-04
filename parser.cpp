#include <basic.h>
#include <parser.h>

struct Keyword {
  char* strname;
  enum TokenClass token_class;
};

void Parser::define_keywords(Scope* scope)
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
    name_decl = scope->bind_name(storage, keywords[i].strname, NameSpace::KEYWORD);
    name_decl->token_class = keywords[i].token_class;
  }
}

Token* Parser::next_token()
{
  assert(token_at < tokens->elem_count);
  NameEntry* name_entry;
  NameDeclaration* name_decl;

  prev_token = token;
  prev_token_at = token_at;
  token = (Token*)tokens->get(++token_at);
  while (token->klass == TokenClass::COMMENT) {
    token = (Token*)tokens->get(++token_at);
  }
  if (token->klass == TokenClass::IDENTIFIER) {
    name_entry = current_scope->lookup(token->lexeme, NameSpace::KEYWORD | NameSpace::TYPE);
    name_decl = name_entry->ns[(int)NameSpace::KEYWORD >> 1];
    if (name_decl) {
      token->klass = name_decl->token_class;
      return token;
    }
    name_decl = name_entry->ns[(int)NameSpace::TYPE >> 1];
    if (name_decl) {
      token->klass = TokenClass::TYPE_IDENTIFIER;
      return token;
    }
  }
  return token;
}

Token* Parser::peek_token()
{
  Token* peek_token;

  prev_token = token;
  prev_token_at = token_at;
  peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
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

enum AstOperator token_to_binop(Token* token)
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

void Parser::parse()
{
  root_scope = Scope::create(storage, 5);
  current_scope = root_scope;

  define_keywords(root_scope);
  token_at = 0;
  token = (Token*)tokens->get(token_at);
  next_token();
  p4program = parse_p4program();
  assert(current_scope == root_scope);
}

/** PROGRAM **/

Ast* Parser::parse_p4program()
{
  Ast* p4program;
  Scope* scope;

  p4program = (Ast*)storage->malloc(sizeof(Ast));
  p4program->kind = AstEnum::p4program;
  p4program->line_no = token->line_no;
  p4program->column_no = token->column_no;
  while (token->klass == TokenClass::SEMICOLON) {
    next_token(); /* empty declaration */
  }
  scope = Scope::create(storage, 6);
  current_scope = scope->push(current_scope);
  p4program->p4program.decl_list = parse_declarationList();
  current_scope = current_scope->pop();
  if (token->klass != TokenClass::END_OF_INPUT) {
    error("%s:%d:%d: error: unexpected token `%s`.",
          source_file, token->line_no, token->column_no, token->lexeme);
  }
  return p4program;
}

Ast* Parser::parse_declarationList()
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {};

  decls = (Ast*)storage->malloc(sizeof(Ast));
  decls->kind = AstEnum::declarationList;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token->token_is_declaration()) {
    ast = parse_declaration();
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token->token_is_declaration() || token->klass == TokenClass::SEMICOLON) {
      if (token->token_is_declaration()) {
        ast = parse_declaration();
        tree_ctor.append_node(&decls->tree, &ast->tree);
      } else if (token->klass == TokenClass::SEMICOLON) {
        next_token(); /* empty declaration */
      }
    }
  }
  return decls;
}

Ast* Parser::parse_declaration()
{
  Ast* decl;

  if (token->token_is_declaration()) {
    decl = (Ast*)storage->malloc(sizeof(Ast));
    decl->kind = AstEnum::declaration;
    decl->line_no = token->line_no;
    decl->column_no = token->column_no;
    if (token->klass == TokenClass::CONST) {
      decl->declaration.decl = parse_variableDeclaration(0);
      return decl;
    } else if (token->klass == TokenClass::EXTERN) {
      decl->declaration.decl = parse_externDeclaration();
      return decl;
    } else if (token->klass == TokenClass::ACTION) {
      decl->declaration.decl = parse_actionDeclaration();
      return decl;
    } else if (token->klass == TokenClass::PARSER) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else {
        decl->declaration.decl = parse_parserDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token->klass == TokenClass::CONTROL) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else {
        decl->declaration.decl = parse_controlDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token->token_is_typeDeclaration()) {
      decl->declaration.decl = parse_typeDeclaration();
      return decl;
    } else if (token->klass == TokenClass::ERROR) {
      decl->declaration.decl = parse_errorDeclaration();
      return decl;
    } else if (token->klass == TokenClass::MATCH_KIND) {
      decl->declaration.decl = parse_matchKindDeclaration();
      return decl;
    } else if (token->token_is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        decl->declaration.decl = parse_instantiation(type_ref);
        return decl;
      } else if (token->token_is_name()) {
        decl->declaration.decl = parse_functionDeclaration(type_ref);
        return decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->token_is_typeOrVoid()) {
      decl->declaration.decl = parse_functionDeclaration(parse_typeRef());
      return decl;
    } else assert(0);
  } else error("%s:%d:%d: error: top-level declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_nonTypeName()
{
  Ast* name;

  if (token->token_is_nonTypeName()) {
    name = (Ast*)storage->malloc(sizeof(Ast));
    name->kind = AstEnum::name;
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

Ast* Parser::parse_name()
{
  Ast* type_name;

  if (token->token_is_name()) {
    if (token->token_is_nonTypeName()) {
      return parse_nonTypeName();
    } else if (token->klass == TokenClass::TYPE_IDENTIFIER) {
      type_name = (Ast*)storage->malloc(sizeof(Ast));
      type_name->kind = AstEnum::name;
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

Ast* Parser::parse_parameterList()
{
  Ast* params, *ast;
  AstTreeCtor tree_ctor = {};

  params = (Ast*)storage->malloc(sizeof(Ast));
  params->kind = AstEnum::parameterList;
  params->line_no = token->line_no;
  params->column_no = token->column_no;
  if (token->token_is_parameter()) {
    ast = parse_parameter();
    tree_ctor.append_node(&params->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_parameter();
      tree_ctor.append_node(&params->tree, &ast->tree);
    }
  }
  return params;
}

Ast* Parser::parse_parameter()
{
  Ast* param;

  if (token->token_is_parameter()) {
    param = (Ast*)storage->malloc(sizeof(Ast));
    param->kind = AstEnum::parameter;
    param->line_no = token->line_no;
    param->column_no = token->column_no;
    param->parameter.direction = parse_direction();
    param->parameter.type = parse_typeRef();
    if (token->token_is_name()) {
      param->parameter.name = parse_name();
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        if (token->token_is_expression()) {
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

enum ParamDirection Parser::parse_direction()
{
  if (token->token_is_direction()) {
    if (token->klass == TokenClass::IN) {
      next_token();
      return ParamDirection::IN;
    } else if (token->klass == TokenClass::OUT) {
      next_token();
      return ParamDirection::OUT;
    } else if (token->klass == TokenClass::INOUT) {
      next_token();
      return ParamDirection::IN | ParamDirection::OUT;
    } else assert(0);
  }
  return (ParamDirection)0;
}

Ast* Parser::parse_packageTypeDeclaration()
{
  Ast* package_decl, *name;

  if (token->klass == TokenClass::PACKAGE) {
    next_token();
    package_decl = (Ast*)storage->malloc(sizeof(Ast));
    package_decl->kind = AstEnum::packageTypeDeclaration;
    package_decl->line_no = token->line_no;
    package_decl->column_no = token->column_no;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      package_decl->packageTypeDeclaration.name = name;
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        package_decl->packageTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
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

Ast* Parser::parse_instantiation(Ast* type_ref)
{
  Ast* inst_stmt;

  if (token->token_is_typeRef() || type_ref) {
    inst_stmt = (Ast*)storage->malloc(sizeof(Ast));
    inst_stmt->kind = AstEnum::instantiation;
    inst_stmt->line_no = token->line_no;
    inst_stmt->column_no = token->column_no;
    inst_stmt->instantiation.type = type_ref ? type_ref : parse_typeRef();
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      inst_stmt->instantiation.args = parse_argumentList();
      if (token->klass == TokenClass::PARENTH_CLOSE) {
        next_token();
        if (token->token_is_name()) {
          inst_stmt->instantiation.name = parse_name();
          if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_constructorParameters()
{
   Ast* params;

  if (token->klass == TokenClass::PARENTH_OPEN) {
    next_token();
    params = parse_parameterList();
    if (token->klass == TokenClass::PARENTH_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return params;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  return 0;
}

Ast* Parser::parse_parserDeclaration(Ast* parser_proto)
{
  Ast* parser_decl;

  if (token->klass == TokenClass::PARENTH_OPEN || token->klass == TokenClass::BRACE_OPEN) {
    parser_decl = (Ast*)storage->malloc(sizeof(Ast));
    parser_decl->kind = AstEnum::parserDeclaration;
    parser_decl->line_no = token->line_no;
    parser_decl->column_no = token->column_no;
    parser_decl->parserDeclaration.proto = parser_proto;
    parser_decl->parserDeclaration.ctor_params = parse_constructorParameters();
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      parser_decl->parserDeclaration.local_elements = parse_parserLocalElements();
      if (token->klass == TokenClass::STATE) {
        parser_decl->parserDeclaration.states = parse_parserStates();
      } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_parserLocalElements()
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {};

  elems = (Ast*)storage->malloc(sizeof(Ast));
  elems->kind = AstEnum::parserLocalElements;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token->token_is_parserLocalElement()) {
    ast = parse_parserLocalElement();
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token->token_is_parserLocalElement()) {
      ast = parse_parserLocalElement();
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_parserLocalElement()
{
  Ast* local_element, *type_ref;

  if (token->token_is_parserLocalElement()) {
    local_element = (Ast*)storage->malloc(sizeof(Ast));
    local_element->kind = AstEnum::parserLocalElement;
    local_element->line_no = token->line_no;
    local_element->column_no = token->column_no;
    if (token->klass == TokenClass::CONST) {
      local_element->parserLocalElement.element = parse_variableDeclaration(0);
      return local_element;
    } else if (token->token_is_typeRef()) {
      type_ref = parse_typeRef();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        local_element->parserLocalElement.element = parse_instantiation(type_ref);
        return local_element;
      } else if (token->token_is_name()) {
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

Ast* Parser::parse_parserTypeDeclaration()
{
  Ast* parser_proto, *name, *method_protos;

  if (token->klass == TokenClass::PARSER) {
    next_token();
    parser_proto = (Ast*)storage->malloc(sizeof(Ast));
    parser_proto->kind = AstEnum::parserTypeDeclaration;
    parser_proto->line_no = token->line_no;
    parser_proto->column_no = token->column_no;
    method_protos = (Ast*)storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = parser_proto->line_no;
    method_protos->column_no = parser_proto->column_no;
    parser_proto->parserTypeDeclaration.method_protos = method_protos;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      parser_proto->parserTypeDeclaration.name = name;
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        parser_proto->parserTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
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

Ast* Parser::parse_parserStates()
{
  Ast* states, *ast;
  AstTreeCtor tree_ctor = {};

  states = (Ast*)storage->malloc(sizeof(Ast));
  states->kind = AstEnum::parserStates;
  states->line_no = token->line_no;
  states->column_no = token->column_no;
  if (token->klass == TokenClass::STATE) {
    ast = parse_parserState();
    tree_ctor.append_node(&states->tree, &ast->tree);
    while (token->klass == TokenClass::STATE) {
      ast = parse_parserState();
      tree_ctor.append_node(&states->tree, &ast->tree);
    }
  }
  return states;
}

Ast* Parser::parse_parserState()
{
  Ast* state;

  if (token->klass == TokenClass::STATE) {
    next_token();
    state = (Ast*)storage->malloc(sizeof(Ast));
    state->kind = AstEnum::parserState;
    state->line_no = token->line_no;
    state->column_no = token->column_no;
    state->parserState.name = parse_name();
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      state->parserState.stmt_list = parse_parserStatements();
      state->parserState.transition_stmt = parse_transitionStatement();
      if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_parserStatements()
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {};

  stmts = (Ast*)storage->malloc(sizeof(Ast));
  stmts->kind = AstEnum::parserStatements;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token->token_is_parserStatement()) {
    ast = parse_parserStatement();
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token->token_is_parserStatement()) {
      ast = parse_parserStatement();
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_parserStatement()
{
  Ast* parser_stmt, *type_ref;

  if (token->token_is_parserStatement()) {
    parser_stmt = (Ast*)storage->malloc(sizeof(Ast));
    parser_stmt->kind = AstEnum::parserStatement;
    parser_stmt->line_no = token->line_no;
    parser_stmt->column_no = token->column_no;
    if (token->token_is_typeRef()) {
      type_ref = parse_typeRef();
      if (token->token_is_name()) {
        parser_stmt->parserStatement.stmt = parse_variableDeclaration(type_ref);
        return parser_stmt;
      } else {
        parser_stmt->parserStatement.stmt = parse_directApplication(type_ref);
        return parser_stmt;
      }
    } else if (token->token_is_assignmentOrMethodCallStatement()) {
      parser_stmt->parserStatement.stmt = parse_assignmentOrMethodCallStatement();
      return parser_stmt;
    } else if (token->klass == TokenClass::BRACE_OPEN) {
      parser_stmt->parserStatement.stmt = parse_parserBlockStatement();
      return parser_stmt;
    } else if (token->klass == TokenClass::CONST) {
      parser_stmt->parserStatement.stmt = parse_variableDeclaration(0);
      return parser_stmt;
    } else if (token->klass == TokenClass::SEMICOLON) {
      Ast* stmt = (Ast*)storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::emptyStatement;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      parser_stmt->parserStatement.stmt = stmt;
      next_token();
      return parser_stmt;
    } else assert(0);
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_parserBlockStatement()
{
  Ast* stmt;

  if (token->klass == TokenClass::BRACE_OPEN) {
    next_token();
    stmt = (Ast*)storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::parserBlockStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    stmt->parserBlockStatement.stmt_list = parse_parserStatements();
    if (token->klass == TokenClass::BRACE_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_transitionStatement()
{
  Ast* transition;

  if (token->klass == TokenClass::TRANSITION) {
    next_token();
    transition = (Ast*)storage->malloc(sizeof(Ast));
    transition->kind = AstEnum::transitionStatement;
    transition->line_no = token->line_no;
    transition->column_no = token->column_no;
    transition->transitionStatement.stmt = parse_stateExpression();
    return transition;
  } else error("%s:%d:%d: error: `transition` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_stateExpression()
{
  Ast* state_expr;

  if (token->token_is_name() || token->klass == TokenClass::SELECT) {
    state_expr = (Ast*)storage->malloc(sizeof(Ast));
    state_expr->kind = AstEnum::stateExpression;
    state_expr->line_no = token->line_no;
    state_expr->column_no = token->column_no;
    if (token->token_is_name()) {
      state_expr->stateExpression.expr = parse_name();
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                  source_file, token->line_no, token->column_no, token->lexeme);
      return state_expr;
    } else if (token->klass == TokenClass::SELECT) {
      state_expr->stateExpression.expr = parse_selectExpression();
      return state_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: state expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_selectExpression()
{
  Ast* select_expr;

  if (token->klass == TokenClass::SELECT) {
    next_token();
    select_expr = (Ast*)storage->malloc(sizeof(Ast));
    select_expr->kind = AstEnum::selectExpression;
    select_expr->line_no = token->line_no;
    select_expr->column_no = token->column_no;
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      select_expr->selectExpression.expr_list = parse_expressionList();
      if (token->klass == TokenClass::PARENTH_CLOSE) {
        next_token();
        if (token->klass == TokenClass::BRACE_OPEN) {
          next_token();
          select_expr->selectExpression.case_list = parse_selectCaseList();
          if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_selectCaseList()
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {};

  cases = (Ast*)storage->malloc(sizeof(Ast));
  cases->kind = AstEnum::selectCaseList;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token->token_is_selectCase()) {
    ast = parse_selectCase();
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token->token_is_selectCase()) {
      ast = parse_selectCase();
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_selectCase()
{
  Ast* select_case;

  if (token->token_is_keysetExpression()) {
    select_case = (Ast*)storage->malloc(sizeof(Ast));
    select_case->kind = AstEnum::selectCase;
    select_case->line_no = token->line_no;
    select_case->column_no = token->column_no;
    select_case->selectCase.keyset_expr = parse_keysetExpression();
    if (token->klass == TokenClass::COLON) {
      next_token();
      if (token->token_is_name()) {
        select_case->selectCase.name = parse_name();
        if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_keysetExpression()
{
  Ast* keyset_expr;

  if (token->klass == TokenClass::PARENTH_OPEN || token->token_is_simpleKeysetExpression()) {
    keyset_expr = (Ast*)storage->malloc(sizeof(Ast));
    keyset_expr->kind = AstEnum::keysetExpression;
    keyset_expr->line_no = token->line_no;
    keyset_expr->column_no = token->column_no;
    if (token->klass == TokenClass::PARENTH_OPEN) {
      keyset_expr->keysetExpression.expr = parse_tupleKeysetExpression();
      return keyset_expr;
    } else if (token->token_is_simpleKeysetExpression()) {
      keyset_expr->keysetExpression.expr = parse_simpleKeysetExpression();
      return keyset_expr;
    } else assert(0);
  } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_tupleKeysetExpression()
{
  Ast* tuple_keyset;

  if (token->klass == TokenClass::PARENTH_OPEN) {
    next_token();
    tuple_keyset = (Ast*)storage->malloc(sizeof(Ast));
    tuple_keyset->kind = AstEnum::tupleKeysetExpression;
    tuple_keyset->line_no = token->line_no;
    tuple_keyset->column_no = token->column_no;
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList();
    if (token->klass == TokenClass::PARENTH_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return tuple_keyset;
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_simpleExpressionList()
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {};

  exprs = (Ast*)storage->malloc(sizeof(Ast));
  exprs->kind = AstEnum::simpleExpressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  if (token->token_is_expression()) {
    ast = parse_simpleKeysetExpression();
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_simpleKeysetExpression();
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_simpleKeysetExpression()
{
  Ast* simple_keyset, *default_keyset, *dontcare_keyset;

  if (token->token_is_simpleKeysetExpression()) {
    simple_keyset = (Ast*)storage->malloc(sizeof(Ast));
    simple_keyset->kind = AstEnum::simpleKeysetExpression;
    simple_keyset->line_no = token->line_no;
    simple_keyset->column_no = token->column_no;
    if (token->token_is_expression()) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(1);
      return simple_keyset;
    } else if (token->klass == TokenClass::DEFAULT) {
      next_token();
      default_keyset = (Ast*)storage->malloc(sizeof(Ast));
      default_keyset->kind = AstEnum::default_;
      default_keyset->line_no = token->line_no;
      default_keyset->column_no = token->column_no;
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (token->klass == TokenClass::DONTCARE) {
      next_token();
      dontcare_keyset = (Ast*)storage->malloc(sizeof(Ast));
      dontcare_keyset->kind = AstEnum::dontcare;
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

Ast* Parser::parse_controlDeclaration(Ast* control_proto)
{
  Ast* control_decl;

  if (token->klass == TokenClass::PARENTH_OPEN || token->klass == TokenClass::BRACE_OPEN) {
    control_decl = (Ast*)storage->malloc(sizeof(Ast));
    control_decl->kind = AstEnum::controlDeclaration;
    control_decl->line_no = token->line_no;
    control_decl->column_no = token->column_no;
    control_decl->controlDeclaration.proto = control_proto;
    control_decl->controlDeclaration.ctor_params = parse_constructorParameters();
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      control_decl->controlDeclaration.local_decls = parse_controlLocalDeclarations();
      if (token->klass == TokenClass::APPLY) {
        next_token();
        control_decl->controlDeclaration.apply_stmt = parse_blockStatement();
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_controlTypeDeclaration()
{
  Ast* control_proto, *name, *method_protos;

  if (token->klass == TokenClass::CONTROL) {
    next_token();
    control_proto = (Ast*)storage->malloc(sizeof(Ast));
    control_proto->kind = AstEnum::controlTypeDeclaration;
    control_proto->line_no = token->line_no;
    control_proto->column_no = token->column_no;
    method_protos = (Ast*)storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = control_proto->line_no;
    method_protos->column_no = control_proto->column_no;
    control_proto->controlTypeDeclaration.method_protos = method_protos;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      control_proto->controlTypeDeclaration.name = name;
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        control_proto->controlTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
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

Ast* Parser::parse_controlLocalDeclaration()
{
  Ast* local_decl, *type_ref;

  if (token->token_is_controlLocalDeclaration()) {
    local_decl = (Ast*)storage->malloc(sizeof(Ast));
    local_decl->kind = AstEnum::controlLocalDeclaration;
    local_decl->line_no = token->line_no;
    local_decl->column_no = token->column_no;
    if (token->klass == TokenClass::CONST) {
      local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(0);
      return local_decl;
    } else if (token->klass == TokenClass::ACTION) {
      local_decl->controlLocalDeclaration.decl = parse_actionDeclaration();
      return local_decl;
    } else if (token->klass == TokenClass::TABLE) {
      local_decl->controlLocalDeclaration.decl = parse_tableDeclaration();
      return local_decl;
    } else if (token->token_is_typeRef()) {
      type_ref = parse_typeRef();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(type_ref);
        return local_decl;
      } else if (token->token_is_name()) {
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

Ast* Parser::parse_controlLocalDeclarations()
{
  Ast* decls, *ast;
  AstTreeCtor tree_ctor = {};

  decls = (Ast*)storage->malloc(sizeof(Ast));
  decls->kind = AstEnum::controlLocalDeclarations;
  decls->line_no = token->line_no;
  decls->column_no = token->column_no;
  if (token->token_is_controlLocalDeclaration()) {
    ast = parse_controlLocalDeclaration();
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token->token_is_controlLocalDeclaration()) {
      ast = parse_controlLocalDeclaration();
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

  if (token->klass == TokenClass::EXTERN) {
    next_token();
    extern_decl = (Ast*)storage->malloc(sizeof(Ast));
    extern_decl->kind = AstEnum::externDeclaration;
    extern_decl->line_no = token->line_no;
    extern_decl->column_no = token->column_no;

    if (token->token_is_typeOrVoid() && token->token_is_nonTypeName()) {
      is_function_type = token->token_is_typeOrVoid() && peek_token()->token_is_name();
    } else if (token->token_is_typeOrVoid()) {
      is_function_type = 1;
    } else if (token->token_is_nonTypeName()) {
      is_function_type = 0;
    } else error("%s:%d:%d: error: extern declaration was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);

    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(0);
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return extern_decl;
    } else {
      extern_type = (Ast*)storage->malloc(sizeof(Ast));
      extern_type->kind = AstEnum::externTypeDeclaration;
      extern_type->line_no = token->line_no;
      extern_type->column_no = token->column_no;
      extern_type->externTypeDeclaration.name = parse_nonTypeName();
      name = extern_type->externTypeDeclaration.name;
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      if (token->klass == TokenClass::BRACE_OPEN) {
        next_token();
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes();
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_methodPrototypes()
{
  Ast* protos, *ast;
  AstTreeCtor tree_ctor = {};

  protos = (Ast*)storage->malloc(sizeof(Ast));
  protos->kind = AstEnum::methodPrototypes;
  protos->line_no = token->line_no;
  protos->column_no = token->column_no;
  if (token->token_is_methodPrototype()) {
    ast = parse_methodPrototype();
    tree_ctor.append_node(&protos->tree, &ast->tree);
    while (token->token_is_methodPrototype()) {
      ast = parse_methodPrototype();
      tree_ctor.append_node(&protos->tree, &ast->tree);
    }
  }
  return protos;
}

Ast* Parser::parse_functionPrototype(Ast* return_type)
{
  Ast* func_proto, *type_ref;
  Ast* name;

  if (token->token_is_typeOrVoid() || return_type) {
    func_proto = (Ast*)storage->malloc(sizeof(Ast));
    func_proto->kind = AstEnum::functionPrototype;
    func_proto->line_no = token->line_no;
    func_proto->column_no = token->column_no;
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      return_type = parse_typeOrVoid();
      if (return_type->kind == AstEnum::name) {
        name = return_type;
        current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
        type_ref = (Ast*)storage->malloc(sizeof(Ast));
        type_ref->kind = AstEnum::typeRef;
        type_ref->line_no = token->line_no;
        type_ref->column_no = token->column_no;
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token->token_is_name()) {
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
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

Ast* Parser::parse_methodPrototype()
{
  Ast* func_proto;

  if (token->token_is_methodPrototype()) {
    if (token->klass == TokenClass::TYPE_IDENTIFIER && peek_token()->klass == TokenClass::PARENTH_OPEN) {
      /* Constructor */
      func_proto = (Ast*)storage->malloc(sizeof(Ast));
      func_proto->kind = AstEnum::functionPrototype;
      func_proto->line_no = token->line_no;
      func_proto->column_no = token->column_no;
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else if (token->token_is_typeOrVoid()) {
      func_proto = parse_functionPrototype(0);
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_typeRef()
{
  Ast* type_ref;

  if (token->token_is_typeRef()) {
    type_ref = (Ast*)storage->malloc(sizeof(Ast));
    type_ref->kind = AstEnum::typeRef;
    type_ref->line_no = token->line_no;
    type_ref->column_no = token->column_no;
    if (token->token_is_baseType()) {
      type_ref->typeRef.type = parse_baseType();
      return type_ref;
    } else if (token->token_is_typeName()) {
      type_ref->typeRef.type = parse_namedType();
      return type_ref;
    } else if (token->klass == TokenClass::TUPLE) {
      type_ref->typeRef.type = parse_tupleType();
      return type_ref;
    } else assert(0);
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_namedType()
{
  Ast* named_type;

  if (token->token_is_typeName()) {
    named_type = parse_typeName();
    if (token->klass == TokenClass::BRACKET_OPEN) {
      named_type = parse_headerStackType(named_type);
      return named_type;
    }
    return named_type;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeName()
{
  Ast* type_name;

  if (token->klass == TokenClass::TYPE_IDENTIFIER) {
    type_name = (Ast*)storage->malloc(sizeof(Ast));
    type_name->kind = AstEnum::name;
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

Ast* Parser::parse_tupleType()
{
  Ast* tuple;

  if (token->klass == TokenClass::TUPLE) {
    tuple = (Ast*)storage->malloc(sizeof(Ast));
    tuple->kind = AstEnum::tupleType;
    tuple->line_no = token->line_no;
    tuple->column_no = token->column_no;
    next_token();
    if (token->klass == TokenClass::ANGLE_OPEN) {
      next_token();
      tuple->tupleType.type_args = parse_typeArgumentList();
      if (token->klass == TokenClass::ANGLE_CLOSE) {
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

Ast* Parser::parse_headerStackType(Ast* named_type)
{
  Ast* type_ref, *type;

  if (token->klass == TokenClass::BRACKET_OPEN) {
    next_token();
    type_ref = (Ast*)storage->malloc(sizeof(Ast));
    type_ref->kind = AstEnum::typeRef;
    type_ref->line_no = named_type->line_no;
    type_ref->column_no = named_type->column_no;
    type_ref->typeRef.type = named_type;
    type = (Ast*)storage->malloc(sizeof(Ast));
    type->kind = AstEnum::headerStackType;
    type->line_no = named_type->line_no;
    type->column_no = named_type->column_no;
    type->headerStackType.type = type_ref;
    if (token->token_is_expression()) {
      type->headerStackType.stack_expr = parse_expression(1);
      if (token->klass == TokenClass::BRACKET_CLOSE) {
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

Ast* Parser::parse_baseType()
{
  Ast* type_name, *type;

  if (token->token_is_baseType()) {
    type_name = (Ast*)storage->malloc(sizeof(Ast));
    type_name->kind = AstEnum::name;
    type_name->line_no = token->line_no;
    type_name->column_no = token->column_no;
    if (token->klass == TokenClass::BOOL) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeBoolean;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeBoolean.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::INT) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeInteger;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeInteger.name = type_name;
      next_token();
      if (token->klass == TokenClass::ANGLE_OPEN) {
        next_token();
        type->baseTypeInteger.size = parse_integerTypeSize();
        if (token->klass == TokenClass::ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TokenClass::BIT) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeBit;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeBit.name = type_name;
      next_token();
      if (token->klass == TokenClass::ANGLE_OPEN) {
        next_token();
        type->baseTypeBit.size = parse_integerTypeSize();
        if (token->klass == TokenClass::ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TokenClass::VARBIT) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeVarbit;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeVarbit.name = type_name;
      next_token();
      if (token->klass == TokenClass::ANGLE_OPEN) {
        next_token();
        type->baseTypeVarbit.size = parse_integerTypeSize();
        if (token->klass == TokenClass::ANGLE_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: '<' was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return type;
    } else if (token->klass == TokenClass::STRING) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeString;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeString.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::VOID) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeVoid;
      type->line_no = token->line_no;
      type->column_no = token->column_no;
      type_name->name.strname = token->lexeme;
      type->baseTypeVoid.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::ERROR) {
      type = (Ast*)storage->malloc(sizeof(Ast));
      type->kind = AstEnum::baseTypeError;
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

Ast* Parser::parse_integerTypeSize()
{
  Ast* type_size;

  type_size = (Ast*)storage->malloc(sizeof(Ast));
  type_size->kind = AstEnum::integerTypeSize;
  type_size->line_no = token->line_no;
  type_size->column_no = token->column_no;
  if (token->klass == TokenClass::INTEGER_LITERAL) {
    type_size->integerTypeSize.size = parse_integer();
  } else if (token->klass == TokenClass::PARENTH_OPEN) {
#if 0
    type_size->size = parse_expression(1);
#endif
    error("%s:%d:%d: error: integer was expected, got `%s`.",
          source_file, token->line_no, token->column_no, token->lexeme);
  } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  return type_size;
}

Ast* Parser::parse_typeOrVoid()
{
  Ast* type, *name;

  if (token->token_is_typeOrVoid()) {
    if (token->token_is_typeRef()) {
      type = parse_typeRef();
      return type;
    } else if (token->klass == TokenClass::VOID) {
      return parse_baseType();
    } else if (token->klass == TokenClass::IDENTIFIER) {
      name = (Ast*)storage->malloc(sizeof(Ast));
      name->kind = AstEnum::name;
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

Ast* Parser::parse_realTypeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token->token_is_realTypeArg()) {
    type_arg = (Ast*)storage->malloc(sizeof(Ast));
    type_arg->kind = AstEnum::realTypeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TokenClass::DONTCARE) {
      next_token();
      dontcare_arg = (Ast*)storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token->token_is_typeRef()) {
      type_arg->realTypeArg.arg = parse_typeRef();
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeArg()
{
  Ast* type_arg, *dontcare_arg;

  if (token->token_is_typeArg()) {
    type_arg = (Ast*)storage->malloc(sizeof(Ast));
    type_arg->kind = AstEnum::typeArg;
    type_arg->line_no = token->line_no;
    type_arg->column_no = token->column_no;
    if (token->klass == TokenClass::DONTCARE) {
      next_token();
      dontcare_arg = (Ast*)storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
      dontcare_arg->line_no = token->line_no;
      dontcare_arg->column_no = token->column_no;
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token->token_is_typeRef()) {
      type_arg->typeArg.arg = parse_typeRef();
      return type_arg;
    } else if (token->token_is_nonTypeName()) {
      type_arg->typeArg.arg = parse_nonTypeName();
      return type_arg;
    } else assert(0);
  } else error("%s:%d:%d: error: type argument was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_typeArgumentList()
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {};

  args = (Ast*)storage->malloc(sizeof(Ast));
  args->kind = AstEnum::typeArgumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token->token_is_typeArg()) {
    ast = parse_typeArg();
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_typeArg();
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_typeDeclaration()
{
  Ast* type_decl;

  if (token->token_is_typeDeclaration()) {
    type_decl = (Ast*)storage->malloc(sizeof(Ast));
    type_decl->kind = AstEnum::typeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token->token_is_derivedTypeDeclaration()) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::TYPEDEF) {
      type_decl->typeDeclaration.decl = parse_typedefDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::PARSER) {
      type_decl->typeDeclaration.decl = parse_parserTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::CONTROL) {
      type_decl->typeDeclaration.decl = parse_controlTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::PACKAGE) {
      type_decl->typeDeclaration.decl = parse_packageTypeDeclaration();
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_derivedTypeDeclaration()
{
  Ast* type_decl;

  if (token->token_is_derivedTypeDeclaration()) {
    type_decl = (Ast*)storage->malloc(sizeof(Ast));
    type_decl->kind = AstEnum::derivedTypeDeclaration;
    type_decl->line_no = token->line_no;
    type_decl->column_no = token->column_no;
    if (token->klass == TokenClass::HEADER) {
      type_decl->derivedTypeDeclaration.decl = parse_headerTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::UNION) {
      type_decl->derivedTypeDeclaration.decl = parse_headerUnionDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::STRUCT) {
      type_decl->derivedTypeDeclaration.decl = parse_structTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::ENUM) {
      type_decl->derivedTypeDeclaration.decl = parse_enumDeclaration();
      return type_decl;
    } else assert(0);
  } else error("%s:%d:%d: error: structure declaration was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_headerTypeDeclaration()
{
  Ast* header_decl;
  Ast* name;

  if (token->klass == TokenClass::HEADER) {
    next_token();
    header_decl = (Ast*)storage->malloc(sizeof(Ast));
    header_decl->kind = AstEnum::headerTypeDeclaration;
    header_decl->line_no = token->line_no;
    header_decl->column_no = token->column_no;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      header_decl->headerTypeDeclaration.name = name;
      if (token->klass == TokenClass::BRACE_OPEN) {
        next_token();
        header_decl->headerTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BRACE_CLOSE) {
          next_token();
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

Ast* Parser::parse_headerUnionDeclaration()
{
  Ast* union_decl;
  Ast* name;

  if (token->klass == TokenClass::UNION) {
    next_token();
    union_decl = (Ast*)storage->malloc(sizeof(Ast));
    union_decl->kind = AstEnum::headerUnionDeclaration;
    union_decl->line_no = token->line_no;
    union_decl->column_no = token->column_no;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      union_decl->headerUnionDeclaration.name = name;
      if (token->klass == TokenClass::BRACE_OPEN) {
        next_token();
        union_decl->headerUnionDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_structTypeDeclaration()
{
  Ast* struct_decl;
  Ast* name;

  if (token->klass == TokenClass::STRUCT) {
    next_token();
    struct_decl = (Ast*)storage->malloc(sizeof(Ast));
    struct_decl->kind = AstEnum::structTypeDeclaration;
    struct_decl->line_no = token->line_no;
    struct_decl->column_no = token->column_no;
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      struct_decl->structTypeDeclaration.name = name;
      if (token->klass == TokenClass::BRACE_OPEN) {
        next_token();
        struct_decl->structTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_structFieldList()
{
  Ast* fields, *ast;
  AstTreeCtor tree_ctor = {};

  fields = (Ast*)storage->malloc(sizeof(Ast));
  fields->kind = AstEnum::structFieldList;
  fields->line_no = token->line_no;
  fields->column_no = token->column_no;
  if (token->token_is_structField()) {
    ast = parse_structField();
    tree_ctor.append_node(&fields->tree, &ast->tree);
    while (token->token_is_structField()) {
      ast = parse_structField();
      tree_ctor.append_node(&fields->tree, &ast->tree);
    }
  }
  return fields;
}

Ast* Parser::parse_structField()
{
  if (token->token_is_structField()) {
    Ast* field = (Ast*)storage->malloc(sizeof(Ast));
    field->kind = AstEnum::structField;
    field->line_no = token->line_no;
    field->column_no = token->column_no;
    field->structField.type = parse_typeRef();
    if (token->token_is_name()) {
      field->structField.name = parse_name();
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_enumDeclaration()
{
  Ast* enum_decl;
  Ast* name;

  if (token->klass == TokenClass::ENUM) {
    next_token();
    enum_decl = (Ast*)storage->malloc(sizeof(Ast));
    enum_decl->kind = AstEnum::enumDeclaration;
    enum_decl->line_no = token->line_no;
    enum_decl->column_no = token->column_no;
    if (token->klass == TokenClass::BIT) {
      next_token();
      if (token->klass == TokenClass::ANGLE_OPEN) {
        next_token();
        if (token->klass == TokenClass::INTEGER_LITERAL) {
          enum_decl->enumDeclaration.type_size = parse_integer();
          if (token->klass == TokenClass::ANGLE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: an integer was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    if (token->token_is_name()) {
      name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
      enum_decl->enumDeclaration.name = name;
      if (token->klass == TokenClass::BRACE_OPEN) {
        next_token();
        if (token->token_is_specifiedIdentifier()) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList();
          if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_errorDeclaration()
{
  Ast* error_decl;

  if (token->klass == TokenClass::ERROR) {
    next_token();
    error_decl = (Ast*)storage->malloc(sizeof(Ast));
    error_decl->kind = AstEnum::errorDeclaration;
    error_decl->line_no = token->line_no;
    error_decl->column_no = token->column_no;
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      if (token->token_is_name()) {
        if (token->token_is_name()) {
          error_decl->errorDeclaration.fields = parse_identifierList();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_matchKindDeclaration()
{
  Ast* match_decl;

  if (token->klass == TokenClass::MATCH_KIND) {
    next_token();
    match_decl = (Ast*)storage->malloc(sizeof(Ast));
    match_decl->kind = AstEnum::matchKindDeclaration;
    match_decl->line_no = token->line_no;
    match_decl->column_no = token->column_no;
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      if (token->token_is_name()) {
        match_decl->matchKindDeclaration.fields = parse_identifierList();
        if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_identifierList()
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {};

  ids = (Ast*)storage->malloc(sizeof(Ast));
  ids->kind = AstEnum::identifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token->token_is_name()) {
    ast = parse_name();
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_name();
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifierList()
{
  Ast* ids, *ast;
  AstTreeCtor tree_ctor = {};

  ids = (Ast*)storage->malloc(sizeof(Ast));
  ids->kind = AstEnum::specifiedIdentifierList;
  ids->line_no = token->line_no;
  ids->column_no = token->column_no;
  if (token->token_is_specifiedIdentifier()) {
    ast = parse_specifiedIdentifier();
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_specifiedIdentifier();
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifier()
{
  Ast* id;

  if (token->token_is_specifiedIdentifier()) {
    id = (Ast*)storage->malloc(sizeof(Ast));
    id->kind = AstEnum::specifiedIdentifier;
    id->line_no = token->line_no;
    id->column_no = token->column_no;
    id->specifiedIdentifier.name = parse_name();
    if (token->klass == TokenClass::EQUAL) {
      next_token();
      if (token->token_is_expression()) {
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

Ast* Parser::parse_typedefDeclaration()
{
  Ast* type_decl;
  Ast* name;

  if (token->klass == TokenClass::TYPEDEF) {
    next_token();
    if (token->token_is_typeRef() || token->token_is_derivedTypeDeclaration()) {
      type_decl = (Ast*)storage->malloc(sizeof(Ast));
      type_decl->kind = AstEnum::typedefDeclaration;
      type_decl->line_no = token->line_no;
      type_decl->column_no = token->column_no;
      if (token->token_is_typeRef()) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef();
      } else if (token->token_is_derivedTypeDeclaration()) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token->token_is_name()) {
        name = parse_name();
        current_scope->bind_name(storage, name->name.strname, NameSpace::TYPE);
        type_decl->typedefDeclaration.name = name;
        if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_assignmentOrMethodCallStatement()
{
  Ast* lvalue, *stmt; 

  if (token->token_is_lvalue()) {
    lvalue = parse_lvalue();
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      stmt = (Ast*)storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::functionCall;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      stmt->functionCall.lhs_expr = lvalue;
      stmt->functionCall.args = parse_argumentList();
      if (token->klass == TokenClass::PARENTH_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return stmt;
    } else if (token->klass == TokenClass::EQUAL) {
      next_token();
      stmt = (Ast*)storage->malloc(sizeof(Ast));
      stmt->kind = AstEnum::assignmentStatement;
      stmt->line_no = token->line_no;
      stmt->column_no = token->column_no;
      stmt->assignmentStatement.lhs_expr = lvalue;
      stmt->assignmentStatement.rhs_expr = parse_expression(1);
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_returnStatement()
{
  Ast* return_stmt;

  if (token->klass == TokenClass::RETURN) {
    next_token();
    return_stmt = (Ast*)storage->malloc(sizeof(Ast));
    return_stmt->kind = AstEnum::returnStatement;
    return_stmt->line_no = token->line_no;
    return_stmt->column_no = token->column_no;
    if (token->token_is_expression())
      return_stmt->returnStatement.expr = parse_expression(1);
    if (token->klass == TokenClass::SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return return_stmt;
  } else error("%s:%d:%d: error: `return` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_exitStatement()
{
  Ast* exit_stmt;

  if (token->klass == TokenClass::EXIT) {
    next_token();
    exit_stmt = (Ast*)storage->malloc(sizeof(Ast));
    exit_stmt->kind = AstEnum::exitStatement;
    exit_stmt->line_no = token->line_no;
    exit_stmt->column_no = token->column_no;
    if (token->klass == TokenClass::SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return exit_stmt;
  } else error("%s:%d:%d: error: `exit` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_conditionalStatement()
{
  Ast* if_stmt;

  if (token->klass == TokenClass::IF) {
    next_token();
    if_stmt = (Ast*)storage->malloc(sizeof(Ast));
    if_stmt->kind = AstEnum::conditionalStatement;
    if_stmt->line_no = token->line_no;
    if_stmt->column_no = token->column_no;
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      if (token->token_is_expression()) {
        if_stmt->conditionalStatement.cond_expr = parse_expression(1);
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
          if (token->token_is_statement()) {
            if_stmt->conditionalStatement.stmt = parse_statement(0);
            if (token->klass == TokenClass::ELSE) {
              next_token();
              if (token->token_is_statement()) {
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

Ast* Parser::parse_directApplication(Ast* type_name)
{
  Ast* apply_stmt;

  if (token->token_is_typeName() || type_name) {
    apply_stmt = (Ast*)storage->malloc(sizeof(Ast));
    apply_stmt->kind = AstEnum::directApplication;
    apply_stmt->line_no = token->line_no;
    apply_stmt->column_no = token->column_no;
    apply_stmt->directApplication.name = type_name ? type_name : parse_typeName();
    if (token->klass == TokenClass::DOT) {
      next_token();
      if (token->klass == TokenClass::APPLY) {
        next_token();
        if (token->klass == TokenClass::PARENTH_OPEN) {
          next_token();
          apply_stmt->directApplication.args = parse_argumentList();
          if (token->klass == TokenClass::PARENTH_CLOSE) {
            next_token();
            if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_statement(Ast* type_name)
{
  Ast* stmt, *empty_stmt;

  if (token->token_is_statement()) {
    stmt = (Ast*)storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::statement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token->token_is_typeName() || type_name) {
      stmt->statement.stmt = parse_directApplication(type_name);
      return stmt;
    } else if (token->token_is_assignmentOrMethodCallStatement()) {
      stmt->statement.stmt = parse_assignmentOrMethodCallStatement();
      return stmt;
    } else if (token->klass == TokenClass::IF) {
      stmt->statement.stmt = parse_conditionalStatement();
      return stmt;
    } else if (token->klass == TokenClass::SEMICOLON) {
      empty_stmt = (Ast*)storage->malloc(sizeof(Ast));
      empty_stmt->kind = AstEnum::emptyStatement;
      empty_stmt->line_no = token->line_no;
      empty_stmt->column_no = token->column_no;
      stmt->statement.stmt = empty_stmt;
      next_token();
      return stmt;
    } else if (token->klass == TokenClass::BRACE_OPEN) {
      stmt->statement.stmt = parse_blockStatement();
      return stmt;
    } else if (token->klass == TokenClass::EXIT) {
      stmt->statement.stmt = parse_exitStatement();
      return stmt;
    } else if (token->klass == TokenClass::RETURN) {
      stmt->statement.stmt = parse_returnStatement();
      return stmt;
    } else if (token->klass == TokenClass::SWITCH) {
      stmt->statement.stmt = parse_switchStatement();
      return stmt;
    }
  } else error("%s:%d:%d: error: statement was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_blockStatement()
{
  Ast* block_stmt;

  if (token->klass == TokenClass::BRACE_OPEN) {
    next_token();
    block_stmt = (Ast*)storage->malloc(sizeof(Ast));
    block_stmt->kind = AstEnum::blockStatement;
    block_stmt->line_no = token->line_no;
    block_stmt->column_no = token->column_no;
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList();
    if (token->klass == TokenClass::BRACE_CLOSE) {
      next_token();
    } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return block_stmt;
  } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_statementOrDeclList()
{
  Ast* stmts, *ast;
  AstTreeCtor tree_ctor = {};

  stmts = (Ast*)storage->malloc(sizeof(Ast));
  stmts->kind = AstEnum::statementOrDeclList;
  stmts->line_no = token->line_no;
  stmts->column_no = token->column_no;
  if (token->token_is_statementOrDeclaration()) {
    ast = parse_statementOrDeclaration();
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token->token_is_statementOrDeclaration()) {
      ast = parse_statementOrDeclaration();
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_switchStatement()
{
  Ast* stmt;

  if (token->klass == TokenClass::SWITCH) {
    next_token();
    stmt = (Ast*)storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::switchStatement;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      stmt->switchStatement.expr = parse_expression(1);
      if (token->klass == TokenClass::PARENTH_CLOSE) {
        next_token();
        if (token->klass == TokenClass::BRACE_OPEN) {
          next_token();
          stmt->switchStatement.switch_cases = parse_switchCases();
          if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_switchCases()
{
  Ast* cases, *ast;
  AstTreeCtor tree_ctor = {};

  cases = (Ast*)storage->malloc(sizeof(Ast));
  cases->kind = AstEnum::switchCases;
  cases->line_no = token->line_no;
  cases->column_no = token->column_no;
  if (token->token_is_switchLabel()) {
    ast = parse_switchCase();
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token->token_is_switchLabel()) {
      ast = parse_switchCase();
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_switchCase()
{
  Ast* switch_case;

  if (token->token_is_switchLabel()) {
    switch_case = (Ast*)storage->malloc(sizeof(Ast));
    switch_case->kind = AstEnum::switchCase;
    switch_case->line_no = token->line_no;
    switch_case->column_no = token->column_no;
    switch_case->switchCase.label = parse_switchLabel();
    if (token->klass == TokenClass::COLON) {
      next_token();
      if (token->klass == TokenClass::BRACE_OPEN) {
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

Ast* Parser::parse_switchLabel()
{
  Ast* switch_label, *default_label;

  if (token->token_is_switchLabel()) {
    switch_label = (Ast*)storage->malloc(sizeof(Ast));
    switch_label->kind = AstEnum::switchLabel;
    switch_label->line_no = token->line_no;
    switch_label->column_no = token->column_no;
    if (token->token_is_name()) {
      switch_label->switchLabel.label = parse_name();
      return switch_label;
    } else if (token->klass == TokenClass::DEFAULT) {
      next_token();
      default_label = (Ast*)storage->malloc(sizeof(Ast));
      default_label->kind = AstEnum::default_;
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

Ast* Parser::parse_statementOrDeclaration()
{
  Ast* stmt, *type_ref;

  if (token->token_is_statementOrDeclaration()) {
    stmt = (Ast*)storage->malloc(sizeof(Ast));
    stmt->kind = AstEnum::statementOrDeclaration;
    stmt->line_no = token->line_no;
    stmt->column_no = token->column_no;
    if (token->token_is_typeRef()) {
      type_ref = parse_typeRef();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        stmt->statementOrDeclaration.stmt = parse_instantiation(type_ref);
        return stmt;
      } else if (token->token_is_name()) {
        stmt->statementOrDeclaration.stmt = parse_variableDeclaration(type_ref);
        return stmt;
      } else {
        stmt->statementOrDeclaration.stmt = parse_statement(type_ref);
        return stmt;
      }
    } else if (token->token_is_statement()) {
      stmt->statementOrDeclaration.stmt = parse_statement(0);
      return stmt;
    } else if (token->klass == TokenClass::CONST) {
      stmt->statementOrDeclaration.stmt = parse_variableDeclaration(0);
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

  if (token->klass == TokenClass::TABLE) {
    next_token();
    table = (Ast*)storage->malloc(sizeof(Ast));
    table->kind = AstEnum::tableDeclaration;
    table->line_no = token->line_no;
    table->column_no = token->column_no;
    table->tableDeclaration.name = parse_name();
    method_protos = (Ast*)storage->malloc(sizeof(Ast));
    method_protos->kind = AstEnum::methodPrototypes;
    method_protos->line_no = table->line_no;
    method_protos->column_no = table->column_no;
    table->tableDeclaration.method_protos = method_protos;
    if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      if (token->token_is_tableProperty()) {
        table->tableDeclaration.prop_list = parse_tablePropertyList();
      } else error("%s:%d:%d: error: table property was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::BRACE_CLOSE) {
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

Ast* Parser::parse_tablePropertyList()
{
  Ast* props, *ast;
  AstTreeCtor tree_ctor = {};

  props = (Ast*)storage->malloc(sizeof(Ast));
  props->kind = AstEnum::tablePropertyList;
  props->line_no = token->line_no;
  props->column_no = token->column_no;
  if (token->token_is_tableProperty()) {
    ast = parse_tableProperty();
    tree_ctor.append_node(&props->tree, &ast->tree);
    while (token->token_is_tableProperty()) {
      ast = parse_tableProperty();
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

  if (token->token_is_tableProperty()) {
#if 0
    if (token->klass == TokenClass::CONST) {
      next_token();
      is_const = 1;
    }
#endif
    table_prop = (Ast*)storage->malloc(sizeof(Ast));
    table_prop->kind = AstEnum::tableProperty;
    table_prop->line_no = token->line_no;
    table_prop->column_no = token->column_no;
    if (token->klass == TokenClass::KEY) {
      next_token();
      prop = (Ast*)storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::keyProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        if (token->klass == TokenClass::BRACE_OPEN) {
          next_token();
          prop->keyProperty.keyelem_list = parse_keyElementList();
          if (token->klass == TokenClass::BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (token->klass == TokenClass::ACTIONS) {
      next_token();
      prop = (Ast*)storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::actionsProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        if (token->klass == TokenClass::BRACE_OPEN) {
          next_token();
          prop->actionsProperty.action_list = parse_actionList();
          if (token->klass == TokenClass::BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
#if 0
    else if (token->klass == TokenClass::ENTRIES) {
      next_token();
      prop = (Ast*)storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::entriesProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        if (token->klass == TokenClass::BRACE_OPEN) {
          next_token();
          if (token_is_keysetExpression(token)) {
            prop->entriesProperty.entries_list = parse_entriesList();
          } else error("%s:%d:%d: error: keyset expression was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
          if (token->klass == TokenClass::BRACE_CLOSE) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    }
    else if (token_is_nonTableKwName(token)) {
      prop = (Ast*)storage->malloc(sizeof(Ast));
      prop->kind = AstEnum::simpleProperty;
      prop->line_no = token->line_no;
      prop->column_no = token->column_no;
      prop->simpleProperty.is_const = is_const;
      prop->simpleProperty.name = parse_name();
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        prop->simpleProperty.init_expr = parse_expression(1);
        if (token->klass == TokenClass::SEMICOLON) {
          next_token();
        } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else assert(0);
#endif
    else error("%s:%d:%d: error: table property was expected, got `%s`.",
                source_file, token->line_no, token->column_no, token->lexeme);
  }
  else error("%s:%d:%d: error: table property was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_keyElementList()
{
  Ast* elems, *ast;
  AstTreeCtor tree_ctor = {};

  elems = (Ast*)storage->malloc(sizeof(Ast));
  elems->kind = AstEnum::keyElementList;
  elems->line_no = token->line_no;
  elems->column_no = token->column_no;
  if (token->token_is_expression()) {
    ast = parse_keyElement();
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token->token_is_expression()) {
      ast = parse_keyElement();
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_keyElement()
{
  Ast* key_elem;

  if (token->token_is_expression()) {
    key_elem = (Ast*)storage->malloc(sizeof(Ast));
    key_elem->kind = AstEnum::keyElement;
    key_elem->line_no = token->line_no;
    key_elem->column_no = token->column_no;
    key_elem->keyElement.expr = parse_expression(1);
    if (token->klass == TokenClass::COLON) {
      next_token();
      key_elem->keyElement.match = parse_name();
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_actionList()
{
  Ast* actions, *ast;
  AstTreeCtor tree_ctor = {};

  actions = (Ast*)storage->malloc(sizeof(Ast));
  actions->kind = AstEnum::actionList;
  actions->line_no = token->line_no;
  actions->column_no = token->column_no;
  if (token->token_is_actionRef()) {
    ast = parse_actionRef();
    tree_ctor.append_node(&actions->tree, &ast->tree);
    if (token->klass == TokenClass::SEMICOLON) {
      next_token();
    } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    while (token->token_is_actionRef()) {
      ast = parse_actionRef();
      tree_ctor.append_node(&actions->tree, &ast->tree);
      if (token->klass == TokenClass::SEMICOLON) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
  }
  return actions;
}

Ast* Parser::parse_actionRef()
{
  Ast* action_ref;

  if (token->token_is_nonTypeName()) {
    action_ref = (Ast*)storage->malloc(sizeof(Ast));
    action_ref->kind = AstEnum::actionRef;
    action_ref->line_no = token->line_no;
    action_ref->column_no = token->column_no;
    action_ref->actionRef.name = parse_nonTypeName();
    if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      if (token->token_is_argument()) {
        action_ref->actionRef.args = parse_argumentList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TokenClass::PARENTH_CLOSE) {
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

#if 0
Ast* Parser::parse_entriesList()
{
  Ast* entries, *ast;
  AstTreeCtor tree_ctor = {0};

  entries = (Ast*)storage->malloc(sizeof(Ast));
  entries->kind = AstEnum::entriesList;
  entries->line_no = token->line_no;
  entries->column_no = token->column_no;
  if (token_is_keysetExpression(token)) {
    ast = parse_entry();
    tree_ctor.append_node(&entries->tree, &ast->tree);
    while (token_is_keysetExpression(token)) {
      ast = parse_entry();
      tree_ctor.append_node(&entries->tree, &ast->tree);
    }
  }
  return entries;
}

Ast* Parser::parse_entry()
{
  Ast* entry;

  if (token_is_keysetExpression(token)) {
    entry = (Ast*)storage->malloc(sizeof(Ast));
    entry->kind = AstEnum::entry;
    entry->line_no = token->line_no;
    entry->column_no = token->column_no;
    entry->entry.keyset = parse_keysetExpression();
    if (token->klass == TokenClass::COLON) {
      next_token();
      entry->entry.action = parse_actionRef();
      if (token->klass == TokenClass::SEMICOLON) {
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
#endif

Ast* Parser::parse_actionDeclaration()
{
  Ast* action_decl;

  if (token->klass == TokenClass::ACTION) {
    next_token();
    action_decl = (Ast*)storage->malloc(sizeof(Ast));
    action_decl->kind = AstEnum::actionDeclaration;
    action_decl->line_no = token->line_no;
    action_decl->column_no = token->column_no;
    if (token->token_is_name()) {
      action_decl->actionDeclaration.name = parse_name();
      if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        action_decl->actionDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
          if (token->klass == TokenClass::BRACE_OPEN) {
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

Ast* Parser::parse_variableDeclaration(Ast* type_ref)
{
  bool is_const = 0;
  Ast* var_decl;

  if (token->klass == TokenClass::CONST) {
    next_token();
    is_const = 1;
  }
  if (token->token_is_typeRef() || type_ref) {
    var_decl = (Ast*)storage->malloc(sizeof(Ast));
    var_decl->kind = AstEnum::variableDeclaration;
    var_decl->line_no = token->line_no;
    var_decl->column_no = token->column_no;
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef();
    if (token->token_is_name()) {
      var_decl->variableDeclaration.name = parse_name();
      if (token->klass == TokenClass::EQUAL) {
        next_token();
        var_decl->variableDeclaration.init_expr = parse_expression(1);
      }
      if (token->klass == TokenClass::SEMICOLON) {
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

Ast* Parser::parse_functionDeclaration(Ast* type_ref)
{
  Ast* func_decl;

  if (token->token_is_typeOrVoid()) {
    func_decl = (Ast*)storage->malloc(sizeof(Ast));
    func_decl->kind = AstEnum::functionDeclaration;
    func_decl->line_no = token->line_no;
    func_decl->column_no = token->column_no;
    func_decl->functionDeclaration.proto = parse_functionPrototype(type_ref);
    if (token->klass == TokenClass::BRACE_OPEN) {
      func_decl->functionDeclaration.stmt = parse_blockStatement();
    } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    return func_decl;
  } else error("%s:%d:%d: error: type was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_argumentList()
{
  Ast* args, *ast;
  AstTreeCtor tree_ctor = {0};

  args = (Ast*)storage->malloc(sizeof(Ast));
  args->kind = AstEnum::argumentList;
  args->line_no = token->line_no;
  args->column_no = token->column_no;
  if (token->token_is_argument()) {
    ast = parse_argument();
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_argument();
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_argument()
{
  Ast* arg, *dontcare_arg;

  if (token->token_is_argument()) {
    arg = (Ast*)storage->malloc(sizeof(Ast));
    arg->kind = AstEnum::argument;
    arg->line_no = token->line_no;
    arg->column_no = token->column_no;
    if (token->token_is_expression()) {
      arg->argument.arg = parse_expression(1);
      return arg;
    } else if (token->klass == TokenClass::DONTCARE) {
      next_token();
      dontcare_arg = (Ast*)storage->malloc(sizeof(Ast));
      dontcare_arg->kind = AstEnum::dontcare;
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

Ast* Parser::parse_expressionList()
{
  Ast* exprs, *ast;
  AstTreeCtor tree_ctor = {0};
  
  exprs = (Ast*)storage->malloc(sizeof(Ast));
  exprs->kind = AstEnum::expressionList;
  exprs->line_no = token->line_no;
  exprs->column_no = token->column_no;
  if (token->token_is_expression()) {
    ast = parse_expression(1);
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (token->klass == TokenClass::COMMA) {
      next_token();
      ast = parse_expression(1);
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_lvalue()
{
  Ast* lvalue, *expr;

  if (token->token_is_lvalue()) {
    lvalue = (Ast*)storage->malloc(sizeof(Ast));
    lvalue->kind = AstEnum::lvalueExpression;
    lvalue->line_no = token->line_no;
    lvalue->column_no = token->column_no;
    lvalue->lvalueExpression.expr = parse_nonTypeName();
    while(token->klass == TokenClass::DOT || token->klass == TokenClass::BRACKET_OPEN) {
      if (token->klass == TokenClass::DOT) {
        next_token();
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::memberSelector;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->memberSelector.lhs_expr = lvalue;
        if (token->token_is_name()) {
          expr->memberSelector.name = parse_name();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = (Ast*)storage->malloc(sizeof(Ast));
        lvalue->kind = AstEnum::lvalueExpression;
        lvalue->line_no = token->line_no;
        lvalue->column_no = token->column_no;
        lvalue->lvalueExpression.expr = expr;
      }
      else if (token->klass == TokenClass::BRACKET_OPEN) {
        next_token();
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::arraySubscript;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->arraySubscript.lhs_expr = lvalue;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TokenClass::BRACKET_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = (Ast*)storage->malloc(sizeof(Ast));
        lvalue->kind = AstEnum::lvalueExpression;
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

Ast* Parser::parse_expression(int priority_threshold)
{
  Ast* primary, *expr;

  if (token->token_is_expression()) {
    primary = parse_expressionPrimary();
    while (token->token_is_exprOperator()) {
      if (token->klass == TokenClass::DOT) {
        next_token();
        Ast* expr;
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::memberSelector;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->memberSelector.lhs_expr = primary;
        if (token->token_is_nonTypeName()) {
          expr->memberSelector.name = parse_nonTypeName();
        } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = (Ast*)storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::BRACKET_OPEN) {
        next_token();
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::arraySubscript;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->arraySubscript.lhs_expr = primary;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TokenClass::BRACKET_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = (Ast*)storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::PARENTH_OPEN) {
        next_token();
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::functionCall;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->functionCall.lhs_expr = primary;
        expr->functionCall.args = parse_argumentList();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = (Ast*)storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::EQUAL) {
        next_token();
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::assignmentStatement;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->assignmentStatement.lhs_expr = primary;
        expr->assignmentStatement.rhs_expr = parse_expression(1);
        primary = (Ast*)storage->malloc(sizeof(Ast));
        primary->kind = AstEnum::expression;
        primary->line_no = expr->line_no;
        primary->column_no = expr->column_no;
        primary->expression.expr = expr;
      } else if (token->token_is_binaryOperator()){
        int priority = operator_priority(token);
        if (priority >= priority_threshold) {
          expr = (Ast*)storage->malloc(sizeof(Ast));
          expr->kind = AstEnum::binaryExpression;
          expr->line_no = token->line_no;
          expr->column_no = token->column_no;
          expr->binaryExpression.left_operand = primary;
          expr->binaryExpression.op = token_to_binop(token);
          expr->binaryExpression.strname = token->lexeme;
          next_token();
          expr->binaryExpression.right_operand = parse_expression(priority + 1);
          primary = (Ast*)storage->malloc(sizeof(Ast));
          primary->kind = AstEnum::expression;
          primary->line_no = expr->line_no;
          primary->column_no = expr->column_no;
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

Ast* Parser::parse_expressionPrimary()
{
  Ast* primary, *expr;

  if (token->token_is_expression()) {
    primary = (Ast*)storage->malloc(sizeof(Ast));
    primary->kind = AstEnum::expression;
    primary->line_no = token->line_no;
    primary->column_no = token->column_no;
    if (token->klass == TokenClass::INTEGER_LITERAL) {
      primary->expression.expr = parse_integer();
      return primary;
    } else if (token->klass == TokenClass::TRUE || token->klass == TokenClass::FALSE) {
      primary->expression.expr = parse_boolean();
      return primary;
    } else if (token->klass == TokenClass::STRING_LITERAL) {
      primary->expression.expr = parse_string();
      return primary;
    } else if (token->klass == TokenClass::DOT) {
      next_token();
      if (token->klass == TokenClass::IDENTIFIER) {
        primary->expression.expr = parse_nonTypeName();
        return primary;
      } else if (token->klass == TokenClass::TYPE_IDENTIFIER) {
        primary->expression.expr = parse_typeName();
        return primary;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->token_is_nonTypeName()) {
      primary->expression.expr = parse_nonTypeName();
      return primary;
    } else if (token->klass == TokenClass::BRACE_OPEN) {
      next_token();
      primary->expression.expr = parse_expressionList();
      if (token->klass == TokenClass::BRACE_CLOSE) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return primary;
    } else if (token->klass == TokenClass::PARENTH_OPEN) {
      next_token();
      if (token->klass == TokenClass::TYPE_IDENTIFIER && peek_token()->klass == TokenClass::DOT) {
        /* (<typeName>.<name>) */
        primary->expression.expr = parse_expression(1);
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else if (token->token_is_typeRef()) {
        expr = (Ast*)storage->malloc(sizeof(Ast));
        expr->kind = AstEnum::castExpression;
        expr->line_no = token->line_no;
        expr->column_no = token->column_no;
        expr->castExpression.type = parse_typeRef();
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
          expr->castExpression.expr = parse_expression(10);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary->expression.expr = expr;
        return primary;
      } else if (token->token_is_expression()) {
        primary->expression.expr = parse_expression(1);
        if (token->klass == TokenClass::PARENTH_CLOSE) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->klass == TokenClass::EXCLAMATION) {
      next_token();
      expr = (Ast*)storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = AstOperator::NOT;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TokenClass::TILDA) {
      next_token();
      expr = (Ast*)storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = AstOperator::BITW_NOT;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TokenClass::UNARY_MINUS) {
      next_token();
      expr = (Ast*)storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::unaryExpression;
      expr->line_no = token->line_no;
      expr->column_no = token->column_no;
      expr->unaryExpression.op = AstOperator::NEG;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->token_is_typeName()) {
      primary->expression.expr = parse_typeName();
      return primary;
    } else if (token->klass == TokenClass::ERROR) {
      next_token();
      expr = (Ast*)storage->malloc(sizeof(Ast));
      expr->kind = AstEnum::name;
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

Ast* Parser::parse_indexExpression()
{
  Ast* index_expr;

  if (token->token_is_expression()) {
    index_expr = (Ast*)storage->malloc(sizeof(Ast));
    index_expr->kind = AstEnum::indexExpression;
    index_expr->line_no = token->line_no;
    index_expr->column_no = token->column_no;
    index_expr->indexExpression.start_index = parse_expression(1);
    if (token->klass == TokenClass::COLON) {
      next_token();
      if (token->token_is_expression()) {
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

Ast* Parser::parse_integer()
{
  Ast* int_literal;

  if (token->klass == TokenClass::INTEGER_LITERAL) {
    int_literal = (Ast*)storage->malloc(sizeof(Ast));
    int_literal->kind = AstEnum::integerLiteral;
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

Ast* Parser::parse_boolean()
{
  Ast* bool_literal;

  if (token->klass == TokenClass::TRUE || token->klass == TokenClass::FALSE) {
    bool_literal = (Ast*)storage->malloc(sizeof(Ast));
    bool_literal->kind = AstEnum::booleanLiteral;
    bool_literal->line_no = token->line_no;
    bool_literal->column_no = token->column_no;
    bool_literal->booleanLiteral.value = (token->klass == TokenClass::TRUE);
    next_token();
    return bool_literal;
  } else error("%s:%d:%d: error: boolean was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_string()
{
  Ast* string_literal;

  if (token->klass == TokenClass::STRING_LITERAL) {
    string_literal = (Ast*)storage->malloc(sizeof(Ast));
    string_literal->kind = AstEnum::stringLiteral;
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
