#pragma once

#include "../helper.h"
#include "expr_parser.h"
#include "../types.h"

typedef struct parse_result_statement_t{
    BOOL valid;
    stmt* statement;
    char* remaining;
    BOOL fatal;         //If this is true mistake in code was found, abort everything and give error
} parse_result_stmt;

parse_result_stmt parse_multiple_statements(char* str);
parse_result_stmt parse_statement(char* str);
void free_stmt(stmt* s);