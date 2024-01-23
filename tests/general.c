// general testing of json-c features

// Path: json-c/tests/general.c

#include "json.h"

#include <stdio.h>
#include <stdlib.h>

int main (void) {
    struct json_obj *obj = malloc (sizeof (struct json_obj));
    json_obj_init (obj, NULL);

    json_obj_set_key (obj, "foo");
    json_obj_set_str (obj, "bar");

    str_view_t sv = json_obj_get_value_str (obj);
    // normal for address sanitizer to complain about this, we have our custom printf specifier :)
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


    // open file
    // get reletive path
    

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


    // parse the json
    struct json json_parsed;
    json_list_storage_init (&json_parsed);
    int ret = json_parse (&json_parsed, source, length);
    // if (ret == -1) {
    //     printf ("error\n");
    //     goto clean1;
    // }

    // test iterator
    for (
      struct json_obj *cur = json_begin (&json_parsed);
      cur != json_end (&json_parsed);
      cur = json_next (&json_parsed, cur)
    ) {
        if (cur == NULL) {
            perror ("error: NULL key\n");
            continue;
        }
        printf ("[key]: %.*s\n", (int) cur->key.len, cur->key.str);
    
    }

    // get a value by key
    struct json_obj *obj1 = json_get_by_key (&json_parsed, "build_date");
    sv = json_obj_get_value_str (obj1);
    printf ("[build_date]: %.*s\n", (int) sv.len, sv.str);
    double wrong_example = json_obj_get_value_number (obj1);
    if (!isnan (wrong_example)) {
        printf ("[build_date]: %f\n", wrong_example);
    } else {
        printf ("[build_date]: %s\n", "nan");
    }

    // get a value by index
    obj1 = json_get_by_index (&json_parsed, 2);
    if (obj1 == NULL) {
        printf ("error\n");
        goto clean1;
    }

    sv = json_obj_get_key (obj1);
    printf ("[key:3]: %.*s\n", (int) sv.len, sv.str);
    printf("[type:3]: %s\n", json_obj_get_type (obj1));

clean1:
    printf("--------------------\n");
    printf("cleaning...\n");
    printf("--------------------\n");
    free (test_str);
    json_destroy (&json_parsed);
    json_obj_destroy (obj);
    free (obj);

}