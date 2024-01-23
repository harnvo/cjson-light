#pragma once

#include "shared.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct json;
struct json_obj;

/* a recursive parser */
__EXPOSED __HEADER_ONLY int
json_parse (struct json *target, const char *source, size_t length);

__EXPOSED
int __json_parse (struct json *target, const char *source, size_t length, int __cur_depth);
  
/* a recursive parser for array */
__EXPOSED
int __json_arr_parse (struct json *target, const char *source, size_t length, int __cur_depth);

/* a recursive parser for object */
__EXPOSED
int __json_obj_parse (struct json_obj *target, const char *source, size_t length, int __cur_depth);

__EXPOSED __HEADER_ONLY int
json_parse (struct json *target, const char *source, size_t length) {
  // skip the leading spaces
  source = strskip (source, length);
  if (*source == '{') {
    return __json_parse (target, source, length, 0);
  } else if (*source == '[') {
    return __json_arr_parse (target, source, length, 0);
  } else {
    raise_error ("error parsing json object: expecting `{` or `[`, but found %c\n", *source);
    return -1;
  }
}

#if defined(__cplusplus)
}
#endif
  