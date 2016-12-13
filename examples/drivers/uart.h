#ifndef __UART_H__
#define __UART_H__

#include <F28x_Project.h>
#include "FreeRTOS.h"

void     UART_open(void);
void     UART_close(void);
uint16_t UART_send(const uint8_t* buff, uint16_t buffSize);
uint16_t UART_receive(uint8_t* buff, uint16_t buffSize, TickType_t timeout);

#endif // __UART_H__
