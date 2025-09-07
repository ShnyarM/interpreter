#pragma once
#include "statement.h"

// Functions
typedef struct function_t{
    char* name;
    stmt* content;
    data_type* return_type;
} function;

function* function_get(char* name, data_type* return_type, stmt* content);
void free_function(function* func); 