#include <string.h>
#include <stdlib.h>
#include "helper.h"
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

// Return TRUE if it was able to update it
BOOL dict_update(dictionary* dict, char* name, int value){
    if(!dict_valid(dict)) return FALSE;

    dict_entry* entry = dict_get_entry(dict, name);
    if(entry != NULL){
        entry->value = value;
        return TRUE;
    }

    //search in child dictionary
    if(dict->child != NULL){
        BOOL result = dict_update(dict->child, name, value);
        return result;
    }

    return FALSE;
}

// adds to list if not present, returns TRUE if successful and returns FALSE if already present
// does not check in child dictionary
BOOL dict_add(dictionary* dict, char* name, int value){ 

    //Create new dictionary if invalid
    if(!dict_valid(dict)){
        *dict = dict_new();
    }

    // If name already in list, return FALSE
    if(dict_get_entry(dict, name) != NULL) return FALSE;

    if(dict->length == dict->capacity){
        dict_increase_capacity(dict);
    }

    dict_entry* entry = dict->list+(dict->length);
    entry->name = copy_of_string(name);
    entry->value = value;
    dict->length++;

    return TRUE;
}

dict_result dict_get(dictionary* dict, char* name){
    if(!dict_valid(dict)){
        return (dict_result){FALSE, 0};
    }

    dict_entry* entry = dict_get_entry(dict, name);
    if(entry != NULL) return (dict_result){TRUE, entry->value};

    //check in child dictionary
    if(dict->child != NULL){
        dict_result child_result = dict_get(dict->child, name);
        if(child_result.valid) return child_result;
    }
    
    return (dict_result){FALSE, 0};
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

void dict_free(dictionary* dict){
    //Free each variable string
    for(int i = 0; i < dict->length; i++){
        free(dict->list[i].name);
    }

    free(dict->list);
    dict->list = NULL;
    dict->length = 0;
    dict->capacity = 0;
}