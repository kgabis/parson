/*
 SPDX-License-Identifier: MIT

 Parson (https://github.com/kgabis/parson)
 Copyright (c) 2012 - 2023 Krzysztof Gabis

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
*/
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "parson.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST(A) do {\
if (A) {\
    g_tests_passed++;\
} else {\
    printf("%d %-72s - FAILED\n", __LINE__, #A);\
    g_tests_failed++;\
}\
} while(0)

#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define DBL_EPSILON 2.2204460492503131e-16
#define DBL_EQ(a, b) (fabs((a) - (b)) < DBL_EPSILON)

const char filename[] = "test_2_comments.txt";


void test_suite_2(JSON_Value *value); /* Test correctness of parsed values */
void test_suite_2_with_comments(void);

static const char *g_tests_path = "tests";

typedef struct failing_alloc {
    int allocation_to_fail;
    int alloc_count;
    int total_count;
    int has_failed;
    int should_fail;
} failing_alloc_t;

static failing_alloc_t g_failing_alloc;

static void *failing_malloc(size_t size);
static void failing_free(void *ptr);

static char * read_file(const char * filename);
const char* get_file_path(const char *filename);

static int g_tests_passed;
static int g_tests_failed;

#ifdef TESTS_MAIN
int main(int argc, char *argv[]) {
#else
int tests_main(int argc, char *argv[]);
int tests_main(int argc, char *argv[]) {
#endif
#if 0 /* unconfuse xcode */
}
#endif
    /* Example functions from readme file:      */
    /* print_commits_info("torvalds", "linux"); */
    /* serialization_example(); */
    /* persistence_example(); */

    puts("################################################################################");
    puts("Running parson tests");

    if (argc == 2) {
        g_tests_path = argv[1];
    } else {
        g_tests_path = "tests";
    }

    test_suite_2_with_comments();


    printf("Tests failed: %d\n", g_tests_failed);
    printf("Tests passed: %d\n", g_tests_passed);
    puts("################################################################################");
    return 0;
}

void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
    JSON_Value *array_value;
    size_t len;
    size_t i;
    TEST(root_value);
    TEST(json_value_get_type(root_value) == JSONObject);
    root_object = json_value_get_object(root_value);

    TEST(json_object_has_value(root_object, "string"));
    TEST(!json_object_has_value(root_object, "_string"));
    TEST(json_object_has_value_of_type(root_object, "object", JSONObject));
    TEST(!json_object_has_value_of_type(root_object, "string array", JSONObject));
    TEST(json_object_has_value_of_type(root_object, "string array", JSONArray));
    TEST(!json_object_has_value_of_type(root_object, "object", JSONArray));
    TEST(json_object_has_value_of_type(root_object, "string", JSONString));
    TEST(!json_object_has_value_of_type(root_object, "positive one", JSONString));
    TEST(json_object_has_value_of_type(root_object, "positive one", JSONNumber));
    TEST(!json_object_has_value_of_type(root_object, "string", JSONNumber));
    TEST(json_object_has_value_of_type(root_object, "boolean true", JSONBoolean));
    TEST(!json_object_has_value_of_type(root_object, "positive one", JSONBoolean));
    TEST(json_object_has_value_of_type(root_object, "null", JSONNull));
    TEST(!json_object_has_value_of_type(root_object, "object", JSONNull));

    TEST(json_object_dothas_value(root_object, "object.nested array"));
    TEST(!json_object_dothas_value(root_object, "_object.nested array"));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested object", JSONObject));
    TEST(!json_object_dothas_value_of_type(root_object, "object.nested array", JSONObject));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested array", JSONArray));
    TEST(!json_object_dothas_value_of_type(root_object, "object.nested object", JSONArray));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested string", JSONString));
    TEST(!json_object_dothas_value_of_type(root_object, "object.nested number", JSONString));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested number", JSONNumber));
    TEST(!json_object_dothas_value_of_type(root_object, "_object.nested whatever", JSONNumber));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested true", JSONBoolean));
    TEST(!json_object_dothas_value_of_type(root_object, "object.nested number", JSONBoolean));
    TEST(json_object_dothas_value_of_type(root_object, "object.nested null", JSONNull));
    TEST(!json_object_dothas_value_of_type(root_object, "object.nested object", JSONNull));

    TEST(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf-8 string"), "あいうえお"));
    TEST(STREQ(json_object_get_string(root_object, "surrogate string"), "lorem𝄞ipsum𝍧lorem"));

    len = json_object_get_string_len(root_object, "string with null");
    TEST(len == 7);
    TEST(memcmp(json_object_get_string(root_object, "string with null"), "abc\0def", len) == 0);

    TEST(DBL_EQ(json_object_get_number(root_object, "positive one"), 1.0));
    TEST(DBL_EQ(json_object_get_number(root_object, "negative one"), -1.0));
    TEST(DBL_EQ(json_object_get_number(root_object, "hard to parse number"), -0.000314));
    TEST(json_object_get_boolean(root_object, "boolean true") == 1);
    TEST(json_object_get_boolean(root_object, "boolean false") == 0);
    TEST(json_value_get_type(json_object_get_value(root_object, "null")) == JSONNull);

    array = json_object_get_array(root_object, "string array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        g_tests_failed++;
    }

    array = json_object_get_array(root_object, "x^2 array");
    if (array != NULL) {
        for (i = 0; i < json_array_get_count(array); i++) {
            TEST(DBL_EQ(json_array_get_number(array, i), (i * i)));
        }
    } else {
        g_tests_failed++;
    }

    TEST(json_object_get_array(root_object, "non existent array") == NULL);
    TEST(STREQ(json_object_dotget_string(root_object, "object.nested string"), "str"));
    TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);
    TEST(json_object_dotget_boolean(root_object, "object.nested false") == 0);
    TEST(json_object_dotget_value(root_object, "object.nested null") != NULL);
    TEST(DBL_EQ(json_object_dotget_number(root_object, "object.nested number"), 123));

    TEST(json_object_dotget_value(root_object, "should.be.null") == NULL);
    TEST(json_object_dotget_value(root_object, "should.be.null.") == NULL);
    TEST(json_object_dotget_value(root_object, ".") == NULL);
    TEST(json_object_dotget_value(root_object, "") == NULL);

    array = json_object_dotget_array(root_object, "object.nested array");
    TEST(array != NULL);
    TEST(json_array_get_count(array) > 1);
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    }
    TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);

    TEST(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "//"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "url"), "https://www.example.com/search?q=12345"));
    TEST(STREQ(json_object_get_string(root_object, "escaped chars"), "\" \\ /"));

    TEST(json_object_get_object(root_object, "empty object") != NULL);
    TEST(json_object_get_array(root_object, "empty array") != NULL);

    TEST(json_object_get_wrapping_value(root_object) == root_value);
    array = json_object_get_array(root_object, "string array");
    array_value = json_object_get_value(root_object, "string array");
    TEST(json_array_get_wrapping_value(array) == array_value);
    TEST(json_value_get_parent(array_value) == root_value);
    TEST(json_value_get_parent(root_value) == NULL);
}

void test_suite_2_with_comments(void) {
    JSON_Value *root_value = NULL;
    root_value = json_parse_file_with_comments(filename);
    test_suite_2(root_value);
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string(root_value))));
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string_pretty(root_value))));
    json_value_free(root_value);
}


