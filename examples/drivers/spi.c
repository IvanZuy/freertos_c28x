//-------------------------------------------------------------------------------------------------
// Simplified SPI driver.
//
//-------------------------------------------------------------------------------------------------
#include "spi.h"
#include "semphr.h"

//-------------------------------------------------------------------------------------------------
#define SPI_BRR        ((200E6 / 4) / 5E6) - 1
#define SPI_CHAR_BITS  8

//-------------------------------------------------------------------------------------------------
static void initPins(void)
{
  EALLOW;

  // Enable internal pull-up for the selected pins
  GpioCtrlRegs.GPBPUD.bit.GPIO58 = 0;  // Enable pull-up on GPIO58 (SPIA_MOSI)
  GpioCtrlRegs.GPBPUD.bit.GPIO59 = 0;  // Enable pull-up on GPIO59 (SPIA_MISO)
  GpioCtrlRegs.GPBPUD.bit.GPIO60 = 0;  // Enable pull-up on GPIO60 (SPIA_CLK)
  GpioCtrlRegs.GPBPUD.bit.GPIO61 = 0;  // Enable pull-up on GPIO61 (SPIA_STE)

  // Set qualification for selected pins to asynch only
  GpioCtrlRegs.GPBQSEL2.bit.GPIO58 = 3;
  GpioCtrlRegs.GPBQSEL2.bit.GPIO59 = 3;
  GpioCtrlRegs.GPBQSEL2.bit.GPIO60 = 3;
  GpioCtrlRegs.GPBQSEL2.bit.GPIO61 = 3;

  //Configure SPI-A pins using GPIO regs
  GpioCtrlRegs.GPBGMUX2.bit.GPIO58 = 3;
  GpioCtrlRegs.GPBMUX2.bit.GPIO58  = 3;	// Configure GPIO58 as SPIA_MOSI
  GpioCtrlRegs.GPBGMUX2.bit.GPIO59 = 3;
  GpioCtrlRegs.GPBMUX2.bit.GPIO59  = 3;	// Configure GPIO59 as SPIA_MISO
  GpioCtrlRegs.GPBGMUX2.bit.GPIO60 = 3;
  GpioCtrlRegs.GPBMUX2.bit.GPIO60  = 3;	// Configure GPIO60 as SPIA_CLK
  GpioCtrlRegs.GPBGMUX2.bit.GPIO61 = 3;
  GpioCtrlRegs.GPBMUX2.bit.GPIO61  = 3;	// Configure GPIO61 as SPIA_STE

  EDIS;
}

//-------------------------------------------------------------------------------------------------
static void initSpia(void)
{
  // Set reset low before configuration changes
  // Clock polarity (0 == rising, 1 == falling)
  // 16-bit character
  // Enable HS mode
  SpiaRegs.SPICCR.bit.SPISWRESET   = 0;
  SpiaRegs.SPICCR.bit.CLKPOLARITY  = 0;
  SpiaRegs.SPICCR.bit.HS_MODE      = 1;
  SpiaRegs.SPICCR.bit.SPICHAR      = (SPI_CHAR_BITS-1);

  // Enable master (0 == slave, 1 == master)
  // Enable transmission (Talk)
  // Clock phase (0 == normal, 1 == delayed)
  // SPI interrupts are disabled
  SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
  SpiaRegs.SPICTL.bit.TALK         = 1;
  SpiaRegs.SPICTL.bit.CLK_PHASE    = 0;
  SpiaRegs.SPICTL.bit.SPIINTENA    = 0;

  // Initialize SPI TX_FIFO register
  SpiaRegs.SPIFFTX.bit.TXFFIL      = 0;
  SpiaRegs.SPIFFTX.bit.TXFFIENA    = 0;
  SpiaRegs.SPIFFTX.bit.TXFFINTCLR  = 1;
  SpiaRegs.SPIFFTX.bit.TXFIFO      = 1;
  SpiaRegs.SPIFFTX.bit.SPIFFENA    = 1;
  SpiaRegs.SPIFFTX.bit.SPIRST      = 1;

  // Initialize SPI RX_FIFO register
  SpiaRegs.SPIFFRX.bit.RXFFIL      = 0x10;
  SpiaRegs.SPIFFRX.bit.RXFFIENA    = 0;
  SpiaRegs.SPIFFRX.bit.RXFFINTCLR  = 1;
  SpiaRegs.SPIFFRX.bit.RXFIFORESET = 1;

  // Initialize SPI CT_FIFO register
  SpiaRegs.SPIFFCT.all = 0;

  SpiaRegs.SPIFFRX.all = 0x2044;
  SpiaRegs.SPIFFCT.all = 0x0;

  // Set the baud rate
  SpiaRegs.SPIBRR.bit.SPI_BIT_RATE = 0;

  // Set FREE bit
  // Halting on a breakpoint will not halt the SPI
  SpiaRegs.SPIPRI.bit.FREE = 1;

  // Release the SPI from reset
  SpiaRegs.SPICCR.bit.SPISWRESET = 1;
}

//-------------------------------------------------------------------------------------------------
static void initDma(void)
{
  
}

//-------------------------------------------------------------------------------------------------
void SPI_open(void)
{
  initPins();
  initSpia();
  initDma();
}

//-------------------------------------------------------------------------------------------------
void SPI_close(void)
{

}

//-------------------------------------------------------------------------------------------------
uint16_t SPI_send(const uint8_t* buff, uint16_t buffSize)
{
  uint16_t i = 0;

  for(i = 0; i < buffSize; i++)
  {
	while(SpiaRegs.SPIFFTX.bit.TXFFST == 0x10) {}
	SpiaRegs.SPITXBUF = buff[i] << (16 - SPI_CHAR_BITS);
  }

  return i;
}

//-------------------------------------------------------------------------------------------------
uint16_t SPI_receive(uint8_t* buff, uint16_t buffSize, TickType_t timeout)
{
  uint16_t i = 0;
  
  return i;
}
