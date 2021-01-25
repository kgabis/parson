
#ifndef parson_parson_int_h
#define parson_parson_int_h

#ifdef __cplusplus
extern "C"
{
#endif

#include "parson.h"


#include <stdlib.h>

extern int parson_escape_slashes;

extern JSON_Malloc_Function parson_malloc;
extern JSON_Free_Function parson_free;

typedef struct json_string
{
    char *chars;
    size_t length;
} JSON_String;

/* Type definitions */
typedef union json_value_value
{
    JSON_String string;
    double number;
    JSON_Object *object;
    JSON_Array *array;
    int boolean;
    int null;
} JSON_Value_Value;

struct json_value_t
{
    JSON_Value *parent;
    JSON_Value_Type type;
    JSON_Value_Value value;
};

struct json_object_t
{
    JSON_Value *wrapping_value;
    char **names;
    JSON_Value **values;
    size_t count;
    size_t capacity;
};

struct json_array_t
{
    JSON_Value *wrapping_value;
    JSON_Value **items;
    size_t count;
    size_t capacity;
};

/* Parser */
 JSON_Status  skip_quotes(const char **string);
 int          parse_utf16(const char **unprocessed, char **processed);
 char *       process_string(const char *input, size_t input_len, size_t *output_len);
 char *       get_quoted_string(const char **string, size_t *output_string_len);
 JSON_Value * parse_object_value(const char **string, size_t nesting);
 JSON_Value * parse_array_value(const char **string, size_t nesting);
 JSON_Value * parse_string_value(const char **string);
 JSON_Value * parse_boolean_value(const char **string);
 JSON_Value * parse_number_value(const char **string);
 JSON_Value * parse_null_value(const char **string);
 JSON_Value * parse_value(const char **string, size_t nesting);

/* Various */
char *read_file(const char *filename);
void remove_comments(char *string, const char *start_token, const char *end_token);
char *parson_strndup(const char *string, size_t n);
char *parson_strdup(const char *string);
int hex_char_to_int(char c);
int parse_utf16_hex(const char *string, unsigned int *result);
int num_bytes_in_utf8_sequence(unsigned char c);
int verify_utf8_sequence(const unsigned char *string, int *len);
int is_valid_utf8(const char *string, size_t string_len);
int is_decimal(const char *string, size_t length);

/* JSON Object */
JSON_Object *json_object_init(JSON_Value *wrapping_value);
JSON_Status json_object_add(JSON_Object *object, const char *name, JSON_Value *value);
JSON_Status json_object_addn(JSON_Object *object, const char *name, size_t name_len, JSON_Value *value);
JSON_Status json_object_resize(JSON_Object *object, size_t new_capacity);
JSON_Value *json_object_getn_value(const JSON_Object *object, const char *name, size_t name_len);
JSON_Status json_object_remove_internal(JSON_Object *object, const char *name, int free_value);
JSON_Status json_object_dotremove_internal(JSON_Object *object, const char *name, int free_value);
void json_object_free(JSON_Object *object);
/* Serialization */
 int    json_serialize_to_buffer_r(const JSON_Value *value, char *buf, int level, int is_pretty, char *num_buf);
 int    json_serialize_string(const char *string, size_t len, char *buf);
 int    append_indent(char *buf, int level);
 int    append_string(char *buf, const char *string);
/* JSON Array */
JSON_Array *json_array_init(JSON_Value *wrapping_value);
JSON_Status json_array_add(JSON_Array *array, JSON_Value *value);
JSON_Status json_array_resize(JSON_Array *array, size_t new_capacity);
void json_array_free(JSON_Array *array);

/* JSON Value */
JSON_Value *json_value_init_string_no_copy(char *string, size_t length);
const JSON_String *json_value_get_string_desc(const JSON_Value *value);
#ifdef __cplusplus
}
#endif
#endif