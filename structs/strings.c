#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strings.h"
#include "../helper.h"

string GLOBAL_EMPTY_STRING = (string){0, 1, "",TRUE};

string* string_new(char* content){
    string* str = (string*) malloc(sizeof(string));
    str->length = strlen(content);
    str->references = 1;
    str->c_string = copy_of_string(content);
    str->global = FALSE;
    
    return str;
}

string* string_convert_from_number(double number){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%g", number);
    return string_new(buffer);
}

// Creates new string which is concatenation of both strings
string* strings_concat(string* str1, string* str2){
    int length = str1->length + str2->length;

    string* str = (string*) malloc(sizeof(string));
    str->length = length;
    str->references = 1;
    str->global = FALSE;

    str->c_string = malloc(length+1);
    strncpy(str->c_string, str1->c_string, length+1);
    strncat(str->c_string, str2->c_string, str2->length);
    
    return str;
}

void string_add_reference(string* str){
    if(str->global) return;
    str->references++;
}

void string_remove_reference(string* str){
    if(str->global) return;
    str->references--;
    if(str->references <= 0){
        free_string(str);
    }
}

void string_print(string* str){
    printf("%s", str->c_string);
}

void free_string(string* str){
    if(str == NULL) return;
    if(str->global) return;

    if(str->c_string != NULL) free(str->c_string);
    free(str);
}
