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
        arena.cpp
        arena.h
        array.h
        ashp4c.cpp
        ast.cpp
        ast.h
        ast_visitor.cpp
        ast_visitor.h
        basic.cpp
        basic.h
        cstring.cpp
        cstring.h
        token.h
        lexer.cpp
        lexer.h
        map.h
        namespace.h
        parser.cpp
        parser.h
        scope.cpp
        scope.h
        strmap.h
        type.cpp
        type.h
        type_checker.h
        list.h
        namespace.cpp
        passes/builtin_methods.cpp
        passes/builtin_methods.h
        passes/declared_type.cpp
        passes/declared_type.h
        passes/drypass.cpp
        passes/name_binding.cpp
        passes/potential_type.cpp
        passes/potential_type.h
        passes/scope_hierarchy.cpp
        passes/scope_hierarchy.h
        passes/select_type.cpp)