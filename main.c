
#include <stdlib.h>
#include <string.h>

#include "parson.h"
#define DOUBLE_SERIALIZATION_FORMAT "%f"

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
