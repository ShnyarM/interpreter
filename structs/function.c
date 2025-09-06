#include "function.h"

//TO DO: add types
function* function_get(char* name, stmt* content){
    function* func = malloc(sizeof(function));
    func->name = copy_of_string(name);
    func->content = content;
    func->return_type = &GLOBAL_INVALID_TYPE;
    return func;
}

void free_function(function* func){
    free(func->name);
    free_stmt(func->content);
    
    free(func);
}