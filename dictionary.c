#include "helper.h"
#include "dictionary.h"

#define INIT_SIZE 10
#define DICT_ENTRY_SIZE sizeof(dict_entry)

dictionary new_dictionary(){
    //Create new dictionary with size INIT_SIZE
    dict_entry* list = (dict_entry*)malloc(DICT_ENTRY_SIZE*INIT_SIZE);
    dictionary dict;
    dict.list = list;
    dict.length = 0;
    dict.capacity = INIT_SIZE;
    return dict;
}


void add_to_dictionary(dictionary* dict, char* name, int value);

dict_result dict_get(dictionary dict, char* name);