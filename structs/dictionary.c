#include <stdlib.h>
#include <string.h>
#include "dictionary.h"

#define INIT_SIZE 1
#define DICT_ENTRY_SIZE sizeof(dict_entry)

static void dict_increase_capacity(dictionary* dict);

dictionary dict_new(){
    //Create new dictionary with size INIT_SIZE
    dict_entry* list = (dict_entry*)malloc(DICT_ENTRY_SIZE*INIT_SIZE);
    dictionary dict;
    dict.list = list;
    dict.length = 0;
    dict.capacity = INIT_SIZE;
    dict.child = NULL;
    return dict;
}

dictionary dict_get_extension(dictionary* child){
    dictionary dict = dict_new();
    dict.child = child;
    return dict;
}

static BOOL dict_valid(dictionary* dict){
    if(dict->list == NULL) return FALSE;
    return TRUE;
}

// returns entry with corresponding name, return NULL if not found
static dict_entry* dict_get_entry(dictionary* dict, char* name){
    for(int i = 0; i < dict->length; i++){
        dict_entry* entry = dict->list+i;
        if(strcmp(name, entry->name) == 0){
            return entry;
        }
    }

    return NULL;
}

// adds plain variable to list if not present, returns ptr to entry if successful and returns NULL if already present
// does not check in child dictionary
static dict_entry* dict_add_to_list(dictionary* dict, char* name){ 

    //Create new dictionary if invalid
    if(!dict_valid(dict)){
        *dict = dict_new();
    }

    // If name already in list, return FALSE
    if(dict_get_entry(dict, name) != NULL) return NULL;

    if(dict->length == dict->capacity){
        dict_increase_capacity(dict);
    }

    dict_entry* entry = dict->list+(dict->length);
    entry->name = copy_of_string(name);

    dict->length++;

    return entry;
}

// adds to list if not present, returns TRUE if successful and returns FALSE if already present
// does not check in child dictionary
BOOL dict_add(dictionary* dict, char* name, data_type* type){ 

    dict_entry* entry = dict_add_to_list(dict, name);
    if(entry == NULL){
        return FALSE;
    }

    entry->data.type = types_copy(type);
    entry->data.value = data_default_value(type);
    return TRUE;
}

// Updates a existing variable
// Return TRUE if it was able to update it
data_update_result dict_update(dictionary* dict, char* name, data data){
    if(!dict_valid(dict)) return (data_update_result){FALSE, NoDict};

    dict_entry* entry = dict_get_entry(dict, name);

    if(entry != NULL){
        data_update_result result = data_update_value(&entry->data, data);
        return result;
    }

    //search in child dictionary
    if(dict->child != NULL){
        data_update_result result = dict_update(dict->child, name, data);
        return result;
    }

    return (data_update_result){FALSE, NotInitialized};
}

data_access_result dict_get(dictionary* dict, char* name){
    data* ref = dict_get_reference(dict, name);

    if(ref == NULL) return INVALID_DATA_ACCESS_RESULT;
    return (data_access_result){TRUE, *ref};
}

data* dict_get_reference(dictionary* dict, char* name){
    if(!dict_valid(dict)){
        return NULL;
    }

    dict_entry* entry = dict_get_entry(dict, name);
    if(entry != NULL) return &entry->data;

    //check in child dictionary
    if(dict->child != NULL){
        data* child_result = dict_get_reference(dict->child, name);
        if(child_result != NULL) return child_result;
    }
    
    return NULL;
}

static void dict_increase_capacity(dictionary* dict){
    if(!dict_valid(dict)) return;

    int new_size = dict->capacity*2;
    dict_entry* old_list = dict->list;
    dict_entry* new_list = (dict_entry*)malloc(DICT_ENTRY_SIZE*new_size);

    for(int i = 0; i < dict->length; i++){
        new_list[i] = old_list[i];
    }

    dict->list = new_list;
    dict->capacity = new_size;
    free(old_list);
}

void free_dict(dictionary* dict){
    //Free each variable
    for(int i = 0; i < dict->length; i++){
        free(dict->list[i].name);
        free_data(dict->list[i].data);
    }

    free(dict->list);
    dict->list = NULL;
    dict->length = 0;
    dict->capacity = 0;
}