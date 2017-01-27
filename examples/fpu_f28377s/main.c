#include <string.h>
#include <stdio.h>
#include "F28x_Project.h"     // Device Headerfile and Examples Include File
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "uart.h"
#include "fpu_test.h"

#define STACK_SIZE  256U
#define RED         0xDEADBEAF
#define BLUE        0xBAADF00D
#define RX_BUFF_SIZE 10

static StaticTask_t redTaskBuffer;
static StackType_t  redTaskStack[STACK_SIZE];

static StaticTask_t blueTaskBuffer;
static StackType_t  blueTaskStack[STACK_SIZE];

static StaticTask_t idleTaskBuffer;
static StackType_t  idleTaskStack[STACK_SIZE];

static char strTaskRed[30];
static char strTaskBlue[30];

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
	uint16_t counter = 0;
	volatile double pi_first;
	volatile double pi_second;

    for(;;)
    {
    	// SOFT: 90mS cycle.
    	// HARD: 35mS cycle.
        pi_first  = compute_pi_machin(1.0/5, 1.0/239);
        pi_second = compute_pi_machin(1.0/5, 1.0/239);
        if(pi_first != pi_second)
        {
        	ledToggle((uint32_t)pvParameters);
        	sprintf(strTaskRed, "R1: %5.4f != %5.4f\n\r", pi_first, pi_second);
        	UART_send((uint8_t*)strTaskRed, strlen(strTaskRed));
        }

        if(++counter == 100)
        {
        	ledToggle((uint32_t)pvParameters);
        	strcpy(strTaskRed, "R1\n\r");
        	UART_send((uint8_t*)strTaskRed, strlen(strTaskRed));
        	counter = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void LED_TaskBlue(void * pvParameters)
{
	uint16_t counter = 0;
	volatile double pi_first;
	volatile double pi_second;

    for(;;)
    {
    	// SOFT: 125mS cycle.
    	// HARD: 46mS cycle.
        pi_first  = compute_pi_gauss(1.0/18, 1.0/57, 1.0/239);
        pi_second = compute_pi_gauss(1.0/18, 1.0/57, 1.0/239);
        if(pi_first != pi_second)
        {
        	ledToggle((uint32_t)pvParameters);
        	sprintf(strTaskBlue, "R2: %5.4f != %5.4f\n\r", pi_first, pi_second);
        	UART_send((uint8_t*)strTaskBlue, strlen(strTaskBlue));
        }

        if(++counter == 100)
        {
        	ledToggle((uint32_t)pvParameters);
        	strcpy(strTaskBlue, "R2\n\r");
        	UART_send((uint8_t*)strTaskBlue, strlen(strTaskBlue));
        	counter = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &idleTaskBuffer;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = STACK_SIZE;
}

//-------------------------------------------------------------------------------------------------
void main(void)
{
    // Step 1. Initialize System Control:
    // PLL, WatchDog, enable Peripheral Clocks
    // This example function is found in the F2837xS_SysCtrl.c file.
    InitSysCtrl();

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

    InitPieVectTable();

    // Initialize the PIE vector table with pointers to the shell Interrupt
    // Service Routines (ISR).
    // This will populate the entire table, even if the interrupt
    // is not used in this example.  This is useful for debug purposes.
    // The shell ISR routines are found in F2837xS_DefaultIsr.c.
    // This function is found in F2837xS_PieVect.c.
    InitPieVectTable();

    UART_open();

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
                      STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * ) BLUE,      // Parameter passed into the task.
                      tskIDLE_PRIORITY + 1, // Priority at which the task is created.
                      blueTaskStack,        // Array to use as the task's stack.
                      &blueTaskBuffer );    // Variable to hold the task's data structure.

    vTaskStartScheduler();
}
