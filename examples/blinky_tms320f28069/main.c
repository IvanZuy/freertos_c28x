#include "DSP28x_Project.h"     // Device Header file and Examples Include File
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define STACK_SIZE  128U

static StaticTask_t redTaskBuffer;
static StackType_t  redTaskStack[STACK_SIZE];

static StaticTask_t blueTaskBuffer;
static StackType_t  blueTaskStack[STACK_SIZE];

static StaticTask_t idleTaskBuffer;
static StackType_t  idleTaskStack[STACK_SIZE];

static SemaphoreHandle_t xSemaphore = NULL;
static StaticSemaphore_t xSemaphoreBuffer;

//-------------------------------------------------------------------------------------------------
void vApplicationSetupTimerInterrupt( void )
{
    // Start the timer than activate timer interrupt to switch into first task.
    EALLOW;
    PieVectTable.TINT2 = &portTICK_ISR;
    EDIS;

    ConfigCpuTimer(&CpuTimer2,
                   configCPU_CLOCK_HZ / 1000000,  // CPU clock in MHz
                   1000000 / configTICK_RATE_HZ); // Timer period in uS
    CpuTimer2Regs.TCR.all = 0x4000;               // Enable interrupt and start timer
    IER |= M_INT14;
}

//-------------------------------------------------------------------------------------------------
static void blueLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    if(counter & 1)
    {
        GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;
    }
    else
    {
        GpioDataRegs.GPBSET.bit.GPIO39 = 1;
    }
}

//-------------------------------------------------------------------------------------------------
static void redLedToggle(void)
{
    static uint32_t counter = 0;

    counter++;
    if(counter & 1)
    {
        GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
    }
    else
    {
        GpioDataRegs.GPBSET.bit.GPIO34 = 1;
    }
}

//-------------------------------------------------------------------------------------------------
interrupt void timer1_ISR( void )
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    xSemaphoreGiveFromISR( xSemaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

//-------------------------------------------------------------------------------------------------
static void setupTimer1( void )
{
    // Start the timer than activate timer interrupt to switch into first task.
    EALLOW;
    PieVectTable.TINT1 = &timer1_ISR;
    EDIS;

    ConfigCpuTimer(&CpuTimer1,
                   configCPU_CLOCK_HZ / 1000000,  // CPU clock in MHz
                   100000);                       // Timer period in uS
    CpuTimer1Regs.TCR.all = 0x4000;               // Enable interrupt and start timer

    IER |= M_INT13;
}

//-------------------------------------------------------------------------------------------------
void LED_TaskRed(void * pvParameters)
{
    for(;;)
    {
        if(xSemaphoreTake( xSemaphore, portMAX_DELAY ) == pdTRUE)
        {
            blueLedToggle();
        }
    }
}

//-------------------------------------------------------------------------------------------------
void LED_TaskBlue(void * pvParameters)
{
    for(;;)
    {
        redLedToggle();
        vTaskDelay(500 / portTICK_PERIOD_MS);
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
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    while(1);
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

    EALLOW;
    GpioCtrlRegs.GPBDIR.bit.GPIO34 = 1;
    GpioCtrlRegs.GPBDIR.bit.GPIO39 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO34 = 1;
    GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;
    EDIS;

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

    // Enable global Interrupts and higher priority real-time debug events:
    EINT;  // Enable Global interrupt INTM
    ERTM;  // Enable Global realtime interrupt DBGM

    setupTimer1();

    xSemaphore = xSemaphoreCreateBinaryStatic( &xSemaphoreBuffer );

    // Create the task without using any dynamic memory allocation.
    xTaskCreateStatic(LED_TaskRed,          // Function that implements the task.
                      "Red LED task",       // Text name for the task.
                      STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * ) 1,         // Parameter passed into the task.
                      tskIDLE_PRIORITY + 2, // Priority at which the task is created.
                      redTaskStack,         // Array to use as the task's stack.
                      &redTaskBuffer );     // Variable to hold the task's data structure.

    xTaskCreateStatic(LED_TaskBlue,         // Function that implements the task.
                      "Blue LED task",      // Text name for the task.
                      STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * ) 2,         // Parameter passed into the task.
                      tskIDLE_PRIORITY + 1, // Priority at which the task is created.
                      blueTaskStack,        // Array to use as the task's stack.
                      &blueTaskBuffer );    // Variable to hold the task's data structure.

    vTaskStartScheduler();
}
