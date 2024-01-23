#include "json_obj.h"
#include "json_store.h"

// destroy the json object
__EXPOSED int
json_obj_destroy (struct json_obj *obj) {
  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    obj->value.object = NULL;
  }

  if (json_obj_owns_source (obj)) {
    debug_print ("freed source=%s\n", obj->__source);
    json_global_hooks.free_fn (obj->__source);
  }

  obj->type = JSON_TYPE_NULL;

  return 0;
}

/* --- prints --- */
__HIDDEN void
__change_line (int flag, int __cur_depth) {
  static const int FLAG_NO_INDENT = 1;
  static const int FLAG_INDENT_BY_2 = 2;
  static const int FLAG_INDENT_BY_4 = 4;
  if (flag & FLAG_NO_INDENT) {
    return;
  }

  if (flag & FLAG_INDENT_BY_2) {
    printf ("\n%*s", __cur_depth * 2, "");
  } else if (flag & FLAG_INDENT_BY_4) {
    printf ("\n%*s", __cur_depth * 4, "");
  } else {
    printf ("\n");
  }
}

__EXPOSED void
__json_obj_print (struct  json_obj *obj, int flag, int __cur_depth) {
  __change_line (flag, __cur_depth);
  // print key?
  if (obj->key.str != NULL) {
    printf ("\"%.*s\":", (int) obj->key.len, obj->key.str);
  }

  if (obj->type & JSON_TYPE_OBJECT) {
    printf ("{");
    struct json *json = obj->value.object;

    if (json_is_empty (json)) {
      printf ("}");
      return;
    }

    for (struct json_obj *cur = json_begin (json); 
         cur != json_end (json); 
         cur = json_next (json, cur)) 
    {
      __json_obj_print (cur, flag, __cur_depth + 1);
      if (json_next (json, cur)) {
        printf (",");
      }
    }

    printf ("}");
  } else if (obj->type & JSON_TYPE_ARRAY) {
    printf ("[");
    struct json *json = obj->value.array;
    
    for (struct json_obj *cur = json_begin (json); 
         cur != json_end (json); 
         cur = json_next (json, cur)) 
    {
      __json_obj_print (cur, flag, __cur_depth + 1);
      if (json_next (json, cur)) {
        printf (",");
      }
    }
    printf ("]");
  } else if (obj->type & JSON_TYPE_STR) {
    printf ("\"%.*s\"", (int) obj->value.str.len, obj->value.str.str);
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    printf ("\"%s\"", obj->value.lz_str);
  } else if (obj->type & JSON_TYPE_FLOAT) {
    printf ("%f", obj->value.number);
  } else if (obj->type & JSON_TYPE_BOOLEAN) {
    if (obj->value.boolean) {
      printf ("true");
    } else {
      printf ("false");
    }
  } else if (obj->type & JSON_TYPE_NULL) {
    printf ("null");
  // } else if (obj->type & JSON_TYPE_RAW) {
  //   printf ("%.*s", (int) obj->value.str.len, obj->value.str.str);
  }
}

/* --- setters --- */
/* All string setters with:
 * 1. const char *str input would try to make a copy of the string.
 *    - If the string is short enough obj value, it would be a lazy string.
 *    If the string is long, it would be a normal string, and the json object would be the owner.
 * 
 * 2. str_view_t view input would not make a copy of the string.
 *   The json object would never be the owner of the string.
*/

// utils
__HIDDEN int
__json_obj_check_destroy_value_str (struct json_obj *obj) {
  // check if the value is a string
  if (!(obj->type & JSON_TYPE_STR))  {
    return 0;
  }

  // key in __source?
  if (json_obj_owns_key (obj)) {
    obj->type &= ~JSON_TYPE_STR;
    return 0;
  } else if (json_obj_owns_source (obj)) {
    // okay to free
    debug_print ("freed source=%s\n", obj->__source);
    json_global_hooks.free_fn (obj->__source);
    obj->__source = NULL;
    obj->__source_len = 0;
    obj->type &= ~ (__JSON_OWNS_SOURCE | JSON_TYPE_STR);
  }

  return 0;
}

__HIDDEN int
__json_obj_set_type (struct json_obj *obj, int type) {
  // conserve the ownership
  if (json_obj_owns_source (obj)) {
    obj->type = (__JSON_OWNS_SOURCE | type);
  } else {
    obj->type = type;
  }

  return 0;
}

/* set key name
*/
__EXPOSED int
json_obj_set_key (struct json_obj *obj, const char *key) {
  // allows null key
  // in this case this becomes an array element
  if (key == NULL) {
    obj->key.str = NULL;
    obj->key.len = 0;
    return 0;
  }

  if (json_obj_owns_value(obj)) {
    debug_print ("obj=%p, source=%s\n", obj, obj->__source);
    // check if there is still enough room for the key
    if (strlen (key) + obj->value.str.len + 1 > obj->__source_len) {
      // not enough room
      // make a copy
      char *tmp = json_global_hooks.malloc_fn (strlen (key) + obj->value.str.len + 1);
      debug_print ("malloced source for %s\n", key);
      if (tmp == NULL) {
        return -1;
      }
      strcpy (tmp, key);
      strcpy (tmp + strlen (key), obj->value.str.str);

      debug_print ("freed source=%s\n", obj->__source);
      json_global_hooks.free_fn (obj->__source);
      obj->__source = tmp;
      obj->__source_len = strlen (key) + obj->value.str.len + 1;
      // obj->type |= __JSON_OWNS_SOURCE; // lol, already a source owner
    } else {
      // enough room, move the value to the end
      memmove (obj->__source + strlen (key), obj->value.str.str, obj->value.str.len);
      strncpy (obj->__source, key, strlen (key));
      // obj->type |= __JSON_OWNS_SOURCE; // lol, already a source owner
    }

    str_view_init (&obj->key, obj->__source, strlen (key));
    return 0;
  }

  if (json_obj_owns_key(obj) && obj->__source_len>=strlen(key)) {
    // enough room, just copy
    strncpy (obj->__source, key, strlen (key));
    obj->key.str = obj->__source;
    obj->key.len = strlen (key);
    return 0;
  }

  // not enough room, make a copy
  if (json_obj_owns_key(obj)) {
    debug_print ("freed source=%s\n", obj->__source);
    json_global_hooks.free_fn (obj->__source);
    obj->__source = NULL;
    obj->__source_len = 0;
    obj->type &= ~__JSON_OWNS_SOURCE;
  }

  char *tmp = json_global_hooks.malloc_fn (strlen (key) + 1);
  debug_print ("malloced source for %s\n", key);
  if (tmp == NULL) {
    return -1;
  }

  strcpy (tmp, key);
  str_view_init (&obj->key, tmp, strlen (key));

  obj->__source = tmp;
  obj->__source_len = strlen (key) + 1;
  obj->type |= __JSON_OWNS_SOURCE;

  return 0;
}

__EXPOSED int
json_obj_set_key_by_view (struct json_obj *obj, str_view_t view) {
  obj->key = view;

  // if only owns key, free it.
  if (json_obj_owns_key (obj) && !json_obj_owns_value (obj)) {
    json_global_hooks.free_fn (obj->__source);
    obj->__source = NULL;
    obj->__source_len = 0;
    obj->type &= ~__JSON_OWNS_SOURCE;
  }
 
  return 0;
}


__EXPOSED int
json_obj_set_null (struct json_obj *obj) {
  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    obj->value.object = NULL;
  }

  __json_obj_check_destroy_value_str (obj);
  __json_obj_set_type (obj, JSON_TYPE_NULL);

  return 0;
}

__EXPOSED int
json_obj_set_number (struct json_obj *obj, double number) {
  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    obj->value.object = NULL;
  }

  __json_obj_check_destroy_value_str (obj);
  __json_obj_set_type (obj, JSON_TYPE_FLOAT);

  
  obj->value.number = number;
  return 0;
}

__EXPOSED int
json_obj_set_boolean (struct json_obj *obj, int boolean) {
  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    obj->value.object = NULL;
  }

  __json_obj_check_destroy_value_str (obj);
  __json_obj_set_type (obj, JSON_TYPE_BOOLEAN);

  obj->value.boolean = boolean>0?1:0;
  return 0;
}


/* set the source string as the value of the json object
 * this function would make a string copy and take ownership of the copy.
 * Short enough string would be lazy string, and does not have the ownership.
 * If you do not want to make a copy, use json_obj_set_str_by_view instead.
*/
__EXPOSED int
json_obj_set_str (struct json_obj *obj, const char *str) {
  if (str == NULL) {
    return -1;
  }

  int be_lazy_str = 0;
  if (strlen(str)<=__JSON_OBJ_LZ_STR_LEN) {
    be_lazy_str = 1;
  }

  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    obj->value.object = NULL;
  }
  
  if (be_lazy_str) {
    __json_obj_check_destroy_value_str (obj);
    __json_obj_set_type (obj, JSON_TYPE_LZ_STR);
    strcpy (obj->value.lz_str, str);
    return 0;
  }

  // not lazy string. take ownership.
  int str_len = strlen (str);
  // case 1: __source contains key.
  // in this case, we need to make a copy of the source string.
  // now the source contains the key and the value.
  if (json_obj_owns_key (obj)) {
    str_len += obj->key.len;

    char *tmp = json_global_hooks.malloc_fn (str_len + 1);
    if (tmp == NULL) {
      return -1;
    }
    strncpy (tmp, obj->key.str, obj->key.len);

    json_global_hooks.free_fn (obj->__source);
    obj->__source = tmp;
    obj->__source_len = str_len + 1;

    strncpy (tmp + obj->key.len, str, strlen (str));
    tmp[str_len] = '\0';
    str_view_init (&obj->value.str, tmp + obj->key.len, strlen (str));
    __json_obj_set_type (obj, JSON_TYPE_STR | __JSON_OWNS_SOURCE);
    return 0;
  }

  // default case: okay to take ownership
  char *tmp = json_global_hooks.malloc_fn (str_len + 1);
  if (tmp == NULL) {
    return -1;
  }

  if (json_obj_owns_source (obj)) {
    json_global_hooks.free_fn (obj->__source);
    obj->__source = NULL;
    obj->__source_len = 0;
    obj->type &= ~__JSON_OWNS_SOURCE;
  }

  obj->__source = tmp;
  strcpy (obj->__source, str);
  obj->__source_len = strlen (str) + 1;

  __json_obj_set_type (obj, JSON_TYPE_STR | __JSON_OWNS_SOURCE);
  
  str_view_init (&obj->value.str, obj->__source, obj->__source_len - 1);
  return 0;
}

__EXPOSED int
json_obj_set_str_by_view (struct json_obj *obj, const str_view_t view) {
  // never take ownership of the source string
  if (json_obj_has_children (obj)) {
    json_destroy (obj->value.object);
    // free
    json_global_hooks.free_fn (obj->value.object);
    obj->value.object = NULL;
  }

  __json_obj_check_destroy_value_str (obj);
  __json_obj_set_type (obj, JSON_TYPE_STR);

  obj->value.str = view;


  return 0;
}

/* --- getters --- */
__EXPOSED str_view_t
json_obj_get_key (struct json_obj *obj) {
  return obj->key;
}

__EXPOSED int
json_obj_get_key_len (struct json_obj *obj) {
  return obj->key.len;
}

__EXPOSED str_view_t
json_obj_get_value_str (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_STR) {
    return obj->value.str;
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    // pack the lazy string into a str_view_t
    str_view_t view;
    view.str = obj->value.lz_str;
    view.len = strlen (obj->value.lz_str);
    return view;

  } else {
    str_view_t view;
    view.str = NULL;
    view.len = 0;
    return view;
  }
}

__EXPOSED int
json_obj_get_value_str_len (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_STR) {
    return obj->value.str.len;
  } else if (obj->type & JSON_TYPE_LZ_STR) {
    return strlen (obj->value.lz_str);
  } else {
    return 0;
  }
}

__EXPOSED double
json_obj_get_value_number (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_FLOAT) {
    return obj->value.number;
  } else {
    return _json_num_nan;
  }
}

__EXPOSED struct json *
json_obj_get_value_object (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_OBJECT) {
    return obj->value.object;
  } else {
    return NULL;
  }
}

__EXPOSED struct json *
json_obj_get_value_array (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_ARRAY) {
    return obj->value.array;
  } else {
    return NULL;
  }
}

__EXPOSED size_t
json_obj_get_value_array_len (struct json_obj *obj) {
  if (obj->type & JSON_TYPE_ARRAY) {
    return json_get_size (obj->value.array);
  } else {
    return 0;
  }
}