#pragma once
#include "arena.h"
#include "hashmap.h"
#include "token.h"


enum Namespace {
  NAMESPACE_TYPE = 1 << 0,
  NAMESPACE_VAR = 1 << 1,
  NAMESPACE_KEYWORD = 1 << 2,
};

struct NameDecl {
  enum AstEnum kind;
  uint32_t id;
  char* strname;
  int line_no;
  struct NameDecl* next_in_scope;
};  

struct NameDecl_Keyword {
  struct NameDecl;
  enum TokenClass token_class;
};

struct NameRef {
  uint32_t id;
  char* strname;
  int line_no;
  struct Scope* scope;
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
struct SymtableEntry* scope_lookup_name(struct Scope* scope, char* name);
struct SymtableEntry* declare_object_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl);
struct NameRef* nameref_get_entry(struct Hashmap* map, uint32_t id);
void nameref_add_entry(struct Hashmap* map, struct NameRef* nameref, uint32_t id);
struct Type* type_get_entry(struct Hashmap* map, uint32_t id);
void type_add_entry(struct Hashmap* map, struct Type* type, uint32_t id);

