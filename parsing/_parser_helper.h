#pragma once

#include "../helper.h"
#include "../types.h"

typedef struct token_result_t{
    char token;
    char* remaining;
} token_result;

token_result get_next_char(char* str);
e_type char_to_operator(char c);