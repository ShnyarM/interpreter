#include "type_parser.h"
#include "types.h"
#include "_parser_helper.h"

#define PARSE_RESULT_TYPE(t, r) (parse_result_type){TRUE, t, r, FALSE}
#define INVALID_PARSE_RESULT_TYPE (parse_result_type){FALSE, NULL, NULL, FALSE}
#define FATAL_PARSE_RESULT_TYPE (parse_result_type){FALSE, NULL, NULL, TRUE}

#define PARSE_INIT size_t ORIG_LINE_COUNT = LINE_COUNTER;
#define REVERT {LINE_COUNTER = ORIG_LINE_COUNT;return INVALID_PARSE_RESULT_TYPE;}
#define FATAL_REVERT(reason) {write_parse_error(reason); return FATAL_PARSE_RESULT_TYPE;}

#define FATAL_CHECK(f) if(f.fatal){return FATAL_PARSE_RESULT_TYPE;}

extern size_t LINE_COUNTER;

parse_result_type parse_atomic_type(char* str){
    PARSE_INIT

    char* number = word_follows(str, "number", FALSE);
    if(number != NULL){
        data_type* number_type = get_atomic_number_type();
        return PARSE_RESULT_TYPE(number_type, number);
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
        char* brackets = word_follows(remaining, "[]", FALSE);
        if(brackets == NULL) break;

        current_type = get_array_type(current_type);
        remaining = brackets;
    }
    
    // Check if whitespace at the end
    if(!is_whitespace(*remaining)){
        free_type(current_type);
        FATAL_REVERT("Unexpected type ending. Expected whitespace.");
    }

    return PARSE_RESULT_TYPE(current_type, remaining);
}