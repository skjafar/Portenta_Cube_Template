/*
 * FreeRTOSTimer.c
 * 
 * configuring the FreeRTOS timer
 * 
 * Created: 12/31/2018 8:42:04 AM
 *  Author: Sofian.jafar
 */

#include "FreeRTOSTimer.h"

#include "timers.h"
#include "FreeRTOSIPConfig.h"

#include "application_config.h"

#include "registers.h"


/* Create 3 timers with different timings. */
// 1Hz
#define application1Hz_TIMER_PERIOD     pdMS_TO_TICKS(1000)
// 10Hz
#define application10Hz_TIMER_PERIOD    pdMS_TO_TICKS(100)
// 100Hz
#define application100Hz_TIMER_PERIOD   pdMS_TO_TICKS(10)

static void prv1HzTimerCallback(TimerHandle_t xTimer);
static void prv10HzTimerCallback(TimerHandle_t xTimer);
static void prv100HzTimerCallback(TimerHandle_t xTimer);

BaseType_t startFreeRTOSTaskTimers(void)
{
    TimerHandle_t x1HzTimer;
	TimerHandle_t x10HzTimer;
	TimerHandle_t x100HzTimer;
    
	BaseType_t x1HzTimerStarted;
	BaseType_t x10HzTimerStarted;
	BaseType_t x100HzTimerStarted;

    REGS.COUNTER_1HZ = 0;
    
    /* Create the 10Hz timer, storing the handle to the created timer in x10HzTimer. */
    x1HzTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
        "1Hz",
        /* The software timer's period in ticks. */
        application1Hz_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
        pdTRUE,
        /* This example does not use the timer id. */
        0,
        /* The callback function to be used by the software timer being created. */
        prv1HzTimerCallback);

    /* Create the 10Hz timer, storing the handle to the created timer in x10HzTimer. */
    x10HzTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
        "10Hz",
        /* The software timer's period in ticks. */
        application10Hz_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
        pdTRUE,
        /* This example does not use the timer id. */
        0,
        /* The callback function to be used by the software timer being created. */
        prv10HzTimerCallback);
    
    /* Create the 100Hz timer, storing the handle to the created timer in x100HzTimer. */
    x100HzTimer = xTimerCreate(
        /* Text name for the software timer - not used by FreeRTOS. */
        "100Hz",
        /* The software timer's period in ticks. */
        application100Hz_TIMER_PERIOD,
        /* Setting uxAutoRealod to pdFALSE creates a one-shot software timer. */
        pdTRUE,
        /* This example does not use the timer id. */
        0,
        /* The callback function to be used by the software timer being created. */
        prv100HzTimerCallback);

    
    /* Check the software timers were created. */
    if ( (x1HzTimer != NULL) &&
         (x10HzTimer != NULL) && 
         (x100HzTimer != NULL) )
    {
        /* Start the software timers, using a block time of 0 (no block time). The scheduler has
        not been started yet so any block time specified here would be ignored anyway. */
        x1HzTimerStarted = xTimerStart(x1HzTimer, 0);
        x10HzTimerStarted = xTimerStart(x10HzTimer, 0);
        x100HzTimerStarted = xTimerStart(x100HzTimer, 0);

        /* The implementation of xTimerStart() uses the timer command queue, and xTimerStart()
        will fail if the timer command queue gets full. The timer service task does not get
        created until the scheduler is started, so all commands sent to the command queue will
        stay in the queue until after the scheduler has been started. Check both calls to
        xTimerStart() passed. */
        if ( (x1HzTimerStarted == pdPASS) &&
             (x10HzTimerStarted == pdPASS) &&
             (x100HzTimerStarted == pdPASS) )
        {
            /* values written to queue. */
            return pdPASS;
        }
        else
        {
            return pdFAIL;
        }
    }
    else
    {
        return pdFAIL;
    }
}

static void prv1HzTimerCallback(TimerHandle_t xTimer)
{
    // increment the 1Hz counter
    REGS.COUNTER_1HZ++;
}

static void prv10HzTimerCallback(TimerHandle_t xTimer)
{
    // notify functions without overwrite, this way the state machine has superiority over this notification
    // TODO: enable I2C temperature reading
    // trigTempReading();
}

static void prv100HzTimerCallback(TimerHandle_t xTimer)
{
    // notify functions without overwrite, this way the state machine has superiority over this notification
}
