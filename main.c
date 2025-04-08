#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "helper.h"
#include "dictionary.h"

#define EXPR_SIZE sizeof(expr)
#define PARSE_RESULT_SIZE sizeof(parse_result)
#define MAX_VARIABLE_LENGTH 50

// <expression> ::= <add-expr>
// <add-expr> ::= <mul-expr> (['+' | '-'] <mul-expr>)*
// <mul-expr> ::= <atomic> (['*' | '/'] <atomic>)*
// <atomic> ::= '-'<atomic> | (<literal> | <variable> | <parens>)
// <parens> ::= '(' <expression> ')'

BOOL is_operator(char c){
    if(c == '+' || c == '-') return TRUE;
    return FALSE;
}

e_type char_to_operator(char c){
    if(c == '+') return Add;
    if(c == '-') return Sub;
    if(c == '*') return Mul;
    if(c == '/') return Div;
    return Add;
}

token_result get_next_char(char* str){
    str = remove_spaces(str);
    return (token_result){*str, str+1};
}

void free_expr(expr* e){
    if(e == NULL) return;

    if(e->name != NULL) free(e->name);

    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free(e);
}

expr* get_expr(e_type type, int val, expr* ex_1, expr* ex_2){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = type;
    expression->val = val;
    expression->name = NULL;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    return expression;
}

expr* get_var_expr(char* name){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = Var;
    expression->val = 0;
    expression->name = copy_of_string(name); //makes its own copy
    expression->expr_1 = NULL;
    expression->expr_2 = NULL;
    return expression;
}

int eval_expr(expr* expression, dictionary* dict){
    if(expression->type == Lit){
        return expression->val;
    }

    if(expression->type == Add){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 + value_2;
    }

    if(expression->type == Sub){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 - value_2;
    }

    if(expression->type == Mul){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 * value_2;
    }

    if(expression->type == Div){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 / value_2;
    }

    if(expression->type == Neg){
        int value_1 = eval_expr(expression->expr_1, dict);

        return -value_1;
    }

    if(expression->type == Var){
        dict_result var_value = dict_get(dict, expression->name);
        if(var_value.valid) return var_value.result;

        return 0;
    }

    return -1;
}

parse_result parse_add_expr(char* str){
    parse_result mul_expr = parse_mul_expr(str);

    if(!mul_expr.valid) return INVALID_PARSE_RESULT;

    char* remaining = mul_expr.remaining;
    
    while(TRUE){
        token_result next = get_next_char(remaining);
        
        if(next.token != '+' && next.token != '-') break;

        parse_result next_mul_expr = parse_mul_expr(next.remaining);

        if(!next_mul_expr.valid) break;

        expr* combined = get_expr(char_to_operator(next.token), 0, mul_expr.expression, next_mul_expr.expression);

        mul_expr.expression = combined;
        mul_expr.remaining = next_mul_expr.remaining;
        remaining = mul_expr.remaining;
    }

    return mul_expr;
}

parse_result parse_mul_expr(char* str){
    parse_result atomic = parse_atomic(str);

    if(!atomic.valid) return INVALID_PARSE_RESULT;

    char* remaining = atomic.remaining;
    
    while(TRUE){
        token_result next = get_next_char(remaining);
        
        if(next.token != '*' && next.token != '/') break;

        parse_result next_atomic = parse_atomic(next.remaining);

        if(!next_atomic.valid) break;

        expr* combined = get_expr(char_to_operator(next.token), 0, atomic.expression, next_atomic.expression);

        atomic.expression = combined;
        atomic.remaining = next_atomic.remaining;
        remaining = atomic.remaining;
    }

    return atomic;
}

parse_result parse_atomic(char* str){
    token_result next = get_next_char(str);
    if(next.token == '-'){
        parse_result atomic = parse_atomic(next.remaining);
        if(atomic.valid){
            expr* neg = get_expr(Neg, 0, atomic.expression, NULL);
            atomic.expression = neg;
            return atomic;
        }
    }


    parse_result integer = parse_integer(str);
    if(integer.valid) return integer;

    parse_result variable = parse_variable(str);
    if(variable.valid) return variable;

    parse_result parens = parse_parens(str);
    if(parens.valid) return parens;

    return INVALID_PARSE_RESULT;
}


parse_result parse_integer(char* str){
    str = remove_spaces(str);

    if(!is_digit(*str)) return INVALID_PARSE_RESULT;

    char* str_start = str;

    // go forward while character infront is still digit
    while(is_digit(str[1])) str++;
    char* str_end = str+1;

    int value = 0;
    int multiplier = 1;
    while(str_start <= str){
        value += (char_to_digit(*str)*multiplier);
        multiplier *= 10;
        str--;
    }

    expr* literal = get_expr(Lit, value, NULL, NULL);
    return PARSE_RESULT(literal, str_end);
}

parse_result parse_variable(char* str){
    str = remove_spaces(str);

    if(!is_var_letter(*str)) return INVALID_PARSE_RESULT;

    //Limit length of variable to 50 characters
    char buffer[MAX_VARIABLE_LENGTH+1];

    int index = 0;
    char* name_start = str;
    while(is_var_letter(*str) || is_digit(*str)){
        if(index >= MAX_VARIABLE_LENGTH){
            return INVALID_PARSE_RESULT; //Variable too long
        }

        buffer[index] = name_start[index];
        index++;
        str++;
    }
    buffer[index] = 0;

    expr* var_expr = get_var_expr(buffer);
    return PARSE_RESULT(var_expr, str);
}

parse_result parse_parens(char* str){
    token_result next = get_next_char(str);
    if(next.token != '(') return INVALID_PARSE_RESULT;

    parse_result add_expr = parse_add_expr(next.remaining);
    if(!add_expr.valid) return INVALID_PARSE_RESULT;

    token_result next_2 = get_next_char(add_expr.remaining);
    if(next_2.token != ')'){
        free_expr(add_expr.expression);
        return INVALID_PARSE_RESULT;
    }

    return PARSE_RESULT(add_expr.expression, next_2.remaining);
}

// Top level function to start parsing process
expr* str_to_expression(char* str){
    parse_result result = parse_add_expr(str);
    if(!result.valid) return NULL;

    //check if there is not additional stuff at end of string
    if(result.remaining[0] != 0){
        free_expr(result.expression);
        return NULL;
    }

    return result.expression;
}

BOOL is_valid_expression(char* str){
    expr* expression = str_to_expression(str);

    if(expression == NULL) return FALSE;
    
    free(expression);
    return TRUE;
}

int calculate(char* str, dictionary* dict){
    expr* expression = str_to_expression(str);

    if(expression == NULL) return 0;

    int result = eval_expr(expression, dict);
    free(expression);
    return result;
}

int main(int argc, char** argv){

    //char* str = "5+(1-1)";
    //parse_result* result = parse_expr(str);
    //printf("%d\n", eval_expr(&(result->expression)));

    if(argc < 2){
        printf("No arithmetic expression has been given.\n");
        return 1;
    }

    dictionary dict = dict_new();
    dict_add(&dict, "x", 5);
    dict_add(&dict, "y", 1);
    dict_add(&dict, "z", 20);
    dict_add(&dict, "a", 40);
    dict_add(&dict, "b", 12);

    int result = calculate(argv[1], &dict);
    if(result == 0){
        if(!is_valid_expression(argv[1])){
            printf("Invalid arithmetic expression!\n");
            return 1;
        }
    }
    printf("%d\n", result);

    return 0;
}

//TODO: dictionary free and valid and new when invalid, variables