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

BOOL stop_next_execution(execution_result e){
    return (!e.succesful || e.stop_execution || e.break_while);
}

static expr_eval_result eval_expr(expr* expression, dictionary* dict){
    expr_eval_result res = {TRUE, 0};
    if(expression == NULL) return res;

    expr_eval_result value1 = eval_expr(expression->expr_1, dict);
    expr_eval_result value2 = eval_expr(expression->expr_2, dict);

    if(!value1.succesful || !value2.succesful){
        res.succesful = FALSE;
        return res;
    }

    if(expression->type == Lit){
        res.value = expression->val;
    }

    if(expression->type == Add){
        res.value = value1.value + value2.value;
    }

    if(expression->type == Sub){
        res.value = value1.value - value2.value;
    }

    if(expression->type == Mul){
        res.value = value1.value * value2.value;
    }

    if(expression->type == Div){
        if(value2.value == 0){
            write_exec_error("Divide by zero error.", expression->line);
            res.succesful = FALSE;
            return res;
        }
        res.value = value1.value / value2.value;
    }

    if(expression->type == Neg){
        res.value = -value1.value;
    }

    if(expression->type == Var){
        dict_result var_value = dict_get(dict, expression->variable);
        if(var_value.valid) res.value = var_value.result;
        else{
            char buffer[EXEC_ERROR_MESSAGE_LENGTH];
            strncpy(buffer, "Variable '", EXEC_ERROR_MESSAGE_LENGTH);
            strcat(buffer, expression->variable);
            strcat(buffer, "' is not initialized.");
            write_exec_error(buffer, expression->line);
            res.succesful = FALSE;
            return res;
        }
    }

    if(expression->type == Bool_Neg){
        res.value = !value1.value;
    }

    if(expression->type == Eq){
        res.value = value1.value == value2.value;
    }

    if(expression->type == Lt){
        res.value = value1.value < value2.value;
    }

    if(expression->type == Lte){
        res.value = value1.value <= value2.value;
    }

    return res;
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
        dict_add(dict, statement->variable, inner_expression.value);
    }

    if(statement->type == Print){
        printf("%d\n", inner_expression.value);
    }

    if(statement->type == If){
        if(inner_expression.value){
            exec = run_program(statement->inner_stmt_1, dict);
        }else{
            exec = run_program(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == While){
        while(inner_expression.value){
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

/*execution_result run_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE};
    if(statement == NULL) return exec;

    expr_eval_result inner_expression = eval_expr(statement->expression, dict);
    if(!inner_expression.succesful){
        exec.succesful = FALSE;
        return exec;
    }

    if(statement->type == Ass){
        dict_add(dict, statement->variable, inner_expression.value);
    }

    if(statement->type == Print){
        printf("%d\n", inner_expression.value);
    }

    if(statement->type == If){
        if(inner_expression.value){
            exec = run_statement(statement->inner_stmt_1, dict);
        }else{
            exec = run_statement(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == While){
        while(inner_expression.value){
            exec = run_statement(statement->inner_stmt_1, dict);
            if(!exec.succesful || exec.stop_execution || exec.break_while){
                exec.break_while = FALSE;
                break;
            }
            inner_expression.value = eval_expr(statement->expression, dict).value; //Recalc expression
        }
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

    if(!exec.succesful || exec.stop_execution || exec.break_while) return exec;
    return run_statement(statement->next_statement, dict);   
}*/