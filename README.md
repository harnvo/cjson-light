# [Unfinished] json-c

A lightweight, modular, fast, and easy-to-use JSON libraey in c.

## Getting started

__Supported Platforms__
I only tested my code with gnu-c11 on Ubuntu-20.04.

__Building__

So far only xmake is supported to build the project. 
Run `xmake` to build 

__Including json-c__

If you make successfully installed it, you may include it like:
```
#include <json.h>
```

## usage

__Data structure__

- __*str_view_t*__
  `str_view_t` is provided to facilitate string usage and avoid frequent malloc/free, and it works quite like the `std::string_view` in c++11. The major difference is that our str_view is *NOT* read-only, and you may modify the char* value inside.
  the struct looks like:
    ```
    
  struct str_view {
    char *str;
    size_t len;
  };
   ...
   
  typedef struct str_view str_view_t;
  ```

  Some sample usage:
  ```
  str_view_t sv;
  str_view_init(&sv, "foo", 3);
  str_view_init_from_str(&sv, "foo");
  // "ello"
  str_view_init_from_substr(&sv, "hello world!\n", 1, 4);

  // "llo"
  str_view_substr_ (&sv, 1, 3);

  str_view_t sv2;
  // sv2 = "lo"
  str_view_substr (sv/*src*/, sv2/*dst*/,1, 2);

  str_view_cmp (sv, sv2);     // false
  str_view_ncmp (sv, sv2, 1); // true
  
  ```


  We also have our own custom printf specifier and one can use `printf` to print a `str_view_t` just like other variables:
  *Warning: only gnu-c supports this feature*
  ```
  str_view_t sv;
  str_view_init(&sv,"foo", 3);
  printf("print str_view_t:%v\n", sv);
  
  >>> print str_view_t:foo
  ```

- __*struct json_obj*__
  `json_obj` is a struct contraining key and value infos. It is defined as follows:

  ```
    struct json_obj {
      str_view_t key; // should not be used for array
    
      union {
        struct json *array;
        struct json *object; // to be introduced in the next data structure : )
        str_view_t str;
        double number;
        int boolean;
    
        char lz_str[__JSON_OBJ_LZ_STR_LEN]; // lazy string to avoid malloc
      } value;

      // FOR INTERNAL USE.
      char *__source;
      size_t __source_len;
      int type;
    };
  ```

  Sample usage:
  ```
    struct json_obj *obj = malloc (sizeof (struct json_obj)); // you may also use your custom malloc/free pair
    json_obj_init (obj, NULL);

    json_obj_set_key (obj, "foo");
    json_obj_set_str (obj, "bar");

    str_view_t sv = json_obj_get_value_str (obj);
    printf ("sv.str=%v\n", sv);    

    // str_view_init (&sv, "hellooooooooo", sizeof ("hellooooooooo") - 1);
    char *test_str = malloc (sizeof (char) * 10+1);
    strcpy (test_str, "hellooooo");
    test_str[10] = '\0';
    str_view_init (&sv, test_str, sizeof ("hellooooo") - 1);

    json_obj_set_str_by_view (obj, sv);
    sv = json_obj_get_value_str (obj);

    printf ("sv.str=%v\n", sv);
    // a evil hack to change the string
    sv.str[0] = 'x';
    // this, however, does not change the string
    str_view_init (&sv, "changed", sizeof ("changed") - 1);
    str_view_t sv2 = json_obj_get_value_str (obj);
    printf ("sv2.str=%v\n", sv2);

    // setting number
    json_obj_set_number (obj, 123.456);
    double num = json_obj_get_value_number (obj);
    printf ("Type: %s; num=%f\n", json_obj_get_type (obj), num);


    free (obj);

  >>> sv.str=bar
  >>> sv.str=hellooooo
  >>> sv2.str=xellooooo
  >>> Type: number; num=123.456000
  ```

  - __*struct json*__
    `struct json` is the structure that stores an array/collection of `struct json_obj`. It is designed to hide implementation details of how the json object array/collection is stored and managed.
    the definition is as follows:
    ```
    struct json {
      void *storage;
      struct _json_op *_op;
    };
    ```
    There are some basic operations exposed to use:
    - init: initiate the json. you need to use different initiation function for different storage type (e.g. `json_list_storage_init`). (So fat only linked-list storage is supported.)
    - `json_destroy`: destroy everything within json.
    - `json_add`: add an json_obj into json. memory copy involved.
    - `_json_add_empty`: add an empty json_obj into json, returns the pointer to the obj. Used in case you wanna avoid memory copy.
    - `json_remove_by_key`: remove a json_obj by key (char*).
    - `json_remove_by_key_view`: remove a json_obj by key (str_view_t).
    - `json_remove_by_index`: remove a json_obj by index.
    - `json_get_by_key`: get a json_obj by key (char*).
    - `json_get_by_key_view`: get a json_obj by key (str_view_t).
    - `json_get_by_index`: get a json_obj by index.
    - `json_get_size`: return number of elements within json (non-recursive. i.e. nested json counts as one obj.)
    - `json_begin`, `json_next`, `json_end`: a iterator for user's convenience. To use it:
      ```
          for (
          struct json_obj *cur = json_begin (&json_parsed);
          cur != json_end (&json_parsed);
          cur = json_next (&json_parsed, cur)
        ) {
          ...
        }
      ```
