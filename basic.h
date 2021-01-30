#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   // malloc & free
#include <stdarg.h>   // va_list, va_start, va_end

#define local static
#define global static
#define internal static
#define external extern
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

#if DEBUG_ENABLED
#define DEBUG(msg, ...) \
  printf((msg), ## __VA_ARGS__);
#else
#define DEBUG(msg, ...) ;
#endif

#define sizeof_array(array) (sizeof(array)/sizeof(array[0]))
#define offsetof(type, member) ((size_t) &((type*)0)->member)
#define containerof(ptr, type, member) ({ \
	const typeof(((type* )0)->member)* mptr = (ptr); \
	(type*)((char *)mptr - offsetof(type, member)); })
#define assert(expr) \
  do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)
void assert_(char* message, char* file, int line);
bool cstr_is_letter(char c);
bool cstr_is_digit(char c);
bool cstr_is_ascii_printable(char c);
char* cstr_copy(char* dest_str, char* src_str);
void cstr_copy_substr(char* dest_str, char* begin_char, char* end_char);
bool cstr_match(char* str_a, char* str_b);
void cstr_print_substr(char* begin_char, char* end_char);
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)
void error_(char* file, int line, char* message, ...);
