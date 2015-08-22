#ifndef __LOGGER_H__
#define __LOGGER_H__

#include "c_types.h"

typedef enum LogLevel {
    DEBUG, INFO, WARNING, ERROR
} LogLevel; 

void log_init(LogLevel level);

void logln_debug(const uint8_t message[]); 
void logln_info(const uint8_t message[]);
void logln_warning(const uint8_t message[]);
void logln_error(const uint8_t message[]);

void log_debug(const uint8_t message[]); 
void log_info(const uint8_t message[]);
void log_warning(const uint8_t message[]);
void log_error(const uint8_t message[]);

#endif
