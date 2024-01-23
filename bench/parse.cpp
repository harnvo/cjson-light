#include <iostream>
#include <chrono>
#include <time.h>
#include "test.h"

#include "cJSON.h"

static void 
test_json_parse (const char *src) {
  struct json json1;
  json_list_storage_init (&json1);
  auto start = std::chrono::high_resolution_clock::now();
  int ret = json_parse (&json1, src, strlen (src));
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
  if (ret == -1) {
    test_fail ("could not parse");
    goto fail;
  }
  printf("our time in ms: %f\n", duration/1000000.0);


  // test cJSON lib
  cJSON *json2;
  start = std::chrono::high_resolution_clock::now();
  json2 = cJSON_Parse(src);
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

int main () {
  // open file
  FILE *fp = fopen ("../test-cases/test2.json", "r");
	if (fp == NULL) {
		printf ("error\n");
		return -1;
	}

	// read the entire file
	fseek (fp, 0, SEEK_END);
	size_t length = ftell (fp);
	fseek (fp, 0, SEEK_SET);
	char *source = (char *) malloc (length + 1);
	fread (source, 1, length, fp);
	source[length] = '\0';

	test_json_parse(source);

  free (source);
}