#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "main.h"
#include "helper.h"
#include "dictionary.h"

#define EXPR_SIZE sizeof(expr)
#define STATEMENT_SIZE sizeof(stmt)
#define PARSE_RESULT_SIZE sizeof(parse_result)
#define MAX_VARIABLE_LENGTH 50

// <expression> ::= <add-expr> (('==' | '<' | '<=') <add-expr>)*
// <add-expr> ::= <mul-expr> (('+' | '-') <mul-expr>)*
// <mul-expr> ::= <atomic> (('*' | '/') <atomic>)*
// <atomic> ::= ['-' | '!'](<literal> | <variable> | <parens>)
// <parens> ::= '(' <expression> ')'

// <program> ::= <statement>';' (<statement>';')*
// <statement> ::== <assignment> | <if> | <while> | <print> | 'escape' | 'stop'
// <assignment> ::== <variable> '=' <expression>
// <if> ::== 'if (' <expression> ')' '{' <program> '}' ['else {' <program> '}']
// <while> ::== 'while (' <expression> ')' '{' <program> '}'
// <print> ::== 'print(' <expression> ')'

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

    if(e->variable != NULL) free(e->variable);

    free_expr(e->expr_1);
    free_expr(e->expr_2);
    free(e);
}

void free_stmt(stmt* s){
    if(s == NULL) return;

    if(s->variable != NULL) free(s->variable);

    free_expr(s->expression);
    free_stmt(s->next_statement);

    free(s);
}

expr* get_expr(e_type type, int val, expr* ex_1, expr* ex_2){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = type;
    expression->val = val;
    expression->variable = NULL;
    expression->expr_1 = ex_1;
    expression->expr_2 = ex_2;
    return expression;
}

expr* get_var_expr(char* name){
    expr* expression = (expr*)malloc(EXPR_SIZE);
    expression->type = Var;
    expression->val = 0;
    expression->variable = copy_of_string(name); //makes its own copy
    expression->expr_1 = NULL;
    expression->expr_2 = NULL;
    return expression;
}

stmt* get_ass_statement(char* variable, expr* expr, stmt* next_statement){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = Ass;
    statement->variable = copy_of_string(variable);
    statement->expression = expr;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_if_statement(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = If;
    statement->variable = NULL;
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = false_branch;
    return statement;
}

stmt* get_while_statement(expr* condition, stmt* true_branch, stmt* next_statement){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = While;
    statement->variable = NULL;
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_print_statement(expr* expression, stmt* next_statement){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = Print;
    statement->variable = NULL;
    statement->expression = expression;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_escape_statement(){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = Esc;
    statement->variable = NULL;
    statement->expression = NULL;
    statement->next_statement = NULL;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_stop_statement(){
    stmt* statement = (stmt*)malloc(STATEMENT_SIZE);
    statement->type = Stop;
    statement->variable = NULL;
    statement->expression = NULL;
    statement->next_statement = NULL;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
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
        dict_result var_value = dict_get(dict, expression->variable);
        if(var_value.valid) return var_value.result;

        return 0;
    }

    if(expression->type == Bool_Neg){
        int value_1 = eval_expr(expression->expr_1, dict);

        return !value_1;
    }

    if(expression->type == Eq){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 == value_2;
    }

    if(expression->type == Lt){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 < value_2;
    }

    if(expression->type == Lte){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 <= value_2;
    }

    return -1;
}

execution_result run_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, TRUE};
    if(statement == NULL) return exec;

    if(statement->type == Ass){
        int value = eval_expr(statement->expression, dict);
        dict_add(dict, statement->variable, value);
    }

    if(statement->type == Print){
        int value = eval_expr(statement->expression, dict);
        printf("%d\n", value);
    }

    if(statement->type == If){
        if(eval_expr(statement->expression, dict)){
            exec = run_statement(statement->inner_stmt_1, dict);
        }else{
            exec = run_statement(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == While){
        while(eval_expr(statement->expression, dict)){
            exec = run_statement(statement->inner_stmt_1, dict);
            if(!exec.succesful || !exec.execute_next) break;
        }
    }

    if(statement->type == Esc){
        //Do not run next statement
        return exec;
    }

    if(statement->type == Stop){
        //Do not run next statement and stop execution
        return (execution_result){TRUE, FALSE};
    }

    if(!exec.succesful || !exec.execute_next) return exec;
    return run_statement(statement->next_statement, dict);   
}

parse_result_expr parse_expr(char* str){
    parse_result_expr add_expr = parse_add_expr(str);

    if(!add_expr.valid) return INVALID_PARSE_RESULT_EXPR;

    char* remaining = add_expr.remaining;
    
    while(TRUE){
        // Get possible operator, looks very ugly
        e_type operator;
        char* next = word_follows(remaining, "==", FALSE);

        if(next == NULL){
            next = word_follows(remaining, "<=", FALSE);

            if(next == NULL){
                next = word_follows(remaining, "<", FALSE);
                if(next == NULL) break;

                operator = Lt;
            }else{
                operator = Lte;
            }
        }else{
            operator = Eq;
        }

        parse_result_expr next_add_expr = parse_add_expr(next);

        if(!next_add_expr.valid) break;

        expr* combined = get_expr(operator, 0, add_expr.expression, next_add_expr.expression);

        add_expr.expression = combined;
        add_expr.remaining = next_add_expr.remaining;
        remaining = add_expr.remaining;
    }

    return add_expr;
}

parse_result_expr parse_add_expr(char* str){
    parse_result_expr mul_expr = parse_mul_expr(str);

    if(!mul_expr.valid) return INVALID_PARSE_RESULT_EXPR;

    char* remaining = mul_expr.remaining;
    
    while(TRUE){
        token_result next = get_next_char(remaining);
        
        if(next.token != '+' && next.token != '-') break;

        parse_result_expr next_mul_expr = parse_mul_expr(next.remaining);

        if(!next_mul_expr.valid) break;

        expr* combined = get_expr(char_to_operator(next.token), 0, mul_expr.expression, next_mul_expr.expression);

        mul_expr.expression = combined;
        mul_expr.remaining = next_mul_expr.remaining;
        remaining = mul_expr.remaining;
    }

    return mul_expr;
}

parse_result_expr parse_mul_expr(char* str){
    parse_result_expr atomic = parse_atomic(str);

    if(!atomic.valid) return INVALID_PARSE_RESULT_EXPR;

    char* remaining = atomic.remaining;
    
    while(TRUE){
        token_result next = get_next_char(remaining);
        
        if(next.token != '*' && next.token != '/') break;

        parse_result_expr next_atomic = parse_atomic(next.remaining);

        if(!next_atomic.valid) break;

        expr* combined = get_expr(char_to_operator(next.token), 0, atomic.expression, next_atomic.expression);

        atomic.expression = combined;
        atomic.remaining = next_atomic.remaining;
        remaining = atomic.remaining;
    }

    return atomic;
}

parse_result_expr parse_atomic(char* str){
    token_result next = get_next_char(str);
    if(next.token == '-'){
        parse_result_expr atomic = parse_atomic(next.remaining);
        if(atomic.valid){
            expr* neg = get_expr(Neg, 0, atomic.expression, NULL);
            atomic.expression = neg;
            return atomic;
        }
    }
    if(next.token == '!'){
        parse_result_expr atomic = parse_atomic(next.remaining);
        if(atomic.valid){
            expr* bool_neg = get_expr(Bool_Neg, 0, atomic.expression, NULL);
            atomic.expression = bool_neg;
            return atomic;
        }
    }


    parse_result_expr integer = parse_integer(str);
    if(integer.valid) return integer;

    parse_result_expr variable = parse_variable(str);
    if(variable.valid) return variable;

    parse_result_expr parens = parse_parens(str);
    if(parens.valid) return parens;

    return INVALID_PARSE_RESULT_EXPR;
}


parse_result_expr parse_integer(char* str){
    str = remove_spaces(str);

    if(!is_digit(*str)) return INVALID_PARSE_RESULT_EXPR;

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
    return PARSE_RESULT_EXPR(literal, str_end);
}

parse_result_expr parse_variable(char* str){
    str = remove_spaces(str);

    if(!is_var_letter(*str)) return INVALID_PARSE_RESULT_EXPR;

    //Limit length of variable to 50 characters
    char buffer[MAX_VARIABLE_LENGTH+1];

    int index = 0;
    char* name_start = str;
    while(is_var_letter(*str) || is_digit(*str)){
        if(index >= MAX_VARIABLE_LENGTH){
            return INVALID_PARSE_RESULT_EXPR; //Variable too long
        }

        buffer[index] = name_start[index];
        index++;
        str++;
    }
    buffer[index] = 0;

    expr* var_expr = get_var_expr(buffer);
    return PARSE_RESULT_EXPR(var_expr, str);
}

parse_result_expr parse_parens(char* str){
    token_result next = get_next_char(str);
    if(next.token != '(') return INVALID_PARSE_RESULT_EXPR;

    parse_result_expr expr = parse_expr(next.remaining);
    if(!expr.valid) return INVALID_PARSE_RESULT_EXPR;

    token_result next_2 = get_next_char(expr.remaining);
    if(next_2.token != ')'){
        free_expr(expr.expression);
        return INVALID_PARSE_RESULT_EXPR;
    }

    return PARSE_RESULT_EXPR(expr.expression, next_2.remaining);
}

parse_result_stmt parse_assignment(char* str){
    parse_result_expr variable = parse_variable(str);
    if(!variable.valid) return INVALID_PARSE_RESULT_STMT;

    token_result next = get_next_char(variable.remaining);
    if(next.token != '='){
        free(variable.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_expr expression = parse_expr(next.remaining);
    if(!expression.valid){
        free(variable.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    stmt* statement = get_ass_statement(variable.expression->variable, expression.expression, NULL);
    return PARSE_RESULT_STMT(statement, expression.remaining);
}

parse_result_stmt parse_if(char* str){
    char* if_str = word_follows(str, "if", FALSE);
    if(if_str == NULL) return INVALID_PARSE_RESULT_STMT;

    token_result open_bracket = get_next_char(if_str);
    if(open_bracket.token != '(') return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression_1 = parse_expr(open_bracket.remaining);
    if(!expression_1.valid) return INVALID_PARSE_RESULT_STMT;

    token_result close_bracket = get_next_char(expression_1.remaining);
    if(close_bracket.token != ')'){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    token_result open_curly_bracket = get_next_char(close_bracket.remaining);
    if(open_curly_bracket.token != '{'){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_stmt program_1 = parse_program(open_curly_bracket.remaining);
    if(!program_1.valid){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    token_result close_curly_bracket = get_next_char(program_1.remaining);
    if(close_curly_bracket.token != '}'){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        return INVALID_PARSE_RESULT_STMT;
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_if_statement(expression_1.expression, program_1.statement, NULL, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket.remaining);

    // From here on else keyword, which is optional
    char* else_str = word_follows(close_curly_bracket.remaining, "else", FALSE);
    if(else_str == NULL) return final;

    open_curly_bracket = get_next_char(else_str);
    if(open_curly_bracket.token != '{') return final;

    parse_result_stmt program_2 = parse_program(open_curly_bracket.remaining);
    if(!program_2.valid) return final;

    close_curly_bracket = get_next_char(program_2.remaining);
    if(close_curly_bracket.token != '}'){
        free_stmt(program_2.statement);
        return final;
    }

    //else was succesful, add to previous if statement
    final_stmt->inner_stmt_2 = program_2.statement;
    final.remaining = close_curly_bracket.remaining;

    return final;
}

parse_result_stmt parse_while(char* str){
    char* while_str = word_follows(str, "while", FALSE);
    if(while_str == NULL) return INVALID_PARSE_RESULT_STMT;

    token_result open_bracket = get_next_char(while_str);
    if(open_bracket.token != '(') return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression_1 = parse_expr(open_bracket.remaining);
    if(!expression_1.valid) return INVALID_PARSE_RESULT_STMT;

    token_result close_bracket = get_next_char(expression_1.remaining);
    if(close_bracket.token != ')'){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    token_result open_curly_bracket = get_next_char(close_bracket.remaining);
    if(open_curly_bracket.token != '{'){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_stmt program_1 = parse_program(open_curly_bracket.remaining);
    if(!program_1.valid){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    token_result close_curly_bracket = get_next_char(program_1.remaining);
    if(close_curly_bracket.token != '}'){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        return INVALID_PARSE_RESULT_STMT;
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_while_statement(expression_1.expression, program_1.statement, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket.remaining);

    return final;
}

parse_result_stmt parse_print(char* str){
    char* print = word_follows(str, "print(", FALSE);
    if(print == NULL) return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression = parse_expr(print);
    if(!expression.valid) return INVALID_PARSE_RESULT_STMT;

    token_result next = get_next_char(expression.remaining);
    if(next.token != ')'){
        free_expr(expression.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    stmt* statement = get_print_statement(expression.expression, NULL);
    return PARSE_RESULT_STMT(statement, next.remaining);
}

parse_result_stmt parse_statement(char* str){
    parse_result_stmt assignment = parse_assignment(str);
    if(assignment.valid) return assignment;

    parse_result_stmt if_statement = parse_if(str);
    if(if_statement.valid) return if_statement;

    parse_result_stmt while_statement = parse_while(str);
    if(while_statement.valid) return while_statement;

    parse_result_stmt print = parse_print(str);
    if(print.valid) return print;

    char* escape = word_follows(str, "escape", FALSE);
    if(escape != NULL) return PARSE_RESULT_STMT(get_escape_statement(), escape);

    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL) return PARSE_RESULT_STMT(get_stop_statement(), stop);

    return INVALID_PARSE_RESULT_STMT;
}

parse_result_stmt parse_program(char* str){
    parse_result_stmt statement = parse_statement(str);
    if(!statement.valid) return INVALID_PARSE_RESULT_STMT;

    char* remaining = statement.remaining;
    stmt* last_statement = statement.statement; //Last statement we have parsed
    
    while(TRUE){
        token_result next = get_next_char(remaining);
        
        if(next.token != ';') break;

        parse_result_stmt next_statement = parse_statement(next.remaining);

        if(!next_statement.valid) break;

        last_statement->next_statement = next_statement.statement;
        last_statement = last_statement->next_statement;

        statement.remaining = next_statement.remaining;
        remaining = statement.remaining;
    }

    token_result last = get_next_char(remaining);
    if(last.token != ';'){
        free_stmt(statement.statement);
        return INVALID_PARSE_RESULT_STMT;
    }
    statement.remaining = last.remaining;

    return statement;
}

/*// Top level function to start parsing process
expr* str_to_expression(char* str){
    parse_result_expr result = parse_add_expr(str);
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
}*/

int execute_program(char* str){
    parse_result_stmt program = parse_program(str);
    if(!program.valid) return 1;

    char* remaining = remove_spaces(program.remaining);
    if(*remaining != 0){
        free_stmt(program.statement);
        return 1;
    }

    dictionary dict = dict_new();

    run_statement(program.statement, &dict);

    dict_free(&dict);
    free_stmt(program.statement);

    return 0;
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("No arithmetic expression has been given.\n");
        return 1;
    }

    /*dictionary dict = dict_new();
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
    dict_free(&dict);*/
    char* file_contents = file_to_string(argv[1]);
    int result = execute_program(file_contents);
    if(result != 0){
        printf("Program is invalid!.\n");
        return 1;
    }
    free(file_contents);

    return 0;
}

//TODO: escape statement for while loops, look if execution result correct, change token_result to follows_char, clean everything up