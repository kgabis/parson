/*
 Parson ( http://kgabis.github.com/parson/ )
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

#include "parson.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define STARTING_CAPACITY       10
#define MAX_CAPACITY         10000
#define MAX_NESTING             19
#define sizeof_token(a)      (sizeof(a) - 1)

#define parson_malloc(a)     malloc(a)
#define parson_free(a)       free((void*)a)
#define parson_realloc(a, b) realloc(a, b)

/* Type definitions */
typedef union json_value_value {
    const char  *string;
    double       number;
    JSON_Object *object;
    JSON_Array  *array;
    int          boolean;
    int          null;
} JSON_Value_Value;

struct json_value_t {
    JSON_Value_Type     type;
    JSON_Value_Value    value;
};

struct json_object_t {
    const char **names;
    JSON_Value **values;
    size_t       count;
    size_t       capacity;
};

struct json_array_t {
    JSON_Value **items;
    size_t       count;
    size_t       capacity;
};

/* JSON Object */
static JSON_Object * json_object_init(void);
static int           json_object_add(JSON_Object *object, const char *name, JSON_Value *value);
static void          json_object_free(JSON_Object *object);

/* JSON Array */
static JSON_Array * json_array_init(void);
static int          json_array_add(JSON_Array *array, JSON_Value *value);
static void         json_array_free(JSON_Array *array);

/* JSON Value */
static JSON_Value * json_value_init_object(void);
static JSON_Value * json_value_init_array(void);
static JSON_Value * json_value_init_string(const char *string);
static JSON_Value * json_value_init_number(double number);
static JSON_Value * json_value_init_boolean(int boolean);
static JSON_Value * json_value_init_null(void);

/* Parser */
static char * parson_strndup(const char *string, size_t n);
static char * parson_strdup(const char *string);
static const char * skip_string(const char *string);
static char * copy_and_remove_whitespaces(const char *string);
static int is_utf(const char *string);
static int is_decimal(const char *string, size_t length);
static const char * parse_escaped_characters(const char *string);
static const char * get_string(const char **string);
static JSON_Value * parse_object_value(const char **string, size_t nesting);
static JSON_Value * parse_array_value(const char **string, size_t nesting);
static JSON_Value * parse_string_value(const char **string);
static JSON_Value * parse_boolean_value(const char **string);
static JSON_Value * parse_number_value(const char **string);
static JSON_Value * parse_null_value(const char **string);
static JSON_Value * parse_value(const char **string, size_t nesting);

/* JSON Object */
static JSON_Object * json_object_init(void) {
    JSON_Object *new_obj = (JSON_Object*)parson_malloc(sizeof(JSON_Object));
    if (!new_obj) { return NULL; }
    new_obj->names = (const char**)parson_malloc(sizeof(char*) * STARTING_CAPACITY);
    if (!new_obj->names) { parson_free(new_obj); return NULL; }
    new_obj->values = (JSON_Value**)parson_malloc(sizeof(JSON_Value*) * STARTING_CAPACITY);
    if (!new_obj->values) { parson_free(new_obj->names); parson_free(new_obj); return NULL; }
    new_obj->capacity = STARTING_CAPACITY;
    new_obj->count = 0;
    return new_obj;
}

static int json_object_add(JSON_Object *object, const char *name, JSON_Value *value) {
    size_t index;
    void *reallocated_ptr;
    if (object->count >= object->capacity) {
        size_t new_capacity = object->capacity * 2;
        if (new_capacity > MAX_CAPACITY) { return 0; }
        reallocated_ptr = parson_realloc((void*)object->names, new_capacity * sizeof(char*));
        if (!reallocated_ptr) { return 0;}
        object->names = (const char**)reallocated_ptr;
        reallocated_ptr = parson_realloc(object->values, new_capacity * sizeof(JSON_Value*));
        if (!reallocated_ptr) { return 0;}
        object->values = (JSON_Value**)reallocated_ptr;
        object->capacity = new_capacity;
    }
    if (json_object_get_value(object, name) != NULL) { return 0; }
    index = object->count;
    object->names[index] = parson_strdup(name);
    if (!object->names[index]) { return 0; }
    object->values[index] = value;
    object->count++;
    return 1;
}

static void json_object_free(JSON_Object *object) {
    size_t i;
    for (i = 0; i < object->count; i++) {
        parson_free(object->names[i]);
        json_value_free(object->values[i]);
    }
    parson_free(object->names);
    parson_free(object->values);
    parson_free(object);
}

/* JSON Array */
static JSON_Array * json_array_init(void) {
    JSON_Array *new_array = (JSON_Array*)parson_malloc(sizeof(JSON_Array));
    if (!new_array) { return NULL; }
    new_array->items = (JSON_Value**)parson_malloc(STARTING_CAPACITY * sizeof(JSON_Value*));
    if (!new_array->items) { parson_free(new_array); return NULL; }
    new_array->capacity = STARTING_CAPACITY;
    new_array->count = 0;
    return new_array;
}

static int json_array_add(JSON_Array *array, JSON_Value *value) {
    void *reallocated_ptr;
    if (array->count >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        if (new_capacity > MAX_CAPACITY) { return 0; }
        reallocated_ptr = parson_realloc(array->items, new_capacity * sizeof(JSON_Value*));
        if (!reallocated_ptr) { return 0; }
        array->items = (JSON_Value**)reallocated_ptr;
        array->capacity = new_capacity;
    }
    array->items[array->count] = value;
    array->count++;
    return 1;
}

static void json_array_free(JSON_Array *array) {
    size_t i;
    for (i = 0; i < array->count; i++) { json_value_free(array->items[i]); }
    parson_free(array->items);
    parson_free(array);
}

/* JSON Value */
static JSON_Value * json_value_init_object(void) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONObject;
    new_value->value.object = json_object_init();
    if (!new_value->value.object) { parson_free(new_value); return NULL; }
    return new_value;
}

static JSON_Value * json_value_init_array(void) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONArray;
    new_value->value.array = json_array_init();
    if (!new_value->value.array) { parson_free(new_value); return NULL; }
    return new_value;
}

static JSON_Value * json_value_init_string(const char *string) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONString;
    new_value->value.string = string;
    return new_value;
}

static JSON_Value * json_value_init_number(double number) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONNumber;
    new_value->value.number = number;
    return new_value;
}

static JSON_Value * json_value_init_boolean(int boolean) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONBoolean;
    new_value->value.boolean = boolean;
    return new_value;
}

static JSON_Value * json_value_init_null(void) {
    JSON_Value *new_value = (JSON_Value*)parson_malloc(sizeof(JSON_Value));
    if (!new_value) { return NULL; }
    new_value->type = JSONNull;
    return new_value;
}

/* Parser */
static char * parson_strndup(const char *string, size_t n) {
    char *output_string = (char*)parson_malloc(n + 1);
    if (!output_string) { return NULL; }
    output_string[n] = '\0';
    strncpy(output_string, string, n);
    return output_string;
}

static char * parson_strdup(const char *string) {
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

static char * copy_and_remove_whitespaces(const char *string) {
    char *output_string = (char*)parson_malloc(strlen(string) + 1);
    char *output_string_ptr = output_string;
    const char *string_ptr = string;
    const char *skipped_string = NULL;
    void *reallocated_ptr;
    char current_char;
    if (!output_string) { return NULL; }
    while (*string_ptr) {
        current_char = *string_ptr;
        switch (current_char) {
            case ' ': case '\r': case '\n': case '\t':
                string_ptr++;
                break;
            case '\"':
                skipped_string = skip_string(string_ptr);
                if (!skipped_string) { parson_free(output_string); return NULL; }
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
    reallocated_ptr = parson_realloc(output_string, strlen(output_string) + 1);
    if (!reallocated_ptr) { parson_free(output_string); return NULL; }
    output_string = (char*)reallocated_ptr;
    return output_string;
}

static int is_utf(const char *string) {
    if (!isxdigit(string[0])) { return 0; }
    if (!isxdigit(string[1])) { return 0; }
    if (!isxdigit(string[2])) { return 0; }
    if (!isxdigit(string[3])) { return 0; }
    return 1;
}

static int is_decimal(const char *string, size_t length) {
    if (strchr(string, 'x') || strchr(string, 'X')) { return 0; }
    if (length > 1 && string[0] == '0' && string[1] != '.') { return 0; }
    if (length > 2 && !strncmp(string, "-0", 2) && string[2] != '.') { return 0; }
    return 1;
}

static const char * parse_escaped_characters(const char *string) {
    char *output_string = (char*)parson_malloc(strlen(string) + 1);
    char *output_string_ptr = output_string;
    const char *string_ptr = string;
    char current_char;
    unsigned int utf_val;
    void *reallocated_ptr;
    if (!output_string) { return NULL; }
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
                    if (!is_utf(string_ptr) ||
                            sscanf(string_ptr, "%4x", &utf_val) == EOF) {
                        parson_free(output_string); return NULL;
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
                    parson_free(output_string);
                    return NULL;
                    break;
            }
        } else if (iscntrl(current_char)) { /* no control characters allowed */
            parson_free(output_string);
            return NULL;
        }
        *output_string_ptr = current_char;
        output_string_ptr++;
        string_ptr++;
    }
    *output_string_ptr = '\0';
    reallocated_ptr = parson_realloc(output_string, strlen(output_string) + 1);
    if (!reallocated_ptr) { parson_free(output_string); return NULL; }
    output_string = (char*)reallocated_ptr;
    return output_string;
}

/* Returns contents of a string inside double quotes and parses escaped
 characters inside.
 Example: "\u006Corem ipsum" -> lorem ipsum */
static const char * get_string(const char **string) {
    char *quote_contents;
    const char *parsed_string;
    const char *after_closing_quote_ptr = skip_string(*string);
    if (!after_closing_quote_ptr) { return NULL; }
    quote_contents = parson_strndup(*string + 1, after_closing_quote_ptr - *string - 2);
    if (!quote_contents) { return NULL; }
    *string = after_closing_quote_ptr;
    parsed_string = parse_escaped_characters(quote_contents);
    parson_free(quote_contents);
    return (const char*)parsed_string;
}

static JSON_Value * parse_value(const char **string, size_t nesting) {
    JSON_Value *output_value = NULL;
    if (*string == NULL || nesting > MAX_NESTING) { return NULL; }
    switch ((*string)[0]) {
        case '{':
            output_value = parse_object_value(string, nesting + 1);
            break;
        case '[':
            output_value = parse_array_value(string, nesting + 1);
            break;
        case '\"':
            output_value = parse_string_value(string);
            break;
        case 'f': case 't':
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

static JSON_Value * parse_object_value(const char **string, size_t nesting) {
    JSON_Value *output_value = json_value_init_object();
    const char *new_key = NULL;
    JSON_Value *new_value = NULL;
    if (!output_value) { return NULL; }
    (*string)++;
    if (**string == '}') { (*string)++; return output_value; } /* empty object */
    while (**string != '\0') {
        new_key = get_string(string);
        if (!new_key || **string != ':') {
            json_value_free(output_value);
            return NULL;
        }
        (*string)++;
        new_value = parse_value(string, nesting);
        if (!new_value) {
            parson_free(new_key);
            json_value_free(output_value);
            return NULL;
        }
        if(!json_object_add(json_value_get_object(output_value), new_key, new_value)) {
            parson_free(new_key);
            parson_free(new_value);
            json_value_free(output_value);
            return NULL;
        }
        parson_free(new_key);
        if (**string != ',') { break; }
        (*string)++;
    }
    if (**string != '}') { json_value_free(output_value); return NULL; }
    (*string)++;
    return output_value;
}

static JSON_Value * parse_array_value(const char **string, size_t nesting) {
    JSON_Value *output_value = json_value_init_array();
    JSON_Value *new_array_value = NULL;
    if (!output_value) { return NULL; }
    (*string)++;
    if (**string == ']') { /* empty array */
        (*string)++;
        return output_value;
    }
    while (**string != '\0') {
        new_array_value = parse_value(string, nesting);
        if (!new_array_value) {
            json_value_free(output_value);
            return NULL;
        }
        if(!json_array_add(json_value_get_array(output_value), new_array_value)) {
            parson_free(new_array_value);
            json_value_free(output_value);
            return NULL;
        }
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
    if (!new_string) { return NULL; }
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
    const char *number_string;
    char *end;
    double number = strtod(*string, &end);
    JSON_Value *output_value;
    number_string = parson_strndup(*string, end - *string);
    if (!number_string) { return NULL; }
    if (is_decimal(number_string, end - *string)) {
        *string = end;
        output_value = json_value_init_number(number);
    } else {
        output_value = NULL;
    }
    free((void*)number_string);
    return output_value;
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
    if (!fp) { return NULL; }
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    rewind(fp);
    file_contents = (char*)parson_malloc(sizeof(char) * (file_size + 1));
    if (!file_contents) { fclose(fp); return NULL; }
    fread(file_contents, file_size, 1, fp);
    fclose(fp);
    file_contents[file_size] = '\0';
    output_value = json_parse_string(file_contents);
    parson_free(file_contents);
    return output_value;
}

JSON_Value * json_parse_string(const char *string) {
    JSON_Value *output_value = NULL;
    const char *json_string = string ? copy_and_remove_whitespaces(string) : NULL;
    const char *json_string_ptr = json_string;
    if (!json_string) { return NULL; }
    if (*json_string == '{' || *json_string == '[') {
        output_value = parse_value((const char**)&json_string_ptr, 0);
    }
    parson_free(json_string);
    return output_value;
}

/* JSON Object API */
JSON_Value * json_object_get_value(const JSON_Object *object, const char *name) {
    size_t i;
    if (!object) { return NULL; }
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
    if (!dot_position) { return json_object_get_value(object, name); }
    object_name = parson_strndup(name, dot_position - name);
    if (!object_name) { return NULL; }
    output_value = json_object_dotget_value(json_object_get_object(object, object_name),
                                            dot_position + 1);
    parson_free(object_name);
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
    return array ? array->count : 0;
}

/* JSON Value API */
JSON_Value_Type json_value_get_type(const JSON_Value *value) {
    return value ? value->type : JSONError;
}

JSON_Object * json_value_get_object(const JSON_Value *value) {
    return json_value_get_type(value) == JSONObject ? value->value.object : NULL;
}

JSON_Array * json_value_get_array(const JSON_Value *value) {
    return json_value_get_type(value) == JSONArray ? value->value.array : NULL;
}

const char * json_value_get_string(const JSON_Value *value) {
    return json_value_get_type(value) == JSONString ? value->value.string : NULL;
}

double json_value_get_number(const JSON_Value *value) {
    return json_value_get_type(value) == JSONNumber ? value->value.number : 0;
}

int json_value_get_boolean(const JSON_Value *value) {
    return json_value_get_type(value) == JSONBoolean ? value->value.boolean : -1;
}

void json_value_free(JSON_Value *value) {
    switch (json_value_get_type(value)) {
        case JSONObject:
            json_object_free(value->value.object);
            break;
        case JSONString:
            if (value->value.string) { parson_free(value->value.string); }
            break;
        case JSONArray:
            json_array_free(value->value.array);
            break;
        default:
            break;
    }
    parson_free(value);
}
