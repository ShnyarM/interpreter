#include "data.h"
#include "function.h"
#include "strings.h"
#include <stdlib.h>
#include <stdio.h>

#define ARRAY_SIZE_INCREASE_PADDING 10 //When increasing the size of an array to a specific value, always add a padding of the specified value

void data_array_add_reference(data_array* arr);
void data_array_remove_reference(data_array* arr);

void data_add_reference(data* d){
    if(d == NULL) return;
    if(data_is_of_kind_array(*d)) data_array_add_reference(data_get_array_from_data(*d));
    if(data_is_string_type(*d)) string_add_reference(data_get_string_from_data(*d));
}

void data_remove_reference(data* d){
    if(d == NULL) return;
    if(data_is_of_kind_array(*d)) data_array_remove_reference(data_get_array_from_data(*d));
    if(data_is_string_type(*d)) string_remove_reference(data_get_string_from_data(*d));
}

// assignable is the data type which can be assigned to lhs
data_update_result data_assign_to(data* lhs, data rhs, data_type* assignable_type){
    if(!types_can_be_assigned_to(assignable_type, rhs.type)) return (data_update_result){FALSE, DifferentType};

    data_add_reference(&rhs);
    free_data(*lhs);

    lhs->value = rhs.value;
    lhs->type = types_copy(rhs.type);

    return (data_update_result){TRUE, NoFailure};
}

BOOL data_is_empty(data d){
    return types_equality(d.type, &GLOBAL_INVALID_TYPE) && d.value.number == 0;
}

BOOL data_is_of_kind_array(data d){
    if(d.type == NULL) return FALSE;
    return types_is_array(d.type);
}

BOOL data_is_number_type(data d){
    if(d.type == NULL) return FALSE;
    return types_is_number(d.type);
}

BOOL data_is_string_type(data d){
    if(d.type == NULL) return FALSE;
    return types_is_string(d.type);
}

BOOL data_is_bool_type(data d){
    if(d.type == NULL) return FALSE;
    return types_is_bool(d.type);
}

data_array* data_get_array_from_data(data d){
    return d.value.arr;
}

string* data_get_string_from_data(data d){
    return d.value.str;
}

function* data_get_function_from_data(data d){
    return d.value.function;
}

BOOL data_to_bool(data d){
    if(data_is_bool_type(d)) return GET_BOOL_FROM_DATA(d);
    if(data_is_number_type(d)) return (BOOL)GET_NUMBER_FROM_DATA(d);
    if(data_is_string_type(d)) return !string_is_empty(data_get_string_from_data(d));
    if(data_is_of_kind_array(d)) return !data_array_is_empty(data_get_array_from_data(d));
    if(data_is_empty(d)) return FALSE;

    return FALSE;
}

// type gets copied
data data_default(data_type* type){
    data d;
    d.type = &GLOBAL_INVALID_TYPE;
    d.value = (data_value){.arr=NULL};
    // Any and Invalid types will get assigned these
    
    if(types_is_number(type)){
        d.type = &GLOBAL_NUMBER_TYPE;
        d.value = (data_value){.number=0};
    }
    else if(types_is_array(type)){
        data_array* arr = data_array_new(type->inner_type);
        d.type = types_copy(type);
        d.value = (data_value){.arr=arr};
    }
    else if(types_is_string(type)){
        d.type = &GLOBAL_STRING_TYPE;
        d.value = (data_value){.str=&GLOBAL_EMPTY_STRING};
    }else if(types_is_bool(type)){
        d.type = &GLOBAL_BOOL_TYPE;
        d.value = (data_value){.bool=FALSE};
    }
    
    return d;
}

BOOL data_equality(data d1, data d2){
    if(!types_equality(d1.type, d2.type)) return FALSE;
    if(data_is_number_type(d1)) return GET_NUMBER_FROM_DATA(d1) == GET_NUMBER_FROM_DATA(d2);
    if(data_is_string_type(d1)) return string_equality(data_get_string_from_data(d1), data_get_string_from_data(d2));
    if(data_is_of_kind_array(d1)) return data_array_equality(data_get_array_from_data(d1), data_get_array_from_data(d2));
    if(data_is_bool_type(d1)) return GET_BOOL_FROM_DATA(d1) == GET_BOOL_FROM_DATA(d2);
    if(data_is_empty(d1) && data_is_empty(d2)) return TRUE;
    return FALSE;
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
    arr->type = types_copy(type);
    return arr;
}

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
        new_list[i] = data_default(arr->type);
    }

    free(arr->list);
    arr->list = new_list;
    arr->capacity = actual_new_capacity;
    arr->initialized = TRUE;
}

//TO DO: add index checks
data_ref_access_result data_array_get_reference(data_array* arr, int index){
    data_ref_access_result res;
    res.valid = FALSE;
    res.ref = NULL;
    res.storable = NULL;


    if(index < 0){
        return res;
    }

    if(!arr->initialized || index >= arr->capacity){
        data_array_increase_size(arr, index);
    }

    if(index+1 > arr->size) arr->size = index+1;
    res.valid = TRUE;
    res.ref = &arr->list[index];
    res.storable = arr->type;
    return res;
}

// type gets copied
// all data which does not align with new type gets reverted to default value
void data_array_set_new_type(data_array* arr, data_type* new_type){
    free_type(arr->type);
    arr->type = types_copy(new_type);

    for(int i = 0; i < arr->capacity; i++){
        if(!types_can_be_assigned_to(new_type, arr->list[i].type)){
            free_data(arr->list[i]);
            arr->list[i] = data_default(new_type);
        }
    }
}

BOOL data_array_equality(data_array* arr1, data_array* arr2){
    if(arr1->size != arr2->size) return FALSE;
    for(int i = 0; i < arr1->size; i++){
        data_ref_access_result ref1 = data_array_get_reference(arr1, i);
        data_ref_access_result ref2 = data_array_get_reference(arr2, i);
        if(!data_equality(*ref1.ref, *ref2.ref)) return FALSE;
    }

    return TRUE;
}

BOOL data_array_is_empty(data_array* arr){
    return arr->size == 0;
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
    data_remove_reference(&data);

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