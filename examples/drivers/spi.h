#ifndef __SPI_H__
#define __SPI_H__

#include <F28x_Project.h>
#include "FreeRTOS.h"

void     SPI_open(void);
void     SPI_close(void);
uint16_t SPI_send(const uint8_t* buff, uint16_t buffSize);
uint16_t SPI_receive(uint8_t* buff, uint16_t buffSize, TickType_t timeout);

#endif // __SPI_H__
