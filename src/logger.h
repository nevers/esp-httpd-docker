#ifndef __LOGGER_H__
#define __LOGGER_H__
#include "c_types.h"

void log_init();
void print(uint8_t* message);
void print_char(uint8_t character);
void print_int(int number);
void println(uint8_t message[]);

#endif
