#pragma once

#include "../helper.h"
#include "../general_structs.h"

typedef struct token_result_t{
    char token;
    char* remaining;
    size_t line_counter_before;
} token_result;

token_result get_next_char(char* str);
e_type char_to_operator(char c);
char* char_follows(char* str, char c, BOOL must_space_follow);
char* word_follows(char* str, char* word, BOOL must_space_follow);
BOOL is_reserved_keyword(char* str);
void write_parse_error(char* message);