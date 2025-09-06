#include "stmt_parser.h"
#include "expr_parser.h"
#include "function_parser.h"
#include "_parser_helper.h"

#define INVALID_PARSE_RESULT_FUNCTION (parse_result_function){FALSE, NULL, NULL, FALSE}
#define FATAL_PARSE_RESULT_FUNCTION (parse_result_function){FALSE, NULL, NULL, TRUE}
#define PARSE_RESULT_FUNCTION(s, r) (parse_result_function){TRUE, s, r, FALSE}

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_FUNCTION;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_FUNCTION;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_FUNCTION;}
#define FATAL_CHECK_WITH_FREE_STMT(f, s)if(f.fatal){free_stmt(s); return FATAL_PARSE_RESULT_FUNCTION;}
#define FATAL_CHECK_WITH_FREE_EXPR(f, e)if(f.fatal){free_expr(e); return FATAL_PARSE_RESULT_FUNCTION;}

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

parse_result_function parse_function(char* str){
    PARSE_INIT

    char* function_keyword = word_follows(str, "function", TRUE);
    if(function_keyword == NULL) REVERT

    parse_result_expr function_name = parse_variable_name(function_keyword);
    FATAL_CHECK(function_name)
    if(!function_name.valid) FATAL_REVERT("Expected function name after 'function' keyword.")

    char* opening_bracket = char_follows(function_name.remaining, '(', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(opening_bracket, OPENING_BRACKET_ERROR, function_name.expression);

    char* closing_bracket = char_follows(opening_bracket, ')', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(closing_bracket, CLOSING_BRACKET_ERROR, function_name.expression);

    /*char* colon = char_follows(opening_bracket, ':', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(colon, "Expected ':'", function_name.expression);

    // TO DO: add types*/

    char* opening_curly_bracket = char_follows(closing_bracket, '{', FALSE);
    FATAL_CHECK_NULL_WITH_FREE_EXPR(opening_curly_bracket, OPENING_CURLY_BRACKET_ERROR, function_name.expression);

    parse_result_stmt code = parse_multiple_statements(opening_curly_bracket);
    FATAL_CHECK_WITH_FREE_EXPR(code, function_name.expression)
    if(!code.valid){
        free_expr(function_name.expression);
        FATAL_REVERT("Invalid code inside function.")
    }

    char* closing_curly_bracket = char_follows(code.remaining, '}', FALSE);
    if(closing_curly_bracket == NULL){
        free_expr(function_name.expression);
        free_stmt(code.statement);
        FATAL_REVERT(CLOSING_CURLY_BRACKET_ERROR)
    }

    function* func = function_get(function_name.expression->variable->name, code.statement);

    free_expr(function_name.expression);

    return PARSE_RESULT_FUNCTION(func, closing_curly_bracket);
} 