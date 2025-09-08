#pragma once

struct expression_t;

#include <stdlib.h>
#include "data.h"
#include "variable_access.h"

// Expressions
typedef enum expr_type_e {Lit, Add, Sub, Mul, Div, Neg, Var_Acc, Function_Call, Bool_Neg, Eq, Lt, Lte} e_type;

typedef struct expression_t{
    e_type type;
    data data;                          // Value if it is literal
    struct variable_access_t* variable; // variable_access
    struct expression_t *expr_1;
    struct expression_t *expr_2;
    struct expression_t *next;          // If it is part of a linked list of expressions
    size_t line;                      // line at which this expression appears
} expr;

expr* expr_get_plain(e_type type, data data, expr *ex_1, expr *ex_2);
expr* expr_get_var_name(char *name);
expr* expr_ger_number_literal(double value);
void free_expr(expr *e);