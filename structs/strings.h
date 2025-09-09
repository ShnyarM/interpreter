#pragma once
#include "../helper.h"

typedef struct string_t{
    int length;
    int references;
    char* c_string;

    BOOL global;        // Signifies whether the string is a global usable string which shouldnt be freed
} string;

string* string_new(char* content);
string* string_convert_from_number(double number);
string* strings_concat(string* str1, string* str2);
void string_add_reference(string* str);
void string_remove_reference(string* str);
void string_print(string* str);
void free_string(string* str);

extern string GLOBAL_EMPTY_STRING;