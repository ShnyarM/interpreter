#include <stdlib.h>
#include "stmt_parser.h"
#include "expr_parser.h"
#include "_parser_helper.h"

extern size_t LINE_COUNTER;

#define INVALID_PARSE_RESULT_STMT (parse_result_stmt){FALSE, NULL, NULL, FALSE}
#define FATAL_PARSE_RESULT_STMT (parse_result_stmt){FALSE, NULL, NULL, TRUE}
#define PARSE_RESULT_STMT(s, r) (parse_result_stmt){TRUE, s, r, FALSE}

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_STMT;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_STMT;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_STMT;}
#define FATAL_CHECK_WITH_FREE_STMT(f, s)if(f.fatal){free_stmt(s); return FATAL_PARSE_RESULT_STMT;}
#define FATAL_CHECK_WITH_FREE_EXPR(f, e)if(f.fatal){free_expr(e); return FATAL_PARSE_RESULT_STMT;}

#define FATAL_CHECK_NULL(f, msg) if(f==NULL){FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_STMT(f, msg, s) if(f==NULL){free_stmt(s);FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_EXPR(f, msg, e) if(f==NULL){free_expr(e);FATAL_REVERT(msg)}

//Standard error messages
extern char OPENING_BRACKET_ERROR[];
extern char CLOSING_BRACKET_ERROR[];
extern char OPENING_CURLY_BRACKET_ERROR[];
extern char CLOSING_CURLY_BRACKET_ERROR[];

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
    PARSE_INIT

    parse_result_expr variable = parse_variable(str);
    FATAL_CHECK(variable)
    if(!variable.valid) REVERT

    /*token_result next = get_next_char(variable.remaining);
    if(next.token != '='){
        free_expr(variable.expression);
        return INVALID_PARSE_RESULT_STMT;
    }*/

    char* equals = char_follows(variable.remaining, '=', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(equals, "Expected equality operator '=' after variable name.", variable.expression)

    parse_result_expr expression = parse_expr(equals);
    FATAL_CHECK_WITH_FREE_EXPR(expression, variable.expression)
    if(!expression.valid){
        free_expr(variable.expression);
        FATAL_REVERT("Invalid expression for variable assignment.")
    }

    stmt* statement = get_ass_statement(variable.expression->variable, expression.expression, NULL);
    free_expr(variable.expression); //Free expression as we dont need it, only needed variable name
    return PARSE_RESULT_STMT(statement, expression.remaining);
}

static parse_result_stmt parse_else(char* str){
    PARSE_INIT

    // From here on else keyword, which is optional
    char* else_str = word_follows(str, "else", FALSE);
    if(else_str == NULL) REVERT

    char* open_curly_bracket = char_follows(else_str, '{', FALSE);
    FATAL_CHECK_NULL(open_curly_bracket, OPENING_CURLY_BRACKET_ERROR)

    parse_result_stmt program_2 = parse_program(open_curly_bracket);
    FATAL_CHECK(program_2)
    if(!program_2.valid) FATAL_REVERT("Invalid statements inside else block.")

    char* close_curly_bracket = char_follows(program_2.remaining, '}', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_STMT(close_curly_bracket, CLOSING_CURLY_BRACKET_ERROR, program_2.statement)

    program_2.remaining = close_curly_bracket;
    return program_2;
}

static parse_result_stmt parse_if(char* str){
    PARSE_INIT

    char* if_str = word_follows(str, "if", FALSE);
    if(if_str == NULL) REVERT

    char* open_bracket = char_follows(if_str, '(', FALSE);
    FATAL_CHECK_NULL(OPENING_BRACKET_ERROR, OPENING_BRACKET_ERROR)

    parse_result_expr expression_1 = parse_expr(open_bracket);
    FATAL_CHECK(expression_1)
    if(!expression_1.valid) FATAL_REVERT("Invalid expression in if statement.")

    char* close_bracket = char_follows(expression_1.remaining, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(close_bracket, CLOSING_BRACKET_ERROR, expression_1.expression)

    char* open_curly_bracket = char_follows(close_bracket, '{', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(open_curly_bracket, OPENING_CURLY_BRACKET_ERROR, expression_1.expression)

    parse_result_stmt program_1 = parse_program(open_curly_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(program_1, expression_1.expression)
    if(!program_1.valid){
        free_expr(expression_1.expression);
        FATAL_REVERT("Invalid statements inside if block.")
    }

    char* close_curly_bracket = char_follows(program_1.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        FATAL_REVERT(CLOSING_CURLY_BRACKET_ERROR)
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_if_statement(expression_1.expression, program_1.statement, NULL, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket);

    //try to get else statement
    parse_result_stmt else_statement = parse_else(close_curly_bracket);
    FATAL_CHECK_WITH_FREE_STMT(else_statement, final_stmt)
    if(!else_statement.valid) return final;



    // From here on else keyword, which is optional
    /*char* else_str = word_follows(close_curly_bracket, "else", FALSE);
    if(else_str == NULL) return final;

    open_curly_bracket = char_follows(else_str, '{', FALSE);
    if(open_curly_bracket == NULL) return final;

    parse_result_stmt program_2 = parse_program(open_curly_bracket);
    if(!program_2.valid) return final;

    close_curly_bracket = char_follows(program_2.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_stmt(program_2.statement);
        return final;
    }*/

    //else was succesful, add to previous if statement
    final_stmt->inner_stmt_2 = else_statement.statement;
    final.remaining = else_statement.remaining;

    return final;
}

static parse_result_stmt parse_while(char* str){
    PARSE_INIT

    char* while_str = word_follows(str, "while", FALSE);
    if(while_str == NULL) REVERT

    char* open_bracket = char_follows(while_str, '(', FALSE);
    FATAL_CHECK_NULL(open_bracket, OPENING_BRACKET_ERROR)

    parse_result_expr expression_1 = parse_expr(open_bracket);
    FATAL_CHECK(expression_1)
    if(!expression_1.valid) FATAL_REVERT("Invalid expression for while statement.")

    char* close_bracket = char_follows(expression_1.remaining, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(close_bracket, CLOSING_BRACKET_ERROR, expression_1.expression)

    char* open_curly_bracket = char_follows(close_bracket, '{', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(open_curly_bracket, OPENING_CURLY_BRACKET_ERROR, expression_1.expression)

    parse_result_stmt program_1 = parse_program(open_curly_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(program_1, expression_1.expression)
    if(!program_1.valid){
        free_expr(expression_1.expression);
        FATAL_REVERT("Invalid statements inside while block.")
    }

    char* close_curly_bracket = char_follows(program_1.remaining, '}', FALSE);
    if(close_curly_bracket == NULL){
        free_expr(expression_1.expression);
        free_stmt(program_1.statement);
        FATAL_REVERT("Expected closing curly bracket '}'")
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_while_statement(expression_1.expression, program_1.statement, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, close_curly_bracket);

    return final;
}

static parse_result_stmt parse_print(char* str){
    PARSE_INIT

    char* print = word_follows(str, "print(", FALSE);
    if(print == NULL) REVERT

    parse_result_expr expression = parse_expr(print);
    FATAL_CHECK(expression)
    if(!expression.valid) REVERT

    char* next = char_follows(expression.remaining, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(next, CLOSING_BRACKET_ERROR, expression.expression)

    stmt* statement = get_print_statement(expression.expression, NULL);
    return PARSE_RESULT_STMT(statement, next);
}

static parse_result_stmt parse_statement(char* str){
    PARSE_INIT

    parse_result_stmt if_statement = parse_if(str);
    FATAL_CHECK(if_statement)
    if(if_statement.valid) return if_statement;

    parse_result_stmt while_statement = parse_while(str);
    FATAL_CHECK(while_statement)
    if(while_statement.valid) return while_statement;

    parse_result_stmt print = parse_print(str);
    FATAL_CHECK(print)
    if(print.valid) return print;

    char* break_statement = word_follows(str, "break", FALSE);
    if(break_statement != NULL) return PARSE_RESULT_STMT(get_plain_statement(Brk), break_statement);

    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL) return PARSE_RESULT_STMT(get_plain_statement(Stop), stop);

    parse_result_stmt assignment = parse_assignment(str);
    FATAL_CHECK(assignment)
    if(assignment.valid) return assignment;

    REVERT //Can maybe remove
}

parse_result_stmt parse_program(char* str){
    PARSE_INIT

    parse_result_stmt statement = parse_statement(str);
    FATAL_CHECK(statement)
    if(!statement.valid) REVERT

    char* remaining = statement.remaining;
    stmt* last_statement = statement.statement; //Last statement we have parsed
    
    while(TRUE){
        size_t counter_before = LINE_COUNTER;
        char* next = char_follows(remaining, ';', FALSE);
        
        if(next == NULL){
            LINE_COUNTER = counter_before;
            break;
        }

        parse_result_stmt next_statement = parse_statement(next);
        FATAL_CHECK_WITH_FREE_STMT(next_statement, statement.statement)
        if(!next_statement.valid){
            LINE_COUNTER = counter_before;
            break;
        }

        last_statement->next_statement = next_statement.statement;
        last_statement = last_statement->next_statement;

        statement.remaining = next_statement.remaining;
        remaining = statement.remaining;
    }

    char* last = char_follows(remaining, ';', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_STMT(last, "Semicolon missing.", statement.statement)

    statement.remaining = last;

    return statement;
}