#define _DEBUG

#include "json.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT(x)                                                             \
  if (!(x)) {                                                                 \
    printf ("error\n");                                                       \
  }

void
printf_red (const char *__fmt__, ...) {
  printf ("\033[0m\033[1;31m");
  va_list args;
  va_start (args, __fmt__);
  vprintf (__fmt__, args);
  va_end (args);
  printf ("\033[0m");
}

#define test_success(str)                                                     \
  do {                                                                        \
    printf ("[%s] ", __func__);                                               \
    printf_red ("[succeed]%s:", "");                                          \
    printf ("%s\n", str);                                                     \
  } while (0)

#define test_success2(str, ...)                                               \
  do {                                                                        \
    printf ("[%s] ", __func__);                                               \
    printf_red ("[succeed]%s:", "");                                          \
    printf (str, __VA_ARGS__);                                                \
    printf ("\n");                                                            \
  } while (0)

#define test_fail(str)                                                        \
  do {                                                                        \
    printf ("%s():%d ", __func__, __LINE__);                                  \
    printf_red ("[fail]%s:", "");                                             \
    printf ("%s\n", str);                                                     \
  } while (0)

#define test_fail2(str, ...)                                                  \
  do {                                                                        \
    printf ("%s():%d ", __func__, __LINE__);                                  \
    printf_red ("[fail]%s:", "");                                             \
    printf (str, __VA_ARGS__);                                                \
    printf ("\n");                                                            \
  } while (0)

#define string_assert_equal(str1, str2) (strcmp (str1, str2) == 0 ? 0 : -1)
