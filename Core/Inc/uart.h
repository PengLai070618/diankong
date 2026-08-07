#ifndef __UART_H
#define __UART_H

#include "main.h"
#include <stdio.h>   

void UART_SendByte(uint8_t ch);
void UART_SendString(char *str);
int fputc(int ch, FILE *f);   

#endif