#ifndef USE_PTHREAD

#include "thread.h"
#include "thread_queue.h"
#include <stdlib.h>
#include <errno.h>
#include <valgrind/valgrind.h>
#include <assert.h>



__attribute__ ((constructor)) 
static void staticinit() {
    // Initialize thread queue
    tqueue_init();

    // Initialize the base threadblock
    struct threadblock * main_threadblock = (struct threadblock *)malloc(sizeof(struct threadblock));
    main_threadblock->status = RUNNING;
    main_threadblock->joined_by = NULL;
    main_threadblock->return_value = NULL;

    for (int i = 0; i < SIGMASK_L; i++){
        main_threadblock->SIGMASK[i]=0;
    }

    // initializes the HANDLING functions
    main_threadblock -> HANDLERS[TH_SIGKILL] = handle_tsigkill;
    for (int i =1; i<SIGMASK_L ; i++){
        main_threadblock -> HANDLERS[i] = handle_not_defined; 
    }

    tqueue_set_main(main_threadblock);
    tqueue_set_current(main_threadblock);
}

__attribute__ ((destructor)) 
static void mainkiller() {
    struct threadblock * current = tqueue_get_current();
    if (current != tqueue_get_main()){
        tblock_free(current);
    }
    free(tqueue_get_main());
}

__attribute__((visibility("default")))
extern thread_t thread_self(void){
    return tqueue_get_current();
}


__attribute__((visibility("default")))
int thread_create(thread_t *newthread, void *(*func)(void *), void *funcarg){
    
    struct threadblock * tb = tblock_init(func, funcarg);

    // Handle error (non sufficient memory)
    if (tb == NULL) return EAGAIN;

    tqueue_push(tb);

    *(newthread) = (thread_t) tb;

    // Return 0 if everything is ok
    return EXIT_SUCCESS;

}


__attribute__((visibility("default")))
int thread_yield(void){
    if (tqueue_is_empty()) return 0;
    
    // Get the current 
    struct threadblock * current = (struct threadblock *) thread_self();

    struct threadblock * next_thread = tqueue_pull();

    tqueue_set_current(next_thread);
    tqueue_push(current);
    current->status = READY;
    next_thread->status = RUNNING;

    // Swap between the current context and the next in the queue
    if(swapcontext(&(current->context), &(next_thread->context)) == -1) return -1;
    tsignal_handling_header();

    return EXIT_SUCCESS;

}


int passive_wait(){
    if (tqueue_is_empty()) return 0;
    
    // Get the current 
    struct threadblock * current = (struct threadblock *) thread_self();

    struct threadblock * next_thread = tqueue_pull(current);

    tqueue_set_current(next_thread);
    current->status = WAITING;
    next_thread->status = RUNNING;

    // Swap between the current context and the next in the queue
    if(swapcontext(&(current->context), &(next_thread->context)) == -1) return -1;
    tsignal_handling_header();
    current->status = RUNNING;

    return EXIT_SUCCESS;
}

__attribute__((visibility("default")))
int thread_join(thread_t thread, void **retval) {
    int intval = 0;

    struct threadblock * th = ((struct threadblock *) thread);

    // HANDLE ERROR: already join
    if (th->joined_by != NULL) return EINVAL;

    assert(th->joined_by == NULL);
    th->joined_by = tqueue_get_current();

    // Passive waiting
    if (th->status != TERMINATED){
        intval = passive_wait();
    }
    // EMERGENCY, JOINING A WAITING THREAD MIGHT INDICATE JOIN DEADLOCK
    if (th->status == WAITING){
        struct threadblock * iter = th->joined_by;
        while (iter->joined_by != NULL){
            if (iter->joined_by == th){
                return EDEADLK;
            }
            iter = iter->joined_by;
        }
        intval = passive_wait();
    }
    assert(th->status == TERMINATED);

    // If retval is not NULL, trying to get return value of closing thread
    if (retval != NULL) {
        *retval = th->return_value;
    }

    // Freeing exiting thread
    if (th != tqueue_get_main()){
        tblock_free(th);
    }

    return intval;
}

__attribute__((visibility("default")))
void thread_exit(void *retval) {

    // Setting terminated status
    struct threadblock * current = thread_self();
    current->return_value = retval;
    current->status = TERMINATED;
    // tqueue_print();

    struct threadblock * maint = tqueue_get_main();

    struct threadblock * joined_by = current->joined_by;
    if (joined_by != NULL){
        joined_by->status = READY;
        tqueue_push(joined_by);
    } 
    
    // If current is last thread -> terminates program
    if (tqueue_is_empty()){
        // If current has main context, just exits
        if (current == maint){
            exit(EXIT_SUCCESS);
        } // Else, give hand to main context to terminate program
        else {
            setcontext(&(maint->context));
            exit(EXIT_FAILURE);
        }
    }

    // If current not last thread -> gives hand to next thread
    else {
        struct threadblock * next = tqueue_pull();
        next->status = RUNNING;
        tqueue_set_current(next);
        // If current has main context, ensure you go back to main context
        if (current == maint){
            swapcontext(&(maint->context), &(next->context));
            exit(EXIT_SUCCESS);
        } // Else, never go back to previous context
        else {
            setcontext(&(next->context));
            exit(EXIT_FAILURE);
        }
    }
}

/****************************************************/
/**************** MUTEX THREAD LIST *****************/
/****************************************************/

struct mutex_tlist {
    thread_t thread;
    struct mutex_tlist * next;
};

struct mutex_tlist * mutex_tlist_push(struct mutex_tlist * head, thread_t thread){
    struct mutex_tlist * block = malloc(sizeof(struct mutex_tlist));
    block->thread = thread;
    block->next = head;
    return block;
}

struct mutex_tlist * mutex_tlist_remove(struct mutex_tlist * head, void (*func)(thread_t)){
    if (head==NULL) return NULL;
    thread_t toremove = head->thread;
    if (toremove != NULL){
        func(toremove);
    }
    struct mutex_tlist * tofree = head;
    head = head->next;
    free(tofree);
    tofree = NULL;
    return head;
}

void thread_unlocker(thread_t thread){
    struct threadblock * to_unlock = (struct threadblock *) thread;
    to_unlock->status = READY;
    tqueue_push(to_unlock);
}

void print_mutex(thread_mutex_t *mutex){
    printf("Mutex is %p//", mutex->owner);
    struct mutex_tlist * iter = mutex->threads_blocked;
    while (iter != NULL){
        printf("%p->", iter->thread);
        iter = iter->next;
    }
    printf("\n");
}

/****************************************************/
/********************** MUTEX ***********************/
/****************************************************/


__attribute__((visibility("default")))
int thread_mutex_init(thread_mutex_t *mutex) { 
    mutex->init = 1;
    mutex->owner = NULL;
    mutex->threads_blocked = NULL;
    return 0; 
}

__attribute__((visibility("default")))
int thread_mutex_destroy(thread_mutex_t *mutex) {

    // Handle error: the mutex is currently locked
    if(mutex->owner != NULL) return EBUSY;

    mutex->init = 0;

    return 0; 
}

__attribute__((visibility("default")))
int thread_mutex_lock(thread_mutex_t *mutex) {     

    // Handle error: the mutex has not been properly initialized
    if (mutex->init != 1) return EINVAL;

    thread_t * self = (thread_t) tqueue_get_current();

    // Handle error: the mutex is already locked by the calling thread 
    if (mutex->owner == self) return EDEADLK;

    if (mutex->owner != NULL && mutex->owner != self){
        mutex->threads_blocked = mutex_tlist_push(mutex->threads_blocked, self);
        passive_wait();
    }

    mutex->owner = self;

    return 0;
}

__attribute__((visibility("default")))
int thread_mutex_unlock(thread_mutex_t *mutex) {

    // Handle error: the mutex has not been properly initialized
    if (mutex->init != 1) return EINVAL;

    thread_t * self = (thread_t) tqueue_get_current();

    // Handle error: the calling thread does not own the mutex 
    if (mutex->owner != self) return EPERM;

    if (mutex->threads_blocked == NULL) return 0;
    struct mutex_tlist * next_owner = mutex->threads_blocked;
    mutex->owner = next_owner->thread;
    mutex->threads_blocked = mutex_tlist_remove(next_owner, thread_unlocker);
    
    return 0; 
}

#else /* USE_PTHREAD */

#include "thread.h"

/* No need to define thread_create, it's already defined in pthread.h */

#endif /* USE_PTHREAD */