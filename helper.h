#pragma once
#include <stdint.h>

#define TRUE 1
#define FALSE 0
typedef uint8_t BOOL;

BOOL is_digit(char c);
BOOL is_var_letter(char c);
char* word_follows(char* str, char* word);
int char_to_digit(char c);
char* remove_spaces(char* str);
char* copy_of_string(char* str);
char* file_to_string(char* filename);