/*
 Parson ( http://kgabis.github.com/parson/ )
 Copyright (c) 2012 - 2014 Krzysztof Gabis

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

#define TEST(A) printf("%-72s-",#A);              \
                if(A){puts(" OK");tests_passed++;} \
                else{puts(" FAIL");tests_failed++;}
#define STREQ(A, B) ((A) && (B) ? strcmp((A), (B)) == 0 : 0)
#define EPSILON 0.000001

void test_suite_1(void); /* Test 3 files from json.org + serialization*/
void test_suite_2(JSON_Value *value); /* Test correctness of parsed values */
void test_suite_2_no_comments(void);
void test_suite_2_with_comments(void);
void test_suite_3(void); /* Test incorrect values */
void test_suite_4(void); /* Test deep copy funtion */
void test_suite_5(void); /* Test building json values from scratch */
void test_suite_6(void); /* Test value comparing verification */
void test_suite_7(void); /* Test schema validation */
void test_suite_8(void); /* Test serialization to file */

void print_commits_info(const char *username, const char *repo);
void persistence_example(void);
void serialization_example(void);
void test_pretty_serialization(void);
void test_count_characters(void);
void test_serialize_to_string(void);

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
    test_pretty_serialization();
    test_count_characters();
    test_serialize_to_string();
    printf("Tests failed: %d\n", tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    return 0;
}

void test_suite_1(void) {
    JSON_Value *val;
    TEST((val = json_parse_file("tests/test_1_1.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file("tests/test_1_2.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }

    TEST((val = json_parse_file_with_comments("tests/test_1_1.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_2.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }
    TEST((val = json_parse_file_with_comments("tests/test_1_3.txt")) != NULL);
    TEST(json_value_equals(json_parse_string(json_serialize_to_string(val)), val));
    if (val) { json_value_free(val); }
}

void test_suite_2(JSON_Value *root_value) {
    JSON_Object *root_object;
    JSON_Array *array;
    size_t i;
    TEST(root_value);
    TEST(json_value_get_type(root_value) == JSONObject);
    root_object = json_value_get_object(root_value);
    TEST(STREQ(json_object_get_string(root_object, "string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf string"), "lorem ipsum"));
    TEST(STREQ(json_object_get_string(root_object, "utf-8 string"), "\xe3\x81\x82\xe3\x81\x84\xe3\x81\x86\xe3\x81\x88\xe3\x81\x8a"));
    TEST(STREQ(json_object_get_string(root_object, "surrogate string"), "lorem\xF0\x9d\x84\x9eipsum\xf0\x9d\x8d\xa7lorem"));
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
    TEST(json_object_dotget_boolean(root_object, "nested true"));

    TEST(STREQ(json_object_get_string(root_object, "/**/"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "//"), "comment"));
    TEST(STREQ(json_object_get_string(root_object, "url"), "https://www.example.com/search?q=12345"));
    TEST(STREQ(json_object_get_string(root_object, "escaped chars"), "\" \\ /"));
}

void test_suite_2_no_comments(void) {
    const char *filename = "tests/test_2.txt";
    JSON_Value *root_value = NULL;
    root_value = json_parse_file(filename);
    test_suite_2(root_value);
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string(root_value))));
    json_value_free(root_value);
}

void test_suite_2_with_comments(void) {
    const char *filename = "tests/test_2_comments.txt";
    JSON_Value *root_value = NULL;
    root_value = json_parse_file_with_comments(filename);
    test_suite_2(root_value);
    TEST(json_value_equals(root_value, json_parse_string(json_serialize_to_string(root_value))));
    json_value_free(root_value);
}

void test_suite_3(void) {
    char nested_20x[] = "[[[[[[[[[[[[[[[[[[[[\"hi\"]]]]]]]]]]]]]]]]]]]]";
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
    TEST(json_parse_string("\"string\"") == NULL);
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

    JSON_Value *val = json_value_init_object();
    JSON_Object *obj = json_value_get_object(val);
    TEST(json_object_set_string(obj, "first", "John") == JSONSuccess);
    TEST(json_object_set_string(obj, "last", "Doe") == JSONSuccess);
    TEST(json_object_set_number(obj, "age", 25) == JSONSuccess);
    TEST(json_object_set_boolean(obj, "registered", 1) == JSONSuccess);
    TEST(json_object_set_value(obj, "interests", json_value_init_array()) == JSONSuccess);
    TEST(json_array_append_string(json_object_get_array(obj, "interests"), "Writing") == JSONSuccess);
    TEST(json_array_append_string(json_object_get_array(obj, "interests"), "Mountain Biking") == JSONSuccess);
    TEST(json_array_replace_string(json_object_get_array(obj, "interests"), 0, "Reading") == JSONSuccess);
    TEST(json_object_dotset_string(obj, "favorites.color", "blue") == JSONSuccess);
    TEST(json_object_dotset_string(obj, "favorites.sport", "running") == JSONSuccess);
    TEST(json_object_dotset_string(obj, "favorites.fruit", "apple") == JSONSuccess);
    TEST(json_object_dotremove(obj, "favorites.fruit") == JSONSuccess);
    TEST(json_object_set_string(obj, "utf string", "\\u006corem\\u0020ipsum") == JSONSuccess);
    TEST(json_object_set_string(obj, "utf-8 string", "あいうえお") == JSONSuccess);
    TEST(json_object_set_string(obj, "surrogate string", "lorem\\uD834\\uDD1Eipsum\\uD834\\uDF67lorem") == JSONSuccess);
    TEST(json_value_equals(val_from_file, val));
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
    json_object_set_string(schema_obj, "first", "");
    json_object_set_string(schema_obj, "last", "");
    json_object_set_number(schema_obj, "age", 0);
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
    a = json_parse_file(filename);
    TEST(json_serialize_to_file(a, temp_filename) == JSONSuccess);
    b = json_parse_file(temp_filename);
    TEST(json_value_equals(a, b));
    remove(temp_filename);
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
    serialized_string = json_serialize_to_string(root_value);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

int file_equals(const char *file1, const char *file2)
{
    FILE * fp1;
    FILE * fp2;
    char ch1, ch2;
    unsigned long offset = 0ul;

    fp1 = fopen(file1, "rb");
    if (fp1 == NULL)
    {
        return 0;
    }

    fp2 = fopen(file2, "rb");
    if (fp2 == NULL)
    {
        fclose(fp1);
        return 0;
    }

    while(!feof(fp1) && !feof(fp2))
    {
        ch1 = fgetc(fp1);
        if (ferror(fp1))
        {
            return 0;
        }

        ch2 = fgetc(fp2);
        if (ferror(fp2))
        {
            return 0;
        }

        if (ch1 != ch2)
        {
            return 0;
            break;
        }
        offset++;
    }

    if (feof(fp1) && feof(fp2))
    {
        return 1;
    }

    return 0;
}

JSON_Value * build_test_value()
{
    JSON_Value *rootValue;
    JSON_Object *root;

    JSON_Value *innerValue;
    JSON_Object *inner;

    rootValue = json_value_init_object();
    root = json_value_get_object(rootValue);
    json_object_set_number(root, "val", 11.3);

    innerValue = json_value_init_object();
    inner = json_value_get_object(innerValue);
    json_object_set_number(inner, "a", 1);
    json_object_set_number(inner, "b", 1.2);
    json_object_set_string(inner, "c", "two");
    json_object_set_boolean(inner, "d", 1);
    json_object_set_boolean(inner, "e", 0);

    json_object_set_boolean(inner, "e", 0);
    json_object_set_value(root, "inner", innerValue);

    {
        JSON_Value *arrayValue;
        JSON_Array *array;
        double val;

        arrayValue = json_value_init_array();
        array = json_value_get_array(arrayValue);
        for (val = 0.0; val < 10; val += 0.1)
        {
            json_array_append_number(array, val);
        }

        json_object_set_value(root, "arr", arrayValue);
    }

    return rootValue;
}

void test_pretty_serialization()
{
    JSON_Value *rootValue;

    rootValue = build_test_value();

    json_serialize_to_file_pretty(rootValue, "out.json");
    json_value_free(rootValue);

    TEST(file_equals("out.json", "tests/pretty.json"));
}

void test_count_characters(void)
{
    JSON_Value *rootValue;
    size_t string_length;

    rootValue = build_test_value();

    string_length = json_count_characters(rootValue);
    json_value_free(rootValue);

    TEST(string_length == 1986);
}

void test_serialize_to_string(void)
{
    char *str;
    JSON_Value *rootValue;
    size_t string_length;

    rootValue = build_test_value();

    str = json_serialize_to_string_pretty(rootValue);
    json_value_free(rootValue);
    string_length = strlen(str);
    json_free_serialized_string(str);
    TEST(string_length == 1986);
}
