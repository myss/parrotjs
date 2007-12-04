#ifndef _MEMPOOL_H_
#define _MEMPOOL_H_

struct Mempool_;
typedef struct Mempool_ *Mempool;

struct Mempool_ {
    void** pool;
    int capacity;
    int size;
};

Mempool mp_newpool(int capacity);
void mp_register(void* object, Mempool mp);
void* mp_malloc(int size, Mempool mp);
void mp_clear(Mempool mp);
void mp_freepool(Mempool mp);

#endif
