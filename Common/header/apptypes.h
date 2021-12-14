/*
 * types.h
 *
 * Created: 3/16/2021 3:53:46 PM
 *  Author: Sofian
 */ 


#ifndef TYPES_H_
#define TYPES_H_

#include <stdint.h>

/****************** Interfacing *******************/
typedef enum 
{
    SYS_COMMAND_OK              = 0,
    READ_REGISTER_OK            = 1,
    WRITE_REGISTER_OK           = 2,
    READ_PARAMETER_OK           = 3,
    WRITE_PARAMETER_OK          = 4,
    CONTROL_INTERFACE_OK        = 5,
    SYS_COMMAND_ERROR           = 6,
    INPUT_TYPE_ERROR            = 7,
    PACKET_SIZE_ERROR           = 8,
    ADDRESS_OUT_OF_RANGE_ERROR  = 9,
    PERMISSION_ERROR            = 10,
    CONTROL_INTERFACE_ERROR     = 11
}reply_status;

/****************** PARAMETERS *******************/
// ensure all data is 32bit, this is a structure for holding all parameters
typedef struct
{
    /***** Ethernet settings *****/
    uint32_t    USES_DHCP;
    
    uint32_t    IP_ADDR[4];
    uint32_t    GATEWAY_ADDR[4];
    uint32_t    DNS_SERVER_ADDR[4];
    uint32_t    NET_MASK[4];

    // Device info
    uint32_t    DEVICE_ID;
}parameters_t;
#define PARAMETERS_T_SIZE   sizeof (parameters_t)
#define PARAMETER_COUNT     PARAMETERS_T_SIZE / sizeof (uint32_t)

/****************** REGISTERS *******************/
// ensure all data is 32bit, this is a structure for holding all registers used on-line
typedef struct
{
    /*********************/
    /***** read only *****/
    /*********************/
    
    // Firmware version
    float               FIRMWARE_VERSION;

    uint32_t            DIG_INPUTS;

    uint32_t            COUNTER_1HZ;

    /**********************/
    /***** read/write *****/
    /**********************/

    uint32_t            DIG_OUTPUTS;
}registers_t;
#define REGISTERS_T_SIZE            sizeof (registers_t)
#define REGISTER_COUNT              REGISTERS_T_SIZE / sizeof (uint32_t)
#define REGISTERS_READ_ONLY_COUNT   12

#endif /* TYPES_H_ */
