/*
 * registers.c
 *
 * Created: 3/16/2021 1:51:55 PM
 *  Author: Sofian
 */ 

#include "registers.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
//#include "FreeRTOSIPConfig.h"

/* Standard includes. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Other includes. */
#include "printf.h"
#include "application_config.h"
#include "parameters.h"
#include "application_commands.h"

union Register
{
    uint32_t    ivalue;
    float       fvalue;
};

/*
 * Register the commands for flash handling
 */
static void     vRegisterRegisters_CLICommands( void );

/*************************************
    CLI commands callback functions
 ************************************/

/*
 * Implements set register value as an integer.
 */
static BaseType_t  prvwrite_Register( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements read register value as an integer.
 */
static BaseType_t  prvread_Register( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*************************************
        CLI commands structures
 ************************************/
/* Structure that defines the "PAR:W:INT <LIST> <ADDRESS> <VALUE> <DataType:i,f>" command line command. */
static const   CLI_Command_Definition_t writeRegister =
{
    "REG:W",
    "\r\nREG:W <ADDRESS> <VALUE> <DataType:i,f>:\r\n Write Integer[i] or Float[f] into <ADDRESS>ed location in register struct.\r\n",
    prvwrite_Register, /* The function to run. */
    3 /* two parameters expected. */
};

/* Structure that defines the "PAR:R <LIST> <ADDRESS> <DataType:i,f>" command line command. */
static const   CLI_Command_Definition_t readRegister =
{
    "REG:R",
    "\r\nREG:R <ADDRESS> <DataType:i,f> :\r\n Read Integer[i] or Float[f] from <ADDRESS>ed location in temp register struct.\r\n",
    prvread_Register, /* The function to run. */
    2 /* one parameter expected. */
};

/*************************************
        Function definitions
 ************************************/
/*
 * change value of register using it's address in the struct
 */
void     vSetRegister(uint32_t address, uint32_t value, reply_status * reply)
{
    if (controllingTaskNumber == pxGetCurrentTaskNumber())
    {
        if (address < REGISTER_COUNT)
        {
            // offset the pointer value by (address of register * 4 bytes(size of register))
            uint32_t pointer = (uint32_t) &REGS + ((address) * 4);
            memcpy((void *)(pointer), &value, 4);
            *reply = WRITE_REGISTER_OK;
        }
        else
        {
            *reply = ADDRESS_OUT_OF_RANGE_ERROR;
        }
    }
    else
    {
        *reply = PERMISSION_ERROR;
    }
}

/*
 * get value of register using it's address in the struct
 */
uint32_t uGetRegister(uint32_t address, reply_status * reply)
{
    if (address < REGISTER_COUNT)
    {
        // offset the pointer value by (address of register * 4 bytes(size of register))
        uint32_t pointer = (uint32_t) &REGS + ((address) * 4);
        *reply = READ_REGISTER_OK;
        return *(uint32_t *)(pointer);
    }
    else
    {
        *reply = ADDRESS_OUT_OF_RANGE_ERROR;
        return 0;
    }
}

static BaseType_t prvwrite_Register( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    char            *pcAddress, *pcValue, *pcDataType;
    BaseType_t      xParameterStringLength;
    uint32_t        address;
    union Register  reg;
    reply_status    reply;    

    /* Obtain the data type for value to be written in that address. 
    Getting last parameter because text needs to be nulled at the end*/
    pcDataType = ( char * ) FreeRTOS_CLIGetParameter
                                    (
                                        pcCommandString,  /* The command string itself. */
                                        3,      /* Return the second parameter. */
                                        &xParameterStringLength /* Store the parameter string length. */
                                    );

    /* Sanity check something was returned. */
    configASSERT( pcDataType );
    
    /* this should be either i or f, give error if anything else. */
    if ( xParameterStringLength != 1)
    {
        strcat( pcWriteBuffer, SYNTAX_ERROR);
        return pdFALSE;
    }
    
    /* Obtain the value to be written in that address. 
    Getting value first because text needs to be nulled at the end*/
    pcValue = ( char * ) FreeRTOS_CLIGetParameter
                                    (
                                        pcCommandString,  /* The command string itself. */
                                        2,      /* Return the second parameter. */
                                        &xParameterStringLength /* Store the parameter string length. */
                                    );

    /* Sanity check something was returned. */
    configASSERT( pcValue );
    /* Terminate the string. */
    pcValue[ xParameterStringLength ] = 0x00;
    
    if (pcDataType[0] == 'i')
    {
        reg.ivalue = atoi(pcValue);
    }
    else if (pcDataType[0] == 'f')
    {
        reg.fvalue = strtof(pcValue, NULL);
    }
    else
    {
        strcat( pcWriteBuffer, SYNTAX_ERROR);
        return pdFALSE;
    }
    
    /* Obtain the address of the parameter. */
    pcAddress = ( char * ) FreeRTOS_CLIGetParameter
                                (
                                    pcCommandString,  /* The command string itself. */
                                    1,      /* Return the first parameter. */
                                    &xParameterStringLength /* Store the parameter string length. */
                                );

    /* Sanity check something was returned. */
    configASSERT( pcAddress );
    /* Terminate the string. */
    pcAddress[ xParameterStringLength ] = 0x00;
    address = atoi(pcAddress);
    
    vSetRegister(address, reg.ivalue, &reply);

    if (reply == WRITE_REGISTER_OK)
    {
        strcat( pcWriteBuffer, NO_ERROR);
    }
    else if (reply == ADDRESS_OUT_OF_RANGE_ERROR)
    {
        strcat( pcWriteBuffer, ADDRESS_ERROR);
    }
    else if (reply == PERMISSION_ERROR)
    {
        strcat( pcWriteBuffer, ACCESS_ERROR);
    }

    return pdFALSE;
}

static BaseType_t  prvread_Register( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    char            *pcAddress, *pcDataType;
    BaseType_t      xParameterStringLength;
    uint32_t        address;
    union Register  reg;
    reply_status    reply;

    /* Obtain the data type for value to be written in that address. 
    Getting last parameter because text needs to be nulled at the end*/
    pcDataType = ( char * ) FreeRTOS_CLIGetParameter
                                    (
                                        pcCommandString,  /* The command string itself. */
                                        2,      /* Return the second parameter. */
                                        &xParameterStringLength /* Store the parameter string length. */
                                    );

    /* Sanity check something was returned. */
    configASSERT( pcDataType );
    
    /* this should be either i or f, give error if anything else. */
    if ( xParameterStringLength != 1)
    {
        strcat( pcWriteBuffer, SYNTAX_ERROR);
        return pdFALSE;
    }

    /* Obtain the address of the parameter. */
    pcAddress = ( char * ) FreeRTOS_CLIGetParameter
                                (
                                    pcCommandString,  /* The command string itself. */
                                    1,      /* Return the first parameter. */
                                    &xParameterStringLength /* Store the parameter string length. */
                                );

    /* Sanity check something was returned. */
    configASSERT( pcAddress );
    /* Terminate the string. */
    pcAddress[ xParameterStringLength ] = 0x00;
    address = atoi(pcAddress);

    reg.ivalue = uGetRegister(address, &reply);
    
    if (reply == READ_REGISTER_OK)
    {
        if (pcDataType[0] == 'i')
        {
            sprintf(pcWriteBuffer, "%d\r\n", reg.ivalue);
        }
        else if (pcDataType[0] == 'f')
        {
            sprintf(pcWriteBuffer, "%e\r\n", reg.fvalue);
        }
        else
        {
            strcat( pcWriteBuffer, SYNTAX_ERROR);
        }
    }
    else if (reply == ADDRESS_OUT_OF_RANGE_ERROR)
    {
        strcat( pcWriteBuffer, ADDRESS_ERROR);
    }
    
    return pdFALSE;
}

void vRegisterRegisters_CLICommands( void )
{
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &writeRegister );
    FreeRTOS_CLIRegisterCommand( &readRegister );
}

void init_Registers(void)
{
    vRegisterRegisters_CLICommands();
}
