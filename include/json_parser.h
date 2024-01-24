#pragma once

#include "shared.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct json;
struct json_obj;

/* a recursive parser */
__EXPOSED __HEADER_ONLY int json_parse (struct json *target,
                                        const char *source, size_t length);

__EXPOSED char *__json_parse (struct json *target, const char *source,
                              size_t length, int __cur_depth);

/* a recursive parser for array */
__EXPOSED char *__json_arr_parse (struct json *target, const char *source,
                                  size_t length, int __cur_depth);

/* a recursive parser for object */
__EXPOSED char *__json_obj_parse (struct json_obj *target, const char *source,
                                  size_t length, int __cur_depth);

__EXPOSED __HEADER_ONLY int
json_parse (struct json *target, const char *source, size_t length) {
  int ret;
  // skip the leading spaces
  source = strskip (source, length);
  if (*source == '{') {
    ret = (__json_parse (target, source, length, 0) == NULL) ? -1 : 0;
  } else if (*source == '[') {
    ret = (__json_arr_parse (target, source, length, 0) == NULL) ? -1 : 0;
  } else {
    raise_error (
        "error parsing json object: expecting `{` or `[`, but found %c\n",
        *source);
    ret = -1;
  }

  return ret;
}

#if defined(__cplusplus)
}
#endif
