#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "executor.h"
#include "../structs/statement.h"
#include "../structs/expression.h"
#include "../structs/dictionary.h"
#include "../structs/program.h"
#include "../structs/data.h"
#include "../structs/function.h"

#define EXEC_ERROR_MESSAGE_LENGTH 100

char EXEC_ERROR_MESSAGE[EXEC_ERROR_MESSAGE_LENGTH];

extern dictionary GLOBAL_DICTIONARY;
extern program RUNNING_PROGRAM;

expr_eval_result eval_expr(expr* expression, dictionary* dict);

//TO DO: Maybe clean up code here, i.e. clean up all data frees and stop executions


void write_exec_error(char* message, size_t line){
    if(*message == 0) return;

    char int_buffer[20];

    strncpy(EXEC_ERROR_MESSAGE, "Runtime error on line ", EXEC_ERROR_MESSAGE_LENGTH);
    sprintf(int_buffer, "%ld", line);
    strcat(EXEC_ERROR_MESSAGE, int_buffer);
    strcat(EXEC_ERROR_MESSAGE, ": ");
    strcat(EXEC_ERROR_MESSAGE, message);
}

void write_var_not_init_error(char* variable_name, size_t line){
    char buffer[EXEC_ERROR_MESSAGE_LENGTH];

    strncpy(buffer, "Variable '", EXEC_ERROR_MESSAGE_LENGTH);
    strcat(buffer, variable_name);
    strcat(buffer, "' is not initialized.");
    write_exec_error(buffer, line);
}

BOOL stop_next_execution(execution_result e){
    return (!e.succesful || e.stop_execution || e.break_while || e.return_activated);
}

// Gets reference to variable
// This will add a new reference to accessed data
// Caller must remove this reference
static data* access_variable(var_access* variable, dictionary* dict){
    if(variable == NULL) return NULL;

    data* inner_access = access_variable(variable->inner_access, dict);
    
    if(variable->type == Literal){
        data* var_value = dict_get_reference(dict, variable->name);
        if(var_value == NULL){
            write_var_not_init_error(variable->name, variable->line);
            data_remove_reference(inner_access);
            return NULL;
        }

        data_add_reference(var_value);
        return var_value;
    }else if(variable->type == Var_Access_Array){
        expr_eval_result index_value = eval_expr(variable->index, dict); // TO DO
        if(!index_value.succesful || inner_access == NULL) return NULL;
        
        if(!is_data_of_kind_array(*inner_access)){
            write_exec_error("Variable can't be accessed as an array, as it is not an array.", variable->line);
            data_remove_reference(inner_access);
            return NULL;
        }

        if(!is_data_number_type(index_value.data)){
            write_exec_error("Index of array access is not of type number.", variable->line);
            data_remove_reference(inner_access);
            return NULL;
        }

        if(GET_NUMBER_FROM_DATA(index_value.data) < 0){
            write_exec_error("Arrays cannot be accessed with a negative index.", variable->line);
            data_remove_reference(inner_access);
            return NULL;
        }

        data* arr_access_value = data_array_get_reference(get_array_from_data(*inner_access), (int)(index_value.data.value.number));
        if(arr_access_value == NULL) return NULL;

        data_remove_reference(inner_access);
        data_add_reference(arr_access_value);
        return arr_access_value;
    }

    return NULL;
}

// calling dict is the dictionary under which the function is being called
// used to evaluate arguments
execution_result run_function(char* function_name, size_t line, expr* arguments, dictionary* calling_dict){
    execution_result exec = (execution_result){TRUE, FALSE, FALSE, FALSE, EMPTY_DATA};

    data_access_result function_data = dict_get(&RUNNING_PROGRAM.functions, function_name);
    if(!function_data.valid){
        write_exec_error("Function has not been defined", line);
        exec.succesful = FALSE;
        return exec;
    }
    function* func = get_function_from_data(function_data.result);

    dictionary function_dict = dict_get_extension(&GLOBAL_DICTIONARY);

    // evaluate arguments and put them into dictionary
    function_parameter* current_parameter = func->parameters;
    expr* current_argument = arguments;
    while(TRUE){
        if(current_parameter == NULL && current_argument == NULL) break;
        if(current_parameter == NULL || current_argument == NULL){
            write_exec_error("Number of arguments in function call does not match number of parameters in function definition.", line);
            exec.succesful = FALSE;
            free_dict(&function_dict);
            return exec;
        }

        expr_eval_result argument_value = eval_expr(current_argument, calling_dict);
        if(!argument_value.succesful || argument_value.stop_execution){
            exec.succesful = argument_value.succesful;
            exec.stop_execution = argument_value.stop_execution;
            free_dict(&function_dict);
            return exec;
        }

        if(!type_equality(argument_value.data.type, current_parameter->type)){
            write_exec_error("Data type of argument does not match declared data type in function definition.", line);
            exec.succesful = FALSE;
            free_dict(&function_dict);
            free_data(argument_value.data);
            return exec;
        }

        dict_add(&function_dict, current_parameter->name, current_parameter->type);
        data_update_result update = dict_update(&function_dict, current_parameter->name, argument_value.data);
        if(!update.valid){
            write_exec_error("This shouldnt be able to happen, something wrong.", line);
            exec.succesful = FALSE;
            free_dict(&function_dict);
            free_data(argument_value.data);
            return exec;
        }

        free_data(argument_value.data);
        current_parameter = current_parameter->next;
        current_argument = current_argument->next;
    }

    execution_result function_exec = run_program(func->content, &function_dict);
    free_dict(&function_dict);

    if(function_exec.stop_execution || !function_exec.succesful) return function_exec;

    if(!type_equality(function_exec.data.type, func->return_type)){
        write_exec_error("Value which function call returned does not match the return type of the function.", line);
        exec.succesful = FALSE;
        return exec;
    }

    data_copy_entire_data(&exec.data, function_exec.data);
    free_data(function_exec.data);
    return exec;
}

// Creates data that must be freed by caller
// TO DO: maybe remove statements that repeat themselves by creating macro
expr_eval_result eval_expr(expr* expression, dictionary* dict){
    expr_eval_result res = (expr_eval_result){TRUE, EMPTY_DATA, FALSE};
    if(expression == NULL) return res;

    // This is placed earlier, since expressions shouldnt get evaluated in this function
    if(expression->type == Function_Call){
        execution_result function_exec = run_function(expression->variable->name, expression->line, expression->expr_1, dict);
        
        res.stop_execution = function_exec.stop_execution;
        res.succesful = function_exec.succesful;
        res.data = function_exec.data;
        return res;
    }

    expr_eval_result value1 = eval_expr(expression->expr_1, dict);
    expr_eval_result value2 = eval_expr(expression->expr_2, dict);
    
    if(!value1.succesful || !value2.succesful){
        res.succesful = FALSE;
        free_data(value1.data);
        free_data(value2.data);
        return res;
    }
    if(value1.stop_execution || value2.stop_execution){
        res.stop_execution = TRUE;
        free_data(value1.data);
        free_data(value2.data);
        return res;
    }

    BOOL both_num = is_data_number_type(value1.data) && is_data_number_type(value2.data);
    // Get number values incase they are numbers and are needed
    double num1 = GET_NUMBER_FROM_DATA(value1.data);
    double num2 = GET_NUMBER_FROM_DATA(value2.data);

    if(expression->type == Lit){
        res.data = expression->data;
    }

    if(expression->type == Add){
        if(!both_num){
            write_exec_error("Invalid operands for '+' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 + num2);
    }

    if(expression->type == Sub){
        if(!both_num){
            write_exec_error("Invalid operands for '-' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 - num2);
    }

    if(expression->type == Mul){
        if(!both_num){
            write_exec_error("Invalid operands for '*' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 * num2);
    }

    if(expression->type == Div){
        if(!both_num){
            write_exec_error("Invalid operands for '/' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        if(num2 == 0){
            write_exec_error("Divide by zero error.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }
        res.data = NUMBER_DATA(num1 / num2);
    }

    if(expression->type == Neg){
        if(!is_data_number_type(value1.data)){
            write_exec_error("Invalid operand for negation operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(-num1);
    }

    if(expression->type == Var_Acc){
        data* var_value = access_variable(expression->variable, dict);
        if(var_value != NULL) data_copy_entire_data(&res.data, *var_value);
        else{
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }
        data_remove_reference(var_value);
    }

    if(expression->type == Bool_Neg){
        if(!is_data_number_type(value1.data)){
            write_exec_error("Invalid operand for '!' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(!num1);
    }

    if(expression->type == Eq){
        if(!both_num){
            write_exec_error("Invalid operand for '==' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 == num2);
    }

    if(expression->type == Lt){
        if(!both_num){
            write_exec_error("Invalid operand for '<' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 < num2);
    }

    if(expression->type == Lte){
        if(!both_num){
            write_exec_error("Invalid operand for '<=' operator.", expression->line);
            res.succesful = FALSE;
            free_data(value1.data);
            free_data(value2.data);
            return res;
        }

        res.data = NUMBER_DATA(num1 <= num2);
    }

    free_data(value1.data);
    free_data(value2.data);
    return res;
}

execution_result run_single_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE, FALSE, EMPTY_DATA};
    if(statement == NULL) return exec;

    expr_eval_result inner_expression = eval_expr(statement->expression, dict);
    if(!inner_expression.succesful){
        exec.succesful = FALSE;
        return exec;
    }
    if(inner_expression.stop_execution){
        exec.stop_execution = TRUE;
        return exec;
    }

    if(statement->type == Ass){
        data* data_reference = access_variable(statement->variable, dict);
        if(data_reference == NULL){
            exec.succesful = FALSE;
            return exec;
        }

        data_remove_reference(data_reference); // already remove reference to old variable
        data_update_result result = data_update_value(data_reference, inner_expression.data);
        if(!result.valid){
            if(result.reason == NotInitialized) write_var_not_init_error("PLACEHOLDER", statement->line);
            else if(result.reason == DifferentType) write_exec_error("Cannot assign to variable, as expression has different type.", statement->line);

            free_data(inner_expression.data);
            exec.succesful = FALSE;
            return exec;
        }
    }

    if(statement->type == Init){
        if(statement->variable->type != Literal){
            write_exec_error("ERROR, variable being initialized is not a literal, this shouldnt even happen.", statement->line);
            free_data(inner_expression.data);
            exec.succesful = FALSE;
            return exec;
        }

        BOOL result = dict_add(dict, statement->variable->name, statement->data_type);
        // result == FALSE, then variable has already been initialized
        if(!result){
            write_exec_error("Variable has already been initialized.", statement->line);
            free_data(inner_expression.data);
            exec.succesful = FALSE;
            return exec;
        }

        if(statement->expression != NULL){
            data_update_result init = dict_update(dict, statement->variable->name, inner_expression.data);
            if(!init.valid){
                exec.succesful = FALSE;
                if(init.reason == DifferentType) write_exec_error("Cannot initialize variable, as the given expression has a different data type", statement->line);
                free_data(inner_expression.data);
                return exec;
            }
        }
    }

    if(statement->type == Print){
        data_print(inner_expression.data);
        printf("\n");
    }

    if(statement->type == Return){
        data_copy_entire_data(&exec.data, inner_expression.data);
        exec.return_activated = TRUE;
    }

    if(statement->type == Expression){
        // Dont need to do anything, as expression has already been calculated further above
    }

    if(statement->type == If){
        if(GET_NUMBER_FROM_DATA(inner_expression.data)){
            exec = run_program(statement->inner_stmt_1, dict);
        }else{
            exec = run_program(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == While){
        while(GET_NUMBER_FROM_DATA(inner_expression.data)){
            exec = run_program(statement->inner_stmt_1, dict);
            if(stop_next_execution(exec)){
                exec.break_while = FALSE;
                break;
            }
            inner_expression = eval_expr(statement->expression, dict); //Recalc expression // TO DO
            if(!inner_expression.succesful){
                exec.succesful = FALSE;
                free_data(inner_expression.data);
                return exec;
            }
        }
    }

    if(statement->type == Block){
        dictionary block_dict = dict_get_extension(dict);
        exec = run_program(statement->inner_stmt_1, &block_dict);
        free_dict(&block_dict);
    }

    if(statement->type == Brk){
        //Do not run next statement
        exec.break_while = TRUE;
        free_data(inner_expression.data);
        return exec;
    }

    if(statement->type == Stop){
        //Do not run next statement and stop execution
        exec.stop_execution = TRUE;
    }

    free_data(inner_expression.data);
    return exec;
}

execution_result run_program(stmt* start_statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE};
    if(start_statement == NULL) return exec;
    
    stmt* current_stmt = start_statement;
    while(current_stmt != NULL){
        exec = run_single_statement(current_stmt, dict);
        if(stop_next_execution(exec)) return exec;

        current_stmt = current_stmt->next_statement;
    }

    return exec;
}