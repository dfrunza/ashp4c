#pragma once
#include "basic.h"
#include "arena.h"

struct Ast;

enum TokenClass
{
  TOK_NONE,
  TOK_SEMICOLON,
  TOK_IDENT,
  TOK_TYPE_IDENT,
  TOK_STRING,
  TOK_INTEGER,
  TOK_WINTEGER,
  TOK_SINTEGER,
  TOK_INTEGER_HEX,
  TOK_WINTEGER_HEX,
  TOK_SINTEGER_HEX,
  TOK_INTEGER_OCT,
  TOK_WINTEGER_OCT,
  TOK_SINTEGER_OCT,
  TOK_INTEGER_BIN,
  TOK_WINTEGER_BIN,
  TOK_SINTEGER_BIN,
  TOK_PARENTH_OPEN,
  TOK_PARENTH_CLOSE,
  TOK_ANGLE_OPEN,
  TOK_ANGLE_CLOSE,
  TOK_BRACE_OPEN,
  TOK_BRACE_CLOSE,
  TOK_DONTCARE,
  TOK_COLON,
  TOK_PERIOD,
  TOK_COMMA,
  TOK_MINUS,
  TOK_PLUS,
  TOK_STAR,
  TOK_SLASH,
  TOK_EQUAL,
  TOK_EQUAL_EQUAL,
  TOK_COMMENT,

  TOK_KW_ACTION,
  TOK_KW_CONST,
  TOK_KW_ENUM,
  TOK_KW_IN,
  TOK_KW_PACKAGE,
  TOK_KW_SELECT,
  TOK_KW_SWITCH,
  TOK_KW_TUPLE,
  TOK_KW_VOID,
  TOK_KW_APPLY,
  TOK_KW_CONTROL,
  TOK_KW_ERROR,
  TOK_KW_HEADER,
  TOK_KW_INOUT,
  TOK_KW_PARSER,
  TOK_KW_STATE,
  TOK_KW_TABLE,
  TOK_KW_TYPEDEF,
  TOK_KW_BIT,
  TOK_KW_DEFAULT,
  TOK_KW_EXTERN,
  TOK_KW_HEADER_UNION,
  TOK_KW_INT,
  TOK_KW_OUT,
  TOK_KW_STRING,
  TOK_KW_TRANSITION,
  TOK_KW_VARBIT,
  TOK_KW_ELSE,
  TOK_KW_EXIT,
  TOK_KW_IF,
  TOK_KW_MATCH_KIND,
  TOK_KW_RETURN,
  TOK_KW_STRUCT,
  //TOK_KW_BOOL,
  //TOK_KW_FALSE,
  //TOK_KW_TRUE,
  //TOK_KW_VERIFY,

  TOK_UNKNOWN,
  TOK_SOI,    // Start Of Input
  TOK_EOI,    // End Of Input
};

enum IdentKind
{
  ID_NONE,
  ID_KEYWORD,
  ID_TYPE,
  ID_TYPEVAR,
  ID_VAR,
  ID_STRUCT_MEMBER,
};

typedef struct Ident
{
  enum IdentKind object_kind;
  char* name;
  struct Ast* ast;
  int scope_level;
  struct Ident* next_in_scope;
}
Ident;

typedef struct Ident_MemberSelector
{
  Ident;
  struct Ident_MemberSelector* next_selector;
}
Ident_MemberSelector;

typedef struct
{
  Ident;
  Ident_MemberSelector* selector;
}
Ident_Type;

typedef struct
{
  Ident;
  enum TokenClass token_klass;
}
Ident_Keyword;

typedef struct
{
  Ident;
  Ident_Type* type_ident;
}
Ident_Var;

typedef struct Namespace_Entry
{
  char* name;
  Ident* ns_global;
  Ident* ns_type;
  struct Namespace_Entry* next;
}
Namespace_Entry;

typedef struct
{
  enum TokenClass klass;
  char* lexeme;
  int line_nr;
  struct Ident* ident;
}
Token;

enum Typexpr_TypeCtor
{
  TYP_NONE,
  TYP_BASIC,
  TYP_TYPE_PARAMETER,
  TYP_PARAMETER,
  TYP_FUNCTION,
  TYP_ENUM,
  TYP_ENUM_FIELD,
  TYP_PARSER,
  TYP_CONTROL,
  TYP_PACKAGE,
  TYP_TYPEDEF,
  TYP_HEADER,
  TYP_STRUCT,
  TYP_EXTERN_OBJECT,
};

enum TypeBasic_Kind
{
  BASTYP_NONE,
  BASTYP_INT,
  BASTYP_VOID,
  BASTYP_BOOL,
  BASTYP_BIT,
  BASTYP_VARBIT,
  BASTYP_STRING,
};

typedef struct Typexpr 
{
  enum Typexpr_TypeCtor kind;
  bool is_prototype;
  char* name;
  struct Ast* ast;
}
Typexpr;

typedef struct Typexpr_Basic
{
  Typexpr;
  enum TypeBasic_Kind basic_type;
}
Typexpr_Basic;

typedef struct Typexpr_EnumField
{
  Typexpr;
  struct Typexpr_EnumField* next_field;
}
Typexpr_EnumField;  // TYP_ENUM_FIELD

typedef struct Typexpr_EnumType
{
  Typexpr;
  Typexpr_EnumField* sentinel_field;
  Typexpr_EnumField* last_field;
  int field_count;
}
Typexpr_EnumType;  // TYP_ENUM

typedef struct Typexpr_TypeParameter
{
  Typexpr;
  struct Typexpr_TypeParameter* next_type_parameter;
}
Typexpr_TypeParameter;

typedef struct Typexpr_Parameter
{
  Typexpr;
}
Typexpr_Parameter;

typedef struct Typexpr_Function
{
  Typexpr;
  struct Typexpr_Function* next_function;  // TYP_FUNCTION

  Typexpr_TypeParameter* sentinel_type_parameter;
  Typexpr_TypeParameter* last_type_parameter;
  int type_parameter_count;

  Typexpr_Parameter* sentinel_parameter;
  Typexpr_Parameter* last_parameter;
  int parameter_count;

  Typexpr* return_type;
}
Typexpr_Function;

typedef struct Typexpr_ExternObject
{
  Typexpr;
  Typexpr_Function* sentinel_function;
  Typexpr_Function* last_function;
  int method_count;
}
Typexpr_ExternObject;  // TYP_EXTERN_OBJECT

enum AstKind
{
  AST_NONE,
  AST_P4PROGRAM,
  AST_DECLARATION,
  AST_ERROR_TYPE,
  AST_STRUCT_PROTOTYPE,
  AST_STRUCT_DECL,
  AST_HEADER_PROTOTYPE,
  AST_HEADER_DECL,
  AST_TYPEDEF,
  AST_TYPEREF,
  AST_IDENT_TYPEREF,
  AST_BIT_TYPEREF,
  AST_INT_TYPEREF,
  AST_STRUCT_FIELD,
  AST_ERROR_CODE,
  AST_IDENT,
  AST_PARAMETER,
  AST_TYPE_PARAMETER,
  AST_PARSER_PROTOTYPE,
  AST_PARSER_DECL,
  AST_PARSER_STATE,
  AST_IDENT_EXPR,
  AST_INTEGER_EXPR,
  AST_WINTEGER_EXPR,
  AST_SINTEGER_EXPR,
  AST_ERROR_EXPR,
  AST_BINARY_EXPR,
  AST_FUNCTION_CALL,
  AST_IDENT_STATE,
  AST_SELECT_STATE,
  AST_SELECT_CASE,
  AST_EXPR_SELECT_CASE,
  AST_DEFAULT_SELECT_CASE,
  AST_TRANSITION_STMT,
  AST_CONTROL_PROTOTYPE,
  AST_CONTROL_DECL,
  AST_BLOCK_STMT,
  AST_EXPR_STMT,
  AST_BOOL,
  AST_ACTION,
  AST_TABLE,
  AST_TABLE_KEY,
  AST_ACTION_REF,
  AST_SIMPLE_PROP,
  AST_VAR_DECL,
  AST_PACKAGE_PROTOTYPE,
  AST_PACKAGE_INSTANCE,
  AST_EXTERN_OBJECT_PROTOTYPE,
  AST_EXTERN_FUNCTION_PROTOTYPE,
  AST_FUNCTION_PROTOTYPE,
};

enum AstDirection
{
  DIR_NONE,
  DIR_IN,
  DIR_OUT,
  DIR_INOUT,
};

enum AstExprOperator
{
  OP_NONE,
  OP_MEMBER_SELECTOR,
  OP_LOGIC_EQUAL,
  OP_FUNCTION_CALL,
  OP_ASSIGN,
  OP_ADDITION,
  OP_SUBTRACT,
};

enum AstTypeParameterKind
{
  TYPPARAM_NONE,
  TYPPARAM_VAR,
  TYPPARAM_INT,
};

typedef struct Ast
{
  enum AstKind kind;
  int line_nr;
  Typexpr* typexpr;
  bool is_builtin;
}
Ast;

typedef struct Ast_Declaration
{
  Ast;
  struct Ast_Declaration* next_decl;
}
Ast_Declaration;

typedef struct Ast_Ident
{
  Ast;
  char* name;
  struct Ast_Ident* next_ident;
}
Ast_Ident;  // AST_IDENT

typedef struct
{
  Ast_Ident;
  Ident_MemberSelector* selector;
}
Ast_ErrorCode;  // AST_ERROR_CODE

typedef struct
{
  Ast;
  char* name;
}
Ast_Typeref;  // AST_TYPEREF

typedef struct
{
  Ast_Typeref;
  int size;
}
Ast_BitTyperef;  // AST_BIT_TYPEREF

typedef struct
{
  Ast_Typeref;
  int size;
}
Ast_IntTyperef;  // AST_INT_TYPEREF

typedef struct
{
  Ast_Declaration;
  Ast_Typeref* typeref;
  char* name;
  Ident_Type* id_info;
}
Ast_Typedef;  // AST_TYPEDEF

typedef struct Ast_StructField
{
  Ast_Ident;
  Ast_Typeref* typeref;
  Ident_MemberSelector* selector;
}
Ast_StructField;  // AST_STRUCT_FIELD

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_StructField* field;
  Ident_Type* id_info;
}
Ast_StructDecl;  // AST_STRUCT_PROTOTYPE
                 // AST_STRUCT_DECL

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_StructField* field;
  Ident_Type* id_info;
}
Ast_HeaderDecl;  // AST_HEADER_PROTOTYPE
                 // AST_HEADER_DECL

typedef struct
{
  Ast_Declaration;
  Ast_ErrorCode* error_code;
  Ident_Type* id_info;
}
Ast_ErrorType;  // AST_ERROR_TYPE

typedef struct Ast_Parameter
{
  Ast;
  enum AstDirection direction;
  Ast_Typeref* typeref;
  char* name;
  struct Ast_Parameter* next_param;
}
Ast_Parameter;  // AST_PARAMETER

typedef struct Ast_TypeParameter
{
  Ast;
  enum AstTypeParameterKind parameter_kind;
  struct Ast_TypeParameter* next_param;
  union
  {
    char* var_name;
    char* int_value;
  };
}
Ast_TypeParameter;  // AST_TYPE_PARAMETER

typedef struct Ast_Expression
{
  Ast;
  struct Ast_Expression* next;
}
Ast_Expression;

typedef struct
{
  Ast_Expression;
  Ast_Expression* argument;
}
Ast_FunctionCallExpr;  // AST_FUNCTION_CALL

typedef struct Ast_BinaryExpr
{
  Ast_Expression;
  enum AstExprOperator op;
  struct Ast_Expression* l_operand;
  struct Ast_Expression* r_operand;
}
Ast_BinaryExpr;  // AST_BINARY_EXPR

typedef struct
{
  Ast_Expression;
  char* name;
}
Ast_IdentExpr;  // AST_IDENT_EXPR

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_IntegerExpr;  // AST_INTEGER_EXPR

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_WIntegerExpr;  // AST_WINTEGER_EXPR

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_SIntegerExpr;  // AST_SINTEGER_EXPR

typedef struct
{
  Ast_Expression;
}
Ast_ErrorExpr;  // AST_ERROR_EXPR

typedef struct
{
  Ast;
}
Ast_StateExpr;

typedef struct
{
  Ast_Declaration;
  char* typename;
  char* name;
  Ast_Expression* initializer;
}
Ast_VarDecl;  // AST_VAR_DECL

typedef struct
{
  Ast_StateExpr;
  char* name;
}
Ast_IdentState;  // AST_IDENT_STATE

typedef struct Ast_SelectCase
{
  Ast;
  char* end_state;
  struct Ast_SelectCase* next;
}
Ast_SelectCase;  // AST_SELECT_CASE

typedef struct
{
  Ast_SelectCase;
  Ast_Expression* key_expr;
}
Ast_ExprSelectCase;  // AST_EXPR_SELECT_CASE

typedef struct
{
  Ast_SelectCase;
}
Ast_DefaultSelectCase;  // AST_DEFAULT_SELECT_CASE

typedef struct
{
  Ast_StateExpr;
  Ast_Expression* expression;
  Ast_SelectCase* select_case;
}
Ast_SelectState;  // AST_SELECT_STATE

typedef struct
{
  Ast;
  Ast_StateExpr* state_expr;
}
Ast_TransitionStmt;  // AST_TRANSITION_STMT

typedef struct Ast_ParserState
{
  Ast;
  char* name;
  Ast_Expression* statement;
  struct Ast_ParserState* next;
  Ast_TransitionStmt* transition_stmt;
}
Ast_ParserState;  // AST_PARSER_STATE

typedef struct
{
  Ast;
}
Ast_Statement;

typedef struct
{
  Ast_Statement;
  Ast_Expression* expression;
}
Ast_ExprStmt;  // AST_EXPR_STMT

typedef struct
{
  Ast_Statement;
  Ast_Expression* statement;
}
Ast_BlockStmt;  // AST_BLOCK_STMT

typedef struct
{
  Ast;
  bool value;
}
Ast_Bool;  // AST_BOOL

typedef struct Ast_TableProperty
{
  Ast;
  struct Ast_TableProperty* next;
}
Ast_TableProperty;

typedef struct Ast_Key
{
  Ast_TableProperty;
  Ast_Expression* expression;
  Ast_Expression* name;
  struct Ast_Key* next_key;
}
Ast_Key;  // AST_TABLE_KEY

typedef struct Ast_ActionRef
{
  Ast_TableProperty;
  char* name;
  Ast_Expression* argument;
  struct Ast_ActionRef* next_action;
}
Ast_ActionRef;  // AST_ACTION_REF

typedef struct Ast_SimpleProp
{
  Ast_TableProperty;
  Ast_Expression* expression;
}
Ast_SimpleProp;  // AST_SIMPLE_PROP

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_Parameter* parameter;
  Ast_BlockStmt* action_body;
}
Ast_ActionDecl;  // AST_ACTION

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_TableProperty* property;
}
Ast_TableDecl;  // AST_TABLE

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_TypeParameter* type_parameter;
  Ast_Parameter* parameter;
  Ident_Type* id_info;
  Ast_Declaration* local_decl;
  Ast_BlockStmt* control_body;
}
Ast_ControlDecl;  // AST_CONTROL_PROTOTYPE
                  // AST_CONTROL_DECL

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_TypeParameter* type_parameter;
  Ast_Parameter* parameter;
  Ident_Type* id_info;
  Ast_ParserState* parser_state;
}
Ast_ParserDecl;  // AST_PARSER_PROTOTYPE
                 // AST_PARSER_DECL

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_TypeParameter* type_parameter;
  Ast_Parameter* parameter;
  Ident_Type* id_info;
}
Ast_PackageDecl;  // AST_PACKAGE_PROTOTYPE

typedef struct
{
  Ast_Declaration;
  Ast_Expression* package;
  char* name;
}
Ast_PackageInstance;  // AST_PACKAGE_INSTANCE

typedef struct Ast_FunctionPrototype
{
  Ast_Declaration;
  char* name;
  Ast_TypeParameter* first_type_parameter;
  Ast_Parameter* first_parameter;
  Ast* return_type;
}
Ast_FunctionDecl;  // AST_FUNCTION_PROTOTYPE
                   // AST_EXTERN_FUNCTION_PROTOTYPE

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_FunctionDecl* first_method;
  Ident_Type* id_info;
}
Ast_ExternObjectDecl;  // AST_EXTERN_OBJECT_PROTOTYPE

typedef struct
{
  Ast;
  Ast_Declaration* declaration;
}
Ast_P4Program;  // AST_P4PROGRAM

