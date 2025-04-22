#include "usart.h"
#include "stdio.h"

// Retarget printf to USART2
int __io_putchar(int ch) {
    // HAL_UART_Transmit function transmits a single byte
    HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
    return ch;  // Return the character transmitted
}
