#ifndef QUICK_LISTS_H_INCLUDED
#define QUICK_LISTS_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>

#define Tptr void*
#define TList Tptr* //void**

typedef struct {

    //unused for now, will allow to not realloc the list each insertion
    //by reallocating something like 10 elements in advance.
    //int size;

    int count;
    TList elements;

} TList_wrapper; //maybe this should be a ptr and allocated, but why do complex stuff ?

TList_wrapper ql_create_list();
void ql_delete_list(TList_wrapper* list);
void ql_append(TList_wrapper* list, Tptr element);
void ql_remove(TList_wrapper* list, Tptr element);

#endif // QUICK_LISTS_H_INCLUDED
