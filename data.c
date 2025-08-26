#include "types.h"
#include "data.h"
#include <stdio.h>

data_update_result data_update_value(data* orig, data new_value){
    if(!IS_SAME_TYPE((*orig), new_value)) return (data_update_result){FALSE, DifferentType};
    // Check if new_value or old value is array, to properly add and remove references
    if(new_value.type.is_array) data_array_add_reference(new_value.value.arr);
    if(orig->type.is_array) data_array_remove_reference(orig->value.arr);

    *orig = new_value;

    return (data_update_result){TRUE, NoFailure};
}

data_array* data_array_new(int size, data_type type){
    if(size <= 0) return NULL;

    data* ptr = (data*)malloc(sizeof(data)*size);
    
    for(int i = 0; i < size; i++){
        ptr[i].type = type;
        ptr[i].value.number = 0;
    }

    data_array* arr = (data_array*)malloc(sizeof(data_array));
    arr->size = size;
    arr->references = 1;
    arr->list = ptr;
    arr->type = type;
    return arr;
}

// Return true if it was successful
data_update_result data_array_set(data_array* arr, int index, data data){
    if(index < 0) return (data_update_result){FALSE, NotInIndexRange};

    data_update_result result = data_update_value(&(arr->list[index]), data);
    return result;
}

data_access_result data_array_get(data_array* arr, int index){
    if(index < 0){
        return INVALID_DATA_ACCESS_RESULT;
    }

    return (data_access_result){TRUE, arr->list[index]};
}

void data_array_add_reference(data_array* arr){
    arr->references++;
}

void data_array_remove_reference(data_array* arr){
    arr->references--;
    if(arr->references <= 0){
        data_array_free(arr);
    }
}

void data_free(data data){
    if(data.type.is_array){
        data_array_remove_reference(data.value.arr);
    }

    if(data.type.atomic_type == Function){
        // TO DO: add free_function function
    }
}

void data_array_free(data_array* arr){
    for(int i = 0; i < arr->size; i++){
        data_free(arr->list[i]);
    } 

    free(arr->list);
    free(arr);
}