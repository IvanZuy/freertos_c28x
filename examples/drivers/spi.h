#ifndef __SPI_H__
#define __SPI_H__

#include <F28x_Project.h>
#include "FreeRTOS.h"

void     SPI_open(void);
void     SPI_close(void);
uint16_t SPI_send(uint8_t* buff, uint16_t buffSize, TickType_t timeout);
uint16_t SPI_sendByte(uint16_t byte);
uint16_t SPI_receive(uint8_t* buff, uint16_t buffSize, TickType_t timeout);
void     SPI_setCsHigh(void);
void     SPI_setCsLow(void);
void     SPI_setClockFreq(uint32_t freqHz);

#endif // __SPI_H__
