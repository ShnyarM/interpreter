#include "expression.h"

extern size_t LINE_COUNTER;

expr *expr_get_plain(e_type type, data data, expr *ex_1, expr *ex_2){
    expr *expression = (expr *)malloc(sizeof(expr));
    expression->type = type;
    expression->data = data;
    expression->variable = NULL;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    expression->line = LINE_COUNTER;
    expression->next = NULL;
    return expression;
}

expr *expr_get_var_name(char *name){
    expr *expression = expr_get_plain(Expr_Var_Acc, EMPTY_DATA, NULL, NULL);
    expression->variable = var_access_get_literal(name);
    return expression;
}

expr* expr_get_number_literal(double value){
    expr* expression = expr_get_plain(Expr_Lit, NUMBER_DATA(value), NULL, NULL);
    return expression;
}

void free_expr(expr *e)
{
    if (e == NULL) return;
    
    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free_expr(e->next);
    free_var_access(e->variable);

    free(e);
}