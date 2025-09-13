#pragma once
#include "statement.h"

// built in functions

typedef struct built_in_exec_result_t{
    BOOL successful;
    data data;
} built_in_exec_result;

typedef built_in_exec_result (*built_in_function)(data_list*, size_t);

built_in_exec_result built_in_print(data_list* args, size_t line);
built_in_exec_result built_in_length(data_list* args, size_t line);
built_in_exec_result built_in_cast_to_number(data_list* args, size_t line);
built_in_exec_result built_in_cast_to_string(data_list* args, size_t line);
built_in_exec_result built_in_type_of(data_list* args, size_t line);

// general functions

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

    BOOL is_built_in;
    built_in_function built_in_func_ptr;
} function;

function* function_get_plain();
function* function_get_built_in(char* name, built_in_function ptr);
function* function_get(char* name, data_type* return_type, stmt* content, function_parameter* parameters);
void free_function(function* func);