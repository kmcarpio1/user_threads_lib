#include "thread_queue.h"
#include <valgrind/valgrind.h>
#include <stdlib.h>

#define THREAD_STACK_SIZE 4096

TAILQ_HEAD(tailhead, threadblock);
struct tailhead queue_head;

struct threadblock * MAIN;
struct threadblock * CURRENT;

/******************************************************/

void auto_exit(void *(*func)(void *), void *args) {
    thread_exit(func(args));
}

struct threadblock * tblock_init(void *(*func)(void *), void *funcarg){

    struct threadblock *new_block = (struct threadblock *)malloc(sizeof(struct threadblock));
    
    // Handle error (non sufficient memory)
    if (new_block == NULL) return NULL;

    new_block->status = READY;

    // Initialize info for new context
    if ( getcontext(&new_block->context) == -1 ) {
		printf("Error while getting context..exiting\n");
		exit(EXIT_FAILURE);
	}

    // Precise new context information
    new_block->context.uc_stack.ss_flags = 0;
    new_block->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    new_block->context.uc_stack.ss_sp = malloc(THREAD_STACK_SIZE);

    // Handle error (non sufficient memory)
    if (new_block->context.uc_stack.ss_sp == NULL) return NULL;

    new_block->context.uc_link = NULL;
    new_block->joined_by = NULL;
    new_block->return_value = NULL;
    new_block->valgrind_id = VALGRIND_STACK_REGISTER(new_block->context.uc_stack.ss_sp,
                                               new_block->context.uc_stack.ss_sp + new_block->context.uc_stack.ss_size);
    

    for (int i = 0; i < SIGMASK_L; i++){
        new_block->SIGMASK[i]=101;
    }

    // initializes the HANDLING functions
    new_block -> HANDLERS[TH_SIGKILL] = handle_tsigkill;
    for (int i =1; i<SIGMASK_L ; i++){
        new_block -> HANDLERS[i] = handle_not_defined; 
    }

    // printf("trace : tblock_init. SIGMASK[0] = %d\n", new_block->SIGMASK[0]);

    //
    // Mapping new context with parameter function
    makecontext(&new_block->context, (void (*)(void)) auto_exit, 2, func, funcarg);

    return new_block;
}

void tblock_free(struct threadblock * block){
    if (block != MAIN) {
        VALGRIND_STACK_DEREGISTER(block->valgrind_id);
        free(block->context.uc_stack.ss_sp);
    }
    free(block);
}

void tblock_print(struct threadblock * block){
    char * statusstring = "";
    switch (block->status)
    {
        case RUNNING:
            statusstring = "RUN";
            break;
        
        case WAITING:
            statusstring = "WAIT";
            break;

        case READY:
            statusstring = "READY";
            break;

        case TERMINATED:
            statusstring = "TERM";
            break;

        default:
            break;
    }
    printf("%p/%s || ", block, statusstring);
}

/******************************************************/

void tqueue_init(){
    TAILQ_INIT(&queue_head);
}

void tqueue_print(){
    struct threadblock *current_block;
    printf("Thread Queue Contents:\n");
    
    tblock_print(CURRENT);
    TAILQ_FOREACH(current_block, &queue_head, next) {
        tblock_print(current_block);
        // You can print more information about the threadblock if needed
    }
    printf("\n");
}

struct threadblock * tqueue_pull(){
    if (TAILQ_EMPTY(&queue_head)) return NULL;
    struct threadblock * toreturn = TAILQ_LAST(&queue_head, tailhead);
    TAILQ_REMOVE(&queue_head, toreturn, next);
    return toreturn;
}

void tqueue_push(struct threadblock * block){
    TAILQ_INSERT_HEAD(&queue_head, block, next);
}

inline struct threadblock * tqueue_get_main(){
    return MAIN;
}

void tqueue_set_main(struct threadblock * block){
    MAIN = block;
}

inline struct threadblock * tqueue_get_current(){
    return CURRENT;
}

void tqueue_set_current(struct threadblock * block){
    CURRENT = block;
}

int tqueue_is_empty(){
    return TAILQ_EMPTY(&queue_head);
}
