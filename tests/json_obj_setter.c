#include "test.h"

static void
__test_json_obj_set_key_short_key (struct json_obj **obj_pp) {
  struct json_obj *obj = *obj_pp;
  if (obj == NULL) {
    perror ("malloc");
    obj = malloc (sizeof (struct json_obj));
    json_obj_init (obj, NULL);
    *obj_pp = obj;
  }

  char *test_key = "skey";
  json_obj_set_key (obj, test_key);
  if (strncmp (obj->key.str, test_key, strlen (test_key)) != 0) {
    test_fail2 ("json_obj_set_key failed, expected: %s, actual: %s", test_key,
                obj->key.str);
    goto clean;
  }

  if (obj->key.len != strlen (test_key)) {
    test_fail2 ("json_obj_set_key failed, expected: %ld, actual: %ld",
                strlen (test_key), obj->key.len);
    goto clean;
  }

  test_success ("short key test");
  return;
clean:
  json_obj_destroy (obj);
  free (obj);
}

static void
__test_json_obj_set_key_long_key (struct json_obj **obj_pp, size_t _len) {
  struct json_obj *obj = *obj_pp;
  if (obj == NULL) {
    perror ("malloc");
    obj = malloc (sizeof (struct json_obj));
    json_obj_init (obj, NULL);
    *obj_pp = obj;
  }

  char *test_key = "long";
  char *long_key = malloc (_len + 1);
  memset (long_key, 'k', _len);
  long_key[_len] = '\0';
  json_obj_set_key (obj, long_key);
  if (strncmp (obj->key.str, long_key, strlen (long_key)) != 0) {
    test_fail2 ("json_obj_set_key failed, expected: %s, actual: %s", long_key,
                obj->key.str);
    goto clean;
  }

  if (obj->key.len != strlen (long_key)) {
    test_fail2 ("json_obj_set_key failed, expected: %ld, actual: %ld",
                strlen (long_key), obj->key.len);
    goto clean;
  }

  test_success ("long key test");
  free (long_key);
  return;

clean:
  free (long_key);
  json_obj_destroy (obj);
  free (obj);
}

static void
test_json_obj_set_key (void) {
  struct json_obj obj;
  json_obj_init (&obj, NULL);
  char *type = json_obj_get_type (&obj);
  if (string_assert_equal (type, "null") != 0) {
    test_fail2 ("json_obj_get_type failed, expected: null, actual: %s", type);
    goto clean;
  }

  struct json_obj *obj_p = &obj;
  __test_json_obj_set_key_short_key (&obj_p);
  size_t long_key_len = 32;
  __test_json_obj_set_key_long_key (&obj_p, long_key_len);

  json_obj_set_key (&obj, "");
  printf ("[key]: %.*s\n", (int)obj.key.len, obj.key.str);
  if (obj.key.len != 0) {
    test_fail2 ("json_obj_set_key failed, expected: %d, actual: %ld", 0,
                obj.key.len);
    goto clean;
  } else if (obj.__source_len != long_key_len + 1) {
    test_fail2 ("json_obj_set_key failed, expected: %ld, actual: %ld",
                long_key_len + 1, obj.__source_len);
    goto clean;
  }
  test_success ("empty key test");

  json_obj_set_key (&obj, "oh");
  printf ("[key]: %.*s\n", (int)obj.key.len, obj.key.str);
  int cur_source_len = obj.__source_len;
  if (cur_source_len != long_key_len + 1) {
    test_fail2 ("json_obj_set_key failed, expected: %ld, actual: %d",
                long_key_len + 1, cur_source_len);
    goto clean;
  }
  test_success ("short key test");

  int ret = json_obj_take_ownership (&obj, "take ownership test",
                                     strlen ("take ownership test"));
  if (ret != -1) {
    test_fail2 ("json_obj_take_ownership should fail, but it didn't, ret: %d",
                ret);
  }
  test_success (
      "take ownership test: a owner is trying to take ownership of a source");

  char *source = NULL;
  size_t source_len = 0;

  json_obj_yield_ownership (&obj, &source, &source_len);
  if (strncmp (source, "ohkkk", 5) != 0) {
    test_fail2 ("json_obj_yield_ownership failed, expected: oh, actual: %s",
                source);
    goto clean;
  }
  printf ("[source]: %s\n", source);

  // char *_ownership_test_str = "take ownership test string";
  char *_ownership_test_str = malloc (12 + 1);
  memset (_ownership_test_str, 'take ', 12);
  _ownership_test_str[12] = '\0';
  ret = json_obj_take_ownership (&obj, _ownership_test_str,
                                 strlen (_ownership_test_str));
  if (ret != 0) {
    test_fail2 ("json_obj_take_ownership should succedd, but failed. ret: %d",
                ret);
    goto clean;
  }
  test_success ("take ownership test: a non-owner is trying to take ownership "
                "of a source");

  // current key should still be "oh"
  if (strncmp (obj.key.str, "oh", obj.key.len) != 0) {
    test_fail2 ("json_obj_take_ownership failed, expected: oh, actual: %.*s",
                (int)obj.key.len, obj.key.str);
    goto clean;
  }
  test_success ("take ownership test: transfer of ownership should not affect "
                "the current key");

  json_global_hooks.free_fn (source);

clean:
  json_obj_destroy (&obj);
}

static void
__test_json_obj_set_str_short_str (struct json_obj **obj_pp) {
  struct json_obj *obj = *obj_pp;
  if (obj == NULL) {
    perror ("malloc");
    obj = malloc (sizeof (struct json_obj));
    json_obj_init (obj, NULL);
    *obj_pp = obj;
  }

  char *test_key = "short";
  json_obj_set_str (obj, test_key);
  if (!json_obj_is_str (obj)) {
    test_fail2 ("json_obj_set_str failed, expected: %s, actual: %s", "string",
                json_obj_get_type (obj));
    goto clean;
  }

  if (!obj->type & JSON_TYPE_LZ_STR) {
    test_fail2 ("json_obj_set_str failed, expected: %s", "lazy string");
    goto clean;
  }

  if (strncmp (obj->value.lz_str, test_key, strlen (test_key)) != 0) {
    test_fail2 ("json_obj_set_str failed, expected: %s, actual: %s", test_key,
                obj->value.lz_str);
    goto clean;
  }

  test_success ("short string test");
  return;

clean:
  json_obj_destroy (obj);
  free (obj);
}

static void
__test_json_obj_set_str_long_str (struct json_obj **obj_pp) {
  struct json_obj *obj = *obj_pp;
  if (obj == NULL) {
    perror ("malloc");
    obj = malloc (sizeof (struct json_obj));
    json_obj_init (obj, NULL);
    *obj_pp = obj;
  }

  char *test_key = "long";
  char *long_str = malloc (100 + 1);
  memset (long_str, 's', 100);
  long_str[100] = '\0';
  json_obj_set_str (obj, long_str);
  if (!json_obj_is_str (obj)) {
    test_fail2 ("json_obj_set_str failed, expected: %s, actual: %s", "string",
                json_obj_get_type (obj));
    goto clean;
  }

  if (obj->type & JSON_TYPE_LZ_STR) {
    test_fail2 ("json_obj_set_str failed, expected type: %s but got %s",
                "string", "lazy string");
    goto clean;
  }

  if (strncmp (obj->value.str.str, long_str, strlen (long_str)) != 0) {
    test_fail2 ("json_obj_set_str failed, expected: %s, actual: %s", long_str,
                obj->value.str.str);
    goto clean;
  }

  // expects ownership
  if (!json_obj_owns_source (obj)) {
    test_fail2 ("json_obj_set_str failed, expected: %s", "ownership");
    goto clean;
  }

  if (strcmp (obj->value.str.str, long_str) != 0) {
    test_fail2 ("json_obj_set_str failed, expected: %s, actual: %s", long_str,
                obj->__source);
    goto clean;
  }

  test_success ("long string test");
  free (long_str);
  return;

clean:
  free (long_str);
  json_obj_destroy (obj);
  free (obj);
}

static void
test_json_obj_set_str (void) {
  struct json_obj *obj = malloc (sizeof (struct json_obj));
  json_obj_init (obj, NULL);

  __test_json_obj_set_str_short_str (&obj);
  __test_json_obj_set_str_long_str (&obj);
  __test_json_obj_set_str_short_str (&obj);

  // test compatibility with json_obj_set_key

  // short key long str
  __test_json_obj_set_key_short_key (&obj);
  __test_json_obj_set_str_long_str (&obj);
  __test_json_obj_set_key_long_key (&obj, 128);

  __test_json_obj_set_key_short_key (&obj);
  __test_json_obj_set_str_long_str (&obj);
  printf ("source=%s\n", obj->__source);

  // long key long str
  __test_json_obj_set_key_long_key (&obj, 128);
  __test_json_obj_set_str_long_str (&obj);
  printf ("source=%s\n", obj->__source);

  // short key short str
  __test_json_obj_set_key_short_key (&obj);
  __test_json_obj_set_str_short_str (&obj);
  printf ("source=%s\n", obj->__source);

  // long key short str
  __test_json_obj_set_key_long_key (&obj, 128);
  __test_json_obj_set_str_short_str (&obj);
  printf ("source=%s\n", obj->__source);

  json_obj_destroy (obj);
  json_global_hooks.free_fn (obj);
}

int
main (void) {

  test_json_obj_set_key ();
  printf ("\n");

  test_json_obj_set_str ();
  return 0;
}