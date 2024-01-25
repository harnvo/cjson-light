#include <iostream>
#include <chrono>
#include <time.h>
#include "test.h"

#include "cjson/cJSON.h"

static void 
test_json_get (const char *src, const char *key) {
  struct json json1;
  cJSON *json2;

  json_list_storage_init (&json1);
  int ret = json_parse (&json1, src, strlen (src));
  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
  if (ret == -1) {
    test_fail ("could not parse");
    goto fail;
  }

  // test our lib
  start = std::chrono::high_resolution_clock::now();
  json_get (&json1, key);
  end = std::chrono::high_resolution_clock::now();

  printf("our time in ms: %f\n", duration/1000000.0);


  // test cJSON lib
  json2 = cJSON_Parse(src);
  start = std::chrono::high_resolution_clock::now();
  cJSON_GetObjectItem(json2, key);
  end = std::chrono::high_resolution_clock::now();
  duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
  printf("cJSON time in ms: %f\n", duration/1000000.0);
  if (json2 == NULL) {
    test_fail ("could not parse");
    goto fail;
  }

fail:
  printf("cleaning...\n");
  json_destroy(&json1);  
  cJSON_Delete(json2);
}

int single_test (char *fname, const char* key) {
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

  test_json_get(source, key);
  free (source);
  return 0;

}

int main () {
  // open file
  single_test("../bench-cases/normal.json","key");
  single_test("../bench-cases/nested-array.json", "root");
}