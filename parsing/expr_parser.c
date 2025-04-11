#include "../helper.h"
#include "../types.h"
#include "expr_parser.h"
#include "_parser_helper.h"

#define INVALID_PARSE_RESULT_EXPR \
    (parse_result_expr) { FALSE, NULL, NULL }
#define PARSE_RESULT_EXPR(e, r) \
    (parse_result_expr) { TRUE, e, r }

extern size_t MAX_VARIABLE_LENGTH;

static parse_result_expr parse_add_expr(char *str);
static parse_result_expr parse_mul_expr(char *str);
static parse_result_expr parse_atomic(char *str);
static parse_result_expr parse_integer(char *str);
static parse_result_expr parse_parens(char *str);

void free_expr(expr *e)
{
    if (e == NULL)
        return;

    if (e->variable != NULL) free(e->variable);

    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free(e);
}

static expr *get_expr(e_type type, int val, expr *ex_1, expr *ex_2)
{
    expr *expression = (expr *)malloc(sizeof(expr));
    expression->type = type;
    expression->val = val;
    expression->variable = NULL;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    return expression;
}

static expr *get_var_expr(char *name)
{
    expr *expression = (expr *)malloc(sizeof(expr));
    expression->type = Var;
    expression->val = 0;
    expression->variable = copy_of_string(name); // makes its own copy
    expression->expr_1 = NULL;
    expression->expr_2 = NULL;
    return expression;
}

parse_result_expr parse_expr(char *str)
{
    parse_result_expr add_expr = parse_add_expr(str);

    if (!add_expr.valid)
        return INVALID_PARSE_RESULT_EXPR;

    char *remaining = add_expr.remaining;

    while (TRUE)
    {
        // Get possible operator, looks very ugly
        e_type operator;
        char *next = word_follows(remaining, "==", FALSE);

        if (next == NULL)
        {
            next = word_follows(remaining, "<=", FALSE);

            if (next == NULL)
            {
                next = word_follows(remaining, "<", FALSE);
                if (next == NULL)
                    break;

                operator= Lt;
            }
            else
            {
                operator= Lte;
            }
        }
        else
        {
            operator= Eq;
        }

        parse_result_expr next_add_expr = parse_add_expr(next);

        if (!next_add_expr.valid)
            break;

        expr *combined = get_expr(operator, 0, add_expr.expression, next_add_expr.expression);

        add_expr.expression = combined;
        add_expr.remaining = next_add_expr.remaining;
        remaining = add_expr.remaining;
    }

    return add_expr;
}

static parse_result_expr parse_add_expr(char *str)
{
    parse_result_expr mul_expr = parse_mul_expr(str);

    if (!mul_expr.valid)
        return INVALID_PARSE_RESULT_EXPR;

    char *remaining = mul_expr.remaining;

    while (TRUE)
    {
        token_result next = get_next_char(remaining);

        if (next.token != '+' && next.token != '-')
            break;

        parse_result_expr next_mul_expr = parse_mul_expr(next.remaining);

        if (!next_mul_expr.valid)
            break;

        expr *combined = get_expr(char_to_operator(next.token), 0, mul_expr.expression, next_mul_expr.expression);

        mul_expr.expression = combined;
        mul_expr.remaining = next_mul_expr.remaining;
        remaining = mul_expr.remaining;
    }

    return mul_expr;
}

static parse_result_expr parse_mul_expr(char *str)
{
    parse_result_expr atomic = parse_atomic(str);

    if (!atomic.valid)
        return INVALID_PARSE_RESULT_EXPR;

    char *remaining = atomic.remaining;

    while (TRUE)
    {
        token_result next = get_next_char(remaining);

        if (next.token != '*' && next.token != '/')
            break;

        parse_result_expr next_atomic = parse_atomic(next.remaining);

        if (!next_atomic.valid)
            break;

        expr *combined = get_expr(char_to_operator(next.token), 0, atomic.expression, next_atomic.expression);

        atomic.expression = combined;
        atomic.remaining = next_atomic.remaining;
        remaining = atomic.remaining;
    }

    return atomic;
}

static parse_result_expr parse_atomic(char *str)
{
    token_result next = get_next_char(str);
    if (next.token == '-')
    {
        parse_result_expr atomic = parse_atomic(next.remaining);
        if (atomic.valid)
        {
            expr *neg = get_expr(Neg, 0, atomic.expression, NULL);
            atomic.expression = neg;
            return atomic;
        }
    }
    if (next.token == '!')
    {
        parse_result_expr atomic = parse_atomic(next.remaining);
        if (atomic.valid)
        {
            expr *bool_neg = get_expr(Bool_Neg, 0, atomic.expression, NULL);
            atomic.expression = bool_neg;
            return atomic;
        }
    }

    parse_result_expr integer = parse_integer(str);
    if (integer.valid)
        return integer;

    parse_result_expr variable = parse_variable(str);
    if (variable.valid)
        return variable;

    parse_result_expr parens = parse_parens(str);
    if (parens.valid)
        return parens;

    return INVALID_PARSE_RESULT_EXPR;
}

static parse_result_expr parse_integer(char *str)
{
    str = remove_spaces(str);

    if (!is_digit(*str))
        return INVALID_PARSE_RESULT_EXPR;

    char *str_start = str;

    // go forward while character infront is still digit
    while (is_digit(str[1]))
        str++;
    char *str_end = str + 1;

    int value = 0;
    int multiplier = 1;
    while (str_start <= str)
    {
        value += (char_to_digit(*str) * multiplier);
        multiplier *= 10;
        str--;
    }

    expr *literal = get_expr(Lit, value, NULL, NULL);
    return PARSE_RESULT_EXPR(literal, str_end);
}

parse_result_expr parse_variable(char *str)
{
    str = remove_spaces(str);

    if (!is_var_letter(*str))
        return INVALID_PARSE_RESULT_EXPR;

    // Limit length of variable to 50 characters
    char buffer[MAX_VARIABLE_LENGTH + 1];

    int index = 0;
    char *name_start = str;
    while (is_var_letter(*str) || is_digit(*str))
    {
        if (index >= MAX_VARIABLE_LENGTH)
        {
            return INVALID_PARSE_RESULT_EXPR; // Variable too long
        }

        buffer[index] = name_start[index];
        index++;
        str++;
    }
    buffer[index] = 0;

    expr *var_expr = get_var_expr(buffer);
    return PARSE_RESULT_EXPR(var_expr, str);
}

static parse_result_expr parse_parens(char *str)
{
    char *parens_open = char_follows(str, '(', FALSE);
    if (parens_open == NULL)
        return INVALID_PARSE_RESULT_EXPR;

    parse_result_expr expr = parse_expr(parens_open);
    if (!expr.valid)
        return INVALID_PARSE_RESULT_EXPR;

    char *parens_close = char_follows(expr.remaining, ')', FALSE);
    if (parens_close == NULL)
    {
        free_expr(expr.expression);
        return INVALID_PARSE_RESULT_EXPR;
    }

    return PARSE_RESULT_EXPR(expr.expression, parens_close);
}