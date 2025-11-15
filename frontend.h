#pragma once
#include "foundation.h"
#include "ast_tree.h"

struct SourceText {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;
};

enum class TokenClass {
  NONE = 0,

  /* Operators and syntactic structure */

  SEMICOLON,
  IDENTIFIER,
  TYPE_IDENTIFIER,
  INTEGER_LITERAL,
  STRING_LITERAL,
  PARENTH_OPEN,
  PARENTH_CLOSE,
  ANGLE_OPEN,
  ANGLE_CLOSE,
  BRACE_OPEN,
  BRACE_CLOSE,
  BRACKET_OPEN,
  BRACKET_CLOSE,
  DONTCARE,
  COLON,
  DOT,
  COMMA,
  MINUS,
  UNARY_MINUS,
  PLUS,
  STAR,
  SLASH,
  EQUAL,
  DOUBLE_EQUAL,
  EXCLAMATION_EQUAL,
  EXCLAMATION,
  DOUBLE_PIPE,
  ANGLE_OPEN_EQUAL,
  ANGLE_CLOSE_EQUAL,
  TILDA,
  AMPERSAND,
  DOUBLE_AMPERSAND,
  TRIPLE_AMPERSAND,
  PIPE,
  CIRCUMFLEX,
  DOUBLE_ANGLE_OPEN,
  DOUBLE_ANGLE_CLOSE,
  COMMENT,

  /* Keywords */

  ACTION,
  ACTIONS,
  ENUM,
  IN,
  PACKAGE,
  SELECT,
  SWITCH,
  TUPLE,
  VOID,
  APPLY,
  CONTROL,
  ERROR,
  HEADER,
  INOUT,
  PARSER,
  STATE,
  TABLE,
  ENTRIES,
  KEY,
  TYPEDEF,
  BOOL,
  TRUE,
  FALSE,
  DEFAULT,
  EXTERN,
  UNION,
  INT,
  BIT,
  VARBIT,
  STRING,
  OUT,
  TRANSITION,
  ELSE,
  EXIT,
  IF,
  MATCH_KIND,
  RETURN,
  STRUCT,
  CONST,

  /* Control */

  UNKNOWN,
  START_OF_INPUT,
  END_OF_INPUT,
  LEXICAL_ERROR,
};

struct Token {
  enum TokenClass klass;
  char* lexeme;
  int line_no;
  int column_no;

  union {
    struct {
      bool is_signed;
      int width;
      int64_t value;
    } integer;
    char* str;
  };

  bool token_is_nonTypeName();
  bool token_is_name();
  bool token_is_typeName();
  bool token_is_nonTableKwName();
  bool token_is_baseType();
  bool token_is_typeRef();
  bool token_is_direction();
  bool token_is_parameter();
  bool token_is_derivedTypeDeclaration();
  bool token_is_typeDeclaration();
  bool token_is_typeArg();
  bool token_is_typeOrVoid();
  bool token_is_actionRef();
  bool token_is_tableProperty();
  bool token_is_switchLabel();
  bool token_is_expressionPrimary();
  bool token_is_expression();
  bool token_is_methodPrototype();
  bool token_is_structField();
  bool token_is_specifiedIdentifier();
  bool token_is_declaration();
  bool token_is_lvalue();
  bool token_is_assignmentOrMethodCallStatement();
  bool token_is_statement();
  bool token_is_statementOrDeclaration();
  bool token_is_argument();
  bool token_is_parserLocalElement();
  bool token_is_parserStatement();
  bool token_is_simpleKeysetExpression();
  bool token_is_keysetExpression();
  bool token_is_selectCase();
  bool token_is_controlLocalDeclaration();
  bool token_is_realTypeArg();
  bool token_is_binaryOperator();
  bool token_is_exprOperator();
};

struct Lexeme {
  char* start;
  char* end;
};

struct Lexer {
  Arena* storage;
  char*  text;
  int    text_size;
  char*  filename;
  int    line_no;
  char*  line_start;
  int    state;
  Token  token;
  Lexeme lexeme[2];
  Array* tokens;

  char char_lookahead(int pos);
  char char_advance(int pos);
  char char_retract();
  void lexeme_advance();
  void token_install_integer(Token* token, Lexeme* lexeme, int base);
  void next_token(Token* token);
  void tokenize(SourceText* source_text);
};

enum class AstEnum {
  none = 0,

  /** PROGRAM **/

  p4program,
  declarationList,
  declaration,
  name,
  parameterList,
  parameter,
  paramDirection,
  packageTypeDeclaration,
  instantiation,

  /** PARSER **/

  parserDeclaration,
  parserTypeDeclaration,
  parserLocalElements,
  parserLocalElement,
  parserStates,
  parserState,
  parserStatements,
  parserStatement,
  parserBlockStatement,
  transitionStatement,
  stateExpression,
  selectExpression,
  selectCaseList,
  selectCase,
  keysetExpression,
  tupleKeysetExpression,
  simpleKeysetExpression,
  simpleExpressionList,

  /** CONTROL **/

  controlDeclaration,
  controlTypeDeclaration,
  controlLocalDeclarations,
  controlLocalDeclaration,

  /** EXTERN **/

  externDeclaration,
  externTypeDeclaration,
  methodPrototypes,
  functionPrototype,

  /** TYPES **/

  typeRef,
  tupleType,
  headerStackType,
  baseTypeBoolean,
  baseTypeInteger,
  baseTypeBit,
  baseTypeVarbit,
  baseTypeString,
  baseTypeVoid,
  baseTypeError,
  integerTypeSize,
  realTypeArg,
  typeArg,
  typeArgumentList,
  typeDeclaration,
  derivedTypeDeclaration,
  headerTypeDeclaration,
  headerUnionDeclaration,
  structTypeDeclaration,
  structFieldList,
  structField,
  enumDeclaration,
  errorDeclaration,
  matchKindDeclaration,
  identifierList,
  specifiedIdentifierList,
  specifiedIdentifier,
  typedefDeclaration,

  /** STATEMENTS **/

  assignmentStatement,
  emptyStatement,
  returnStatement,
  exitStatement,
  conditionalStatement,
  directApplication,
  statement,
  blockStatement,
  statementOrDeclaration,
  statementOrDeclList,
  switchStatement,
  switchCases,
  switchCase,
  switchLabel,

  /** TABLES **/

  tableDeclaration,
  tablePropertyList,
  tableProperty,
  keyProperty,
  keyElementList,
  keyElement,
  actionsProperty,
  actionList,
  actionRef,
#if 0
  entriesProperty,
  entriesList,
  entry,
  simpleProperty,
#endif
  actionDeclaration,

  /** VARIABLES **/

  variableDeclaration,

  /** EXPRESSIONS **/

  functionDeclaration,
  argumentList,
  argument,
  expressionList,
  expression,
  lvalueExpression,
  binaryExpression,
  unaryExpression,
  functionCall,
  memberSelector,
  castExpression,
  arraySubscript,
  indexExpression,
  integerLiteral,
  booleanLiteral,
  stringLiteral,
  default_,
  dontcare,
};
char* AstEnum_to_string(enum AstEnum ast);

enum class AstOperator : int {
  NONE = 0,

  /* Arithmetic */

  ADD,
  SUB,
  MUL,
  DIV,
  NEG,

  /* Logical */

  AND,
  OR,
  NOT,

  /* Relational */

  EQ,
  NEQ,
  LESS,
  GREAT,
  LESS_EQ,
  GREAT_EQ,

  /* Bitwise */

  BITW_AND,
  BITW_OR,
  BITW_XOR,
  BITW_NOT,
  BITW_SHL,
  BITW_SHR,

  MASK,
};

enum class ParamDirection : int {
  NONE = 0,
  IN   = 1 << 1,
  OUT  = 1 << 2,
};
inline ParamDirection operator | (ParamDirection lhs, ParamDirection rhs) {
    return (ParamDirection)((int)lhs | (int)rhs);
}
inline ParamDirection operator & (ParamDirection lhs, ParamDirection rhs) {
    return (ParamDirection)((int)lhs & (int)rhs);
}

struct Ast {
  enum AstEnum kind;
  int line_no;
  int column_no;
  AstTree tree;

  union {

    /** PROGRAM **/

    struct {
      struct Ast* decl_list;
    } p4program;

    struct {
    } declarationList;

    struct {
      struct Ast* decl;
    } declaration;

    struct {
      char* strname;
    } name;

    struct {
    } parameterList;

    struct {
      enum ParamDirection direction;
      struct Ast* name;
      struct Ast* type;
      struct Ast* init_expr;
    } parameter;

    struct {
      struct Ast* name;
      struct Ast* params;
    } packageTypeDeclaration;

    struct {
      struct Ast* name;
      struct Ast* type;
      struct Ast* args;
    } instantiation;

    /** PARSER **/

    struct {
      struct Ast* proto;
      struct Ast* ctor_params;
      struct Ast* local_elements;
      struct Ast* states;
    } parserDeclaration;

    struct {
      struct Ast* name;
      struct Ast* params;
      struct Ast* method_protos;
    } parserTypeDeclaration;

    struct {
    } parserLocalElements;

    struct {
      struct Ast* element;
    } parserLocalElement;

    struct {
    } parserStates;

    struct {
      struct Ast* name;
      struct Ast* stmt_list;
      struct Ast* transition_stmt;
    } parserState;

    struct {
    } parserStatements;

    struct {
      struct Ast* stmt;
    } parserStatement;

    struct {
      struct Ast* stmt_list;
    } parserBlockStatement;

    struct {
      struct Ast* stmt;
    } transitionStatement;

    struct {
      struct Ast* expr;
    } stateExpression;

    struct {
      struct Ast* expr_list;
      struct Ast* case_list;
    } selectExpression;

    struct {
    } selectCaseList;

    struct {
      struct Ast* keyset_expr;
      struct Ast* name;
    } selectCase;

    struct {
      struct Ast* expr;
    } keysetExpression;

    struct {
      struct Ast* expr_list;
    } tupleKeysetExpression;

    struct {
      struct Ast* expr;
    } simpleKeysetExpression;

    struct {
    } simpleExpressionList;

    /** CONTROL **/

    struct {
      struct Ast* proto;
      struct Ast* ctor_params;
      struct Ast* local_decls;
      struct Ast* apply_stmt;
    } controlDeclaration;

    struct {
      struct Ast* name;
      struct Ast* params;
      struct Ast* method_protos;
    } controlTypeDeclaration;

    struct {
    } controlLocalDeclarations;

    struct {
      struct Ast* decl;
    } controlLocalDeclaration;

    /** EXTERN **/

    struct {
      struct Ast* decl;
    } externDeclaration;

    struct {
      struct Ast* name;
      struct Ast* method_protos;
    } externTypeDeclaration;

    struct {
    } methodPrototypes;

    struct {
      struct Ast* return_type;
      struct Ast* name;
      struct Ast* params;
    } functionPrototype;

    /** TYPES **/

    struct {
      struct Ast* type;
    } typeRef;

    struct {
      struct Ast* type_args;
    } tupleType;

    struct {
      struct Ast* type;
      struct Ast* stack_expr;
    } headerStackType;

    struct {
      struct Ast* name;
    } baseTypeBoolean;

    struct {
      struct Ast* name;
      struct Ast* size;
    } baseTypeInteger;

    struct {
      struct Ast* name;
      struct Ast* size;
    } baseTypeBit;

    struct {
      struct Ast* name;
      struct Ast* size;
    } baseTypeVarbit;

    struct {
      struct Ast* name;
    } baseTypeString;

    struct {
      struct Ast* name;
    } baseTypeVoid;

    struct {
      struct Ast* name;
    } baseTypeError;

    struct {
      struct Ast* size;
    } integerTypeSize;

    struct {
      struct Ast* arg;
    } realTypeArg;

    struct {
      struct Ast* arg;
    } typeArg;

    struct {
    } typeArgumentList;

    struct {
      struct Ast* decl;
    } typeDeclaration;

    struct {
      struct Ast* decl;
    } derivedTypeDeclaration;

    struct {
      struct Ast* name;
      struct Ast* fields;
    } headerTypeDeclaration;

    struct {
      struct Ast* name;
      struct Ast* fields;
    } headerUnionDeclaration;

    struct {
      struct Ast* name;
      struct Ast* fields;
    } structTypeDeclaration;

    struct {
    } structFieldList;

    struct {
      struct Ast* type;
      struct Ast* name;
    } structField;

    struct {
      struct Ast* type_size;
      struct Ast* name;
      struct Ast* fields;
    } enumDeclaration;

    struct {
      struct Ast* fields;
    } errorDeclaration;

    struct {
      struct Ast* fields;
    } matchKindDeclaration;

    struct {
    } identifierList;

    struct {
    } specifiedIdentifierList;

    struct {
      struct Ast* name;
      struct Ast* init_expr;
    } specifiedIdentifier;

    struct {
      struct Ast* type_ref;
      struct Ast* name;
    } typedefDeclaration;

    /** STATEMENTS **/

    struct {
      struct Ast* lhs_expr;
      struct Ast* rhs_expr;
    } assignmentStatement;

    struct {
      struct Ast* lhs_expr;
      struct Ast* args;
    } functionCall;

    struct {
      struct Ast* expr;
    } returnStatement;

    struct {
    } exitStatement;

    struct {
      struct Ast* cond_expr;
      struct Ast* stmt;
      struct Ast* else_stmt;
    } conditionalStatement;

    struct {
      struct Ast* name;
      struct Ast* args;
    } directApplication;

    struct {
      struct Ast* stmt;
    } statement;

    struct {
      struct Ast* stmt_list;
    } blockStatement;

    struct {
    } statementOrDeclList;

    struct {
      struct Ast* expr;
      struct Ast* switch_cases;
    } switchStatement;

    struct {
    } switchCases;

    struct {
      struct Ast* label;
      struct Ast* stmt;
    } switchCase;

    struct {
      struct Ast* label;
    } switchLabel;

    struct {
      struct Ast* stmt;
    } statementOrDeclaration;

    /** TABLES **/

    struct {
      struct Ast* name;
      struct Ast* prop_list;
      struct Ast* method_protos;
    } tableDeclaration;

    struct {
    } tablePropertyList;

    struct {
      struct Ast* prop;
    } tableProperty;

    struct {
      struct Ast* keyelem_list;
    } keyProperty;

    struct {
    } keyElementList;

    struct {
      struct Ast* expr;
      struct Ast* match;
    } keyElement;

    struct {
      struct Ast* action_list;
    } actionsProperty;

    struct {
    } actionList;

    struct {
      struct Ast* name;
      struct Ast* args;
    } actionRef;

#if 0
    struct {
      struct Ast* entries_list;
    } entriesProperty;

    struct {
    } entriesList;

    struct {
      struct Ast* keyset;
      struct Ast* action;
    } entry;

    struct {
      struct Ast* name;
      struct Ast* init_expr;
      bool is_const;
    } simpleProperty;
#endif

    struct {
      struct Ast* name;
      struct Ast* params;
      struct Ast* stmt;
    } actionDeclaration;

    /** VARIABLES **/

    struct {
      struct Ast* type;
      struct Ast* name;
      struct Ast* init_expr;
      bool is_const;
    } variableDeclaration;

    /** EXPRESSIONS **/

    struct {
      struct Ast* proto;
      struct Ast* stmt;
    } functionDeclaration;

    struct {
    } argumentList;

    struct {
      struct Ast* arg;
    } argument;

    struct {
    } expressionList;

    struct {
      struct Ast* expr;
    } lvalueExpression;

    struct {
      struct Ast* expr;
    } expression;

    struct {
      struct Ast* type;
      struct Ast* expr;
    } castExpression;

    struct {
      enum AstOperator op;
      char* strname;
      struct Ast* operand;
    } unaryExpression;

    struct {
      enum AstOperator op;
      char* strname;
      struct Ast* left_operand;
      struct Ast* right_operand;
    } binaryExpression;

    struct {
      struct Ast* lhs_expr;
      struct Ast* name;
    } memberSelector;

    struct {
      struct Ast* lhs_expr;
      struct Ast* index_expr;
    } arraySubscript;

    struct {
      struct Ast* start_index;
      struct Ast* end_index;
    } indexExpression;

    struct {
      bool is_signed;
      int value;
      int width;
    } integerLiteral;

    struct {
      bool value;
    } booleanLiteral;

    struct {
      char* value;
    } stringLiteral;

    struct {
    } default_, dontcare;
  };

  Ast* clone(Arena* storage);
};

struct NameEntry;
struct NameDeclaration;

enum class NameSpace : int {
  VAR     = 1 << 0,
  TYPE    = 1 << 1,
  KEYWORD = 1 << 2,
};
#define NameSpace_COUNT 3
inline NameSpace operator | (NameSpace lhs, NameSpace rhs) {
    return (NameSpace)((int)lhs | (int)rhs);
}
inline NameSpace operator & (NameSpace lhs, NameSpace rhs) {
    return (NameSpace)((int)lhs & (int)rhs);
}
char* NameSpace_to_string(enum NameSpace ns);

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Strmap name_table;

  static Scope* create(Arena* storage, int segment_count);
  Scope* push(Scope* parent_scope);
  Scope* pop();
  NameEntry* lookup(char* name, enum NameSpace ns);
  NameDeclaration* builtin_lookup(char* strname, enum NameSpace ns);
  NameDeclaration* bind(Arena* storage, char* strname, enum NameSpace ns);
};

struct Parser {
  Arena* storage;
  Ast* p4program;
  char* source_file;
  Array* tokens;
  int token_at;
  int prev_token_at;
  Token* token;
  Token* prev_token;
  Scope* current_scope;
  Scope* root_scope;

/** PROGRAM **/

  Ast* parse_p4program();
  Ast* parse_declarationList();
  Ast* parse_declaration();
  Ast* parse_nonTypeName();
  Ast* parse_name();
  Ast* parse_parameterList();
  Ast* parse_parameter();
  enum ParamDirection parse_direction();
  Ast* parse_packageTypeDeclaration();
  Ast* parse_instantiation(Ast* type_ref);
  Ast* parse_constructorParameters();

/** PARSER **/

  Ast* parse_parserDeclaration(Ast* parser_proto);
  Ast* parse_parserLocalElements();
  Ast* parse_parserLocalElement();
  Ast* parse_parserTypeDeclaration();
  Ast* parse_parserStates();
  Ast* parse_parserState();
  Ast* parse_parserStatements();
  Ast* parse_parserStatement();
  Ast* parse_parserBlockStatement();
  Ast* parse_transitionStatement();
  Ast* parse_stateExpression();
  Ast* parse_selectExpression();
  Ast* parse_selectCaseList();
  Ast* parse_selectCase();
  Ast* parse_keysetExpression();
  Ast* parse_tupleKeysetExpression();
  Ast* parse_simpleExpressionList();
  Ast* parse_simpleKeysetExpression();

/** CONTROL **/

  Ast* parse_controlDeclaration(Ast* control_proto);
  Ast* parse_controlTypeDeclaration();
  Ast* parse_controlLocalDeclaration();
  Ast* parse_controlLocalDeclarations();

/** EXTERN **/

  Ast* parse_externDeclaration();
  Ast* parse_methodPrototypes();
  Ast* parse_functionPrototype(Ast* return_type);
  Ast* parse_methodPrototype();

/** TYPES **/

  Ast* parse_typeRef();
  Ast* parse_namedType();
  Ast* parse_typeName();
  Ast* parse_tupleType();
  Ast* parse_headerStackType(Ast* named_type);
  Ast* parse_baseType();
  Ast* parse_integerTypeSize();
  Ast* parse_typeOrVoid();
  Ast* parse_realTypeArg();
  Ast* parse_typeArg();
  Ast* parse_typeArgumentList();
  Ast* parse_typeDeclaration();
  Ast* parse_derivedTypeDeclaration();
  Ast* parse_headerTypeDeclaration();
  Ast* parse_headerUnionDeclaration();
  Ast* parse_structTypeDeclaration();
  Ast* parse_structFieldList();
  Ast* parse_structField();
  Ast* parse_enumDeclaration();
  Ast* parse_errorDeclaration();
  Ast* parse_matchKindDeclaration();
  Ast* parse_identifierList();
  Ast* parse_specifiedIdentifierList();
  Ast* parse_specifiedIdentifier();
  Ast* parse_typedefDeclaration();

/** STATEMENTS **/

  Ast* parse_assignmentOrMethodCallStatement();
  Ast* parse_returnStatement();
  Ast* parse_exitStatement();
  Ast* parse_conditionalStatement();
  Ast* parse_directApplication(Ast* type_name);
  Ast* parse_statement(Ast* type_name);
  Ast* parse_blockStatement();
  Ast* parse_statementOrDeclList();
  Ast* parse_switchStatement();
  Ast* parse_switchCases();
  Ast* parse_switchCase();
  Ast* parse_switchLabel();
  Ast* parse_statementOrDeclaration();

/** TABLES **/

  Ast* parse_tableDeclaration();
  Ast* parse_tablePropertyList();
  Ast* parse_tableProperty();
  Ast* parse_keyElementList();
  Ast* parse_keyElement();
  Ast* parse_actionList();
  Ast* parse_actionRef();
  Ast* parse_entriesList();
  Ast* parse_entry();
  Ast* parse_actionDeclaration();

/** VARIABLES **/

  Ast* parse_variableDeclaration(Ast* type_ref);

/** EXPRESSIONS **/

  Ast* parse_functionDeclaration(Ast* type_ref);
  Ast* parse_argumentList();
  Ast* parse_argument();
  Ast* parse_expressionList();
  Ast* parse_lvalue();
  Ast* parse_expression(int priority_threshold);
  Ast* parse_expressionPrimary();
  Ast* parse_indexExpression();
  Ast* parse_integer();
  Ast* parse_boolean();
  Ast* parse_string();

  void parse();
  Token* next_token();
  Token* peek_token();
  void define_keywords(Scope* scope);
};

struct BuiltinMethodBuilder {
  Arena* storage;
};

struct ScopeBuilder {
  Arena* storage;
  Ast* p4program;
  Scope* root_scope;
  Scope* current_scope;
  Map* scope_map;

/** PROGRAM **/

  void visit_p4program(Ast* p4program);
  void visit_declarationList(Ast* decl_list);
  void visit_declaration(Ast* decl);
  void visit_name(Ast* name);
  void visit_parameterList(Ast* params);
  void visit_parameter(Ast* param);
  void visit_packageTypeDeclaration(Ast* type_decl);
  void visit_instantiation(Ast* inst);

/** PARSER **/

  void visit_parserDeclaration(Ast* parser_decl);
  void visit_parserTypeDeclaration(Ast* type_decl);
  void visit_parserLocalElements(Ast* local_elements);
  void visit_parserLocalElement(Ast* local_element);
  void visit_parserStates(Ast* states);
  void visit_parserState(Ast* state);
  void visit_parserStatements(Ast* stmts);
  void visit_parserStatement(Ast* stmt);
  void visit_parserBlockStatement(Ast* block_stmt);
  void visit_transitionStatement(Ast* transition_stmt);
  void visit_stateExpression(Ast* state_expr);
  void visit_selectExpression(Ast* select_expr);
  void visit_selectCaseList(Ast* case_list);
  void visit_selectCase(Ast* select_case);
  void visit_keysetExpression(Ast* keyset_expr);
  void visit_tupleKeysetExpression(Ast* tuple_expr);
  void visit_simpleKeysetExpression(Ast* simple_expr);
  void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

  void visit_controlDeclaration(Ast* control_decl);
  void visit_controlTypeDeclaration(Ast* type_decl);
  void visit_controlLocalDeclarations(Ast* local_decls);
  void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

  void visit_externDeclaration(Ast* extern_decl);
  void visit_externTypeDeclaration(Ast* type_decl);
  void visit_methodPrototypes(Ast* protos);
  void visit_functionPrototype(Ast* func_proto);

/** TYPES **/

  void visit_typeRef(Ast* type_ref);
  void visit_tupleType(Ast* type);
  void visit_headerStackType(Ast* type_decl);
  void visit_baseTypeBoolean(Ast* bool_type);
  void visit_baseTypeInteger(Ast* int_type);
  void visit_baseTypeBit(Ast* bit_type);
  void visit_baseTypeVarbit(Ast* varbit_type);
  void visit_baseTypeString(Ast* str_type);
  void visit_baseTypeVoid(Ast* void_type);
  void visit_baseTypeError(Ast* error_type);
  void visit_integerTypeSize(Ast* type_size);
  void visit_realTypeArg(Ast* type_arg);
  void visit_typeArg(Ast* type_arg);
  void visit_typeArgumentList(Ast* arg_list);
  void visit_typeDeclaration(Ast* type_decl);
  void visit_derivedTypeDeclaration(Ast* type_decl);
  void visit_headerTypeDeclaration(Ast* header_decl);
  void visit_headerUnionDeclaration(Ast* union_decl);
  void visit_structTypeDeclaration(Ast* struct_decl);
  void visit_structFieldList(Ast* field_list);
  void visit_structField(Ast* field);
  void visit_enumDeclaration(Ast* enum_decl);
  void visit_errorDeclaration(Ast* error_decl);
  void visit_matchKindDeclaration(Ast* match_decl);
  void visit_identifierList(Ast* ident_list);
  void visit_specifiedIdentifierList(Ast* ident_list);
  void visit_specifiedIdentifier(Ast* ident);
  void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

  void visit_assignmentStatement(Ast* assign_stmt);
  void visit_functionCall(Ast* func_call);
  void visit_returnStatement(Ast* return_stmt);
  void visit_exitStatement(Ast* exit_stmt);
  void visit_conditionalStatement(Ast* cond_stmt);
  void visit_directApplication(Ast* applic_stmt);
  void visit_statement(Ast* stmt);
  void visit_blockStatement(Ast* block_stmt);
  void visit_statementOrDeclList(Ast* stmt_list);
  void visit_switchStatement(Ast* switch_stmt);
  void visit_switchCases(Ast* switch_cases);
  void visit_switchCase(Ast* switch_case);
  void visit_switchLabel(Ast* label);
  void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

  void visit_tableDeclaration(Ast* table_decl);
  void visit_tablePropertyList(Ast* prop_list);
  void visit_tableProperty(Ast* table_prop);
  void visit_keyProperty(Ast* key_prop);
  void visit_keyElementList(Ast* element_list);
  void visit_keyElement(Ast* element);
  void visit_actionsProperty(Ast* actions_prop);
  void visit_actionList(Ast* action_list);
  void visit_actionRef(Ast* action_ref);
  void visit_entriesProperty(Ast* entries_prop);
  void visit_entriesList(Ast* entries_list);
  void visit_entry(Ast* entry);
  void visit_simpleProperty(Ast* simple_prop);
  void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

  void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

  void visit_functionDeclaration(Ast* func_decl);
  void visit_argumentList(Ast* arg_list);
  void visit_argument(Ast* arg);
  void visit_expressionList(Ast* expr_list);
  void visit_lvalueExpression(Ast* lvalue_expr);
  void visit_expression(Ast* expr);
  void visit_castExpression(Ast* cast_expr);
  void visit_unaryExpression(Ast* unary_expr);
  void visit_binaryExpression(Ast* binary_expr);
  void visit_memberSelector(Ast* selector);
  void visit_arraySubscript(Ast* subscript);
  void visit_indexExpression(Ast* index_expr);
  void visit_booleanLiteral(Ast* bool_literal);
  void visit_integerLiteral(Ast* int_literal);
  void visit_stringLiteral(Ast* str_literal);
  void visit_default(Ast* default_);
  void visit_dontcare(Ast* dontcare);

  void scope_hierarchy();
};

struct NameBinder {
  Arena* storage;
  Ast* p4program;
  Scope* root_scope;
  Scope* current_scope;
  Map* scope_map;
  Map* decl_map;
  Array* type_array;

/** PROGRAM **/

  void visit_p4program(Ast* p4program);
  void visit_declarationList(Ast* decl_list);
  void visit_declaration(Ast* decl);
  void visit_name(Ast* name);
  void visit_parameterList(Ast* params);
  void visit_parameter(Ast* param);
  void visit_packageTypeDeclaration(Ast* type_decl);
  void visit_instantiation(Ast* inst);

/** PARSER **/

  void visit_parserDeclaration(Ast* parser_decl);
  void visit_parserTypeDeclaration(Ast* type_decl);
  void visit_parserLocalElements(Ast* local_elements);
  void visit_parserLocalElement(Ast* local_element);
  void visit_parserStates(Ast* states);
  void visit_parserState(Ast* state);
  void visit_parserStatements(Ast* stmts);
  void visit_parserStatement(Ast* stmt);
  void visit_parserBlockStatement(Ast* block_stmt);
  void visit_transitionStatement(Ast* transition_stmt);
  void visit_stateExpression(Ast* state_expr);
  void visit_selectExpression(Ast* select_expr);
  void visit_selectCaseList(Ast* case_list);
  void visit_selectCase(Ast* select_case);
  void visit_keysetExpression(Ast* keyset_expr);
  void visit_tupleKeysetExpression(Ast* tuple_expr);
  void visit_simpleKeysetExpression(Ast* simple_expr);
  void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

  void visit_controlDeclaration(Ast* control_decl);
  void visit_controlTypeDeclaration(Ast* type_decl);
  void visit_controlLocalDeclarations(Ast* local_decls);
  void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

  void visit_externDeclaration(Ast* extern_decl);
  void visit_externTypeDeclaration(Ast* type_decl);
  void visit_methodPrototypes(Ast* protos, NameDeclaration* name_decl);
  void visit_functionPrototype(Ast* func_proto);

/** TYPES **/

  void visit_typeRef(Ast* type_ref);
  void visit_tupleType(Ast* type);
  void visit_headerStackType(Ast* type_decl);
  void visit_baseTypeBoolean(Ast* bool_type);
  void visit_baseTypeInteger(Ast* int_type);
  void visit_baseTypeBit(Ast* bit_type);
  void visit_baseTypeVarbit(Ast* varbit_type);
  void visit_baseTypeString(Ast* str_type);
  void visit_baseTypeVoid(Ast* void_type);
  void visit_baseTypeError(Ast* error_type);
  void visit_integerTypeSize(Ast* type_size);
  void visit_realTypeArg(Ast* type_arg);
  void visit_typeArg(Ast* type_arg);
  void visit_typeArgumentList(Ast* arg_list);
  void visit_typeDeclaration(Ast* type_decl);
  void visit_derivedTypeDeclaration(Ast* type_decl);
  void visit_headerTypeDeclaration(Ast* header_decl);
  void visit_headerUnionDeclaration(Ast* union_decl);
  void visit_structTypeDeclaration(Ast* struct_decl);
  void visit_structFieldList(Ast* field_list, NameDeclaration* name_decl);
  void visit_structField(Ast* field);
  void visit_enumDeclaration(Ast* enum_decl);
  void visit_errorDeclaration(Ast* error_decl);
  void visit_matchKindDeclaration(Ast* match_decl);
  int  visit_identifierList(Ast* ident_list);
  void visit_specifiedIdentifierList(Ast* ident_list, NameDeclaration* name_decl);
  void visit_specifiedIdentifier(Ast* ident);
  void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

  void visit_assignmentStatement(Ast* assign_stmt);
  void visit_functionCall(Ast* func_call);
  void visit_returnStatement(Ast* return_stmt);
  void visit_exitStatement(Ast* exit_stmt);
  void visit_conditionalStatement(Ast* cond_stmt);
  void visit_directApplication(Ast* applic_stmt);
  void visit_statement(Ast* stmt);
  void visit_blockStatement(Ast* block_stmt);
  void visit_statementOrDeclList(Ast* stmt_list);
  void visit_switchStatement(Ast* switch_stmt);
  void visit_switchCases(Ast* switch_cases);
  void visit_switchCase(Ast* switch_case);
  void visit_switchLabel(Ast* label);
  void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

  void visit_tableDeclaration(Ast* table_decl);
  void visit_tablePropertyList(Ast* prop_list);
  void visit_tableProperty(Ast* table_prop);
  void visit_keyProperty(Ast* key_prop);
  void visit_keyElementList(Ast* element_list);
  void visit_keyElement(Ast* element);
  void visit_actionsProperty(Ast* actions_prop);
  void visit_actionList(Ast* action_list);
  void visit_actionRef(Ast* action_ref);
  void visit_entriesProperty(Ast* entries_prop);
  void visit_entriesList(Ast* entries_list);
  void visit_entry(Ast* entry);
  void visit_simpleProperty(Ast* simple_prop);
  void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

  void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

  void visit_functionDeclaration(Ast* func_decl);
  void visit_argumentList(Ast* arg_list);
  void visit_argument(Ast* arg);
  void visit_expressionList(Ast* expr_list);
  void visit_lvalueExpression(Ast* lvalue_expr);
  void visit_expression(Ast* expr);
  void visit_castExpression(Ast* cast_expr);
  void visit_unaryExpression(Ast* unary_expr);
  void visit_binaryExpression(Ast* binary_expr);
  void visit_memberSelector(Ast* selector);
  void visit_arraySubscript(Ast* subscript);
  void visit_indexExpression(Ast* index_expr);
  void visit_booleanLiteral(Ast* bool_literal);
  void visit_integerLiteral(Ast* int_literal);
  void visit_stringLiteral(Ast* str_literal);
  void visit_default(Ast* default_);
  void visit_dontcare(Ast* dontcare);

  void name_bind();
};

enum class TypeEnum : int {
  NONE = 0,
  VOID,
  BOOL,
  INT,
  BIT,
  VARBIT,
  STRING,
  ANY,
  ENUM,
  TYPEDEF,
  FUNCTION,
  EXTERN,
  PACKAGE,
  PARSER,
  CONTROL,
  TABLE,
  STRUCT,
  HEADER,
  UNION,
  STACK,
  STATE,
  FIELD,
  ERROR,
  MATCH_KIND,
  NAMEREF,
  TYPE,
  TUPLE,
  PRODUCT,
};
char* TypeEnum_to_string(enum TypeEnum type);

struct Type {
  enum TypeEnum ty_former;
  char* strname;
  Ast* ast;

  union {
    struct {
      int size;
    } basic;

    struct {
      struct Type* ref;
    } typedef_;

    struct {
      struct Type* fields;
      int field_count;
      int i;
    } struct_, enum_;

    struct {
      struct Type* params;
      struct Type* return_;
    } function;

    struct {
      struct Type* methods;
      struct Type* ctors;
    } extern_;

    struct {
      struct Type* params;
      struct Type* ctor_params;
      struct Type* methods;
    } parser, control;

    struct {
      struct Type* methods;
    } table;

    struct {
      struct Type* params;
    } package;

    struct {
      struct Type* element;
      int size;
    } header_stack;

    struct {
      struct Type* type;
    } field;

    struct {
      Ast* name;
      struct Scope* scope;
    } nameref;

    struct {
      struct Type* type;
    } type;

    struct {
      struct Type* left;
      struct Type* right;
    } tuple; /* 2-tuple */

    struct {
      struct Type** members;
      int count;
    } product;
  };

  Type* actual_type();
  Type* effective_type();
};

enum class PotentialTypeEnum : int {
  NONE = 0,
  SET,
  PRODUCT,
};

struct PotentialType {
  enum PotentialTypeEnum kind;

  union {
    struct {
      Map members;
    } set;

    struct {
      struct PotentialType** members;
      int count;
    } product;
  };
};

struct NameDeclaration {
  char* strname;
  struct NameDeclaration* next_in_scope;

  union {
    Ast* ast;
    enum TokenClass token_class;
  };

  Type* type;
};

struct NameEntry {
  NameDeclaration* ns[NameSpace_COUNT];
};

struct TypeChecker {
  Arena* storage;
  Ast* p4program;
  char* source_file;
  Scope* root_scope;
  Map* scope_map;
  Map* decl_map;
  Array* type_array;
  Array* type_equiv_pairs;
  Map* type_env;
  Map* potype_map;

  bool match_type(PotentialType* potential_types, Type* required_ty);
  bool match_params(PotentialType* potential_args, Type* params_ty);
  void collect_matching_member(PotentialType* tau, Type* product_ty,
        char* strname, PotentialType* potential_args);
};

bool type_equiv(TypeChecker* checker, Type* u, Type* v);
bool match_type(TypeChecker* checker, PotentialType* potential_types, Type* required_ty);
bool match_params(TypeChecker* checker, PotentialType* potential_args, Type* params_ty);

struct DeclaredTypesPass : TypeChecker {

/** PROGRAM **/

  void visit_p4program(Ast* p4program);
  void visit_declarationList(Ast* decl_list);
  void visit_declaration(Ast* decl);
  void visit_name(Ast* name);
  void visit_parameterList(Ast* params);
  void visit_parameter(Ast* param);
  void visit_packageTypeDeclaration(Ast* package_decl);
  void visit_instantiation(Ast* inst);

/** PARSER **/

  void visit_parserDeclaration(Ast* parser_decl);
  void visit_parserTypeDeclaration(Ast* type_decl);
  void visit_parserLocalElements(Ast* local_elements);
  void visit_parserLocalElement(Ast* local_element);
  void visit_parserStates(Ast* states);
  void visit_parserState(Ast* state);
  void visit_parserStatements(Ast* stmts);
  void visit_parserStatement(Ast* stmt);
  void visit_parserBlockStatement(Ast* block_stmt);
  void visit_transitionStatement(Ast* transition_stmt);
  void visit_stateExpression(Ast* state_expr);
  void visit_selectExpression(Ast* select_expr);
  void visit_selectCaseList(Ast* case_list);
  void visit_selectCase(Ast* select_case);
  void visit_keysetExpression(Ast* keyset_expr);
  void visit_tupleKeysetExpression(Ast* tuple_expr);
  void visit_simpleKeysetExpression(Ast* simple_expr);
  void visit_simpleExpressionList(Ast* expr_list);

/** CONTROL **/

  void visit_controlDeclaration(Ast* control_decl);
  void visit_controlTypeDeclaration(Ast* type_decl);
  void visit_controlLocalDeclarations(Ast* local_decls);
  void visit_controlLocalDeclaration(Ast* local_decl);

/** EXTERN **/

  void visit_externDeclaration(Ast* extern_decl);
  void visit_externTypeDeclaration(Ast* type_decl);
  void visit_methodPrototypes(Ast* protos, Type* ctor_ty, char* ctor_strname);
  void visit_functionPrototype(Ast* func_proto, Type* ctor_ty, char* ctor_strname);

/** TYPES **/

  void visit_typeRef(Ast* type_ref);
  void visit_tupleType(Ast* type);
  void visit_headerStackType(Ast* type_decl);
  void visit_baseTypeBoolean(Ast* bool_type);
  void visit_baseTypeInteger(Ast* int_type);
  void visit_baseTypeBit(Ast* bit_type);
  void visit_baseTypeVarbit(Ast* varbit_type);
  void visit_baseTypeString(Ast* str_type);
  void visit_baseTypeVoid(Ast* void_type);
  void visit_baseTypeError(Ast* error_type);
  void visit_integerTypeSize(Ast* type_size);
  void visit_realTypeArg(Ast* type_arg);
  void visit_typeArg(Ast* type_arg);
  void visit_typeArgumentList(Ast* args);
  void visit_typeDeclaration(Ast* type_decl);
  void visit_derivedTypeDeclaration(Ast* type_decl);
  void visit_headerTypeDeclaration(Ast* header_decl);
  void visit_headerUnionDeclaration(Ast* union_decl);
  void visit_structTypeDeclaration(Ast* struct_decl);
  void visit_structFieldList(Ast* fields);
  void visit_structField(Ast* field);
  void visit_enumDeclaration(Ast* enum_decl);
  void visit_errorDeclaration(Ast* error_decl);
  void visit_matchKindDeclaration(Ast* match_decl);
  void visit_identifierList(Ast* ident_list, Type* enum_ty, Type* idents_ty, int* i);
  void visit_specifiedIdentifierList(Ast* ident_list, Type* enum_ty);
  void visit_specifiedIdentifier(Ast* ident, Type* enum_ty);
  void visit_typedefDeclaration(Ast* typedef_decl);

/** STATEMENTS **/

  void visit_assignmentStatement(Ast* assign_stmt);
  void visit_functionCall(Ast* func_call);
  void visit_returnStatement(Ast* return_stmt);
  void visit_exitStatement(Ast* exit_stmt);
  void visit_conditionalStatement(Ast* cond_stmt);
  void visit_directApplication(Ast* applic_stmt);
  void visit_statement(Ast* stmt);
  void visit_blockStatement(Ast* block_stmt);
  void visit_statementOrDeclList(Ast* stmt_list);
  void visit_switchStatement(Ast* switch_stmt);
  void visit_switchCases(Ast* switch_cases);
  void visit_switchCase(Ast* switch_case);
  void visit_switchLabel(Ast* label);
  void visit_statementOrDeclaration(Ast* stmt);

/** TABLES **/

  void visit_tableDeclaration(Ast* table_decl);
  void visit_tablePropertyList(Ast* prop_list);
  void visit_tableProperty(Ast* table_prop);
  void visit_keyProperty(Ast* key_prop);
  void visit_keyElementList(Ast* element_list);
  void visit_keyElement(Ast* element);
  void visit_actionsProperty(Ast* actions_prop);
  void visit_actionList(Ast* action_list);
  void visit_actionRef(Ast* action_ref);
  void visit_entriesProperty(Ast* entries_prop);
  void visit_entriesList(Ast* entries_list);
  void visit_entry(Ast* entry);
  void visit_simpleProperty(Ast* simple_prop);
  void visit_actionDeclaration(Ast* action_decl);

/** VARIABLES **/

  void visit_variableDeclaration(Ast* var_decl);

/** EXPRESSIONS **/

  void visit_functionDeclaration(Ast* func_decl);
  void visit_argumentList(Ast* args);
  void visit_argument(Ast* arg);
  void visit_expressionList(Ast* expr_list);
  void visit_lvalueExpression(Ast* lvalue_expr);
  void visit_expression(Ast* expr);
  void visit_castExpression(Ast* cast_expr);
  void visit_unaryExpression(Ast* unary_expr);
  void visit_binaryExpression(Ast* binary_expr);
  void visit_memberSelector(Ast* selector);
  void visit_arraySubscript(Ast* subscript);
  void visit_indexExpression(Ast* index_expr);
  void visit_booleanLiteral(Ast* bool_literal);
  void visit_integerLiteral(Ast* int_literal);
  void visit_stringLiteral(Ast* str_literal);
  void visit_default(Ast* default_);
  void visit_dontcare(Ast* dontcare);

  void declared_types();
};