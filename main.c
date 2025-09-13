#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "helper.h"
#include "parsing/stmt_parser.h"
#include "parsing/expr_parser.h"
#include "parsing/function_parser.h"
#include "parsing/_parser_helper.h"
#include "executing/executor.h"
#include "structs/dictionary.h"
#include "structs/program.h"
#include "structs/function.h"

size_t MAX_VARIABLE_LENGTH = 50;
size_t MAX_STRING_LENGTH = 500;
program RUNNING_PROGRAM;
dictionary GLOBAL_DICTIONARY;

extern char EXEC_ERROR_MESSAGE[];
extern char PARSE_ERROR_MESSAGE[];

// <expression> ::= <add-expr> (('==' | '<' | '<=') <add-expr>)*
// <add-expr> ::= <mul-expr> (('+' | '-') <mul-expr>)*
// <mul-expr> ::= <atomic> (('*' | '/') <atomic>)*
// <atomic> ::= ['-' | '!'](<literal> | <variable_access> | <parens> | <function_call> | <array> | <string> | <bool> | <null>)
// <parens> ::= '(' <expression> ')'

// <function_call> ::== <variable_name>'('[<expression_listg>]')'

// <array> ::== '[' [<expression_list>] ']'
// <expression_list> ::== <expression> (',' <expression>)*

// <string> ::== '"' (<character>)* '"'
// <bool> ::== 'true' | 'false'
// <null> ::== 'null'

// <variable_name> ::== ('_' | <letter>)('_' | <letter> | <digit>)*
// <variable_access> ::== <variable_name>( ('['<expression']') )*

// <type> ::== <atomic_type>('[]')*
// <atomic_type> ::== 'number' | 'any'

// <program> ::= (<statement> | <function>)*

// <function> ::= 'function ' <variable_name> '(' [<parameter_list>] ')' [':' <type>] '{' <multiple statements> '}'
// <parameter_list> ::= <type> <variable_name> (',' <type> <variable_name>)*

// <multiple_statements> ::= <statement> (<statement>)*
// <statement> ::== <initialization> | <assignment> | <if> | <while> | <for> | <block> | 'break;' | 'stop;' | <expression>';'

// <initialization> ::== <type> <variable_name> ['=' <expression>]';'
// <assignment> ::== <variable_access> '=' <expression>';'

// <if> ::== 'if (' <expression> ')' <statement> ['else' <statement>]
// <while> ::== 'while (' <expression> ')' <statement>
// <for> ::== 'for' '(' <statement> <expression>';' <statement> ')' <statement>
// <block> ::=  '{' <multiple_statements> '}'
// <return> ::= 'return ' [<expression>]';' 

program parse_program(char* str){
    // No parse init here

    program prog;
    prog.first_statement = NULL;
    prog.functions = dict_new();
    dict_add_built_in_functions(&prog.functions);
    prog.valid = TRUE;

    stmt* first_statement = NULL;
    stmt* last_statement = NULL;
    char* remaining = str;

    BOOL fatal = FALSE;

    while(TRUE){
        parse_result_function function = parse_function(remaining);
        if(function.fatal){
            fatal = TRUE;
            break;
        }
        if(function.valid){
            // create data block which encapsules function
            data func = FUNCTION_DATA(function.function);
            data_ref_access_result result = dict_add(&prog.functions, function.function->name, &GLOBAL_FUNCTION_TYPE);

            if(!result.valid){
                fatal = TRUE;
                write_parse_error("Duplicate function names are not allowed.");
                free_function(function.function);
            }

            data_assign_to(result.ref, func, result.storable);
            remaining = function.remaining;
            continue;
        }

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

        break; //No statement or function could be found
    }

    // check if nothing else at end
    remove_spaces(&remaining);
    if(!fatal && *remaining != 0){
        fatal = TRUE;
        write_parse_error("Unknown symbol at end of file.");
    }

    if(fatal){
        free_dict(&prog.functions);
        free_stmt(first_statement);
        prog.valid = FALSE;
        return prog;
    }

    prog.first_statement = first_statement;
    return prog;
}

int execute_program(char* str, int argc, char** argv){
    program program = parse_program(str);
    if(!program.valid){
        printf("%s\n", PARSE_ERROR_MESSAGE);
        return 1;
    }

    GLOBAL_DICTIONARY = dict_new();
    RUNNING_PROGRAM = program;

    // add args
    data_type* string_array_type = types_get_array_type(&GLOBAL_STRING_TYPE);
    data_array* args = data_array_new(&GLOBAL_STRING_TYPE);
    data array_data = (data){string_array_type, {.arr=args}};
    for(int i = 0; i < argc; i++){
        data arg = STRING_DATA(argv[i]);
        data_ref_access_result ref = data_array_get_reference(args, i);
        data_assign_to(ref.ref, arg, &GLOBAL_STRING_TYPE);
        free_data(arg);
    }

    data_ref_access_result ref = dict_add(&GLOBAL_DICTIONARY, "_sys_args", string_array_type);
    data_assign_to(ref.ref, array_data, ref.ref->type);
    free_data(array_data);


    execution_result exec = run_program(program.first_statement, &GLOBAL_DICTIONARY);

    free_dict(&GLOBAL_DICTIONARY);
    free_stmt(program.first_statement);
    free_dict(&program.functions);

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
    int result = execute_program(file_contents, argc-1, argv+1);
    free(file_contents);

    return result;
}

//TO DO: for loops testing, greater than, dictionaries, maybe file system