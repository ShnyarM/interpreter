#include "types.h"
#include "helper.h"
#include <stdlib.h>

void free_type(data_type* t){
    if(t == NULL) return;
    
    free_type(t->inner_type);
    free(t);
}

data_type* get_plain_type(){
    data_type* t = (data_type*)malloc(sizeof(data_type));
    t->atomic_type = None;
    t->kind = None;
    t->inner_type = NULL;
    return t;
}

data_type* get_atomic_number_type(){
    data_type* t = get_plain_type();
    t->kind = Atomic;
    t->atomic_type = Number;
    return t;
}

// Transfers ownership of type
data_type* get_array_type(data_type* inner){
    data_type* t = get_plain_type();
    t->kind = Array;
    t->inner_type = inner;
    return t;
}

data_type* copy_of_type(data_type* t){
    if(t == NULL) return NULL;

    data_type* copy = get_plain_type();
    copy->atomic_type = t->atomic_type;
    copy->kind = t->kind;

    data_type* inner_copy = copy_of_type(t->inner_type);
    copy->inner_type = inner_copy;

    return copy;
}

BOOL type_equality(data_type* t1, data_type* t2){
    if(t1 == NULL && t2 == NULL) return TRUE;
    if(t1 == NULL || t2 == NULL) return FALSE;

    BOOL same_atomic = t1->atomic_type == t2->atomic_type;
    BOOL same_kind = t1->kind == t2->kind;
    BOOL same_inner = type_equality(t1->inner_type, t2->inner_type);

    return same_atomic && same_kind && same_inner;
}