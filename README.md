##About
Parson is a lighweight [json](http://json.org) parser and reader written in C.  

##Features
* Full JSON support
* Lightweight (only 2 files)
* Simple API
* Addressing json values with dot notation (similiar to C structs or objects in most OO languages, e.g. "objectA.objectB.value")
* C89 compatible
* Test suites

##Installation
Run the following code:
```
git clone https://github.com/kgabis/parson.git
```
and copy parson.h and parson.c to you source code tree.

Run ```make test``` to compile and run tests.

##Example
Here is a function, which prints basic commit info (date, sha and author) from a github repository.  It's also included in tests.c file, you can just uncomment and run it.
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

##License
[The MIT License (MIT)](http://opensource.org/licenses/mit-license.php)