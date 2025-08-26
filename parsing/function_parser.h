#pragma once

#include "../helper.h"
#include "expr_parser.h"
#include "../types.h"

typedef struct parse_result_function_t{
    BOOL valid;
    function* function;
    char* remaining;
    BOOL fatal;         //If this is true mistake in code was found, abort everything and give error
} parse_result_function;

parse_result_function parse_function(char* str);
void free_function(function* s);