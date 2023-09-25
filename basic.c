#include <memory.h>  /* memset */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  /* exit */
#include <stdarg.h>  /* va_list, va_start, va_end */
#include <math.h>  /* floor, ceil, log10 */
#include "foundation.h"

void
_assert(char* message, char* file, int line)
{
  printf("%s:%d: ", file, line);
  if(!message || message[0] == '\0') {
    message = "";
  }
  printf("assert(%s)\n", message);
  exit(2);
}

bool
cstr_is_letter(char c)
{
  return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
}

bool
cstr_is_digit(char c, int base)
{
  if (base == 10) {
    return '0' <= c && c <= '9';
  } else if (base == 16) {
    return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
  } else if (base == 8) {
    return '0' <= c && c <= '7';
  } else if (base == 2) {
    return c == '0' || c == '1';
  } else assert(0);
  return false;
}

bool
cstr_is_ascii_printable(char c)
{
  return ' ' <= c && c <= '~';
}

bool
cstr_is_whitespace(char c)
{
  return c == ' ' || c == '\n' || c == '\r' || c == '\t';
}

int
cstr_len(char* str)
{
  int len = 0;
  while(*str++ != 0)
    len++;
  return len;
}

char*
cstr_copy(char* dest_str, char* src_str)
{
  do
    *dest_str++ = *src_str++;
  while(*src_str);
  return dest_str;
}

void
cstr_copy_substr(char* dest_str, char* begin_char, char* end_char)
{
  char* src_str = begin_char;

  do
    *dest_str++ = *src_str++;
  while(src_str <= end_char);
}

bool
cstr_start_with(char* str, char* prefix)
{
  while(*str == *prefix) {
    str++;
    prefix++;
    if(*prefix == '\0')
      break;
  }
  bool result = (*prefix == '\0');
  return result;
}

bool
cstr_match(char* str_a, char* str_b)
{
  while (*str_a == *str_b) {
    str_a++;
    str_b++;
    if (*str_a == '\0')
      break;
  }
  bool result = (*str_a == *str_b);
  return result;
}

void
cstr_print_substr(char* begin_char, char* end_char)
{
  char* c = begin_char;
  while (c <= end_char) {
    printf("%c", *c);
    c++;
  }
}

void
_error(char* file, int line, char* message, ...)
{
  printf("[ERROR] ");
  if (!message) {
    printf("at %s:%d\n", file, line);
  } else {
    va_list args;
    va_start(args, message);
    vprintf(message, args);
    va_end(args);
    printf("\n");
  }
  exit(1);
}

