#include "type_parser.h"
#include "../structs/types.h"
#include "_parser_helper.h"

#define PARSE_RESULT_TYPE(t, r) (parse_result_type){TRUE, t, r, FALSE}
#define INVALID_PARSE_RESULT_TYPE (parse_result_type){FALSE, NULL, NULL, FALSE}
#define FATAL_PARSE_RESULT_TYPE (parse_result_type){FALSE, NULL, NULL, TRUE}

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_TYPE;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_TYPE;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_TYPE;}

extern size_t LINE_COUNTER;

//Standard error messages
extern char OPENING_BRACKET_ERROR[];
extern char CLOSING_BRACKET_ERROR[];
extern char OPENING_CURLY_BRACKET_ERROR[];
extern char CLOSING_CURLY_BRACKET_ERROR[];
extern char OPENING_SQUARE_BRACKET_ERROR[];
extern char CLOSING_SQUARE_BRACKET_ERROR[];

parse_result_type parse_atomic_type(char* str){
    PARSE_INIT

    char* number = word_follows(str, "number", FALSE);
    if(number != NULL){
        data_type* number_type = &GLOBAL_NUMBER_TYPE;
        return PARSE_RESULT_TYPE(number_type, number);
    }

    char* string = word_follows(str, "string", FALSE);
    if(string != NULL){
        data_type* string_type = &GLOBAL_STRING_TYPE;
        return PARSE_RESULT_TYPE(string_type, string);
    }

    REVERT
}

parse_result_type parse_type(char* str){
    PARSE_INIT

    parse_result_type atomic = parse_atomic_type(str);
    FATAL_CHECK(atomic);
    if(!atomic.valid) REVERT

    data_type* current_type = atomic.type;
    char* remaining = atomic.remaining;
    // HERE
    while(TRUE){
        char* open_square_brackets = char_follows(remaining, '[', FALSE);
        if(open_square_brackets == NULL) break;

        char* close_square_brackets = char_follows(open_square_brackets, ']', FALSE);
        if(close_square_brackets == NULL){
            free_type(current_type);
            FATAL_REVERT(CLOSING_SQUARE_BRACKET_ERROR);
        }

        current_type = types_get_array_type(current_type);
        remaining = close_square_brackets;
    }

    return PARSE_RESULT_TYPE(current_type, remaining);
}