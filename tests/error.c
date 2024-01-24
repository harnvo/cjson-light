#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "test.h"
#include <dirent.h>

const char *root = "../test-cases/";

static void
test_json_parse (const char *src) {
  struct json json1;
  json_list_storage_init (&json1);
  int ret = json_parse (&json1, src, strlen (src));
  if (ret == -1) {
    test_fail ("could not parse");
    goto fail;
  }

fail:
  printf ("cleaning...\n");
  json_destroy (&json1);
}

int
main () {
  // open all .json files starting with error_

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir (root)) == NULL) {
    perror ("error");
    return -1;
  }

  while ((ent = readdir (dir)) != NULL) {
    if (ent->d_type != DT_REG) {
      continue;
    }

    if (strstr (ent->d_name, "error_") == ent->d_name
        && strstr (ent->d_name, ".json") != NULL) {
      printf ("[test]: %s\n", ent->d_name);
      char path[256];
      strcpy (path, root);
      strcat (path, ent->d_name);
      FILE *fp = fopen (path, "r");
      if (fp == NULL) {
        perror ("error");
        return -1;
      }

      // read the entire file
      fseek (fp, 0, SEEK_END);
      size_t length = ftell (fp);
      fseek (fp, 0, SEEK_SET);
      char source[length + 1];
      fread (source, 1, length, fp);
      source[length] = '\0';

      test_json_parse (source);
      printf ("--------------------------------\n");
    }
  }

  closedir (dir);

  return 0;
}