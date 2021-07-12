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
  int scope_level;
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


void symtable_init();
void symtable_set_storage(struct Arena* symtable_storage_);
struct SymtableEntry* get_symtable_entry(char* name);
int push_scope();
void pop_scope();
