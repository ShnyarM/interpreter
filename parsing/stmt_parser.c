#include <stdlib.h>
#include "stmt_parser.h"
#include "expr_parser.h"
#include "_parser_helper.h"

#define INVALID_PARSE_RESULT_STMT (parse_result_stmt){FALSE, NULL, NULL}
#define PARSE_RESULT_STMT(s, r) (parse_result_stmt){TRUE, s, r}

static parse_result_stmt parse_assignment(char* str);
static parse_result_stmt parse_if(char* str);
static parse_result_stmt parse_while(char* str);
static parse_result_stmt parse_print(char* str);
static parse_result_stmt parse_statement(char* str);

void free_stmt(stmt* s){
    if(s == NULL) return;

    if(s->variable != NULL) free(s->variable);

    free_expr(s->expression);
    free_stmt(s->inner_stmt_1);
    free_stmt(s->inner_stmt_2);
    free_stmt(s->next_statement);

    free(s);
}

stmt* get_ass_statement(char* variable, expr* expr, stmt* next_statement){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = Ass;
    statement->variable = copy_of_string(variable);
    statement->expression = expr;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_if_statement(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = If;
    statement->variable = NULL;
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = false_branch;
    return statement;
}

stmt* get_while_statement(expr* condition, stmt* true_branch, stmt* next_statement){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = While;
    statement->variable = NULL;
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_print_statement(expr* expression, stmt* next_statement){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = Print;
    statement->variable = NULL;
    statement->expression = expression;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

stmt* get_plain_statement(s_type type){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = type;
    statement->variable = NULL;
    statement->expression = NULL;
    statement->next_statement = NULL;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    return statement;
}

static parse_result_stmt parse_assignment(char* str){
    parse_result_expr variable = parse_variable(str);
    if(!variable.valid) return INVALID_PARSE_RESULT_STMT;

    token_result next = get_next_char(variable.remaining);
    if(next.token != '='){
        free_expr(variable.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_expr expression = parse_expr(next.remaining);
    if(!expression.valid){
        free_expr(variable.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    stmt* statement = get_ass_statement(variable.expression->variable, expression.expression, NULL);
    free_expr(variable.expression); //Free expression as we dont need it, only needed variable name
    return PARSE_RESULT_STMT(statement, expression.remaining);
}

static parse_result_stmt parse_if(char* str){
    char* if_str = word_follows(str, "if", FALSE);
    if(if_str == NULL) return INVALID_PARSE_RESULT_STMT;

    char* open_bracket = char_follows(if_str, '(', FALSE);
    if(open_bracket == NULL) return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression_1 = parse_expr(open_bracket);
    if(!expression_1.valid) return INVALID_PARSE_RESULT_STMT;

    char* close_bracket = char_follows(expression_1.remaining, ')', FALSE);
    if(close_bracket == NULL){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    char* open_curly_bracket = char_follows(close_bracket, '{', FALSE);
    if(open_curly_bracket == NULL){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_stmt program_1 = parse_program(open_curly_bracket);
    if(!program_1.valid){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    char* close_curly_bracket = char_follows(program_1.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        return INVALID_PARSE_RESULT_STMT;
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_if_statement(expression_1.expression, program_1.statement, NULL, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket);

    // From here on else keyword, which is optional
    char* else_str = word_follows(close_curly_bracket, "else", FALSE);
    if(else_str == NULL) return final;

    open_curly_bracket = char_follows(else_str, '{', FALSE);
    if(open_curly_bracket == NULL) return final;

    parse_result_stmt program_2 = parse_program(open_curly_bracket);
    if(!program_2.valid) return final;

    close_curly_bracket = char_follows(program_2.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_stmt(program_2.statement);
        return final;
    }

    //else was succesful, add to previous if statement
    final_stmt->inner_stmt_2 = program_2.statement;
    final.remaining = close_curly_bracket;

    return final;
}

static parse_result_stmt parse_while(char* str){
    char* while_str = word_follows(str, "while", FALSE);
    if(while_str == NULL) return INVALID_PARSE_RESULT_STMT;

    char* open_bracket = char_follows(while_str, '(', FALSE);
    if(open_bracket == NULL) return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression_1 = parse_expr(open_bracket);
    if(!expression_1.valid) return INVALID_PARSE_RESULT_STMT;

    char* close_bracket = char_follows(expression_1.remaining, ')', FALSE);
    if(close_bracket == NULL){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    char* open_curly_bracket = char_follows(close_bracket, '{', FALSE);
    if(open_curly_bracket == NULL){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    parse_result_stmt program_1 = parse_program(open_curly_bracket);
    if(!program_1.valid){
        free_expr(expression_1.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    char* close_curly_bracket = char_follows(program_1.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        return INVALID_PARSE_RESULT_STMT;
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_while_statement(expression_1.expression, program_1.statement, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket);

    return final;
}

static parse_result_stmt parse_print(char* str){
    char* print = word_follows(str, "print(", FALSE);
    if(print == NULL) return INVALID_PARSE_RESULT_STMT;

    parse_result_expr expression = parse_expr(print);
    if(!expression.valid) return INVALID_PARSE_RESULT_STMT;

    char* next = char_follows(expression.remaining, ')', FALSE);
    if(next == NULL){
        free_expr(expression.expression);
        return INVALID_PARSE_RESULT_STMT;
    }

    stmt* statement = get_print_statement(expression.expression, NULL);
    return PARSE_RESULT_STMT(statement, next);
}

static parse_result_stmt parse_statement(char* str){
    parse_result_stmt assignment = parse_assignment(str);
    if(assignment.valid) return assignment;

    parse_result_stmt if_statement = parse_if(str);
    if(if_statement.valid) return if_statement;

    parse_result_stmt while_statement = parse_while(str);
    if(while_statement.valid) return while_statement;

    parse_result_stmt print = parse_print(str);
    if(print.valid) return print;

    char* break_statement = word_follows(str, "break", FALSE);
    if(break_statement != NULL) return PARSE_RESULT_STMT(get_plain_statement(Brk), break_statement);

    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL) return PARSE_RESULT_STMT(get_plain_statement(Stop), stop);

    return INVALID_PARSE_RESULT_STMT;
}

parse_result_stmt parse_program(char* str){
    parse_result_stmt statement = parse_statement(str);
    if(!statement.valid) return INVALID_PARSE_RESULT_STMT;

    char* remaining = statement.remaining;
    stmt* last_statement = statement.statement; //Last statement we have parsed
    
    while(TRUE){
        char* next = char_follows(remaining, ';', FALSE);
        
        if(next == NULL) break;

        parse_result_stmt next_statement = parse_statement(next);

        if(!next_statement.valid) break;

        last_statement->next_statement = next_statement.statement;
        last_statement = last_statement->next_statement;

        statement.remaining = next_statement.remaining;
        remaining = statement.remaining;
    }

    char* last = char_follows(remaining, ';', FALSE);
    if(last == NULL){
        free_stmt(statement.statement);
        return INVALID_PARSE_RESULT_STMT;
    }
    statement.remaining = last;

    return statement;
}