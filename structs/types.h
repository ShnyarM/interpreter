#pragma once

#define NUMBER_ATOMIC_DATA_TYPE ((data_type){Type_Kind_Atomic, Type_Atomic_Number, NULL})
#define INVALID_DATA_TYPE ((data_type){None, None, NULL})

typedef enum atomic_d_type_e {None, Any, Type_Atomic_Number} atomic_data_type;
typedef enum d_type_kind_e {None, Type_Kind_Atomic, Type_Kind_Array, Function} data_type_kind;

typedef struct data_type_t {
    data_type_kind kind;
    atomic_data_type atomic_type;
    struct data_type_t* inner_type;
} data_type;

void free_type(data_type* t);
data_type* get_atomic_number_type();
data_type* get_array_type(data_type* inner);
data_type* copy_of_type(data_type* t);
BOOL type_equality(data_type* t1, data_type* t2);
BOOL is_type_array(data_type t);
BOOL is_type_number(data_type t);