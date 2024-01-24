#include <iostream>
#include <chrono>
#include <time.h>
#include <math.h>

#include "test.h"
#include "cjson/cJSON.h"

static void 
cal_mean_std (double *array, int num_repeats, double *mean, double *std) {
  double sum = 0;
  for (int i=0;i<num_repeats;i++) {
    sum += array[i];
  }
  *mean = sum / num_repeats;

  double sum2 = 0;
  for (int i=0;i<num_repeats;i++) {
    sum2 += (array[i] - *mean) * (array[i] - *mean);
  }
  *std = sqrt (sum2 / num_repeats);
}

static double
cal_normal_dist_p_value (double mean1, double std1, int n1, double mean2, double std2, int n2) {
  // calculate the p value of the normal distribution
  // H0: mean1 == mean2

  __float128 mean_diff = mean1 - mean2;
  __float128 std_diff = sqrt (std1*std1/n1 + std2*std2/n2);
  __float128 z = mean_diff / std_diff;

  // calculate the p value
  double p_value = 0;
  if (z > 0) {
    p_value = 0.5 * erfc ((double) z / sqrt (2));
  } else {
    p_value = 0.5 * erfc ( (double) -z / sqrt (2));
  }

  return p_value;
}

static void 
test_json_parse (int num_repeats, const char *src, struct json** our_json, cJSON** cJSON_json) {
  struct json *json1 = *our_json;
  cJSON *json2 = *cJSON_json;

  auto start = std::chrono::high_resolution_clock::now();
  auto end = std::chrono::high_resolution_clock::now();
  int ret = -1;
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-end).count();
  auto duration2 = std::chrono::duration_cast<std::chrono::nanoseconds>(end-end).count();

  // use an array to store the duration
  double duration_array[num_repeats], mean1, std1;
  double duration2_array[num_repeats], mean2, std2;
  double p_value;

  for (int i=0;i<num_repeats;i++) {
    start = std::chrono::high_resolution_clock::now();
    // to be fair, we should allocate a new json struct every time
    json1 = (struct json *) malloc (sizeof (struct json));
    json_list_storage_init (json1);
    ret = json_parse (json1, src, strlen (src));
    end = std::chrono::high_resolution_clock::now();
    duration_array[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    if (ret == -1) {
      test_fail ("could not parse");
      free (json1);
      goto fail;
    }
    json_destroy(json1);  
    free (json1);
  }
  cal_mean_std (duration_array, num_repeats, &mean1, &std1);
  printf("our time in ms: %f\n", mean1/1000000.0);
  printf("time std in ms: %f\n", std1/1000000.0);
	// json_destroy(json1);  
  // free (json1);

  // test cJSON lib
  for (int i=0;i<num_repeats;i++) {
    start = std::chrono::high_resolution_clock::now();
    json2 = cJSON_Parse(src);
    end = std::chrono::high_resolution_clock::now();
    duration2_array[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    if (json2 == NULL) {
      test_fail ("could not parse");
      cJSON_Delete(json2);
      goto fail;
    }
    cJSON_Delete(json2);
  }

  cal_mean_std (duration2_array, num_repeats, &mean2, &std2);
  printf("cJSON time in ms: %f\n", mean2/1000000.0);
  printf("time std in ms: %f\n", std2/1000000.0);

  // calculate the p value
  p_value = cal_normal_dist_p_value (mean1, std1, num_repeats, mean2, std2, num_repeats);
  printf("p value: %f\n", p_value);

  // if the p value is more than 0.05, repeat more

  json1 = (struct json *) malloc (sizeof (struct json));
  json_list_storage_init (json1);
  ret = json_parse (json1, src, strlen (src));

  *our_json = json1;
  json2 = cJSON_Parse(src);
  *cJSON_json = json2;

  return;

fail:
  printf("cleaning...\n");
  cJSON_Delete(json2);
}

static void
test_json_index (int num_repeats, const char *target_key, struct json* our_json, cJSON* cJSON_json) {
  double duration_array[num_repeats], mean1, std1;
  double duration2_array[num_repeats], mean2, std2;

  double p_value;

  auto start = std::chrono::high_resolution_clock::now();
  struct json_obj *obj;
  cJSON *cJSON_obj;
  auto end = std::chrono::high_resolution_clock::now();
  auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();


  // benchmark the index time

  // our json
  for (int i=0;i<num_repeats;i++) {
    start = std::chrono::high_resolution_clock::now();
    obj = json_get (our_json, target_key);
    end = std::chrono::high_resolution_clock::now();
    duration_array[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    if (obj == NULL) {
      test_fail ("could not index");
      goto fail;
    }
  }

  cal_mean_std (duration_array, num_repeats, &mean1, &std1);


  // cJSON
  for (int i=0;i<num_repeats;i++) {
    start = std::chrono::high_resolution_clock::now();
    cJSON_obj = cJSON_GetObjectItemCaseSensitive(cJSON_json, target_key);
    end = std::chrono::high_resolution_clock::now();
    duration2_array[i] = std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    if (cJSON_obj == NULL) {
      test_fail ("could not index");
      goto fail;
    }
  }

  cal_mean_std (duration2_array, num_repeats, &mean2, &std2);

  printf("our time in ms: %f\n", mean1/1000000.0);
  printf("time std in ms: %f\n", std1/1000000.0);
  printf("cJSON time in ms: %f\n", mean2/1000000.0);
  printf("time std in ms: %f\n", std2/1000000.0);

  // calculate the p value
  p_value = cal_normal_dist_p_value (mean1, std1, num_repeats, mean2, std2, num_repeats);
  printf("p value: %f\n", p_value);

fail:
  return;
}

int single_test (int num_repeats, char *fname, const char *target_key) {
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
  test_json_parse(num_repeats, source, &our_json, &cJSON_json);
  if (our_json == NULL || cJSON_json == NULL) {
    printf ("error\n");
    return -1;
  }

  if (target_key != NULL) {
    test_json_index(num_repeats, target_key, our_json, cJSON_json);
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
  single_test(1000, "../bench-cases/normal.json",NULL);
  single_test(10000, "../bench-cases/nested-array.json",NULL);
  single_test(10000, "../bench-cases/nested-obj.json",NULL);
  single_test(10, "../bench-cases/big.json",NULL);

  single_test(10000, "../bench-cases/index1.json","70e0a07682557075f4b36f02c8672af2ea8fbc5a969f215375290e45a195aa67");
  single_test(1000, "../bench-cases/index2.json", "ptimk1023");
}