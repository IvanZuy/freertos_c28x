//-------------------------------------------------------------------------------------------------
// Simplified UART driver.
//
//-------------------------------------------------------------------------------------------------
#include "uart.h"
#include "semphr.h"

//-------------------------------------------------------------------------------------------------
static SemaphoreHandle_t xTxMutex = NULL;
static StaticSemaphore_t xTxMutexBuffer;
static SemaphoreHandle_t xRxSemaphore = NULL;
static StaticSemaphore_t xRxSemaphoreBuffer;

//-------------------------------------------------------------------------------------------------
interrupt void sciaRx_ISR(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR(xRxSemaphore, &xHigherPriorityTaskWoken);

    PieCtrlRegs.PIEACK.all|=0x100;       // Issue PIE Group 9 ack

    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//-------------------------------------------------------------------------------------------------
void UART_open(void)
{
  // Init OS primitives.
  xTxMutex = xSemaphoreCreateMutexStatic(&xTxMutexBuffer);
  xRxSemaphore = xSemaphoreCreateBinaryStatic(&xRxSemaphoreBuffer);

  // Init pins.
  GPIO_SetupPinMux(84, GPIO_MUX_CPU1, 5);
  GPIO_SetupPinOptions(85, GPIO_OUTPUT, GPIO_ASYNC);
  GPIO_SetupPinMux(85, GPIO_MUX_CPU1, 5);
  GPIO_SetupPinOptions(85, GPIO_INPUT, GPIO_PUSHPULL);

  // Setup FIFO.
  SciaRegs.SCIFFTX.all = 0xE040;
  SciaRegs.SCIFFRX.all = 0x2061;
  SciaRegs.SCIFFCT.all = 0x0;

  // Setup UART parameters.
  SciaRegs.SCICCR.all = 0x0007;   // 1 stop bit,  No loopback
                                  // No parity,8 char bits,
                                  // async mode, idle-line protocol
  SciaRegs.SCICTL1.all = 0x0003;  // enable TX, RX, internal SCICLK,
                                  // Disable RX ERR, SLEEP, TXWAKE
  SciaRegs.SCICTL2.all = 0x0003;
  SciaRegs.SCICTL2.bit.RXBKINTENA = 1;


  // SCIA at 9600 baud
//  SciaRegs.SCIHBAUD.all = 0x0002;
//  SciaRegs.SCILBAUD.all = 0x008B;

  // SCIA at 115200 baud
  if(ClkCfgRegs.LOSPCP.all == 0)
  {
    SciaRegs.SCIHBAUD.all = 0x0000;
    SciaRegs.SCILBAUD.all = 0x00D9;
  }
  else
  {
    SciaRegs.SCIHBAUD.all = 0x0000;
    SciaRegs.SCILBAUD.all = 0x0036;
  }

  SciaRegs.SCICTL1.all = 0x0023;  // Relinquish SCI from Reset


  // Init UART RX interrupt.
  EALLOW;
  PieVectTable.SCIA_RX_INT = &sciaRx_ISR;
  EDIS;
  PieCtrlRegs.PIECTRL.bit.ENPIE = 1;   // Enable the PIE block
  PieCtrlRegs.PIEIER9.bit.INTx1 = 1;   // PIE Group 9, INT1, SCIA_RX
  IER = M_INT9;                        // Enable CPU INT9
}

//-------------------------------------------------------------------------------------------------
void UART_close(void)
{

}

//-------------------------------------------------------------------------------------------------
uint16_t UART_send(const uint8_t* buff, uint16_t buffSize)
{
  uint16_t i = 0;

  if(xSemaphoreTake(xTxMutex, portMAX_DELAY) == pdTRUE)
  {
    for(i = 0; i < buffSize; i++)
    {
      while(SciaRegs.SCIFFTX.bit.TXFFST == 0x10) {}
      SciaRegs.SCITXBUF.all = buff[i];
    }

    xSemaphoreGive(xTxMutex);
  }

  return i;
}

//-------------------------------------------------------------------------------------------------
uint16_t UART_receive(uint8_t* buff, uint16_t buffSize, TickType_t timeout)
{
  uint16_t i = 0;

  while(i < buffSize)
  {
    if(xSemaphoreTake(xRxSemaphore, timeout) == pdTRUE)
    {
      while((SciaRegs.SCIFFRX.bit.RXFFST != 0) && (i < buffSize))
      {
        buff[i++] = SciaRegs.SCIRXBUF.all;
      }
      SciaRegs.SCIFFRX.bit.RXFFINTCLR=1;   // Clear Interrupt flag
    }
    else
    {
      break;
    }
  }
  
  return i;
}
