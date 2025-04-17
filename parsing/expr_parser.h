#pragma once

#include "../helper.h"
#include "../types.h"

typedef struct parse_result_expr_t{
    BOOL valid;
    expr* expression;
    char* remaining;
    BOOL fatal;         //If this is true mistake in code was found, abort everything and give error
} parse_result_expr;

parse_result_expr parse_expr(char* str);
parse_result_expr parse_variable(char* str);
void free_expr(expr* e);