#include "data.h"
#include "function.h"
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_SIZE_INCREASE_PADDING 10 //When increasing the size of an array to a specific value, always add a padding of the specified value

data_update_result data_update_value(data* orig, data new_value){
    if(!type_equality(orig->type, new_value.type)) return (data_update_result){FALSE, DifferentType};
    // Check if new_value or old value is array, to properly add and remove references
    if(is_data_of_kind_array(new_value)) data_array_add_reference(new_value.value.arr);
    if(is_data_of_kind_array(*orig)) data_array_remove_reference(orig->value.arr);

    orig->value = new_value.value;

    return (data_update_result){TRUE, NoFailure};
}

BOOL is_empty_data(data d){
    return type_equality(d.type, &GLOBAL_INVALID_TYPE) && d.value.number == 0;
}

BOOL is_data_of_kind_array(data d){
    if(d.type == NULL) return FALSE;
    return is_type_array(d.type);
}

BOOL is_data_number_type(data d){
    if(d.type == NULL) return FALSE;
    return is_type_number(d.type);
}

data_array* get_array_from_data(data d){
    return d.value.arr;
}

function* get_function_from_data(data d){
    return d.value.function;
}

// Returns a default value for a given data type
data_value data_default_value(data_type* type){
    if(is_type_number(type)) return (data_value){.number=0};
    if(is_type_array(type)){
        data_array* arr = data_array_new(type);
        return (data_value){.arr=arr};
    }

    return (data_value){.number=0};
}

void data_print(data d){
    if(is_data_of_kind_array(d)){ //If its array, print every value
        data_array_print(get_array_from_data(d));
    }else if(is_empty_data(d)){
        printf("VOID");
    }else{
        printf("%.15g", GET_NUMBER_FROM_DATA(d));
    }
}

// Returns a new data array, which is not initialized yet, i.e., has no list attached to it
// Type gets copied
data_array* data_array_new(data_type* type){
    data_array* arr = (data_array*)malloc(sizeof(data_array));
    arr->initialized = FALSE;
    arr->size = 0;
    arr->capacity = 0;
    arr->references = 1;
    arr->list = NULL;
    arr->type = copy_of_type(type);
    return arr;
}

// Return true if it was successful
/*data_update_result data_array_set(data_array* arr, int index, data data){
    if(index < 0) return (data_update_result){FALSE, NotInIndexRange};

    data_update_result result = data_update_value(&(arr->list[index]), data);
    return result;
}*/

/*data_access_result data_array_get(data_array* arr, int index){
    if(index < 0){
        return INVALID_DATA_ACCESS_RESULT;
    }

    return (data_access_result){TRUE, arr->list[index]};
}*/

void data_array_increase_size(data_array* arr, int new_needed_capacity){
    int possibility1 = arr->size*2;
    int possibility2 = new_needed_capacity+ARRAY_SIZE_INCREASE_PADDING;
    int actual_new_capacity = possibility1 > possibility2 ? possibility1 : possibility2;

    data* new_list = (data*)malloc(sizeof(data)*actual_new_capacity);

    for(int i = 0; i < arr->capacity; i++){
        new_list[i].type = arr->list[i].type;
        new_list[i].value = arr->list[i].value;
    }

    for(int i = arr->capacity; i < actual_new_capacity; i++){
        new_list[i].type = copy_of_type(arr->type->inner_type);
        new_list[i].value = data_default_value(arr->type->inner_type);
    }

    free(arr->list);
    arr->list = new_list;
    arr->capacity = actual_new_capacity;
    arr->initialized = TRUE;
}

//TO DO: add index checks
data* data_array_get_reference(data_array* arr, int index){
    if(index < 0){
        return NULL;
    }

    if(!arr->initialized || index >= arr->capacity){
        data_array_increase_size(arr, index);
    }

    if(index+1 > arr->size) arr->size = index+1;
    return &arr->list[index];
}

void data_array_print(data_array* arr){
    if(!arr->initialized){
        printf("[]");
        return;
    }

    printf("[");

    int size = data_array_get_size(arr);
    for(int i = 0; i < size-1; i++){
        data_print(*data_array_get_reference(arr, i));
        printf(", ");
    }

    data_print(*data_array_get_reference(arr, size-1));
    printf("]");
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

    if(data.type->kind == Type_Kind_Function){
        free_function(data.value.function);
    }

    free_type(data.type);
}

void free_data_array(data_array* arr){
    for(int i = 0; i < arr->capacity; i++){
        free_data(arr->list[i]);
    }

    free_type(arr->type);
    free(arr->list);
    free(arr);
}