#pragma once

#include "json_obj.h"
#include "shared.h"
#include "str_view.h"

#include <stdio.h>
#include <stdlib.h>

// define storage place for json_obj

#if defined(__cplusplus)
extern "C" {
#endif

struct _json_op {
  int (*add) (struct json *json, struct json_obj *obj);
  struct json_obj *(*_add_empty) (struct json *json, str_view_t *key);
  // int (*remove) (struct json *json, struct json_obj *obj);
  int (*remove_by_key) (struct json *json, str_view_t *key);
  int (*remove_by_index) (struct json *json, size_t index);

  struct json_obj *(*get_by_key) (struct json *json, str_view_t *key);
  struct json_obj *(*get_by_index) (struct json *json, size_t index);

  void (*destroy) (struct json *json);

  /* size */
  size_t (*size) (const struct json *json);

  /* a iterator for convenience. */
  struct json_obj *(*begin) (const struct json *json);
  struct json_obj *(*next) (const struct json *json, struct json_obj *obj);
  struct json_obj *(*end) (const struct json *json);

  /* extra op for internal use.
   * If you want to use it, you should know what you are doing.
   * currently unused.
   */
  void (*__remove_by_obj) (struct json *json, struct json_obj *obj);
};

struct json_list_storage;
struct json_hash_table_storage;
struct json_array_storage;

struct json;

/* ---------------------- */
/* - storage definition - */
/* ---------------------- */
struct json_list_storage_node;
typedef struct json_list_storage {
  struct json_list_storage_node *head;
  struct json_list_storage_node *tail;
  size_t size;

} json_list_storage_t;

typedef struct json_array_storage {
  struct json_obj *beg;
  struct json_obj *end;
  struct json_obj *end_of_storage;
} json_array_storage_t;

struct json_hash_table_bucket;
typedef struct json_hash_table_storage {
  int num_buckets; // bucket is not gonna be that big, right..?
  size_t num_elements;

  struct json_hash_table_bucket *buckets;

  // an internal context holder for iterator
  int __cur_bucket;
  // struct json_obj *__cur_obj;
} json_hash_table_storage_t;

struct json {
  struct _json_op *_op;
  union {
    struct json_list_storage list;
    struct json_hash_table_storage hash_table;
    struct json_array_storage array;
  } _storage;
};

__EXPOSED __HEADER_ONLY int
json_add (struct json *json, struct json_obj *obj) {
  return json->_op->add (json, obj);
}

__EXPOSED __HEADER_ONLY struct json_obj *
_json_add_empty (struct json *json, str_view_t *key) {
  return json->_op->_add_empty (json, key);
}

__EXPOSED __HEADER_ONLY int
json_remove (struct json *json, const char *key) {
  // make a str_view_t
  str_view_t _view = { (char *)key, strlen (key) };
  return json->_op->remove_by_key (json, &_view);
}

__EXPOSED __HEADER_ONLY int
json_remove_by_view (struct json *json, str_view_t *key) {
  return json->_op->remove_by_key (json, key);
}

__EXPOSED __HEADER_ONLY int
json_remove_by_index (struct json *json, size_t index) {
  return json->_op->remove_by_index (json, index);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_get (struct json *json, const char *key) {
  // make a str_view_t
  str_view_t _view = { (char *)key, strlen (key) };
  return json->_op->get_by_key (json, &_view);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_get_by_view (struct json *json, str_view_t key) {
  return json->_op->get_by_key (json, &key);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_index (struct json *json, size_t index) {
  return json->_op->get_by_index (json, index);
}

__EXPOSED __HEADER_ONLY void
json_destroy (struct json *json) {
  json->_op->destroy (json);
}

__EXPOSED __HEADER_ONLY size_t
json_get_size (const struct json *json) {
  return json->_op->size (json);
}

__EXPOSED __HEADER_ONLY int
json_is_empty (const struct json *json) {
  return json_get_size (json) == 0;
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_begin (const struct json *json) {
  return json->_op->begin (json);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_next (const struct json *json, struct json_obj *obj) {
  return json->_op->next (json, obj);
}

__EXPOSED __HEADER_ONLY struct json_obj *
json_end (const struct json *json) {
  return json->_op->end (json);
}

// iterative print
__HIDDEN __HEADER_ONLY void
__json_print (const struct json *json, int flags, int __cur_depth) {
  struct json_obj *obj = json_begin (json);
  while (obj != json_end (json)) {
    __json_obj_print (obj, flags, __cur_depth);
    obj = json_next (json, obj);
  }
}

__EXPOSED __HEADER_ONLY void
json_print (const struct json *json, int flags) {
  __json_print (json, flags, 0);
}

/* --- list storage --- */
int json_list_storage_init (struct json *json);

int json_array_storage_init (struct json *json, int __default_size);

// void json_list_storage_destroy (struct json *json);

#if defined(__cplusplus)
}
#endif