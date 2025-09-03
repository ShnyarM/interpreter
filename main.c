#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "helper.h"
#include "parsing/stmt_parser.h"
#include "parsing/expr_parser.h"
#include "parsing/function_parser.h"
#include "parsing/_parser_helper.h"
#include "executing/executor.h"
#include "dictionary.h"

size_t MAX_VARIABLE_LENGTH = 50;
program RUNNING_PROGRAM;
dictionary GLOBAL_DICTIONARY;

extern char EXEC_ERROR_MESSAGE[];
extern char PARSE_ERROR_MESSAGE[];

// <expression> ::= <add-expr> (('==' | '<' | '<=') <add-expr>)*
// <add-expr> ::= <mul-expr> (('+' | '-') <mul-expr>)*
// <mul-expr> ::= <atomic> (('*' | '/') <atomic>)*
// <atomic> ::= ['-' | '!'](<literal> | <var_or_array> | <parens>)
// <parens> ::= '(' <expression> ')'
// <var_or_array> ::== <variable> [ '[' <expression> ']' ]
// <variable> ::== ('_' | <letter>)('_' | <letter> | <digit>)*
// <type> ::== <atomic_type>('[]')*
// <atomic_type> ::== 'number' | 'any'

// <function_call> ::== <variable>'()'

// <program> ::= (<statement> | <function>)*
// <function> ::= 'function ' <variable> '()' ':' <type> '{' <multiple statements> '}'
// <multiple_statements> ::= <statement> (<statement>)*
// <statement> ::== <function_call>';' | <initialization> | <assignment> | <if> | <while> | <print> | <block> | 'break;' | 'stop;'

// <initialization> ::== <var_init> | <array_init>';'
// <var_init> ::== <atomic_type> <variable> ['=' <expression>]';'
// <array_init> ::== '<atomic_type>['<expression>']' <variable>';'
// <assignment> ::== <var_or_array> '=' <expression>';'

// <if> ::== 'if (' <expression> ')' <statement> ['else' <statement>]
// <while> ::== 'while (' <expression> ')' <statement>
// <print> ::== 'print(' <expression> ')'';'
// <block> ::=  '{' <multiple_statements> '}'

program parse_program(char* str){
    // No parse init here

    program prog;
    prog.first_statement = NULL;
    prog.functions = dict_new();
    prog.valid = TRUE;

    stmt* first_statement = NULL;
    stmt* last_statement = NULL;
    char* remaining = str;

    BOOL fatal = FALSE;

    while(TRUE){
        parse_result_stmt statement = parse_statement(remaining);
        if(statement.fatal){
            fatal = TRUE;
            break;
        }
        if(statement.valid){
            if(first_statement == NULL){
                first_statement = statement.statement;
                last_statement = statement.statement;
            }else{
                last_statement->next_statement = statement.statement;
                last_statement = last_statement->next_statement;
            }
            remaining = statement.remaining;
            continue;
        }

        parse_result_function function = parse_function(remaining);
        if(function.fatal){
            fatal = TRUE;
            break;
        }
        if(function.valid){
            // create data block which encapsules function
            data func = FUNCTION_DATA(function.function);
            BOOL result = dict_add(&prog.functions, function.function->name, func);

            if(!result){
                fatal = TRUE;
                write_parse_error("Duplicate function names are not allowed.");
                free_function(function.function);
            }

            remaining = function.remaining;
            continue;
        }

        break; //No statement or function could be found
    }

    // check if nothing else at end
    remove_spaces(&remaining);
    if(!fatal && *remaining != 0){
        fatal = TRUE;
        write_parse_error("Unknown symbol at end of file.");
    }

    if(fatal){
        dict_free(&prog.functions);
        free_stmt(first_statement);
        prog.valid = FALSE;
        return prog;
    }

    prog.first_statement = first_statement;
    return prog;
}

int execute_program(char* str){
    program program = parse_program(str);
    if(!program.valid){
        printf("%s\n", PARSE_ERROR_MESSAGE);
        return 1;
    }

    GLOBAL_DICTIONARY = dict_new();
    RUNNING_PROGRAM = program;

    execution_result exec = run_program(program.first_statement, &GLOBAL_DICTIONARY);

    dict_free(&GLOBAL_DICTIONARY);
    free_stmt(program.first_statement);
    dict_free(&program.functions);

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

//TODO: first arguments for function calls, then return values for function calls, after that clean up type system (proper support for multi dimensional arrays)