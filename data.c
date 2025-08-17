#include "types.h"
#include "data.h"

#define ARRAY_STRUCT_SIZE sizeof(array)

data* data_array_new(int size){
    if(size <= 0) return NULL;

    data* ptr = (data*)malloc(sizeof(data)*size);

    return ptr;
}

// Return true if it was successful
BOOL data_array_set(data* arr, int index, data data){
    if(index < 0){
        return FALSE;
    }

    arr[index] = data;
    return TRUE;
}

data_access_result data_array_get(data* arr, int index){
    if(index < 0){
        return INVALID_DATA_ACCESS_RESULT;
    }

    return (data_access_result){TRUE, arr[index]};
}

void data_array_free(data* arr){
    // TO DO: Free stuff inside of array
    free(arr);
}