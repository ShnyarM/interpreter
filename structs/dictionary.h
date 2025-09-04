#pragma once

#include "data.h"

typedef struct dictionary_entry_t{
    char* name;
    data data;
} dict_entry;

typedef struct dictionary_t{
    dict_entry* list;
    int length;           //How many entries are in list
    int capacity;         //How how capacity does list have
    struct dictionary_t* child;     // child dictionary if dictionary is extender
} dictionary;

dictionary dict_new();
dictionary dict_get_extension(dictionary* child);
data_update_result dict_update(dictionary* dict, char* name, data data);
BOOL dict_add(dictionary* dict, char* name, data_type type);
data_access_result dict_get(dictionary* dict, char* name);
data* dict_get_reference(dictionary* dict, char* name);
void dict_free(dictionary* dict);