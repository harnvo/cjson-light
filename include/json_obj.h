#pragma once
#include "__debug_tool.h"
#include "shared.h"
#include "str_view.h"
#include "json_parser.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(__cplusplus)
extern "C" {
#endif

// global string view
static const str_view_t __json_keyword_null = { "null", 4 };
static const str_view_t __json_keyword_true = { "true", 4 };
static const str_view_t __json_keyword_false = { "false", 5 };

// json
enum json_type {
  JSON_TYPE_NULL = 1<<0,
  JSON_TYPE_OBJECT = 1 << 1,
  JSON_TYPE_ARRAY = 1 << 2,

  JSON_TYPE_STR = 1 << 3,
  JSON_TYPE_LZ_STR = 1 << 4,

  // JSON_TYPE_INT     = 1<<4,
  JSON_TYPE_FLOAT = 1 << 5,

  JSON_TYPE_BOOLEAN = 1 << 6,
  // JSON_TYPE_RAW = 1 << 7,

  // the highest bit is reserved for internal use
  __JSON_OWNS_SOURCE = 1 << 31,
};

#define __JSON_OBJ_LZ_STR_LEN sizeof (str_view_t)

struct json_obj {
  // TODO: should i simply put this inside storage?
  str_view_t key; // should not be used for array

  union {
    struct json *array;
    struct json *object;
    str_view_t str;
    double number;
    int boolean;

    char lz_str[__JSON_OBJ_LZ_STR_LEN]; // lazy string to avoid malloc
  } value;

  // it is possible that json_obj owns a string.
  // if it does, it will free the string when it is freed.
  // CAUTION 1: assumes unique ownership. If the string is shared, it will
  // cause problem.
  // CAUTION 2: might do in-place modification of the string,
  // even if they do not own it!
  // CAUTION 3: it does not necessarily means that
  // the string is used as a key/value.
  char *__source;
  size_t __source_len;
  int type;
};

/* declarations */
__HEADER_ONLY char *json_obj_get_type (struct json_obj *obj);

__HEADER_ONLY int json_obj_init (struct json_obj *obj, const char *raw_str);

__HEADER_ONLY int json_obj_init_view (struct json_obj *obj, str_view_t raw_str);

int json_obj_destroy (struct json_obj *obj);

__HEADER_ONLY int json_obj_owns_source (struct json_obj *obj);
__HEADER_ONLY int json_obj_owns_key (struct json_obj *obj);
__HEADER_ONLY int json_obj_owns_value (struct json_obj *obj);

__HEADER_ONLY int json_obj_take_ownership (struct json_obj *obj, char *source,
                                           size_t len);
__HEADER_ONLY int json_obj_yield_ownership (struct json_obj *obj, char **dst,
                                            size_t *dst_len);

__HEADER_ONLY int json_obj_is_number (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_str (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_boolean (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_object (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_array (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_null (struct json_obj *obj);
__HEADER_ONLY int json_obj_has_children (struct json_obj *obj);
__HEADER_ONLY int json_obj_is_array_element (struct json_obj *obj);

__HEADER_ONLY int json_obj_asnum (struct json_obj *obj);
__HEADER_ONLY int json_obj_asbool (struct json_obj *obj);
__HEADER_ONLY int json_obj_asstr (struct json_obj *obj);
__HEADER_ONLY int json_obj_tonum (struct json_obj *obj, double *dst);
__HEADER_ONLY int json_obj_tostr (struct json_obj *obj, char **dst,
                                  size_t *dst_len);

int json_obj_set_key (struct json_obj *obj, const char *key);
int json_obj_set_key_by_view (struct json_obj *obj, str_view_t key);

int json_obj_set_null (struct json_obj *obj);
int json_obj_set_number (struct json_obj *obj, double number);
int json_obj_set_boolean (struct json_obj *obj, int boolean);
int json_obj_set_str (struct json_obj *obj, const char *str);
int json_obj_set_str_by_view (struct json_obj *obj,
                                            str_view_t str);

str_view_t json_obj_get_key (struct json_obj *obj);
int json_obj_get_key_len (struct json_obj *obj);

str_view_t json_obj_get_value_str (struct json_obj *obj);
int json_obj_get_value_str_len (struct json_obj *obj);

int json_obj_get_value_boolean (struct json_obj *obj);
double json_obj_get_value_number (struct json_obj *obj);

struct json *json_obj_get_value_object (struct json_obj *obj);
struct json *json_obj_get_value_array (struct json_obj *obj);
size_t json_obj_get_value_array_len (struct json_obj *obj);

// TOOD: unimplemented
void __json_obj_print (struct  json_obj *obj, int flag, int __cur_depth);

__HEADER_ONLY char *
json_obj_get_type (struct json_obj *obj) {
  if (json_obj_is_array(obj)) {
    return "array";
  } else if (json_obj_is_object(obj)) {
    return "object";
  } else if (json_obj_is_str(obj)) {
    return "string";
  } else if (json_obj_is_number(obj)) {
    return "number";
  } else if (json_obj_is_boolean(obj)) {
    return "boolean";
  } else if (json_obj_is_null(obj)) {
    return "null";
  } else {
    return "unknown";
  }
}

/* --- init&destroy --- */
__EXPOSED __HEADER_ONLY int
json_obj_init (struct json_obj *obj, const char *raw_str) {
  if (raw_str == NULL) {
    // return a null json_obj
    memset (obj, 0, sizeof (struct json_obj));
    obj->type = JSON_TYPE_NULL;
    return 0;
  }

  int ret = __json_obj_parse(obj, raw_str, strlen(raw_str), 0);
  if (ret < 0) {
    return ret;
  }

  // if (_take_ownership) {
  //   obj->type |= __JSON_OWNS_SOURCE;
  //   obj->__source = (char *)raw_str;
  //   obj->__source_len = strlen (raw_str);
  // }

  return ret;
}

__EXPOSED __HEADER_ONLY int
json_obj_init_view (struct json_obj *obj, str_view_t raw_str) {
  int ret = __json_obj_parse(obj, raw_str.str, raw_str.len, 0);
  if (ret < 0) {
    return ret;
  }

  // if (_take_ownership) {
  //   obj->type |= __JSON_OWNS_SOURCE;
  //   obj->__source = (char *)raw_str.str;
  //   obj->__source_len = raw_str.len;
  // }
  return ret;
}

/* -- very basic utils -- */
__HEADER_ONLY
int
json_obj_owns_source (struct json_obj *obj) {
  return obj->type & __JSON_OWNS_SOURCE;
}

__EXPOSED __HEADER_ONLY int
json_obj_owns_key (struct json_obj *obj) {
  if (!json_obj_owns_source (obj)) {
    return 0;
  }

  // if (!(obj->type & JSON_TYPE_STR)) {
  //   return 0;
  // }

  // check ownership. See if the key is is within the source string.
  if (obj->key.str >= obj->__source
      && obj->key.str < obj->__source + obj->__source_len) {
    return 1;
  }

  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_owns_value (struct json_obj *obj) {
  if (!json_obj_owns_source (obj)) {
    return 0;
  }

  if (!(obj->type & JSON_TYPE_STR)) {
    return 0;
  }

  // check ownership. See if the value is is within the source string.
  if (obj->value.str.str >= obj->__source
      && obj->value.str.str < obj->__source + obj->__source_len) {
    return 1;
  }

  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_take_ownership (struct json_obj *obj, char *source, size_t len) {
  static char* err = "Cannot take ownership: \
json_obj is already a owner! the json_obj have to yield ownership before owning a source string.";
  if (json_obj_owns_source (obj)) {
    char *source = __FILE__; int line = __LINE__;
    // lock
    _lock (&global_json_error.lock);
    global_json_error.text = err;
    global_json_error.source = source;
    global_json_error.line = line;
    global_json_error.column = 0;
    _unlock (&global_json_error.lock);
    return -1;
  }
  // sanity check: out-of-bound
  obj->__source_len = strlen (source) < len ? strlen (source) : len;
  obj->__source = source;
  obj->type |= __JSON_OWNS_SOURCE;

  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_yield_ownership (struct json_obj *obj, char **dst, size_t *dst_len) {
  if (json_obj_owns_source (obj)) {
    *dst = obj->__source;
    *dst_len = obj->__source_len;
    obj->__source = NULL;
    obj->__source_len = 0;
    obj->type &= ~__JSON_OWNS_SOURCE;
    return 0;
  }

  return -1;
}

__HEADER_ONLY int
json_obj_is_number (struct json_obj *obj) {
  return obj->type & JSON_TYPE_FLOAT;
}

__HEADER_ONLY int
json_obj_is_str (struct json_obj *obj) {
  return obj->type & (JSON_TYPE_STR | JSON_TYPE_LZ_STR);
}

__HEADER_ONLY int
json_obj_is_boolean (struct json_obj *obj) {
  return obj->type & JSON_TYPE_BOOLEAN;
}

__HEADER_ONLY int
json_obj_is_object (struct json_obj *obj) {
  return obj->type & JSON_TYPE_OBJECT;
}

__HEADER_ONLY int
json_obj_is_array (struct json_obj *obj) {
  return obj->type & JSON_TYPE_ARRAY;
}

__HEADER_ONLY int
json_obj_is_null (struct json_obj *obj) {
  return obj->type & JSON_TYPE_NULL;
}

__HEADER_ONLY int
json_obj_has_children (struct json_obj *obj) {
  return obj->type & (JSON_TYPE_OBJECT | JSON_TYPE_ARRAY);
}

__HEADER_ONLY int
json_obj_is_array_element (struct json_obj *obj) {
  return obj->key.str == NULL;
}

/* --- basic type convertions --- */
__EXPOSED __HEADER_ONLY int
json_obj_asnum (struct json_obj *obj) {
  // supports: number, str, lz_str, boolean
  if (json_obj_is_number (obj)) {
    return 0;
  } else if (obj->type & JSON_TYPE_STR) {
    int ret = str_view_toi (&obj->value.str, (int *)&obj->value.number);
    if (ret < 0) {
      return -1;
    }
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    stod (obj->value.lz_str, &obj->value.number);
  } else if (json_obj_is_boolean (obj)) {
    obj->value.number = (double)obj->value.boolean;
  } else {
    return -1;
  }

  obj->type = JSON_TYPE_FLOAT;
  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_asbool (struct json_obj *obj) {
  // supports: number, str, lz_str, boolean
  if (json_obj_is_boolean (obj)) {
    return 0;
  } else if (obj->type & JSON_TYPE_STR) {
    if (str_view_cmp (&obj->value.str, &__json_keyword_true) == 0) {
      obj->value.boolean = 1;
    } else if (str_view_cmp (&obj->value.str, &__json_keyword_false) == 0) {
      obj->value.boolean = 0;

    } else {
      return -1;
    }

    obj->type &= ~JSON_TYPE_STR;
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    if (strcmp (obj->value.lz_str, "true") == 0) {
      obj->value.boolean = 1;
    } else if (strcmp (obj->value.lz_str, "false") == 0) {
      obj->value.boolean = 0;
    } else {
      return -1;
    }

    obj->type &= ~JSON_TYPE_LZ_STR;
  } else if (json_obj_is_number (obj)) {
    obj->value.boolean = (int)obj->value.number != 0;

    obj->type &= ~JSON_TYPE_FLOAT;
  } else {
    return -1;
  }

  obj->type &= JSON_TYPE_BOOLEAN;
  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_asstr (struct json_obj *obj) {
  debug_print ("json_obj type: %s\n", json_obj_get_type (obj));

  // basic supports: number, str, lz_str, boolean
  if (json_obj_is_str (obj)) {
    // do nothing
  } else if (json_obj_is_boolean (obj)) {
    if (obj->value.boolean) {
      obj->value.str = __json_keyword_true;
    } else {
      obj->value.str = __json_keyword_false;
    }
    debug_assert (!json_obj_owns_source (obj));    

    obj->type &= ~JSON_TYPE_BOOLEAN;
    obj->type |= JSON_TYPE_STR;

    debug_assert (!json_obj_owns_source (obj));    
  } else if (json_obj_is_number (obj)) {
    // to lazy string
    sprintf (obj->value.lz_str, "%f", obj->value.number);

    obj->type &= ~JSON_TYPE_FLOAT;
    obj->type &= JSON_TYPE_LZ_STR;
  }

  else if (json_obj_is_object (obj) || json_obj_is_array (obj)) {
    // not supported yet
    // TODO: do we really need this feature?
    return -1;
  } else {
    return -1;
  }

  return 0;
}

__EXPOSED __HEADER_ONLY int
json_obj_tonum (struct json_obj *obj, double *dst) {
  if (json_obj_is_number (obj)) {
    *dst = obj->value.number;
    return 0;
  } else if (json_obj_is_str (obj)) {
    return str_view_tod (&obj->value.str, dst);
  } else if (json_obj_is_boolean (obj)) {
    *dst = (double)obj->value.boolean;
    return 0;
  } else {
    return -1;
  }
}

// be careful: memory allocation involved
__EXPOSED __HEADER_ONLY int
json_obj_tostr (struct json_obj *obj, char **dst, size_t *dst_len) {
  if (obj->type & JSON_TYPE_LZ_STR) {
    *dst = (char *)json_global_hooks.malloc_fn (strlen (obj->value.lz_str) + 1);
    if (*dst == NULL) {
      return -1;
    }

    strcpy (*dst, obj->value.lz_str);
    *dst_len = strlen (*dst);

    return 0;
  } else if (obj->type & JSON_TYPE_STR) {
    *dst = (char *)json_global_hooks.malloc_fn (obj->value.str.len + 1);
    if (*dst == NULL) {
      return -1;
    }

    strncpy (*dst, obj->value.str.str, obj->value.str.len);
    (*dst)[obj->value.str.len] = '\0';
    *dst_len = obj->value.str.len;

    return 0;
  } else if (obj->type & JSON_TYPE_BOOLEAN) {
    if (obj->value.boolean) {
      *dst = (char *)json_global_hooks.malloc_fn (__json_keyword_true.len + 1);
      if (*dst == NULL) {
        return -1;
      }

      strncpy (*dst, __json_keyword_true.str, __json_keyword_true.len);
      (*dst)[__json_keyword_true.len] = '\0';
      *dst_len = __json_keyword_true.len;
    } else {
      *dst = (char *)json_global_hooks.malloc_fn (__json_keyword_false.len + 1);
      if (*dst == NULL) {
        return -1;
      }

      strncpy (*dst, __json_keyword_false.str, __json_keyword_false.len);
      (*dst)[__json_keyword_false.len] = '\0';
      *dst_len = __json_keyword_false.len;
    }

    return 0;
  } else if (obj->type & JSON_TYPE_FLOAT) {
    *dst = (char *)json_global_hooks.malloc_fn (32);
    if (*dst == NULL) {
      return -1;
    }

    sprintf (*dst, "%f", obj->value.number);
    *dst_len = strlen (*dst);

    return 0;
  } else if (obj->type & JSON_TYPE_OBJECT) {
    // not supported yet
    return -1;
  } else if (obj->type & JSON_TYPE_ARRAY) {
    // not supported yet
    return -1;
  } else {
    // not supported yet
    return -1;
  }

  return 0;
}



#if defined(__cplusplus)
}
#endif