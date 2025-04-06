int is_digit(char c){
    if(c >= '0' && c <= '9') return 1;
    return 0;
}

int char_to_digit(char c){
    if(!is_digit(c)) return 0;

    return c-'0';
}