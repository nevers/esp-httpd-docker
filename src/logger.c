#include "logger.h"
#include "user_interface.h"
#include "uart_register.h"
#include "osapi.h"
//TODO introduce log levels
//TODO rename to log_info, log_error, log_debug
void disableDebugMessages();
void nop(char);

void log_init() {
    disableDebugMessages();
    uart_div_modify(0, UART_CLK_FREQ/115200); // Set the UART baud rate
    print("\033[2J"); // Clear the screen
}

void disableDebugMessages() {
    os_install_putc1(nop);    
}

void nop(char c) {
}

void println(uint8_t message[]) {
    print(message);
    print("\n\r");
}

void print(uint8_t* message) {
    int i;
    for(i = 0; i < strlen(message); i++)
        print_char(message[i]);
}

void print_int(int number) {
    char numberString[8];
    os_sprintf(numberString, "%d", number);
    print(numberString);
}

void print_char(uint8_t character) {
    while (true) {
        uint32_t fifo_cnt = READ_PERI_REG(UART_STATUS(0)) & (UART_TXFIFO_CNT << UART_TXFIFO_CNT_S);
        if ((fifo_cnt >> UART_TXFIFO_CNT_S & UART_TXFIFO_CNT) < 126)
            break;
    }
    WRITE_PERI_REG(UART_FIFO(0) , character);
}
