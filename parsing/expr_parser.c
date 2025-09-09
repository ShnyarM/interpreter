#include "../helper.h"
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
extern char OPENING_SQUARE_BRACKET_ERROR[];
extern char CLOSING_SQUARE_BRACKET_ERROR[];

extern size_t MAX_VARIABLE_LENGTH;
extern size_t LINE_COUNTER;

static parse_result_expr parse_add_expr(char *str);
static parse_result_expr parse_mul_expr(char *str);
static parse_result_expr parse_atomic(char *str);
static parse_result_expr parse_integer(char *str);
static parse_result_expr parse_parens(char *str);
static parse_result_expr parse_function_call(char* str);
static parse_result_expr parse_array(char* str);

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

                operator=Expr_Lt;
            }
            else
            {
                operator= Expr_Lte;
            }
        }
        else
        {
            operator= Expr_Eq;
        }

        parse_result_expr next_add_expr = parse_add_expr(next);
        FATAL_CHECK_WITH_FREE_EXPR(next_add_expr, add_expr.expression)
        if (!next_add_expr.valid){
            LINE_COUNTER = counter_before;
            break;
        }

        expr *combined = expr_get_plain(operator, EMPTY_DATA, add_expr.expression, next_add_expr.expression);

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

        expr *combined = expr_get_plain(char_to_operator(next.token), EMPTY_DATA, mul_expr.expression, next_mul_expr.expression);

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

        expr *combined = expr_get_plain(char_to_operator(next.token), EMPTY_DATA, atomic.expression, next_atomic.expression);

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
            expr *neg = expr_get_plain(Expr_Neg, EMPTY_DATA, atomic.expression, NULL);
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
            expr *bool_neg = expr_get_plain(Expr_Bool_Neg, EMPTY_DATA, atomic.expression, NULL);
            atomic.expression = bool_neg;
            return atomic;
        }
    }
    LINE_COUNTER = next.line_counter_before;

    parse_result_expr integer = parse_integer(str);
    FATAL_CHECK(integer)
    if (integer.valid)
        return integer;

    parse_result_expr function_call = parse_function_call(str);
    FATAL_CHECK(function_call)
    if (function_call.valid)
        return function_call;

    parse_result_expr variable = parse_variable_access(str);
    FATAL_CHECK(variable)
    if (variable.valid)
        return variable;

    parse_result_expr array = parse_array(str);
    FATAL_CHECK(array)
    if (array.valid)
        return array;

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

    expr *literal = expr_get_plain(Expr_Lit, NUMBER_DATA((double)value), NULL, NULL);
    return PARSE_RESULT_EXPR(literal, str_end);
}

parse_result_expr parse_variable_name(char *str)
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

    expr *var_expr = expr_get_var_name(buffer);
    return PARSE_RESULT_EXPR(var_expr, str);
}

parse_result_expr parse_variable_access(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_variable_name(str);
    FATAL_CHECK(variable)
    if(!variable.valid) REVERT

    char* remaining = variable.remaining;

    while(TRUE){
        char* square_open_brackets = char_follows(remaining, '[', FALSE);
        if(square_open_brackets == NULL) break;
        
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

        var_access* array_access = var_access_get_array(variable.expression->variable, index.expression);
        variable.expression->variable = array_access;
        remaining = square_close_brackets;
    }

    variable.remaining = remaining;

    return variable;
}

static parse_result_expr parse_function_call(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_variable_name(str);
    // FATAL check gets skipped here, as it will falsely flag number declarations as keyword violations
    // TO DO: maybe do keyword violation check only in clear variable inits
    if(!variable.valid || variable.fatal) REVERT

    char* opening_bracket = char_follows(variable.remaining, '(', FALSE);
    if(opening_bracket == NULL){
        free_expr(variable.expression);
        REVERT
    }

    expr* arguments = NULL;
    char* remaining = opening_bracket;

    // Get optional arguments
    parse_result_expr arguments_parse = parse_expression_list(opening_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(arguments_parse, variable.expression);
    if(arguments_parse.valid){
        arguments = arguments_parse.expression;
        remaining = arguments_parse.remaining;
    }

    char* closing_bracket = char_follows(remaining, ')', FALSE);
    if(closing_bracket == NULL){
        free_expr(variable.expression);
        free_expr(arguments);
        FATAL_REVERT(CLOSING_BRACKET_ERROR);
    }

    expr* expr = expr_get_plain(Expr_Function_Call, EMPTY_DATA, NULL, NULL);
    expr->variable = variable.expression->variable;
    expr->expr_1 = arguments;
    
    variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
    free_expr(variable.expression);
    return PARSE_RESULT_EXPR(expr, closing_bracket);
}

parse_result_expr parse_expression_list(char* str){
    PARSE_INIT

    parse_result_expr first_expression = parse_expr(str);
    FATAL_CHECK(first_expression);
    if(!first_expression.valid) REVERT;

    expr* first = first_expression.expression;
    expr* last = first;
    char* remaining = first_expression.remaining;

    while(TRUE){
        char* comma = char_follows(remaining, ',', FALSE);
        if(comma == NULL) break;

        parse_result_expr expression = parse_expr(comma);
        FATAL_CHECK_WITH_FREE_EXPR(expression, first);
        if(!expression.valid){
            free_expr(first);
            FATAL_REVERT("Expected an expression after ',' keyword in expression list.");
        }

        last->next = expression.expression;
        last = expression.expression;
        remaining = expression.remaining;
    }

    return PARSE_RESULT_EXPR(first, remaining);
}

static parse_result_expr parse_array(char* str){
    PARSE_INIT

    char* square_bracket = char_follows(str, '[', FALSE);
    if(square_bracket == NULL) REVERT

    // get optional expression list
    expr* list = NULL;
    char* remaining = square_bracket;

    parse_result_expr parse_list = parse_expression_list(square_bracket);
    FATAL_CHECK(parse_list);
    if(parse_list.valid){
        list = parse_list.expression;
        remaining = parse_list.remaining;
    }

    char* closing_bracket = char_follows(remaining, ']', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(closing_bracket, CLOSING_SQUARE_BRACKET_ERROR, list);

    expr* expression = expr_get_plain(Expr_Array, EMPTY_DATA, list, NULL);
    return PARSE_RESULT_EXPR(expression, closing_bracket);
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