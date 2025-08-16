#pragma once
#include "types.h"
#include "data.h"

// array starts 
typedef struct array_t {
    int size;
    data_type data_type;
    data* list;
} array;