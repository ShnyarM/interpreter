#include <stdio.h>
#include "executor.h"
#include "../types.h"
#include "../dictionary.h"

static int eval_expr(expr* expression, dictionary* dict){
    if(expression->type == Lit){
        return expression->val;
    }

    if(expression->type == Add){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 + value_2;
    }

    if(expression->type == Sub){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 - value_2;
    }

    if(expression->type == Mul){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 * value_2;
    }

    if(expression->type == Div){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 / value_2;
    }

    if(expression->type == Neg){
        int value_1 = eval_expr(expression->expr_1, dict);

        return -value_1;
    }

    if(expression->type == Var){
        dict_result var_value = dict_get(dict, expression->variable);
        if(var_value.valid) return var_value.result;

        return 0;
    }

    if(expression->type == Bool_Neg){
        int value_1 = eval_expr(expression->expr_1, dict);

        return !value_1;
    }

    if(expression->type == Eq){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 == value_2;
    }

    if(expression->type == Lt){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 < value_2;
    }

    if(expression->type == Lte){
        int value_1 = eval_expr(expression->expr_1, dict);
        int value_2 = eval_expr(expression->expr_2, dict);

        return value_1 <= value_2;
    }

    return -1;
}

execution_result run_statement(stmt* statement, dictionary* dict){
    execution_result exec = {TRUE, FALSE, FALSE};
    if(statement == NULL) return exec;

    if(statement->type == Ass){
        int value = eval_expr(statement->expression, dict);
        dict_add(dict, statement->variable, value);
    }

    if(statement->type == Print){
        int value = eval_expr(statement->expression, dict);
        printf("%d\n", value);
    }

    if(statement->type == If){
        if(eval_expr(statement->expression, dict)){
            exec = run_statement(statement->inner_stmt_1, dict);
        }else{
            exec = run_statement(statement->inner_stmt_2, dict);
        }
    }

    if(statement->type == While){
        while(eval_expr(statement->expression, dict)){
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