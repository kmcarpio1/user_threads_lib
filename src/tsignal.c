#include "tsignal.h"
#include "thread_queue.h"


void tsignal(int sig, void * (*func)()){
    struct threadblock * tb = (struct threadblock * ) thread_self();
    if (sig != TH_SIGKILL){
        tb -> HANDLERS[sig] = func;
    }
}

int tsignal_send(thread_t * th, int sig){
    if ( sig < 0 || sig >= SIGMASK_L ){
        return -1;
    }
    struct threadblock * t = (struct threadblock * ) *th;
    t->SIGMASK[sig] = 1;
    return 1;
}

void tsignal_handling_header(){
    struct threadblock * tb = (struct threadblock * ) thread_self();

        for (int i = 0; i < SIGMASK_L ; i++){
            if (tb -> SIGMASK[i] == 1 ){
                if (tb == tqueue_get_main()){
                    printf("sending signals to main is forbidden\n.");
                    tb -> SIGMASK[i] = 0;// sets bitmask to initial value
                }
                else{
                    tb -> HANDLERS[i]();// executes the appropriate handler
                    tb -> SIGMASK[i] = 0;// sets bitmask to initial value
                }
            }
        }

}

void * handle_tsigkill(){
    thread_exit(NULL);
    return (void*) 0xdeadbeef; //shut up commpiler !!!
}

void * handle_not_defined(){
    printf("no handler defined for this signal\n");
    return (void*) 0xdeadbeef; //shut up commpiler !!!
}


