#include "function.h"

// takes ownership of type
function* function_get(char* name, data_type* return_type, stmt* content){
    function* func = malloc(sizeof(function));
    func->name = copy_of_string(name);
    func->content = content;
    func->return_type = return_type;
    return func;
}

void free_function(function* func){
    if(func->name != NULL) free(func->name);
    free_stmt(func->content);
    free_type(func->return_type);
    
    free(func);
}