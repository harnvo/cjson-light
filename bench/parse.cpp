#include <iostream>
#include <chrono>
#include <time.h>
#include "test.h"

#include "cjson/cJSON.h"

static void 
test_json_parse (const char *src, struct json** our_json, cJSON** cJSON_json) {
  struct json *json1 = *our_json;
  cJSON *json2 = *cJSON_json;

  auto start = std::chrono::high_resolution_clock::now();
  // to be fair, we should allocate a new json struct every time
  json1 = (struct json *) malloc (sizeof (struct json));
  json_list_storage_init (json1);
  int ret = json_parse (json1, src, strlen (src));
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
  if (ret == -1) {
    test_fail ("could not parse");
    free (json1);
    goto fail;
  }
  printf("our time in ms: %f\n", duration/1000000.0);
	// json_destroy(json1);  
  // free (json1);

  // test cJSON lib
  start = std::chrono::high_resolution_clock::now();
  json2 = cJSON_Parse(src);
  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
  printf("cJSON time in ms: %f\n", duration/1000000.0);
  if (json2 == NULL) {
    test_fail ("could not parse");
    goto fail;
  }

  *our_json = json1;
  *cJSON_json = json2;

  return;

fail:
  printf("cleaning...\n");
  cJSON_Delete(json2);
}

static void
test_json_index (const char *target_key, struct json* our_json, cJSON* cJSON_json) {
  // benchmark the index time

  // our json
  auto start = std::chrono::high_resolution_clock::now();
  struct json_obj *obj = json_get (our_json, target_key);
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();

  printf("our index time in ms: %f\n", duration/1000000.0);

  // cJSON
  auto start2 = std::chrono::high_resolution_clock::now();
  cJSON *obj2 = cJSON_GetObjectItem(cJSON_json, target_key);
  auto end2 = std::chrono::high_resolution_clock::now();
  auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end2-start2).count();

  printf("cJSON index time in ms: %f\n", duration2/1000000.0);
}

int single_test (char *fname, const char *target_key) {
  // print file name
  char *last_slash = strrchr (fname, '/');
  if (last_slash == NULL) {
    last_slash = fname;
  } else {
    last_slash++;
  }

  printf ("--- %s ---\n", last_slash);

  // open file
  FILE *fp;
  char *source;
  size_t length;
  fp = fopen (fname, "r");
  if (fp == NULL) {
    printf ("error\n");
    return -1;
  }

  // read the entire file
  fseek (fp, 0, SEEK_END);
  length = ftell (fp);
  fseek (fp, 0, SEEK_SET);
  source = (char *) malloc (length + 1);
  fread (source, 1, length, fp);
  source[length] = '\0';

  struct json *our_json = NULL;
  cJSON *cJSON_json = NULL;
  test_json_parse(source, &our_json, &cJSON_json);
  if (our_json == NULL || cJSON_json == NULL) {
    printf ("error\n");
    return -1;
  }

  if (target_key != NULL) {
    test_json_index(target_key, our_json, cJSON_json);
  }

  // clean
  json_destroy(our_json);
  free (our_json);
  cJSON_Delete(cJSON_json);

  free (source);
  return 0;

}

int main () {
  // open file
  single_test("../bench-cases/normal.json",NULL);
  single_test("../bench-cases/nested-array.json",NULL);
  single_test("../bench-cases/nested-obj.json",NULL);
  single_test("../bench-cases/big.json",NULL);

  single_test("../bench-cases/index1.json","70e0a07682557075f4b36f02c8672af2ea8fbc5a969f215375290e45a195aa67");
  single_test("../bench-cases/index2.json", "ptimk1023");
}