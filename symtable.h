#pragma once
#include "arena.h"
#include "hash.h"
#include "token.h"


enum SymbolKind {
  Symbol_NONE_,
  Symbol_Keyword,
  Symbol_Type,
  Symbol_Ident,
};

struct Symbol {
  enum SymbolKind symbol_kind;
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


void scope_init(struct Scope* scope, int capacity_log2);
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* find_symtable_entry(struct Scope* scope, char* name);
struct SymtableEntry* get_symtable_entry(struct Scope* scope, char* name);
struct Symbol* new_ident(struct Scope* scope, char* name, struct Ast* ast, int line_nr);
struct Symbol* new_type(struct Scope* scope, char* name, struct Ast* ast, int line_nr);

struct Scope* new_scope(int capacity_log2);
struct Scope* push_scope();
struct Scope* pop_scope();
struct Scope* get_current_scope();
struct SymtableEntry* scope_resolve_name(struct Scope* scope, char* name);

