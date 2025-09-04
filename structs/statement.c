#include "statement.h"

extern size_t LINE_COUNTER;

static stmt* get_plain_statement(s_type type){
    stmt* statement = (stmt*)malloc(sizeof(stmt));
    statement->type = type;
    statement->variable = NULL;
    statement->expression = NULL;
    statement->next_statement = NULL;
    statement->inner_stmt_1 = NULL;
    statement->inner_stmt_2 = NULL;
    statement->line = LINE_COUNTER;
    statement->data_type = NULL;
    return statement;
}

stmt* get_ass_statement(char* variable, expr* expr, stmt* next_statement){
    stmt* statement = get_plain_statement(Ass);
    statement->variable = copy_of_string(variable);
    statement->expression = expr;
    statement->next_statement = next_statement;
    return statement;
}

stmt* stmt_get_init_stmt(var_access* variable, expr* value, data_type* type, stmt* next_statement){
    stmt* statement = get_plain_statement(Init);
    statement->variable = variable;
    statement->expression = value;
    statement->next_statement = next_statement;
    statement->data_type = type;
    return statement;
}

stmt* get_if_statement(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement){
    stmt* statement = get_plain_statement(If);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = false_branch;
    return statement;
}

stmt* get_while_statement(expr* condition, stmt* true_branch, stmt* next_statement){
    stmt* statement = get_plain_statement(While);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    return statement;
}

stmt* get_print_statement(expr* expression, stmt* next_statement){
    stmt* statement = get_plain_statement(Print);
    statement->expression = expression;
    statement->next_statement = next_statement;
    return statement;
}

stmt* get_block_statement(stmt* inside, stmt* next_statement){
    stmt* statement = get_plain_statement(Block);
    statement->inner_stmt_1 = inside;
    statement->next_statement = next_statement;
    return statement;
}

void free_stmt(stmt* s){
    if(s == NULL) return;

    if(s->variable != NULL) free(s->variable);

    free_expr(s->expression);
    free_stmt(s->inner_stmt_1);
    free_stmt(s->inner_stmt_2);
    free_stmt(s->next_statement);
    free_var_access(s->variable);

    free(s);
}