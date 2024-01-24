#include "test.h"

int
main () {
  // open file
  FILE *fp = fopen ("/home/zichuan/json-c/test-cases/test1.json", "r");
  if (fp == NULL) {
    printf ("error\n");
    return -1;
  }

  // read the entire file
  fseek (fp, 0, SEEK_END);
  size_t length = ftell (fp);
  fseek (fp, 0, SEEK_SET);
  char source[length + 1];
  fread (source, 1, length, fp);
  source[length] = '\0';

  // parse the json
  struct json json1;
  json_list_storage_init (&json1);
  int ret = json_parse (&json1, source, length);
  if (ret == -1) {
    printf ("error\n");
    return -1;
  }

  printf ("the rest of the string: %s\n", source + ret);
  // test iterator
  for (struct json_obj *cur = json_begin (&json1); cur != json_end (&json1);
       cur = json_next (&json1, cur)) {
    if (cur == NULL) {
      perror ("error: NULL key\n");
      continue;
    }
    printf ("[key]: %.*s\n", (int)cur->key.len, cur->key.str);
  }

  // walk through the json
  struct json_obj *obj = json_get (&json1, "build_date");

  str_view_t sv = obj->value.str;

  printf ("[build_date]: %.*s\n", (int)sv.len, sv.str);

  struct json_obj *obj1 = json_index (&json1, 2);
  if (obj1 == NULL) {
    printf ("error\n");
    return -1;
  }

  ASSERT (json_obj_is_str (obj1));
  printf ("[3]: %.*s\n", (int)obj1->value.str.len, obj1->value.str.str);

  obj = json_get (&json1, "boolean_test1");
  if (obj->type == JSON_TYPE_BOOLEAN) {
    printf ("[boolean_test1]: %d\n", obj->value.boolean);
  }

  obj = json_get (&json1, "boolean_test2");
  if (obj->type == JSON_TYPE_BOOLEAN) {
    printf ("[boolean_test2]: %d\n", obj->value.boolean);
  }

  obj = json_get (&json1, "test_nested");
  ASSERT (json_obj_is_object (obj));
  struct json_obj *obj2 = json_get (obj->value.object, "c");
  ASSERT (json_obj_is_number (obj2));

  json_obj_asstr (obj2);
  printf ("[test_nested.c]: %s\n", obj2->value.lz_str);

  obj2 = json_get (obj->value.object, "d");
  ASSERT (json_obj_is_boolean (obj2));

  printf ("[test_nested.d]: %d\n", obj2->value.boolean);
  json_obj_asstr (obj2);
  // not a lz_str, but string view
  printf ("[test_nested.d]: %.*s\n", (int)obj2->value.str.len,
          obj2->value.str.str);

  printf ("--- destroy ---\n");
  json_destroy (&json1);
}