#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "helper.h"


typedef enum expr_type {Lit, Add, Sub, Mul, Div, Neg, Var, Bool_Neg, Eq, Lt, Lte} e_type;

typedef struct expression_t{
    e_type type;
    int val;                          // Value if it is literal
    char* variable;                   // Name if it is variable
    struct expression_t *expr_1;
    struct expression_t *expr_2;
} expr;

typedef enum statement_type {Ass, Print, If, While, Brk, Stop} s_type;

typedef struct statement_t{
    s_type type;
    char* variable;
    expr* expression;
    struct statement_t* next_statement;

    struct statement_t* inner_stmt_1; //Used for if true branch and for while
    struct statement_t* inner_stmt_2; //used for if false branch (else path)
} stmt;