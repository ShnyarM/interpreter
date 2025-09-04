#include "expression.h"

extern size_t LINE_COUNTER;

expr *expr_get_plain(e_type type, data data, expr *ex_1, expr *ex_2)
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

expr *expr_get_var_name(char *name)
{
    expr *expression = expr_get_plain(Var_Acc, EMPTY_DATA, NULL, NULL);
    expression->variable = var_access_get_literal(name);
    return expression;
}

expr* expr_ger_number_literal(double value){
    expr* expression = expr_get_plain(Lit, NUMBER_DATA(value), NULL, NULL);
    return expression;
}

void free_expr(expr *e)
{
    if (e == NULL) return;

    if (e->variable != NULL) free(e->variable);

    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free_var_access(e->variable);

    free(e);
}