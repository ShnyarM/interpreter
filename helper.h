#pragma once
#include <stdint.h>

#define TRUE 1
#define FALSE 0
typedef uint8_t BOOL;

BOOL is_digit(char c);
BOOL is_var_letter(char c);
int char_to_digit(char c);
char* remove_spaces(char* str);