#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "strings.h"
#include "../helper.h"
#include "data.h"

string GLOBAL_EMPTY_STRING = (string){0, 1, "",TRUE};

string* string_new(char* content){
    string* str = (string*) malloc(sizeof(string));
    str->length = strlen(content);
    str->references = 1;
    str->c_string = copy_of_string(content);
    str->global = FALSE;
    
    return str;
}

string* string_get_encapsuled(string* s){
    string* str = (string*) malloc(sizeof(string));
    str->length = s->length+2;
    str->references = 1;
    str->global = FALSE;

    size_t buffer_size = s->length+3;
    str->c_string = (char*)malloc(buffer_size);
    str->c_string[0] = '"';
    snprintf(str->c_string+1, str->length+1, "%s", s->c_string);
    str->c_string[buffer_size-2] = '"';
    str->c_string[buffer_size-1] = 0;
    
    return str;
}

string* string_from_number(double number){
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%.15g", number);
    return string_new(buffer);
}

string* string_from_bool(BOOL b){
    char buffer[7];
    if(b) snprintf(buffer, sizeof(buffer), "%s", "true");
    else snprintf(buffer, sizeof(buffer), "%s", "false");
    return string_new(buffer);
}

string* string_from_data_array(data_array* arr){
    if(!arr->initialized){
        return string_new("[]");
    }

    size_t capacity = 128;
    size_t used = 1;
    char* buffer = malloc(capacity);

    buffer[0] = '[';

    int size = data_array_get_size(arr);
    for(int i = 0; i < size; i++){
        data_ref_access_result ref = data_array_get_reference(arr, i);
        string* data_string = string_from_data(*ref.ref, TRUE);

        if(capacity-used < data_string->length+5){
            capacity*=2;
            buffer = realloc(buffer, capacity);
        }

        snprintf(buffer+used, data_string->length+1, "%s", data_string->c_string);
        used += data_string->length;

        if(i < size-1){
            snprintf(buffer+used, 3, ", ");
            used += 2;
        }
        free_string(data_string);
    }

    snprintf(buffer+used, 2, "]");
    string* final = string_new(buffer);
    free(buffer);
    return final;
}

string* string_from_data(data d, BOOL encapsuled){
    if(data_is_of_kind_array(d)){ //If its array, print every value
        return string_from_data_array(data_get_array_from_data(d));
    }else if(data_is_empty(d)){
        return string_new("NULL");
    }else if(data_is_string_type(d)){
        if(encapsuled) return string_get_encapsuled(data_get_string_from_data(d));
        else return string_new(data_get_string_from_data(d)->c_string);
    }else if(data_is_bool_type(d)){
        return string_from_bool(GET_BOOL_FROM_DATA(d));
    }else{
        return string_from_number(GET_NUMBER_FROM_DATA(d));
    }
}

string* string_from_type(data_type* type){
    if(type==NULL) return &GLOBAL_EMPTY_STRING;
    if(types_is_invalid(type)) return string_new("INVALID");
    if(types_is_any(type)) return string_new("any");
    if(types_is_bool(type)) return string_new("bool");
    if(types_is_number(type)) return string_new("number");
    if(types_is_string(type)) return string_new("string");
    if(types_is_array(type)){
        string* inner = string_from_type(type->inner_type);
        string* brackets = string_new("[]");
        string* result = strings_concat(inner, brackets);
        free_string(inner);
        free_string(brackets);
        return result;
    }

    return &GLOBAL_EMPTY_STRING;
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

BOOL string_equality(string* s1, string* s2){
    if(strncmp(s1->c_string, s2->c_string, s1->length > s2->length ? s2->length+1 : s1->length+1) == 0) return TRUE;
    else return FALSE;
}

BOOL string_is_empty(string* str){
    return *str->c_string == 0;
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

void free_string(string* str){
    if(str == NULL) return;
    if(str->global) return;

    if(str->c_string != NULL) free(str->c_string);
    free(str);
}
