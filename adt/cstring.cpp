#include <stdio.h>
#include "basic.h"
#include "cstring.h"

namespace cstring {
  bool is_letter(char c) {
    return ('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z');
  }

  bool is_digit(char c, int base) {
    if (base == 10) {
      return '0' <= c && c <= '9';
    } else if (base == 16) {
      return ('0' <= c && c <= '9') || ('a' <= c && c <= 'f') || ('A' <= c && c <= 'F');
    } else if (base == 8) {
      return '0' <= c && c <= '7';
    } else if (base == 2) {
      return c == '0' || c == '1';
    } else
      assert(0);
    return 0;
  }

  bool is_ascii_printable(char c) {
    return ' ' <= c && c <= '~';
  }

  bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\r' || c == '\t';
  }

  int len(char *str) {
    int len = 0;
    while (*str++ != 0)
      len++;
    return len;
  }

  char *copy(char *dest_str, char *src_str) {
    do
      *dest_str++ = *src_str++;
    while (*src_str);
    return dest_str;
  }

  void copy_substr(char *dest_str, char *begin_char, char *end_char) {
    char *src_str = begin_char;

    do
      *dest_str++ = *src_str++;
    while (src_str <= end_char);
  }

  bool start_with(char *str, char *prefix) {
    while (*str == *prefix) {
      str++;
      prefix++;
      if (*prefix == '\0')
        break;
    }
    bool result = (*prefix == '\0');
    return result;
  }

  bool match(char *str_a, char *str_b) {
    while (*str_a == *str_b) {
      str_a++;
      str_b++;
      if (*str_a == '\0')
        break;
    }
    bool result = (*str_a == *str_b);
    return result;
  }

  void print_substr(char *begin_char, char *end_char) {
    char *c = begin_char;
    while (c <= end_char) {
      printf("%c", *c);
      c++;
    }
  }
}