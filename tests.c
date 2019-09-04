/*
 SPDX-License-Identifier: MIT

 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2019 Krzysztof Gabis

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST(A) printf("%d %-72s-", __LINE__, #A);\
                if(A){puts(" OK");tests_passed++;}\
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define EPSILON 0.000001
#define COMPARESERIALIZATION(A) f = fopen(A, "rb");\
    fseek(f, 0, SEEK_END);\
    fsize = ftell(f);\
    fseek(f, 0, SEEK_SET);\
    source = malloc(fsize + 1);\
    fread(source, 1, fsize, f);\
    fclose(f);\
    val = json_parse_string(source);\
    serial = json_serialize_to_string(val);\
    TEST(source != NULL && serial != NULL && strcmp(source, serial)==0);\
    free(source);\
    json_value_free(val);\

void test_suite_1(void); /* Test 3 files from json.org + serialization*/
void test_suite_2(JSON_Value *value); /* Test correctness of parsed values */
void test_suite_2_no_comments(void);
void test_suite_2_with_comments(void);
void test_suite_3(void); /* Test parsing valid and invalid strings */
void test_suite_4(void); /* Test deep copy function */
void test_suite_5(void); /* Test building json values from scratch */
void test_suite_6(void); /* Test value comparing verification */
void test_suite_7(void); /* Test schema validation */
void test_suite_8(void); /* Test serialization */
void test_suite_9(void); /* Test serialization (pretty) */
void test_suite_10(void); /* Testing for memory leaks */
void test_suite_11(void); /* Additional things that require testing */
void test_suite_minefield(void); /* Testing for minefield test suite strings */

void print_commits_info(const char *username, const char *repo);
void persistence_example(void);
void serialization_example(void);

static int malloc_count;
static void *counted_malloc(size_t size);
static void counted_free(void *ptr);

static char * read_file(const char * filename);

static int tests_passed;
static int tests_failed;

int main() {
    /* Example functions from readme file:      */
    /* print_commits_info("torvalds", "linux"); */
    /* serialization_example(); */
    /* persistence_example(); */
    json_set_allocation_functions(counted_malloc, counted_free);
    test_suite_1();
    test_suite_2_no_comments();
    test_suite_2_with_comments();
    test_suite_3();
    test_suite_4();
    test_suite_5();
    test_suite_6();
    test_suite_7();
    test_suite_8();
    test_suite_9();
    test_suite_10();
    test_suite_11();
    test_suite_minefield();

    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}

void test_suite_1(void) {
    JSON_Value *val;
    TEST((val = json_parse_file("tests/test_1_1.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file("tests/test_1_2.txt")) == NULL); /* Over 2048 levels of nesting */
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_1.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_2.txt")) == NULL); /* Over 2048 levels of nesting */
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }
}

void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
    JSON_Value *array_value;
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

    TEST(json_object_get_number(root_object, "positive one") == 1.0);
    TEST(json_object_get_number(root_object, "negative one") == -1.0);
    TEST(fabs(json_object_get_number(root_object, "hard to parse number") - (-0.000314)) < EPSILON);
    TEST(json_object_get_boolean(root_object, "boolean true") == 1);
    TEST(json_object_get_boolean(root_object, "boolean false") == 0);
    TEST(json_value_get_type(json_object_get_value(root_object, "null")) == JSONNull);

    array = json_object_get_array(root_object, "string array");
    if (array != NULL && json_array_get_count(array) > 1) {
        TEST(STREQ(json_array_get_string(array, 0), "lorem"));
        TEST(STREQ(json_array_get_string(array, 1), "ipsum"));
    } else {
        tests_failed++;
    }

    array = json_object_get_array(root_object, "x^2 array");
    if (array != NULL) {
        for (i = 0; i < json_array_get_count(array); i++) {
            TEST(json_array_get_number(array, i) == (i * i));
        }
    } else {
        tests_failed++;
    }

    TEST(json_object_get_array(root_object, "non existent array") == NULL);
    TEST(STREQ(json_object_dotget_string(root_object, "object.nested string"), "str"));
    TEST(json_object_dotget_boolean(root_object, "object.nested true") == 1);
    TEST(json_object_dotget_boolean(root_object, "object.nested false") == 0);
    TEST(json_object_dotget_value(root_object, "object.nested null") != NULL);
    TEST(json_object_dotget_number(root_object, "object.nested number") == 123);

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

void test_suite_2_no_comments(void) {
    const char *filename = "tests/test_2.txt";
    JSON_Value *root_value = NULL;
    root_value = json_parse_file(filename);
    test_suite_2(root_value);
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string(root_value))));
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string_pretty(root_value))));
    json_value_free(root_value);
}

void test_suite_2_with_comments(void) {
    const char *filename = "tests/test_2_comments.txt";
    JSON_Value *root_value = NULL;
    root_value = json_parse_file_with_comments(filename);
    test_suite_2(root_value);
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string(root_value))));
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string_pretty(root_value))));
    json_value_free(root_value);
}

void test_suite_3(void) {
    puts("Testing valid strings:");
    TEST(json_parse_string("{\"lorem\":\"ipsum\"}") != NULL);
    TEST(json_parse_string("[\"lorem\"]") != NULL);
    TEST(json_parse_string("null") != NULL);
    TEST(json_parse_string("true") != NULL);
    TEST(json_parse_string("false") != NULL);
    TEST(json_parse_string("\"string\"") != NULL);
    TEST(json_parse_string("123") != NULL);

    puts("Test UTF-16 parsing:");
    TEST(STREQ(json_string(json_parse_string("\"\\u0024x\"")), "$x"));
    TEST(STREQ(json_string(json_parse_string("\"\\u00A2x\"")), "¢x"));
    TEST(STREQ(json_string(json_parse_string("\"\\u20ACx\"")), "€x"));
    TEST(STREQ(json_string(json_parse_string("\"\\uD801\\uDC37x\"")), "𐐷x"));

    puts("Testing invalid strings:");
    malloc_count = 0;
    TEST(json_parse_string(NULL) == NULL);
    TEST(json_parse_string("") == NULL); /* empty string */
    TEST(json_parse_string("[\"lorem\",]") == NULL);
    TEST(json_parse_string("{\"lorem\":\"ipsum\",}") == NULL);
    TEST(json_parse_string("{lorem:ipsum}") == NULL);
    TEST(json_parse_string("[,]") == NULL);
    TEST(json_parse_string("[,") == NULL);
    TEST(json_parse_string("[") == NULL);
    TEST(json_parse_string("]") == NULL);
    TEST(json_parse_string("{\"a\":0,\"a\":0}") == NULL); /* duplicate keys */
    TEST(json_parse_string("{:,}") == NULL);
    TEST(json_parse_string("{,}") == NULL);
    TEST(json_parse_string("{,") == NULL);
    TEST(json_parse_string("{:") == NULL);
    TEST(json_parse_string("{") == NULL);
    TEST(json_parse_string("}") == NULL);
    TEST(json_parse_string("x") == NULL);
    TEST(json_parse_string("{:\"no name\"}") == NULL);
    TEST(json_parse_string("[,\"no first value\"]") == NULL);
    TEST(json_parse_string("{\"key\"\"value\"}") == NULL);
    TEST(json_parse_string("{\"a\"}") == NULL);
    TEST(json_parse_string("[\"\\u00zz\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u00\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\u\"]") == NULL); /* invalid utf value */
    TEST(json_parse_string("[\"\\\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\"\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\0\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\a\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\b\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\t\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\n\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\f\"]") == NULL); /* control character */
    TEST(json_parse_string("[\"\r\"]") == NULL); /* control character */
    TEST(json_parse_string("[0x2]") == NULL);    /* hex */
    TEST(json_parse_string("[0X2]") == NULL);    /* HEX */
    TEST(json_parse_string("[07]") == NULL);     /* octals */
    TEST(json_parse_string("[0070]") == NULL);
    TEST(json_parse_string("[07.0]") == NULL);
    TEST(json_parse_string("[-07]") == NULL);
    TEST(json_parse_string("[-007]") == NULL);
    TEST(json_parse_string("[-07.0]") == NULL);
    TEST(json_parse_string("[\"\\uDF67\\uD834\"]") == NULL); /* wrong order surrogate pair */
    TEST(json_parse_string("[1.7976931348623157e309]") == NULL);
    TEST(json_parse_string("[-1.7976931348623157e309]") == NULL);
    TEST(malloc_count == 0);
}

void test_suite_4() {
    const char *filename = "tests/test_2.txt";
    JSON_Value *a = NULL, *a_copy = NULL;
    printf("Testing %s:\n", filename);
    a = json_parse_file(filename);
    TEST(json_value_equals(a, a)); /* test equality test */
    a_copy = json_value_deep_copy(a);
    TEST(a_copy != NULL);
    TEST(json_value_equals(a, a_copy));
}

void test_suite_5(void) {
    double zero = 0.0; /* msvc is silly (workaround for error C2124) */

    JSON_Value *val_from_file = json_parse_file("tests/test_5.txt");

    JSON_Value *val = NULL, *val_parent;
    JSON_Object *obj = NULL;
    JSON_Array *interests_arr = NULL;

    JSON_Value *remove_test_val = NULL;
    JSON_Array *remove_test_arr = NULL;

    val = json_value_init_object();
    TEST(val != NULL);

    obj = json_value_get_object(val);
    TEST(obj != NULL);

    TEST(json_object_set_string(obj, "first", "John") == JSONSuccess);
    TEST(json_object_set_string(obj, "last", "Doe") == JSONSuccess);
    TEST(json_object_set_number(obj, "age", 25) == JSONSuccess);
    TEST(json_object_set_boolean(obj, "registered", 1) == JSONSuccess);

    TEST(json_object_set_value(obj, "interests", json_value_init_array()) == JSONSuccess);
    interests_arr = json_object_get_array(obj, "interests");
    TEST(interests_arr != NULL);
    TEST(json_array_append_string(interests_arr, "Writing") == JSONSuccess);
    TEST(json_array_append_string(interests_arr, "Mountain Biking") == JSONSuccess);
    TEST(json_array_replace_string(interests_arr, 0, "Reading") == JSONSuccess);

    TEST(json_object_dotset_string(obj, "favorites.color", "blue") == JSONSuccess);
    TEST(json_object_dotset_string(obj, "favorites.sport", "running") == JSONSuccess);
    TEST(json_object_dotset_string(obj, "favorites.fruit", "apple") == JSONSuccess);
    TEST(json_object_dotremove(obj, "favorites.fruit") == JSONSuccess);
    TEST(json_object_set_string(obj, "utf string", "lorem ipsum") == JSONSuccess);
    TEST(json_object_set_string(obj, "utf-8 string", "あいうえお") == JSONSuccess);
    TEST(json_object_set_string(obj, "surrogate string", "lorem𝄞ipsum𝍧lorem") == JSONSuccess);
    TEST(json_object_set_string(obj, "windows path", "C:\\Windows\\Path") == JSONSuccess);
    TEST(json_value_equals(val_from_file, val));

    TEST(json_object_set_string(obj, NULL, "") == JSONFailure);
    TEST(json_object_set_string(obj, "last", NULL) == JSONFailure);
    TEST(json_object_set_string(obj, NULL, NULL) == JSONFailure);
    TEST(json_object_set_value(obj, NULL, NULL) == JSONFailure);

    TEST(json_object_dotset_string(obj, NULL, "") == JSONFailure);
    TEST(json_object_dotset_string(obj, "favorites.color", NULL) == JSONFailure);
    TEST(json_object_dotset_string(obj, NULL, NULL) == JSONFailure);
    TEST(json_object_dotset_value(obj, NULL, NULL) == JSONFailure);

    TEST(json_array_append_string(NULL, "lorem") == JSONFailure);
    TEST(json_array_append_value(interests_arr, NULL) == JSONFailure);
    TEST(json_array_append_value(NULL, NULL) == JSONFailure);

    TEST(json_array_remove(NULL, 0) == JSONFailure);
    TEST(json_array_replace_value(interests_arr, 0, NULL) == JSONFailure);
    TEST(json_array_replace_string(NULL, 0, "lorem") == JSONFailure);
    TEST(json_array_replace_string(interests_arr, 100, "not existing") == JSONFailure);

    TEST(json_array_append_string(json_object_get_array(obj, "interests"), NULL) == JSONFailure);

    TEST(json_array_append_string(interests_arr, "Writing") == JSONSuccess);
    TEST(json_array_remove(interests_arr, 0) == JSONSuccess);
    TEST(json_array_remove(interests_arr, 1) == JSONSuccess);
    TEST(json_array_remove(interests_arr, 0) == JSONSuccess);
    TEST(json_array_remove(interests_arr, 0) == JSONFailure); /* should be empty by now */

    val_parent = json_value_init_null();
    TEST(json_object_set_value(obj, "x", val_parent) == JSONSuccess);
    TEST(json_object_set_value(obj, "x", val_parent) == JSONFailure);

    val_parent = json_value_init_null();
    TEST(json_array_append_value(interests_arr, val_parent) == JSONSuccess);
    TEST(json_array_append_value(interests_arr, val_parent) == JSONFailure);

    val_parent = json_value_init_null();
    TEST(json_array_replace_value(interests_arr, 0, val_parent) == JSONSuccess);
    TEST(json_array_replace_value(interests_arr, 0, val_parent) == JSONFailure);

    TEST(json_object_remove(obj, "interests") == JSONSuccess);

    /* UTF-8 tests */
    TEST(json_object_set_string(obj, "correct string", "κόσμε") == JSONSuccess);

    TEST(json_object_set_string(obj, "boundary 1", "\xed\x9f\xbf") == JSONSuccess);
    TEST(json_object_set_string(obj, "boundary 2", "\xee\x80\x80") == JSONSuccess);
    TEST(json_object_set_string(obj, "boundary 3", "\xef\xbf\xbd") == JSONSuccess);
    TEST(json_object_set_string(obj, "boundary 4", "\xf4\x8f\xbf\xbf") == JSONSuccess);

    TEST(json_object_set_string(obj, "first continuation byte", "\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "last continuation byte", "\xbf") == JSONFailure);

    TEST(json_object_set_string(obj, "impossible sequence 1", "\xfe") == JSONFailure);
    TEST(json_object_set_string(obj, "impossible sequence 2", "\xff") == JSONFailure);
    TEST(json_object_set_string(obj, "impossible sequence 3", "\xfe\xfe\xff\xff") == JSONFailure);

    TEST(json_object_set_string(obj, "overlong 1", "\xc0\xaf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 2", "\xc1\xbf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 3", "\xe0\x80\xaf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 4", "\xe0\x9f\xbf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 5", "\xf0\x80\x80\xaf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 6", "\xf0\x8f\xbf\xbf") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong 7", "\xf0\x8f\xbf\xbf") == JSONFailure);

    TEST(json_object_set_string(obj, "overlong null 1", "\xc0\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong null 2", "\xe0\x80\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong null 3", "\xf0\x80\x80\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong null 4", "\xf8\x80\x80\x80\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "overlong null 5", "\xfc\x80\x80\x80\x80\x80") == JSONFailure);

    TEST(json_object_set_string(obj, "single surrogate 1", "\xed\xa0\x80") == JSONFailure);
    TEST(json_object_set_string(obj, "single surrogate 2", "\xed\xaf\xbf") == JSONFailure);
    TEST(json_object_set_string(obj, "single surrogate 3", "\xed\xbf\xbf") == JSONFailure);

    /* Testing removing values from array, order of the elements should be preserved */
    remove_test_val = json_parse_string("[1, 2, 3, 4, 5]");
    remove_test_arr = json_array(remove_test_val);
    json_array_remove(remove_test_arr, 2);
    TEST(json_value_equals(remove_test_val, json_parse_string("[1, 2, 4, 5]")));
    json_array_remove(remove_test_arr, 0);
    TEST(json_value_equals(remove_test_val, json_parse_string("[2, 4, 5]")));
    json_array_remove(remove_test_arr, 2);
    TEST(json_value_equals(remove_test_val, json_parse_string("[2, 4]")));

    /* Testing nan and inf */
    TEST(json_object_set_number(obj, "num", 0.0 / zero) == JSONFailure);
    TEST(json_object_set_number(obj, "num", 1.0 / zero) == JSONFailure);
}

void test_suite_6(void) {
    const char *filename = "tests/test_2.txt";
    JSON_Value *a = NULL;
    JSON_Value *b = NULL;
    a = json_parse_file(filename);
    b = json_parse_file(filename);
    TEST(json_value_equals(a, b));
    json_object_set_string(json_object(a), "string", "eki");
    TEST(!json_value_equals(a, b));
    a = json_value_deep_copy(b);
    TEST(json_value_equals(a, b));
    json_array_append_number(json_object_get_array(json_object(b), "string array"), 1337);
    TEST(!json_value_equals(a, b));
}

void test_suite_7(void) {
    JSON_Value *val_from_file = json_parse_file("tests/test_5.txt");
    JSON_Value *schema = json_value_init_object();
    JSON_Object *schema_obj = json_value_get_object(schema);
    JSON_Array *interests_arr = NULL;
    json_object_set_string(schema_obj, "first", "");
    json_object_set_string(schema_obj, "last", "");
    json_object_set_number(schema_obj, "age", 0);
    json_object_set_value(schema_obj, "interests", json_value_init_array());
    interests_arr = json_object_get_array(schema_obj, "interests");
    json_array_append_string(interests_arr, "");
    json_object_set_null(schema_obj, "favorites");
    TEST(json_validate(schema, val_from_file) == JSONSuccess);
    json_object_set_string(schema_obj, "age", "");
    TEST(json_validate(schema, val_from_file) == JSONFailure);
}

void test_suite_8(void) {
    const char *filename = "tests/test_2.txt";
    const char *temp_filename = "tests/test_2_serialized.txt";
    JSON_Value *a = NULL;
    JSON_Value *b = NULL;
    char *buf = NULL;
    size_t serialization_size = 0;
    a = json_parse_file(filename);
    TEST(json_serialize_to_file(a, temp_filename) == JSONSuccess);
    b = json_parse_file(temp_filename);
    TEST(json_value_equals(a, b));
    remove(temp_filename);
    serialization_size = json_serialization_size(a);
    buf = json_serialize_to_string(a);
    TEST((strlen(buf)+1) == serialization_size);
}

void test_suite_9(void) {
    const char *filename = "tests/test_2_pretty.txt";
    const char *temp_filename = "tests/test_2_serialized_pretty.txt";
    char *file_contents = NULL;
    char *serialized = NULL;
    JSON_Value *a = NULL;
    JSON_Value *b = NULL;
    size_t serialization_size = 0;
    a = json_parse_file(filename);
    TEST(json_serialize_to_file_pretty(a, temp_filename) == JSONSuccess);
    b = json_parse_file(temp_filename);
    TEST(json_value_equals(a, b));
    remove(temp_filename);
    serialization_size = json_serialization_size_pretty(a);
    serialized = json_serialize_to_string_pretty(a);
    TEST((strlen(serialized)+1) == serialization_size);

    file_contents = read_file(filename);

    TEST(STREQ(file_contents, serialized));
}

void test_suite_10(void) {
    JSON_Value *val;
    char *serialized;

    malloc_count = 0;

    val = json_parse_file("tests/test_1_1.txt");
    json_value_free(val);

    val = json_parse_file("tests/test_1_3.txt");
    json_value_free(val);

    val = json_parse_file("tests/test_2.txt");
    serialized = json_serialize_to_string_pretty(val);
    json_free_serialized_string(serialized);
    json_value_free(val);

    val = json_parse_file("tests/test_2_pretty.txt");
    json_value_free(val);

    TEST(malloc_count == 0);
}

void test_suite_11() {
    const char * array_with_slashes = "[\"a/b/c\"]";
    const char * array_with_escaped_slashes = "[\"a\\/b\\/c\"]";
    char *serialized = NULL;
    JSON_Value *value = json_parse_string(array_with_slashes);

    serialized = json_serialize_to_string(value);
    TEST(STREQ(array_with_escaped_slashes, serialized));

    json_set_escape_slashes(0);
    serialized = json_serialize_to_string(value);
    TEST(STREQ(array_with_slashes, serialized));

    json_set_escape_slashes(1);
    serialized = json_serialize_to_string(value);
    TEST(STREQ(array_with_escaped_slashes, serialized));
}

/* test suite based on https://github.com/nst/JSONTestSuite referenced in http://seriot.ch/parsing_json.php by Nicolas Seriot */
void test_suite_minefield(void){
    JSON_Value *val;
    char* serial;
    char* source;
    FILE *f;
    long fsize;

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/number_1.000000000000000005.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/number_1000000000000000.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/number_10000000000000000999.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/number_1e-999.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/number_1e6.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/object_key_nfc_nfd.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/object_key_nfd_nfc.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/object_same_key_different_values.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/object_same_key_same_value.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/object_same_key_unclear_values.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_1_escaped_invalid_codepoint.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_1_invalid_codepoint.json");  

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_2_escaped_invalid_codepoints.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_2_invalid_codepoints.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_3_escaped_invalid_codepoints.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_3_invalid_codepoints.json");

    COMPARESERIALIZATION("tests/JSONTestSuite/test_transform/string_with_escaped_NULL.json");

    /* non-acceptance */
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_1_true_without_comma.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_a_invalid_utf8.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_colon_instead_of_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_comma_after_close.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_comma_and_number.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_double_comma.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_double_extra_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_extra_close.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_extra_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_incomplete.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_incomplete_invalid_value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_inner_array_no_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_invalid_utf8.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_items_separated_by_semicolon.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_just_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_just_minus.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_missing_value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_newlines_unclosed.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_number_and_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_number_and_several_commas.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_spaces_vertical_tab_formfeed.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_star_inside.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_unclosed.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_unclosed_trailing_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_unclosed_with_new_lines.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_array_unclosed_with_object_inside.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_incomplete_false.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_incomplete_null.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_incomplete_true.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_multidigit_number_then_00.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_++.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_+1.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_+Inf.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_-01.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_-1.0..json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_-2..json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_-NaN.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_.-1.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_.2e-3.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0.1.2.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0.3e+.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0.3e.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0.e1.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0e+.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0e.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0_capital_E+.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_0_capital_E.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_1.0e+.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_1.0e-.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_1.0e.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_1eE2.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_1_000.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_2.e+3.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_2.e-3.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_2.e3.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_9.e+.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_expression.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_hex_1_digit.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_hex_2_digits.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_Inf.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_infinity.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_invalid+-.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_invalid-negative-real.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_invalid-utf-8-in-bigger-int.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_invalid-utf-8-in-exponent.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_invalid-utf-8-in-int.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_minus_infinity.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_minus_sign_with_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_minus_space_1.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_NaN.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_neg_int_starting_with_zero.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_neg_real_without_int_part.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_neg_with_garbage_at_end.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_real_garbage_after_e.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_real_without_fractional_part.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_real_with_invalid_utf8_after_e.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_starting_with_dot.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_U+FF11_fullwidth_digit_one.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_with_alpha.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_with_alpha_char.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_number_with_leading_zero.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_bad_value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_bracket_key.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_comma_instead_of_colon.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_double_colon.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_emoji.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_garbage_at_end.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_key_with_single_quotes.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_lone_continuation_byte_in_key_and_trailing_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_missing_colon.json") == NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_missing_key.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_missing_semicolon.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_missing_value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_no-colon.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_non_string_key.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_non_string_key_but_huge_number_instead.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_repeated_null_null.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_several_trailing_commas.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_single_quote.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_trailing_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_trailing_comment.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_trailing_comment_open.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_trailing_comment_slash_open.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_trailing_comment_slash_open_incomplete.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_two_commas_in_a_row.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_unquoted_key.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_unterminated-value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_with_single_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_object_with_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_single_space.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_1_surrogate_then_escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_1_surrogate_then_escape_u.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_1_surrogate_then_escape_u1.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_1_surrogate_then_escape_u1x.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_accentuated_char_no_quotes.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_backslash_00.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_escaped_backslash_bad.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_escaped_ctrl_char_tab.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_escaped_emoji.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_escape_x.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_incomplete_escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_incomplete_escaped_character.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_incomplete_surrogate.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_incomplete_surrogate_escape_invalid.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_invalid-utf-8-in-escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_invalid_backslash_esc.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_invalid_unicode_escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_invalid_utf8_after_escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_leading_uescaped_thinspace.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_no_quotes_with_bad_escape.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_single_doublequote.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_single_quote.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_single_string_no_double_quotes.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_start_escape_unclosed.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_unescaped_crtl_char.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_unescaped_newline.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_unescaped_tab.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_unicode_CapitalU.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_string_with_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_100000_opening_arrays.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_angle_bracket_..json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_angle_bracket_null.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_array_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_array_with_extra_array_close.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_array_with_unclosed_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_ascii-unicode-identifier.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_capitalized_True.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_close_unopened_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_comma_instead_of_closing_brace.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_double_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_end_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_incomplete_UTF8_BOM.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_lone-invalid-utf-8.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_lone-open-bracket.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_no_data.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_null-byte-outside-string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_number_with_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_object_followed_by_closing_object.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_object_unclosed_no_value.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_object_with_comment.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_object_with_trailing_garbage.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_apostrophe.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_object.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_open_object.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_open_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_array_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object_close_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object_comma.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object_open_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object_open_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_object_string_with_apostrophes.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_open_open.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_single_eacute.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_single_star.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_trailing_#.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_U+2060_word_joined.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_uescaped_LF_before_string.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unclosed_array.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unclosed_array_partial_null.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unclosed_array_unfinished_false.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unclosed_array_unfinished_true.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unclosed_object.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_unicode-identifier.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_UTF8_BOM_no_data.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_whitespace_formfeed.json") == NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/n_structure_whitespace_U+2060_word_joiner.json") == NULL);

    /* acceptance: */
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_arraysWithSpaces.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_empty-string.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_empty.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_ending_with_newline.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_false.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_heterogeneous.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_null.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_with_1_and_newline.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_with_leading_space.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_with_several_null.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_array_with_trailing_space.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_0e+1.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_0e1.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_after_space.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_double_close_to_zero.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_int_with_exp.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_minus_zero.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_negative_int.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_negative_one.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_negative_zero.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_capital_e.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_capital_e_neg_exp.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_capital_e_pos_exp.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_exponent.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_fraction_exponent.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_neg_exp.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_real_pos_exponent.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_simple_int.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_number_simple_real.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_basic.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_duplicated_key.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_duplicated_key_and_value.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_empty.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_empty_key.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_escaped_null_in_key.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_extreme_numbers.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_long_strings.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_simple.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_string_unicode.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_object_with_newlines.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_1_2_3_bytes_UTF-8_sequences.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_accepted_surrogate_pair.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_accepted_surrogate_pairs.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_allowed_escapes.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_backslash_and_u_escaped_zero.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_backslash_doublequotes.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_comments.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_double_escape_a.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_double_escape_n.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_escaped_control_character.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_escaped_noncharacter.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_in_array.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_in_array_with_leading_space.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_last_surrogates_1_and_2.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_nbsp_uescaped.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_nonCharacterInUTF-8_U+10FFFF.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_nonCharacterInUTF-8_U+FFFF.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_null_escape.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_one-byte-utf-8.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_pi.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_reservedCharacterInUTF-8_U+1BFFF.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_simple_ascii.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_space.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_surrogates_U+1D11E_MUSICAL_SYMBOL_G_CLEF.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_three-byte-utf-8.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_two-byte-utf-8.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_u+2028_line_sep.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_u+2029_par_sep.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_uEscape.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_uescaped_newline.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unescaped_char_delete.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicodeEscapedBackslash.json") != NULL); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_2.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_escaped_double_quote.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+10FFFE_nonchar.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+1FFFE_nonchar.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+200B_ZERO_WIDTH_SPACE.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+2064_invisible_plus.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+FDD0_nonchar.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_unicode_U+FFFE_nonchar.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_utf8.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_string_with_del_character.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_false.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_int.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_negative_real.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_null.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_string.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_lonely_true.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_string_empty.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_trailing_newline.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_true_in_array.json") != NULL);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/y_structure_whitespace_array.json") != NULL);

    /* undefined */
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_double_huge_neg_exp.json") || 1); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_huge_exp.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_neg_int_huge_exp.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_pos_double_huge_exp.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_real_neg_overflow.json") || 1); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_real_pos_overflow.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_real_underflow.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_too_big_neg_int.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_too_big_pos_int.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_number_very_big_negative_int.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_object_key_lone_2nd_surrogate.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_1st_surrogate_but_2nd_missing.json") || 1); 
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_1st_valid_surrogate_2nd_invalid.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_incomplete_surrogates_escape_valid.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_incomplete_surrogate_and_escape_valid.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_incomplete_surrogate_pair.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_invalid_lonely_surrogate.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_invalid_surrogate.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_invalid_utf-8.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_inverted_surrogates_U+1D11E.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_iso_latin_1.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_lone_second_surrogate.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_lone_utf8_continuation_byte.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_not_in_unicode_range.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_overlong_sequence_2_bytes.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_overlong_sequence_6_bytes.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_overlong_sequence_6_bytes_null.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_truncated-utf-8.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_UTF-16LE_with_BOM.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_UTF-8_invalid_sequence.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_utf16BE_no_BOM.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_utf16LE_no_BOM.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_string_UTF8_surrogate_U+D800.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_structure_500_nested_arrays.json") || 1);
    TEST(json_parse_file("tests/JSONTestSuite/test_parsing/i_structure_UTF-8_BOM_empty_object.json") || 1);
}

void print_commits_info(const char *username, const char *repo) {
    JSON_Value *root_value;
    JSON_Array *commits;
    JSON_Object *commit;
    size_t i;

    char curl_command[512];
    char cleanup_command[256];
    char output_filename[] = "commits.json";

    /* it ain't pretty, but it's not a libcurl tutorial */
    sprintf(curl_command,
        "curl -s \"https://api.github.com/repos/%s/%s/commits\" > %s",
        username, repo, output_filename);
    sprintf(cleanup_command, "rm -f %s", output_filename);
    system(curl_command);

    /* parsing json and validating output */
    root_value = json_parse_file(output_filename);
    if (json_value_get_type(root_value) != JSONArray) {
        system(cleanup_command);
        return;
    }

    /* getting array from root value and printing commit info */
    commits = json_value_get_array(root_value);
    printf("%-10.10s %-10.10s %s\n", "Date", "SHA", "Author");
    for (i = 0; i < json_array_get_count(commits); i++) {
        commit = json_array_get_object(commits, i);
        printf("%.10s %.10s %s\n",
               json_object_dotget_string(commit, "commit.author.date"),
               json_object_get_string(commit, "sha"),
               json_object_dotget_string(commit, "commit.author.name"));
    }

    /* cleanup code */
    json_value_free(root_value);
    system(cleanup_command);
}

void persistence_example(void) {
    JSON_Value *schema = json_parse_string("{\"name\":\"\"}");
    JSON_Value *user_data = json_parse_file("user_data.json");
    char buf[256];
    const char *name = NULL;
    if (user_data == NULL || json_validate(schema, user_data) != JSONSuccess) {
        puts("Enter your name:");
        scanf("%s", buf);
        user_data = json_value_init_object();
        json_object_set_string(json_object(user_data), "name", buf);
        json_serialize_to_file(user_data, "user_data.json");
    }
    name = json_object_get_string(json_object(user_data), "name");
    printf("Hello, %s.", name);
    json_value_free(schema);
    json_value_free(user_data);
    return;
}

void serialization_example(void) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "name", "John Smith");
    json_object_set_number(root_object, "age", 25);
    json_object_dotset_string(root_object, "address.city", "Cupertino");
    json_object_dotset_value(root_object, "contact.emails",
                             json_parse_string("[\"email@example.com\", \"email2@example.com\"]"));
    serialized_string = json_serialize_to_string_pretty(root_value);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

static char * read_file(const char * filename) {
    FILE *fp = fopen(filename, "r");
    size_t size_to_read = 0;
    size_t size_read = 0;
    long pos;
    char *file_contents;
    if (!fp) {
        return NULL;
    }
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return NULL;
    }
    size_to_read = pos;
    rewind(fp);
    file_contents = (char*)malloc(sizeof(char) * (size_to_read + 1));
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    size_read = fread(file_contents, 1, size_to_read, fp);
    if (size_read == 0 || ferror(fp)) {
        fclose(fp);
        free(file_contents);
        return NULL;
    }
    fclose(fp);
    file_contents[size_read] = '\0';
    return file_contents;
}

static void *counted_malloc(size_t size) {
    void *res = malloc(size);
    if (res != NULL) {
        malloc_count++;
    }
    return res;
}

static void counted_free(void *ptr) {
    if (ptr != NULL) {
        malloc_count--;
    }
    free(ptr);
}
