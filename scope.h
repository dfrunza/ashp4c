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
  union {
    struct Ast* decl;
    enum TokenClass token_class;
  };
  char* strname;
  int line_no;
  struct Scope* scope;
  struct NameDecl* nextdecl_in_scope;
};  

struct NameRef {
  struct Ast* ref;
  char* strname;
  int line_no;
  struct Scope* scope;
};

struct NameEntry {
  char* strname;
  struct NameDecl* ns_type;
  struct NameDecl* ns_var;
  struct NameDecl* ns_keyword;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  struct Hashmap declarations;
};

void scope_init(struct Arena* scope_storage);
struct NameEntry* namedecl_get_or_create(struct Hashmap* declarations, char* name);
struct NameEntry* namedecl_get_entry(struct Hashmap* declarations, char* name);
struct Scope* push_scope();
struct Scope* pop_scope();
struct NameEntry* scope_lookup_name(struct Scope* scope, char* name);
struct NameEntry* declare_name_in_scope(struct Scope* scope, enum Namespace ns, struct NameDecl* decl);
struct NameRef* nameref_get(struct Hashmap* map, uint32_t id);
void nameref_add(struct Hashmap* map, struct NameRef* nameref, uint32_t id);

