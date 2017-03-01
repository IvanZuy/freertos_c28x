#include <stdio.h>
#include <string.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uart.h"
#include "spi.h"
#include "SD.h"
#include "ff.h"
#include "diskio.h"

#define STACK_SIZE       256U
#define STACK_SIZE_BLUE  2048U
#define STACK_SIZE_IDLE  256U
#define RED              0xDEADBEAF
#define BLUE             0xBAADF00D
#define RX_BUFF_SIZE     10

static StaticTask_t redTaskBuffer;
static StackType_t  redTaskStack[STACK_SIZE];

static StaticTask_t blueTaskBuffer;
static StackType_t  blueTaskStack[STACK_SIZE_BLUE];

static StaticTask_t idleTaskBuffer;
static StackType_t  idleTaskStack[STACK_SIZE_IDLE];

#pragma DATA_SECTION(fatFs, "ramgs0");
#pragma DATA_SECTION(fil, "ramgs0");
#pragma DATA_SECTION(buff, "ramgs0");
FATFS    fatFs;
FIL      fil;
uint16_t buff[512];

//-------------------------------------------------------------------------------------------------
DWORD get_fattime (void)
{
  return 0;
}

//-------------------------------------------------------------------------------------------------
void vApplicationStackOverflowHook( TaskHandle_t xTask, signed char *pcTaskName )
{
    while(1);
}

//-------------------------------------------------------------------------------------------------
static void blueLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_WritePin(13, counter & 1);
}

//-------------------------------------------------------------------------------------------------
static void redLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    GPIO_WritePin(12, counter & 1);
}

//-------------------------------------------------------------------------------------------------
static void ledToggle(uint32_t led)
{
    if(RED == led)
    {
        redLedToggle();
    }
    else
    if(BLUE == led)
    {
        blueLedToggle();
    }
}

//-------------------------------------------------------------------------------------------------
void vApplicationSetupTimerInterrupt( void )
{
    // Start the timer than activate timer interrupt to switch into first task.
    EALLOW;
    PieVectTable.TIMER2_INT = &portTICK_ISR;
    EDIS;

    ConfigCpuTimer(&CpuTimer2,
                   configCPU_CLOCK_HZ / 1000000,  // CPU clock in MHz
                   1000000 / configTICK_RATE_HZ); // Timer period in uS
    CpuTimer2Regs.TCR.all = 0x4000;               // Enable interrupt and start timer
    IER |= M_INT14;
}

//-------------------------------------------------------------------------------------------------
void LED_TaskRed(void * pvParameters)
{
    uint16_t br = 0;
    uint8_t buffer[RX_BUFF_SIZE] = {0};

    for(;;)
    {
        br = UART_receive(buffer, RX_BUFF_SIZE, 100 / portTICK_PERIOD_MS);
        if(br > 0)
        {
//            ledToggle((uint32_t)pvParameters);
            UART_send(buffer, br);
        }
    }
}

//-------------------------------------------------------------------------------------------------
void LED_TaskBlue(void * pvParameters)
{
	FRESULT    fresult;
	uint32_t   br;
	uint32_t   totalBytesProcessed;
	TickType_t timeElapsed;

	sd_card_insertion();
	sd_initialization();

	SPI_setClockFreq(10E6);

	sd_read_register(SEND_CSD);		//Read CSD register
	sd_read_register(READ_OCR);		//Read OCR register
	sd_read_register(SEND_CID);		//Read CID register

  for(;;)
  {
    GPIO_WritePin(12, 1);
    disk_initialize(0);
    fresult = f_mount(&fatFs, "", 1);
    GPIO_WritePin(12, 0);

    if(fresult == FR_OK)
    {
      // File read benchmark
//      fresult = f_open(&fil, "test1.bin", FA_READ);
//      if(fresult == FR_OK)
//      {
//        totalBytesProcessed = 0;
//        timeElapsed = xTaskGetTickCount();
//        do
//        {
//          fresult = f_read(&fil, buff, sizeof(buff), &br);
//          totalBytesProcessed += br;
//        }while((fresult == FR_OK) && (br == sizeof(buff)));
//
//        timeElapsed = xTaskGetTickCount() - timeElapsed;
//        snprintf((char*)buff, sizeof(buff),
//                 "BR: %lu in %lu mS, %lu Kb/sec\n\r",
//                 totalBytesProcessed,
//                 timeElapsed,
//                 totalBytesProcessed/timeElapsed);
//        UART_send(buff, strlen(buff));
//        f_close(&fil);
//      }
//
//      vTaskDelay(500 / portTICK_PERIOD_MS);

      // File write benchmark
      fresult = f_open(&fil, "test2.bin", FA_WRITE | FA_CREATE_ALWAYS);
      if(fresult == FR_OK)
      {
        totalBytesProcessed = 0;
        timeElapsed = xTaskGetTickCount();
        do
        {
          fresult = f_write(&fil, buff, sizeof(buff), &br);
          totalBytesProcessed += br;
        }while((fresult == FR_OK) && (totalBytesProcessed < 1500000));

        timeElapsed = xTaskGetTickCount() - timeElapsed;
        snprintf((char*)buff, sizeof(buff),
                 "BW: %lu in %lu mS, %lu Kb/sec\n\r",
                 totalBytesProcessed,
                 timeElapsed,
                 (timeElapsed != 0) ? (totalBytesProcessed/timeElapsed) : 0);
        UART_send(buff, strlen(buff));
        f_close(&fil);
      }

      f_mount(NULL, "", 1);
    }

    ledToggle((uint32_t)pvParameters);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

//-------------------------------------------------------------------------------------------------
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer   = &idleTaskBuffer;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize   = STACK_SIZE_IDLE;
}

//-------------------------------------------------------------------------------------------------
void main(void)
{
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2837xS_SysCtrl.c file.
    InitSysCtrl();

    //EALLOW;
    //ClkCfgRegs.LOSPCP.all = 0;
    //EDIS;

    // Step 2. Initialize GPIO:
    // This example function is found in the F2837xS_Gpio.c file and
    // illustrates how to set the GPIO to it's default state.
    InitCpuTimers();
    InitGpio();
    GPIO_SetupPinMux(12, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinMux(13, GPIO_MUX_CPU1, 0);
    GPIO_SetupPinOptions(12, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_SetupPinOptions(13, GPIO_OUTPUT, GPIO_PUSHPULL);
    GPIO_WritePin(12, 1);
    GPIO_WritePin(13, 1);

    // Step 3. Clear all interrupts and initialize PIE vector table:
    // Disable CPU interrupts
    DINT;

    // Initialize the PIE control registers to their default state.
    // The default state is all PIE interrupts disabled and flags
    // are cleared.
    // This function is found in the F2837xS_PieCtrl.c file.
    InitPieCtrl();

    // Disable CPU interrupts and clear all CPU interrupt flags:
    IER = 0x0000;
    IFR = 0x0000;

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2837xS_DefaultIsr.c.
    // This function is found in F2837xS_PieVect.c.
    InitPieVectTable();

    UART_open();
    SPI_open();

    // Enable global Interrupts and higher priority real-time debug events:
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    // Create the task without using any dynamic memory allocation.
    xTaskCreateStatic(LED_TaskRed,          // Function that implements the task.
                      "Red LED task",       // Text name for the task.
                      STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * ) RED,       // Parameter passed into the task.
                      tskIDLE_PRIORITY + 1, // Priority at which the task is created.
                      redTaskStack,         // Array to use as the task's stack.
                      &redTaskBuffer );     // Variable to hold the task's data structure.

    xTaskCreateStatic(LED_TaskBlue,         // Function that implements the task.
                      "Blue LED task",      // Text name for the task.
                      STACK_SIZE_BLUE,      // Number of indexes in the xStack array.
                      ( void * ) BLUE,      // Parameter passed into the task.
                      tskIDLE_PRIORITY + 1, // Priority at which the task is created.
                      blueTaskStack,        // Array to use as the task's stack.
                      &blueTaskBuffer );    // Variable to hold the task's data structure.

    vTaskStartScheduler();
}
