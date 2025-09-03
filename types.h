#pragma once

typedef enum atomic_d_type_e {None, Any, Number} atomic_data_type;
typedef enum d_type_kind_e {None, Atomic, Array, Function} data_type_kind;

typedef struct data_type_t {
    data_type_kind kind;
    atomic_data_type atomic_type;
    struct data_type_t* inner_type;
} data_type;

void free_type(data_type* t);
data_type* get_atomic_number_type();
data_type* copy_of_type(data_type* t);
BOOL type_equality(data_type* t1, data_type* t2);