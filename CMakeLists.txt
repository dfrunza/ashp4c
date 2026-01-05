cmake_minimum_required(VERSION 3.25)
project(ashp4c)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS
    "-g3 -ggdb \
    -fpermissive -fms-extensions \
    -Wno-pointer-arith -Wno-sign-compare -Wno-nonnull-compare -Winline -Wno-write-strings \
    -Wno-format -Wno-unused-function -Wreturn-type -Wall")

include_directories(.)

add_executable(ashp4c
        ashp4c.cpp
        command_line.cpp
        command_line.h
        adt/array.cpp
        adt/array.h
        adt/basic.cpp
        adt/basic.h
        adt/cstring.cpp
        adt/cstring.h
        adt/map.cpp
        adt/map.h
        adt/strmap.cpp
        adt/strmap.h
        adt/tree.cpp
        adt/tree.h
        adt/list.cpp
        adt/list.h
        memory/arena.cpp
        memory/arena.h
        frontend/ast.cpp
        frontend/ast.h
        frontend/token.cpp
        frontend/token.h
        frontend/lexer.cpp
        frontend/lexer.h
        frontend/namespace.h
        frontend/parser.cpp
        frontend/parser.h
        frontend/scope.cpp
        frontend/scope.h
        frontend/type.cpp
        frontend/type.h
        frontend/namespace.cpp
        midend/ast_visitor.cpp
        midend/ast_visitor.h
        midend/potential_type.cpp
        midend/potential_type.h
        midend/type_checker.cpp
        midend/type_checker.h
        midend/passes/builtin_methods.cpp
        midend/passes/builtin_methods.h
        midend/passes/declared_type.cpp
        midend/passes/declared_type.h
        midend/passes/drypass.cpp
        midend/passes/name_binding.cpp
        midend/passes/name_binding.h
        midend/passes/potential_type.cpp
        midend/passes/potential_type.h
        midend/passes/scope_hierarchy.cpp
        midend/passes/scope_hierarchy.h
        midend/passes/select_type.cpp)