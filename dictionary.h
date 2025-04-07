#pragma once

#include "helper.h"

typedef struct dictionary_entry_t{
    char* name;
    int value;
} dict_entry;

typedef struct dictionary_t{
    dict_entry* list;
    int length;           //How many entries are in list
    int capacity;         //How how capacity does list have
} dictionary;

typedef struct dict_result_r{
    BOOL valid;
    int result;
} dict_result;

dictionary new_dictionary();
void add_to_dictionary(dictionary* dict, char* name, int value);
dict_result dict_get(dictionary dict, char* name);
