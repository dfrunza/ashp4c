#pragma once
#include "basic.h"
#include "arena.h"

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
  //TOK_KW_VAR,

  TOK_UNKNOWN,
  TOK_SOI,    // Start Of Input
  TOK_EOI,    // End Of Input
};

enum IdentObjectKind
{
  IDOBJ_NONE,
  IDOBJ_KEYWORD,
  IDOBJ_TYPE,
  IDOBJ_TYPEVAR,
  IDOBJ_VAR,
  IDOBJ_FIELD,
};

typedef struct IdentInfo
{
  enum IdentObjectKind object_kind;
  char* name;
  int scope_level;
  struct IdentInfo* next_in_scope;
}
IdentInfo;

typedef struct IdentInfo_Selector
{
  IdentInfo;
  struct IdentInfo_Selector* next_selector;
}
IdentInfo_Selector;

typedef struct
{
  IdentInfo;
  IdentInfo_Selector* selector;
}
IdentInfo_Type;

typedef struct
{
  IdentInfo;
  enum TokenClass token_klass;
}
IdentInfo_Keyword;

typedef struct
{
  IdentInfo;
  IdentInfo_Type* id_info;
}
IdentInfo_Var;

typedef struct SymbolTable_Entry
{
  char* name;
  IdentInfo* ns_global;
  IdentInfo* ns_type;
  struct SymbolTable_Entry* next;
}
SymbolTable_Entry;

typedef struct
{
  enum TokenClass klass;
  char* lexeme;
  int line_nr;
  struct IdentInfo* ident;
}
Token;

enum AstKind
{
  AST_NONE,
  AST_P4PROGRAM,
  AST_DECLARATION,
  AST_ERROR_TYPE,
  AST_STRUCT_TYPE_DECL,
  AST_STRUCT_TYPE,
  AST_HEADER_TYPE_DECL,
  AST_HEADER_TYPE,
  AST_TYPEDEF,
  AST_TYPEREF,
  AST_IDENT_TYPEREF,
  AST_BIT_TYPEREF,
  AST_INT_TYPEREF,
  AST_STRUCT_FIELD,
  AST_ERROR_CODE,
  AST_IDENT,
  AST_PARAMETER,
  AST_PARAMETER_LIST,
  AST_TYPE_PARAMETER,
  AST_TYPE_PARAMETER_LIST,
  AST_PARSER_TYPE_DECL,
  AST_PARSER_TYPE,
  AST_PARSER_STATE,
  AST_IDENT_EXPR,
  AST_INTEGER_EXPR,
  AST_WINTEGER_EXPR,
  AST_SINTEGER_EXPR,
  AST_ERROR_EXPR,
  AST_BINARY_EXPR,
  AST_FUNCTION_CALL,
  AST_SELECT_TRANS,
  AST_IDENT_STATE,
  AST_SELECT_STATE,
  AST_SELECT_CASE,
  AST_EXPR_SELECT_CASE,
  AST_DEFAULT_SELECT_CASE,
  AST_TRANSITION_STMT,
  AST_CONTROL_TYPE_DECL,
  AST_CONTROL_TYPE,
  AST_BLOCK_STMT,
  AST_EXPR_STMT,
  AST_BOOL,
  AST_ACTION,
  AST_TABLE,
  AST_TABLE_KEY,
  AST_ACTION_REF,
  AST_SIMPLE_PROP,
  AST_VAR_DECL,
  AST_PACKAGE_TYPE_DECL,
  AST_INSTANTIATION,
  AST_EXTERN_OBJECT_DECL,
  AST_EXTERN_FUNCTION_DECL,
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
  struct Ast_Ident* next_id;
}
Ast_Ident;

typedef struct
{
  Ast_Ident;
  IdentInfo_Selector* selector;
}
Ast_ErrorCode;

typedef struct
{
  Ast;
  char* name;
}
Ast_Typeref;

typedef struct
{
  Ast_Typeref;
  int size;
}
Ast_BitTyperef;

typedef struct
{
  Ast_Typeref;
  int size;
}
Ast_IntTyperef;

typedef struct
{
  Ast_Declaration;
  Ast_Typeref* typeref;
  char* name;
  IdentInfo_Type* id_info;
}
Ast_Typedef;

typedef struct Ast_StructField
{
  Ast_Ident;
  Ast_Typeref* typeref;
  IdentInfo_Selector* selector;
}
Ast_StructField;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_StructField* field;
  int field_count;
  IdentInfo_Type* id_info;
}
Ast_StructType;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_StructField* field;
  int field_count;
  IdentInfo_Type* id_info;
}
Ast_HeaderType;

typedef struct
{
  Ast_Declaration;
  Ast_ErrorCode* error_code;
  int code_count;
  IdentInfo_Type* id_info;
}
Ast_ErrorType;

typedef struct Ast_Parameter
{
  Ast;
  enum AstDirection direction;
  Ast_Typeref* typeref;
  char* name;
  struct Ast_Parameter* next_param;
}
Ast_Parameter;

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
Ast_TypeParameter;

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
  int argument_count;
}
Ast_FunctionCallExpr;

typedef struct Ast_BinaryExpr
{
  Ast_Expression;
  enum AstExprOperator op;
  struct Ast_Expression* l_operand;
  struct Ast_Expression* r_operand;
}
Ast_BinaryExpr;

typedef struct
{
  Ast_Expression;
  char* name;
}
Ast_IdentExpr;

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_IntegerExpr;

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_WIntegerExpr;

typedef struct
{
  Ast_Expression;
  int value;
}
Ast_SIntegerExpr;

typedef struct
{
  Ast_Expression;
}
Ast_ErrorExpr;

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
Ast_VarDecl;

typedef struct
{
  Ast_StateExpr;
  char* name;
}
Ast_IdentState;

typedef struct Ast_SelectCase
{
  Ast;
  char* end_state;
  struct Ast_SelectCase* next;
}
Ast_SelectCase;

typedef struct
{
  Ast_SelectCase;
  Ast_Expression* key_expr;
}
Ast_ExprSelectCase;

typedef struct
{
  Ast_SelectCase;
}
Ast_DefaultSelectCase;

typedef struct
{
  Ast_StateExpr;
  Ast_Expression* expression;
  Ast_SelectCase* select_case;
}
Ast_SelectState;

typedef struct
{
  Ast;
  Ast_StateExpr* state_expr;
}
Ast_TransitionStmt;

typedef struct Ast_ParserState
{
  Ast;
  char* name;
  Ast_Expression* statement;
  struct Ast_ParserState* next;
  Ast_TransitionStmt* transition_stmt;
}
Ast_ParserState;

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
Ast_ExprStmt;

typedef struct
{
  Ast_Statement;
  Ast_Expression* statement;
}
Ast_BlockStmt;

typedef struct
{
  Ast;
  bool value;
}
Ast_Bool;

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
Ast_Key;

typedef struct Ast_ActionRef
{
  Ast_TableProperty;
  char* name;
  Ast_Expression* argument;
  struct Ast_ActionRef* next_action;
}
Ast_ActionRef;

typedef struct Ast_SimpleProp
{
  Ast_TableProperty;
  Ast_Expression* expression;
}
Ast_SimpleProp;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_Parameter* parameter;
  Ast_BlockStmt* action_body;
}
Ast_ActionDecl;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_TableProperty* property;
}
Ast_TableDecl;

typedef struct
{
  Ast;
  char* name;
  Ast_Parameter* parameter;
  IdentInfo_Type* id_info;
  Ast_Declaration* local_decl;
  int local_decl_count;
  Ast_BlockStmt* control_body;
}
Ast_ControlType;

typedef struct
{
  Ast;
  char* name;
  Ast_TypeParameter* type_parameter;
  Ast_Parameter* parameter;
  int param_count;
  IdentInfo_Type* id_info;
  Ast_ParserState* parser_state;
}
Ast_ParserType;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_Parameter* parameter;
  int param_count;
  IdentInfo_Type* id_info;
}
Ast_PackageTypeDecl;

typedef struct
{
  Ast_Declaration;
  Ast_Expression* package;
  char* name;
}
Ast_Instantiation;

typedef struct Ast_FunctionPrototype
{
  Ast_Declaration;
  char* name;
  Ast_TypeParameter* type_parameter;
  Ast_Parameter* parameter;
  int param_count;
  IdentInfo_Type* return_type;
}
Ast_FunctionPrototype;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_FunctionPrototype* method;
  int method_count;
  IdentInfo_Type* id_info;
}
Ast_ExternObjectDecl;

typedef struct
{
  Ast_Declaration;
  char* name;
  Ast_Parameter* parameter;
  int param_count;
  IdentInfo_Type* return_type;
}
Ast_ExternFunctionDecl;

typedef struct
{
  Ast;
  Ast_Declaration* declaration;
  int decl_count;
}
Ast_P4Program;

enum TypeTable_TypeCtor
{
  TYP_NONE,
  TYP_BASIC,
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
};

typedef struct TypeTable_Entry
{
  enum TypeTable_TypeCtor kind;
  bool is_decl;
  char* name;

  union
  {
    struct TypeTable_EnumType
    {
      struct TypeTable_Entry* enum_field;  // sentinel
      struct TypeTable_Entry* last_field;
      int field_count;
    }
    enum_type;

    struct TypeTable_EnumField
    {
      struct TypeTable_Entry* next_field;
    }
    enum_field;
  };
}
TypeTable_Entry;

