#pragma once
#include "../helper.h"
#include "data.h"

struct data_t;

typedef struct string_t{
    int length;
    int references;
    char* c_string;

    BOOL global;        // Signifies whether the string is a global usable string which shouldnt be freed
} string;

string* string_new(char* content);
string* string_from_data(struct data_t d, BOOL encapsuled);
string* strings_concat(string* str1, string* str2);
BOOL string_equality(string* s1, string* s2);
BOOL string_is_empty(string* str);
void string_add_reference(string* str);
void string_remove_reference(string* str);
void free_string(string* str);

extern string GLOBAL_EMPTY_STRING;