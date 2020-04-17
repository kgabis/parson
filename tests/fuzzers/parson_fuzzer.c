#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <sys/unistd.h>

#include "parson.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
	
	JSON_Value *root_value;
	JSON_Array *get_array;
	JSON_Object *arr;
	JSON_Value *schema = json_parse_string((char *)data);
	int i;
	size_t count;
	
	char filename[256];
	sprintf(filename, "/tmp/libfuzzer.%d", getpid());
	FILE *fp = fopen(filename, "wb");
	if (!fp)
		return 0;
	fwrite(data, size, 1, fp);
	fclose(fp); 
	
	root_value = json_parse_file(filename);
	json_validate(schema, root_value);
	get_array = json_value_get_array(root_value);
	count = json_array_get_count(get_array);
	
	for (i=0; i<(int)count; i++){
		arr = json_array_get_object(get_array, i);
	}
	
	json_value_free(schema);
	json_value_free(root_value);
	unlink(filename);
	return 0;
}
