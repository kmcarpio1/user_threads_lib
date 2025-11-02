#include "log.h"
#include <time.h> 
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

static const char* log_file = DEFAULT_LOG_PATH;

static const char* security_levels[] = {
    "EMERGENCY",
    "ALERT",
    "CRITICAL",
    "ERROR", 
    "WARNING",
    "NOTICE",
    "INFORMATIONAL",
    "DEBUG"
};


//static const char* security_color[] = {
//    "\033[91m",      /* Bright Red */
//    "\033[35m",      /* Magenta */
//    "\033[33m",      /* Yellow */
//    "\033[31m",      /* Red */ 
//    "\033[33m",      /* Orange */
//    "\033[34m",      /* Blue */
//    "\033[32m",      /* Greeen */
//    "\033[36m"      /* Cyan */
//};
//#define RESET   "\033[0m"

void cleaning_file(){
    FILE* fd;
    if((fd=fopen(log_file, "w")) == NULL){
        fprintf(stderr, "\033[31m Error: \033[0m Impossible to open the log file.");
    }
    fclose(fd);
}



void new_entry_log(LOGGING_LEVELS level, const char* function_name ,int thread_number, const char* message){
    
    FILE* fd;
    if((fd=fopen(log_file, "a")) == NULL){
        fprintf(stderr, "\033[31m Error: \033[0m Impossible to open the log file.");
    }

    const time_t t = time(NULL);
    char* date = ctime(&t);

    if(message != NULL){
        fprintf(fd, "%s\nState: %s \nAt %sOn thread number %d \nMessage: %s \n\n",  function_name,  security_levels[level], date,thread_number , message);
    }

    else{
        fprintf(fd, "%s\nState: %s \nAt %sOn thread number %d\n\n",  function_name,  security_levels[level], date,thread_number);
    }

    fclose(fd);
}


