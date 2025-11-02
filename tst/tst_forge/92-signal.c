#include "tsignal.h"
#include "thread.h"
#include <unistd.h>
#include <assert.h>
#include "thread_queue.h"


void * thfunc1(void * th){
    thread_t * t =  th;
    thread_t self = thread_self();

    void * custom_signal_print(){
        printf("SIGUSR signal t 1 !  \n");
        return (void*) 0xdeadbeef;
    }

    tsignal(TH_SIGUSR,custom_signal_print);


    printf("\n");
    printf("t 1 s'execute\n");
    printf("t 1 s'envoie deux fois un SIGUSR\n");
    printf("\n");
    tsignal_send(&self,TH_SIGUSR);
    tsignal_send(&self,TH_SIGUSR);
    thread_yield();
    printf("t 1 s'execute et tue t 2\n");
    tsignal_send(t, TH_SIGKILL);
    printf("t1 yield, on vérifie que le sigmask est remis à 0\n ");
    thread_yield();
    printf("le main est vivant\n");
    
    return (void*) "t 1 a fini";
}

void * thfunc2( void * th){
    thread_t * t =  th;

    void * custom_signal_print(){
        printf("SIGKILL signal for t 2  !  \n");
        return (void*) 0xdeadbeef;
    }

    printf("t 2 s'exexcute\n");

    printf("pour survivre, t 2 tente de personnaliser le SIGKILL\n");
    tsignal(TH_SIGKILL, custom_signal_print);

    printf("on va tous mourir un jour, t 2 tente de tuer le main\n");
    tsignal_send(t, TH_SIGKILL);

    printf("\n");
    thread_yield();

    return (void*) "t 2 a miraculeusement survécu";
}

void * thfunc4(void * th){

    thread_t * t3 =  th;


    printf("t 4 s'execute et tue t 3, qui devait le join\n");


    tsignal_send(t3, TH_SIGKILL);
    // //tquee_print();
    thread_yield();

    return (void*) "t 4 a finit";
}

void * thfunc3(void * th){
    int err;
    thread_t th4;
    thread_t t_self = thread_self();
    printf("t 3 créer un thread t 4 et yield \n");
     //tquee_print();
    err =  thread_create(&th4,thfunc4,&t_self);
    assert(!err);
    *(thread_t **)th = th4;

    thread_yield();
    printf("t 3 tente de join t mais il est mort\n");
    err = thread_join(th4,NULL);
    assert(!err);

    printf("\n");
    return (void*) "t 3 a finit";
}



int main(){
    thread_t th1,th2,th3;
    thread_t t_main = thread_self();

    void * retval1 = NULL ;
    void * retval2 = NULL ;
    void * retval3 = NULL ;
    thread_t * th4 = NULL ;
    int err ;
    err = thread_create(&th1,thfunc1, &th2);
    assert(!err);
    err = thread_create(&th2,thfunc2,&t_main);
    assert(!err);
    
    err = thread_join(th2,&retval2);
    assert(!err);
    err = thread_join(th1,&retval1);
    assert(!err);
    printf("retour de t 1 : %s \n", (char*)retval1);
    if(retval2 == NULL){
    printf("retval2 : NULL\n");
    }

    
    printf("\nDeuxième test  : Tuer son père.\n\n");

    err = thread_create(&th3,thfunc3,&th4);
    //tquee_print();
    assert(!err);
    thread_yield();
    // t4 ne sera pas join.
    err = thread_join(th3,retval3);
    assert(!err);
    if(retval3 == NULL){
    printf("retval3 : NULL\n");
    }

    printf("le main est obligé de free t 4 manuellement\n");
    if (th4 != NULL){
        tblock_free((struct threadblock*)th4);
        }
    assert(!err);



}