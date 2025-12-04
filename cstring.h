#pragma once

namespace cstring {
  bool is_letter(char c);
  bool is_digit(char c, int base);
  bool is_ascii_printable(char c);
  bool is_whitespace(char c);
  int len(char *str);
  char *copy(char *dest_str, char *src_str);
  void copy_substr(char *dest_str, char *begin_char, char *end_char);
  bool start_with(char *str, char *prefix);
  bool match(char *str_a, char *str_b);
  void print_substr(char *begin_char, char *end_char);
}