#include <stdlib.h>
#include "variable_access.h"
#include "expression.h"
#include "../helper.h"
#include <stdio.h>

extern size_t LINE_COUNTER;

var_access* var_access_get_plain(){
    var_access* v = (var_access*)malloc(sizeof(var_access));

    v->type = Literal;
    v->inner_access = NULL;
    v->name = NULL;
    v->index = NULL;
    v->line = LINE_COUNTER;

    return v;
}

var_access* var_access_get_literal(char* name){
    var_access* v = var_access_get_plain();
    v->type = Literal;
    v->name = copy_of_string(name);

    return v;
}

// takes ownership of both inner and index expression
var_access* var_access_get_array(var_access* inner, expr* index){
    var_access* v = var_access_get_plain();
    v->type = Var_Access_Array;
    v->inner_access = inner;
    v->index = index;

    return v;
}

void free_var_access(var_access* v){
    if(v == NULL) return;
    //printf("%p\n%p\n\n", v, v->name);

    free_var_access(v->inner_access);
    free_expr(v->index);
    if(v->name != NULL) free(v->name);

    free(v);
}