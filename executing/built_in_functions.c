#include <stdio.h>
#include "../structs/function.h" // also header of this file
#include "executor.h"

#define BAD_EXEC_RESULT ((built_in_exec_result){FALSE, EMPTY_DATA})
#define EXEC_RESULT(data) ((built_in_exec_result){TRUE, data})

built_in_exec_result built_in_print(data_list* args, size_t line){
    data_list* current = args;
    while(current != NULL){
        string* str = string_from_data(current->data, FALSE);
        printf("%s", str->c_string);
        if(current->next != NULL) printf(" ");

        free_string(str);
        current = current->next;
    }
    printf("\n");

    return EXEC_RESULT(EMPTY_DATA);
}

built_in_exec_result built_in_length(data_list* args, size_t line){
    if(args == NULL || args->size != 1){
        write_exec_error("length() function takes in exactly 1 argument.", line);
        return BAD_EXEC_RESULT;
    }

    data result;
    if(data_is_of_kind_array(args->data)){
        result = NUMBER_DATA(data_get_array_from_data(args->data)->size);
    }else if(data_is_string_type(args->data)){
        result = NUMBER_DATA(data_get_string_from_data(args->data)->length);
    }else{
        write_exec_error("length() can only take arrays or strings as arguments.", line);
        return BAD_EXEC_RESULT;
    }

    return EXEC_RESULT(result);
}

built_in_exec_result built_in_cast_to_number(data_list* args, size_t line){
    if(args == NULL || args->size != 1){
        write_exec_error("castToNumber() function takes in exactly 1 argument.", line);
        return BAD_EXEC_RESULT;
    }

    data result;
    if(data_is_string_type(args->data)){
        string* str = data_get_string_from_data(args->data);
        result = NUMBER_DATA(strtod(str->c_string, NULL));
    }else if(data_is_bool_type(args->data)){
        if(GET_BOOL_FROM_DATA(args->data)) result = NUMBER_DATA(1);
        else result = NUMBER_DATA(0);
    }else{
        write_exec_error("Given data type cannot be cast to a number.", line);
        return BAD_EXEC_RESULT;
    }

    return EXEC_RESULT(result);
}

built_in_exec_result built_in_cast_to_string(data_list* args, size_t line){
    if(args == NULL || args->size != 1){
        write_exec_error("castToString() function takes in exactly 1 argument.", line);
        return BAD_EXEC_RESULT;
    }

    string* str = string_from_data(args->data, FALSE);

    return EXEC_RESULT(STRING_DATA_PTR(str));
}

built_in_exec_result built_in_type_of(data_list* args, size_t line){
    if(args == NULL || args->size != 1){
        write_exec_error("typeOf() function takes in exactly 1 argument.", line);
        return BAD_EXEC_RESULT;
    }

    string* str = string_from_type(args->data.type);

    return EXEC_RESULT(STRING_DATA_PTR(str));
}