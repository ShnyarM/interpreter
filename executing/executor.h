#pragma once

#include "../helper.h"
#include "../types.h"
#include "../dictionary.h"

typedef struct execution_result_t{
    BOOL succesful;
    BOOL stop_execution;
    BOOL break_while;
} execution_result;

execution_result run_statement(stmt* statement, dictionary* dict);