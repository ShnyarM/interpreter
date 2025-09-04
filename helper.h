#pragma once
#include <stdint.h>

#define TRUE 1
#define FALSE 0
typedef uint8_t BOOL;

BOOL is_digit(char c);
BOOL is_var_letter(char c);
BOOL is_whitespace(char c);
int char_to_digit(char c);
int remove_spaces(char** str);
char* copy_of_string(char* str);
char* file_to_string(char* filename);