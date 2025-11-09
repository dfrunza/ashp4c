#pragma once
#include "foundation.h"
#include "ast_tree.h"

typedef struct SourceText {
  Arena* storage;
  char* text;
  int text_size;
  char* filename;
} SourceText;

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

typedef struct Token {
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
} Token;

typedef struct Lexeme {
  char* start;
  char* end;
} Lexeme;

typedef struct Lexer {
  Arena* storage;
  char*  text;
  int    text_size;
  char*  filename;
  int    line_no;
  char*  line_start;
  int    state;
  Lexeme lexeme[2];
  Array* tokens;
} Lexer;

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

enum class AstOperator : uint16_t {
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

enum class ParamDirection : uint8_t {
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

typedef struct Ast {
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
} Ast;

struct NameEntry;
struct NameDeclaration;

enum class NameSpace : uint8_t {
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

typedef struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Strmap name_table;

  static Scope* create(Arena* storage, int segment_count);
  Scope* push(Scope* parent_scope);
  Scope* pop();
  NameEntry* lookup(char* name, enum NameSpace ns);
  NameDeclaration* builtin_lookup(char* strname, enum NameSpace ns);
  NameDeclaration* bind(Arena* storage, char* strname, enum NameSpace ns);
} Scope;

typedef struct Parser {
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
} Parser;

typedef struct BuiltinMethodBuilder {
  Arena* storage;
} BuiltinMethodBuilder;

typedef struct ScopeBuilder {
  Arena* storage;
  Ast* p4program;
  Scope* root_scope;
  Scope* current_scope;
  Map* scope_map;
} ScopeBuilder;

typedef struct NameBinder {
  Arena* storage;
  Ast* p4program;
  Scope* root_scope;
  Scope* current_scope;
  Map* scope_map;
  Map* decl_map;
  Array* type_array;
} NameBinder;

enum class TypeEnum : uint16_t {
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

typedef struct Type {
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
} Type;

enum class PotentialTypeEnum : uint8_t {
  NONE = 0,
  SET,
  PRODUCT,
};

typedef struct PotentialType {
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
} PotentialType;

typedef struct TypeChecker {
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
} TypeChecker;

bool type_equiv(TypeChecker* checker, Type* u, Type* v);
bool match_type(TypeChecker* checker, PotentialType* potential_types, Type* required_ty);
bool match_params(TypeChecker* checker, PotentialType* potential_args, Type* params_ty);

typedef struct NameDeclaration {
  char* strname;
  struct NameDeclaration* next_in_scope;

  union {
    Ast* ast;
    enum TokenClass token_class;
  };

  Type* type;
} NameDeclaration;

typedef struct NameEntry {
  NameDeclaration* ns[NameSpace_COUNT];
} NameEntry;
