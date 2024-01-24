#include "__debug_tool.h"
#include "json_store.h"

#define __DEFAULT_ARRAY_STORAGE_CAPACITY 8

/* --------------------- */
/* --- array storage --- */
/* --------------------- */

int json_array_storage_init (struct json *json, int __default_size);

void json_array_storage_destroy (struct json *json);

int json_array_storage_add (struct json *json, struct json_obj *obj);
struct json_obj *json_array_storage_add_empty (struct json *json,
                                               str_view_t *__key);

int json_array_storage_remove_by_key (struct json *json, str_view_t *key);
int json_array_storage_remove_by_index (struct json *json, size_t index);

struct json_obj *json_array_storage_get_by_key (struct json *json,
                                                str_view_t *key);
struct json_obj *json_array_storage_get_by_index (struct json *json,
                                                  size_t index);

size_t json_array_storage_size (const struct json *json);

struct json_obj *json_array_storage_begin (const struct json *json);
struct json_obj *json_array_storage_next (const struct json *json,
                                          struct json_obj *obj);
struct json_obj *json_array_storage_end (const struct json *json);

/* ------------------------ */

struct _json_op json_op_array_storage = {
  .add = json_array_storage_add,
  ._add_empty = json_array_storage_add_empty,
  .remove_by_key = json_array_storage_remove_by_key,
  .remove_by_index = json_array_storage_remove_by_index,
  .get_by_key = json_array_storage_get_by_key,
  .get_by_index = json_array_storage_get_by_index,
  .destroy = json_array_storage_destroy,

  .size = json_array_storage_size,

  .begin = json_array_storage_begin,
  .next = json_array_storage_next,
  .end = json_array_storage_end,
};

/* --- init&destory --- */

int
json_array_storage_init (struct json *json, int __default_size) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  if (__default_size <= 0) {
    __default_size = __DEFAULT_ARRAY_STORAGE_CAPACITY;
  }

  storage->beg = (struct json_obj *)json_global_hooks.malloc_fn (
      sizeof (struct json_obj) * __default_size);
  storage->end = storage->beg;
  storage->end_of_storage = storage->beg + __default_size;

  json->_op = &json_op_array_storage;
  return 0;
}

void
json_array_storage_destroy (struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  for (struct json_obj *p = storage->beg; p < storage->end; p++) {
    json_obj_destroy (p);
  }

  json_global_hooks.free_fn (storage->beg);
  return 0;
}

/* --- internal utils --- */
__HIDDEN int
json_array_storage_expand_capacity (struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  size_t cur_capacity = storage->end_of_storage - storage->beg;
  size_t cur_size = json_get_size (json);
  struct json_obj *new_objs = (struct json_obj *)json_global_hooks.malloc_fn (
      sizeof (struct json_obj) * cur_capacity * 2);

  memcpy (new_objs, storage->beg, sizeof (struct json_obj) * cur_size);
  json_global_hooks.free_fn (storage->beg);

  storage->beg = new_objs;
  storage->end = storage->beg + cur_size;
  storage->end_of_storage = storage->beg + cur_capacity * 2;
  return 0;
}

__HIDDEN int
json_array_storage_shrink_capacity (struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  size_t new_capacity = (storage->end_of_storage - storage->beg) / 2;
  struct json_obj *new_objs = (struct json_obj *)json_global_hooks.malloc_fn (
      sizeof (struct json_obj) * new_capacity);

  memcpy (new_objs, storage->beg,
          sizeof (struct json_obj) * (storage->end - storage->beg));
  json_global_hooks.free_fn (storage->beg);

  storage->beg = new_objs;
  storage->end_of_storage = storage->beg + new_capacity;
  return 0;
}

/* --- add --- */
int
json_array_storage_add (struct json *json, struct json_obj *obj) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  if (storage->end >= storage->end_of_storage) {
    json_array_storage_expand_capacity (json);
  }

  *storage->end = *obj;
  storage->end++;
  return 0;
}

struct json_obj *
json_array_storage_add_empty (struct json *json, str_view_t *__key) {
  // here __key is useless
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  if (storage->end >= storage->end_of_storage) {
    json_array_storage_expand_capacity (json);
  }

  struct json_obj *obj = storage->end;
  storage->end++;

  return obj;
}

/* --- remove --- */
int
json_array_storage_remove_by_key (struct json *json, str_view_t *key) {
  static const char *err_msg
      = "json_array_storage_remove_by_key: not supported";
  raise_error ("%s\n", err_msg);
  return -1;
}

int
json_array_storage_remove_by_index (struct json *json, size_t index) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  struct json_obj *obj = storage->beg + index;

  if (obj >= storage->end) {
    static const char *err_msg
        = "json_array_storage_remove_by_index: index out of range";
    raise_error ("%s\n", err_msg);

    return -1;
  }

  storage->end--;

  for (struct json_obj *p = obj; p < storage->end; p++) {
    *p = *(p + 1);
  }

  return 0;
}

/* --- get --- */
struct json_obj *
json_array_storage_get_by_key (struct json *json, str_view_t *key) {
  static const char *err_msg = "json_array_storage_get_by_key: not supported";
  raise_error ("%s\n", err_msg);
  return NULL;
}

struct json_obj *
json_array_storage_get_by_index (struct json *json, size_t index) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  struct json_obj *obj = storage->beg + index;

  if (obj >= storage->end) {
    static const char *err_msg
        = "json_array_storage_get_by_index: index out of range";
    raise_error ("%s\n", err_msg);
    return NULL;
  }

  return obj;
}

/* --- size --- */
size_t
json_array_storage_size (const struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;
  debug_print ("json_array_storage_size: %lu\n", storage->end - storage->beg);
  return storage->end - storage->beg;
}

/* --- iterator --- */
struct json_obj *
json_array_storage_begin (const struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;
  return storage->beg;
}

struct json_obj *
json_array_storage_next (const struct json *json, struct json_obj *obj) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;

  return obj + 1;
}

struct json_obj *
json_array_storage_end (const struct json *json) {
  json_array_storage_t *storage
      = (json_array_storage_t *)&json->_storage.array;
  return storage->end;
}

/* ---------------------- */
