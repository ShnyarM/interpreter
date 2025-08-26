#include "../helper.h"
#include "../types.h"
#include "expr_parser.h"
#include "_parser_helper.h"

#define INVALID_PARSE_RESULT_EXPR (parse_result_expr) { FALSE, NULL, NULL, FALSE }
#define FATAL_PARSE_RESULT_EXPR (parse_result_expr){FALSE, NULL, NULL, TRUE}
#define PARSE_RESULT_EXPR(e, r) (parse_result_expr) { TRUE, e, r, FALSE }

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_EXPR;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_EXPR;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_EXPR;}
#define FATAL_CHECK_WITH_FREE_STMT(f, s)if(f.fatal){free_stmt(s); return FATAL_PARSE_RESULT_EXPR;}
#define FATAL_CHECK_WITH_FREE_EXPR(f, e)if(f.fatal){free_expr(e); return FATAL_PARSE_RESULT_EXPR;}

#define FATAL_CHECK_NULL(f, msg) if(f==NULL){FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_STMT(f, msg, s) if(f==NULL){free_stmt(s);FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_EXPR(f, msg, e) if(f==NULL){free_expr(e);FATAL_REVERT(msg)}

//Standard error messages
extern char OPENING_BRACKET_ERROR[];
extern char CLOSING_BRACKET_ERROR[];
extern char OPENING_CURLY_BRACKET_ERROR[];
extern char CLOSING_CURLY_BRACKET_ERROR[];

extern size_t MAX_VARIABLE_LENGTH;
extern size_t LINE_COUNTER;

static parse_result_expr parse_add_expr(char *str);
static parse_result_expr parse_mul_expr(char *str);
static parse_result_expr parse_atomic(char *str);
static parse_result_expr parse_integer(char *str);
static parse_result_expr parse_parens(char *str);

void free_expr(expr *e)
{
    if (e == NULL) return;

    if (e->variable != NULL) free(e->variable);

    free_expr(e->expr_1);
    free_expr(e->expr_2);

    free(e);
}

static expr *get_expr(e_type type, data data, expr *ex_1, expr *ex_2)
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

static expr *get_var_expr(char *name)
{
    expr *expression = get_expr(Var, EMPTY_DATA, NULL, NULL);
    expression->variable = copy_of_string(name); // makes its own copy
    return expression;
}

expr* get_number_literal(double value){
    expr* expression = get_expr(Lit, NUMBER_DATA(value), NULL, NULL);
    return expression;
}

parse_result_expr parse_expr(char *str)
{   
    PARSE_INIT

    parse_result_expr add_expr = parse_add_expr(str);
    FATAL_CHECK(add_expr)
    if (!add_expr.valid) REVERT

    char *remaining = add_expr.remaining;

    while (TRUE)
    {
        size_t counter_before = LINE_COUNTER;
        
        // Get possible operator, looks very ugly
        e_type operator;
        char *next = word_follows(remaining, "==", FALSE);

        if (next == NULL)
        {
            next = word_follows(remaining, "<=", FALSE);

            if (next == NULL)
            {
                next = word_follows(remaining, "<", FALSE);
                if (next == NULL){
                    LINE_COUNTER = counter_before;
                    break;
                }

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
        FATAL_CHECK_WITH_FREE_EXPR(next_add_expr, add_expr.expression)
        if (!next_add_expr.valid){
            LINE_COUNTER = counter_before;
            break;
        }

        expr *combined = get_expr(operator, EMPTY_DATA, add_expr.expression, next_add_expr.expression);

        add_expr.expression = combined;
        add_expr.remaining = next_add_expr.remaining;
        remaining = add_expr.remaining;
    }

    return add_expr;
}

static parse_result_expr parse_add_expr(char *str)
{
    PARSE_INIT

    parse_result_expr mul_expr = parse_mul_expr(str);
    FATAL_CHECK(mul_expr)
    if (!mul_expr.valid) REVERT

    char *remaining = mul_expr.remaining;

    while (TRUE)
    {
        size_t counter_before = LINE_COUNTER;

        token_result next = get_next_char(remaining);

        if (next.token != '+' && next.token != '-'){
            LINE_COUNTER = counter_before;
            break;
        }

        parse_result_expr next_mul_expr = parse_mul_expr(next.remaining);
        FATAL_CHECK_WITH_FREE_EXPR(next_mul_expr, mul_expr.expression)
        if (!next_mul_expr.valid){
            LINE_COUNTER = counter_before;
            break;
        }

        expr *combined = get_expr(char_to_operator(next.token), EMPTY_DATA, mul_expr.expression, next_mul_expr.expression);

        mul_expr.expression = combined;
        mul_expr.remaining = next_mul_expr.remaining;
        remaining = mul_expr.remaining;
    }

    return mul_expr;
}

static parse_result_expr parse_mul_expr(char *str)
{
    PARSE_INIT

    parse_result_expr atomic = parse_atomic(str);
    FATAL_CHECK(atomic)
    if (!atomic.valid) REVERT

    char *remaining = atomic.remaining;

    while (TRUE)
    {
        size_t counter_before = LINE_COUNTER;

        token_result next = get_next_char(remaining);

        if (next.token != '*' && next.token != '/'){
            LINE_COUNTER = counter_before;
            break;
        }

        parse_result_expr next_atomic = parse_atomic(next.remaining);
        FATAL_CHECK_WITH_FREE_EXPR(next_atomic, atomic.expression)
        if (!next_atomic.valid){
            LINE_COUNTER = counter_before;
            break;
        }

        expr *combined = get_expr(char_to_operator(next.token), EMPTY_DATA, atomic.expression, next_atomic.expression);

        atomic.expression = combined;
        atomic.remaining = next_atomic.remaining;
        remaining = atomic.remaining;
    }

    return atomic;
}

static parse_result_expr parse_atomic(char *str)
{
    PARSE_INIT

    token_result next = get_next_char(str);
    if (next.token == '-')
    {
        parse_result_expr atomic = parse_atomic(next.remaining);
        FATAL_CHECK(atomic)
        if (atomic.valid)
        {
            expr *neg = get_expr(Neg, EMPTY_DATA, atomic.expression, NULL);
            atomic.expression = neg;
            return atomic;
        }
    }
    if (next.token == '!')
    {
        parse_result_expr atomic = parse_atomic(next.remaining);
        FATAL_CHECK(atomic)
        if (atomic.valid)
        {
            expr *bool_neg = get_expr(Bool_Neg, EMPTY_DATA, atomic.expression, NULL);
            atomic.expression = bool_neg;
            return atomic;
        }
    }
    LINE_COUNTER = next.line_counter_before;

    parse_result_expr integer = parse_integer(str);
    FATAL_CHECK(integer)
    if (integer.valid)
        return integer;

    parse_result_expr variable = parse_var_or_array(str);
    FATAL_CHECK(variable)
    if (variable.valid)
        return variable;

    parse_result_expr parens = parse_parens(str);
    FATAL_CHECK(parens)
    if (parens.valid)
        return parens;

    REVERT
}

static parse_result_expr parse_integer(char *str)
{
    PARSE_INIT

    LINE_COUNTER += remove_spaces(&str);

    if (!is_digit(*str)) REVERT

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

    expr *literal = get_expr(Lit, NUMBER_DATA((double)value), NULL, NULL);
    return PARSE_RESULT_EXPR(literal, str_end);
}

parse_result_expr parse_variable(char *str)
{   
    PARSE_INIT

    LINE_COUNTER += remove_spaces(&str);

    if (!is_var_letter(*str)) REVERT

    // Limit length of variable to 50 characters
    char buffer[MAX_VARIABLE_LENGTH + 1];

    int index = 0;
    char *name_start = str;
    while (is_var_letter(*str) || is_digit(*str))
    {
        if (index >= MAX_VARIABLE_LENGTH)
        {
            FATAL_REVERT("Variable name is too long. Please choose a shorter name.")
        }

        buffer[index] = name_start[index];
        index++;
        str++;
    }
    buffer[index] = 0;

    if(is_reserved_keyword(buffer)){
        FATAL_REVERT("Variable name is a reserved keyword, please use a different name.")
    }

    expr *var_expr = get_var_expr(buffer);
    return PARSE_RESULT_EXPR(var_expr, str);
}

parse_result_expr parse_var_or_array(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_variable(str);
    FATAL_CHECK(variable)
    if(!variable.valid) REVERT

    char* square_open_brackets = char_follows(variable.remaining, '[', FALSE);
    if(square_open_brackets == NULL) return variable;
    
    parse_result_expr index = parse_expr(square_open_brackets);
    FATAL_CHECK(index)
    if(!index.valid){
        free_expr(variable.expression);
        FATAL_REVERT("Invalid index for array.")
    }

    char* square_close_brackets = char_follows(index.remaining, ']', FALSE);
    if(square_close_brackets == NULL){
        free_expr(variable.expression);
        free_expr(index.expression);
        FATAL_REVERT("Expected closing square bracket.")
    }

    variable.expression->type = Arr_Access;
    variable.expression->expr_1 = index.expression;
    variable.remaining = square_close_brackets;

    return variable;
}

static parse_result_expr parse_parens(char *str)
{
    PARSE_INIT

    char *parens_open = char_follows(str, '(', FALSE);
    if (parens_open == NULL) REVERT

    parse_result_expr expr = parse_expr(parens_open);
    FATAL_CHECK(expr)
    if (!expr.valid) FATAL_REVERT("Invalid expression after opening brackets.")

    char *parens_close = char_follows(expr.remaining, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(parens_close, CLOSING_BRACKET_ERROR, expr.expression)

    return PARSE_RESULT_EXPR(expr.expression, parens_close);
}