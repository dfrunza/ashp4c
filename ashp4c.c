#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   /* exit */
#include "foundation.h"
#include "frontend.h"
#include "ashp4c.h"

typedef struct CmdlineArg {
  char* name;
  char* value;
  struct CmdlineArg* next_arg;
} CmdlineArg;

static void
read_source_text(Arena* storage, char* filename, SourceText* source_text/*out*/)
{
  FILE* f_stream;
  char* text;

  f_stream = fopen(filename, "rb");
  if (!f_stream) {
    error("Could not open file '%s'.", filename);
  }
  fseek(f_stream, 0, SEEK_END);
  int text_size = ftell(f_stream);
  fseek(f_stream, 0, SEEK_SET);
  text = arena_malloc(storage, (text_size + 1)*sizeof(char));
  fread(text, sizeof(char), text_size, f_stream);
  text[text_size] = '\0';
  fclose(f_stream);
  source_text->text = text;
  source_text->text_size = text_size;
  source_text->filename = filename;
}

static CmdlineArg*
find_unnamed_arg(CmdlineArg* args)
{
  CmdlineArg* unnamed_arg = 0;
  CmdlineArg* arg;
  
  arg = args;
  while (arg) {
    if (!arg->name) {
      unnamed_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return unnamed_arg;
}

#if 0
static CmdlineArg*
find_named_arg(char* name, CmdlineArg* args)
{
  CmdlineArg* named_arg = 0;
  CmdlineArg* arg = args;
  while (arg) {
    if (arg->name && cstr_match(name, arg->name)) {
      named_arg = arg;
      break;
    }
    arg = arg->next_arg;
  }
  return named_arg;
}
#endif

static CmdlineArg*
parse_cmdline_args(Arena* storage, int arg_count, char* args[])
{
  CmdlineArg* arg_list = 0;
  CmdlineArg *prev_arg, *cmdline_arg;
  CmdlineArg sentinel_arg = {};
  char* raw_arg;

  if (arg_count <= 1) {
    return arg_list;
  }
  prev_arg = &sentinel_arg;
  int i = 1;
  while (i < arg_count) {
    cmdline_arg = arena_malloc(storage, sizeof(CmdlineArg));
    if (cstr_start_with(args[i], "--")) {
      raw_arg = args[i] + 2;  /* skip the `--` prefix */
      cmdline_arg->name = raw_arg;
    } else {
      cmdline_arg->value = args[i];
    }
    prev_arg->next_arg = cmdline_arg;
    prev_arg = cmdline_arg;
    i += 1;
  }
  arg_list = sentinel_arg.next_arg;
  return arg_list;
}

int
main(int arg_count, char* args[])
{
  struct Keyword {
    char* strname;
    enum TokenClass token_class;
  };
  struct BuiltinName {
    char* strname;
    enum NameSpace ns;
  };
  struct BuiltinType {
    char* strname;
    enum TypeEnum ty_former;
  };

  struct Keyword keywords[] = {
    {"action",  TK_ACTION},
    {"actions", TK_ACTIONS},
    {"entries", TK_ENTRIES},
    {"enum",    TK_ENUM},
    {"in",      TK_IN},
    {"package", TK_PACKAGE},
    {"select",  TK_SELECT},
    {"switch",  TK_SWITCH},
    {"tuple",   TK_TUPLE},
    {"control", TK_CONTROL},
    {"error",   TK_ERROR},
    {"header",  TK_HEADER},
    {"inout",   TK_INOUT},
    {"parser",  TK_PARSER},
    {"state",   TK_STATE},
    {"table",   TK_TABLE},
    {"key",     TK_KEY},
    {"typedef", TK_TYPEDEF},
    {"default", TK_DEFAULT},
    {"extern",  TK_EXTERN},
    {"out",     TK_OUT},
    {"else",    TK_ELSE},
    {"exit",    TK_EXIT},
    {"if",      TK_IF},
    {"return",  TK_RETURN},
    {"struct",  TK_STRUCT},
    {"apply",   TK_APPLY},
    {"const",   TK_CONST},
    {"bool",    TK_BOOL},
    {"true",    TK_TRUE},
    {"false",   TK_FALSE},
    {"void",    TK_VOID},
    {"int",     TK_INT},
    {"bit",     TK_BIT},
    {"varbit",  TK_VARBIT},
    {"string",  TK_STRING},
    {"match_kind",   TK_MATCH_KIND},
    {"transition",   TK_TRANSITION},
    {"header_union", TK_HEADER_UNION},
  };

  struct BuiltinName builtin_names[] = {
    {"void",   NAMESPACE_TYPE},
    {"bool",   NAMESPACE_TYPE},
    {"int",    NAMESPACE_TYPE},
    {"bit",    NAMESPACE_TYPE},
    {"varbit", NAMESPACE_TYPE},
    {"string", NAMESPACE_TYPE},
    {"error",  NAMESPACE_TYPE},
    {"match_kind", NAMESPACE_TYPE},
    {"_",      NAMESPACE_TYPE},
    {"accept", NAMESPACE_VAR},
    {"reject", NAMESPACE_VAR},
  };

  struct BuiltinType builtin_types[] = {
    {"void",       TYPE_VOID},
    {"bool",       TYPE_BOOL},
    {"int",        TYPE_INT},
    {"bit",        TYPE_BIT},
    {"varbit",     TYPE_VARBIT},
    {"string",     TYPE_STRING},
    {"error",      TYPE_ERROR},
    {"match_kind", TYPE_MATCH_KIND},
    {"_",          TYPE_DONTCARE},
  };

  CmdlineArg* cmdline, *filename;
  SourceText source_text = {0};
  Arena storage = {0}, scratch_storage = {0};
  Array* tokens;
  Ast* name, *program;
  NameEntry* name_entry;
  NameDeclaration* name_decl;
  Scope* root_scope;
  Array* type_array;
  Type* builtin_ty;
  Map* type_env, *decl_map, *scope_map;

  reserve_memory(500*KILOBYTE);

  cmdline = parse_cmdline_args(&storage, arg_count, args);
  filename = find_unnamed_arg(cmdline);
  if (!filename) {
    printf("<filename> is required.\n");
    exit(1);
  }

  root_scope = scope_create(&storage, 5);
  for (int i = 0; i < sizeof(keywords)/sizeof(keywords[0]); i++) {
    name_decl = scope_bind(&storage, root_scope, keywords[i].strname, NAMESPACE_KEYWORD);
    name_decl->token_class = keywords[i].token_class;
  }
  for (int i = 0; i < sizeof(builtin_names)/sizeof(builtin_names[0]); i++) {
    name = arena_malloc(&storage, sizeof(Ast));
    name->kind = AST_name;
    name->name.strname = builtin_names[i].strname;
    name_decl = scope_bind(&storage, root_scope, name->name.strname, builtin_names[i].ns);
    name_decl->ast = name;
  }

  type_array = array_create(&storage, sizeof(Type), 5);
  type_env = arena_malloc(&storage, sizeof(Map));
  *type_env = (Map){0};
  for (int i = 0; i < sizeof(builtin_types)/sizeof(builtin_types[0]); i++) {
    name_entry = scope_lookup(root_scope, builtin_types[i].strname, NAMESPACE_TYPE);
    name_decl = name_entry_getdecl(name_entry, NAMESPACE_TYPE);
    builtin_ty = (Type*)array_append(&storage, type_array, sizeof(Type));
    builtin_ty->ty_former = builtin_types[i].ty_former;
    builtin_ty->strname = name_decl->strname;
    builtin_ty->ast = name_decl->ast;
    name_decl->type = builtin_ty;
    map_insert(&storage, type_env, name_decl->ast, builtin_ty, 0);
  }

  builtin_ty = builtin_type(root_scope, "error");
  *builtin_ty = (Type){0};
  builtin_ty->builtin_enum.fields = (Type*)array_append(&storage, type_array, sizeof(Type));
  builtin_ty->builtin_enum.fields->ty_former = TYPE_PRODUCT;

  builtin_ty = builtin_type(root_scope, "match_kind");
  *builtin_ty = (Type){0};
  builtin_ty->builtin_enum.fields = (Type*)array_append(&storage, type_array, sizeof(Type));
  builtin_ty->builtin_enum.fields->ty_former = TYPE_PRODUCT;

  read_source_text(&scratch_storage, filename->value, &source_text);
  tokens = tokenize_source_text(&storage, &source_text);
  program = parse_program(&storage, source_text.filename, tokens, root_scope);
  arena_free(&scratch_storage);

  drypass(source_text.filename, program);
  scope_map = build_scopes(&storage, source_text.filename, program, root_scope);
  build_symtable(&storage, source_text.filename, program, root_scope,
      scope_map, &decl_map);
  build_type_env(&storage, source_text.filename, program, root_scope, type_array,
      scope_map, decl_map, type_env);
  build_potential_types(&storage, source_text.filename, program, root_scope,
      scope_map, decl_map, type_env);

  arena_free(&storage);
  return 0;
}

