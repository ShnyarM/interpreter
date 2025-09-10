#pragma once
#include "../helper.h"
#include "types.h"
#include "strings.h"

// helper Macros

// Data types
/*#define NUMBER_ATOMIC_DATA_TYPE ((data_type){Atomic, Number, NULL})
#define NUMBER_ARRAY_DATA_TYPE ((data_type){Number, TRUE})
#define INVALID_DATA_TYPE ((data_type){None, FALSE})
#define FUNCTION_DATA_TYPE ((data_type){Function, FALSE})*/

/*#define IS_ARRAY(d) (d.type.is_array)
#define IS_ATOMIC_NUM_TYPE(d) (d.type.atomic_type == Number && d.type.is_array == FALSE)
#define IS_SAME_TYPE(d1, d2) (d1.type.atomic_type == d2.type.atomic_type && d1.type.is_array == d2.type.is_array)*/


// Data with value
#define EMPTY_DATA ((data){&GLOBAL_INVALID_TYPE, {NULL}})
#define NUMBER_DATA(v) ((data){&GLOBAL_ATOMIC_NUMBER_TYPE, (data_value){.number=v}})
#define FUNCTION_DATA(ptr) ((data){&GLOBAL_FUNCTION_TYPE, (data_value){.function=ptr}})
#define STRING_DATA(s) ((data){&GLOBAL_STRING_TYPE, (data_value){.str=string_new(s)}})
#define STRING_DATA_PTR(ptr) ((data){&GLOBAL_STRING_TYPE, (data_value){.str=ptr}})

// Data extraction
#define GET_NUMBER_FROM_DATA(d) (d.value.number)

// Data access results
#define INVALID_DATA_ACCESS_RESULT ((data_access_result){FALSE, EMPTY_DATA})

struct data_t;
struct data_array_t;
struct string_t;

// Variable / Data types

typedef union data_value_u {
    struct data_array_t* arr;
    struct function_t* function;
    struct string_t* str;
    double number;
} data_value;

typedef struct data_t {
    data_type* type;
    data_value value;
} data;

typedef struct data_access_result_t{
    BOOL valid;
    data result;
} data_access_result;

enum data_update_failure_reason {NoFailure, NotInitialized, DifferentType, NoDict, NotInIndexRange};

typedef struct data_update_result_t{
    BOOL valid;
    enum data_update_failure_reason reason;
} data_update_result;

void free_data(data data);
void data_add_reference(data* d);
void data_remove_reference(data* d);
void data_copy_entire_data(data* destination, data value);
data_update_result data_update_value(data* orig, data new_value);
BOOL data_is_empty(data d);
BOOL data_is_of_kind_array(data d);
BOOL data_is_number_type(data d);
BOOL data_is_string_type(data d);
struct data_array_t* data_get_array_from_data(data d);
struct function_t* data_get_function_from_data(data d);
struct string_t* data_get_string_from_data(data d);
data_value data_default_value(data_type* type);

// Arrays

#define INVALID_ARRAY ((data_array){0, 0, NULL, NULL})

typedef struct data_array_t{
    BOOL initialized;   // Does array have an initialized list?

    int size;           // size which will be shown to user, i.e. size which is "expected"
    int capacity;       // actual size, i.e. how much the underlying array can hold
    int references;
    data_type* type;    // type which the array stores
    data* list;
} data_array;

data_array* data_array_new(data_type* type);
//data_update_result data_array_set(data_array* arr, int index, data data);
//data_access_result data_array_get(data_array* arr, int index);
data* data_array_get_reference(data_array* arr, int index);
void data_array_set_new_type(data_array* arr, data_type* new_type);
int data_array_get_size(data_array* arr);
void free_data_array(data_array* arr);
