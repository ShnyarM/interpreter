#pragma once

#include "helper.h"
#include "types.h"

typedef struct dictionary_entry_t{
    char* name;
    void* value;
    d_type type;

    BOOL array; 
    size_t array_size;
} dict_entry;

typedef struct dictionary_t{
    dict_entry* list;
    int length;           //How many entries are in list
    int capacity;         //How how capacity does list have
    struct dictionary_t* child;     // child dictionary if dictionary is extender
} dictionary;

typedef struct dict_result_r{
    BOOL valid;
    void* result;
    d_type type;
} dict_result;

dictionary dict_new();
dictionary dict_get_extension(dictionary* child);
BOOL dict_update(dictionary* dict, char* name, int value);
BOOL dict_add(dictionary* dict, char* name, int value);
dict_result dict_get(dictionary* dict, char* name);
void dict_free(dictionary* dict);
