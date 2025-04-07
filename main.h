#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "helper.h"

typedef enum expr_type {Lit, Add, Sub, Mul, Div, Neg, Var} e_type;

typedef struct expression_t{
    e_type type;
    int val;                      // Value if it is literal
    char* name;                   // Name if it is variable
    struct expression_t *expr_1;
    struct expression_t *expr_2;
} expr;

typedef struct parse_result_t{
    BOOL valid;
    expr* expression;
    char* remaining;
} parse_result;

typedef struct token_result_t{
    char token;
    char* remaining;
} token_result;

#define INVALID_PARSE_RESULT (parse_result){FALSE, NULL, NULL}
#define PARSE_RESULT(e, r) (parse_result){TRUE, e, r}

parse_result parse_parens(char* str);
parse_result parse_integer(char* str);
parse_result parse_atomic(char* str);
parse_result parse_mul_expr(char* str);
parse_result parse_variable(char* str);