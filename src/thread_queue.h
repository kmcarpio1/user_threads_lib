#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include "thread.h"
#include "tsignal.h"
#include <sys/queue.h>
#include <valgrind/valgrind.h>

#include <ucontext.h>

enum thread_status {
    RUNNING,
    READY,
    WAITING,
    TERMINATED
};

struct threadblock {
    enum thread_status status;
    ucontext_t context;
    struct threadblock * joined_by;
    void * return_value;
    int valgrind_id;
    TAILQ_ENTRY(threadblock) next;
    int SIGMASK[SIGMASK_L];    
    void * (*HANDLERS[SIGMASK_L])();
};

/******************************************************/

struct threadblock * tblock_init();

void tblock_free(struct threadblock * block);

/******************************************************/

void tqueue_init();

void tqueue_print();

struct threadblock * tqueue_pull();

void tqueue_push(struct threadblock * block);

struct threadblock * tqueue_get_main();

void tqueue_set_main(struct threadblock * block);

struct threadblock * tqueue_get_current();

void tqueue_set_current(struct threadblock * block);

int tqueue_is_empty();

#endif /* THREAD_QUEUE_H */