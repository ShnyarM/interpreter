#pragma once

#include "data.h"
#include "variable_access.h"

typedef struct dictionary_entry_t{
    char* name;
    data data;
    data_type* storable;    // What kind of data_type this entry can store
} dict_entry;

typedef struct dictionary_t{
    dict_entry* list;
    int length;           //How many entries are in list
    int capacity;         //How how capacity does list have
    struct dictionary_t* child;     // child dictionary if dictionary is extender
} dictionary;

dictionary dict_new();
dictionary dict_get_extension(dictionary* child);
data_ref_access_result dict_add(dictionary* dict, char* name, data_type* type);
data_access_result dict_get(dictionary* dict, char* name);
data_ref_access_result dict_get_reference(dictionary* dict, char* name);
data_ref_access_result dict_add_built_in_functions(dictionary* dict);
void free_dict(dictionary* dict); 