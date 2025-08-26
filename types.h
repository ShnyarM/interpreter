#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "helper.h"
#include "data.h"
#include "dictionary.h"

// Expressions
typedef enum expr_type {Lit, Add, Sub, Mul, Div, Neg, Var, Arr_Access, Bool_Neg, Eq, Lt, Lte} e_type;

typedef struct expression_t{
    e_type type;
    data data;                          // Value if it is literal
    char* variable;                   // Name if it is variable
    struct expression_t *expr_1;
    struct expression_t *expr_2;
    size_t line;                      // line at which this expression appears
} expr;


// Statements
typedef enum statement_type {Ass, Arr_Ass, Var_Init, Arr_Init, Print, If, While, Block, Brk, Stop, Function_Call} s_type;

typedef struct statement_t{
    s_type type;
    char* variable;
    expr* expression;
    expr* expression_2;               // used for array assignments to store index
    struct statement_t* next_statement;
    size_t line;                      // line at which this expression appears
    data_type data_type;              // for variable initialization statements, store the type of the variable

    struct statement_t* inner_stmt_1; //Used for if true branch, while and block
    struct statement_t* inner_stmt_2; //used for if false branch (else path)$
} stmt;

// Functions
typedef struct function_t{
    char* name;
    stmt* content;
    data_type return_type;
} function;

// Program
typedef struct program_t{
    BOOL valid;
    stmt* first_statement;
    dictionary functions;
} program;