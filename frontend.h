typedef struct SourceText {
  char*  text;
  int    text_size;
  char*  filename;
} SourceText;

enum TokenClass {
  TK_NONE = 0,

  /* Operators and syntactic structure */

  TK_SEMICOLON,
  TK_IDENTIFIER,
  TK_TYPE_IDENTIFIER,
  TK_INTEGER_LITERAL,
  TK_STRING_LITERAL,
  TK_PARENTH_OPEN,
  TK_PARENTH_CLOSE,
  TK_ANGLE_OPEN,
  TK_ANGLE_CLOSE,
  TK_BRACE_OPEN,
  TK_BRACE_CLOSE,
  TK_BRACKET_OPEN,
  TK_BRACKET_CLOSE,
  TK_DONTCARE,
  TK_COLON,
  TK_DOT,
  TK_COMMA,
  TK_MINUS,
  TK_UNARY_MINUS,
  TK_PLUS,
  TK_STAR,
  TK_SLASH,
  TK_EQUAL,
  TK_DOUBLE_EQUAL,
  TK_EXCLAMATION_EQUAL,
  TK_EXCLAMATION,
  TK_DOUBLE_PIPE,
  TK_ANGLE_OPEN_EQUAL,
  TK_ANGLE_CLOSE_EQUAL,
  TK_TILDA,
  TK_AMPERSAND,
  TK_DOUBLE_AMPERSAND,
  TK_TRIPLE_AMPERSAND,
  TK_PIPE,
  TK_CIRCUMFLEX,
  TK_DOUBLE_ANGLE_OPEN,
  TK_DOUBLE_ANGLE_CLOSE,
  TK_COMMENT,

  /* Keywords */

  TK_ACTION,
  TK_ACTIONS,
  TK_ENUM,
  TK_IN,
  TK_PACKAGE,
  TK_SELECT,
  TK_SWITCH,
  TK_TUPLE,
  TK_VOID,
  TK_APPLY,
  TK_CONTROL,
  TK_ERROR,
  TK_HEADER,
  TK_INOUT,
  TK_PARSER,
  TK_STATE,
  TK_TABLE,
  TK_ENTRIES,
  TK_KEY,
  TK_TYPEDEF,
  TK_BOOL,
  TK_TRUE,
  TK_FALSE,
  TK_DEFAULT,
  TK_EXTERN,
  TK_HEADER_UNION,
  TK_INT,
  TK_BIT,
  TK_VARBIT,
  TK_STRING,
  TK_OUT,
  TK_TRANSITION,
  TK_ELSE,
  TK_EXIT,
  TK_IF,
  TK_MATCH_KIND,
  TK_RETURN,
  TK_STRUCT,
  TK_CONST,

  /* Control */

  TK_UNKNOWN,
  TK_START_OF_INPUT,
  TK_END_OF_INPUT,
  TK_LEXICAL_ERROR,
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

enum AstEnum {
  AST_none = 0,

  /** PROGRAM **/

  AST_p4program,
  AST_declarationList,
  AST_declaration,
  AST_name,
  AST_parameterList,
  AST_parameter,
  AST_paramDirection,
  AST_packageTypeDeclaration,
  AST_instantiation,

  /** PARSER **/

  AST_parserDeclaration,
  AST_parserTypeDeclaration,
  AST_parserLocalElements,
  AST_parserLocalElement,
  AST_parserStates,
  AST_parserState,
  AST_parserStatements,
  AST_parserStatement,
  AST_parserBlockStatement,
  AST_transitionStatement,
  AST_stateExpression,
  AST_selectExpression,
  AST_selectCaseList,
  AST_selectCase,
  AST_keysetExpression,
  AST_tupleKeysetExpression,
  AST_simpleKeysetExpression,
  AST_simpleExpressionList,

  /** CONTROL **/

  AST_controlDeclaration,
  AST_controlTypeDeclaration,
  AST_controlLocalDeclarations,
  AST_controlLocalDeclaration,

  /** EXTERN **/

  AST_externDeclaration,
  AST_externTypeDeclaration,
  AST_methodPrototypes,
  AST_functionPrototype,

  /** TYPES **/

  AST_typeRef,
  AST_namedType,
  AST_tupleType,
  AST_headerStackType,
  AST_baseTypeBoolean,
  AST_baseTypeInteger,
  AST_baseTypeBit,
  AST_baseTypeVarbit,
  AST_baseTypeString,
  AST_baseTypeVoid,
  AST_baseTypeError,
  AST_integerTypeSize,
  AST_realTypeArg,
  AST_typeArg,
  AST_typeArgumentList,
  AST_typeDeclaration,
  AST_derivedTypeDeclaration,
  AST_headerTypeDeclaration,
  AST_headerUnionDeclaration,
  AST_structTypeDeclaration,
  AST_structFieldList,
  AST_structField,
  AST_enumDeclaration,
  AST_errorDeclaration,
  AST_matchKindDeclaration,
  AST_identifierList,
  AST_specifiedIdentifierList,
  AST_specifiedIdentifier,
  AST_typedefDeclaration,

  /** STATEMENTS **/

  AST_assignmentStatement,
  AST_emptyStatement,
  AST_returnStatement,
  AST_exitStatement,
  AST_conditionalStatement,
  AST_directApplication,
  AST_statement,
  AST_blockStatement,
  AST_statementOrDeclaration,
  AST_statementOrDeclList,
  AST_switchStatement,
  AST_switchCases,
  AST_switchCase,
  AST_switchLabel,

  /** TABLES **/

  AST_tableDeclaration,
  AST_tablePropertyList,
  AST_tableProperty,
  AST_keyProperty,
  AST_keyElementList,
  AST_keyElement,
  AST_actionsProperty,
  AST_actionList,
  AST_actionRef,
  AST_entriesProperty,
  AST_entriesList,
  AST_entry,
  AST_simpleProperty,
  AST_actionDeclaration,

  /** VARIABLES **/

  AST_variableDeclaration,
  AST_constantDeclaration,

  /** EXPRESSIONS **/

  AST_functionDeclaration,
  AST_argumentList,
  AST_argument,
  AST_expressionList,
  AST_expression,
  AST_lvalueExpression,
  AST_binaryExpression,
  AST_unaryExpression,
  AST_functionCall,
  AST_memberSelector,
  AST_castExpression,
  AST_arraySubscript,
  AST_indexExpression,
  AST_integerLiteral,
  AST_booleanLiteral,
  AST_stringLiteral,
  AST_dontcare,
  AST_default,
};

enum AstOperator {
  OP_NONE = 0,

  /* Arithmetic */

  OP_ADD,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_NEG,

  /* Logical */

  OP_AND,
  OP_OR,
  OP_NOT,

  /* Relational */

  OP_EQ,
  OP_NEQ,
  OP_LESS,
  OP_GREAT,
  OP_LESS_EQ,
  OP_GREAT_EQ,

  /* Bitwise */

  OP_BITW_AND,
  OP_BITW_OR,
  OP_BITW_XOR,
  OP_BITW_NOT,
  OP_BITW_SHL,
  OP_BITW_SHR,

  OP_MASK,
};

enum AstParamDirection {
  PARAMDIR_NONE = 0,
  PARAMDIR_IN   = 1 << 1,
  PARAMDIR_OUT  = 1 << 2,
};

typedef struct Ast {
  enum AstEnum kind;
  int line_no;
  int column_no;
  struct Ast* right_sibling;

  union {

    /** PROGRAM **/

    struct {
      struct Ast* decl_list;
    } p4program;

    struct {
      struct Ast* first_child;
    } declarationList;

    struct {
      struct Ast* decl;
    } declaration;

    struct {
      char* strname;
    } name;

    struct {
      struct Ast* first_child;
    } parameterList;

    struct {
      enum AstParamDirection direction;
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
    } parserTypeDeclaration;

    struct {
      struct Ast* first_child;
    } parserLocalElements;

    struct {
      struct Ast* element;
    } parserLocalElement;

    struct {
      struct Ast* first_child;
    } parserStates;

    struct {
      struct Ast* name;
      struct Ast* stmt_list;
      struct Ast* transition_stmt;
    } parserState;

    struct {
      struct Ast* first_child;
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
      struct Ast* first_child;
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
      struct Ast* first_child;
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
    } controlTypeDeclaration;

    struct {
      struct Ast* first_child;
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
      struct Ast* first_child;
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
      struct Ast* first_child;
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
      struct Ast* first_child;
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
      struct Ast* first_child;
    } identifierList;

    struct {
      struct Ast* first_child;
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
      struct Ast* first_child;
    } statementOrDeclList;

    struct {
      struct Ast* expr;
      struct Ast* switch_cases;
    } switchStatement;

    struct {
      struct Ast* first_child;
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
    } tableDeclaration;

    struct {
      struct Ast* first_child;
    } tablePropertyList;

    struct {
      struct Ast* prop;
    } tableProperty;

    struct {
      struct Ast* keyelem_list;
    } keyProperty;

    struct {
      struct Ast* first_child;
    } keyElementList;

    struct {
      struct Ast* expr;
      struct Ast* match;
    } keyElement;

    struct {
      struct Ast* action_list;
    } actionsProperty;

    struct {
      struct Ast* first_child;
    } actionList;

    struct {
      struct Ast* name;
      struct Ast* args;
    } actionRef;

    struct {
      struct Ast* entries_list;
    } entriesProperty;

    struct {
      struct Ast* first_child;
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
      struct Ast* first_child;
    } argumentList;

    struct {
      struct Ast* arg;
    } argument;

    struct {
      struct Ast* first_child;
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
      struct Ast* operand;
    } unaryExpression;

    struct {
      enum AstOperator op;
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
} Ast;

char* AstEnum_to_string(enum AstEnum ast);

typedef struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Hashmap name_table;
} Scope;

enum TypeEnum {
  TYPE_NONE = 0,

  TYPE_VOID,
  TYPE_BOOL,
  TYPE_INT,
  TYPE_BIT,
  TYPE_VARBIT,
  TYPE_STRING,
  TYPE_ANY,
  TYPE_ENUM,
  TYPE_TYPEDEF,
  TYPE_FUNCTION,
  TYPE_EXTERN,
  TYPE_PACKAGE,
  TYPE_PARSER,
  TYPE_CONTROL,
  TYPE_TABLE,
  TYPE_STRUCT,
  TYPE_HEADER,
  TYPE_HEADER_UNION,
  TYPE_HEADER_STACK,
  TYPE_STATE,
  TYPE_FIELD,
  TYPE_ERROR,
  TYPE_MATCH_KIND,
  TYPE_NAMEREF,
  TYPE_TYPE,
  TYPE_TUPLE,
  TYPE_PRODUCT,
};

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
    } parser, control, package;

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
} Type;

typedef struct PotentialType {
  union {
    Map members;

    struct {
      struct PotentialType** members;
      int count;
    } product;
  };
} PotentialType;

Type*  actual_type(Type* type);
Type*  effective_type(Type* type);
bool   type_equiv(Type* u, Type* v);
char*  TypeEnum_to_string(enum TypeEnum type);
bool   match_type(PotentialType* tau, Type* required_ty);
bool   match_function_args(Type* func_ty, PotentialType* potential_args);

typedef struct NameDeclaration {
  char* strname;
  struct NameDeclaration* next_in_scope;
  
  union {
    Ast* ast;
    enum TokenClass token_class;
  };

  Type* type;
} NameDeclaration;

enum NameSpace {
  NAMESPACE_VAR     = 1 << 0,
  NAMESPACE_TYPE    = 1 << 1,
  NAMESPACE_KEYWORD = 1 << 2,
};
#define NameSpace_COUNT 3

typedef struct NameEntry {
  NameDeclaration* ns[NameSpace_COUNT];
} NameEntry;

char* NameSpace_to_string(enum NameSpace ns);

Scope*     scope_create(Arena* storage, int segment_count);
Scope*     scope_push(Scope* scope, Scope* parent_scope);
Scope*     scope_pop(Scope* scope);
NameEntry* scope_lookup(Scope* scope, char* name, enum NameSpace ns);
NameEntry* scope_lookup_current(Scope* scope, char* strname);
NameDeclaration* scope_bind(Arena* storage, Scope* scope, char* strname, enum NameSpace ns);
NameDeclaration* builtin_lookup(Scope* scope, char* strname, enum NameSpace ns);
