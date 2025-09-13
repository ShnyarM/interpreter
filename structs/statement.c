#include "statement.h"

extern size_t LINE_COUNTER;

stmt* stmt_get_plain(s_type type){
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

stmt* stmt_get_ass(var_access* variable, expr* expr, stmt* next_statement){
    stmt* statement = stmt_get_plain(Stmt_Ass);
    statement->variable = variable;
    statement->expression = expr;
    statement->next_statement = next_statement;
    return statement;
}

// takes ownership of data_type
stmt* stmt_get_init_stmt(var_access* variable, expr* value, data_type* type, stmt* next_statement){
    stmt* statement = stmt_get_plain(Stmt_Init);
    statement->variable = variable;
    statement->expression = value;
    statement->next_statement = next_statement;
    statement->data_type = type;
    return statement;
}

stmt* stmt_get_if(expr* condition, stmt* true_branch, stmt* false_branch, stmt* next_statement){
    stmt* statement = stmt_get_plain(Stmt_If);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    statement->inner_stmt_2 = false_branch;
    return statement;
}

stmt* stmt_get_while(expr* condition, stmt* true_branch, stmt* next_statement){
    stmt* statement = stmt_get_plain(Stmt_While);
    statement->expression = condition;
    statement->next_statement = next_statement;
    statement->inner_stmt_1 = true_branch;
    return statement;
}

stmt* stmt_get_block(stmt* inside, stmt* next_statement){
    stmt* statement = stmt_get_plain(Stmt_Block);
    statement->inner_stmt_1 = inside;
    statement->next_statement = next_statement;
    return statement;
}

void free_stmt(stmt* s){
    if(s == NULL) return;

    free_expr(s->expression);
    free_stmt(s->inner_stmt_1);
    free_stmt(s->inner_stmt_2);
    free_stmt(s->next_statement);
    free_type(s->data_type);
    free_var_access(s->variable);

    free(s);
}