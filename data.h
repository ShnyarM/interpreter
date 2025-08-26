#pragma once
#include "helper.h"

// helper Macros

// Data types
#define NUMBER_ATOMIC_DATA_TYPE ((data_type){Number, FALSE})
#define NUMBER_ARRAY_DATA_TYPE ((data_type){Number, TRUE})
#define INVALID_DATA_TYPE ((data_type){None, FALSE})

#define IS_ARRAY(d) (d.type.is_array)
#define IS_ATOMIC_NUM_TYPE(d) (d.type.atomic_type == Number && d.type.is_array == FALSE)
#define IS_SAME_TYPE(d1, d2) (d1.type.atomic_type == d2.type.atomic_type && d1.type.is_array == d2.type.is_array)


// Data with value
#define EMPTY_DATA ((data){INVALID_DATA_TYPE, {NULL}, 0})
#define NUMBER_DATA(v) ((data){NUMBER_ATOMIC_DATA_TYPE, (data_value){.number=v}, 1})
#define NUMBER_ARRAY_DATA(ptr, s) ((data){NUMBER_ARRAY_DATA_TYPE, (data_value){.arr=ptr}, s}) // Takes in ptr to array and size

// Data extraction
#define GET_NUMBER_FROM_DATA(d) (d.value.number)

// Data access results
#define INVALID_DATA_ACCESS_RESULT ((data_access_result){FALSE, (data){INVALID_DATA_TYPE, {NULL}}})

// Arrays
#define INVALID_ARRAY ((data_array){0, 0, INVALID_DATA_TYPE, NULL})


struct data_t;
struct data_array_t;

// Variable / Data types
typedef enum atomic_d_type_e {None, Any, Number} atomic_data_type;

typedef struct data_type_t {
    atomic_data_type atomic_type;
    BOOL is_array;
} data_type;

typedef union data_value_u {
    struct data_array_t* arr;
    double number;
} data_value;

typedef struct data_t {
    data_type type;
    data_value value;
    int size; //size incase it is array
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

void data_free(data data);
data_update_result data_update_value(data* orig, data new_value);

//Arrays

typedef struct data_array_t{
    int size;
    int references;
    data_type type;
    data* list;
} data_array;

data_array* data_array_new(int size, data_type type);
data_update_result data_array_set(data_array* arr, int index, data data);
data_access_result data_array_get(data_array* arr, int index);
void data_array_add_reference(data_array* arr);
void data_array_remove_reference(data_array* arr);
void data_array_free(data_array* arr);
