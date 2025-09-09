#include "types.h"
#include "../helper.h"
#include <stdlib.h>
#include <stdio.h>

data_type GLOBAL_INVALID_TYPE = (data_type){Type_Kind_None, Type_Atomic_None, NULL, TRUE};
data_type GLOBAL_ANY_TYPE = (data_type){Type_Kind_Any, Type_Atomic_None, NULL, TRUE};
data_type GLOBAL_ATOMIC_NUMBER_TYPE = (data_type){Type_Kind_Atomic, Type_Atomic_Number, NULL, TRUE};
data_type GLOBAL_FUNCTION_TYPE = (data_type){Type_Kind_Function, Type_Atomic_None, NULL, TRUE};

data_type* types_get_plain(){
    data_type* t = (data_type*)malloc(sizeof(data_type));
    t->atomic_type = Type_Atomic_None;
    t->kind = Type_Kind_None;
    t->inner_type = NULL;
    t->global = FALSE;
    return t;
}

// Transfers ownership of type
data_type* types_get_array_type(data_type* inner){
    data_type* t = types_get_plain();
    t->kind = Type_Kind_Array;
    t->inner_type = inner;
    return t;
}

data_type* types_copy(data_type* t){
    if(t == NULL) return NULL;
    if(t->global) return t;

    data_type* copy = types_get_plain();
    copy->atomic_type = t->atomic_type;
    copy->kind = t->kind;

    data_type* inner_copy = types_copy(t->inner_type);
    copy->inner_type = inner_copy;

    return copy;
}

BOOL types_equality(data_type* t1, data_type* t2){
    if(t1 == NULL && t2 == NULL) return TRUE;
    if(t1 == NULL || t2 == NULL) return FALSE;

    BOOL same_atomic = t1->atomic_type == t2->atomic_type;
    BOOL same_kind = t1->kind == t2->kind;
    BOOL same_inner = types_equality(t1->inner_type, t2->inner_type);

    return same_atomic && same_kind && same_inner;
}

BOOL types_can_be_assigned_to(data_type* lhs, data_type* rhs){
    if(lhs->kind == Type_Kind_Any) return TRUE;
    if(types_equality(lhs, rhs)) return TRUE;
    return FALSE;
}

BOOL types_is_array(data_type* t){
    return t->kind == Type_Kind_Array;
}

BOOL types_is_number(data_type* t){
    return t->kind == Type_Kind_Atomic && t->atomic_type == Type_Atomic_Number;
}

BOOL types_is_any(data_type* t){
    return t->kind == Type_Kind_Any;
}


void free_type(data_type* t){
    if(t == NULL) return;
    //printf("%p\n\n", t);
    if(t->global) return;

    free_type(t->inner_type);
    free(t);
}