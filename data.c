#include "types.h"
#include "data.h"

#define ARRAY_STRUCT_SIZE sizeof(array)

data* data_array_new(int size, data_type type){
    if(size <= 0) return NULL;

    data* ptr = (data*)malloc(sizeof(data)*size);
    
    for(int i = 0; i < size; i++){
        ptr[i].type = type;
        ptr[i].value.number = 0;
    }

    return ptr;
}

// Return true if it was successful
data_update_result data_array_set(data* arr, int index, data data){
    if(index < 0) return (data_update_result){FALSE, NotInIndexRange};
    if(!IS_SAME_TYPE(data, arr[index])) return (data_update_result){FALSE, DifferentType};

    arr[index] = data;
    return (data_update_result){TRUE, NoFailure};
}

data_access_result data_array_get(data* arr, int index){
    if(index < 0){
        return INVALID_DATA_ACCESS_RESULT;
    }

    return (data_access_result){TRUE, arr[index]};
}

void data_free(data data){
    if(data.type.is_array){
        data_array_free(data.value.arr, data.size);
    }
}

void data_array_free(data* arr, int size){
    for(int i = 0; i < size; i++){
        data_free(arr[i]);
    }

    free(arr);
}