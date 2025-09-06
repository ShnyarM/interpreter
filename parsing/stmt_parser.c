/*#include <stdlib.h>
#include "stmt_parser.h"
#include "expr_parser.h"
#include "function_parser.h"
#include "_parser_helper.h"*/

#include "stmt_parser.h"
#include "expr_parser.h"
#include "type_parser.h"
#include "../structs/statement.h"
#include "../structs/expression.h"
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
extern char OPENING_SQUARE_BRACKET_ERROR[];
extern char CLOSING_SQUARE_BRACKET_ERROR[];
extern char SEMICOLON_ERROR[];

static parse_result_stmt parse_assignment(char* str);
static parse_result_stmt parse_if(char* str);
static parse_result_stmt parse_while(char* str);
static parse_result_stmt parse_print(char* str);
parse_result_stmt parse_statement(char* str);
parse_result_stmt parse_multiple_statements(char* str);


static parse_result_stmt parse_assignment(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_variable_access(str);
    FATAL_CHECK(variable)
    if(!variable.valid) REVERT

    char* equals = char_follows(variable.remaining, '=', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(equals, "Expected equality operator '=' after variable name.", variable.expression)

    parse_result_expr expression = parse_expr(equals);
    FATAL_CHECK_WITH_FREE_EXPR(expression, variable.expression)
    if(!expression.valid){
        free_expr(variable.expression);
        FATAL_REVERT("Invalid expression for variable assignment.")
    }

    char* last = char_follows(expression.remaining, ';', FALSE);
    if(last == NULL){
        free_expr(variable.expression);
        free_expr(expression.expression);
        FATAL_REVERT(SEMICOLON_ERROR)
    }

    stmt* statement = get_ass_statement(variable.expression->variable, expression.expression, NULL);

    variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
    free_expr(variable.expression); //Free expression as we dont need it, only needed variable name
    return PARSE_RESULT_STMT(statement, last);
}

static parse_result_stmt parse_initialization(char* str){
    PARSE_INIT

    parse_result_type type = parse_type(str);
    FATAL_CHECK(type)
    if(!type.valid) REVERT

    parse_result_expr variable = parse_variable_name(type.remaining);
    FATAL_CHECK(variable)
    if(!variable.valid) FATAL_REVERT("Expected variable name after initialization keyword.")

    char* equals = char_follows(variable.remaining, '=', FALSE);
    if(equals == NULL){
        // No direct value assignment, create assignment which assigns 0
        char* semicolon = char_follows(variable.remaining, ';', FALSE);
        FATAL_CHECK_NULL_WITH_FREE_EXPR(semicolon, SEMICOLON_ERROR, variable.expression)

        stmt* statement = stmt_get_init_stmt(variable.expression->variable, NULL, type.type, NULL);

        variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
        free_expr(variable.expression);
        return PARSE_RESULT_STMT(statement, semicolon);
    }

    parse_result_expr expression = parse_expr(equals);
    FATAL_CHECK(expression)
    if(!expression.valid){
        free_expr(variable.expression);
        FATAL_REVERT("Invalid expression after variable initialization.")
    }

    char* semicolon = char_follows(expression.remaining, ';', FALSE);
    if(semicolon == NULL){
        free_expr(variable.expression);
        free_expr(expression.expression);
        FATAL_REVERT(SEMICOLON_ERROR)
    }

    stmt* statement = stmt_get_init_stmt(variable.expression->variable, expression.expression, type.type, NULL);

    variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
    free_expr(variable.expression);
    return PARSE_RESULT_STMT(statement, semicolon);
}

static parse_result_stmt parse_else(char* str){
    PARSE_INIT

    // From here on else keyword, which is optional
    char* else_str = word_follows(str, "else", FALSE);
    if(else_str == NULL) REVERT

    parse_result_stmt statement = parse_statement(else_str);
    FATAL_CHECK(statement)
    if(!statement.valid) FATAL_REVERT("Invalid statement inside else block.")

    return statement;
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

    parse_result_stmt statement = parse_statement(close_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(statement, expression_1.expression)
    if(!statement.valid){
        free_expr(expression_1.expression);
        FATAL_REVERT("Invalid statement inside if block.")
    }

    // construct statement which will be returned if else fails
    stmt* final_stmt = get_if_statement(expression_1.expression, statement.statement, NULL, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, statement.remaining);

    //try to get else statement
    parse_result_stmt else_statement = parse_else(statement.remaining);
    FATAL_CHECK_WITH_FREE_STMT(else_statement, final_stmt)
    if(!else_statement.valid) return final;

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

    parse_result_stmt statement = parse_statement(close_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(statement, expression_1.expression)
    if(!statement.valid){
        free_expr(expression_1.expression);
        FATAL_REVERT("Invalid statement inside while block.")
    }

    stmt* final_stmt = get_while_statement(expression_1.expression, statement.statement, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, statement.remaining);

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

    char* last = char_follows(next, ';', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(last, SEMICOLON_ERROR, expression.expression)

    stmt* statement = get_print_statement(expression.expression, NULL);
    return PARSE_RESULT_STMT(statement, last);
}

static parse_result_stmt parse_statement_function_call(char* str){
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

    char* closing_bracket = char_follows(opening_bracket, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(closing_bracket, CLOSING_BRACKET_ERROR, variable.expression);

    char* semi_colon = char_follows(closing_bracket, ';', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(semi_colon, SEMICOLON_ERROR, variable.expression);

    stmt* stmt = stmt_get_plain(Function_Call);
    stmt->variable = variable.expression->variable;
    
    variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
    free_expr(variable.expression);
    return PARSE_RESULT_STMT(stmt, semi_colon);
}

static parse_result_stmt parse_block(char* str){
    PARSE_INIT

    char* curly_bracket_open = char_follows(str, '{', FALSE);
    if(curly_bracket_open == NULL) REVERT

    parse_result_stmt statements = parse_multiple_statements(curly_bracket_open);
    FATAL_CHECK(statements)
    if(!statements.valid) FATAL_REVERT("Invalid statements inside block.")

    char* curly_bracket_close = char_follows(statements.remaining, '}', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_STMT(curly_bracket_close, CLOSING_CURLY_BRACKET_ERROR, statements.statement)

    stmt* final_statement = get_block_statement(statements.statement, NULL);
    return PARSE_RESULT_STMT(final_statement, curly_bracket_close);
}

parse_result_stmt parse_statement(char* str){
    PARSE_INIT

    if(word_follows(str, "function", TRUE)) REVERT

    parse_result_stmt if_statement = parse_if(str);
    FATAL_CHECK(if_statement)
    if(if_statement.valid) return if_statement;

    parse_result_stmt while_statement = parse_while(str);
    FATAL_CHECK(while_statement)
    if(while_statement.valid) return while_statement;

    parse_result_stmt print = parse_print(str);
    FATAL_CHECK(print)
    if(print.valid) return print;

    parse_result_stmt function_call = parse_statement_function_call(str);
    FATAL_CHECK(function_call)
    if(function_call.valid) return function_call;

    parse_result_stmt block = parse_block(str);
    FATAL_CHECK(block)
    if(block.valid) return block;

    // Break statement
    char* break_statement = word_follows(str, "break", FALSE);
    if(break_statement != NULL){
        char* last = char_follows(break_statement, ';', FALSE);
        FATAL_CHECK_NULL(last, SEMICOLON_ERROR)
        return PARSE_RESULT_STMT(stmt_get_plain(Brk), last);
    }

    // Stop statement
    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL){
        char* last = char_follows(stop, ';', FALSE);
        FATAL_CHECK_NULL(last, SEMICOLON_ERROR)
        return PARSE_RESULT_STMT(stmt_get_plain(Stop), last);
    }

    parse_result_stmt init = parse_initialization(str);
    FATAL_CHECK(init)
    if(init.valid) return init;

    parse_result_stmt assignment = parse_assignment(str);
    FATAL_CHECK(assignment)
    if(assignment.valid) return assignment;

    REVERT //Can maybe remove
}

parse_result_stmt parse_multiple_statements(char* str){
    PARSE_INIT

    parse_result_stmt statement = parse_statement(str);
    FATAL_CHECK(statement)
    if(!statement.valid) REVERT

    char* remaining = statement.remaining;
    stmt* last_statement = statement.statement; //Last statement we have parsed
    
    while(TRUE){
        size_t counter_before = LINE_COUNTER;

        parse_result_stmt next_statement = parse_statement(remaining);
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

    statement.remaining = remaining;

    return statement;
}