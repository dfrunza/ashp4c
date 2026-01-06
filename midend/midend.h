#pragma once

#include "adt/map.h"
#include "adt/array.h"
#include "frontend/lexer.h"
#include "frontend/frontend.h"
#include "midend/type_checker.h"

struct Midend {
  Map* scope_map;
  Map* decl_map;
  Array* type_array;
  Map* type_env;
  Map* potype_map;
  TypeChecker type_checker;

  void do_analysis(Arena* storage, Arena* scratch,
         SourceText* source_text, Frontend* frontend);
};