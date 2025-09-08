#include "stmt_parser.h"
#include "expr_parser.h"
#include "function_parser.h"
#include "_parser_helper.h"
#include "type_parser.h"

#define INVALID_PARSE_RESULT_FUNCTION (parse_result_function){FALSE, NULL, NULL, FALSE}
#define FATAL_PARSE_RESULT_FUNCTION (parse_result_function){FALSE, NULL, NULL, TRUE}
#define PARSE_RESULT_FUNCTION(s, r) (parse_result_function){TRUE, s, r, FALSE}

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_FUNCTION;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_FUNCTION;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_FUNCTION;}
#define FATAL_CHECK_WITH_FREE_STMT(f, s)if(f.fatal){free_stmt(s); return FATAL_PARSE_RESULT_FUNCTION;}
#define FATAL_CHECK_WITH_FREE_EXPR(f, e)if(f.fatal){free_expr(e); return FATAL_PARSE_RESULT_FUNCTION;}
#define FATAL_CHECK_WITH_FREE_TYPE(f, t)if(f.fatal){free_type(t); return FATAL_PARSE_RESULT_FUNCTION;}

#define FATAL_CHECK_NULL(f, msg) if(f==NULL){FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_STMT(f, msg, s) if(f==NULL){free_stmt(s);FATAL_REVERT(msg)}
#define FATAL_CHECK_NULL_WITH_FREE_EXPR(f, msg, e) if(f==NULL){free_expr(e);FATAL_REVERT(msg)}

extern size_t LINE_COUNTER;

//Standard error messages
extern char OPENING_BRACKET_ERROR[];
extern char CLOSING_BRACKET_ERROR[];
extern char OPENING_CURLY_BRACKET_ERROR[];
extern char CLOSING_CURLY_BRACKET_ERROR[];
extern char OPENING_SQUARE_BRACKET_ERROR[];
extern char CLOSING_SQUARE_BRACKET_ERROR[];
extern char SEMICOLON_ERROR[];

// <parameter_list> ::= <type> <variable_name> (',' <type> <variable_name>)*
parse_result_function parse_parameter_list(char* str){
    PARSE_INIT

    parse_result_type first_type = parse_type(str);
    FATAL_CHECK(first_type);
    if(!first_type.valid) REVERT;

    if(!is_whitespace(*first_type.remaining)){
        free_type(first_type.type);
        REVERT;
    }

    parse_result_expr first_name = parse_variable_name(first_type.remaining);
    FATAL_CHECK_WITH_FREE_TYPE(first_name, first_type.type);
    if(!first_name.valid){
        free_type(first_type.type);
        FATAL_REVERT("Expected variable name after type declaration in parameter list.");
    }

    function_parameter* first = function_parameter_get(first_name.expression->variable->name, first_type.type);
    function_parameter* last = first;
    char* remaining = first_name.remaining;
    free_expr(first_name.expression); // Not needed anymore, as variable name string has been copied into first->name

    while(TRUE){
        char* comma = char_follows(remaining, ',', FALSE);
        if(comma == NULL) break;

        parse_result_type type = parse_type(comma);
        if(type.fatal){
            free_function_parameter(first);
            return FATAL_PARSE_RESULT_FUNCTION;
        }
        if(!type.valid || !is_whitespace(*type.remaining)){
            free_function_parameter(first);
            free_type(type.type);
            FATAL_REVERT("Expected type declaration after ',' in parameter list.");
        }

        parse_result_expr name = parse_variable_name(type.remaining);
        if(name.fatal){
            free_function_parameter(first);
            free_type(type.type);
            return FATAL_PARSE_RESULT_FUNCTION;
        }
        if(!name.valid){
            free_function_parameter(first);
            free_type(type.type);
            FATAL_REVERT("Expected variable name after type declaration in parameter list.");
        }

        function_parameter* parameter = function_parameter_get(name.expression->variable->name, type.type);
        last->next = parameter;
        last = parameter;
        remaining = name.remaining;

        free_expr(name.expression);
    }

    // create a function which encapsulates result
    function* func = function_get_plain();
    func->parameters = first;

    return PARSE_RESULT_FUNCTION(func, remaining);
}

parse_result_function parse_function(char* str){
    PARSE_INIT

    char* function_keyword = word_follows(str, "function", TRUE);
    if(function_keyword == NULL) REVERT

    parse_result_expr function_name = parse_variable_name(function_keyword);
    FATAL_CHECK(function_name)
    if(!function_name.valid) FATAL_REVERT("Expected function name after 'function' keyword.")

    char* opening_bracket = char_follows(function_name.remaining, '(', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(opening_bracket, OPENING_BRACKET_ERROR, function_name.expression);

    function_parameter* parameters = NULL;
    char* r = opening_bracket;

    parse_result_function parameter_parse = parse_parameter_list(opening_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(parameter_parse, function_name.expression);
    if(parameter_parse.valid){
        parameters = parameter_parse.function->parameters;
        parameter_parse.function->parameters = NULL;
        free_function(parameter_parse.function); // Not needed anymore, parameters have been extracted

        r = parameter_parse.remaining;
    }

    char* closing_bracket = char_follows(r, ')', FALSE);
    if(closing_bracket == NULL){
        free_expr(function_name.expression);
        free_function_parameter(parameters);
        FATAL_REVERT(CLOSING_BRACKET_ERROR);
    }

    data_type* return_type = NULL;
    char* remaining = NULL;

    char* colon = char_follows(closing_bracket, ':', FALSE);
    if(colon != NULL){
        parse_result_type type = parse_type(colon);
        if(type.fatal){
            free_expr(function_name.expression);
            free_function_parameter(parameters);
            return FATAL_PARSE_RESULT_FUNCTION;
        }
        if(!type.valid){
            free_expr(function_name.expression);
            free_function_parameter(parameters);
            FATAL_REVERT("Expected return type declaration after ':' keyword in function declaration.");
        }

        return_type = type.type;
        remaining =  type.remaining;
    }else{
        return_type = &GLOBAL_ANY_TYPE; // No return type declaration exists, so function can return any type
        remaining = closing_bracket;
    }

    parse_result_stmt block = parse_block(remaining);
    if(block.fatal){
        free_expr(function_name.expression);
        free_function_parameter(parameters);
        free_type(return_type);
        return FATAL_PARSE_RESULT_FUNCTION;
    }
    if(!block.valid){
        free_expr(function_name.expression);
        free_function_parameter(parameters);
        free_type(return_type);
        FATAL_REVERT(OPENING_CURLY_BRACKET_ERROR);
    }

    function* func = function_get(function_name.expression->variable->name, return_type, block.statement->inner_stmt_1, parameters);

    block.statement->inner_stmt_1 = NULL; // remove ownership
    free_stmt(block.statement);
    free_expr(function_name.expression);

    return PARSE_RESULT_FUNCTION(func, block.remaining);
} 