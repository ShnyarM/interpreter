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
static data_ref_access_result access_variable(var_access* variable, dictionary* dict){
    data_ref_access_result res;
    res.ref = NULL;
    res.valid = FALSE;
    res.storable = &GLOBAL_INVALID_TYPE;

    if(variable == NULL) return res;

    data_ref_access_result inner_access = access_variable(variable->inner_access, dict);
    
    if(variable->type == Literal){
        data_ref_access_result var_value = dict_get_reference(dict, variable->name);
        if(!var_value.valid){
            write_var_not_init_error(variable->name, variable->line);
            return res;
        }

        return var_value;
    }else if(variable->type == Var_Access_Array){
        expr_eval_result index_value = eval_expr(variable->index, dict); // TO DO
        if(!index_value.succesful || !inner_access.valid) return res;
        
        if(!data_is_of_kind_array(*inner_access.ref)){
            write_exec_error("Variable can't be accessed as an array, as it is not an array.", variable->line);
            return res;
        }

        if(!data_is_number_type(index_value.data)){
            write_exec_error("Index of array access is not of type number.", variable->line);
            return res;
        }

        if(GET_NUMBER_FROM_DATA(index_value.data) < 0){
            write_exec_error("Arrays cannot be accessed with a negative index.", variable->line);
            return res;
        }

        data_ref_access_result arr_access_value = data_array_get_reference(data_get_array_from_data(*inner_access.ref), (int)(index_value.data.value.number));
        if(!arr_access_value.valid) return res;
        return arr_access_value;
    }

    return res;
}

// calling dict is the dictionary under which the function is being called
// used to evaluate arguments
execution_result run_function(char* function_name, size_t line, expr* arguments, dictionary* calling_dict){
    execution_result exec = (execution_result){TRUE, FALSE, FALSE, FALSE, EMPTY_DATA};
    execution_result function_exec  = (execution_result){TRUE, FALSE, FALSE, FALSE, EMPTY_DATA};
    dictionary function_dict = dict_get_extension(&GLOBAL_DICTIONARY);

    data_access_result function_data = dict_get(&RUNNING_PROGRAM.functions, function_name);
    if(!function_data.valid){
        write_exec_error("Function has not been defined", line);
        goto UNSUCCESSFUL_FUNCTION_EXECUTION;
    }
    function* func = data_get_function_from_data(function_data.result);

    // evaluate arguments and put them into dictionary
    function_parameter* current_parameter = func->parameters;
    expr* current_argument = arguments;
    while(TRUE){
        if(current_parameter == NULL && current_argument == NULL) break;
        if(current_parameter == NULL || current_argument == NULL){
            write_exec_error("Number of arguments in function call does not match number of parameters in function definition.", line);
            goto UNSUCCESSFUL_FUNCTION_EXECUTION;    
        }

        expr_eval_result argument_value = eval_expr(current_argument, calling_dict);
        if(!argument_value.succesful || argument_value.stop_execution){
            exec.succesful = argument_value.succesful;
            exec.stop_execution = argument_value.stop_execution;
            free_dict(&function_dict);
            return exec;
        }

        if(!types_can_be_assigned_to(current_parameter->type, argument_value.data.type)){
            write_exec_error("Data type of argument does not match declared data type in function definition.", line);
            free_data(argument_value.data);
            goto UNSUCCESSFUL_FUNCTION_EXECUTION;
        }

        data_ref_access_result ref = dict_add(&function_dict, current_parameter->name, current_parameter->type);
        if(!ref.valid){
            write_exec_error("Function can't be called, as it has two parameters with the same name.", line);
            free_data(argument_value.data);
            goto UNSUCCESSFUL_FUNCTION_EXECUTION;
        }

        data_update_result update = data_assign_to(ref.ref, argument_value.data, ref.storable);
        if(!update.valid){
            write_exec_error("This shouldnt be able to happen, something wrong.", line);
            free_data(argument_value.data);
            goto UNSUCCESSFUL_FUNCTION_EXECUTION;
        }

        free_data(argument_value.data);
        current_parameter = current_parameter->next;
        current_argument = current_argument->next;
    }

    function_exec = run_program(func->content, &function_dict);

    if(!function_exec.succesful) goto UNSUCCESSFUL_FUNCTION_EXECUTION;
    if(function_exec.stop_execution){
        exec.stop_execution = TRUE;
        goto UNSUCCESSFUL_FUNCTION_EXECUTION_FREE;
    }

    data_update_result res = data_assign_to(&exec.data, function_exec.data, func->return_type);
    if(!res.valid){
        write_exec_error("Value which function call returned does not match the return type of the function.", line);
        goto UNSUCCESSFUL_FUNCTION_EXECUTION;
    }

    free_data(function_exec.data);
    free_dict(&function_dict);
    return exec;

UNSUCCESSFUL_FUNCTION_EXECUTION:
    exec.succesful = FALSE;
UNSUCCESSFUL_FUNCTION_EXECUTION_FREE:
    free_data(function_exec.data);
    free_dict(&function_dict);
    return exec;
}

// Creates data that must be freed by caller
expr_eval_result eval_expr(expr* expression, dictionary* dict){
    expr_eval_result res = (expr_eval_result){TRUE, EMPTY_DATA, FALSE};
    if(expression == NULL) return res;

    // This is placed earlier, since expressions shouldnt get evaluated in this function
    if(expression->type == Expr_Function_Call){
        execution_result function_exec = run_function(expression->variable->name, expression->line, expression->expr_1, dict);
        
        res.stop_execution = function_exec.stop_execution;
        res.succesful = function_exec.succesful;
        res.data = function_exec.data;
        return res;
    }

    if(expression->type == Expr_Array){
        data_array* arr = data_array_new(&GLOBAL_ANY_TYPE);
        res.data.value.arr = arr;

        expr* current_expression = expression->expr_1;
        data_type* current_believed_type = NULL;
        BOOL is_any_data_type = FALSE;
        int i = 0;

        if(current_expression == NULL){ // EMPTY ARRAY
            res.data.type = types_get_array_type(&GLOBAL_ANY_TYPE);
            return res;
        } 

        while(current_expression != NULL){
            expr_eval_result value = eval_expr(current_expression, dict);
            if(!value.succesful || value.stop_execution){
                res = value;
                free_data_array(arr);
                return res;
            }
            if(!is_any_data_type){
                if(current_believed_type == NULL) current_believed_type = value.data.type;
                else{
                    if(!types_equality(current_believed_type, value.data.type)){
                        is_any_data_type = TRUE;
                        current_believed_type = &GLOBAL_ANY_TYPE;
                    }
                }
            }

            data_ref_access_result ref = data_array_get_reference(arr, i);
            data_assign_to(ref.ref, value.data, &GLOBAL_ANY_TYPE);
            free_data(value.data);
            current_expression = current_expression->next;
            i++;
        }

        data_array_set_new_type(arr, current_believed_type);
        res.data.type = types_get_array_type(types_copy(current_believed_type));
        return res;
    }


    expr_eval_result value1 = eval_expr(expression->expr_1, dict);
    expr_eval_result value2 = eval_expr(expression->expr_2, dict);
    
    if(!value1.succesful || !value2.succesful){
        goto UNSUCCESSFUL_EVAL_EXPR;
    }
    if(value1.stop_execution || value2.stop_execution){
        res.stop_execution = TRUE;
        goto UNSUCCESSFUL_EVAL_EXPR_FREE;
    }

    BOOL both_num = data_is_number_type(value1.data) && data_is_number_type(value2.data);
    // Get number values incase they are numbers and are needed
    double num1 = GET_NUMBER_FROM_DATA(value1.data);
    double num2 = GET_NUMBER_FROM_DATA(value2.data);

    if(expression->type == Expr_Lit){
        res.data = expression->data;
    }

    if(expression->type == Expr_Add){
        if(both_num){ // Both are numbers
            res.data = NUMBER_DATA(num1 + num2);
        }else if(data_is_string_type(value1.data) && data_is_string_type(value2.data)){ // Both are strings
            string* new_string = strings_concat(data_get_string_from_data(value1.data), data_get_string_from_data(value2.data));
            res.data = STRING_DATA_PTR(new_string);
        }else if(data_is_string_type(value1.data)){ // First is string
            string* converted = string_from_data(value2.data, FALSE);
            string* new_string = strings_concat(data_get_string_from_data(value1.data), converted);
            res.data = STRING_DATA_PTR(new_string);
            free_string(converted);
        }else if(data_is_string_type(value2.data)){ // second is string
            string* converted = string_from_data(value1.data, FALSE);
            string* new_string = strings_concat(converted, data_get_string_from_data(value2.data));
            res.data = STRING_DATA_PTR(new_string);
            free_string(converted);
        }else{
            write_exec_error("Invalid operands for '+' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }
    }

    if(expression->type == Expr_Sub){
        if(!both_num){
            write_exec_error("Invalid operands for '-' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(num1 - num2);
    }

    if(expression->type == Expr_Mul){
        if(!both_num){
            write_exec_error("Invalid operands for '*' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(num1 * num2);
    }

    if(expression->type == Expr_Div){
        if(!both_num){
            write_exec_error("Invalid operands for '/' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        if(num2 == 0){
            write_exec_error("Divide by zero error.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }
        res.data = NUMBER_DATA(num1 / num2);
    }

    if(expression->type == Expr_Neg){
        if(!data_is_number_type(value1.data)){
            write_exec_error("Invalid operand for negation operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(-num1);
    }

    if(expression->type == Expr_Var_Acc){
        data_ref_access_result var_value = access_variable(expression->variable, dict);
        if(var_value.valid) data_assign_to(&res.data, *var_value.ref, &GLOBAL_ANY_TYPE);
        else{
            goto UNSUCCESSFUL_EVAL_EXPR;
        }
    }

    if(expression->type == Expr_Bool_Neg){
        if(!data_is_number_type(value1.data)){
            write_exec_error("Invalid operand for '!' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(!num1);
    }

    if(expression->type == Expr_Eq){
        BOOL result = data_equality(value1.data, value2.data);
        res.data = NUMBER_DATA((double)result);
    }

    if(expression->type == Expr_Lt){
        if(!both_num){
            write_exec_error("Invalid operand for '<' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(num1 < num2);
    }

    if(expression->type == Expr_Lte){
        if(!both_num){
            write_exec_error("Invalid operand for '<=' operator.", expression->line);
            goto UNSUCCESSFUL_EVAL_EXPR;
        }

        res.data = NUMBER_DATA(num1 <= num2);
    }

    free_data(value1.data);
    free_data(value2.data);
    return res;

UNSUCCESSFUL_EVAL_EXPR:
    res.succesful = FALSE;
UNSUCCESSFUL_EVAL_EXPR_FREE:
    free_data(value1.data);
    free_data(value2.data);
    return res;
}

execution_result run_single_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE, FALSE, EMPTY_DATA};
    if(statement == NULL) return exec;

    expr_eval_result inner_expression = eval_expr(statement->expression, dict);
    if(!inner_expression.succesful){
        goto UNSUCCESSFUL_STATEMENT_EXECUTION;
    }
    if(inner_expression.stop_execution){
        exec.stop_execution = TRUE;
        return exec;
    }

    if(statement->type == Stmt_Ass){
        data_ref_access_result data_reference = access_variable(statement->variable, dict);
        if(!data_reference.valid){
            goto UNSUCCESSFUL_STATEMENT_EXECUTION;
        }

        data_update_result result = data_assign_to(data_reference.ref, inner_expression.data, data_reference.storable);
        if(!result.valid){
            if(result.reason == NotInitialized) write_var_not_init_error("PLACEHOLDER", statement->line);
            else if(result.reason == DifferentType) write_exec_error("Cannot assign to variable, as expression has different type.", statement->line);

            goto UNSUCCESSFUL_STATEMENT_EXECUTION;
        }
    }

    if(statement->type == Stmt_Init){
        if(statement->variable->type != Literal){
            write_exec_error("ERROR, variable being initialized is not a literal, this shouldnt even happen.", statement->line);
            goto UNSUCCESSFUL_STATEMENT_EXECUTION;
        }

        data_ref_access_result ref = dict_add(dict, statement->variable->name, statement->data_type);
        // !ref.valid, then variable has already been initialized
        if(!ref.valid){
            write_exec_error("Variable has already been initialized.", statement->line);
            goto UNSUCCESSFUL_STATEMENT_EXECUTION;
        }

        if(statement->expression != NULL){
            data_update_result init = data_assign_to(ref.ref, inner_expression.data, ref.storable);
            if(!init.valid){
                if(init.reason == DifferentType) write_exec_error("Cannot initialize variable, as the given expression has a different data type", statement->line);
                goto UNSUCCESSFUL_STATEMENT_EXECUTION;
            }
        }
    }

    if(statement->type == Stmt_Print){
        string* str = string_from_data(inner_expression.data, FALSE);
        printf("%s\n", str->c_string);
        free_string(str);
    }

    if(statement->type == Stmt_Return){
        data_assign_to(&exec.data, inner_expression.data, &GLOBAL_ANY_TYPE);
        exec.return_activated = TRUE;
    }

    if(statement->type == Stmt_Expression){
        // Dont need to do anything, as expression has already been calculated further above
    }

    if(statement->type == Stmt_If){
        // TO DO: better boolean evaluation
        if(GET_NUMBER_FROM_DATA(inner_expression.data)){
            exec = run_program(statement->inner_stmt_1, dict);
        }else{
            exec = run_program(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == Stmt_While){
        // TO DO: better boolean evaluation
        while(GET_NUMBER_FROM_DATA(inner_expression.data)){
            exec = run_program(statement->inner_stmt_1, dict);
            if(stop_next_execution(exec)){
                exec.break_while = FALSE;
                break;
            }

            data old_inner_expression = inner_expression.data;
            inner_expression = eval_expr(statement->expression, dict); //Recalc expression
            free_data(old_inner_expression);
            if(!inner_expression.succesful){
                goto UNSUCCESSFUL_STATEMENT_EXECUTION;
            }
        }
    }

    if(statement->type == Stmt_Block){
        dictionary block_dict = dict_get_extension(dict);
        exec = run_program(statement->inner_stmt_1, &block_dict);
        free_dict(&block_dict);
    }

    if(statement->type == Stmt_Brk){
        //Do not run next statement
        exec.break_while = TRUE;
    }

    if(statement->type == Stmt_Stop){
        //Do not run next statement and stop execution
        exec.stop_execution = TRUE;
    }

    free_data(inner_expression.data);
    return exec;

UNSUCCESSFUL_STATEMENT_EXECUTION:
    exec.succesful = FALSE;
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