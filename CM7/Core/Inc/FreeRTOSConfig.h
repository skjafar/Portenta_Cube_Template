/* USER CODE BEGIN Header */
/*
 * FreeRTOS Kernel V10.3.1
 * Portion Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Portion Copyright (C) 2019 StMicroelectronics, Inc.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */
/* USER CODE END Header */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * These parameters and more are described within the 'configuration' section of the
 * FreeRTOS API documentation available on the FreeRTOS.org web site.
 *
 * See http://www.freertos.org/a00110.html
 *----------------------------------------------------------*/

/* USER CODE BEGIN Includes */
/* Section where include file can be added */
/* USER CODE END Includes */

/* Ensure definitions are only used by the compiler, and not by the assembler. */
#if defined(__ICCARM__) || defined(__CC_ARM) || defined(__GNUC__)
  #include <stdint.h>
  extern uint32_t SystemCoreClock;
  void xPortSysTickHandler(void);
#endif

#ifndef CMSIS_device_header
#define CMSIS_device_header "stm32h7xx.h"
#endif /* CMSIS_device_header */

#ifndef configCPU_CLOCK_HZ
#define configCPU_CLOCK_HZ (SystemCoreClock)
#endif

// <o> Max Priority for task <5-128>
// <i> Defines the max priority of task
// <i> Default: 5
// <id> freertos_max_priorities
#ifndef configMAX_PRIORITIES
#define configMAX_PRIORITIES (13)
#endif

// <o> Minimal stack size<64-1024>
// <i> Defines minimal number of stack size
// <i> Default: 64
// <id> freertos_minimal_stack_size
#ifndef configMINIMAL_STACK_SIZE
#define configMINIMAL_STACK_SIZE (64)
#endif

/* configTOTAL_HEAP_SIZE is not used when heap_3.c is used. */
// <o> Heap size<64-1048576:4>
// <i> Defines the heap size(byte) on system
// <i> value should be multiples of 4
// <i> Default: 2400
// <id> freertos_total_heap_size
#ifndef configTOTAL_HEAP_SIZE
#define configTOTAL_HEAP_SIZE ((size_t)(configMINIMAL_STACK_SIZE * 4 * 700))
#endif

// <q> Enable mutex
// <id> freertos_use_mutexes
#ifndef configUSE_MUTEXES
#define configUSE_MUTEXES 1
#endif

// <q> Enable counting semaphore
// <id> freertos_use_counting_semaphores
#ifndef configUSE_COUNTING_SEMAPHORES
#define configUSE_COUNTING_SEMAPHORES 1
#endif

// </h>

// <e> Advanced
// <id> freertos_advanced
#ifndef FREERTOS_ADVANCED_CONFIG
#define FREERTOS_ADVANCED_CONFIG 1
#endif

// <q> Enable preemption
// <id> freertos_use_preemption
#ifndef configUSE_PREEMPTION
#define configUSE_PREEMPTION 1
#endif

// <q> Use recursive mutex
// <id> freertos_use_recursive_mutexes
#ifndef configUSE_RECURSIVE_MUTEXES
#define configUSE_RECURSIVE_MUTEXES 1
#endif

// <q> Generate runtime stats
// <id> freertos_generate_run_time_stats
#ifndef configGENERATE_RUN_TIME_STATS
#define configGENERATE_RUN_TIME_STATS 1
#endif

// <q> Use 16bit tick
// <id> freertos_use_16_bit_ticks
#ifndef configUSE_16_BIT_TICKS
#define configUSE_16_BIT_TICKS 0
#endif

// <q> Check stack overflow
// <id> freertos_check_for_stack_overflow
#ifndef configCHECK_FOR_STACK_OVERFLOW
#define configCHECK_FOR_STACK_OVERFLOW 1
#endif

// <q> Use maclloc failed hook
// <id> freertos_use_malloc_failed_hook
#ifndef configUSE_MALLOC_FAILED_HOOK
#define configUSE_MALLOC_FAILED_HOOK 1
#endif

// <q> Use idle hook
// <id> freertos_use_idle_hook
#ifndef configUSE_IDLE_HOOK
#define configUSE_IDLE_HOOK 0
#endif

// <q> Use tick hook
// <i> if open, you must realize vApplicationTickHook function
// <id> freertos_use_tick_hook
#ifndef configUSE_TICK_HOOK
#define configUSE_TICK_HOOK 0
#endif

// <q> Use tickless idle
// <id> freertos_use_tickless_idle
#ifndef configUSE_TICKLESS_IDLE
#define configUSE_TICKLESS_IDLE 0
#endif

// <q> Use trace facility
// <id> freertos_use_trace_facility
#ifndef configUSE_TRACE_FACILITY
#define configUSE_TRACE_FACILITY 1
#endif

// <q> Use statistics formating functions
// <id> freertos_use_stats_formatting_functions
#ifndef configUSE_STATS_FORMATTING_FUNCTIONS
#define configUSE_STATS_FORMATTING_FUNCTIONS 1
#endif

// <q> Use port optimised selection functions
// <i> default is not supposed for Cortex-M0
// <id> freertos_use_port_optimised_functions
#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

// <q> Use application task tag functions
// <id> freertos_use_application_task_tag_functions
#ifndef configUSE_APPLICATION_TASK_TAG
#define configUSE_APPLICATION_TASK_TAG 0
#endif

// <q> Use co-routines
// <id> freertos_use_co_routines
/* Co-routine definitions. */
#ifndef configUSE_CO_ROUTINES
#define configUSE_CO_ROUTINES 0
#endif

// <o> Co-routine max priority <1-2>
// <i> Default is 2
// <id> freertos_max_co_routine_priorities
#ifndef configMAX_CO_ROUTINE_PRIORITIES
#define configMAX_CO_ROUTINE_PRIORITIES (2)
#endif

// <o> Tick Rate (Hz) <1-1000000>
// <i> Default is 1KHz
// <id> freertos_tick_rate_hz
#ifndef configTICK_RATE_HZ
#define configTICK_RATE_HZ ((TickType_t)1000)
#endif

/* Software timer definitions. */
// <q> Enable timer
// <id> freertos_use_timers
#ifndef configUSE_TIMERS
#define configUSE_TIMERS 1
#endif

// <o> Timer task priority <1-10>
// <i> Default is 2
// <id> freertos_timer_task_priority
#ifndef configTIMER_TASK_PRIORITY
#define configTIMER_TASK_PRIORITY configMAX_PRIORITIES - 3 
#endif

#define configTIMER_QUEUE_LENGTH 5

// <o> Timer task stack size <32-512:4>
// <i> Default is 64
// <id> freertos_timer_task_stack_depth
#ifndef TIMER_TASK_STACK_DEPTH
#define configTIMER_TASK_STACK_DEPTH (2 * configMINIMAL_STACK_SIZE)
#endif

/* Cortex-M specific definitions. */
#ifdef __NVIC_PRIO_BITS
 /* __BVIC_PRIO_BITS will be specified when CMSIS is being used. */
 #define configPRIO_BITS         __NVIC_PRIO_BITS
#else
 #define configPRIO_BITS         4
#endif

#define configMAX_TASK_NAME_LEN (16)
#define configIDLE_SHOULD_YIELD 1
#define configQUEUE_REGISTRY_SIZE 8
#define configUSE_QUEUE_SETS 1
#define configENABLE_BACKWARD_COMPATIBILITY 1

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

// <q> Include function to set task priority
// <id> freertos_vtaskpriorityset
#ifndef INCLUDE_vTaskPrioritySet
#define INCLUDE_vTaskPrioritySet 0
#endif

// <q> Include function to get task priority
// <id> freertos_uxtaskpriorityget
#ifndef INCLUDE_uxTaskPriorityGet
#define INCLUDE_uxTaskPriorityGet 0
#endif

// <q> Include function to delete task
// <id> freertos_vtaskdelete
#ifndef INCLUDE_vTaskDelete
#define INCLUDE_vTaskDelete 1
#endif

// <q> Include function to suspend task
// <id> freertos_vtasksuspend
#ifndef INCLUDE_vTaskSuspend
#define INCLUDE_vTaskSuspend 1
#endif

// <q> Include function to resume task from ISR
// <id> freertos_xresumefromisr
#ifndef INCLUDE_xResumeFromISR
#define INCLUDE_xResumeFromISR 0
#endif

// <q> Include task delay utilities
// <id> freertos_vtaskdelayuntil
#ifndef INCLUDE_vTaskDelayUntil
#define INCLUDE_vTaskDelayUntil 0
#endif

// <q> Include task delay function
// <id> freertos_vtaskdelay
#ifndef INCLUDE_vTaskDelay
#define INCLUDE_vTaskDelay 1
#endif

#define INCLUDE_xTaskGetSchedulerState 1

// <q> Include the function to get current task handler
// <id> freertos_xtaskgetcurrenttaskhandle
#ifndef INCLUDE_xTaskGetCurrentTaskHandle
#define INCLUDE_xTaskGetCurrentTaskHandle 1
#endif

#define INCLUDE_uxTaskGetStackHighWaterMark 0

// <q> Include the function to get idle task handler
// <id> freertos_xtaskgetidletaskhandle
#ifndef INCLUDE_xTaskGetIdleTaskHandle
#define INCLUDE_xTaskGetIdleTaskHandle 0
#endif

#define INCLUDE_xTimerGetTimerDaemonTaskHandle 0

// <q> Include the function to get task name
// <id> freertos_pctaskgettaskname
#ifndef INCLUDE_pcTaskGetTaskName
#define INCLUDE_pcTaskGetTaskName 1
#endif

// <q> Include the function to get task state
// <id> freertos_etaskgetstate
#ifndef INCLUDE_eTaskGetState
#define INCLUDE_eTaskGetState 0
#endif

// <q> Include the function to clean task resources
// <id> freertos_vtaskcleanupresources
#ifndef INCLUDE_vTaskCleanUpResources
#define INCLUDE_vTaskCleanUpResources 0
#endif

// <q> Include the function to pend timer call
// <id> freertos_xtimerpendfunctioncall
#ifndef INCLUDE_xTimerPendFunctionCall
#define INCLUDE_xTimerPendFunctionCall 0
#endif

// </e>

/* Normal assert() semantics without relying on the provision of an assert.h
header file. */
// TODO: use this later
/* #define configASSERT(x)                                                                                                \
    if ((x) == 0) {                                                                                                    \
         taskDISABLE_INTERRUPTS();                                                                                      \
         for (;;)                                                                                                       \
             ;                                                                                                          \
    } */

/* Assert call defined for debug builds. */
extern void vAssertCalled( const char * pcFile,
                           uint32_t ulLine );
#ifndef configASSERT
    #define configASSERT( x )    if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ )
#endif

/* Definitions that map the FreeRTOS port interrupt handlers to their CMSIS
standard names - or at least those used in the unmodified vector table. */

#define vPortSVCHandler SVC_Handler

#define xPortPendSVHandler PendSV_Handler
// #define xPortSysTickHandler SysTick_Handler

/* Used when configGENERATE_RUN_TIME_STATS is 1. */
#if configGENERATE_RUN_TIME_STATS
// extern void     vConfigureTimerForRunTimeStats(void);
extern uint64_t ullGetRunTimeCounterValue(void);
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()  //vConfigureTimerForRunTimeStats()
#define portGET_RUN_TIME_COUNTER_VALUE() ullGetRunTimeCounterValue()
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY 15 //0x07

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

#endif /* FREERTOS_CONFIG_H */
