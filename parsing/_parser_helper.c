#include "_parser_helper.h"
#include "../helper.h"
#include "../types.h"

token_result get_next_char(char* str){
    str = remove_spaces(str);
    return (token_result){*str, str+1};
}

e_type char_to_operator(char c){
    if(c == '+') return Add;
    if(c == '-') return Sub;
    if(c == '*') return Mul;
    if(c == '/') return Div;
    return Add;
}