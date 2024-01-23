#include "test.h"

typedef int (*obj_assertion_t) (struct json_obj *obj);

static int
__recursive_test (const struct json *json, obj_assertion_t ass_fx, int ass_should_succ){
  for (
    struct json_obj *cur = json_begin (json);
    cur != json_end (json);
    cur = json_next (json, cur)
  ) {
    if (cur == NULL) {
      perror ("error: NULL key\n");
      continue;
    }
    printf ("[key]: %.*s\n", (int) cur->key.len, cur->key.str);
    if (ass_should_succ){
      if (!ass_fx(cur)) {
					return -1;
			}
    } else {
      if (ass_fx(cur)) {
				return -1;
			}
    }

    if (json_obj_has_children (cur)) {
      __recursive_test (cur->value.object, ass_fx, ass_should_succ);
    }
  
  }
}

static void 
test_json_parse (const char *src) {
  struct json json1;
  json_list_storage_init (&json1);
  int ret = json_parse (&json1, src, strlen (src));
  if (ret == -1) {
    test_fail ("could not parse");
    goto fail;
  }

  // json_parse never take ownership of the source string
  ret = __recursive_test (&json1, json_obj_owns_source, 0);
	if (ret == -1) {
		test_fail("detecting owner!");
		goto fail;
	} else {
   test_success("no obj is owner");
	}

fail:
  printf("cleaning...\n");
	json_destroy(&json1);
}

int main () {
  // open file
  FILE *fp = fopen ("../test-cases/test1.json", "r");
	if (fp == NULL) {
		printf ("error\n");
		return -1;
	}

	// read the entire file
	fseek (fp, 0, SEEK_END);
	size_t length = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	char source [length + 1];
	fread (source, 1, length, fp);
	source[length] = '\0';

	test_json_parse(source);

}