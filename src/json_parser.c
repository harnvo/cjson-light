#include "__debug_tool.h"
#include "json_obj.h"
#include "json_store.h"
#include "json_parser.h"

#define __JSON_MAX_DEPTH 128

int
json_value_parse (struct json_obj *target, const char *source, size_t length, int __cur_depth) {
  if (source == NULL || length == 0) {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: invalid source. stop here:%s\n", src_trunc);
    return -1;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: too deep. stop here:%s\n", src_trunc);
    return -1;
  }

  char *src_end = (char*) source + length;

  /* parse the value */
  str_view_t value;
  str_view_init (&value, source, src_end - source);
  // special cases?
  // TODO: look a bit too messy here. refactor it later
  static const char *err = "json_obj_parse: invalid keyword value when parsing \
`%s`. Did you mean %s?\n";
  if( strncmp(source, "false", 5) == 0 ) {
    target->type = JSON_TYPE_BOOLEAN;
    target->value.boolean = 0;
    // return the offset
    source += 5;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element(target)) {
        fprintf(stderr, err, "array element", "false");
      } else {
        char _key[target->key.len+1];
        snprintf(_key, target->key.len+1, "%.*s", (int)target->key.len, target->key.str);
        fprintf(stderr, err, _key, "false");
      }
      while (*source != ',' && *source != '}' && *source != ']' && *source != '\0' && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy(src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf(stderr, "fatal: unmatched quote. stop here:%s\n", src_trunc);
          return -1;
        }
        source++;
      }
    }
    return length - (src_end - source);
  }

  if( strncmp(source, "true", 4) == 0 ) {
    target->type = JSON_TYPE_BOOLEAN;
    target->value.boolean = 1;
    // return the offset
    source += 4;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element(target)) {
        fprintf(stderr, err, "array element", "true");
      } else {
        char _key[target->key.len+1];
        snprintf(_key, target->key.len+1, "%.*s", (int)target->key.len, target->key.str);
        fprintf(stderr, err, _key, "true");
      }
      // find the next comma
      while (*source != ',' && *source != '}' && *source != '\0' && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy(src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf(stderr, "fatal: unmatched quote. stop here:%s\n", src_trunc);
          return -1;
        }
        source++;
      }
    }
    return length - (src_end - source);
  }

  if( strncmp(source, "null", 4) == 0) {
    target->type = JSON_TYPE_NULL;
    memset(&target->value, 0, sizeof(target->value));
    // return the offset
    source += 4;
    source = strskip (source, src_end - source);
    if (*source != ',' && *source != '}') {
      if (json_obj_is_array_element(target)) {
        fprintf(stderr, err, "array element", "null");
      } else {
        char _key[target->key.len+1];
        snprintf(_key, target->key.len+1, "%.*s", (int)target->key.len, target->key.str);
        fprintf(stderr, err, _key, "null");
      }
      while (*source != ',' && *source != '}' && *source != '\0' && source < src_end) {
        if (*source == '\"') {
          char src_trunc[33];
          strncpy(src_trunc, source, 32);
          src_trunc[32] = '\0';
          fprintf(stderr, "fatal: unmatched quote. stop here:%s\n", src_trunc);
          return -1;
        }
        source++;
      }
    }
    return length - (src_end - source);
  }

  // TODO: finish the value parser
  switch (*source) {
    case '{': {
      // object
      struct json *new_json = (struct json *)json_global_hooks.malloc_fn (sizeof (struct json));
      // choose the storage type
      json_list_storage_init(new_json);
      int ret = __json_parse (new_json, source, src_end - source, __cur_depth + 1);
      if (ret == -1) {
        char src_trunc[33];
        strncpy(src_trunc, source, 32);
        src_trunc[32] = '\0';
        raise_error ("error parsing json object. stop here:%s\n", src_trunc);
        return -1;
      }

      target->value.object = new_json;
      target->type = JSON_TYPE_OBJECT;
      
      source += ret;
      // skip the right brace
      source = strskip (source, src_end - source);
      if (*source != '}') {
        char src_trunc[33];
        strncpy(src_trunc, source, 32);
        src_trunc[32] = '\0';
        raise_error ("error parsing json object: unmatched bracket. stop here:%s\n", src_trunc);
        return -1;
      }
      source++;

      return length - (src_end - source);
    }
    case '[': {
      // array
      struct json *new_json = (struct json *)json_global_hooks.malloc_fn (sizeof (struct json));
      json_list_storage_init(new_json);
      int ret = __json_arr_parse (new_json, source, src_end - source, __cur_depth + 1);
      if (ret == -1) {
        // no more warnings, we already have the error message
        raise_error ("traceback%s.\n", "");
        return -1;
      }
      target->value.array = new_json;
      target->type = JSON_TYPE_ARRAY;

      source += ret;
      // skip the right brace
      source = strskip (source, src_end - source);
      if (*source != ']') {
        char src_trunc[33];
        strncpy(src_trunc, source, 32);
        src_trunc[32] = '\0';
        raise_error ("error parsing json array: unmatched bracket. stop here:%s\n", src_trunc);
        return -1;
      }
      source++;

      return length - (src_end - source);
    }
    case '\"': {
      // string
      str_view_t value;
      str_view_init (&value, source, src_end - source);
      int ret = str_view_parse_str (&value, value);
      if (ret == -1) {
        char src_trunc[33];
        strncpy(src_trunc, source, 32);
        src_trunc[32] = '\0';
        raise_error ("error parsing json string. stop here:%s\n", src_trunc);
        return -1;
      }
      target->value.str = value;
      target->type = JSON_TYPE_STR;

      source += (value.len + 2);
      return length - (src_end - source);
    }
    default: {
    // number
      str_view_t value;
      str_view_init (&value, source, src_end - source);
      int flag = str_view_tod (&value, &target->value.number);
      if (flag == -1) {
        return -1;
      }
      target->type = JSON_TYPE_FLOAT;
      // return the offset
      source += flag;
      return length - (src_end - source);
    }
  }

  char src_trunc[33];
  strncpy(src_trunc, source, 32);
  src_trunc[32] = '\0';
  printf ("error parsing json value. stop here:%s\n", src_trunc);
  return -1;
}

/* parse the target recursively. NEVER take ownership of the source.
 * returns:
 *    -1: error
 *    otherwise: the offset
 * 
 * example:
 * >>> char* source = "\"key\" : \"\u0020\"IT WOULD STOP HERE "; size_t length = strlen (source);
 * >>> int ret = json_obj_parse (target, source, length);
 * >>> printf ("%s\n", source + ret);
 * 
 * output:
 * >>>IT WOULD STOP HERE 
 *
*/
int
__json_obj_parse (struct json_obj *target, const char *source, size_t length, int __cur_depth) {
  if (source == NULL || length == 0) {
    return -1;
  }

  char *src_end = (char*) source + length;

  /* skip as usual */
  source = strskip (source, length);
  if (*source != '\"') {
    return -1;
  }
  // length = src_end - source;


  /* parse the string */
  str_view_t key;
  str_view_init (&key, source, src_end - source);
  int flag = str_view_parse_str (&key, key);
  if (flag == -1) {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: invalid key. stop here:%s\n", src_trunc);
    return -1;
  }

  target->key = key;
  source += key.len + 2;

  /* skip the string */
  source = strskip (source, src_end - source);
  if (*source != ':') {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: missing colon. stop here:%s\n", src_trunc);
    return -1;
  }
  source++;

  /* skip the colon */
  source = strskip (source, src_end - source);
  if (*source == '\0') {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json object: unexpected end. stop here:%s\n", src_trunc);
    return -1;
  }

  int ret = json_value_parse (target, source, src_end - source, __cur_depth);
  if (ret == -1) {
    return -1;
  }
  source += ret;
  return length - (src_end - source);
}


/* the core parser.
 *
 * returns:
 *   -1: error
 *  otherwise: the offset
 */  
__HIDDEN int
__json_parse (struct json *target, const char *source, size_t length, int __cur_depth) {
  if (source == NULL || length == 0) {
    return -1;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    return -1;
  }
  
  char *src_end = (char*) source + length;
  if ( unlikely(*source != '{') ) {
    // maybe you mean to parse json array?
    if (*source == '[') {
      char src_trunc[33];
      strncpy(src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("trying to parse json object, but found json array. stop here:%s\n", src_trunc);
    }
    return -1;
  }
  source++;

  // special case: check if the object is empty
  source = strskip (source, src_end - source);
  if (*source == '}') {
    return length - (src_end - source);
  }

  /* parse the string */
  while (1) {
    struct json_obj new_obj;
    int offset = __json_obj_parse (&new_obj, source, src_end - source, __cur_depth + 1);
    if (offset == -1) {
      return -1;
    }
    source += offset;

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
      return length - (src_end - source);
    }
  }

}

__EXPOSED int
__json_arr_parse (struct json *target, const char *source, size_t length, int __cur_depth) {
  if (source == NULL || length == 0) {
    return -1;
  }
  if (__cur_depth > __JSON_MAX_DEPTH) {
    return -1;
  }
  
  char *src_end = (char*) source + length;
  if ( unlikely(*source != '[') ) {
    char src_trunc[33];
    strncpy(src_trunc, source, 32);
    src_trunc[32] = '\0';
    raise_error ("error parsing json array: unmatched bracket. stop here:%s\n", src_trunc);
    return -1;
  }
  source++;

  // special case: check if the object is empty
  source = strskip (source, src_end - source);
  if (*source == ']') {
    return length - (src_end - source);
  }

  /* parse the string */
  while (1) {
    // TODO: is add_empty a good idea here?
    struct json_obj new_obj;
    new_obj.key.str = NULL;
    // skip
    source = strskip (source, src_end - source);

    int offset = json_value_parse (&new_obj, source, src_end - source, __cur_depth);
    if (offset == -1) {
      char src_trunc[33];
      strncpy(src_trunc, source, 32);
      src_trunc[32] = '\0';
      raise_error ("error parsing json array. stop here:%s\n", src_trunc);
      return -1;
    }
    source += offset;

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
    if (*source == ']') {
      return length - (src_end - source);
    }
  }
}
