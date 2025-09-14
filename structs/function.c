#include "function.h"

built_in_function_definition BUILT_IN_FUNCTIONS[BUILT_IN_FUNCTION_COUNT] = {
    (built_in_function_definition){"print", &built_in_print},
    (built_in_function_definition){"length", &built_in_length},
    (built_in_function_definition){"castToNumber", &built_in_cast_to_number},
    (built_in_function_definition){"castToString", &built_in_cast_to_string},
    (built_in_function_definition){"typeOf", &built_in_type_of},
    (built_in_function_definition){"input", &built_in_input}
};

// takes ownership of return_type
function_parameter* function_parameter_get(char* name, data_type* return_type){
    function_parameter* arg = malloc(sizeof(function_parameter));
    arg->name = copy_of_string(name);
    arg->type = return_type;
    arg->next = NULL;
    return arg;
}

// takes ownership of type
function* function_get_plain(){
    function* func = malloc(sizeof(function));
    func->name = NULL;
    func->content = NULL;
    func->return_type = NULL;
    func->parameters = NULL;
    func->is_built_in = FALSE;
    func->built_in_func_ptr = NULL;
    return func;
}

function* function_get_built_in(char* name, built_in_function ptr){
    function* func = function_get_plain();
    func->name = copy_of_string(name);
    func->is_built_in = TRUE;
    func->built_in_func_ptr = ptr;
    return func;
}

// takes ownership of type
function* function_get(char* name, data_type* return_type, stmt* content, function_parameter* parameters){
    function* func = function_get_plain();
    func->name = copy_of_string(name);
    func->content = content;
    func->return_type = return_type;
    func->parameters = parameters;
    return func;
}

void free_function_parameter(function_parameter* arg){
    if(arg == NULL) return;

    if(arg->name != NULL) free(arg->name);
    free_type(arg->type);
    free_function_parameter(arg->next);

    free(arg);
}

void free_function(function* func){
    if(func == NULL) return;

    if(func->name != NULL) free(func->name);
    free_stmt(func->content);
    free_type(func->return_type);
    free_function_parameter(func->parameters);
    
    free(func);
}