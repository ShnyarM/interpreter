#include "helper.h"

BOOL is_digit(char c){
    if(c >= '0' && c <= '9') return TRUE;
    return FALSE;
}

BOOL is_var_letter(char c){
    if(c >= 'A' && c <= 'Z') return TRUE;
    if(c >= 'a' && c <= 'z') return TRUE;
    if(c == '_') return TRUE;
    return FALSE;
}

int char_to_digit(char c){
    if(!is_digit(c)) return 0;

    return c-'0';
}

char* remove_spaces(char* str){
    while(*str == ' ') str++;
    return str;
}