#include <stdio.h>
#include <string.h>
#include "executor.h"
#include "../types.h"
#include "../dictionary.h"

#define EXEC_ERROR_MESSAGE_LENGTH 100

char EXEC_ERROR_MESSAGE[EXEC_ERROR_MESSAGE_LENGTH];

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
    return (!e.succesful || e.stop_execution || e.break_while);
}

static expr_eval_result eval_expr(expr* expression, dictionary* dict){
    expr_eval_result res = (expr_eval_result){TRUE, EMPTY_DATA};
    if(expression == NULL) return res;

    expr_eval_result value1 = eval_expr(expression->expr_1, dict);
    expr_eval_result value2 = eval_expr(expression->expr_2, dict);

    BOOL both_num = IS_ATOMIC_NUM_TYPE(value1.data) && IS_ATOMIC_NUM_TYPE(value2.data);
    // Get number values incase they are numbers and are needed
    double num1 = GET_NUMBER_FROM_DATA(value1.data);
    double num2 = GET_NUMBER_FROM_DATA(value2.data);

    if(!value1.succesful || !value2.succesful){
        res.succesful = FALSE;
        return res;
    }

    if(expression->type == Lit){
        res.data = expression->data;
    }

    if(expression->type == Add){
        if(!both_num){
            write_exec_error("Invalid operands for '+' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 + num2);
    }

    if(expression->type == Sub){
        if(!both_num){
            write_exec_error("Invalid operands for '-' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 - num2);
    }

    if(expression->type == Mul){
        if(!both_num){
            write_exec_error("Invalid operands for '*' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 * num2);
    }

    if(expression->type == Div){
        if(!both_num){
            write_exec_error("Invalid operands for '/' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        if(num2 == 0){
            write_exec_error("Divide by zero error.", expression->line);
            res.succesful = FALSE;
            return res;
        }
        res.data = NUMBER_DATA(num1 / num2);
    }

    if(expression->type == Neg){
        if(!IS_ATOMIC_NUM_TYPE(value1.data)){
            write_exec_error("Invalid operand for negation operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(-num1);
    }

    if(expression->type == Var){
        data_access_result var_value = dict_get(dict, expression->variable);
        if(var_value.valid) res.data = var_value.result;
        else{
            write_var_not_init_error(expression->variable, expression->line);
            res.succesful = FALSE;
            return res;
        }
    }

    if(expression->type == Arr_Access){
        int index = (int)GET_NUMBER_FROM_DATA(value1.data);

        data_access_result data_array = dict_get(dict, expression->variable);
        if(!data_array.valid){
            write_var_not_init_error(expression->variable, expression->line);
            res.succesful = FALSE;
            return res;
        }
        if(!IS_ARRAY(data_array.result)){
            write_exec_error("Variable is not an array.", expression->line);
            res.succesful = FALSE;
            return res;
        }
        if(index < 0 || index >= data_array.result.size){
            write_exec_error("Index is not within array bounds.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        data_access_result var_value = data_array_get(data_array.result.value.arr, index);
        if(var_value.valid) res.data = var_value.result;
        else{
            write_exec_error("Couldn't get value from array.", expression->line);
            res.succesful = FALSE;
            return res;
        }
    }

    if(expression->type == Bool_Neg){
        if(!IS_ATOMIC_NUM_TYPE(value1.data)){
            write_exec_error("Invalid operand for '!' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(!num1);
    }

    if(expression->type == Eq){
        if(!both_num){
            write_exec_error("Invalid operand for '==' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 == num2);
    }

    if(expression->type == Lt){
        if(!both_num){
            write_exec_error("Invalid operand for '<' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 < num2);
    }

    if(expression->type == Lte){
        if(!both_num){
            write_exec_error("Invalid operand for '<=' operator.", expression->line);
            res.succesful = FALSE;
            return res;
        }

        res.data = NUMBER_DATA(num1 <= num2);
    }

    return res;
}

void print_data(data d, BOOL new_line){
    if(IS_ARRAY(d)){ //If its array, print every value
        printf("[");

        int size = d.size;
        for(int i = 0; i < size-1; i++){
            print_data(d.value.arr[i], FALSE);
            printf(", ");
        }

        print_data(d.value.arr[size-1], FALSE);
        printf("]");
    }else{
        printf("%.15g", GET_NUMBER_FROM_DATA(d));
    }

    if(new_line) printf("\n");
}

execution_result run_single_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE};
    if(statement == NULL) return exec;

    expr_eval_result inner_expression = eval_expr(statement->expression, dict);
    if(!inner_expression.succesful){
        exec.succesful = FALSE;
        return exec;
    }

    if(statement->type == Ass){
        data_update_result result = dict_update(dict, statement->variable, inner_expression.data);
        // result == FALSE, then variable has not been initialized
        if(!result.valid){
            if(result.reason == NotInitialized) write_var_not_init_error(statement->variable, statement->line);
            else if(result.reason == DifferentType) write_exec_error("Cannot assign to variable, as expression has different type.", statement->line);

            exec.succesful = FALSE;
            return exec;
        }
    }

    if(statement->type == Arr_Ass){
        expr_eval_result index_expression = eval_expr(statement->expression_2, dict);
        if(!index_expression.succesful){
            exec.succesful = FALSE;
            return exec;
        }
        int index = (int)GET_NUMBER_FROM_DATA(index_expression.data);

        data value = inner_expression.data;

        data_access_result data_array = dict_get(dict, statement->variable);
        if(!data_array.valid){
            write_var_not_init_error(statement->variable, statement->line);
            exec.succesful = FALSE;
            return exec;
        }
        if(!IS_ARRAY(data_array.result)){
            write_exec_error("Variable is not an array.", statement->line);
            exec.succesful = FALSE;
            return exec;
        }
        if(index < 0 || index >= data_array.result.size){
            write_exec_error("Index is not within array bounds.", statement->line);
            exec.succesful = FALSE;
            return exec;
        }

        data_update_result result = data_array_set(data_array.result.value.arr, index, value);

        // result == FALSE, then variable has already been initialized
        if(!result.valid){
            if(result.reason == NotInIndexRange) write_exec_error("Index is not within array bounds.", statement->line);
            else if(result.reason == DifferentType) write_exec_error("Cannot assign to variable, as expression has different type.", statement->line);
            else write_exec_error("Couldn't assign value to variable.", statement->line);

            exec.succesful = FALSE;
            return exec;
        }
    }

    if(statement->type == Var_Init){
        BOOL result = dict_add(dict, statement->variable, inner_expression.data);
        // result == FALSE, then variable has already been initialized
        if(!result){
            write_exec_error("Variable has already been initialized.", statement->line);
            exec.succesful = FALSE;
            return exec;
        }
    }

    if(statement->type == Arr_Init){
        int size = (int)GET_NUMBER_FROM_DATA(inner_expression.data);
        if(size <= 0){
            write_exec_error("Array size can't be zero or negative.", statement->line);
            exec.succesful = FALSE;
            return exec;
        }

        data* array_ptr = data_array_new(size, statement->data_type); 
        data data_array = NUMBER_ARRAY_DATA(array_ptr, size);
        BOOL result = dict_add(dict, statement->variable, data_array);

        // result == FALSE, then variable has already been initialized
        if(!result){
            write_exec_error("Variable has already been initialized.", statement->line);
            exec.succesful = FALSE;
            return exec;
        }
    }

    if(statement->type == Print){
        print_data(inner_expression.data, TRUE);
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
            inner_expression = eval_expr(statement->expression, dict); //Recalc expression
            if(!inner_expression.succesful){
                exec.succesful = FALSE;
                return exec;
            }
        }
    }

    if(statement->type == Block){
        dictionary block_dict = dict_get_extension(dict);
        exec = run_program(statement->inner_stmt_1, &block_dict);
        dict_free(&block_dict);
    }

    if(statement->type == Brk){
        //Do not run next statement
        exec.break_while = TRUE;
        return exec;
    }

    if(statement->type == Stop){
        //Do not run next statement and stop execution
        exec.stop_execution = TRUE;
    }

    if(stop_next_execution(exec)) return exec;
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