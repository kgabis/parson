## About
Parson is a lighweight [json](http://json.org) library written in C.

## Features
* Full JSON support
* Lightweight (only 2 files)
* Simple API
* Addressing json values with dot notation (similar to C structs or objects in most OO languages, e.g. "objectA.objectB.value")
* C89 compatible
* Test suites

## Installation
Run:
```
git clone https://github.com/kgabis/parson.git
```
and copy parson.h and parson.c to you source code tree.

Run ```make test``` to compile and run tests.

## Examples
### Parsing JSON
Here is a function, which prints basic commit info (date, sha and author) from a github repository.  
```c
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

```
Calling ```print_commits_info("torvalds", "linux");``` prints:  
```
Date       SHA        Author
2012-10-15 dd8e8c4a2c David Rientjes
2012-10-15 3ce9e53e78 Michal Marek
2012-10-14 29bb4cc5e0 Randy Dunlap
2012-10-15 325adeb55e Ralf Baechle
2012-10-14 68687c842c Russell King
2012-10-14 ddffeb8c4d Linus Torvalds
...
```

### Persistence
In this example I'm using parson to save user information to a file and then load it and validate later.
```c
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
```

### Serialization
Creating JSON values is very simple thanks to the dot notation. 
Object hierarchy is automatically created when addressing specific fields. 
In the following example I create a simple JSON value containing basic information about a person.
```c
void serialization_example(void) {
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    char *serialized_string = NULL;
    json_object_set_string(root_object, "name", "John Smith");
    json_object_set_number(root_object, "age", 25);
    json_object_dotset_string(root_object, "address.city", "Cupertino");
    json_object_dotset_value(root_object, "contact.emails", json_parse_string("[\"email@example.com\",\"email2@example.com\"]"));
    serialized_string = json_serialize_to_string_pretty(root_value);
    puts(serialized_string);
    json_free_serialized_string(serialized_string);
    json_value_free(root_value);
}

```

Output:
```
{
    "name": "John Smith",
    "age": 25,
    "address": {
        "city": "Cupertino"
    },
    "contact": {
        "emails": [
            "email@example.com",
            "email2@example.com"
        ]
    }
}
```

## Contributing

I will always merge *working* bug fixes. However, if you want to add something new to the API, please create an "issue" on github for this first so we can discuss if it should end up in the library before you start implementing it.
Remember to follow parson's code style and write appropriate tests.

## My other projects
* [kgflags](https://github.com/kgabis/kgflags) - easy to use command-line flag parsing library   
* [agnes](https://github.com/kgabis/agnes) - header-only NES emulation library

## License
[The MIT License (MIT)](http://opensource.org/licenses/mit-license.php)
