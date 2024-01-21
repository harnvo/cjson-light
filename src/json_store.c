#include "json_store.h"
#include "__debug_tool.h"

/* -------------------- */
/* --- list storage --- */
/* -------------------- */

typedef struct json_list_storage_node {
  struct json_obj obj; // no matter what node is, it should never be a pointer
                       // to json_obj!
  struct json_list_storage_node *next;
  struct json_list_storage_node *prev;
} json_list_storage_node_t;

typedef struct json_list_storage {
  struct json_list_storage_node *head;
  struct json_list_storage_node *tail;
  size_t size;

} json_list_storage_t;

int json_list_storage_init (struct json *json, json_list_storage_t *storage);

void json_list_storage_destroy (struct json *json);

__HIDDEN
int json_list_storage_add (struct json *json, struct json_obj *obj);

__HIDDEN
struct json_obj *json_list_storage_add_empty (struct json *json);

__HIDDEN
int json_list_storage_remove_by_key (struct json *json, str_view_t *key);

__HIDDEN
int json_list_storage_remove_by_index (struct json *json, size_t index);

__HIDDEN
struct json_obj *json_list_storage_get_by_key (struct json *json, str_view_t *key);

__HIDDEN
struct json_obj *json_list_storage_get_by_index (struct json *json, size_t index);

__HIDDEN
size_t json_list_storage_size (const struct json *json);

__HIDDEN
struct json_obj *json_list_storage_begin (const struct json *json);

__HIDDEN
struct json_obj *json_list_storage_next (const struct json *json, struct json_obj *obj);

__HIDDEN
struct json_obj *json_list_storage_end (const struct json *json);

/* ------------------------ */

struct _json_op json_op_list_storage = {
  ._add_empty = json_list_storage_add_empty,
  .remove_by_key = json_list_storage_remove_by_key,
  .remove_by_index = json_list_storage_remove_by_index,
  .get_by_key = json_list_storage_get_by_key,
  .get_by_index = json_list_storage_get_by_index,
  .destroy = json_list_storage_destroy,

  .size = json_list_storage_size,

  .begin = json_list_storage_begin,
  .next = json_list_storage_next,
  .end = json_list_storage_end,
};

/* --- init&destory --- */

int
json_list_storage_init (struct json *json, json_list_storage_t *storage) {
  if (storage) {
    json->storage = storage;
  } else {
    json->storage = json_global_hooks.malloc_fn (sizeof (json_list_storage_t));
    if (!json->storage) {
      return -1;
    }
    storage = (json_list_storage_t *)json->storage;
  }

  storage->head = NULL;
  storage->tail = NULL;
  storage->size = 0;

  json->storage = storage;

  json->_op = &json_op_list_storage;

  return 0;
}

void
json_list_storage_destroy (struct json *json) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node = storage->head;
  json_list_storage_node_t *next_node = NULL;

  debug_print ("node=%p\n", node);

  while (node) {
    next_node = node->next;
    // nested json?
    if (!json_obj_is_array_element (&node->obj)) {
      debug_print ("json_key=%.*s\n", (int) node->obj.key.len, node->obj.key.str);
    }
    if (node->obj.type == JSON_TYPE_OBJECT
        || node->obj.type == JSON_TYPE_ARRAY) {    
      debug_print ("destroy nested json%s.\n", "");
      json_destroy (node->obj.value.object);
      json_global_hooks.free_fn (node->obj.value.object);
    }

    if (json_obj_owns_source (&node->obj)) {
      debug_print ("free source=%p\n", node->obj.value.str.str);
      json_global_hooks.free_fn (node->obj.__source);
    }

    // destroy json_obj
    // json_obj_destroy (&node->obj);
    json_global_hooks.free_fn (node);
    node = next_node;
  }

  

  // TODO: what if storage is not allocated by malloc?
  json_global_hooks.free_fn (storage);
  json->storage = NULL;

  return;
}

/* --- basic operations --- */

__HIDDEN int
json_list_storage_add (struct json *json, struct json_obj *obj) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *new_node
      = (json_list_storage_node_t *)json_global_hooks.malloc_fn (
          sizeof (json_list_storage_node_t));
  if (!new_node) {
    return -1;
  }

  memcpy (&new_node->obj, obj, sizeof (struct json_obj));
  new_node->next = NULL;
  new_node->prev = NULL;

  if (storage->size == 0) {
    storage->head = new_node;
    storage->tail = new_node;
    storage->size = 1;
  }

  storage->tail->next = new_node;
  new_node->prev = storage->tail;
  storage->tail = new_node;
  storage->size++;

  return 0;
}

__HIDDEN struct json_obj *
json_list_storage_add_empty (struct json *json) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *new_node
      = (json_list_storage_node_t *)json_global_hooks.malloc_fn (
          sizeof (json_list_storage_node_t));
  if (!new_node) {
    return NULL;
  }

  new_node->next = NULL;
  new_node->prev = NULL;

  if (storage->size == 0) {
    storage->head = new_node;
    storage->tail = new_node;
    storage->size = 1;
  }

  storage->tail->next = new_node;
  new_node->prev = storage->tail;
  storage->tail = new_node;
  storage->size++;

  return &new_node->obj;

}

__HIDDEN int
json_list_storage_remove_by_key (struct json *json, str_view_t *key) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node = storage->head;

  while (node) {
    // warning: not a good one. need to be rewritten to support
    // case-insensitive json key
    if (str_view_cmp (&node->obj.key, key) != 0) {
      node = node->next;
      continue;
    }

    // remove node
    if (node->prev) {
      node->prev->next = node->next;
    } else {
      storage->head = node->next;
    }

    // has child?
    if (node->obj.type == JSON_TYPE_OBJECT
        || node->obj.type == JSON_TYPE_ARRAY) {
      json_destroy (node->obj.value.object);
    }

    storage->size--;
    json_global_hooks.free_fn (node);
    return 0;
  }

  return -1;
}

__HIDDEN int
json_list_storage_remove_by_index (struct json *json, size_t index) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node = storage->head;

  while (node) {
    if (index == 0) {
      // remove node
      if (node->prev) {
        node->prev->next = node->next;
      } else {
        storage->head = node->next;
      }

      storage->size--;
      json_global_hooks.free_fn (node);
      return 0;
    }

    node = node->next;
    index--;
  }

  return -1;
}

__HIDDEN struct json_obj *
json_list_storage_get_by_key (struct json *json, str_view_t *key) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node = storage->head;

  while (node) {
    // warning: not a good one. need to be rewritten to support
    // case-insensitive json key
    if (str_view_cmp (&node->obj.key, key) != 0) {
      node = node->next;
      continue;
    }

    return &node->obj;
  }

  return NULL;
}

__HIDDEN struct json_obj *
json_list_storage_get_by_index (struct json *json, size_t index) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node = storage->head;

  while (node) {
    if (index == 0) {
      return &node->obj;
    }

    node = node->next;
    index--;
  }

  return NULL;
}

__HIDDEN size_t
json_list_storage_size (const struct json *json) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  return storage->size;
}

/* an internal iterator */
__HIDDEN json_list_storage_node_t *
__json_list_storage_begin (const struct json *json) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  return storage->head;
}

__HIDDEN json_list_storage_node_t *
__json_list_storage_next (const struct json *json, json_list_storage_node_t *node) {
  return node->next;
}

__HIDDEN json_list_storage_node_t *
__json_list_storage_end (const struct json *json) {
  return NULL;
}

/* iterator exposed to _json_op */
__HIDDEN struct json_obj *
json_list_storage_begin (const struct json *json) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  return &storage->head->obj;
}

__HIDDEN struct json_obj *
json_list_storage_next (const struct json *json, struct json_obj *obj) {
  json_list_storage_t *storage = (json_list_storage_t *)json->storage;
  json_list_storage_node_t *node;

#ifdef __DEBUG
  // debug checktool to prevent user from using a obj not inside the storage
  node = storage->head;
  while (node) {
    if (&node->obj == obj) {
      goto check_passed;
    }
    node = node->next;
  }
  debug_print ("[json_list_storage_next] check failed: obj=%p not inside storage node!\n", obj);
  return NULL;
check_passed:
#endif

  // I am doing a hack here: we already know the obj is in the storage node,
  // and obj is the first member of json_list_storage_node_t, so we can
  // just force convert it to json_list_storage_node_t and get the next node
  // Not sure if this is a good idea, but it works anyway :)

  node = (json_list_storage_node_t *)obj;
  node = node->next;
  if (!node) {
    return json_list_storage_end (json);
  }

  return &node->obj;
}

__HIDDEN struct json_obj *
json_list_storage_end (const struct json *json) {
  return NULL;
}

/* ------------------- */
/* --- rbt storage --- */
/* ------------------- */

#ifdef __cplusplus
}
#endif

/* --- vector atorage for cpp --- */
// TODO: implement&test this
#ifdef __cplusplus
#include <vector>

template <typename _Alloc = std::allocator<struct json_obj *> >
struct json_vector_storage {
  std::vector<struct json_obj *, _Alloc> *vec;
};

typedef struct json_vector_storage<json_allocator<struct json_obj *> >
    json_vector_storage_t;

__HEADER_ONLY int
json_vector_storage_add (struct json *json, struct json_obj *obj) {
  json_vector_storage_t *storage = (json_vector_storage_t *)json->storage;
  storage->vec->push_back (obj);
  return 0;
}
#endif
