/*
 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2016 Krzysztof Gabis

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

#define TEST(A) printf("%-72s-",#A);\
                if(A){puts(" OK");tests_passed++;}\
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define EPSILON 0.000001

void test_suite_1(void); /* Test 3 files from json.org + serialization*/
void test_suite_2(JSON_Value *value); /* Test correctness of parsed values */
void test_suite_2_no_comments(void);
void test_suite_2_with_comments(void);
void test_suite_3(void); /* Test parsing valid and invalid strings */
void test_suite_4(void); /* Test deep copy funtion */
void test_suite_5(void); /* Test building json values from scratch */
void test_suite_6(void); /* Test value comparing verification */
void test_suite_7(void); /* Test schema validation */
void test_suite_8(void); /* Test serialization */
void test_suite_9(void); /* Test serialization (pretty) */
void test_suite_10(void); /* Test string encoding */

void print_commits_info(const char *username, const char *repo);
void persistence_example(void);
void serialization_example(void);

static char * read_file(const char * filename);

static int tests_passed;
static int tests_failed;

int main() {
    /* Example functions from readme file:      */
    /* print_commits_info("torvalds", "linux"); */
    /* serialization_example(); */
    /* persistence_example(); */

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

    TEST((val = json_parse_file("tests/test_1_2.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_1.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_2.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    TEST(json_value_equals(json_parse_string(json_serialize_to_string_pretty(val)), val));
    if (val) { json_value_free(val); }
}

void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
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
    char nested_20x[] = "[[[[[[[[[[[[[[[[[[[[\"hi\"]]]]]]]]]]]]]]]]]]]]";

    puts("Testing valid strings:");
    TEST(json_parse_string("{\"lorem\":\"ipsum\"}") != NULL);
    TEST(json_parse_string("[\"lorem\"]") != NULL);
    TEST(json_parse_string("null") != NULL);
    TEST(json_parse_string("true") != NULL);
    TEST(json_parse_string("false") != NULL);
    TEST(json_parse_string("\"string\"") != NULL);
    TEST(json_parse_string("123") != NULL);

    puts("Testing invalid strings:");
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
    TEST(json_parse_string(nested_20x) == NULL); /* too deep */
    TEST(json_parse_string("[0x2]") == NULL);    /* hex */
    TEST(json_parse_string("[0X2]") == NULL);    /* HEX */
    TEST(json_parse_string("[07]") == NULL);     /* octals */
    TEST(json_parse_string("[0070]") == NULL);
    TEST(json_parse_string("[07.0]") == NULL);
    TEST(json_parse_string("[-07]") == NULL);
    TEST(json_parse_string("[-007]") == NULL);
    TEST(json_parse_string("[-07.0]") == NULL);
    TEST(json_parse_string("[\"\\uDF67\\uD834\"]") == NULL); /* wrong order surrogate pair */
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
    JSON_Value *val_from_file = json_parse_file("tests/test_5.txt");

    JSON_Value *val = NULL;
    JSON_Object *obj = NULL;
    JSON_Array *interests_arr = NULL;

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
    const char *raw_str = "\"\\/\b\f\n\r\t\"test";
    char *enc_str = json_encode_string(raw_str);

    TEST(STREQ(enc_str, "\"\\\"\\\\\\/\\b\\f\\n\\r\\t\\\"test\""));
    free(enc_str);
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
    size_t file_size;
    long pos;
    char *file_contents;
    if (!fp)
        return NULL;
    fseek(fp, 0L, SEEK_END);
    pos = ftell(fp);
    if (pos < 0) {
        fclose(fp);
        return NULL;
    }
    file_size = pos;
    rewind(fp);
    file_contents = (char*)malloc(sizeof(char) * (file_size + 1));
    if (!file_contents) {
        fclose(fp);
        return NULL;
    }
    if (fread(file_contents, file_size, 1, fp) < 1) {
        if (ferror(fp)) {
            fclose(fp);
            free(file_contents);
            return NULL;
        }
    }
    fclose(fp);
    file_contents[file_size] = '\0';
    return file_contents;
}
