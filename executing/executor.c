#include <stdio.h>
#include <string.h>
#include "executor.h"
#include "../types.h"
#include "../dictionary.h"

#define EXEC_ERROR_MESSAGE_LENGTH 100

char EXEC_ERROR_MESSAGE[EXEC_ERROR_MESSAGE_LENGTH];

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
            strncpy(EXEC_ERROR_MESSAGE, "Divide by zero error.", EXEC_ERROR_MESSAGE_LENGTH);
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
            strncpy(EXEC_ERROR_MESSAGE, "Variable '", EXEC_ERROR_MESSAGE_LENGTH);
            strcat(EXEC_ERROR_MESSAGE, expression->variable);
            strcat(EXEC_ERROR_MESSAGE, "' is not initialized.");
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

execution_result run_statement(stmt* statement, dictionary* dict){
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
}