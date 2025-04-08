#include <string.h>
#include <stdlib.h>
#include "helper.h"
#include "dictionary.h"

#define INIT_SIZE 1
#define DICT_ENTRY_SIZE sizeof(dict_entry)

void dict_increase_capacity(dictionary* dict);

dictionary dict_new(){
    //Create new dictionary with size INIT_SIZE
    dict_entry* list = (dict_entry*)malloc(DICT_ENTRY_SIZE*INIT_SIZE);
    dictionary dict;
    dict.list = list;
    dict.length = 0;
    dict.capacity = INIT_SIZE;
    return dict;
}

BOOL dict_valid(dictionary* dict){
    if(dict->list == NULL) return FALSE;
    return TRUE;
}

// Return TRUE if it was able to update it
BOOL dict_update(dictionary* dict, char* name, int value){
    if(!dict_valid(dict)) return FALSE;

    for(int i = 0; i < dict->length; i++){
        dict_entry* entry = dict->list+i;
        if(strcmp(name, entry->name) == 0){
            entry->value = value;
            return TRUE;
        }
    }

    return FALSE;
}

void dict_add(dictionary* dict, char* name, int value){
    if(!dict_valid(dict)) return;

    if(dict->length == dict->capacity){
        dict_increase_capacity(dict);
    }

    // If name already in list, just update
    if(dict_update(dict, name, value)) return;

    dict_entry* entry = dict->list+(dict->length);
    entry->name = copy_of_string(name);
    entry->value = value;
    dict->length++;
}

dict_result dict_get(dictionary* dict, char* name){
    for(int i = 0; i < dict->length; i++){
        dict_entry entry = dict->list[i];

        if(strcmp(name, entry.name) == 0){
            return (dict_result){TRUE, entry.value};
        }
    }

    return (dict_result){FALSE, 0};
}

void dict_increase_capacity(dictionary* dict){
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

}