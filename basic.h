#pragma once
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>   // exit
#include <stdarg.h>   // va_list, va_start, va_end


#define global static
#define internal static
#define external extern
#define true 1u
#define false 0u
#define bool uint32_t
#define KILOBYTE 1024
#define MEGABYTE 1024*KILOBYTE

void assert_(char* message, char* file, int line);
void error_(char* file, int line, char* message, ...);
bool cstr_is_letter(char c);
bool cstr_is_digit(char c, int base);
bool cstr_is_ascii_printable(char c);
bool cstr_is_whitespace(char c);
int cstr_len(char* str);
char* cstr_copy(char* dest_str, char* src_str);
void cstr_copy_substr(char* dest_str, char* begin_char, char* end_char);
bool cstr_start_with(char* str, char* prefix);
bool cstr_match(char* str_a, char* str_b);
void cstr_print_substr(char* begin_char, char* end_char);
bool bytes_match(uint8_t* bytes_a, int len_a, uint8_t* bytes_b, int len_b);

#define assert(expr) \
  do { if(!(expr)) assert_(#expr, __FILE__, __LINE__); } while(0)
#define error(msg, ...) error_(__FILE__, __LINE__, (msg), ## __VA_ARGS__)
