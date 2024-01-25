
#include "json.h"

#include <printf.h>
/* See:
 * https://www.gnu.org/software/libc/manual/html_node/Customizing-Printf.html
 * for more information on how to implement custom printf format specifiers.
 */

__HIDDEN int
_str_view_printf (FILE *stream, const struct printf_info *info,
                  const void *const *args) {
  str_view_t sv = *((str_view_t *)args[0]);
  return fprintf (stream, "%.*s", sv.len, sv.str);
}

__HIDDEN int
_str_view_arginfo (const struct printf_info *info, size_t n, int *argtypes) {
  if (n > 0) {
    argtypes[0] = PA_POINTER;
  }
  return 1;
}

__HIDDEN int
_json_obj_value_printf (FILE *stream, const struct printf_info *info,
                        const void *const *args) {
  struct json_obj *obj = *((struct json_obj **)args[0]);
  if (obj->type & JSON_TYPE_STR) {
    return fprintf (stream, "%.*s", (int)obj->value.str.len,
                    obj->value.str.str);
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    return fprintf (stream, "%s", obj->value.lz_str);
  } else if (json_obj_is_number (obj)) {
    return fprintf (stream, "%f", obj->value.number);
  } else if (json_obj_is_boolean (obj)) {
    if (obj->value.boolean) {
      return fprintf (stream, "true");
    } else {
      return fprintf (stream, "false");
    }
  } else if (json_obj_is_null (obj)) {
    return fprintf (stream, "null");
  } else if (json_obj_has_children (obj)) {
    return fprintf (stream, "%J", obj->value.object);
  } else {
    return fprintf (stream, "[JSON_OBJ_UNKNOWN]");
  }
}

__HIDDEN int
_json_obj_value_arginfo (const struct printf_info *info, size_t n,
                         int *argtypes) {
  if (n > 0) {
    argtypes[0] = PA_POINTER;
  }
  return 1;
}

__HIDDEN int
_json_obj_printf (FILE *stream, const struct printf_info *info,
                  const void *const *args) {
  struct json_obj *obj = *((struct json_obj **)args[0]);
  //   return fprintf (stream, "%J", obj);
  // TODO: implement this
  return 0;
}

__HIDDEN int
_json_obj_arginfo (const struct printf_info *info, size_t n, int *argtypes) {
  if (n > 0) {
    argtypes[0] = PA_POINTER;
  }
  return 1;
}

#if defined(__GNUC__) || defined(__clang__)
__attribute__ ((constructor)) void
_custom_printf_init (void) {
  register_printf_function ('J', _json_obj_printf, _json_obj_arginfo);
  register_printf_function ('j', _json_obj_value_printf,
                            _json_obj_value_arginfo);
  register_printf_function ('v', _str_view_printf, _str_view_arginfo);
}

#define CUSTOM_PRINTF_INIT() printf ("No need to call CUSTOM_PRINTF_INIT, you already init it!\n");

#else
void
_custom_printf_init (void) {
  printf (" constructor attribute is not supported, try to call CUSTOM_PRINTF_INIT manually\n");
}

#define CUSTOM_PRINTF_INIT() do { \
  register_printf_function ('J', _json_obj_printf, _json_obj_arginfo); \
  register_printf_function ('j', _json_obj_value_printf,               \
                            _json_obj_value_arginfo);                  \
  register_printf_function ('v', _str_view_printf, _str_view_arginfo); \
} while (0)

#error "Constructor attribute is not supported"

#endif