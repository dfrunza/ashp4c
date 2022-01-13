#pragma once
#include "arena.h"
#include "hashmap.h"
#include "token.h"


enum ObjectKind {
  OBJECT_NONE,
  OBJECT_NAME_REF,
  OBJECT_KEYWORD,
  OBJECT_VAR,
  OBJECT_CONST,
  OBJECT_PARAM,
  OBJECT_TYPE_PARAM,
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

struct ObjectDescriptor {
  char* name;
  enum ObjectKind object_kind;
  struct Ast* ast;
  struct ObjectDescriptor* next_in_scope;
};  

struct Object_Keyword {
  struct ObjectDescriptor;
  enum TokenClass token_klass;
};

enum Namespace {
  NAMESPACE_NONE,
  NAMESPACE_TYPE,
  NAMESPACE_GENERAL,
  NAMESPACE_KEYWORD,
};

struct SymtableEntry {
  char* name;
  struct ObjectDescriptor* ns_keyword;
  struct ObjectDescriptor* ns_type;
  struct ObjectDescriptor* ns_general;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  struct Scope* first_child_scope;
  struct Scope* right_sibling_scope;
  struct Hashmap symtable;
};


void symtable_init(struct Arena* symtable_storage_, struct Arena* temp_storage_);
void scope_init(struct Scope* scope, int capacity_log2);
struct SymtableEntry* symtable_get_or_create_entry(struct Scope* scope, char* name);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_root_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);
struct SymtableEntry* declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct ObjectDescriptor* descriptor, int line_nr);
