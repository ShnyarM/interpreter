#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "helper.h"

#define EXPR_SIZE sizeof(expr)
#define PARSE_RESULT_SIZE sizeof(parse_result)

// Expr := Atom (('+' | '-') Atom)*
// Atom := Int | '(' Expr ')'

BOOL is_operator(char c){
    if(c == '+' || c == '-') return TRUE;
    return FALSE;
}

e_type char_to_operator(char c){
    if(c == '+') return Add;
    if(c == '-') return Sub;
    return Add;
}

void free_expr(expr* e){
    if(e == NULL) return;
    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free(e);
}

expr* get_expr(e_type type, int val, expr* ex_1, expr* ex_2){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = type;
    expression->val = val;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    return expression;
}

int eval_expr(expr* expression){
    if(expression->type == Lit){
        return expression->val;
    }

    if(expression->type == Add){
        int value_1 = eval_expr(expression->expr_1);
        int value_2 = eval_expr(expression->expr_2);

        return value_1 + value_2;
    }

    if(expression->type == Sub){
        int value_1 = eval_expr(expression->expr_1);
        int value_2 = eval_expr(expression->expr_2);

        return value_1 - value_2;
    }

    return -1;
}

parse_result parse_integer(char* str){
    str = remove_spaces(str);

    if(!is_digit(*str) && *str != '-') return INVALID_PARSE_RESULT;

    int negative_mult = 1;
    while(*str == '-'){
        negative_mult *= -1;
        str++;
    }
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

    expr* literal = get_expr(Lit, value*negative_mult, NULL, NULL);
    return PARSE_RESULT(literal, str_end);
}

parse_result parse_atom(char* str){
    // Try if its a integer
    parse_result integer = parse_integer(str);

    if(integer.valid){
        return integer;
    }

    str = remove_spaces(str);

    // Try if its an expression surrounded by brackets
    if(*str == '('){
        parse_result expression = parse_expr(str+1);
        if(!expression.valid){
            return INVALID_PARSE_RESULT;
        }

        expression.remaining = remove_spaces(expression.remaining);

        if(*(expression.remaining) != ')'){
            free(expression.expression);
            return INVALID_PARSE_RESULT;
        }

        expression.remaining++;
        return expression;
    }

    return INVALID_PARSE_RESULT;
}


parse_result parse_oatom(char* str){
    str = remove_spaces(str);

    if(is_operator(*str)){
        parse_result atom = parse_atom(str+1);

        if(!atom.valid){
            return INVALID_PARSE_RESULT;
        }

        expr* new_expression = get_expr(char_to_operator(*str), 0, NULL, atom.expression);
        atom.expression = new_expression;
        return atom;
    }
    return INVALID_PARSE_RESULT;
}

parse_result parse_expr(char* str){
    parse_result atom = parse_atom(str);

    if(!atom.valid) return INVALID_PARSE_RESULT;

    char* remaining = atom.remaining;
    
    while(TRUE){
        parse_result new_atom = parse_oatom(remaining);

        if(!new_atom.valid) break;

        new_atom.expression->expr_1 = atom.expression;
        atom = new_atom;
        remaining = atom.remaining;
    }

    return atom;
}

// Top level function to start parsing process
expr* str_to_expression(char* str){
    parse_result result = parse_expr(str);
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

int calculate(char* str){
    expr* expression = str_to_expression(str);

    if(expression == NULL) return 0;

    int result = eval_expr(expression);
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

    int result = calculate(argv[1]);
    if(result == 0){
        if(!is_valid_expression(argv[1])){
            printf("Invalid arithmetic expression!\n");
            return 1;
        }
    }
    printf("%d\n", result);

    return 0;
}

//TODO: multiplication