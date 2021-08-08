#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum SymbolKind {
  Symbol_None,
  Symbol_Keyword,
  Symbol_Type,
  Symbol_Ident,
};

struct Symbol {
  enum SymbolKind ident_kind;
  char* name;
  struct Ast* ast;
  struct Symbol* next_in_scope;
};  

struct Symbol_Keyword {
  struct Symbol;
  enum TokenClass token_klass;
};

struct SymtableEntry {
  char* name;
  struct Symbol* id_kw;
  struct Symbol* id_type;
  struct Symbol* id_ident;
  struct SymtableEntry* next_entry;
};

struct Scope {
  int scope_level;
  struct Scope* parent_scope;
  int capacity_log2;
  int capacity;
  int entry_count;
  struct UnboundedArray symtable;
};


void scope_init();
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* get_symtable_entry(struct Scope* scope, char* name);
bool name_is_declared_local(struct Scope* scope, char* name, enum SymbolKind kind);
struct Symbol* new_ident(struct Scope* scope, char* name, struct Ast* ast, int line_nr);
struct Symbol* new_type(struct Scope* scope, char* name, struct Ast* ast, int line_nr);

struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

