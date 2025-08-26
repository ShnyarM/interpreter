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
extern char OPENING_SQUARE_BRACKET_ERROR[];
extern char CLOSING_SQUARE_BRACKET_ERROR[];
extern char SEMICOLON_ERROR[];

stmt* get_plain_statement(s_type type);
static parse_result_stmt parse_assignment(char* str);
static parse_result_stmt parse_if(char* str);
static parse_result_stmt parse_while(char* str);
static parse_result_stmt parse_print(char* str);
static parse_result_stmt parse_statement(char* str);
static parse_result_stmt parse_multiple_statements(char* str);

void free_stmt(stmt* s){
    if(s == NULL) return;

    if(s->variable != NULL) free(s->variable);

    free_expr(s->expression);
    free_expr(s->expression_2);
    free_stmt(s->inner_stmt_1);
    free_stmt(s->inner_stmt_2);
    free_stmt(s->next_statement);

    free(s);
}

stmt* get_ass_statement(char* variable, expr* expr, stmt* next_statement){
    stmt* statement = get_plain_statement(Ass);
    statement->variable = copy_of_string(variable);
    statement->expression = expr;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_arr_init_statement(char* variable, expr* size, data_type type, stmt* next_statement){
    stmt* statement = get_plain_statement(Arr_Init);
    statement->variable = copy_of_string(variable);
    statement->expression = size;
    statement->data_type = type;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_var_init_statement(char* variable, expr* value, stmt* next_statement){
    stmt* statement = get_plain_statement(Var_Init);
    statement->variable = copy_of_string(variable);
    statement->expression = value;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_if_statement(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement){
    stmt* statement = get_plain_statement(If);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = false_branch;
    return statement;
}

stmt* get_while_statement(expr* condition, stmt* true_branch, stmt* next_statement){
    stmt* statement = get_plain_statement(While);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    return statement;
}

stmt* get_print_statement(expr* expression, stmt* next_statement){
    stmt* statement = get_plain_statement(Print);
    statement->expression = expression;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_block_statement(stmt* inside, stmt* next_statement){
    stmt* statement = get_plain_statement(Block);
    statement->inner_stmt_1 = inside;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_plain_statement(s_type type){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = type;
    statement->variable = NULL;
    statement->expression = NULL;
    statement->expression_2 = NULL;
    statement->next_statement = NULL;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    statement->line = LINE_COUNTER;
    return statement;
}

static parse_result_stmt parse_assignment(char* str){
    PARSE_INIT

    parse_result_expr variable = parse_var_or_array(str);
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

    //If variable being assigned is array element, change to array assignment
    if(variable.expression->type == Arr_Access){
        statement->type = Arr_Ass;
        statement->expression_2 = variable.expression->expr_1; //Index value

        variable.expression->expr_1 = NULL; //remove reference to expression, so it doesnt get free when freeing variable
    }

    free_expr(variable.expression); //Free expression as we dont need it, only needed variable name
    return PARSE_RESULT_STMT(statement, last);
}

static parse_result_stmt parse_var_init(char* str){
    PARSE_INIT

    char* int_keyword = word_follows(str, "number", TRUE);
    if(int_keyword == NULL) REVERT

    parse_result_expr variable = parse_variable(int_keyword);
    FATAL_CHECK(variable)
    if(!variable.valid) FATAL_REVERT("Expected variable name after initialization keyword.")

    char* equals = char_follows(variable.remaining, '=', FALSE);
    if(equals == NULL){
        // No direct value assignment, create assignment which assigns 0
        char* semicolon = char_follows(variable.remaining, ';', FALSE);
        FATAL_CHECK_NULL_WITH_FREE_EXPR(semicolon, SEMICOLON_ERROR, variable.expression)

        stmt* statement = get_var_init_statement(variable.expression->variable, get_number_literal(0), NULL);
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

    stmt* statement = get_var_init_statement(variable.expression->variable, expression.expression, NULL);

    free_expr(variable.expression);
    return PARSE_RESULT_STMT(statement, semicolon);
}

static parse_result_stmt parse_arr_init(char* str){
    PARSE_INIT

    char* number_keyword = word_follows(str, "number", FALSE);
    if(number_keyword == NULL) REVERT

    char* open_square_brackets = char_follows(number_keyword, '[', FALSE);
    if(open_square_brackets == NULL) REVERT

    parse_result_expr size = parse_expr(open_square_brackets);
    FATAL_CHECK(size)
    if(!size.valid) FATAL_REVERT("Expected size of array being initialized.")

    char* close_square_brackets = char_follows(size.remaining, ']', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(close_square_brackets, CLOSING_SQUARE_BRACKET_ERROR, size.expression)

    parse_result_expr variable = parse_variable(close_square_brackets);
    FATAL_CHECK(variable)
    if(!variable.valid) FATAL_REVERT("Expected variable name after array initialization.")

    char* semicolon = char_follows(variable.remaining, ';', FALSE);
    if(semicolon == NULL){
        free_expr(size.expression);
        free_expr(variable.expression);
        if(char_follows(variable.remaining, '=', FALSE)){
            FATAL_REVERT("Arrays initializations can't have assignments.")
        }
        FATAL_REVERT(SEMICOLON_ERROR)
    }

    data_type type = NUMBER_ATOMIC_DATA_TYPE; // denotes the type of the values inside the array
    stmt* statement = get_arr_init_statement(variable.expression->variable, size.expression, type, NULL);
    free_expr(variable.expression);
    return PARSE_RESULT_STMT(statement, semicolon);
}

static parse_result_stmt parse_initialization(char* str){
    PARSE_INIT

    parse_result_stmt arr_init = parse_arr_init(str);
    FATAL_CHECK(arr_init)
    if(arr_init.valid) return arr_init;

    parse_result_stmt var_init = parse_var_init(str);
    FATAL_CHECK(var_init)
    if(var_init.valid) return var_init;

    REVERT
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

    parse_result_stmt block = parse_block(str);
    FATAL_CHECK(block)
    if(block.valid) return block;

    // Break statement
    char* break_statement = word_follows(str, "break", FALSE);
    if(break_statement != NULL){
        char* last = char_follows(break_statement, ';', FALSE);
        FATAL_CHECK_NULL(last, SEMICOLON_ERROR)
        return PARSE_RESULT_STMT(get_plain_statement(Brk), last);
    }

    // Stop statement
    char* stop = word_follows(str, "stop", FALSE);
    if(stop != NULL){
        char* last = char_follows(stop, ';', FALSE);
        FATAL_CHECK_NULL(last, SEMICOLON_ERROR)
        return PARSE_RESULT_STMT(get_plain_statement(Stop), last);
    }

    parse_result_stmt init = parse_initialization(str);
    FATAL_CHECK(init)
    if(init.valid) return init;

    parse_result_stmt assignment = parse_assignment(str);
    FATAL_CHECK(assignment)
    if(assignment.valid) return assignment;

    REVERT //Can maybe remove
}

static parse_result_stmt parse_multiple_statements(char* str){
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

parse_result_stmt parse_program(char* str){
    // No parse init here

    parse_result_stmt statements = parse_multiple_statements(str);
    FATAL_CHECK(statements)
    if(!statements.valid) FATAL_REVERT("Invalid program.");

    // check if nothing else at end
    remove_spaces(&statements.remaining);
    if(*statements.remaining != 0){
        free_stmt(statements.statement);
        FATAL_REVERT("Unknown symbol.")
    }

    return statements;
}