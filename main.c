#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "helper.h"
#include "parsing/stmt_parser.h"
#include "executing/executor.h"
#include "dictionary.h"

size_t MAX_VARIABLE_LENGTH = 50;

// <expression> ::= <add-expr> (('==' | '<' | '<=') <add-expr>)*
// <add-expr> ::= <mul-expr> (('+' | '-') <mul-expr>)*
// <mul-expr> ::= <atomic> (('*' | '/') <atomic>)*
// <atomic> ::= ['-' | '!'](<literal> | <variable> | <parens>)
// <parens> ::= '(' <expression> ')'

// <program> ::= <statement>';' (<statement>';')*
// <statement> ::== <assignment> | <if> | <while> | <print> | 'break' | 'stop'
// <assignment> ::== <variable> '=' <expression>
// <if> ::== 'if (' <expression> ')' '{' <program> '}' ['else {' <program> '}']
// <while> ::== 'while (' <expression> ')' '{' <program> '}'
// <print> ::== 'print(' <expression> ')'

int execute_program(char* str){
    parse_result_stmt program = parse_program(str);
    if(!program.valid) return 1;

    char* remaining = remove_spaces(program.remaining);
    if(*remaining != 0){
        free_stmt(program.statement);
        return 1;
    }

    dictionary dict = dict_new();

    run_statement(program.statement, &dict);

    dict_free(&dict);
    free_stmt(program.statement);

    return 0;
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("No program has been given to be executed.\n");
        return 1;
    }

    char* file_contents = file_to_string(argv[1]);
    int result = execute_program(file_contents);
    if(result != 0){
        printf("Program is invalid!.\n");
        return 1;
    }
    free(file_contents);

    return 0;
}

//TODO: error messages for execution, error messages for parsing