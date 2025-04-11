#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "helper.h"

typedef enum expr_type {Lit, Add, Sub, Mul, Div, Neg, Var, Bool_Neg, Eq, Lt, Lte} e_type;
typedef enum statement_type {Ass, Print, If, While, Brk, Stop} s_type;

typedef struct expression_t{
    e_type type;
    int val;                          // Value if it is literal
    char* variable;                   // Name if it is variable
    struct expression_t *expr_1;
    struct expression_t *expr_2;
} expr;

typedef struct statement_t{
    s_type type;
    char* variable;
    expr* expression;
    struct statement_t* next_statement;

    struct statement_t* inner_stmt_1; //Used for if true branch and for while
    struct statement_t* inner_stmt_2; //used for if false branch (else path)
} stmt;

typedef struct parse_result_expr_t{
    BOOL valid;
    expr* expression;
    char* remaining;
} parse_result_expr;

typedef struct parse_result_statement_t{
    BOOL valid;
    stmt* statement;
    char* remaining;
} parse_result_stmt;

typedef struct token_result_t{
    char token;
    char* remaining;
} token_result;

typedef struct execution_result_t{
    BOOL succesful;
    BOOL stop_execution;
    BOOL break_while;
} execution_result;

#define INVALID_PARSE_RESULT_EXPR (parse_result_expr){FALSE, NULL, NULL}
#define PARSE_RESULT_EXPR(e, r) (parse_result_expr){TRUE, e, r}

#define INVALID_PARSE_RESULT_STMT (parse_result_stmt){FALSE, NULL, NULL}
#define PARSE_RESULT_STMT(s, r) (parse_result_stmt){TRUE, s, r}

parse_result_expr parse_add_expr(char* str);
parse_result_expr parse_parens(char* str);
parse_result_expr parse_integer(char* str);
parse_result_expr parse_atomic(char* str);
parse_result_expr parse_mul_expr(char* str);
parse_result_expr parse_variable(char* str);
parse_result_stmt parse_program(char* str);