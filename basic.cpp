#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "foundation.h"

void assert_(char* message, char* file, int line)
{
  printf("%s:%d: ", file, line);
  if(!message || message[0] == '\0') {
    message = "";
  }
  printf("assert(%s)\n", message);
  exit(2);
}

void error_(char* file, int line, char* message, ...)
{
  va_list args;
  va_start(args, message);
  vprintf(message, args);
  va_end(args);
  printf("\n");
  exit(1);
}

