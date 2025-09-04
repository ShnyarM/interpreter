#include "data.h"
#include <stdlib.h>

data_update_result data_update_value(data* orig, data new_value){
    if(!type_equality(orig->type, new_value.type)) return (data_update_result){FALSE, DifferentType};
    // Check if new_value or old value is array, to properly add and remove references
    if(is_data_of_kind_array(new_value)) data_array_add_reference(new_value.value.arr);
    if(is_data_of_kind_array(*orig)) data_array_remove_reference(orig->value.arr);

    *orig = new_value;

    return (data_update_result){TRUE, NoFailure};
}

BOOL is_data_of_kind_array(data d){
    return is_type_array(*d.type);
}

BOOL is_data_number_type(data d){
    return is_type_number(*d.type);
}

data_array* get_array_from_data(data d){
    return d.value.arr;
}

// Ownership of type gets transferred
data_array* data_array_new(int size, data_type* type){
    if(size <= 0) return NULL;

    data* ptr = (data*)malloc(sizeof(data)*size);
    
    for(int i = 0; i < size; i++){
        ptr[i].type = copy_of_type(type);
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

//TO DO: add index checks
data* data_array_get_reference(data_array* arr, int index){
    if(index < 0){
        return NULL;
    }

    return &arr->list[index];
}

int data_array_get_size(data_array* arr){
    return arr->size;
}

void data_array_add_reference(data_array* arr){
    arr->references++;
}

void data_array_remove_reference(data_array* arr){
    arr->references--;
    if(arr->references <= 0){
        free_data_array(arr);
    }
}

void free_data(data data){
    if(data.type->kind == Type_Kind_Array){
        data_array_remove_reference(data.value.arr);
    }

    if(data.type->kind == Function){
        // TO DO: add free_function function
    }

    free_type(data.type);
}

void free_data_array(data_array* arr){
    for(int i = 0; i < arr->size; i++){
        data_free(arr->list[i]);
    } 

    free(arr->list);
    free(arr);
}