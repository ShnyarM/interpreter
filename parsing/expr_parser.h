#pragma once

#include "../helper.h"
#include "../structs/expression.h"

typedef struct parse_result_expr_t{
    BOOL valid;
    expr* expression;
    char* remaining;
    BOOL fatal;         //If this is true mistake in code was found, abort everything and give error
} parse_result_expr;

expr* get_number_literal(double value);
parse_result_expr parse_expr(char* str);
parse_result_expr parse_variable_name(char* str);
parse_result_expr parse_variable_access(char* str);