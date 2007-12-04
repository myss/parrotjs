#include <stdlib.h>
#include "list.h"
#include "mempool.h"

PjsList newList(Mempool mp) {
    PjsList list = mp_malloc(sizeof(struct List_), mp);
    list->start = list->end = 0;
    return list;
}
static ListEntry newListEntry(void* elem, Mempool mp) {
    ListEntry entry = mp_malloc(sizeof(struct ListEntry_), mp);
    entry->elem = elem;
    entry->next = 0;
    return entry;
}
void pushFront(void* elem, PjsList list, Mempool mp) {
    ListEntry entry = newListEntry(elem, mp);
    if (list->start) {
        entry->next = list->start;
        list->start = entry;
    } else {
        list->start = list->end = entry;
    }
}
void pushBack(void* elem, PjsList list, Mempool mp) {
    ListEntry entry = newListEntry(elem, mp);
    entry->next = 0;
    if (list->end) {
        list->end->next = entry;
        list->end = entry;
    } else {
        list->start = list->end = entry;
    }
}
void* popFront(PjsList list) {
    ListEntry old;
    void* elem;
    
    if (list->start == 0)
        return 0;
    old = list->start;
    elem = old->elem;
    list->start = old->next;
    free(old);
    return elem;
}
int pjslist_length(PjsList list) {
    int len;
    ListEntry entry;
    
    if (! list) 
        return 0;
        
    len = 0;
    entry = list->start;
    while (entry) {
        len++;
        entry = entry->next;
    }
    return len;
}

void freeList(PjsList list) {
    ListEntry entry = list->start;
    while(entry) {
        ListEntry next = entry->next;
        free(entry);
        entry = next;
    }
    free(list);
}

PjsList appendList(PjsList a, PjsList b) {
    a->end->next = b->start;
    a->end = b->end;
    return a;
}
void foreachElem(ForEachFunc func, PjsList list) {
    ListEntry entry = list->start;
    while(entry) {
        func(entry->elem);
        entry = entry->next;
    }
}
void foreachElem_index(ForEachFunc_index func, PjsList list) {
    ListEntry entry = list->start;
    int i = 0;
    while(entry) {
        func(entry->elem, i);
        entry = entry->next;
        i++;
    }
}
