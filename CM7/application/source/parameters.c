/*
 * flash.c
 *
 * Created: 2/7/2021 3:29:12 PM
 *  Author: Sofian.jafar
 */ 

#include "parameters.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "FreeRTOS_CLI.h"
#include "FreeRTOSIPConfig.h"

/* Standard includes. */
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Other includes. */
#include "printf.h"
#include "application_config.h"
//#include "driver_init.h"
#include "application_commands.h"

union Parameter
{
    uint32_t    ivalue;
    float       fvalue;
};

parameters_t    PARS;

/*************************************
    Static functions prototypes
 ************************************/
/*
 * Register the commands for flash handling
 */
static void     vRegisterParameters_CLICommands( void );

static void     initialize_parameters_in_flash(void);

/*************************************
    CLI commands callback functions
 ************************************/

/*
 * Implements set parameter value as an integer.
 */
static BaseType_t  prvwrite_Parameter( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements read parameter value as an integer.
 */
static BaseType_t  prvread_Parameter( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*************************************
        CLI commands structures
 ************************************/
/* Structure that defines the "PAR:W:INT <LIST> <ADDRESS> <VALUE> <DataType:i,f>" command line command. */
static const   CLI_Command_Definition_t writeParameter =
{
    "PAR:W",
    "\r\nPAR:W <ADDRESS> <VALUE> <DataType:i,f>:\r\n Write Integer[i] or Float[f] into <ADDRESS>ed location in parameter <LIST>.\r\n",
    prvwrite_Parameter, /* The function to run. */
    3 /* two parameters expected. */
};

/* Structure that defines the "PAR:R <LIST> <ADDRESS> <DataType:i,f>" command line command. */
static const   CLI_Command_Definition_t readParameter =
{
    "PAR:R",
    "\r\nPAR:R <ADDRESS> <DataType:i,f> :\r\n Read Integer[i] or Float[f] from <ADDRESS>ed location in parameter <LIST>.\r\n",
    prvread_Parameter, /* The function to run. */
    2 /* one parameter expected. */
};


/*************************************
        Function definitions
 ************************************/
/*
 * change value of parameter using it's address in the struct
 */
void     vSetParameter(uint32_t address, uint32_t value, reply_status * reply)
{
    if (controllingTaskNumber == pxGetCurrentTaskNumber())
    {
        if (address < PARAMETER_COUNT)
        {
            // offset the pointer value by (address of parameter * 4 bytes(size of parameter))
            uint32_t pointer = (uint32_t) &PARS + ((address) * 4);
            memcpy((void *)(pointer), &value, 4);
            *reply = WRITE_PARAMETER_OK;
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
 * get value of parameter using it's address in the struct
 */
uint32_t uGetParameter(uint32_t address, reply_status * reply)
{
    if (address < PARAMETER_COUNT)
    {
        // offset the pointer value by (address of parameter * 4 bytes(size of parameter))
        uint32_t pointer = (uint32_t) &PARS + ((address) * 4);
        *reply = READ_PARAMETER_OK;
        return *(uint32_t *)(pointer);
    }
    else
    {
        *reply = ADDRESS_OUT_OF_RANGE_ERROR;
        return 0;
    }
}

/*
 * write parameters struct to flash
 */
void vWritetoFlash(parameters_t * parList)
{
//    flash_write(&FLASH_0, FLASH_PARAMETERS_PAGE, (uint8_t *) &PARS, PARAMETERS_T_SIZE);
}

/*
 * read parameters struct from flash
 */
void vReadFromFlash()
{
//    flash_read(&FLASH_0, FLASH_PARAMETERS_PAGE, (uint8_t *) &PARS, PARAMETERS_T_SIZE);
}

static BaseType_t prvwrite_Parameter( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    char            *pcAddress, *pcValue, *pcDataType;
    BaseType_t      xParameterStringLength;
    uint32_t        address;
    union Parameter par; 
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
        par.ivalue = atoi(pcValue);
    }
    else if (pcDataType[0] == 'f')
    {
        par.fvalue = strtof(pcValue, NULL);
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

    vSetParameter(address, par.ivalue, &reply);
    
    if (reply == WRITE_PARAMETER_OK)
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

static BaseType_t  prvread_Parameter( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
    char            *pcAddress, *pcDataType;
    BaseType_t      xParameterStringLength;
    uint32_t        address;
    union Parameter par;
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

    par.ivalue = uGetParameter(address, &reply);

    if (reply == READ_PARAMETER_OK)
    {
        if (pcDataType[0] == 'i')
        {
            sprintf(pcWriteBuffer, "%d\r\n", par.ivalue);
        }
        else if (pcDataType[0] == 'f')
        {
            sprintf(pcWriteBuffer, "%e\r\n", par.fvalue);
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

void vRegisterParameters_CLICommands( void )
{
    /* Register all the command line commands defined immediately above. */
    FreeRTOS_CLIRegisterCommand( &writeParameter );
    FreeRTOS_CLIRegisterCommand( &readParameter );
}

void init_Parameters(void)
{
    initialize_parameters_in_flash();
    // read stored parameters to be used
    vReadFromFlash(&PARS);
    // register all commands for parameter handling
    vRegisterParameters_CLICommands();
}

void initialize_parameters_in_flash()
{
    PARS.USES_DHCP                      = 1;
    
    PARS.IP_ADDR[0]                     = configIP_ADDR0;
    PARS.IP_ADDR[1]                     = configIP_ADDR1;
    PARS.IP_ADDR[2]                     = configIP_ADDR2;
    PARS.IP_ADDR[3]                     = configIP_ADDR3;

    PARS.GATEWAY_ADDR[0]                = configGATEWAY_ADDR0;
    PARS.GATEWAY_ADDR[1]                = configGATEWAY_ADDR1;
    PARS.GATEWAY_ADDR[2]                = configGATEWAY_ADDR2;
    PARS.GATEWAY_ADDR[3]                = configGATEWAY_ADDR3;

    PARS.DNS_SERVER_ADDR[0]             = configDNS_SERVER_ADDR0;
    PARS.DNS_SERVER_ADDR[1]             = configDNS_SERVER_ADDR1;
    PARS.DNS_SERVER_ADDR[2]             = configDNS_SERVER_ADDR2;
    PARS.DNS_SERVER_ADDR[3]             = configDNS_SERVER_ADDR3;

    PARS.NET_MASK[0]                    = configNET_MASK0;
    PARS.NET_MASK[1]                    = configNET_MASK1;
    PARS.NET_MASK[2]                    = configNET_MASK2;
    PARS.NET_MASK[3]                    = configNET_MASK3;

    vWritetoFlash(&PARS);
}