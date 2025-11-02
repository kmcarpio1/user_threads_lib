#ifndef LOGGER_H
#define LOGGER_H

#include <stdlib.h>
#include <stdbool.h>
#define DEFAULT_LOG_PATH "logging.log"

typedef enum{
    EMERGENCY,
    ALERT,
    CRITICAL,
    ERROR,
    WARNING,
    NOTICE,
    INFORMATIONAL,
    DEBUG
} LOGGING_LEVELS;

/**
 * To erase all information written in logging.log
*/
void cleaning_file();

/**
 * Logger function 
*/
void new_entry_log(LOGGING_LEVELS level, const char* function_name, int thread_number, const char* message);


#endif 