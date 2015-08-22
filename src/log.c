#include "log.h"
#include "user_interface.h"
#include "uart_register.h"
#include "osapi.h"

void disable_debug_messages();
void nop(char);
void logstr(const uint8_t message[], LogLevel level);
void logln(const uint8_t message[], LogLevel level);
void logchar(uint8_t character);

LogLevel log_level;

void log_init(LogLevel level) {
    log_level = level;
    disable_debug_messages();
    uart_div_modify(0, UART_CLK_FREQ/115200); // Set the UART baud rate
    log_error("\033[2J\r\n"); // Clear the screen
}

void disable_debug_messages() {
    os_install_putc1(nop);    
}

void nop(char c) {
}

void logln_debug(const uint8_t message[]) {
    logln(message, DEBUG);
}

void logln_info(const uint8_t message[]) {
    logln(message, INFO);
}

void logln_warning(const uint8_t message[]) {
    logln(message, WARNING);
}

void logln_error(const uint8_t message[]) {
    logln(message, ERROR);
}

void log_debug(const uint8_t message[]) {
    logstr(message, DEBUG);
}

void log_info(const uint8_t message[]) {
    logstr(message, INFO);
}

void log_warning(const uint8_t message[]) {
    logstr(message, WARNING);
}

void log_error(const uint8_t message[]) {
    logstr(message, ERROR);
}

void logln(const uint8_t message[], LogLevel level) {
    logstr(message, level);
    logstr("\n\r", level);
}

void logstr(const uint8_t message[], LogLevel level) {
    if(log_level > level)
        return;

    int i;
    for(i = 0; i < strlen(message); i++)
        logchar(message[i]);
}

void logchar(uint8_t character) {
    while (true) {
        uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
            break;
    }
    WRITE_PERI_REG(UART_FIFO(0) , character);
}
