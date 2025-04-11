#include <string.h>
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

// If char doesn't follow, then return NULL, else return pointer after char
char* char_follows(char* str, char c, BOOL must_space_follow){
    str = remove_spaces(str);
    if(*str != c) return NULL;
    if(must_space_follow && (str[1] != ' ' && str[1] != '\n' && str[1] != 0)) return NULL;

    return str+1;
}

// If word doesn't follow return NULL, else it returns point where it stopped
char* word_follows(char* str, char* word, BOOL must_space_follow){
    str = remove_spaces(str);

    int i = 0;
    while(word[i] != 0){
        if(str[i] != word[i]) return NULL;
        i++;
    }
    if(must_space_follow && (str[i] != ' ' && str[i] != '\n' && str[i] != 0)) return NULL;
    
    return str+i;
}

int char_to_digit(char c){
    if(!is_digit(c)) return 0;

    return c-'0';
}

char* remove_spaces(char* str){
    while(*str == ' ' || *str == '\n') str++;
    return str;
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
    }
    buffer[length] = 0;

    return buffer;
}