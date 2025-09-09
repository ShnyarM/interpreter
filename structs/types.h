#pragma once
#include "../helper.h"

//#define ATOMIC_NUMBER_DATA_TYPE ((data_type){Type_Kind_Atomic, Type_Atomic_Number, NULL, FALSE})
//#define ATOMIC_NUMBER_DATA_TYPE ((data_type){Type_Kind_Atomic, Type_Atomic_Number, NULL, FALSE})
#define INVALID_DATA_TYPE ((data_type){None, None, NULL})

typedef enum atomic_d_type_e {Type_Atomic_None, Type_Atomic_Any, Type_Atomic_Number, Type_Atomic_String} atomic_data_type;
typedef enum d_type_kind_e {Type_Kind_None, Type_Kind_Any, Type_Kind_Atomic, Type_Kind_Array, Type_Kind_Function} data_type_kind;

typedef struct data_type_t {
    data_type_kind kind;
    atomic_data_type atomic_type;
    struct data_type_t* inner_type;
    BOOL global;                    // Used to say if a data_type object is a global, i.e. it can be used by multiple and doesn't need to be freed
} data_type;

void free_type(data_type* t);
data_type* types_get_array_type(data_type* inner);
data_type* types_copy(data_type* t);
BOOL types_equality(data_type* t1, data_type* t2);
BOOL types_can_be_assigned_to(data_type* lhs, data_type* rhs);
BOOL types_is_array(data_type* t);
BOOL types_is_number(data_type* t);
BOOL types_is_string(data_type* t);
BOOL types_is_any(data_type* t);

extern data_type GLOBAL_INVALID_TYPE;
extern data_type GLOBAL_ANY_TYPE;
extern data_type GLOBAL_ATOMIC_NUMBER_TYPE;
extern data_type GLOBAL_STRING_TYPE;
extern data_type GLOBAL_FUNCTION_TYPE;