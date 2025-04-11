#pragma once

#include "../helper.h"
#include "expr_parser.h"
#include "../types.h"

typedef struct parse_result_statement_t{
    BOOL valid;
    stmt* statement;
    char* remaining;
} parse_result_stmt;

parse_result_stmt parse_program(char* str);
void free_stmt(stmt* s);