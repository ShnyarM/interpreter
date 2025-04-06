#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "helper.h"

typedef enum expr_type {Lit, Add, Sub} e_type;

typedef struct expression_t{
    e_type type;
    int val;
    struct expression_t *expr_1;
    struct expression_t *expr_2;
} expr;

typedef struct parse_result_t{
    BOOL valid;
    expr* expression;
    char* remaining;
} parse_result;

#define INVALID_PARSE_RESULT (parse_result){FALSE, NULL, NULL}
#define PARSE_RESULT(e, r) (parse_result){TRUE, e, r}

parse_result parse_expr(char* str);