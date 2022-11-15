/*
 * application_commands.c
 *
 * Created: 11/8/2018 10:37:30 AM
 *  Author: Sofian.jafar
 */ 


#include "application_commands.h"


/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

#include "application_config.h"
#include "parameters.h"
#include "shared_data.h"
#include "registers.h"
#include "taskstats.h"
#include "main.h"

#include "printf.h"

#include "stdbool.h"

// variables required for deciding which interface has the control of the system
volatile uint32_t controllingTaskNumber;
volatile uint32_t USBTaskNumber;
volatile uint32_t TCP_CLITaskNumber;

/*************************************
    CLI commands callback functions
 ************************************/
/*
 * Implements the shared memory Read command.
 */
static BaseType_t prvSharedMemoryReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the firmware reset command.
 */
static BaseType_t prvFirmwareResetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*************************************
        CLI commands structures
 ************************************/
/* Structure that defines the "SHARE:R" command line command.  This
reads the Shared Memory value directly. */
static const CLI_Command_Definition_t SharedMemoryRead =
{
    "SHARE:R",
    "\r\nSHARE:R <CH> :\r\n Reads the 16bit shared memory value of <CH> in decimal. [0 - 4048]\r\n",
    prvSharedMemoryReadCommand, /* The function to run. */
    1 /* One parameters is expected, which can take any value. */
};

/* Structure that defines the "DACWrite" command line command.  This
sends a new value to the DAC stream using appropriate function. [0 - 4048] */
static const CLI_Command_Definition_t FirmwareReset =
{
    "FIRMWARE:RESET",
    "\r\nFIRMWARE:RESET :\r\n Reset firmware and reload new parameters from flash\r\n",
    prvFirmwareResetCommand, /* The function to run. */
    0 /* No parameters are expected. */
};

void vRegisterApplicationCLICommands( void )
{
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &SharedMemoryRead );
    FreeRTOS_CLIRegisterCommand( &FirmwareReset );
    vRegisterTaskStats_CLICommands();
}
/*-----------------------------------------------------------*/

// Implements the shared memory Read command.
static BaseType_t prvSharedMemoryReadCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    uint32_t    channel;
    uint16_t    shared_data_sample = 0;
    
    configASSERT( pcWriteBuffer );

    char *pcParameter;
    BaseType_t xParameterStringLength;
    
    pcParameter = ( char * ) FreeRTOS_CLIGetParameter(pcCommandString,         /* The command string itself. */
                                           1,                       /* Return the next parameter. */
                                           &xParameterStringLength  /* Store the parameter string length. */
                                          );
    
    // Terminate char array
    pcParameter[ xParameterStringLength ] = 0x00;

    channel = strtoul(pcParameter, NULL, 10);

    /* Check if conversion was correct and withing range */
    if ( (channel == 0) || (channel > SHARED_MEMORY_CHANNELS) )
    {
        // address out of range
        strcat( pcWriteBuffer, ADDRESS_ERROR);
        return pdFALSE;
    }
    
    shared_data_sample = read_shared_memory(channel - 1);

    sprintf(pcWriteBuffer, "%d\n\r", shared_data_sample);

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}

// Implement Firmware reset command
static BaseType_t prvFirmwareResetCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    /* Remove compile time warnings about unused parameters, and check the
    write buffer is not NULL.  NOTE - for simplicity, this example assumes the
    write buffer length is adequate, so does not check for buffer overflows. */
    ( void ) pcCommandString;
    ( void ) xWriteBufferLen;
    configASSERT( pcWriteBuffer );
    
    HAL_NVIC_SystemReset();

    // anything from here after makes no sense, but I like keeping code consistent :P
    strcpy(pcWriteBuffer, NO_ERROR);

    /* There is no more data to return after this single string, so return
    pdFALSE. */
    return pdFALSE;
}
