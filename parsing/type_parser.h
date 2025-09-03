#pragma once
#include "../types.h"
#include "../helper.h"


typedef struct parse_result_type_t{
    BOOL valid;
    data_type* type;
    char* remaining;
    BOOL fatal;         //If this is true mistake in code was found, abort everything and give error
} parse_result_type;