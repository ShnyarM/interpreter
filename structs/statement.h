#pragma once

#include <stdlib.h>
#include "expression.h"
#include "variable_access.h"

typedef enum statement_type {Ass, Init, Print, If, While, Block, Brk, Stop, Expression, Return} s_type;

typedef struct statement_t{
    s_type type;
    var_access* variable;
    expr* expression;
    struct statement_t* next_statement;
    data_type* data_type;              // for variable initialization statements, store the type of the variable

    struct statement_t* inner_stmt_1; //Used for if true branch, while and block
    struct statement_t* inner_stmt_2; //used for if false branch (else path)$

    size_t line;                      // line at which this expression appears
} stmt;

stmt* stmt_get_plain(s_type type);
stmt* get_ass_statement(var_access* variable, expr* expr, stmt* next_statement);
stmt* stmt_get_init_stmt(var_access* variable, expr* value, data_type* type, stmt* next_statement);
stmt* get_if_statement(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement);
stmt* get_while_statement(expr* condition, stmt* true_branch, stmt* next_statement);
stmt* get_print_statement(expr* expression, stmt* next_statement);
stmt* get_block_statement(stmt* inside, stmt* next_statement);
void free_stmt(stmt* s); 