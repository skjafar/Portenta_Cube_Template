/*
 * application_config.h
 *
 * Created: 11/1/2018 12:19:06 PM
 *  Author: Sofian.jafar
 *
    ***************************************************************************
    *    Configure the different aspects of the application                   *
    ***************************************************************************
    1 tab == 4 spaces!
*/

#ifndef APPLICATION_CONFIG_H_
#define APPLICATION_CONFIG_H_

#ifdef CORE_CM7
	#include "FreeRTOSConfig.h"
#endif

/* Enable ICache and DCache */
#define ENABLE_ICACHE               1

#define ENABLE_DCACHE               0
#if (ENABLE_DCACHE == 1)
    #define CONF_BOARD_ENABLE_CACHE
#endif

/****************************************************
 ***** EXTRA Definitions for application macros *****
 ****************************************************/
// use notifications for blocking messaging between tasks
#define xTaskMessageWait(message)   xTaskNotifyWait( 0xffffffff, 0xffffffff, message, portMAX_DELAY)

// gpio_set_pin_level helper macro
#define set_pin(pin, level)         HAL_GPIO_WritePin(LED_G_GPIO_Port, pin, level)
#define get_pin(pin)                HAL_GPIO_ReadPin(LED_G_GPIO_Port, pin)

// set and unset bit helper macro
#define set_bit(register, mask)     register |= mask
#define unset_bit(register, mask)   register &= ~mask             

/**********************************
 ***** Shared Memory Settings *****
 *********************************/
// number of shared memory channels, these can be filled by ADC readings by M4 for example
#define SHARED_MEMORY_CHANNELS                12

// size of each reading in the shared memory 16 bit = 2 bytes
#define SHARED_MEMORY_BITS                    16

// one complete read is 16bit (2bytes) * number of SHARED_MEMORY Channels
#define SHARED_MEMORY_TOTAL_SAMPLE_LENGTH     (SHARED_MEMORY_CHANNELS)
/* Shared Memory uses TIMER_0 (set at 10kHz) for sampling, maximum frequency is 10ksample/s
   Shared Memory sampling period = SHARED_MEMORY_SAMPLING_PERIOD_MULT * CONF_TC0_TIMER_RC_TICK */
#define SHARED_MEMORY_SAMPLING_PERIOD_MULT    1
#define SHARED_MEMORY_BUFFER_LENGTH           10000

/************************
 ***** Tasks Config *****
 ************************/
/**************** Common CLI ***********************/
#define NO_ERROR                        "OK\r\n"
#define COMMAND_ERROR                   "Command error\r\n"
#define PARAMETERS_ERROR                "Parameter error\r\n"
#define SYNTAX_ERROR                    "Syntax error\r\n"
#define VALUE_ERROR                     "Value Error\r\n"
#define ADDRESS_ERROR                   "Address error\r\n"
#define ACCESS_ERROR                    "Not allowed\r\n"

/**************** TCP CLI ***********************/
// Task Priority
#define TCP_CLI_TASK_PRIORITY           tskIDLE_PRIORITY + 5
// Task stack size
#define TCP_CLI_TASK_STACK_SIZE         3 * configMINIMAL_STACK_SIZE
// Task name
#define TCP_CLI_TASK_NAME               "[A] TCP CLI"
// Port for TCP telnet interface
#define TCP_CLI_PORT                    2008

/**************** USB CLI ***********************/
// Task Priority
#define USB_CLI_TASK_PRIORITY           (tskIDLE_PRIORITY + 1)
// Task stack size
#define USB_CLI_TASK_STACK_SIZE         ( 4 * configMINIMAL_STACK_SIZE )
// Task name
#define USB_CLI_TASK_NAME               "[A] USB CLI"

/**************** USB info print ***********************/
// Task Priority
#define USB_INFO_TASK_STACK_PRIORITY    (tskIDLE_PRIORITY + 1)
// Task stack size
#define USB_INFO_TASK_STACK_SIZE        ( 4 * configMINIMAL_STACK_SIZE )
// Task name
#define USB_INFO_TASK_NAME              "[A] USB Info"

/**************** LEDs toggling **********************/
// Task Priority, set it to highest priority
#define LED_TASK_STACK_PRIORITY         (configMAX_PRIORITIES - 1)
// Task stack size
#define LED_TASK_STACK_SIZE             1 * configMINIMAL_STACK_SIZE
// Task name
#define LED1_TASK_NAME                   "[A] LED1"
#define LED2_TASK_NAME                   "[A] LED2"

#define LED_MAX_PERIOD_MS               5000
#define LED_MIN_PERIOD_MS               50

#define LED_STARTUP_PERIOD_0_MS         2000
#define LED_STARTUP_PERIOD_1_MS         2000
#define LED_ETH_CONNECTED_PERIOD_0_MS   1000
#define LED_ETH_CONNECTED_PERIOD_1_MS   500
#define LED_INTERLOCK_PERIOD_0_MS       100
#define LED_INTERLOCK_PERIOD_1_MS       100

#define LED1_INDEX                      0
#define LED2_INDEX                      1

#endif /* APPLICATION_CONFIG_H_ */