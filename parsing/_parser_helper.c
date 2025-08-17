#include <string.h>
#include <stdio.h>
#include "_parser_helper.h"
#include "../helper.h"
#include "../types.h"

#define PARSE_ERROR_MESSAGE_LENGTH 100

size_t LINE_COUNTER = 1;

char PARSE_ERROR_MESSAGE[PARSE_ERROR_MESSAGE_LENGTH];

//Standard error messages
char OPENING_BRACKET_ERROR[] = "Expected opening bracket '('.";
char CLOSING_BRACKET_ERROR[] = "Expected closing bracket ')'.";
char OPENING_CURLY_BRACKET_ERROR[] = "Expected opening curly bracket '{'.";
char CLOSING_CURLY_BRACKET_ERROR[] = "Expected closing curly bracket '}'.";
char OPENING_SQUARE_BRACKET_ERROR[] = "Expected opening square bracket '['.";
char CLOSING_SQUARE_BRACKET_ERROR[] = "Expected closing square bracket ']'.";
char SEMICOLON_ERROR[] = "Semicolon missing.";

#define RESERVED_KEYWORDS_LENGTH 7
char RESERVED_KEYWORDS[RESERVED_KEYWORDS_LENGTH][10] = {"if", "else", "while", "print", "break", "stop", "number"};

//MAJOR POINT FOR POTENTIAL LINE ERROR
token_result get_next_char(char* str){
    size_t line = LINE_COUNTER;
    LINE_COUNTER += remove_spaces(&str);
    return (token_result){*str, str+1, line};
}

e_type char_to_operator(char c){
    if(c == '+') return Add;
    if(c == '-') return Sub;
    if(c == '*') return Mul;
    if(c == '/') return Div;
    return Add;
}

// If char doesn't follow, then return NULL, else return pointer after char
// updates LINE_COUNTER if succesful
char* char_follows(char* str, char c, BOOL must_space_follow){
    size_t new_line_counter = LINE_COUNTER;
    new_line_counter += remove_spaces(&str);

    if(*str != c) return NULL;
    if(must_space_follow && (str[1] != ' ' && str[1] != '\n' && str[1] != 0)) return NULL;

    LINE_COUNTER = new_line_counter;
    return str+1;
}

// If word doesn't follow return NULL, else it returns point where it stopped
// updates LINE_COUNTER if succesful
char* word_follows(char* str, char* word, BOOL must_space_follow){
    size_t new_line_counter = LINE_COUNTER;
    new_line_counter += remove_spaces(&str);

    int i = 0;
    while(word[i] != 0){
        if(str[i] != word[i]) return NULL;
        i++;
    }
    if(must_space_follow && (str[i] != ' ' && str[i] != '\n' && str[i] != 0)) return NULL;
    
    LINE_COUNTER = new_line_counter;
    return str+i;
}

BOOL is_reserved_keyword(char* str){
    for(int i = 0; i < RESERVED_KEYWORDS_LENGTH; i++){
        if(strcmp(RESERVED_KEYWORDS[i], str) == 0) return TRUE;
    }

    return FALSE;
}

void write_parse_error(char* message){
    if(*message == 0) return;

    char int_buffer[20];

    strncpy(PARSE_ERROR_MESSAGE, "Error during parsing on line ", PARSE_ERROR_MESSAGE_LENGTH);
    sprintf(int_buffer, "%ld", LINE_COUNTER);
    strcat(PARSE_ERROR_MESSAGE, int_buffer);
    strcat(PARSE_ERROR_MESSAGE, ": ");
    strcat(PARSE_ERROR_MESSAGE, message);
}