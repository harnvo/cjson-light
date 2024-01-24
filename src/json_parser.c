#include "json_parser.h"
#include "__debug_tool.h"
#include "json_obj.h"
#include "json_store.h"

#define __JSON_MAX_DEPTH 128

char *
json_value_parse (struct json_obj *target, const char *source, size_t length,
                  int __cur_depth) {
  if (source == NULL || length == 0) {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: invalid source. stop here:%s\n",
                 src_trunc);
    return NULL;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: too deep. stop here:%s\n",
                 src_trunc);
    return NULL;
  }

  char *src_end = (char *)source + length;

  /* parse the value */
  str_view_t value;
  str_view_init (&value, source, src_end - source);
  // special cases?
  // TODO: look a bit too messy here. refactor it later
  static const char *err
      = "json_obj_parse: invalid keyword value when parsing \
`%s`. Did you mean %s?\n";
  if (strncmp (source, "false", 5) == 0) {
    target->type = JSON_TYPE_BOOLEAN;
    target->value.boolean = 0;
    // return the offset
    source += 5;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element (target)) {
        fprintf (stderr, err, "array element", "false");
      } else {
        char _key[target->key.len + 1];
        snprintf (_key, target->key.len + 1, "%.*s", (int)target->key.len,
                  target->key.str);
        fprintf (stderr, err, _key, "false");
      }
      while (*source != ',' && *source != '}' && *source != ']'
             && *source != '\0' && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy (src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf (stderr, "fatal: unmatched quote. stop here:%s\n",
                   src_trunc);
          return NULL;
        }
        source++;
      }
    }
    return (char *)source;
  }

  if (strncmp (source, "true", 4) == 0) {
    target->type = JSON_TYPE_BOOLEAN;
    target->value.boolean = 1;
    // return the offset
    source += 4;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element (target)) {
        fprintf (stderr, err, "array element", "true");
      } else {
        char _key[target->key.len + 1];
        snprintf (_key, target->key.len + 1, "%.*s", (int)target->key.len,
                  target->key.str);
        fprintf (stderr, err, _key, "true");
      }
      // find the next comma
      while (*source != ',' && *source != '}' && *source != '\0'
             && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy (src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf (stderr, "fatal: unmatched quote. stop here:%s\n",
                   src_trunc);
          return NULL;
        }
        source++;
      }
    }
    return (char *)source;
  }

  if (strncmp (source, "null", 4) == 0) {
    target->type = JSON_TYPE_NULL;
    memset (&target->value, 0, sizeof (target->value));
    // return the offset
    source += 4;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element (target)) {
        fprintf (stderr, err, "array element", "null");
      } else {
        char _key[target->key.len + 1];
        snprintf (_key, target->key.len + 1, "%.*s", (int)target->key.len,
                  target->key.str);
        fprintf (stderr, err, _key, "null");
      }
      while (*source != ',' && *source != '}' && *source != '\0'
             && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy (src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf (stderr, "fatal: unmatched quote. stop here:%s\n",
                   src_trunc);
          return NULL;
        }
        source++;
      }
    }
    return (char *)source;
  }

  // TODO: finish the value parser
  switch (*source) {
  case '{': {
    // object
    struct json *new_json
        = (struct json *)json_global_hooks.malloc_fn (sizeof (struct json));
    // choose the storage type
    json_list_storage_init (new_json);
    char *_source
        = __json_parse (new_json, source, src_end - source, __cur_depth + 1);
    if (_source == NULL) {
      json_destroy (new_json);
      json_global_hooks.free_fn (new_json);
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("error parsing json object. stop here:%s\n", src_trunc);
      return NULL;
    }
    source = _source;

    target->value.object = new_json;
    target->type = JSON_TYPE_OBJECT;

    // skip the right brace
    source = strskip (source, src_end - source);
    if (*source != '}') {
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error (
          "error parsing json object: unmatched bracket. stop here:%s\n",
          src_trunc);
      return NULL;
    }
    source++;

    return (char *)source;
  }
  case '[': {
    // array
    struct json *new_json
        = (struct json *)json_global_hooks.malloc_fn (sizeof (struct json));
    json_array_storage_init (new_json, -1);
    source = __json_arr_parse (new_json, source, src_end - source,
                               __cur_depth + 1);
    if (source == NULL) {
      json_destroy (new_json);
      json_global_hooks.free_fn (new_json);
      // no more warnings, we already have the error message
      raise_error ("traceback%s.\n", "");
      return NULL;
    }
    target->value.array = new_json;
    target->type = JSON_TYPE_ARRAY;

    // skip the right brace
    source = strskip (source, src_end - source);
    if (*source != ']') {
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error (
          "error parsing json array: unmatched bracket. stop here:%s\n",
          src_trunc);
      return NULL;
    }
    source++;

    return (char *)source;
  }
  case '\"': {
    // string
    str_view_t value;
    str_view_init (&value, source, src_end - source);
    int ret = str_view_parse_str (&value, value);
    if (ret == -1) {
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("error parsing json string. stop here:%s\n", src_trunc);
      return NULL;
    }
    target->value.str = value;
    target->type = JSON_TYPE_STR;

    source += (value.len + 2);
    return (char *)source;
  }
  default: {
    // number
    str_view_t value;
    str_view_init (&value, source, src_end - source);
    int flag = str_view_tod (&value, &target->value.number);
    if (flag == -1) {
      return NULL;
    }
    target->type = JSON_TYPE_FLOAT;
    // return the offset
    source += flag;
    return (char *)source;
  }
  }

  char src_trunc[33];
  strncpy (src_trunc, source, 32);
  src_trunc[32] = '\0';
  printf ("error parsing json value. stop here:%s\n", src_trunc);
  return NULL;
}

/* the core parser.
 *
 * returns:
 *   -1: error
 *  otherwise: the offset
 */
__HIDDEN char *
__json_parse (struct json *target, const char *source, size_t length,
              int __cur_depth) {
  if (source == NULL || length == 0) {
    return NULL;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    return NULL;
  }

  char *src_end = (char *)source + length;
  if (unlikely (*source != '{')) {
    // maybe you mean to parse json array?
    if (*source == '[') {
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("trying to parse json object, but found json array. "
                   "stop here:%s\n",
                   src_trunc);
    }
    return -1;
  }
  source++;

  // special case: check if the object is empty
  source = strskip (source, src_end - source);
  if (*source == '}') {
    return (char *)source;
  }

  /* parse the string */
  while (1) {
    struct json_obj new_obj;
    source
        = __json_obj_parse (&new_obj, source, src_end - source, __cur_depth);
    if (source == NULL) {
      return NULL;
    }

    // add the new obj to the target
    json_add (target, &new_obj);

    // skip the comma
    source = strskip (source, src_end - source);
    if (*source == ',') {
      source++;
      continue;
    }

    // skip the right brace
    source = strskip (source, src_end - source);
    if (*source == '}') {
      return (char *)source;
    }
  }
}

__EXPOSED char *
__json_arr_parse (struct json *target, const char *source, size_t length,
                  int __cur_depth) {
  if (source == NULL || length == 0) {
    return NULL;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    return NULL;
  }

  char *src_end = (char *)source + length;
  if (unlikely (*source != '[')) {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json array: unmatched bracket. stop here:%s\n",
                 src_trunc);
    return NULL;
  }
  source++;

  // special case: check if the object is empty
  source = strskip (source, src_end - source);
  if (*source == ']') {
    return (char *)source;
  }

  /* parse the string */
  while (1) {
    struct json_obj *new_obj = _json_add_empty (target, NULL);
    new_obj->key.str = NULL;
    // skip
    source = strskip (source, src_end - source);

    char *_source
        = json_value_parse (new_obj, source, src_end - source, __cur_depth);
    if (_source == NULL) {
      size_t index = json_get_size (target) - 1;
      printf ("size-1: %d\n", index);
      json_remove_by_index (target, index);
      char src_trunc[33];
      strncpy (src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("error parsing json array. stop here:%s\n", src_trunc);
      return NULL;
    }
    source = _source;

    // skip the comma
    source = strskip (source, src_end - source);
    if (*source == ',') {
      source++;
      continue;
    }

    // skip the right brace
    source = strskip (source, src_end - source);
    if (*source == ']') {
      return (char *)source;
    }
  }
}

/* parse the target recursively. NEVER take ownership of the source.
 * returns:
 *    -1: error
 *    otherwise: the offset
 *
 * example:
 * >>> char* source = "\"key\" : \"\u0020\"IT WOULD STOP HERE "; size_t length
 * = strlen (source);
 * >>> int ret = json_obj_parse (target, source, length);
 * >>> printf ("%s\n", source + ret);
 *
 * output:
 * >>>IT WOULD STOP HERE
 *
 */
char *
__json_obj_parse (struct json_obj *target, const char *source, size_t length,
                  int __cur_depth) {
  if (source == NULL || length == 0) {
    return NULL;
  }

  char *src_end = (char *)source + length;

  /* skip as usual */
  source = strskip (source, length);
  if (*source != '\"') {
    return NULL;
  }
  // length = src_end - source;

  /* parse the string */
  str_view_t key;
  str_view_init (&key, source, src_end - source);
  int flag = str_view_parse_str (&key, key);
  if (flag == -1) {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: invalid key. stop here:%s\n",
                 src_trunc);
    return NULL;
  }
  target->key = key;
  source += key.len + 2;

  /* skip the string */
  source = strskip (source, src_end - source);
  if (*source != ':') {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: missing colon. stop here:%s\n",
                 src_trunc);
    return NULL;
  }
  source++;

  /* skip the colon */
  source = strskip (source, src_end - source);
  if (*source == '\0') {
    char src_trunc[33];
    strncpy (src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: unexpected end. stop here:%s\n",
                 src_trunc);
    return NULL;
  }

  source = json_value_parse (target, source, src_end - source, __cur_depth);
  if (source == NULL) {
    return NULL;
  }
  return (char *)source;
}
