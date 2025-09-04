#pragma once

// Variable accesses
typedef enum var_access_e {Literal, Var_Access_Array} var_access_type;

typedef struct variable_access_t{
    var_access_type type;
    char* name;                             // Name of variable for literal access
    expr* index;                              // For array accesses
    struct variable_access_t* inner_access;

    size_t line;                              // Line where this access occurs
} var_access;

var_access* var_access_get_literal(char* name);
var_access* var_access_get_array(var_access* inner, expr* index);
void free_var_access(var_access* v);
