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
#define FATAL_CHECK_WITH_FREE_TYPE(f, t)if(f.fatal){free_type(t); return FATAL_PARSE_RESULT_STMT;}

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
parse_result_stmt parse_statement(char* str);
parse_result_stmt parse_multiple_statements(char* str);


static parse_result_stmt parse_assignment(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_variable_access(str);
    FATAL_CHECK(variable)
    if(!variable.valid) REVERT

    char* equals = char_follows(variable.remaining, '=', FALSE);
    if(equals == NULL){
        free_expr(variable.expression);
        REVERT
    }

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

    stmt* statement = stmt_get_ass(variable.expression->variable, expression.expression, NULL);

    variable.expression->variable = NULL; // remove since variable access still gets used in statement, if not removed it would get freed
    free_expr(variable.expression); //Free expression as we dont need it, only needed variable name
    return PARSE_RESULT_STMT(statement, last);
}

static parse_result_stmt parse_initialization(char* str){
    PARSE_INIT

    parse_result_type type = parse_type(str);
    FATAL_CHECK(type)
    if(!type.valid) REVERT

    // Check if whitespace at the end
    if(!is_whitespace(*type.remaining)){
        free_type(type.type);
        REVERT //Normal revert, since it could still be a variable name
    }

    parse_result_expr variable = parse_variable_name(type.remaining);
    FATAL_CHECK_WITH_FREE_TYPE(variable, type.type);
    if(!variable.valid){
        free_type(type.type);
        FATAL_REVERT("Expected variable name after initialization keyword.");
    }

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
    if(expression.fatal){
        free_type(type.type);
        free_expr(variable.expression);
        return FATAL_PARSE_RESULT_STMT;
    }
    if(!expression.valid){
        free_type(type.type);
        free_expr(variable.expression);
        FATAL_REVERT("Invalid expression after variable initialization.")
    }

    char* semicolon = char_follows(expression.remaining, ';', FALSE);
    if(semicolon == NULL){
        free_type(type.type);
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
    stmt* final_stmt = stmt_get_if(expression_1.expression, statement.statement, NULL, NULL);
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

    stmt* final_stmt = stmt_get_while(expression_1.expression, statement.statement, NULL);
    parse_result_stmt final = PARSE_RESULT_STMT(final_stmt, statement.remaining);

    return final;
}

static parse_result_stmt parse_return(char* str){
    PARSE_INIT

    char* return_keyword = word_follows(str, "return", FALSE);
    if(return_keyword == NULL) REVERT

    // If no whitespace, statement must end here
    if(!is_whitespace(*return_keyword)){
        char* semi_c = char_follows(return_keyword, ';', FALSE);
        if(semi_c == NULL) REVERT //Revert, since it could still be a variable name

        stmt* statement = stmt_get_plain(Stmt_Return);

        return PARSE_RESULT_STMT(statement, semi_c);
    }

    // return statement doesnt have to have a expresion
    expr* return_expression = NULL;
    char* remaining = return_keyword;

    parse_result_expr expression = parse_expr(return_keyword);
    FATAL_CHECK(expression)
    if(expression.valid){
        return_expression = expression.expression;
        remaining = expression.remaining;
    }

    char* semi_colon = char_follows(remaining, ';', FALSE);
    if(semi_colon == NULL){
        if(return_expression != NULL) free_expr(return_expression);
        FATAL_REVERT(SEMICOLON_ERROR);
    }

    stmt* statement = stmt_get_plain(Stmt_Return);
    statement->expression = return_expression;

    return PARSE_RESULT_STMT(statement, semi_colon);
}

parse_result_stmt parse_block(char* str){
    PARSE_INIT

    char* curly_bracket_open = char_follows(str, '{', FALSE);
    if(curly_bracket_open == NULL) REVERT

    parse_result_stmt statements = parse_multiple_statements(curly_bracket_open);
    FATAL_CHECK(statements)
    if(!statements.valid) FATAL_REVERT("Invalid statements inside block.")

    char* curly_bracket_close = char_follows(statements.remaining, '}', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_STMT(curly_bracket_close, CLOSING_CURLY_BRACKET_ERROR, statements.statement)

    stmt* final_statement = stmt_get_block(statements.statement, NULL);
    return PARSE_RESULT_STMT(final_statement, curly_bracket_close);
}

parse_result_stmt parse_expression_statement(char* str){
    PARSE_INIT

    parse_result_expr expression = parse_expr(str);
    FATAL_CHECK(expression)
    if(!expression.valid) REVERT

    char* semi_colon = char_follows(expression.remaining, ';', FALSE);
    if(semi_colon == NULL){
        free_expr(expression.expression);
        FATAL_REVERT(SEMICOLON_ERROR);
    }

    stmt* statement = stmt_get_plain(Stmt_Expression);
    statement->expression = expression.expression;
    return PARSE_RESULT_STMT(statement, semi_colon);
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

    parse_result_stmt ret = parse_return(str);
    FATAL_CHECK(ret)
    if(ret.valid) return ret;

    parse_result_stmt init = parse_initialization(str);
    FATAL_CHECK(init)
    if(init.valid) return init;

    // Break statement
    char* break_statement = word_follows(str, "break", FALSE);
    if(break_statement != NULL){
        char* last = char_follows(break_statement, ';', FALSE);
        if(last != NULL) return PARSE_RESULT_STMT(stmt_get_plain(Stmt_Brk), last);
        else{ // Only throw fatal error if white space follows
            if(is_whitespace(*break_statement)) FATAL_REVERT(SEMICOLON_ERROR);
        }
    }

    // Stop statement
    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL){
        char* last = char_follows(stop, ';', FALSE);
        FATAL_CHECK_NULL(last, SEMICOLON_ERROR)
        return PARSE_RESULT_STMT(stmt_get_plain(Stmt_Stop), last);
    }

    parse_result_stmt block = parse_block(str);
    FATAL_CHECK(block)
    if(block.valid) return block;

    parse_result_stmt assignment = parse_assignment(str);
    FATAL_CHECK(assignment)
    if(assignment.valid) return assignment;

    parse_result_stmt expression = parse_expression_statement(str);
    FATAL_CHECK(expression)
    if(expression.valid) return expression;

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