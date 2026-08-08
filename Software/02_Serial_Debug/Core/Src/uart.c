#include "uart.h"
#include "usart.h"   

void UART_SendByte(uint8_t ch)
{
    HAL_UART_Transmit(&huart1, &ch, 1, 100);
}

void UART_SendString(char *str)
{
    while (*str)
    {
        UART_SendByte((uint8_t)*str++);
    }
}

int fputc(int ch, FILE *f)
{
    UART_SendByte((uint8_t)ch);
    return ch;
}