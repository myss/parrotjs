#ifndef _LIST_H_
#define _LIST_H_

#include "mempool.h"

typedef void ForEachFunc(void* elem);
typedef void ForEachFunc_index(void* elem, int index);

struct List_;
typedef struct List_ *PjsList;

struct ListEntry_;
typedef struct ListEntry_ *ListEntry;

struct List_ {
    ListEntry start, end;
};
struct ListEntry_ {
    void* elem;
    ListEntry next;
};

PjsList newList(Mempool mp);
void pushFront(void* elem, PjsList list, Mempool mp);
void pushBack(void* elem, PjsList list, Mempool mp);
void* popFront(PjsList list);

int pjslist_length(PjsList list);

void freeList(PjsList list);

/* Destructs list a */
PjsList appendList(PjsList a, PjsList b);

void foreachElem(ForEachFunc func, PjsList list);
void foreachElem_index(ForEachFunc_index func, PjsList list);

#endif
