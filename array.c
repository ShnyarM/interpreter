#include "types.h"
#include "array.h"

#define ARRAY_STRUCT_SIZE sizeof(array)

array* array_new(int size, data_type type){
    array* new_array = (array*)malloc(ARRAY_STRUCT_SIZE);
    data* new_list = (data*)malloc(sizeof(data)*size);

    new_array->list = new_list;
    new_array->data_type = type;
    new_array->size = size;

    return new_array;
}

// Return true if it was successful
BOOL array_set(array* arr, int index, data data){
    if(index < 0 || index >= (arr->size)){
        return FALSE;
    }

    arr->list[index] = data;
    return TRUE;
}

data_access_result array_get(array* arr, int index){
    if(index < 0 || index >= (arr->size)){
        return INVALID_DATA_ACCESS_RESULT;
    }

    return (data_access_result){TRUE, arr->list[index]};
}

void array_free(array* arr){
    // TO DO: Free stuff inside of array
    free(arr->list);
    free(arr);
}