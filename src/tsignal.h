#ifndef __TSIGNAL_H__
#define __TSIGNAL_H__

#include <ucontext.h>
#include "thread.h"


#define TH_SIGKILL 0 // exits the thread

#define TH_SIGUSR 1 // customed by the user

#define SIGMASK_L 2 

extern void * (*HANDLERS[SIGMASK_L])();// array of handling functions

void tsignal(int sig, void * (*func)());// allows to customize the handling functions(excepting kill)

int tsignal_send(thread_t *t, int sig);

void tsignal_handling_header();// header executed by the thread when it takes hand

void * handle_tsigkill();

void * handle_not_defined();// default handling function for SIGUSR

#endif