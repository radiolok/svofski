/* Standard includes. */
#include <stdlib.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#include "common.h"
#include "leds.h"
#include "comchat.h"
#include "effector.h"
#include "gpio.h"
#include "display.h"

/*-----------------------------------------------------------*/

/* Constants to setup the PLL. */
#define mainPLL_MUL_4           ((unsigned char) 0x0003)
#define mainPLL_DIV_1           ((unsigned char) 0x0000)
#define mainPLL_ENABLE          ((unsigned char) 0x0001)
#define mainPLL_CONNECT         ((unsigned char) 0x0003)
#define mainPLL_FEED_BYTE1      ((unsigned char) 0xaa)
#define mainPLL_FEED_BYTE2      ((unsigned char) 0x55)
#define mainPLL_LOCK            ((unsigned long) 0x0400)

/* Constants to setup the MAM. */
#define mainMAM_TIM_3           ((unsigned char) 0x03)
#define mainMAM_MODE_FULL       ((unsigned char) 0x02)

/* Constants to setup the peripheral bus. */
#define mainBUS_CLK_FULL        ((unsigned char) 0x01)

/* Priorities for the demo application tasks. */

#define PRIORITY_LED            (tskIDLE_PRIORITY + 3)
#define PRIORITY_SERIAL         (tskIDLE_PRIORITY + 4)
#define CHECK_TASK_PRIORITY     (tskIDLE_PRIORITY + 4)
#define PRIORITY_DISPLAY        (tskIDLE_PRIORITY + 4)
#define PRIORITY_SERVOR         (tskIDLE_PRIORITY + 8)

/* Constants used by the vMemCheckTask() task. */
#define mainCOUNT_INITIAL_VALUE ((unsigned long) 0)
#define mainNO_TASK             (0)

/* The size of the memory blocks allocated by the vMemCheckTask() task. */
#define mainMEM_CHECK_SIZE_1    ((size_t) 51)
#define mainMEM_CHECK_SIZE_2    ((size_t) 52)
#define mainMEM_CHECK_SIZE_3    ((size_t) 151)

/*-----------------------------------------------------------*/

/*
 * Configure the processor for use with the Olimex demo board.  This includes
 * setup for the I/O, system clock, and access timings.
 */
static void prvSetupHardware(void);

static void prvLEDSetup(void);

/*-----------------------------------------------------------*/

/*
 * Starts all the other tasks, then starts the scheduler. 
 */
int main(void)
{
    prvSetupHardware();

    startLEDFlashTasks(PRIORITY_LED);

    createSerialChatTasks(PRIORITY_SERIAL);

    effector.Init(PRIORITY_SERVOR);

    display.CreateTask(PRIORITY_DISPLAY);

    /* Now all the tasks have been started - start the scheduler.

    NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
    The processor MUST be in supervisor mode when vTaskStartScheduler is 
    called.  The demo applications included in the FreeRTOS.org download switch
    to supervisor mode prior to main being called.  If you are not using one of
    these demo application projects then ensure Supervisor mode is used here. */
    vTaskStartScheduler();

    /* Should never reach here! */
    return 0;
}
/*-----------------------------------------------------------*/

static void prvSetupHardware( void )
{
    #ifdef RUN_FROM_RAM
        /* Remap the interrupt vectors to RAM if we are are running from RAM. */
        SCB_MEMMAP = 2;
    #endif

    /* Setup the PLL to multiply the XTAL input by 4. */
    SCB_PLLCFG = ( mainPLL_MUL_4 | mainPLL_DIV_1 );

    /* Activate the PLL by turning it on then feeding the correct sequence of
    bytes. */
    SCB_PLLCON = mainPLL_ENABLE;
    SCB_PLLFEED = mainPLL_FEED_BYTE1;
    SCB_PLLFEED = mainPLL_FEED_BYTE2;

    /* Wait for the PLL to lock... */
    while( !( SCB_PLLSTAT & mainPLL_LOCK ) );

    /* ...before connecting it using the feed sequence again. */
    SCB_PLLCON = mainPLL_CONNECT;
    SCB_PLLFEED = mainPLL_FEED_BYTE1;
    SCB_PLLFEED = mainPLL_FEED_BYTE2;

    /* Setup and turn on the MAM.  Three cycle access is used due to the fast
       PLL used.  It is possible faster overall performance could be obtained by
       tuning the MAM and PLL settings. */
    MAM_TIM = mainMAM_TIM_3;
    MAM_CR = mainMAM_MODE_FULL;

    /* Setup the peripheral bus to be the same as the PLL output. */
    SCB_VPBDIV = mainBUS_CLK_FULL;
    
    /* Initialise LED outputs. */
    prvLEDSetup();
}

void prvLEDSetup(void) {
    PCB_PINSEL1 &= ~_BV2(25,24)   // 00: GPIO P0.28
                  & ~_BV2(29,28);  // 00: GPIO P0.30
    // P0.28, P0.30 are outputs
    GPIO_IODIR |= _BV2(28,30);
}

