/*
 Copyright (c) 2012 Krzysztof Gabis
 
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "parson.h"

#define STARTING_CAPACITY 10
#define sizeof_token(a) (sizeof(a) - 1)

/* Type definitions */
union json_value_value {
    const char * string;
    double number;
    JSON_Object *object;
    JSON_Array *array;
    int boolean;
    int null;
};

struct json_value_t {
    enum json_value_type type;
    union json_value_value value;
};

struct json_object_t {
    const char **names;
    JSON_Value **values;
    size_t count;
    size_t capacity;
};

struct json_array_t {
    JSON_Value **items;
    size_t count;
    size_t capacity;
};

/* JSON Object */
static JSON_Object * json_object_init();
static int json_object_add(JSON_Object *object, const char *name, JSON_Value *value);
static void json_object_free(JSON_Object *object);

/* JSON Array */
static JSON_Array * json_array_init();
static void json_array_add(JSON_Array *array, JSON_Value *value);
static void json_array_free(JSON_Array *array);

/* JSON Value */
static JSON_Value * json_value_init_object();
static JSON_Value * json_value_init_array();
static JSON_Value * json_value_init_string(const char *string);
static JSON_Value * json_value_init_number(double number);
static JSON_Value * json_value_init_boolean(int boolean);
static JSON_Value * json_value_init_null();

/* Parser */
char * parson_strndup(const char *string, size_t n);
char * parson_strdup(const char *string);
static const char * skip_string(const char *string);
static char * copy_and_remove_whitespaces(const char *string);
static int is_utf_string(const char *string);
static const char * parse_escaped_characters(const char *string);
static const char * get_string(const char **string);
static JSON_Value * parse_object_value(const char **string);
static JSON_Value * parse_array_value(const char **string);
static JSON_Value * parse_string_value(const char **string);
static JSON_Value * parse_boolean_value(const char **string);
static JSON_Value * parse_number_value(const char **string);
static JSON_Value * parse_null_value(const char **string);
static JSON_Value * parse_value(const char **string);

/* JSON Object */
static JSON_Object * json_object_init() {
    JSON_Object *new_object = (JSON_Object*)malloc(sizeof(JSON_Object));
    new_object->names = (const char**)malloc(sizeof(char*) * STARTING_CAPACITY);
    new_object->values = (JSON_Value**)malloc(sizeof(JSON_Value*) * STARTING_CAPACITY);
    new_object->capacity = STARTING_CAPACITY;
    new_object->count = 0;
    return new_object;
}

static int json_object_add(JSON_Object *object, const char *name, JSON_Value *value) {
    size_t index;
    if (object->count >= object->capacity) {
        size_t new_capacity = object->capacity * 2;
        object->names = (const char**)realloc((void*)object->names, new_capacity * sizeof(char*));
        object->values = (JSON_Value**)realloc(object->values, new_capacity * sizeof(JSON_Value*));
        object->capacity = new_capacity;
    }    
    if (json_object_get_value(object, name) != NULL) { return 0; }    
    index = object->count;
    object->names[index] = parson_strdup(name);
    object->values[index] = value;
    object->count++;
    return 1;
}

static void json_object_free(JSON_Object *object) {
    size_t i;
    for (i = 0; i < object->count; i++) {
        free((void*)object->names[i]);
        json_value_free(object->values[i]);
    }
    free((void*)object->names);
    free(object->values);
    free(object);
}

/* JSON Array */
static JSON_Array * json_array_init() {
    JSON_Array *new_array = (JSON_Array*)malloc(sizeof(JSON_Array));
    new_array->items = (JSON_Value**)malloc(STARTING_CAPACITY * sizeof(JSON_Value*));
    new_array->capacity = STARTING_CAPACITY;
    new_array->count = 0;
    return new_array;
}

static void json_array_add(JSON_Array *array, JSON_Value *value) {
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        array->items = (JSON_Value**)realloc(array->items, new_capacity * sizeof(JSON_Value*));
        array->capacity = new_capacity;
    }
    array->items[array->count] = value;
    array->count++;
}

static void json_array_free(JSON_Array *array) {
    size_t i;
    for (i = 0; i < array->count; i++) {
        json_value_free(array->items[i]);
    }
    free(array->items);
    free(array);
}

/* JSON Value */
static JSON_Value * json_value_init_object() {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONObject;
    new_value->value.object = json_object_init();
    return new_value;
}

static JSON_Value * json_value_init_array() {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONArray;
    new_value->value.array = json_array_init();
    return new_value;
}

static JSON_Value * json_value_init_string(const char *string) {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONString;
    new_value->value.string = string;
    return new_value;
}

static JSON_Value * json_value_init_number(double number) {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONNumber;
    new_value->value.number = number;
    return new_value;
}

static JSON_Value * json_value_init_boolean(int boolean) {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONBoolean;
    new_value->value.boolean = boolean;
    return new_value;
}

static JSON_Value * json_value_init_null() {
    JSON_Value *new_value = (JSON_Value*)malloc(sizeof(JSON_Value));
    new_value->type = JSONNull;
    return new_value;
}

/* Parser */
char * parson_strndup(const char *string, size_t n) {
    char *output_string = (char*)calloc(n + 1, 1);
    strncpy(output_string, string, n);
    return output_string;
}

char * parson_strdup(const char *string) {
    return parson_strndup(string, strlen(string));
}

static const char * skip_string(const char *string) {
    string++;
    while (*string != '\0' && *string != '\"') {
        if (*string == '\\') { string++; if (*string == '\0') { break; } }
        string++;
    }
    if (*string == '\0') { return NULL; }
    return string + 1;
}

static char *copy_and_remove_whitespaces(const char *string) {
    char *output_string = (char*)malloc(strlen(string) + 1);
    char *output_string_ptr = output_string;
    const char *string_ptr = string;
    const char *skipped_string = NULL;
    char current_char;    
    while (*string_ptr) {
        current_char = *string_ptr;        
        switch (current_char) {
            case ' ': case '\r': case '\n': case '\t':
                string_ptr++;
                break;
            case '\"':
                skipped_string = skip_string(string_ptr);
                if (skipped_string == NULL) { free(output_string); return NULL; }
                strncpy(output_string_ptr, string_ptr, skipped_string - string_ptr);
                output_string_ptr = output_string_ptr + (skipped_string - string_ptr);
                string_ptr = skipped_string;
                break;
            default:
                *output_string_ptr = current_char;
                string_ptr++;
                output_string_ptr++;
                break;
        }
    }
    *output_string_ptr = '\0';
    output_string = (char*)realloc(output_string, strlen(output_string) + 1);
    return output_string;
}

static int is_utf_string(const char *string) {
    int i;
    if (strlen(string) < 4) { return 0; }
    for (i = 0; i < 4; i++) { if (!isxdigit(string[i])) { return 0; } }
    return 1;
}

static const char * parse_escaped_characters(const char *string) {
    char *output_string = (char*)malloc(strlen(string) + 1);
    char *output_string_ptr = output_string;
    const char *string_ptr = string;
    char current_char;
    unsigned int utf_val;    
    while (*string_ptr) {
        current_char = *string_ptr;
        if (current_char == '\\') {
            string_ptr++;
            current_char = *string_ptr;
            switch (current_char) {
                case '\"': case '\\': case '/': break;
                case 'b': current_char = '\b'; break;
                case 'f': current_char = '\f'; break;
                case 'n': current_char = '\n'; break;
                case 'r': current_char = '\r'; break;
                case 't': current_char = '\t'; break;
                case 'u':
                    string_ptr++;
                    if (!is_utf_string(string_ptr) ||
                            sscanf(string_ptr, "%4x", &utf_val) == EOF) {
                        free(output_string); return NULL;
                    }
                    if (utf_val < 0x80) {
                        current_char = utf_val;
                    } else if (utf_val < 0x800) {
                        *output_string_ptr++ = (utf_val >> 6) | 0xC0;
                        current_char = ((utf_val | 0x80) & 0xBF);
                    } else {
                        *output_string_ptr++ = (utf_val >> 12) | 0xE0;
                        *output_string_ptr++ = (((utf_val >> 6) | 0x80) & 0xBF);
                        current_char = ((utf_val | 0x80) & 0xBF);
                    }
                    string_ptr += 3;
 					break;
                default:
                    free(output_string);
                    return NULL;
                    break;
            }
        } else if (iscntrl(current_char)) { /* no control characters allowed */
            free(output_string); 
            return NULL;
        }                   
        *output_string_ptr = current_char;
        output_string_ptr++;
        string_ptr++;
    }
    *output_string_ptr = '\0';
    output_string = (char*)realloc(output_string, strlen(output_string) + 1);
    return output_string;
}

/* Returns contents of a string inside double quotes and parses escaped
 characters inside.
 Example: "\u006Corem ipsum" -> lorem ipsum */
static const char * get_string(const char **string) {
    char *quote_contents;
    const char *parsed_string;
    const char *after_closing_quote_ptr = skip_string(*string);    
    if (after_closing_quote_ptr == NULL) { return NULL; }
    quote_contents = parson_strndup(*string + 1, after_closing_quote_ptr - *string - 2);
    *string = after_closing_quote_ptr;
    parsed_string = parse_escaped_characters(quote_contents);
    free(quote_contents);
    return (const char*)parsed_string;
}

static JSON_Value * parse_value(const char **string) {
    JSON_Value *output_value = NULL;
    if (*string == NULL) { return NULL; }
    switch ((*string)[0]) {
        case '{':
            output_value = parse_object_value(string);
            break;
        case '[':
            output_value = parse_array_value(string);
            break;
        case '\"':
            output_value = parse_string_value(string);
            break;
        case 'f':
        case 't':
            output_value = parse_boolean_value(string);
            break;
        case '-':
        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            output_value = parse_number_value(string);
            break;
        case 'n':
            output_value = parse_null_value(string);
            break;
        default:
            return NULL;
    }
    return output_value;
}

static JSON_Value * parse_object_value(const char **string) {
    JSON_Value *output_value = json_value_init_object();
    const char *new_key = NULL;
    JSON_Value *new_value = NULL;    
    (*string)++;
    if (**string == '}') { (*string)++; return output_value; } /* empty object */
    while (**string != '\0') {
        new_key = get_string(string);
        if (new_key == NULL || **string != ':') {
            json_value_free(output_value);
            return NULL;
        }
        (*string)++;
        new_value = parse_value(string);
        if (new_value == NULL) {
            free((void*)new_key);
            json_value_free(output_value);
            return NULL;
        }
        if(!json_object_add(json_value_get_object(output_value), new_key, new_value)) {
            free((void*)new_key);
            free(new_value);
            json_value_free(output_value);
            return NULL;
        }
        free((void*)new_key);        
        if (**string != ',') { break; }
        (*string)++;
    }
    if (**string != '}') { json_value_free(output_value); return NULL; }
    (*string)++;
    return output_value;
}

static JSON_Value * parse_array_value(const char **string) {
    JSON_Value *output_value = json_value_init_array();
    JSON_Value *new_array_value = NULL;    
    (*string)++;
    if (**string == ']') { /* empty array */
        (*string)++;
        return output_value;
    }
    while (**string != '\0') {
        new_array_value = parse_value(string);
        if (new_array_value == NULL) {
            json_value_free(output_value);
            return NULL;
        }
        json_array_add(json_value_get_array(output_value), new_array_value);
        if (**string != ',') { break; }
        (*string)++; 
    }
    if (**string != ']') {
        json_value_free(output_value);
        return NULL;
    }
    (*string)++;
    return output_value;
}

static JSON_Value * parse_string_value(const char **string) {
    const char *new_string = get_string(string);
    if (new_string == NULL) { return NULL; }
    return json_value_init_string(new_string);
}

static JSON_Value * parse_boolean_value(const char **string) {
    size_t true_token_size = sizeof_token("true");
    size_t false_token_size = sizeof_token("false");    
    if (strncmp("true", *string, true_token_size) == 0) {        
        *string += true_token_size;
        return json_value_init_boolean(1);
    } else if (strncmp("false", *string, false_token_size) == 0) {
        *string += false_token_size;
        return json_value_init_boolean(0);
    }
    return NULL;
}

static JSON_Value * parse_number_value(const char **string) {
    return json_value_init_number(strtod(*string, (char**)string));
}

static JSON_Value * parse_null_value(const char **string) {
    size_t token_size = sizeof_token("null");
    if (strncmp("null", *string, token_size) == 0) {
        *string += token_size;
        return json_value_init_null();
    }
    return NULL;
}

/* Parser API */
JSON_Value * json_parse_file(const char *filename) {
    FILE *fp = fopen(filename, "r");
    size_t file_size;
    char *file_contents;
    JSON_Value *output_value;    
    if (fp == NULL) { return NULL; }
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    file_contents = (char*)malloc(sizeof(char) * (file_size + 1));
    fread(file_contents, file_size, 1, fp);
    fclose(fp);
    file_contents[file_size] = '\0';
    output_value = json_parse_string(file_contents);
    free(file_contents);
    return output_value;
}

JSON_Value * json_parse_string(const char *string) {
    JSON_Value *output_value = NULL;
    const char *json_string = string ? copy_and_remove_whitespaces(string) : NULL;
    const char *json_string_ptr = json_string;
    if (json_string == NULL) { return NULL; }
    if (*json_string == '{' || *json_string == '[') {
        output_value = parse_value((const char**)&json_string_ptr);
    }
    free((void*)json_string);
    return output_value;
}

/* JSON Object API */
JSON_Value * json_object_get_value(const JSON_Object *object, const char *name) {
    size_t i;
    if (object == NULL) { return NULL; }
    for (i = 0; i < object->count; i++) {
        if (strcmp(object->names[i], name) == 0) { return object->values[i]; }
    }
    return NULL;
}

const char * json_object_get_string(const JSON_Object *object, const char *name) {
    return json_value_get_string(json_object_get_value(object, name));
}

double json_object_get_number(const JSON_Object *object, const char *name) {
    return json_value_get_number(json_object_get_value(object, name));
}

JSON_Object * json_object_get_object(const JSON_Object *object, const char *name) {
    return json_value_get_object(json_object_get_value(object, name));
}

JSON_Array * json_object_get_array(const JSON_Object *object, const char *name) {
    return json_value_get_array(json_object_get_value(object, name));
}

int json_object_get_boolean(const JSON_Object *object, const char *name) {
    return json_value_get_boolean(json_object_get_value(object, name));
}

JSON_Value * json_object_dotget_value(const JSON_Object *object, const char *name) {
    const char *object_name, *dot_position = strchr(name, '.');
    JSON_Value *output_value;
    if (dot_position == NULL) { return json_object_get_value(object, name); }
    object_name = parson_strndup(name, dot_position - name);
    output_value = json_object_dotget_value(json_object_get_object(object, object_name),
                                    dot_position + 1);
    free((void*)object_name);
    return output_value;
}

const char * json_object_dotget_string(const JSON_Object *object, const char *name) {
    return json_value_get_string(json_object_dotget_value(object, name));
}

double json_object_dotget_number(const JSON_Object *object, const char *name) {
    return json_value_get_number(json_object_dotget_value(object, name));
}

JSON_Object * json_object_dotget_object(const JSON_Object *object, const char *name) {
    return json_value_get_object(json_object_dotget_value(object, name));
}

JSON_Array * json_object_dotget_array(const JSON_Object *object, const char *name) {
    return json_value_get_array(json_object_dotget_value(object, name));
}

int json_object_dotget_boolean(const JSON_Object *object, const char *name) {
    return json_value_get_boolean(json_object_dotget_value(object, name));
}

/* JSON Array API */
JSON_Value * json_array_get_value(const JSON_Array *array, size_t index) {
    if (index >= json_array_get_count(array)) { return NULL; }
    return array->items[index];
}

const char * json_array_get_string(const JSON_Array *array, size_t index) {
    return json_value_get_string(json_array_get_value(array, index));
}

double json_array_get_number(const JSON_Array *array, size_t index) {
    return json_value_get_number(json_array_get_value(array, index));
}

JSON_Object * json_array_get_object(const JSON_Array *array, size_t index) {
    return json_value_get_object(json_array_get_value(array, index));
}

JSON_Array * json_array_get_array(const JSON_Array *array, size_t index) {
    return json_value_get_array(json_array_get_value(array, index));
}

int json_array_get_boolean(const JSON_Array *array, size_t index) {
    return json_value_get_boolean(json_array_get_value(array, index));
}

size_t json_array_get_count(const JSON_Array *array) {
    return array != NULL ? array->count : 0;
}

/* JSON Value API */
enum json_value_type json_value_get_type(const JSON_Value *value) {
    return value != NULL ? value->type : JSONError;
}

JSON_Object * json_value_get_object(const JSON_Value *value) {
    if (value == NULL || value->type != JSONObject) { return NULL; }
    return value->value.object;
}

JSON_Array * json_value_get_array(const JSON_Value *value) {
    if (value == NULL || value->type != JSONArray) { return NULL; }
    return value->value.array;
}

const char * json_value_get_string(const JSON_Value *value) {
    if (value == NULL || value->type != JSONString) { return NULL; }
    return value->value.string;
}

double json_value_get_number(const JSON_Value *value) {
    if (value == NULL || value->type != JSONNumber) { return 0; }
    return value->value.number;
}

int json_value_get_boolean(const JSON_Value *value) {
    if (value == NULL || value->type != JSONBoolean) { return -1; }
    return value->value.boolean;
}

void json_value_free(JSON_Value *value) {
    switch (value->type) {
        case JSONObject:
            json_object_free(value->value.object);
            break;
        case JSONString:
            if (value->value.string != NULL) { free((void*)value->value.string); }
            break;
        case JSONArray:
            json_array_free(value->value.array);
            break;
        default:
            break;
    }
    free(value);
}
