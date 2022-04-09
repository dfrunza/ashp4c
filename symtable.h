#pragma once
#include "arena.h"
#include "hashmap.h"
#include "token.h"


enum DeclEnum {
  DECL_KEYWORD = 1,
  DECL_VAR,
  DECL_TYPEVAR,
  DECL_CONST,
  DECL_PARAM,
  DECL_STRUCT_FIELD,
  DECL_ENUM_FIELD,
  DECL_ACTION,
  DECL_PARSER,
  DECL_PARSER_PROTO,
  DECL_PARSER_STATE,
  DECL_CONTROL,
  DECL_CONTROL_PROTO,
  DECL_FUNCTION,
  DECL_FUNCTION_PROTO,
  DECL_EXTERN,
  DECL_STRUCT,
  DECL_HEADER,
  DECL_HEADER_UNION,
  DECL_PACKAGE,
  DECL_INSTANTIATION,
  DECL_VOID,
  DECL_BOOL,
  DECL_INT,
  DECL_BIT,
  DECL_VARBIT,
  DECL_STRING,
  DECL_ERROR,
  DECL_ENUM,
  DECL_MATCH_KIND,
  DECL_TABLE,
  DECL_TYPE,
  DECL_TYPEDEF,
};

enum Namespace {
  NAMESPACE_TYPE = 1 << 0,
  NAMESPACE_VAR = 1 << 1,
  NAMESPACE_KEYWORD = 1 << 2,
};

struct NameDecl {
  enum DeclEnum kind;
  struct Ast_Name* name;
  char* strname;
  int line_no;
  struct NameDecl* next_in_scope;
};  

struct Name_Keyword {
  struct NameDecl;
  enum TokenClass token_class;
};

struct NameRef {
  struct Ast_Name* name;
  char* strname;
  int line_no;
  struct Scope* scope;
  struct NameDecl* ns_type;
  struct NameDecl* ns_var;
};

struct SymtableEntry {
  char* strname;
  struct NameDecl* ns_keyword;
  struct NameDecl* ns_type;
  struct NameDecl* ns_var;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  struct Hashmap declarations;
};


void symtable_init(struct Arena* symtable_storage);
void scope_init(struct Scope* scope, int capacity_log2);
struct SymtableEntry* symtable_get_or_create_entry(struct Hashmap* declarations, char* name);
struct SymtableEntry* symtable_get_entry(struct Hashmap* declarations, char* name);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_root_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_lookup_name(struct Scope* scope, enum Namespace ns, char* name);
struct SymtableEntry* declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl);
