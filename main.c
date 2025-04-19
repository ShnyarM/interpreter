#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "helper.h"
#include "parsing/stmt_parser.h"
#include "executing/executor.h"
#include "dictionary.h"

size_t MAX_VARIABLE_LENGTH = 50;

extern char EXEC_ERROR_MESSAGE[];
extern char PARSE_ERROR_MESSAGE[];

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
    if(!program.valid){
        printf("%s\n", PARSE_ERROR_MESSAGE);
        return 1;
    }

    remove_spaces(&program.remaining);
    if(*program.remaining != 0){
        free_stmt(program.statement);
        return 1;
    }

    dictionary dict = dict_new();

    execution_result exec = run_statement(program.statement, &dict);

    dict_free(&dict);
    free_stmt(program.statement);

    if(!exec.succesful){
        printf("There was an error while running the script. Error:\n%s\n", EXEC_ERROR_MESSAGE);
        return 1;
    }

    return 0;
}

int main(int argc, char** argv){
    if(argc < 2){
        printf("No program has been given to be executed.\n");
        return 1;
    }

    char* file_contents = file_to_string(argv[1]);
    if(file_contents == NULL){
        printf("File doesn't exist.\n");
        return 1;
    }
    int result = execute_program(file_contents);
    free(file_contents);

    return result;
}

//TODO: a lot of testing...
// make execution use iteration for statements instead of recursion, clean up program parsing