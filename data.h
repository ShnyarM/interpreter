#pragma once
#include "helper.h"

// helper Macros

// Data types
#define ATOMIC_NUMBER_DATA_TYPE ((data_type){Number, FALSE})

#define IS_ATOMIC_NUM_TYPE(d) (d.type.type == Number && d.type.is_array == FALSE)

// Data with value
#define EMPTY_DATA ((data){(data_type){None, FALSE}, {NULL}})
#define NUMBER_DATA(v) ((data){ATOMIC_NUMBER_DATA_TYPE, (data_value){.number=v}})

// Data extraction
#define GET_NUMBER_FROM_DATA(d) (d.value.number)

// Data access results
#define INVALID_DATA_ACCESS_RESULT ((data_access_result){FALSE, (data){(data_type){None, FALSE}, {NULL}}})

// Variable / Data types
typedef enum atomic_d_type_e {None, Number} atomic_data_type;

typedef struct data_type_e {
    atomic_data_type type;
    BOOL is_array;
} data_type;

typedef union data_value_u {
    void* arr;
    double number;
} data_value;

typedef struct data_e {
    data_type type;
    data_value value;
} data;

typedef struct data_access_result_t{
    BOOL valid;
    data result;
} data_access_result;
