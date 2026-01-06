#pragma once

#include "adt/map.h"
#include "adt/array.h"
#include "frontend/lexer.h"
#include "frontend/frontend.h"
#include "midend/type_checker.h"
#include "midend/passes/builtin_methods.h"
#include "midend/passes/scope_hierarchy.h"
#include "midend/passes/name_binding.h"
#include "midend/passes/declared_type.h"
#include "midend/passes/potential_type.h"
#include "midend/passes/select_type.h"

struct Midend {
  Map* scope_map;
  Map* decl_map;
  Array* type_array;
  Map* type_env;
  Map* po_type_map;

  BuiltinMethodsPass builtin_methods;
  ScopeHierarchyPass scope_hierarchy;
  NameBindingPass name_binding;
  DeclaredTypePass declared_types;
  PotentialTypePass potential_types;
  SelectTypePass select_type;

  TypeChecker type_checker;

  void do_analysis(Arena* storage, Arena* scratch,
         SourceText* source_text, Frontend* frontend);
};