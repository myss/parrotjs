#include <stdlib.h>
#include <assert.h>

#include "mempool.h"

static void doubleCapacity(Mempool mp) {
    mp->pool = (void**) realloc(mp->pool, 2*mp->capacity * sizeof(void*));
    assert(mp->pool);
    mp->capacity *= 2;
}

Mempool mp_newpool(int capacity) {
    Mempool mp;
    
    if (capacity < 1) capacity = 1;
    mp = (Mempool) malloc(sizeof(struct Mempool_));
    assert(mp);
    mp->size = 0;
    mp->pool = (void**) malloc(capacity * sizeof(void*));
    assert(mp->pool);
    mp->capacity = capacity;
    return mp;
}
void mp_register(void* object, Mempool mp) {
    if (mp->size == mp->capacity)
        doubleCapacity(mp);
    mp->pool[mp->size] = object;
    ++mp->size;
}
void* mp_malloc(int size, Mempool mp) {
    void* object = malloc(size);
    if (mp)
        mp_register(object, mp);
    return object;
}
void mp_clear(Mempool mp) {
    void **start, **end;
    assert(mp);
    start = mp->pool - 1;
    end = start + mp->size;
    while (end != start) {
        free(*end);
        end--;
    }
}
void mp_freepool(Mempool mp) {
    mp_clear(mp);
    free(mp->pool);
    free(mp);
}

/*int main() {
    Mempool mp = mp_newpool(1);
    int i;
    for(;;) {
        char* ch = (char*) mp_malloc(100, mp);
        fscanf(stdin, "%s", ch);
        if (! ch[0]) break;
        printf("%s\n", ch);
    }
    mp_freepool(mp);
    return 0;
}*/

