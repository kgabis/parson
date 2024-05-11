#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "parson.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size < 1) {
        return 0;
    }

    // Creating a fake JSON string from input data
    char *json_str = (char*)malloc(size + 1);
    if (json_str == NULL) {
        return 0;
    }
    memcpy(json_str, data, size);
    json_str[size] = '\0';

    // Creating a fake JSON_Value from the JSON string
    JSON_Value *json_val = json_parse_string(json_str);
    if (json_val == NULL) {
        free(json_str);
        return 0;
    }

    // Fake file name
    const char *file_name = "output.json";

    // Calling the target function with the fake JSON_Value and file name
    json_serialize_to_file_pretty(json_val, file_name);

    // Cleanup
    json_value_free(json_val);
    free(json_str);

    return 0;
}
