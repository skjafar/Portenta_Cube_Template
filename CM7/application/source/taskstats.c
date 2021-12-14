/*
 * taskstats.c
 *
 * Created: 3/22/2021 1:14:38 PM
 *  Author: Sofian
 */ 

#include "taskstats.h"
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"

#include <stdint.h>
#include <string.h>

#include "printf.h"

/*
 * Macros used by vListTask to indicate which state a task is in.
 */
#define tskRUNNING_CHAR      ( 'X' )
#define tskBLOCKED_CHAR      ( 'B' )
#define tskREADY_CHAR        ( 'R' )
#define tskDELETED_CHAR      ( 'D' )
#define tskSUSPENDED_CHAR    ( 'S' )

/*************************************
    CLI commands callback functions
 ************************************/

/*
 * Implements set register value as an integer.
 */
static BaseType_t prvTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */

static const CLI_Command_Definition_t TaskStats =
{
    "task-stats", /* The command string to type. */
    "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
    prvTaskStats, /* The function to run. */
    0 /* No parameters are expected. */
};

static char * prvWriteNameToBuffer( char * pcBuffer,
                                    const char * pcTaskName )
{
    size_t x;

    /* Start by copying the entire string. */
    strcpy( pcBuffer, pcTaskName );

    /* Pad the end of the string with spaces to ensure columns line up when
        * printed out. */
    for( x = strlen( pcBuffer ); x < ( size_t ) ( configMAX_TASK_NAME_LEN - 1 ); x++ )
    {
        pcBuffer[ x ] = ' ';
    }

    /* Terminate. */
    pcBuffer[ x ] = ( char ) 0x00;

    /* Return the new end of string. */
    return &( pcBuffer[ x ] );
}

void taskStats( char * pcWriteBuffer )
{
    TaskStatus_t * pxTaskStatusArray;
    UBaseType_t uxArraySize, x;
    uint32_t ulTotalTime, ulStatsAsPercentage;
    char cStatus;

    uint32_t uxCurrentNumberOfTasks = (unsigned int)uxTaskGetNumberOfTasks();

    /*
        * PLEASE NOTE:
        *
        * This function is provided for convenience only, and is used by many
        * of the demo applications.  Do not consider it to be part of the
        * scheduler.
        *
        * vTaskList() calls uxTaskGetSystemState(), then formats part of the
        * uxTaskGetSystemState() output into a human readable table that
        * displays task names, states and stack usage as well as cpu time usage.
        *
        * vTaskList() has a dependency on the sprintf() C library function that
        * might bloat the code size, use a lot of stack, and provide different
        * results on different platforms.  An alternative, tiny, third party,
        * and limited functionality implementation of sprintf() is provided in
        * many of the FreeRTOS/Demo sub-directories in a file called
        * printf-stdarg.c (note printf-stdarg.c does not provide a full
        * snprintf() implementation!).
        *
        * It is recommended that production systems call uxTaskGetSystemState()
        * directly to get access to raw stats data, rather than indirectly
        * through a call to vTaskList().
        *

        * Note on note:
        * This function is only used by the USB-info task, which has very low priority.
    */
    /* Make sure the write buffer does not contain a string. */
    *pcWriteBuffer = ( char ) 0x00;

    /* Take a snapshot of the number of tasks in case it changes while this
        * function is executing. */
    uxArraySize = uxCurrentNumberOfTasks;

    /* Allocate an array index for each task.  NOTE!  if
        * configSUPPORT_DYNAMIC_ALLOCATION is set to 0 then pvPortMalloc() will
        * equate to NULL. */
    pxTaskStatusArray = pvPortMalloc( uxCurrentNumberOfTasks * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */

    if( pxTaskStatusArray != NULL )
    {
        /* Generate the (binary) data. */
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalTime );

        /* For percentage calculations. */
        ulTotalTime /= 100UL;

        /* Avoid divide by zero errors. */
        if( ulTotalTime > 0UL )
        {
            
            strcpy(pcWriteBuffer, "Tasks Name\tState\tPri\tStack\tNum\tAbs. Time [ms]\t% Time\r\n");
            pcWriteBuffer += strlen(pcWriteBuffer);
            strcpy(pcWriteBuffer, "**********************************************************************\r\n");
            pcWriteBuffer += strlen(pcWriteBuffer);            
            
            /* Create a human readable table from the binary data. */
            for( x = 0; x < uxArraySize; x++ )
            {
                switch( pxTaskStatusArray[ x ].eCurrentState )
                {
                    case eRunning:
                        cStatus = tskRUNNING_CHAR;
                        break;

                    case eReady:
                        cStatus = tskREADY_CHAR;
                        break;

                    case eBlocked:
                        cStatus = tskBLOCKED_CHAR;
                        break;

                    case eSuspended:
                        cStatus = tskSUSPENDED_CHAR;
                        break;

                    case eDeleted:
                        cStatus = tskDELETED_CHAR;
                        break;

                    case eInvalid: /* Fall through. */
                    default:       /* Should not get here, but it is included
                                    * to prevent static checking errors. */
                        cStatus = ( char ) 0x00;
                        break;
                }

                /* What percentage of the total run time has the task used?
                    * This will always be rounded down to the nearest integer.
                    * ulTotalRunTimeDiv100 has already been divided by 100. */
                ulStatsAsPercentage = pxTaskStatusArray[ x ].ulRunTimeCounter / ulTotalTime;

                /* Write the task name to the string, padding with spaces so it
                    * can be printed in tabular form more easily. */
                pcWriteBuffer = prvWriteNameToBuffer( pcWriteBuffer, pxTaskStatusArray[ x ].pcTaskName );


                if( ulStatsAsPercentage > 0UL )
                {
                    #ifdef portLU_PRINTF_SPECIFIER_REQUIRED
                        {
                            sprintf( pcWriteBuffer, "\t%lu\t\t%lu%%\r\n", pxTaskStatusArray[ x ].ulRunTimeCounter, ulStatsAsPercentage );
                        }
                    #else
                        {
                            /* sizeof( int ) == sizeof( long ) so a smaller
                                * printf() library can be used. */
                            sprintf( pcWriteBuffer, "\t%c\t%u\t%u\t%u\t%-16.1f%u%%\r\n", 
                                                    cStatus,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].uxCurrentPriority,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].usStackHighWaterMark,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].xTaskNumber,
                                                    (( float ) pxTaskStatusArray[ x ].ulRunTimeCounter) / 10.0,
                                                    ( unsigned int ) ulStatsAsPercentage ); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
                        }
                    #endif
                }
                else
                {
                    /* If the percentage is zero here then the task has
                        * consumed less than 1% of the total run time. */
                    #ifdef portLU_PRINTF_SPECIFIER_REQUIRED
                        {
                            sprintf( pcWriteBuffer, "\t%lu\t\t<1%%\r\n", pxTaskStatusArray[ x ].ulRunTimeCounter );
                        }
                    #else
                        {
                            /* sizeof( int ) == sizeof( long ) so a smaller
                                * printf() library can be used. */
                            sprintf( pcWriteBuffer, "\t%c\t%u\t%u\t%u\t%-16.1f<1%%\r\n", 
                                                    cStatus,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].uxCurrentPriority,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].usStackHighWaterMark,
                                                    ( unsigned int ) pxTaskStatusArray[ x ].xTaskNumber,
                                                    (( float ) pxTaskStatusArray[ x ].ulRunTimeCounter) / 10.0); /*lint !e586 sprintf() allowed as this is compiled with many compilers and this is a utility function only - not part of the core kernel implementation. */
                        }
                    #endif
                }

                pcWriteBuffer += strlen( pcWriteBuffer );                                                                                                                                                                                                /*lint !e9016 Pointer arithmetic ok on char pointers especially as in this case where it best denotes the intent of the code. */
            }
        }
        else
        {
            mtCOVERAGE_TEST_MARKER();
        }

        /* Free the array again.  NOTE!  If configSUPPORT_DYNAMIC_ALLOCATION
            * is 0 then vPortFree() will be #defined to nothing. */
        vPortFree( pxTaskStatusArray );
    }
    else
    {
        mtCOVERAGE_TEST_MARKER();
    }
}

/*----------------------------------------------------------*/

static BaseType_t prvTaskStats( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );
    
    taskStats(pcWriteBuffer);
    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}

/*-----------------------------------------------------------*/

void vRegisterTaskStats_CLICommands( void )
{
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &TaskStats );
}