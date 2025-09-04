#include <stdlib.h>
#include <stdio.h>
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

BOOL is_whitespace(char c){
    if(c == ' ') return TRUE;
    if(c == '\n') return TRUE;
    if(c == '\t') return TRUE;
    return FALSE;
}

int char_to_digit(char c){
    if(!is_digit(c)) return 0;

    return c-'0';
}

// removes spaces from string and returns amount of new lines skipped
int remove_spaces(char** str){
    int new_line_counter = 0;
    while(**str == ' ' || **str == '\n'){
        if(**str == '\n') new_line_counter++;
        (*str)++;
    }
    return new_line_counter;
}

// Creates a copy of a string by dynamically allocating space
char* copy_of_string(char* str){
    size_t len = strlen(str);
    char* new_str = (char*)malloc(len+1);
    strncpy(new_str, str, len+1);
    return new_str;
}

char* file_to_string(char* filename){
    char* buffer = 0;
    long length;
    FILE* f = fopen (filename, "rb");
    
    if (f)
    {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = malloc (length+1);
        if (buffer){
          fread (buffer, 1, length, f);
        }
        fclose (f);
    }else{
        return NULL;
    }
    buffer[length] = 0;

    return buffer;
}