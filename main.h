#include <stdint.h>
#include <stdlib.h>

typedef enum expr_type {Lit, Add, Sub} e_type;

typedef struct expression_t{
    e_type type;
    int val;
    struct expression_t *expr_1;
    struct expression_t *expr_2;
} expr;

typedef struct parse_result_t{
    int valid;
    expr expression;
    char* remaining;
} parse_result;

#define INVALID_PARSE_RESULT {0, {Lit, 0, NULL, NULL}, NULL}

parse_result* parse_expr(char* str);