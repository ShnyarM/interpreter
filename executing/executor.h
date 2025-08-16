#pragma once

#include "../helper.h"
#include "../types.h"
#include "../dictionary.h"

typedef struct execution_result_t{
    BOOL succesful;
    BOOL stop_execution;
    BOOL break_while;
} execution_result;

typedef struct expr_eval_result_t{
    BOOL succesful;
    data data;
} expr_eval_result;

//execution_result run_statement(stmt* statement, dictionary* dict);
execution_result run_program(stmt* start_statement, dictionary* dict);