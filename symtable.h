#pragma once
#include "arena.h"
#include "hashmap.h"
#include "token.h"


enum ObjectKind {
  OBJECT_NONE,
  OBJECT_KEYWORD,
  OBJECT_VAR,
  OBJECT_TYPEVAR,
  OBJECT_CONST,
  OBJECT_PARAM,
  OBJECT_STRUCT_FIELD,
  OBJECT_ENUM_FIELD,
  OBJECT_ACTION,
  OBJECT_PARSER,
  OBJECT_PARSER_PROTO,
  OBJECT_PARSER_STATE,
  OBJECT_CONTROL,
  OBJECT_CONTROL_PROTO,
  OBJECT_FUNCTION,
  OBJECT_FUNCTION_PROTO,
  OBJECT_EXTERN,
  OBJECT_STRUCT,
  OBJECT_HEADER,
  OBJECT_HEADER_UNION,
  OBJECT_PACKAGE,
  OBJECT_INSTANTIATION,
  OBJECT_VOID,
  OBJECT_BOOL,
  OBJECT_INT,
  OBJECT_BIT,
  OBJECT_VARBIT,
  OBJECT_STRING,
  OBJECT_ERROR,
  OBJECT_ENUM,
  OBJECT_MATCH_KIND,
  OBJECT_TABLE,
  OBJECT_TYPE,
  OBJECT_TYPEDEF,
};

enum Namespace {
  NAMESPACE_NONE,
  NAMESPACE_TYPE = 1 << 0,
  NAMESPACE_VAR = 1 << 1,
  NAMESPACE_KEYWORD = 1 << 2,
};

struct NamedObject {
  enum ObjectKind kind;
  char* strname;
  int line_nr;
  struct NamedObject* next_in_scope;
};  

struct Object_Keyword {
  struct NamedObject;
  enum TokenClass token_klass;
};

enum NameRefKind {
  NAMEREF_NONE,
  NAMEREF_VAR,
  NAMEREF_TYPE,
  NAMEREF_MEMBER,
};

struct NameRef {
  enum NameRefKind kind;
  char* strname;
  int line_nr;
  uint32_t name_id;
  struct NamedObject* descriptor;
  struct Scope* scope;
  union {
    struct Ast* member_expr;
  };
};

struct SymtableEntry {
  char* strname;
  struct NamedObject* ns_keyword;
  struct NamedObject* ns_type;
  struct NamedObject* ns_var;
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
struct SymtableEntry* declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct NamedObject* descriptor);
