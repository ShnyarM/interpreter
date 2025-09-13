#pragma once

#include "../helper.h"
#include "../structs/data.h"
#include "../structs/statement.h"
#include "../structs/dictionary.h"

typedef struct execution_result_t{
    BOOL succesful;
    BOOL stop_execution;
    BOOL break_while;
    BOOL return_activated;
    data data;
} execution_result;

typedef struct expr_eval_result_t{
    BOOL succesful;
    data data;
    BOOL stop_execution;
} expr_eval_result;

typedef struct expr_list_eval_result_t{
    BOOL succesful;
    data_list* data;
    BOOL stop_execution;
} expr_list_eval_result;

//execution_result run_statement(stmt* statement, dictionary* dict);
void write_exec_error(char* message, size_t line);
execution_result run_program(stmt* start_statement, dictionary* dict);