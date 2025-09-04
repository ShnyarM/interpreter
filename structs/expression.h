#pragma once

#include <stdlib.h>
#include "data.h"
#include "variable_access.h"

// Expressions
typedef enum expr_type_e {Lit, Add, Sub, Mul, Div, Neg, Var_Acc, Bool_Neg, Eq, Lt, Lte} e_type;

typedef struct expression_t{
    e_type type;
    data data;                          // Value if it is literal
    data_type* data_type;               // For type expressions
    var_access* variable;                   // Name if it is variable
    struct expression_t *expr_1;
    struct expression_t *expr_2;
    size_t line;                      // line at which this expression appears
} expr;

expr *get_expr(e_type type, data data, expr *ex_1, expr *ex_2)
{
    expr *expression = (expr *)malloc(sizeof(expr));
    expression->type = type;
    expression->data = data;
    expression->variable = NULL;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    expression->line = LINE_COUNTER;
    return expression;
}

expr *expr_get_var_name(char *name);
expr* expr_ger_number_literal(double value);
void free_expr(expr *e);