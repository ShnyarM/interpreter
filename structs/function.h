#pragma once
#include "statement.h"

typedef struct function_parameter_t{
    char* name;
    data_type* type;
    struct function_parameter_t* next;
} function_parameter;

function_parameter* function_parameter_get(char* name, data_type* return_type);
void free_function_parameter(function_parameter* arg);

// Functions
typedef struct function_t{
    char* name;
    stmt* content;
    data_type* return_type;
    function_parameter* parameters;
} function;

function* function_get_plain();
function* function_get(char* name, data_type* return_type, stmt* content, function_parameter* parameters);
void free_function(function* func); 