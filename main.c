#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parson.h"
#define DOUBLE_SERIALIZATION_FORMAT "%f"

typedef struct serializationContext
{
    FILE *file;
    int identation;
}SerContext;

static void json_serialize_string_file(const char *string, SerContext *ctx) {
    size_t i = 0, len = strlen(string);
    char c = '\0';
    fprintf(ctx->file, "\"");
    for (i = 0; i < len; i++) {
        c = string[i];
        switch (c) {
            case '\"': fprintf(ctx->file, "\\\"");   break;
            case '\\': fprintf(ctx->file, "\\\\");   break;
            case '\b': fprintf(ctx->file, "\\b");    break;
            case '\f': fprintf(ctx->file, "\\f");    break;
            case '\n': fprintf(ctx->file, "\\n");    break;
            case '\r': fprintf(ctx->file, "\\r");    break;
            case '\t': fprintf(ctx->file, "\\t");    break;
            default:   fprintf(ctx->file, "%c", c); break;
        }
    }
    fprintf(ctx->file, "\"");
}

void WriteIdentation(SerContext *ctx)
{
    int i;
    for (i = 0; i < ctx->identation; ++i)
    {
        fprintf(ctx->file, "    ");
    }
}

JSON_Status json_serialize_to_file_pretty_r(const JSON_Value *value, SerContext *ctx)
{
    const char *key = NULL, *string = NULL;
    JSON_Value *temp_value = NULL;
    JSON_Array *array = NULL;
    JSON_Object *object = NULL;
    size_t i = 0, count = 0;
    double num = 0.0;
    JSON_Status status = JSONError;

    switch (json_value_get_type(value)) {
        case JSONArray:
            array = json_value_get_array(value);
            count = json_array_get_count(array);
            fprintf(ctx->file, "[\n");
            ctx->identation++;
            for (i = 0; i < count; i++) {
                temp_value = json_array_get_value(array, i);
                WriteIdentation(ctx);
                status = json_serialize_to_file_pretty_r(temp_value, ctx);
                if (status != JSONSuccess)
                {
                    return status;
                }
                if (i < (count - 1))
                    fprintf(ctx->file, ",\n");
                else
                    fprintf(ctx->file, "\n");
            }
            WriteIdentation(ctx);
            fprintf(ctx->file,  "]");
            ctx->identation--;
            return JSONSuccess;
        case JSONObject:
            object = json_value_get_object(value);
            count  = json_object_get_count(object);
            fprintf(ctx->file, "{\n");
            ctx->identation++;
            for (i = 0; i < count; i++) {
                key = json_object_get_name(object, i);
                WriteIdentation(ctx);
                json_serialize_string_file(key, ctx);
                fprintf(ctx->file, " : ");
                temp_value = json_object_get_value(object, key);
                json_serialize_to_file_pretty_r(temp_value, ctx);
                if (i < (count - 1))
                    fprintf(ctx->file, ",\n");
            }
            ctx->identation--;
            fprintf(ctx->file, "\n");
            WriteIdentation(ctx);
            fprintf(ctx->file, "}");

            return JSONSuccess;
        case JSONString:
            string = json_value_get_string(value);
            json_serialize_string_file(string, ctx);
            return JSONSuccess;
        case JSONBoolean:
            if (json_value_get_boolean(value)) {
                fprintf(ctx->file, "true");
            } else {
                fprintf(ctx->file, "false");
            }
            return JSONSuccess;
        case JSONNumber:
            num = json_value_get_number(value);
            if (num == ((double)(int)num)) { /*  check if num is integer */
                fprintf(ctx->file, "%d", (int)num);
            } else {
                fprintf(ctx->file, DOUBLE_SERIALIZATION_FORMAT, num);
            }
            return JSONSuccess;
        case JSONNull:
            fprintf(ctx->file, "null");
            return JSONSuccess;
        case JSONError:
            return JSONError;
        default:
            return JSONError;
    }
}


JSON_Status json_serialize_to_file_pretty(const JSON_Value *value, const char *filename) {
    JSON_Status return_code = JSONSuccess;
    SerContext ctx;

    ctx.identation = 0;
    ctx.file = fopen (filename, "w");
    if (ctx.file != NULL)
    {
        json_serialize_to_file_pretty_r(value, &ctx);

        if (fclose (ctx.file) == EOF) {
            return_code = JSONFailure;
        }
    }
    return return_code;
}

int main()
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

    json_serialize_to_file_pretty(rootValue, "out.json");


    json_value_free(rootValue);
    return 0;
}
