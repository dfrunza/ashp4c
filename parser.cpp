#include "basic.h"
#include "parser.h"

struct Keyword {
  char* strname;
  enum TokenClass token_class;
};

void Parser::define_keywords(Scope* scope)
{
  struct Keyword keywords[] = {
    {"action",  TokenClass::Action},
    {"actions", TokenClass::Actions},
    {"entries", TokenClass::Entries},
    {"enum",    TokenClass::Enum},
    {"in",      TokenClass::In},
    {"package", TokenClass::Package},
    {"select",  TokenClass::Select},
    {"switch",  TokenClass::Switch},
    {"tuple",   TokenClass::Tuple},
    {"control", TokenClass::Control},
    {"error",   TokenClass::Error},
    {"header",  TokenClass::Header},
    {"inout",   TokenClass::InOut},
    {"parser",  TokenClass::Parser},
    {"state",   TokenClass::State},
    {"table",   TokenClass::Table},
    {"key",     TokenClass::Key},
    {"typedef", TokenClass::Typedef},
    {"default", TokenClass::Default},
    {"extern",  TokenClass::Extern},
    {"out",     TokenClass::Out},
    {"else",    TokenClass::Else},
    {"exit",    TokenClass::Exit},
    {"if",      TokenClass::If},
    {"return",  TokenClass::Return},
    {"struct",  TokenClass::Struct},
    {"apply",   TokenClass::Apply},
    {"const",   TokenClass::Const},
    {"bool",    TokenClass::Bool},
    {"true",    TokenClass::True},
    {"false",   TokenClass::False},
    {"void",    TokenClass::Void},
    {"int",     TokenClass::Int},
    {"bit",     TokenClass::Bit},
    {"varbit",  TokenClass::Varbit},
    {"string",  TokenClass::String},
    {"match_kind",   TokenClass::MatchKind},
    {"transition",   TokenClass::Transition},
    {"header_union", TokenClass::Union},
  };

  for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
    NameDeclaration* name_decl = scope->bind_name(storage, keywords[i].strname, NameSpace::Keyword);
    name_decl->token_class = keywords[i].token_class;
  }
}

Token* Parser::next_token()
{
  assert(token_at < tokens->element_count);

  prev_token = token;
  prev_token_at = token_at;
  token = (Token*)tokens->get(++token_at);
  while (token->klass == TokenClass::Comment) {
    token = (Token*)tokens->get(++token_at);
  }
  if (token->klass == TokenClass::Identifier) {
    NameEntry* name_entry = current_scope->lookup(token->lexeme, NameSpace::Keyword | NameSpace::Type);
    NameDeclaration* name_decl = name_entry->get_declarations(NameSpace::Keyword);
    if (name_decl) {
      token->klass = name_decl->token_class;
      return token;
    }
    name_decl = name_entry->get_declarations(NameSpace::Type);
    if (name_decl) {
      token->klass = TokenClass::TypeIdentifier;
      return token;
    }
  }
  return token;
}

Token* Parser::peek_token()
{
  prev_token = token;
  prev_token_at = token_at;
  Token* peek_token = next_token();
  token = prev_token;
  token_at = prev_token_at;
  return peek_token;
}

static int operator_priority(Token* token)
{
  if (token->klass == TokenClass::DoubleAmpersand || token->klass == TokenClass::DoublePipe) {
    /* Logical AND, OR */
    return 1;
  } else if (token->klass == TokenClass::DoubleEqual || token->klass == TokenClass::ExclamationEqual
             || token->klass == TokenClass::AngleOpen /* < */ || token->klass == TokenClass::AngleClose /* > */
      || token->klass == TokenClass::AngleOpenEqual /* <= */ || token->klass == TokenClass::AngleCloseEqual /* >= */) {
    /* Relational ops  */
    return 2;
  }
  else if (token->klass == TokenClass::Plus || token->klass == TokenClass::Minus
           || token->klass == TokenClass::Ampersand || token->klass == TokenClass::Pipe
           || token->klass == TokenClass::Circumflex || token->klass == TokenClass::DoubleAngleOpen /* << */
           || token->klass == TokenClass::DoubleAngleClose /* >> */) {
    /* Addition and subtraction; bitwise ops */
    return 3;
  }
  else if (token->klass == TokenClass::Star || token->klass == TokenClass::Slash) {
    /* Multiplication and division */
    return 4;
  }
  else if (token->klass == TokenClass::TripleAmpersand) {
    /* Mask */
    return 5;
  }
  else assert(0);
  return 0;
}

enum AstOperator token_to_binop(Token* token)
{
  switch (token->klass) {
    case TokenClass::DoubleAmpersand:
      return AstOperator::And;
    case TokenClass::DoublePipe:
      return AstOperator::Or;
    case TokenClass::DoubleEqual:
      return AstOperator::Eq;
    case TokenClass::ExclamationEqual:
      return AstOperator::Neq;
    case TokenClass::AngleOpen:
      return AstOperator::Less;
    case TokenClass::AngleClose:
      return AstOperator::Great;
    case TokenClass::AngleOpenEqual:
      return AstOperator::LessEq;
    case TokenClass::AngleCloseEqual:
      return AstOperator::GreatEq;
    case TokenClass::Plus:
      return AstOperator::Add;
    case TokenClass::Minus:
      return AstOperator::Sub;
    case TokenClass::Star:
      return AstOperator::Mul;
    case TokenClass::Slash:
      return AstOperator::Div;
    case TokenClass::Ampersand:
      return AstOperator::BitwAnd;
    case TokenClass::Pipe:
      return AstOperator::BitwOr;
    case TokenClass::Circumflex:
      return AstOperator::BitwXor;
    case TokenClass::DoubleAngleOpen:
      return AstOperator::BitwShl;
    case TokenClass::DoubleAngleClose:
      return AstOperator::BitwShr;
    case TokenClass::TripleAmpersand:
      return AstOperator::Mask;
    default: return (AstOperator)0;
  }
}

void Parser::parse()
{
  root_scope = Scope::allocate(storage, 5);
  root_scope->init();
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
  Ast* p4program = Ast::allocate(storage);
  p4program->init(AstEnum::p4program, token->line_no, token->column_no);
  while (token->klass == TokenClass::Semicolon) {
    next_token(); /* empty declaration */
  }
  Scope* scope = Scope::allocate(storage, 6);
  scope->init();
  current_scope = scope->push(current_scope);
  p4program->p4program.decl_list = parse_declarationList();
  current_scope = current_scope->pop();
  if (token->klass != TokenClass::EndOfInput) {
    error("%s:%d:%d: error: unexpected token `%s`.",
          source_file, token->line_no, token->column_no, token->lexeme);
  }
  return p4program;
}

Ast* Parser::parse_declarationList()
{
  Ast* decls = Ast::allocate(storage);
  decls->init(AstEnum::declarationList, token->line_no, token->column_no);
  if (token->is_declaration()) {
    Ast* ast = parse_declaration();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token->is_declaration() || token->klass == TokenClass::Semicolon) {
      if (token->is_declaration()) {
        ast = parse_declaration();
        tree_ctor.append_node(&decls->tree, &ast->tree);
      } else if (token->klass == TokenClass::Semicolon) {
        next_token(); /* empty declaration */
      }
    }
  }
  return decls;
}

Ast* Parser::parse_declaration()
{
  if (token->is_declaration()) {
    Ast* decl = Ast::allocate(storage);
    decl->init(AstEnum::declaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::Const) {
      decl->declaration.decl = parse_variableDeclaration(0);
      return decl;
    } else if (token->klass == TokenClass::Extern) {
      decl->declaration.decl = parse_externDeclaration();
      return decl;
    } else if (token->klass == TokenClass::Action) {
      decl->declaration.decl = parse_actionDeclaration();
      return decl;
    } else if (token->klass == TokenClass::Parser) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else {
        decl->declaration.decl = parse_parserDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token->klass == TokenClass::Control) {
      decl->declaration.decl = parse_typeDeclaration();
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else {
        decl->declaration.decl = parse_controlDeclaration(decl->declaration.decl);
      }
      return decl;
    } else if (token->is_typeDeclaration()) {
      decl->declaration.decl = parse_typeDeclaration();
      return decl;
    } else if (token->klass == TokenClass::Error) {
      decl->declaration.decl = parse_errorDeclaration();
      return decl;
    } else if (token->klass == TokenClass::MatchKind) {
      decl->declaration.decl = parse_matchKindDeclaration();
      return decl;
    } else if (token->is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TokenClass::ParenthOpen) {
        decl->declaration.decl = parse_instantiation(type_ref);
        return decl;
      } else if (token->is_name()) {
        decl->declaration.decl = parse_functionDeclaration(type_ref);
        return decl;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->is_typeOrVoid()) {
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
  if (token->is_nonTypeName()) {
    Ast* name = Ast::allocate(storage);
    name->init(AstEnum::name, token->line_no, token->column_no);
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
  if (token->is_name()) {
    if (token->is_nonTypeName()) {
      return parse_nonTypeName();
    } else if (token->klass == TokenClass::TypeIdentifier) {
      Ast* type_name = Ast::allocate(storage);
      type_name->init(AstEnum::name, token->line_no, token->column_no);
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
  Ast* params = Ast::allocate(storage);
  params->init(AstEnum::parameterList, token->line_no, token->column_no);
  if (token->is_parameter()) {
    Ast* ast = parse_parameter();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&params->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_parameter();
      tree_ctor.append_node(&params->tree, &ast->tree);
    }
  }
  return params;
}

Ast* Parser::parse_parameter()
{
  if (token->is_parameter()) {
    Ast* param = Ast::allocate(storage);
    param->init(AstEnum::parameter, token->line_no, token->column_no);
    param->parameter.direction = parse_direction();
    param->parameter.type = parse_typeRef();
    if (token->is_name()) {
      param->parameter.name = parse_name();
      if (token->klass == TokenClass::Equal) {
        next_token();
        if (token->is_expression()) {
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
  if (token->is_direction()) {
    if (token->klass == TokenClass::In) {
      next_token();
      return ParamDirection::In;
    } else if (token->klass == TokenClass::Out) {
      next_token();
      return ParamDirection::Out;
    } else if (token->klass == TokenClass::InOut) {
      next_token();
      return ParamDirection::In | ParamDirection::Out;
    } else assert(0);
  }
  return (ParamDirection)0;
}

Ast* Parser::parse_packageTypeDeclaration()
{
  if (token->klass == TokenClass::Package) {
    next_token();
    Ast* package_decl = Ast::allocate(storage);
    package_decl->init(AstEnum::packageTypeDeclaration, token->line_no, token->column_no);
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      package_decl->packageTypeDeclaration.name = name;
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        package_decl->packageTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
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
  if (token->is_typeRef() || type_ref) {
    Ast* inst_stmt = Ast::allocate(storage);
    inst_stmt->init(AstEnum::instantiation, token->line_no, token->column_no);
    inst_stmt->instantiation.type = type_ref ? type_ref : parse_typeRef();
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      inst_stmt->instantiation.args = parse_argumentList();
      if (token->klass == TokenClass::ParenthClose) {
        next_token();
        if (token->is_name()) {
          inst_stmt->instantiation.name = parse_name();
          if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::ParenthOpen) {
    next_token();
    Ast* params = parse_parameterList();
    if (token->klass == TokenClass::ParenthClose) {
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
  if (token->klass == TokenClass::ParenthOpen || token->klass == TokenClass::BraceOpen) {
    Ast* parser_decl = Ast::allocate(storage);
    parser_decl->init(AstEnum::parserDeclaration, token->line_no, token->column_no);
    parser_decl->parserDeclaration.proto = parser_proto;
    parser_decl->parserDeclaration.ctor_params = parse_constructorParameters();
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      parser_decl->parserDeclaration.local_elements = parse_parserLocalElements();
      if (token->klass == TokenClass::State) {
        parser_decl->parserDeclaration.states = parse_parserStates();
      } else error("%s:%d:%d: error: `state` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::BraceClose) {
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
  Ast* elems = Ast::allocate(storage);
  elems->init(AstEnum::parserLocalElements, token->line_no, token->column_no);
  if (token->is_parserLocalElement()) {
    Ast* ast = parse_parserLocalElement();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token->is_parserLocalElement()) {
      ast = parse_parserLocalElement();
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_parserLocalElement()
{
  if (token->is_parserLocalElement()) {
    Ast* local_element = Ast::allocate(storage);
    local_element->init(AstEnum::parserLocalElement, token->line_no, token->column_no);
    if (token->klass == TokenClass::Const) {
      local_element->parserLocalElement.element = parse_variableDeclaration(0);
      return local_element;
    } else if (token->is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TokenClass::ParenthOpen) {
        local_element->parserLocalElement.element = parse_instantiation(type_ref);
        return local_element;
      } else if (token->is_name()) {
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
  if (token->klass == TokenClass::Parser) {
    next_token();
    Ast* parser_proto = Ast::allocate(storage);
    parser_proto->init(AstEnum::parserTypeDeclaration, token->line_no, token->column_no);
    Ast* method_protos = Ast::allocate(storage);
    method_protos->init(AstEnum::methodPrototypes, parser_proto->line_no, parser_proto->column_no);
    parser_proto->parserTypeDeclaration.method_protos = method_protos;
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      parser_proto->parserTypeDeclaration.name = name;
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        parser_proto->parserTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
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
  Ast* states = Ast::allocate(storage);
  states->init(AstEnum::parserStates, token->line_no, token->column_no);
  if (token->klass == TokenClass::State) {
    Ast* ast = parse_parserState();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&states->tree, &ast->tree);
    while (token->klass == TokenClass::State) {
      ast = parse_parserState();
      tree_ctor.append_node(&states->tree, &ast->tree);
    }
  }
  return states;
}

Ast* Parser::parse_parserState()
{
  if (token->klass == TokenClass::State) {
    next_token();
    Ast* state = Ast::allocate(storage);
    state->init(AstEnum::parserState, token->line_no, token->column_no);
    state->parserState.name = parse_name();
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      state->parserState.stmt_list = parse_parserStatements();
      state->parserState.transition_stmt = parse_transitionStatement();
      if (token->klass == TokenClass::BraceClose) {
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
  Ast* stmts = Ast::allocate(storage);
  stmts->init(AstEnum::parserStatements, token->line_no, token->column_no);
  if (token->is_parserStatement()) {
    Ast* ast = parse_parserStatement();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token->is_parserStatement()) {
      ast = parse_parserStatement();
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_parserStatement()
{
  if (token->is_parserStatement()) {
    Ast* parser_stmt = Ast::allocate(storage);
    parser_stmt->init(AstEnum::parserStatement, token->line_no, token->column_no);
    if (token->is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->is_name()) {
        parser_stmt->parserStatement.stmt = parse_variableDeclaration(type_ref);
        return parser_stmt;
      } else {
        parser_stmt->parserStatement.stmt = parse_directApplication(type_ref);
        return parser_stmt;
      }
    } else if (token->is_assignmentOrMethodCallStatement()) {
      parser_stmt->parserStatement.stmt = parse_assignmentOrMethodCallStatement();
      return parser_stmt;
    } else if (token->klass == TokenClass::BraceOpen) {
      parser_stmt->parserStatement.stmt = parse_parserBlockStatement();
      return parser_stmt;
    } else if (token->klass == TokenClass::Const) {
      parser_stmt->parserStatement.stmt = parse_variableDeclaration(0);
      return parser_stmt;
    } else if (token->klass == TokenClass::Semicolon) {
      Ast* stmt = Ast::allocate(storage);
      stmt->init(AstEnum::emptyStatement, token->line_no, token->column_no);
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
  if (token->klass == TokenClass::BraceOpen) {
    next_token();
    Ast* stmt = Ast::allocate(storage);
    stmt->init(AstEnum::parserBlockStatement, token->line_no, token->column_no);
    stmt->parserBlockStatement.stmt_list = parse_parserStatements();
    if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::Transition) {
    next_token();
    Ast* transition = Ast::allocate(storage);
    transition->init(AstEnum::transitionStatement, token->line_no, token->column_no);
    transition->transitionStatement.stmt = parse_stateExpression();
    return transition;
  } else error("%s:%d:%d: error: `transition` was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_stateExpression()
{
  if (token->is_name() || token->klass == TokenClass::Select) {
    Ast* state_expr = Ast::allocate(storage);
    state_expr->init(AstEnum::stateExpression, token->line_no, token->column_no);
    if (token->is_name()) {
      state_expr->stateExpression.expr = parse_name();
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                  source_file, token->line_no, token->column_no, token->lexeme);
      return state_expr;
    } else if (token->klass == TokenClass::Select) {
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
  if (token->klass == TokenClass::Select) {
    next_token();
    Ast* select_expr = Ast::allocate(storage);
    select_expr->init(AstEnum::selectExpression, token->line_no, token->column_no);
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      select_expr->selectExpression.expr_list = parse_expressionList();
      if (token->klass == TokenClass::ParenthClose) {
        next_token();
        if (token->klass == TokenClass::BraceOpen) {
          next_token();
          select_expr->selectExpression.case_list = parse_selectCaseList();
          if (token->klass == TokenClass::BraceClose) {
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
  Ast* cases = Ast::allocate(storage);
  cases->init(AstEnum::selectCaseList, token->line_no, token->column_no);
  if (token->is_selectCase()) {
    Ast* ast = parse_selectCase();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token->is_selectCase()) {
      ast = parse_selectCase();
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_selectCase()
{
  if (token->is_keysetExpression()) {
    Ast* select_case = Ast::allocate(storage);
    select_case->init(AstEnum::selectCase, token->line_no, token->column_no);
    select_case->selectCase.keyset_expr = parse_keysetExpression();
    if (token->klass == TokenClass::Colon) {
      next_token();
      if (token->is_name()) {
        select_case->selectCase.name = parse_name();
        if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::ParenthOpen || token->is_simpleKeysetExpression()) {
    Ast* keyset_expr = Ast::allocate(storage);
    keyset_expr->init(AstEnum::keysetExpression, token->line_no, token->column_no);
    if (token->klass == TokenClass::ParenthOpen) {
      keyset_expr->keysetExpression.expr = parse_tupleKeysetExpression();
      return keyset_expr;
    } else if (token->is_simpleKeysetExpression()) {
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
  if (token->klass == TokenClass::ParenthOpen) {
    next_token();
    Ast* tuple_keyset = Ast::allocate(storage);
    tuple_keyset->init(AstEnum::tupleKeysetExpression, token->line_no, token->column_no);
    tuple_keyset->tupleKeysetExpression.expr_list = parse_simpleExpressionList();
    if (token->klass == TokenClass::ParenthClose) {
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
  Ast* exprs = Ast::allocate(storage);
  exprs->init(AstEnum::simpleExpressionList, token->line_no, token->column_no);
  if (token->is_expression()) {
    Ast* ast = parse_simpleKeysetExpression();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_simpleKeysetExpression();
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_simpleKeysetExpression()
{
  if (token->is_simpleKeysetExpression()) {
    Ast* simple_keyset = Ast::allocate(storage);
    simple_keyset->init(AstEnum::simpleKeysetExpression, token->line_no, token->column_no);
    if (token->is_expression()) {
      simple_keyset->simpleKeysetExpression.expr = parse_expression(1);
      return simple_keyset;
    } else if (token->klass == TokenClass::Default) {
      next_token();
      Ast* default_keyset = Ast::allocate(storage);
      default_keyset->init(AstEnum::default_, token->line_no, token->column_no);
      simple_keyset->simpleKeysetExpression.expr = default_keyset;
      return simple_keyset;
    } else if (token->klass == TokenClass::Dontcare) {
      next_token();
      Ast* dontcare_keyset = Ast::allocate(storage);
      dontcare_keyset->init(AstEnum::dontcare, token->line_no, token->column_no);
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
  if (token->klass == TokenClass::ParenthOpen || token->klass == TokenClass::BraceOpen) {
    Ast* control_decl = Ast::allocate(storage);
    control_decl->init(AstEnum::controlDeclaration, token->line_no, token->column_no);
    control_decl->controlDeclaration.proto = control_proto;
    control_decl->controlDeclaration.ctor_params = parse_constructorParameters();
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      control_decl->controlDeclaration.local_decls = parse_controlLocalDeclarations();
      if (token->klass == TokenClass::Apply) {
        next_token();
        control_decl->controlDeclaration.apply_stmt = parse_blockStatement();
        if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::Control) {
    next_token();
    Ast* control_proto = Ast::allocate(storage);
    control_proto->init(AstEnum::controlTypeDeclaration, token->line_no, token->column_no);
    Ast* method_protos = Ast::allocate(storage);
    method_protos->init(AstEnum::methodPrototypes, control_proto->line_no, control_proto->column_no);
    control_proto->controlTypeDeclaration.method_protos = method_protos;
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      control_proto->controlTypeDeclaration.name = name;
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        control_proto->controlTypeDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
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
  if (token->is_controlLocalDeclaration()) {
    Ast* local_decl = Ast::allocate(storage);
    local_decl->init(AstEnum::controlLocalDeclaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::Const) {
      local_decl->controlLocalDeclaration.decl = parse_variableDeclaration(0);
      return local_decl;
    } else if (token->klass == TokenClass::Action) {
      local_decl->controlLocalDeclaration.decl = parse_actionDeclaration();
      return local_decl;
    } else if (token->klass == TokenClass::Table) {
      local_decl->controlLocalDeclaration.decl = parse_tableDeclaration();
      return local_decl;
    } else if (token->is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TokenClass::ParenthOpen) {
        local_decl->controlLocalDeclaration.decl = parse_instantiation(type_ref);
        return local_decl;
      } else if (token->is_name()) {
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
  Ast* decls = Ast::allocate(storage);
  decls->init(AstEnum::controlLocalDeclarations, token->line_no, token->column_no);
  if (token->is_controlLocalDeclaration()) {
    Ast* ast = parse_controlLocalDeclaration();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&decls->tree, &ast->tree);
    while (token->is_controlLocalDeclaration()) {
      ast = parse_controlLocalDeclaration();
      tree_ctor.append_node(&decls->tree, &ast->tree);
    }
  }
  return decls;
}

/** EXTERN **/

Ast* Parser::parse_externDeclaration()
{
  if (token->klass == TokenClass::Extern) {
    next_token();

    bool is_function_type = 0;
    if (token->is_typeOrVoid() && token->is_nonTypeName()) {
      is_function_type = token->is_typeOrVoid() && peek_token()->is_name();
    } else if (token->is_typeOrVoid()) {
      is_function_type = 1;
    } else if (token->is_nonTypeName()) {
      is_function_type = 0;
    } else error("%s:%d:%d: error: extern declaration was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);

    Ast* extern_decl = Ast::allocate(storage);
    extern_decl->init(AstEnum::externDeclaration, token->line_no, token->column_no);
    if (is_function_type) {
      extern_decl->externDeclaration.decl = parse_functionPrototype(0);
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return extern_decl;
    } else {
      Ast* extern_type = Ast::allocate(storage);
      extern_type->init(AstEnum::externTypeDeclaration, token->line_no, token->column_no);
      extern_type->externTypeDeclaration.name = parse_nonTypeName();
      Ast* name = extern_type->externTypeDeclaration.name;
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      if (token->klass == TokenClass::BraceOpen) {
        next_token();
        extern_type->externTypeDeclaration.method_protos = parse_methodPrototypes();
        if (token->klass == TokenClass::BraceClose) {
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
  Ast* protos = Ast::allocate(storage);
  protos->init(AstEnum::methodPrototypes, token->line_no, token->column_no);
  if (token->is_methodPrototype()) {
    Ast* ast = parse_methodPrototype();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&protos->tree, &ast->tree);
    while (token->is_methodPrototype()) {
      ast = parse_methodPrototype();
      tree_ctor.append_node(&protos->tree, &ast->tree);
    }
  }
  return protos;
}

Ast* Parser::parse_functionPrototype(Ast* return_type)
{
  if (token->is_typeOrVoid() || return_type) {
    Ast* func_proto = Ast::allocate(storage);
    func_proto->init(AstEnum::functionPrototype, token->line_no, token->column_no);
    if (return_type) {
      func_proto->functionPrototype.return_type = return_type;
    } else {
      return_type = parse_typeOrVoid();
      if (return_type->kind == AstEnum::name) {
        Ast* name = return_type;
        current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
        Ast* type_ref = Ast::allocate(storage);
        type_ref->init(AstEnum::typeRef, token->line_no, token->column_no);
        type_ref->typeRef.type = name;
        return_type = type_ref;
      }
      func_proto->functionPrototype.return_type = return_type;
    }
    if (token->is_name()) {
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
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
  if (token->is_methodPrototype()) {
    if (token->klass == TokenClass::TypeIdentifier && peek_token()->klass == TokenClass::ParenthOpen) {
      /* Constructor */
      Ast* func_proto = Ast::allocate(storage);
      func_proto->init(AstEnum::functionPrototype, token->line_no, token->column_no);
      func_proto->functionPrototype.name = parse_name();
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        func_proto->functionPrototype.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `(` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return func_proto;
    } else if (token->is_typeOrVoid()) {
      Ast* func_proto = parse_functionPrototype(0);
      if (token->klass == TokenClass::Semicolon) {
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
  if (token->is_typeRef()) {
    Ast* type_ref = Ast::allocate(storage);
    type_ref->init(AstEnum::typeRef, token->line_no, token->column_no);
    if (token->is_baseType()) {
      type_ref->typeRef.type = parse_baseType();
      return type_ref;
    } else if (token->is_typeName()) {
      type_ref->typeRef.type = parse_namedType();
      return type_ref;
    } else if (token->klass == TokenClass::Tuple) {
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
  if (token->is_typeName()) {
    Ast* named_type = parse_typeName();
    if (token->klass == TokenClass::BracketOpen) {
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

  if (token->klass == TokenClass::TypeIdentifier) {
    type_name = Ast::allocate(storage);
    type_name->init(AstEnum::name, token->line_no, token->column_no);
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
  if (token->klass == TokenClass::Tuple) {
    Ast* tuple = Ast::allocate(storage);
    tuple->init(AstEnum::tupleType, token->line_no, token->column_no);
    next_token();
    if (token->klass == TokenClass::AngleOpen) {
      next_token();
      tuple->tupleType.type_args = parse_typeArgumentList();
      if (token->klass == TokenClass::AngleClose) {
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
  if (token->klass == TokenClass::BracketOpen) {
    next_token();
    Ast* type_ref = Ast::allocate(storage);
    type_ref->init(AstEnum::typeRef, named_type->line_no, named_type->column_no);
    type_ref->typeRef.type = named_type;
    Ast* type = Ast::allocate(storage);
    type->init(AstEnum::headerStackType, named_type->line_no, named_type->column_no);
    type->headerStackType.type = type_ref;
    if (token->is_expression()) {
      type->headerStackType.stack_expr = parse_expression(1);
      if (token->klass == TokenClass::BracketClose) {
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
  if (token->is_baseType()) {
    Ast* type_name = Ast::allocate(storage);
    type_name->init(AstEnum::name, token->line_no, token->column_no);
    if (token->klass == TokenClass::Bool) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeBoolean, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeBoolean.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::Int) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeInteger, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeInteger.name = type_name;
      next_token();
      if (token->klass == TokenClass::AngleOpen) {
        next_token();
        type->baseTypeInteger.size = parse_integerTypeSize();
        if (token->klass == TokenClass::AngleClose) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TokenClass::Bit) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeBit, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeBit.name = type_name;
      next_token();
      if (token->klass == TokenClass::AngleOpen) {
        next_token();
        type->baseTypeBit.size = parse_integerTypeSize();
        if (token->klass == TokenClass::AngleClose) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      }
      return type;
    } else if (token->klass == TokenClass::Varbit) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeVarbit, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeVarbit.name = type_name;
      next_token();
      if (token->klass == TokenClass::AngleOpen) {
        next_token();
        type->baseTypeVarbit.size = parse_integerTypeSize();
        if (token->klass == TokenClass::AngleClose) {
          next_token();
        } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: '<' was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return type;
    } else if (token->klass == TokenClass::String) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeString, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeString.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::Void) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeVoid, token->line_no, token->column_no);
      type_name->name.strname = token->lexeme;
      type->baseTypeVoid.name = type_name;
      next_token();
      return type;
    } else if (token->klass == TokenClass::Error) {
      Ast* type = Ast::allocate(storage);
      type->init(AstEnum::baseTypeError, token->line_no, token->column_no);
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
  Ast* type_size = Ast::allocate(storage);
  type_size->init(AstEnum::integerTypeSize, token->line_no, token->column_no);
  if (token->klass == TokenClass::IntegerLiteral) {
    type_size->integerTypeSize.size = parse_integer();
  } else if (token->klass == TokenClass::ParenthOpen) {
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
  if (token->is_typeOrVoid()) {
    if (token->is_typeRef()) {
      Ast* type = parse_typeRef();
      return type;
    } else if (token->klass == TokenClass::Void) {
      return parse_baseType();
    } else if (token->klass == TokenClass::Identifier) {
      Ast* name = Ast::allocate(storage);
      name->init(AstEnum::name, token->line_no, token->column_no);
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
  if (token->is_realTypeArg()) {
    Ast* type_arg = Ast::allocate(storage);
    type_arg->init(AstEnum::realTypeArg, token->line_no, token->column_no);
    if (token->klass == TokenClass::Dontcare) {
      next_token();
      Ast* dontcare_arg = Ast::allocate(storage);
      dontcare_arg->init(AstEnum::dontcare, token->line_no, token->column_no);
      type_arg->realTypeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token->is_typeRef()) {
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
  if (token->is_typeArg()) {
    Ast* type_arg = Ast::allocate(storage);
    type_arg->init(AstEnum::typeArg, token->line_no, token->column_no);
    if (token->klass == TokenClass::Dontcare) {
      next_token();
      Ast* dontcare_arg = Ast::allocate(storage);
      dontcare_arg->init(AstEnum::dontcare, token->line_no, token->column_no);
      type_arg->typeArg.arg = dontcare_arg;
      return type_arg;
    } else if (token->is_typeRef()) {
      type_arg->typeArg.arg = parse_typeRef();
      return type_arg;
    } else if (token->is_nonTypeName()) {
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
  Ast* args = Ast::allocate(storage);
  args->init(AstEnum::typeArgumentList, token->line_no, token->column_no);
  if (token->is_typeArg()) {
    Ast* ast = parse_typeArg();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_typeArg();
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_typeDeclaration()
{
  if (token->is_typeDeclaration()) {
    Ast* type_decl = Ast::allocate(storage);
    type_decl->init(AstEnum::typeDeclaration, token->line_no, token->column_no);
    if (token->is_derivedTypeDeclaration()) {
      type_decl->typeDeclaration.decl = parse_derivedTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Typedef) {
      type_decl->typeDeclaration.decl = parse_typedefDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Parser) {
      type_decl->typeDeclaration.decl = parse_parserTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Control) {
      type_decl->typeDeclaration.decl = parse_controlTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Package) {
      type_decl->typeDeclaration.decl = parse_packageTypeDeclaration();
      if (token->klass == TokenClass::Semicolon) {
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
  if (token->is_derivedTypeDeclaration()) {
    Ast* type_decl = Ast::allocate(storage);
    type_decl->init(AstEnum::derivedTypeDeclaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::Header) {
      type_decl->derivedTypeDeclaration.decl = parse_headerTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Union) {
      type_decl->derivedTypeDeclaration.decl = parse_headerUnionDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Struct) {
      type_decl->derivedTypeDeclaration.decl = parse_structTypeDeclaration();
      return type_decl;
    } else if (token->klass == TokenClass::Enum) {
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
  if (token->klass == TokenClass::Header) {
    next_token();
    Ast* header_decl = Ast::allocate(storage);
    header_decl->init(AstEnum::headerTypeDeclaration, token->line_no, token->column_no);
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      header_decl->headerTypeDeclaration.name = name;
      if (token->klass == TokenClass::BraceOpen) {
        next_token();
        header_decl->headerTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::Union) {
    next_token();
    Ast* union_decl = Ast::allocate(storage);
    union_decl->init(AstEnum::headerUnionDeclaration, token->line_no, token->column_no);
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      union_decl->headerUnionDeclaration.name = name;
      if (token->klass == TokenClass::BraceOpen) {
        next_token();
        union_decl->headerUnionDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::Struct) {
    next_token();
    Ast* struct_decl = Ast::allocate(storage);
    struct_decl->init(AstEnum::structTypeDeclaration, token->line_no, token->column_no);
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      struct_decl->structTypeDeclaration.name = name;
      if (token->klass == TokenClass::BraceOpen) {
        next_token();
        struct_decl->structTypeDeclaration.fields = parse_structFieldList();
        if (token->klass == TokenClass::BraceClose) {
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
  Ast* fields = Ast::allocate(storage);
  fields->init(AstEnum::structFieldList, token->line_no, token->column_no);
  if (token->is_structField()) {
    Ast* ast = parse_structField();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&fields->tree, &ast->tree);
    while (token->is_structField()) {
      ast = parse_structField();
      tree_ctor.append_node(&fields->tree, &ast->tree);
    }
  }
  return fields;
}

Ast* Parser::parse_structField()
{
  if (token->is_structField()) {
    Ast* field = Ast::allocate(storage);
    field->init(AstEnum::structField, token->line_no, token->column_no);
    field->structField.type = parse_typeRef();
    if (token->is_name()) {
      field->structField.name = parse_name();
      if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::Enum) {
    next_token();
    Ast* enum_decl = Ast::allocate(storage);
    enum_decl->init(AstEnum::enumDeclaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::Bit) {
      next_token();
      if (token->klass == TokenClass::AngleOpen) {
        next_token();
        if (token->klass == TokenClass::IntegerLiteral) {
          enum_decl->enumDeclaration.type_size = parse_integer();
          if (token->klass == TokenClass::AngleClose) {
            next_token();
          } else error("%s:%d:%d: error: `>` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: an integer was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `<` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
    if (token->is_name()) {
      Ast* name = parse_name();
      current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
      enum_decl->enumDeclaration.name = name;
      if (token->klass == TokenClass::BraceOpen) {
        next_token();
        if (token->is_specifiedIdentifier()) {
          enum_decl->enumDeclaration.fields = parse_specifiedIdentifierList();
          if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::Error) {
    next_token();
    Ast* error_decl = Ast::allocate(storage);
    error_decl->init(AstEnum::errorDeclaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      if (token->is_name()) {
        if (token->is_name()) {
          error_decl->errorDeclaration.fields = parse_identifierList();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        if (token->klass == TokenClass::BraceClose) {
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
  if (token->klass == TokenClass::MatchKind) {
    next_token();
    Ast* match_decl = Ast::allocate(storage);
    match_decl->init(AstEnum::matchKindDeclaration, token->line_no, token->column_no);
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      if (token->is_name()) {
        match_decl->matchKindDeclaration.fields = parse_identifierList();
        if (token->klass == TokenClass::BraceClose) {
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
  Ast* ids = Ast::allocate(storage);
  ids->init(AstEnum::identifierList, token->line_no, token->column_no);
  if (token->is_name()) {
    Ast* ast = parse_name();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_name();
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifierList()
{
  Ast* ids = Ast::allocate(storage);
  ids->init(AstEnum::specifiedIdentifierList, token->line_no, token->column_no);
  if (token->is_specifiedIdentifier()) {
    Ast* ast = parse_specifiedIdentifier();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&ids->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_specifiedIdentifier();
      tree_ctor.append_node(&ids->tree, &ast->tree);
    }
  }
  return ids;
}

Ast* Parser::parse_specifiedIdentifier()
{
  if (token->is_specifiedIdentifier()) {
    Ast* id = Ast::allocate(storage);
    id->init(AstEnum::specifiedIdentifier, token->line_no, token->column_no);
    id->specifiedIdentifier.name = parse_name();
    if (token->klass == TokenClass::Equal) {
      next_token();
      if (token->is_expression()) {
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
  if (token->klass == TokenClass::Typedef) {
    next_token();
    if (token->is_typeRef() || token->is_derivedTypeDeclaration()) {
      Ast* type_decl = Ast::allocate(storage);
      type_decl->init(AstEnum::typedefDeclaration, token->line_no, token->column_no);
      if (token->is_typeRef()) {
        type_decl->typedefDeclaration.type_ref = parse_typeRef();
      } else if (token->is_derivedTypeDeclaration()) {
        type_decl->typedefDeclaration.type_ref = parse_derivedTypeDeclaration();
      } else assert(0);
      if (token->is_name()) {
        Ast* name = parse_name();
        current_scope->bind_name(storage, name->name.strname, NameSpace::Type);
        type_decl->typedefDeclaration.name = name;
        if (token->klass == TokenClass::Semicolon) {
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
  if (token->is_lvalue()) {
    Ast* lvalue = parse_lvalue();
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      Ast* stmt = Ast::allocate(storage);
      stmt->init(AstEnum::functionCall, token->line_no, token->column_no);
      stmt->functionCall.lhs_expr = lvalue;
      stmt->functionCall.args = parse_argumentList();
      if (token->klass == TokenClass::ParenthClose) {
        next_token();
      } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else error("%s:%d:%d: error: `;` expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return stmt;
    } else if (token->klass == TokenClass::Equal) {
      next_token();
      Ast* stmt = Ast::allocate(storage);
      stmt->init(AstEnum::assignmentStatement, token->line_no, token->column_no);
      stmt->assignmentStatement.lhs_expr = lvalue;
      stmt->assignmentStatement.rhs_expr = parse_expression(1);
      if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::Return) {
    next_token();
    Ast* return_stmt = Ast::allocate(storage);
    return_stmt->init(AstEnum::returnStatement, token->line_no, token->column_no);
    if (token->is_expression())
      return_stmt->returnStatement.expr = parse_expression(1);
    if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::Exit) {
    next_token();
    Ast* exit_stmt = Ast::allocate(storage);
    exit_stmt->init(AstEnum::exitStatement, token->line_no, token->column_no);
    if (token->klass == TokenClass::Semicolon) {
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
  if (token->klass == TokenClass::If) {
    next_token();
    Ast* if_stmt = Ast::allocate(storage);
    if_stmt->init(AstEnum::conditionalStatement, token->line_no, token->column_no);
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      if (token->is_expression()) {
        if_stmt->conditionalStatement.cond_expr = parse_expression(1);
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
          if (token->is_statement()) {
            if_stmt->conditionalStatement.stmt = parse_statement(0);
            if (token->klass == TokenClass::Else) {
              next_token();
              if (token->is_statement()) {
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
  if (token->is_typeName() || type_name) {
    Ast* apply_stmt = Ast::allocate(storage);
    apply_stmt->init(AstEnum::directApplication, token->line_no, token->column_no);
    apply_stmt->directApplication.name = type_name ? type_name : parse_typeName();
    if (token->klass == TokenClass::Dot) {
      next_token();
      if (token->klass == TokenClass::Apply) {
        next_token();
        if (token->klass == TokenClass::ParenthOpen) {
          next_token();
          apply_stmt->directApplication.args = parse_argumentList();
          if (token->klass == TokenClass::ParenthClose) {
            next_token();
            if (token->klass == TokenClass::Semicolon) {
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
  if (token->is_statement()) {
    Ast* stmt = Ast::allocate(storage);
    stmt->init(AstEnum::statement, token->line_no, token->column_no);
    if (token->is_typeName() || type_name) {
      stmt->statement.stmt = parse_directApplication(type_name);
      return stmt;
    } else if (token->is_assignmentOrMethodCallStatement()) {
      stmt->statement.stmt = parse_assignmentOrMethodCallStatement();
      return stmt;
    } else if (token->klass == TokenClass::If) {
      stmt->statement.stmt = parse_conditionalStatement();
      return stmt;
    } else if (token->klass == TokenClass::Semicolon) {
      Ast* empty_stmt = Ast::allocate(storage);
      empty_stmt->init(AstEnum::emptyStatement, token->line_no, token->column_no);
      stmt->statement.stmt = empty_stmt;
      next_token();
      return stmt;
    } else if (token->klass == TokenClass::BraceOpen) {
      stmt->statement.stmt = parse_blockStatement();
      return stmt;
    } else if (token->klass == TokenClass::Exit) {
      stmt->statement.stmt = parse_exitStatement();
      return stmt;
    } else if (token->klass == TokenClass::Return) {
      stmt->statement.stmt = parse_returnStatement();
      return stmt;
    } else if (token->klass == TokenClass::Switch) {
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
  if (token->klass == TokenClass::BraceOpen) {
    next_token();
    Ast* block_stmt = Ast::allocate(storage);
    block_stmt->init(AstEnum::blockStatement, token->line_no, token->column_no);
    block_stmt->blockStatement.stmt_list = parse_statementOrDeclList();
    if (token->klass == TokenClass::BraceClose) {
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
  Ast* stmts = Ast::allocate(storage);
  stmts->init(AstEnum::statementOrDeclList, token->line_no, token->column_no);
  if (token->is_statementOrDeclaration()) {
    Ast* ast = parse_statementOrDeclaration();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&stmts->tree, &ast->tree);
    while (token->is_statementOrDeclaration()) {
      ast = parse_statementOrDeclaration();
      tree_ctor.append_node(&stmts->tree, &ast->tree);
    }
  }
  return stmts;
}

Ast* Parser::parse_switchStatement()
{
  if (token->klass == TokenClass::Switch) {
    next_token();
    Ast* stmt = Ast::allocate(storage);
    stmt->init(AstEnum::switchStatement, token->line_no, token->column_no);
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      stmt->switchStatement.expr = parse_expression(1);
      if (token->klass == TokenClass::ParenthClose) {
        next_token();
        if (token->klass == TokenClass::BraceOpen) {
          next_token();
          stmt->switchStatement.switch_cases = parse_switchCases();
          if (token->klass == TokenClass::BraceClose) {
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
  Ast* cases = Ast::allocate(storage);
  cases->init(AstEnum::switchCases, token->line_no, token->column_no);
  if (token->is_switchLabel()) {
    Ast* ast = parse_switchCase();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&cases->tree, &ast->tree);
    while (token->is_switchLabel()) {
      ast = parse_switchCase();
      tree_ctor.append_node(&cases->tree, &ast->tree);
    }
  }
  return cases;
}

Ast* Parser::parse_switchCase()
{
  if (token->is_switchLabel()) {
    Ast* switch_case = Ast::allocate(storage);
    switch_case->init(AstEnum::switchCase, token->line_no, token->column_no);
    switch_case->switchCase.label = parse_switchLabel();
    if (token->klass == TokenClass::Colon) {
      next_token();
      if (token->klass == TokenClass::BraceOpen) {
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
  if (token->is_switchLabel()) {
    Ast* switch_label = Ast::allocate(storage);
    switch_label->init(AstEnum::switchLabel, token->line_no, token->column_no);
    if (token->is_name()) {
      switch_label->switchLabel.label = parse_name();
      return switch_label;
    } else if (token->klass == TokenClass::Default) {
      next_token();
      Ast* default_label = Ast::allocate(storage);
      default_label->init(AstEnum::default_, token->line_no, token->column_no);
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
  if (token->is_statementOrDeclaration()) {
    Ast* stmt = Ast::allocate(storage);
    stmt->init(AstEnum::statementOrDeclaration, token->line_no, token->column_no);
    if (token->is_typeRef()) {
      Ast* type_ref = parse_typeRef();
      if (token->klass == TokenClass::ParenthOpen) {
        stmt->statementOrDeclaration.stmt = parse_instantiation(type_ref);
        return stmt;
      } else if (token->is_name()) {
        stmt->statementOrDeclaration.stmt = parse_variableDeclaration(type_ref);
        return stmt;
      } else {
        stmt->statementOrDeclaration.stmt = parse_statement(type_ref);
        return stmt;
      }
    } else if (token->is_statement()) {
      stmt->statementOrDeclaration.stmt = parse_statement(0);
      return stmt;
    } else if (token->klass == TokenClass::Const) {
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
  if (token->klass == TokenClass::Table) {
    next_token();
    Ast* table = Ast::allocate(storage);
    table->init(AstEnum::tableDeclaration, token->line_no, token->column_no);
    table->tableDeclaration.name = parse_name();
    Ast* method_protos = Ast::allocate(storage);
    method_protos->init(AstEnum::methodPrototypes, table->line_no, table->column_no);
    table->tableDeclaration.method_protos = method_protos;
    if (token->klass == TokenClass::BraceOpen) {
      next_token();
      if (token->is_tableProperty()) {
        table->tableDeclaration.prop_list = parse_tablePropertyList();
      } else error("%s:%d:%d: error: table property was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      if (token->klass == TokenClass::BraceClose) {
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
  Ast* props = Ast::allocate(storage);
  props->init(AstEnum::tablePropertyList, token->line_no, token->column_no);
  if (token->is_tableProperty()) {
    Ast* ast = parse_tableProperty();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&props->tree, &ast->tree);
    while (token->is_tableProperty()) {
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
  if (token->is_tableProperty()) {
#if 0
    if (token->klass == TokenClass::CONST) {
      next_token();
      is_const = 1;
    }
#endif
    Ast* table_prop = Ast::allocate(storage);
    table_prop->init(AstEnum::tableProperty, token->line_no, token->column_no);
    if (token->klass == TokenClass::Key) {
      next_token();
      Ast* prop = Ast::allocate(storage);
      prop->init(AstEnum::keyProperty, token->line_no, token->column_no);
      if (token->klass == TokenClass::Equal) {
        next_token();
        if (token->klass == TokenClass::BraceOpen) {
          next_token();
          prop->keyProperty.keyelem_list = parse_keyElementList();
          if (token->klass == TokenClass::BraceClose) {
            next_token();
          } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                       source_file, token->line_no, token->column_no, token->lexeme);
        } else error("%s:%d:%d: error: `{` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else error("%s:%d:%d: error: `=` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      table_prop->tableProperty.prop = prop;
      return table_prop;
    } else if (token->klass == TokenClass::Actions) {
      next_token();
      Ast* prop = Ast::allocate(storage);
      prop->init(AstEnum::actionsProperty, token->line_no, token->column_no);
      if (token->klass == TokenClass::Equal) {
        next_token();
        if (token->klass == TokenClass::BraceOpen) {
          next_token();
          prop->actionsProperty.action_list = parse_actionList();
          if (token->klass == TokenClass::BraceClose) {
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
  Ast* elems = Ast::allocate(storage);
  elems->init(AstEnum::keyElementList, token->line_no, token->column_no);
  if (token->is_expression()) {
    Ast* ast = parse_keyElement();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&elems->tree, &ast->tree);
    while (token->is_expression()) {
      ast = parse_keyElement();
      tree_ctor.append_node(&elems->tree, &ast->tree);
    }
  }
  return elems;
}

Ast* Parser::parse_keyElement()
{
  if (token->is_expression()) {
    Ast* key_elem = Ast::allocate(storage);
    key_elem->init(AstEnum::keyElement, token->line_no, token->column_no);
    key_elem->keyElement.expr = parse_expression(1);
    if (token->klass == TokenClass::Colon) {
      next_token();
      key_elem->keyElement.match = parse_name();
      if (token->klass == TokenClass::Semicolon) {
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
  Ast* actions = Ast::allocate(storage);
  actions->init(AstEnum::actionList, token->line_no, token->column_no);
  if (token->is_actionRef()) {
    Ast* ast = parse_actionRef();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&actions->tree, &ast->tree);
    if (token->klass == TokenClass::Semicolon) {
      next_token();
    } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                 source_file, token->line_no, token->column_no, token->lexeme);
    while (token->is_actionRef()) {
      ast = parse_actionRef();
      tree_ctor.append_node(&actions->tree, &ast->tree);
      if (token->klass == TokenClass::Semicolon) {
        next_token();
      } else error("%s:%d:%d: error: `;` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
    }
  }
  return actions;
}

Ast* Parser::parse_actionRef()
{
  if (token->is_nonTypeName()) {
    Ast* action_ref = Ast::allocate(storage);
    action_ref->init(AstEnum::actionRef, token->line_no, token->column_no);
    action_ref->actionRef.name = parse_nonTypeName();
    if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      if (token->is_argument()) {
        action_ref->actionRef.args = parse_argumentList();
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
      } else if (token->klass == TokenClass::ParenthClose) {
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
  TreeCtor tree_ctor = {0};

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
  if (token->klass == TokenClass::Action) {
    next_token();
    Ast* action_decl = Ast::allocate(storage);
    action_decl->init(AstEnum::actionDeclaration, token->line_no, token->column_no);
    if (token->is_name()) {
      action_decl->actionDeclaration.name = parse_name();
      if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        action_decl->actionDeclaration.params = parse_parameterList();
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
          if (token->klass == TokenClass::BraceOpen) {
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
  if (token->klass == TokenClass::Const) {
    next_token();
    is_const = 1;
  }
  if (token->is_typeRef() || type_ref) {
    Ast* var_decl = Ast::allocate(storage);
    var_decl->init(AstEnum::variableDeclaration, token->line_no, token->column_no);
    var_decl->variableDeclaration.type = type_ref ? type_ref : parse_typeRef();
    if (token->is_name()) {
      var_decl->variableDeclaration.name = parse_name();
      if (token->klass == TokenClass::Equal) {
        next_token();
        var_decl->variableDeclaration.init_expr = parse_expression(1);
      }
      if (token->klass == TokenClass::Semicolon) {
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

  if (token->is_typeOrVoid()) {
    func_decl = Ast::allocate(storage);
    func_decl->init(AstEnum::functionDeclaration, token->line_no, token->column_no);
    func_decl->functionDeclaration.proto = parse_functionPrototype(type_ref);
    if (token->klass == TokenClass::BraceOpen) {
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
  Ast* args = Ast::allocate(storage);
  args->init(AstEnum::argumentList, token->line_no, token->column_no);
  if (token->is_argument()) {
    Ast* ast = parse_argument();
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&args->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_argument();
      tree_ctor.append_node(&args->tree, &ast->tree);
    }
  }
  return args;
}

Ast* Parser::parse_argument()
{
  if (token->is_argument()) {
    Ast* arg = Ast::allocate(storage);
    arg->init(AstEnum::argument, token->line_no, token->column_no);
    if (token->is_expression()) {
      arg->argument.arg = parse_expression(1);
      return arg;
    } else if (token->klass == TokenClass::Dontcare) {
      next_token();
      Ast* dontcare_arg = Ast::allocate(storage);
      dontcare_arg->init(AstEnum::dontcare, token->line_no, token->column_no);
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
  Ast* exprs = Ast::allocate(storage);
  exprs->init(AstEnum::expressionList, token->line_no, token->column_no);
  if (token->is_expression()) {
    Ast* ast = parse_expression(1);
    TreeConstructor tree_ctor = {};
    tree_ctor.append_node(&exprs->tree, &ast->tree);
    while (token->klass == TokenClass::Comma) {
      next_token();
      ast = parse_expression(1);
      tree_ctor.append_node(&exprs->tree, &ast->tree);
    }
  }
  return exprs;
}

Ast* Parser::parse_lvalue()
{
  if (token->is_lvalue()) {
    Ast* lvalue = Ast::allocate(storage);
    lvalue->init(AstEnum::lvalueExpression, token->line_no, token->column_no);
    lvalue->lvalueExpression.expr = parse_nonTypeName();
    while(token->klass == TokenClass::Dot || token->klass == TokenClass::BracketOpen) {
      if (token->klass == TokenClass::Dot) {
        next_token();
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::memberSelector, token->line_no, token->column_no);
        expr->memberSelector.lhs_expr = lvalue;
        if (token->is_name()) {
          expr->memberSelector.name = parse_name();
        } else error("%s:%d:%d: error: name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = Ast::allocate(storage);
        lvalue->init(AstEnum::lvalueExpression, token->line_no, token->column_no);
        lvalue->lvalueExpression.expr = expr;
      }
      else if (token->klass == TokenClass::BracketOpen) {
        next_token();
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::arraySubscript, token->line_no, token->column_no);
        expr->arraySubscript.lhs_expr = lvalue;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TokenClass::BracketClose) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        lvalue = Ast::allocate(storage);
        lvalue->init(AstEnum::lvalueExpression, token->line_no, token->column_no);
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
  if (token->is_expression()) {
    Ast* primary = parse_expressionPrimary();
    while (token->is_exprOperator()) {
      if (token->klass == TokenClass::Dot) {
        next_token();
        Ast* expr;
        expr = Ast::allocate(storage);
        expr->init(AstEnum::memberSelector, token->line_no, token->column_no);
        expr->memberSelector.lhs_expr = primary;
        if (token->is_nonTypeName()) {
          expr->memberSelector.name = parse_nonTypeName();
        } else error("%s:%d:%d: error: non-type name was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = Ast::allocate(storage);
        primary->init(AstEnum::expression, expr->line_no, expr->column_no);
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::BracketOpen) {
        next_token();
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::arraySubscript, token->line_no, token->column_no);
        expr->arraySubscript.lhs_expr = primary;
        expr->arraySubscript.index_expr = parse_indexExpression();
        if (token->klass == TokenClass::BracketClose) {
          next_token();
        } else error("%s:%d:%d: error: `]` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = Ast::allocate(storage);
        primary->init(AstEnum::expression, expr->line_no, expr->column_no);
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::ParenthOpen) {
        next_token();
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::functionCall, token->line_no, token->column_no);
        expr->functionCall.lhs_expr = primary;
        expr->functionCall.args = parse_argumentList();
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary = Ast::allocate(storage);
        primary->init(AstEnum::expression, expr->line_no, expr->column_no);
        primary->expression.expr = expr;
      } else if (token->klass == TokenClass::Equal) {
        next_token();
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::assignmentStatement, token->line_no, token->column_no);
        expr->assignmentStatement.lhs_expr = primary;
        expr->assignmentStatement.rhs_expr = parse_expression(1);
        primary = Ast::allocate(storage);
        primary->init(AstEnum::expression, expr->line_no, expr->column_no);
        primary->expression.expr = expr;
      } else if (token->is_binaryOperator()){
        int priority = operator_priority(token);
        if (priority >= priority_threshold) {
          Ast* expr = Ast::allocate(storage);
          expr->init(AstEnum::binaryExpression, token->line_no, token->column_no);
          expr->binaryExpression.left_operand = primary;
          expr->binaryExpression.op = token_to_binop(token);
          expr->binaryExpression.strname = token->lexeme;
          next_token();
          expr->binaryExpression.right_operand = parse_expression(priority + 1);
          primary = Ast::allocate(storage);
          primary->init(AstEnum::expression, expr->line_no, expr->column_no);
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
  if (token->is_expression()) {
    Ast* primary = Ast::allocate(storage);
    primary->init(AstEnum::expression, token->line_no, token->column_no);
    if (token->klass == TokenClass::IntegerLiteral) {
      primary->expression.expr = parse_integer();
      return primary;
    } else if (token->klass == TokenClass::True || token->klass == TokenClass::False) {
      primary->expression.expr = parse_boolean();
      return primary;
    } else if (token->klass == TokenClass::StringLiteral) {
      primary->expression.expr = parse_string();
      return primary;
    } else if (token->klass == TokenClass::Dot) {
      next_token();
      if (token->klass == TokenClass::Identifier) {
        primary->expression.expr = parse_nonTypeName();
        return primary;
      } else if (token->klass == TokenClass::TypeIdentifier) {
        primary->expression.expr = parse_typeName();
        return primary;
      } else error("%s:%d:%d: error: unexpected token `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->is_nonTypeName()) {
      primary->expression.expr = parse_nonTypeName();
      return primary;
    } else if (token->klass == TokenClass::BraceOpen) {
      next_token();
      primary->expression.expr = parse_expressionList();
      if (token->klass == TokenClass::BraceClose) {
        next_token();
      } else error("%s:%d:%d: error: `}` was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      return primary;
    } else if (token->klass == TokenClass::ParenthOpen) {
      next_token();
      if (token->klass == TokenClass::TypeIdentifier && peek_token()->klass == TokenClass::Dot) {
        /* (<typeName>.<name>) */
        primary->expression.expr = parse_expression(1);
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else if (token->is_typeRef()) {
        Ast* expr = Ast::allocate(storage);
        expr->init(AstEnum::castExpression, token->line_no, token->column_no);
        expr->castExpression.type = parse_typeRef();
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
          expr->castExpression.expr = parse_expression(10);
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        primary->expression.expr = expr;
        return primary;
      } else if (token->is_expression()) {
        primary->expression.expr = parse_expression(1);
        if (token->klass == TokenClass::ParenthClose) {
          next_token();
        } else error("%s:%d:%d: error: `)` was expected, got `%s`.",
                     source_file, token->line_no, token->column_no, token->lexeme);
        return primary;
      } else error("%s:%d:%d: error: expression was expected, got `%s`.",
                   source_file, token->line_no, token->column_no, token->lexeme);
      assert(0);
    } else if (token->klass == TokenClass::Exclamation) {
      next_token();
      Ast* expr = Ast::allocate(storage);
      expr->init(AstEnum::unaryExpression, token->line_no, token->column_no);
      expr->unaryExpression.op = AstOperator::Not;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TokenClass::Tilda) {
      next_token();
      Ast* expr = Ast::allocate(storage);
      expr->init(AstEnum::unaryExpression, token->line_no, token->column_no);
      expr->unaryExpression.op = AstOperator::BitwNot;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->klass == TokenClass::UnaryMinus) {
      next_token();
      Ast* expr = Ast::allocate(storage);
      expr->init(AstEnum::unaryExpression, token->line_no, token->column_no);
      expr->unaryExpression.op = AstOperator::Neg;
      expr->unaryExpression.strname = token->lexeme;
      expr->unaryExpression.operand = parse_expression(1);
      primary->expression.expr = expr;
      return primary;
    } else if (token->is_typeName()) {
      primary->expression.expr = parse_typeName();
      return primary;
    } else if (token->klass == TokenClass::Error) {
      next_token();
      Ast* expr = Ast::allocate(storage);
      expr->init(AstEnum::name, token->line_no, token->column_no);
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
  if (token->is_expression()) {
    Ast* index_expr = Ast::allocate(storage);
    index_expr->init(AstEnum::indexExpression, token->line_no, token->column_no);
    index_expr->indexExpression.start_index = parse_expression(1);
    if (token->klass == TokenClass::Colon) {
      next_token();
      if (token->is_expression()) {
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
  if (token->klass == TokenClass::IntegerLiteral) {
    Ast* int_literal = Ast::allocate(storage);
    int_literal->init(AstEnum::integerLiteral, token->line_no, token->column_no);
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
  if (token->klass == TokenClass::True || token->klass == TokenClass::False) {
    Ast* bool_literal = Ast::allocate(storage);
    bool_literal->init(AstEnum::booleanLiteral, token->line_no, token->column_no);
    bool_literal->booleanLiteral.value = (token->klass == TokenClass::True);
    next_token();
    return bool_literal;
  } else error("%s:%d:%d: error: boolean was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}

Ast* Parser::parse_string()
{
  if (token->klass == TokenClass::StringLiteral) {
    Ast* string_literal = Ast::allocate(storage);
    string_literal->init(AstEnum::stringLiteral, token->line_no, token->column_no);
    string_literal->stringLiteral.value = token->lexeme;
    next_token();
    return string_literal;
  } else error("%s:%d:%d: error: string was expected, got `%s`.",
               source_file, token->line_no, token->column_no, token->lexeme);
  assert(0);
  return 0;
}
