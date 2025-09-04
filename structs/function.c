#include "function.h"

//TO DO: add types
static function* get_function(char* name, stmt* content){
    function* func = malloc(sizeof(function));
    func->name = copy_of_string(name);
    func->content = content;
    func->return_type = INVALID_DATA_TYPE;
    return func;
}

void free_function(function* func){
    free(func->name);
    free_stmt(func->content);
    
    free(func);
}