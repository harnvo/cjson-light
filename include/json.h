// this should be the ONLY include file for json-c

#include "json_obj.h"
#include "json_parser.h"
#include "json_store.h"

/* extra apis only revealed to user. */

__HEADER_ONLY struct json *json_new (char *json_str);

__HEADER_ONLY struct json *json_new2 (char *json_str, size_t len);

__HEADER_ONLY void json_delete (struct json *json);

__HEADER_ONLY struct json *
json_new (char *json_str) {
  return json_new2 (json_str, strlen (json_str));
}

__HEADER_ONLY struct json *
json_new2 (char *json_str, size_t len) {
  struct json *json
      = (struct json *)json_global_hooks.malloc_fn (sizeof (struct json));
  if (!json) {
    return NULL;
  }

  int ret = json_parse (json, json_str, len);
  if (ret < 0) {
    json_global_hooks.free_fn (json);
    return NULL;
  }

  return json;
}

__HEADER_ONLY void
json_delete (struct json *json) {
  json_destroy (json);
  json_global_hooks.free_fn (json);
}