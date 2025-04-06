#include <stdio.h>
#include <stdint.h>
#include "main.h"
#include "helper.h"

#define EXPR_SIZE sizeof(expr)
#define PARSE_RESULT_SIZE sizeof(parse_result)

// Expr := Atom (('+' | '-') Atom)*
// Atom := Int | '(' Expr ')'

int is_operator(char c){
    if(c == '+' || c == '-') return 1;
    return 0;
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

void free_parse_result(parse_result* r){
    if(r == NULL) return;
    free_expr(r->expression.expr_1);
    free_expr(r->expression.expr_2);
    free(r);
}

expr* get_expr_ptr(e_type type, int val, expr* ex_1, expr* ex_2){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = type;
    expression->val = val;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    return expression;
}

expr get_expr(e_type type, int val, expr* ex_1, expr* ex_2){
    expr expression;
    expression.type = type;
    expression.val = val;
    expression.expr_1 = ex_1;
    expression.expr_2 = ex_2;
    return expression;
}

parse_result* get_parse_results(expr expression, char* remaining){
    parse_result* p = (parse_result*)malloc(PARSE_RESULT_SIZE);
    p->expression = expression;
    p->remaining = remaining;
    p->valid = 1;
    return p;
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

parse_result* parse_integer(char* str){
    if(!is_digit(*str) && *str != '-') return NULL;

    int negative_mult = 1;
    if(*str == '-'){
        negative_mult = -1;
        str++;
    }
    if(!is_digit(*str)) return NULL;

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

    expr literal = get_expr(Lit, value*negative_mult, NULL, NULL);
    return get_parse_results(literal, str_end);
}

parse_result* parse_atom(char* str){
    // Try if its a integer
    parse_result* integer = parse_integer(str);

    if(integer != NULL){
        return integer;
    }

    // Try if its an expression surrounded by brackets
    if(*str == '('){
        parse_result* expression = parse_expr(str+1);
        if(expression == NULL){
            return NULL;
        }

        if(*(expression->remaining) != ')'){
            free(expression);
            return NULL;
        }

        expression->remaining++;
        return expression;
    }

    return NULL;
}


parse_result* parse_oatom(char* str){
    if(is_operator(*str)){
        parse_result* atom = parse_atom(str+1);

        if(atom == NULL){
            return NULL;
        }

        expr* expr_ptr = get_expr_ptr(atom->expression.type, atom->expression.val, atom->expression.expr_1, atom->expression.expr_2);
        expr new_expression = get_expr(char_to_operator(*str), 0, NULL, expr_ptr);
        atom->expression = new_expression;
        return atom;
    }
    return NULL;
}

parse_result* parse_expr(char* str){
    parse_result* atom = parse_atom(str);

    if(atom == NULL) return NULL;

    char* remaining = atom->remaining;
    
    while(1){
        parse_result* new_atom = parse_oatom(remaining);

        if(new_atom == NULL) break;

        new_atom->expression.expr_1 = get_expr_ptr(atom->expression.type, atom->expression.val, atom->expression.expr_1, atom->expression.expr_2);
        parse_result* temp = atom;
        atom = new_atom;
        remaining = atom->remaining;
        
        free(temp);
    }

    return atom;
}

expr* str_to_expression(char* str){
    parse_result* result = parse_expr(str);
    if(result == NULL) return NULL;

    expr* expression = get_expr_ptr(result->expression.type, result->expression.val, result->expression.expr_1, result->expression.expr_2);
    free(result);
    return expression;
}

int is_valid_expression(char* str){
    expr* expression = str_to_expression(str);

    if(expression == NULL) return 0;
    
    free(expression);
    return 1;
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

//TODO: fix brackets, allow spaces, double negation, multiplication