#include "types.h"
#include "../helper.h"
#include <stdlib.h>
#include <stdio.h>

data_type GLOBAL_INVALID_TYPE = (data_type){Type_Kind_None, NULL, TRUE};
data_type GLOBAL_ANY_TYPE = (data_type){Type_Kind_Any, NULL, TRUE};
data_type GLOBAL_NUMBER_TYPE = (data_type){Type_Kind_Number, NULL, TRUE};
data_type GLOBAL_STRING_TYPE = (data_type){Type_Kind_String, NULL, TRUE};
data_type GLOBAL_FUNCTION_TYPE = (data_type){Type_Kind_Function, NULL, TRUE};

data_type* types_get_plain(){
    data_type* t = (data_type*)malloc(sizeof(data_type));
    t->kind = Type_Kind_None;
    t->inner_type = NULL;
    t->global = FALSE;
    return t;
}

data_type* types_get_extension(data_type* inner, data_type_kind extension_type){
    data_type* t = types_get_plain();
    t->kind = extension_type;
    t->inner_type = inner;
    return t;
}

// Transfers ownership of type
data_type* types_get_array_type(data_type* inner){
    return types_get_extension(inner, Type_Kind_Array);
}

data_type* types_copy(data_type* t){
    if(t == NULL) return NULL;
    if(t->global) return t;

    data_type* copy = types_get_plain();
    copy->kind = t->kind;

    data_type* inner_copy = types_copy(t->inner_type);
    copy->inner_type = inner_copy;

    return copy;
}

BOOL types_equality(data_type* t1, data_type* t2){
    if(t1 == NULL && t2 == NULL) return TRUE;
    if(t1 == NULL || t2 == NULL) return FALSE;

    BOOL same_kind = t1->kind == t2->kind;
    BOOL same_inner = types_equality(t1->inner_type, t2->inner_type);

    return same_kind && same_inner;
}

BOOL types_can_be_assigned_to(data_type* lhs, data_type* rhs){
    if(lhs == NULL && rhs == NULL) return TRUE;
    if(lhs == NULL || rhs == NULL) return FALSE;

    if(lhs->kind == Type_Kind_Any) return TRUE;
    if(lhs->kind == rhs->kind && types_can_be_assigned_to(lhs->inner_type, rhs->inner_type)) return TRUE;

    return FALSE;
}

// creates a new type, which is the most common type of the two types
data_type* types_most_common_type(data_type* t1, data_type* t2){
    if(t1 == NULL || t2 == NULL) return NULL;
    
    if(types_equality(t1, t2)) return types_copy(t1);
    if(t1->kind == t2->kind) return types_get_extension(types_most_common_type(t1->inner_type, t2->inner_type), t1->kind);

    return &GLOBAL_ANY_TYPE;
}

BOOL types_is_array(data_type* t){
    return t->kind == Type_Kind_Array;
}

BOOL types_is_number(data_type* t){
    return t->kind == Type_Kind_Number;
}

BOOL types_is_string(data_type* t){
    return t->kind == Type_Kind_String;
}

BOOL types_is_any(data_type* t){
    return t->kind == Type_Kind_Any;
}

BOOL types_is_invalid(data_type* t){
    return t->kind == Type_Kind_None;
}


void free_type(data_type* t){
    if(t == NULL) return;
    //printf("%p\n\n", t);
    if(t->global) return;

    free_type(t->inner_type);
    free(t);
}