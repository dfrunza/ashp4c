#pragma once

enum TokenClass {
  TK_SEMICOLON = 1,
  TK_IDENTIFIER,
  TK_TYPE_IDENTIFIER,
  TK_INT_LITERAL,
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
  TK_DOTPREFIX,
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
  TK_TYPE,
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

  /* Special */
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
    } i;  /* integer */
    char* str;
  };
} Token;

enum AstEnum {
  AST_NAME = 1,
  AST_DOTNAME = AST_NAME,
  AST_BOOL_TYPE,
  AST_INT_TYPE,
  AST_BIT_TYPE,
  AST_VARBIT_TYPE,
  AST_STRING_TYPE,
  AST_VOID_TYPE,
  AST_CONST,
  AST_EXTERN,
  AST_FUNCTION_PROTO,
  AST_ACTION,
  AST_HEADER,
  AST_HEADER_UNION,
  AST_STRUCT,
  AST_ENUM,
  AST_ENUM_FIELD,
  AST_TYPE,
  AST_TYPE_PARAM,
  AST_PARSER,
  AST_CONTROL,
  AST_PACKAGE,
  AST_INSTANTIATION,
  AST_ERROR_ENUM,
  AST_MATCH_KIND,
  AST_ERROR_TYPE,
  AST_FUNCTION,
  AST_DONTCARE,
  AST_INT_TYPESIZE,
  AST_INT_LITERAL,
  AST_BOOL_LITERAL,
  AST_STRING_LITERAL,
  AST_TUPLE,
  AST_TUPLE_KEYSET,
  AST_HEADER_STACK,
  AST_SPECIALIZED_TYPE,
  AST_SPECIFIED_IDENT,
  AST_STRUCT_FIELD,
  AST_PARSER_PROTO,
  AST_ARGUMENT,
  AST_VAR,
  AST_PARAM,
  AST_NODE_LIST,
  AST_ASSIGNMENT_STMT,
  AST_EMPTY_ELEMENT,
  AST_DEFAULT_STMT,
  AST_SELECT_CASE,
  AST_PARSER_STATE,
  AST_CONTROL_PROTO,
  AST_KEY_ELEMENT,
  AST_ACTION_REF,
  AST_TABLE,
  AST_TABLE_ENTRY,
  AST_TABLE_KEY,
  AST_TABLE_ACTIONS,
  AST_TABLE_ENTRIES,
  AST_TABLE_SINGLE_ENTRY,
  AST_IF_STMT,
  AST_EXIT_STMT,
  AST_RETURN_STMT,
  AST_SWITCH_STMT,
  AST_SWITCH_CASE,
  AST_BLOCK_STMT,
  AST_P4PROGRAM,
  AST_KVPAIR,
  AST_SELECT_EXPR,
  AST_EXPRESSION_LIST,
  AST_CAST_EXPR,
  AST_UNARY_EXPR,
  AST_BINARY_EXPR,
  AST_MEMBER_SELECT,
  AST_SUBSCRIPT,
  AST_FUNCTION_CALL,
};

enum AstOperator {
  OP_ADD = 1,
  OP_SUB,
  OP_MUL,
  OP_DIV,
  OP_AND,
  OP_OR,
  OP_NOT,
  OP_EQUAL,
  OP_NOT_EQUAL,
  OP_LESS,
  OP_GREATER,
  OP_LESS_EQUAL,
  OP_GREATER_EQUAL,
  OP_BITWISE_AND,
  OP_BITWISE_OR,
  OP_BITWISE_XOR,
  OP_BITWISE_NOT,
  OP_BITWISE_SHIFT_LEFT,
  OP_BITWISE_SHIFT_RIGHT,
  OP_MASK,
  OP_NEG,
};

enum AstParamDirection {
  PARAMDIR_IN = 1,
  PARAMDIR_OUT,
  PARAMDIR_INOUT,
};

typedef struct Ast {
  enum AstEnum kind;
  uint32_t id;
  int line_no;
  int column_no;
} Ast;

typedef struct Ast_NodeList {
  Ast;
  DList list;
  int node_count;
} Ast_NodeList;

typedef struct Ast_Expression {
  Ast;
  Ast_NodeList type_args;
} Ast_Expression;

typedef struct Ast_Name {
  Ast_Expression;
  char* strname;
} Ast_Name;

typedef struct Ast_BoolType {
  Ast;
  Ast* name;
} Ast_BoolType;

typedef struct Ast_ErrorType {
  Ast;
  Ast* name;
} Ast_ErrorType;

typedef struct Ast_IntType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_IntType;

typedef struct Ast_BitType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_BitType;

typedef struct Ast_VarbitType {
  Ast;
  Ast* name;
  Ast* size;
} Ast_VarbitType;

typedef struct Ast_StringType {
  Ast;
  Ast* name;
} Ast_StringType;

typedef struct Ast_VoidType {
  Ast;
  Ast* name;
} Ast_VoidType;

typedef struct Ast_Const {
  Ast;
  Ast* name;
  Ast* type;
  Ast* expr;
} Ast_Const;

typedef struct Ast_Extern {
  Ast;
  Ast* name;
  Ast_NodeList type_params;
  Ast_NodeList method_protos;
} Ast_Extern;

typedef struct Ast_FunctionProto {
  Ast;
  bool is_extern;
  Ast* return_type;
  Ast* name;
  Ast_NodeList type_params;
  Ast_NodeList params;
} Ast_FunctionProto;

typedef struct Ast_Action {
  Ast;
  Ast* name;
  Ast_NodeList params;
  Ast* stmt;
} Ast_Action;

typedef struct Ast_Header {
  Ast;
  Ast* name;
  Ast_NodeList fields;
} Ast_Header;

typedef struct Ast_HeaderUnion {
  Ast;
  Ast* name;
  Ast_NodeList fields;
} Ast_HeaderUnion;

typedef struct Ast_Struct {
  Ast;
  Ast* name;
  Ast_NodeList fields;
} Ast_Struct;

typedef struct Ast_Enum {
  Ast;
  Ast* name;
  Ast* type_size;
  Ast_NodeList id_list;
} Ast_Enum;

typedef struct Ast_Type {
  Ast;
  Ast* name;
  bool is_typedef;
  Ast* type_ref;
} Ast_Type;

typedef struct Ast_Parser {
  Ast;
  Ast* proto;
  Ast_NodeList ctor_params;
  Ast_NodeList local_elements;
  Ast_NodeList states;
} Ast_Parser;

typedef struct Ast_Control {
  Ast;
  Ast* proto;
  Ast_NodeList ctor_params;
  Ast_NodeList local_decls;
  Ast* apply_stmt;
} Ast_Control;

typedef struct Ast_Package {
  Ast;
  Ast* name;
  Ast_NodeList type_params;
  Ast_NodeList params;
} Ast_Package;

typedef struct Ast_Instantiation {
  Ast;
  Ast* name;
  Ast* type;
  Ast_NodeList args;
} Ast_Instantiation;

typedef struct Ast_ErrorEnum {
  Ast;
  Ast_NodeList id_list;
} Ast_ErrorEnum;

typedef struct Ast_MatchKind {
  Ast;
  Ast_NodeList id_list;
} Ast_MatchKind;

typedef struct Ast_Function {
  Ast;
  Ast* proto;
  Ast* stmt;
} Ast_Function;

typedef struct Ast_IntTypeSize {
  Ast;
  Ast* size;
} Ast_IntTypeSize;

typedef struct Ast_IntLiteral {
  Ast_Expression;
  bool is_signed;
  int value;
  int width;
} Ast_IntLiteral;

typedef struct Ast_BoolLiteral {
  Ast_Expression;
  bool value;
} Ast_BoolLiteral;

typedef struct Ast_StringLiteral {
  Ast_Expression;
  char* value;
} Ast_StringLiteral;

typedef struct Ast_Tuple {
  Ast;
  Ast_NodeList type_args;
} Ast_Tuple;

typedef struct Ast_TupleKeyset {
  Ast;
  Ast_NodeList expr_list;
} Ast_TupleKeyset;

typedef struct Ast_HeaderStack {
  Ast;
  Ast* name;
  Ast* stack_expr;
} Ast_HeaderStack;

typedef struct Ast_SpecializedType {
  Ast;
  Ast* name;
  Ast_NodeList type_args;
} Ast_SpecializedType;

typedef struct Ast_SpecifiedIdent {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_SpecifiedIdent;

typedef struct Ast_StructField {
  Ast;
  Ast* name;
  Ast* type;
} Ast_StructField;

typedef struct Ast_ParserProto {
  Ast;
  Ast* name;
  Ast_NodeList type_params;
  Ast_NodeList params;
} Ast_ParserProto;

typedef struct Ast_Argument {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_Argument;

typedef struct Ast_Var {
  Ast;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_Var;

typedef struct Ast_Param {
  Ast;
  enum AstParamDirection direction;
  Ast* name;
  Ast* type;
  Ast* init_expr;
} Ast_Param;

typedef struct Ast_AssignmentStmt {
  Ast;
  Ast* lvalue;
  Ast* expr;
} Ast_AssignmentStmt;

typedef struct Ast_SelectCase {
  Ast;
  Ast* name;
  Ast* keyset;
} Ast_SelectCase;

typedef struct Ast_ParserState {
  Ast;
  Ast* name;
  Ast_NodeList stmt_list;
  Ast* trans_stmt;
} Ast_ParserState;

typedef struct Ast_ControlProto {
  Ast;
  Ast* name;
  Ast_NodeList type_params;
  Ast_NodeList params;
} Ast_ControlProto;

typedef struct Ast_KeyElement {
  Ast;
  Ast* name;
  Ast* expr;
} Ast_KeyElement;

typedef struct Ast_ActionRef {
  Ast;
  Ast* name;
  Ast_NodeList args;
} Ast_ActionRef;

typedef struct Ast_TableEntry {
  Ast;
  Ast* keyset;
  Ast* action;
} Ast_TableEntry;

typedef struct Ast_TableKey {
  Ast;
  Ast_NodeList keyelem_list;
} Ast_TableKey;

typedef struct Ast_TableActions {
  Ast;
  Ast_NodeList action_list;
} Ast_TableActions;

typedef struct Ast_TableEntries {
  Ast;
  bool is_const;
  Ast_NodeList entries;
} Ast_TableEntries;

typedef struct Ast_TableSingleEntry {
  Ast;
  Ast* name;
  Ast* init_expr;
} Ast_TableSingleEntry;

typedef struct Ast_Table {
  Ast;
  Ast* name;
  Ast_NodeList prop_list;
} Ast_Table;

typedef struct Ast_IfStmt {
  Ast;
  Ast* cond_expr;
  Ast* stmt;
  Ast* else_stmt;
} Ast_IfStmt;

typedef struct Ast_ReturnStmt {
  Ast;
  Ast* expr;
} Ast_ReturnStmt;

typedef struct Ast_SwitchCase {
  Ast;
  Ast* label;
  Ast* stmt;
} Ast_SwitchCase;

typedef struct Ast_SwitchStmt {
  Ast;
  Ast* expr;
  Ast_NodeList switch_cases;
} Ast_SwitchStmt;

typedef struct Ast_BlockStmt {
  Ast;
  Ast_NodeList stmt_list;
} Ast_BlockStmt;

typedef struct Ast_KVPair {
  Ast_Expression;
  Ast* name;
  Ast* expr;
} Ast_KVPair;

typedef struct Ast_P4Program {
  Ast;
  Ast_NodeList decl_list;
  int last_node_id;
} Ast_P4Program;

typedef struct Ast_SelectExpr {
  Ast_Expression;
  Ast_NodeList expr_list;
  Ast_NodeList case_list;
} Ast_SelectExpr;

typedef struct Ast_ExpressionList {
  Ast_Expression;
  Ast_NodeList expr_list;
} Ast_ExpressionList;

typedef struct Ast_CastExpr {
  Ast_Expression;
  Ast* to_type;
  Ast* expr;
} Ast_CastExpr;

typedef struct Ast_UnaryExpr {
  Ast_Expression;
  enum AstOperator op;
  Ast* operand;
} Ast_UnaryExpr;

typedef struct Ast_BinaryExpr {
  Ast_Expression;
  enum AstOperator op;
  Ast* left_operand;
  Ast* right_operand;
} Ast_BinaryExpr;

typedef struct Ast_MemberSelect {
  Ast_Expression;
  Ast* lhs_expr;
  Ast* member_name;
} Ast_MemberSelect;

typedef struct Ast_Subscript {
  Ast_Expression;
  Ast* index;
  Ast* end_index;
} Ast_Subscript;

typedef struct Ast_FunctionCall {
  Ast_Expression;
  Ast* callee_expr;
  Ast_NodeList args;
} Ast_FunctionCall;

typedef struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  Hashmap decls;
} Scope;

typedef struct NameDecl {
  union {
    Ast* ast;
    enum TokenClass token_class;
  };
  char* strname;
  int line_no;
  int column_no;
  struct NameDecl* next_decl;
} NameDecl;

typedef struct NameRef {
  Ast* ast;
  char* strname;
  int line_no;
  int column_no;
  Scope* scope;
} NameRef;

typedef struct NameEntry {
  char* strname;
  NameDecl* ns_type;
  NameDecl* ns_var;
  NameDecl* ns_keyword;
} NameEntry;

void scope_init(Arena* scope_storage);
NameEntry* namedecl_get_or_create(Hashmap* decls, char* name);
NameEntry* namedecl_get(Hashmap* decls, char* name);
Scope* push_scope();
Scope* pop_scope();
NameEntry* scope_lookup_name(Scope* scope, char* name);
void declare_type_name(Scope* scope, Ast_Name* name, Ast* ast);
void declare_var_name(Scope* scope, Ast_Name* name, Ast* ast);
void declare_keyword(Scope* scope, char* strname, enum TokenClass token_class);
NameRef* nameref_get(Hashmap* map, uint32_t id);
void nameref_add(Hashmap* map, NameRef* nameref, uint32_t id);

enum TypeEnum {
  TYPE_VOID = 1,
  TYPE_BOOL,
  TYPE_INT,
  TYPE_BIT,
  TYPE_VARBIT,
  TYPE_STRING,
  TYPE_ERROR,
  TYPE_TYPESET,
  TYPE_TYPEDEF,
  TYPE_TYPENAME,
  TYPE_TYPEPARAM,
  TYPE_PRODUCT,
  TYPE_UNION,
  TYPE_FUNCTION,
  TYPE_FUNCTION_CALL,
};

typedef struct Type {
  enum TypeEnum ctor;
  Ast* ast;
  struct Type* type_params;
} Type;

typedef struct Type_TypeSet {
  Type;
  DList members;
  DList* last_member;
  int member_count;
} Type_TypeSet;

typedef struct Type_TypeDef {
  Type;
  char* strname;
} Type_TypeDef;

typedef struct Type_TypeName {
  Type;
  char* strname;
} Type_TypeName;

typedef struct Type_TypeParam {
  Type;
  char* strname;
} Type_TypeParam;

typedef struct Type_Product {
  Type;
  Type* lhs_ty;
  Type* rhs_ty;
} Type_Product;

typedef struct Type_Union {
  Type;
  Type* lhs_ty;
  Type* rhs_ty;
} Type_Union;

typedef struct Type_Function {
  Type;
  Type* params_ty;
  Type* return_ty;
} Type_Function;

typedef struct Type_FunctionCall {
  Type;
  Type* args_ty;
  Type* return_ty;
} Type_FunctionCall;

Type_TypeSet* typeset_create(Hashmap* map, uint32_t id);
Type_TypeSet* typeset_get(Hashmap* map, uint32_t id);
void typeset_add_type(Type_TypeSet* ty_set, Type* type);
void typeset_add_set(Type_TypeSet* to_set, Type_TypeSet* from_set);

