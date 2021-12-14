/*
 * TCPIP.c
 *
 * Created: 11/17/2018 11:08:12 AM
 *  Author: Sofian.jafar
 */ 

#include "TCPIP.h"
#include "application_config.h"
//#include "console.h"
//#include "driver_init.h"
//#include "utils.h"
#include "LED.h"
#include "parameters.h"

#include "FreeRTOS_DHCP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"
//#include "UDPLoggingPrintf.h"
#include "FreeRTOSTimer.h"

#if (USE_IPERF == 1)
    #include "iperf_task.h"
#endif

#include <stdio.h>

/* The maximum time to wait for a closing socket to close. */
#define cmdSHUTDOWN_DELAY	( pdMS_TO_TICKS( 5000 ) )

/* define whether UDP logging is used on or not
 * port is define in FreeRTOSIPConfig.h  */
#define mainCREATE_UDP_LOGGING_TASK     0

/* Define names that will be used for DNS, LLMNR and NBNS searches.  This allows
mainHOST_NAME to be used when the IP address is not known.  For example
"ping RTOSDemo" to resolve RTOSDemo to an IP address then send a ping request. */
#define mainHOST_NAME           "portenta-STM"
#define mainDEVICE_NICK_NAME    "portenta"

/* The default IP and MAC address used by the demo.  The address configuration
defined here will be used if ipconfigUSE_DHCP is 0, or if ipconfigUSE_DHCP is
1 but a DHCP server could not be contacted.  See the online documentation for
more information.  http://www.FreeRTOS.org/tcp */

/* Default MAC address configuration. */
const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };

void TCPIP_init(void)
{
    // Initialize FreeRTOS+TCP
    uint8_t ucIPAddress[ 4 ] = { PARS.IP_ADDR[0], PARS.IP_ADDR[1], PARS.IP_ADDR[2], PARS.IP_ADDR[3] };
    uint8_t ucNetMask[ 4 ] = { PARS.NET_MASK[0], PARS.NET_MASK[1], PARS.NET_MASK[2], PARS.NET_MASK[3] };
    uint8_t ucGatewayAddress[ 4 ] = { PARS.GATEWAY_ADDR[0], PARS.GATEWAY_ADDR[1], PARS.GATEWAY_ADDR[2], PARS.GATEWAY_ADDR[3] };
    uint8_t ucDNSServerAddress[ 4 ] = { PARS.DNS_SERVER_ADDR[0], PARS.DNS_SERVER_ADDR[1], PARS.DNS_SERVER_ADDR[2], PARS.DNS_SERVER_ADDR[3] };
    FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
}

void prvGracefulShutdown( Socket_t xSocket )
{
TickType_t xTimeOnShutdown;

    /* Initiate a shutdown in case it has not already been initiated. */
    FreeRTOS_shutdown( xSocket, FREERTOS_SHUT_RDWR );

    /* Wait for the shutdown to take effect, indicated by FreeRTOS_recv()
    returning an error. */
    xTimeOnShutdown = xTaskGetTickCount();
    do
    {
        if( FreeRTOS_recv( xSocket, NULL, ipconfigTCP_MSS, FREERTOS_MSG_DONTWAIT) < 0 )
        {
            break;
        }
    } while( ( xTaskGetTickCount() - xTimeOnShutdown ) < cmdSHUTDOWN_DELAY );

    /* Finished with the socket and the task. */
    FreeRTOS_closesocket( xSocket );
}
/*-----------------------------------------------------------*/

Socket_t prvOpenTCPServerSocket( uint16_t usPort )
{
struct freertos_sockaddr xBindAddress;
Socket_t xSocket;
static const TickType_t xReceiveTimeOut = portMAX_DELAY;
const BaseType_t xBacklog = 20;
BaseType_t xReuseSocket = pdTRUE;

    /* Attempt to open the socket. */
    xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
    configASSERT( xSocket != FREERTOS_INVALID_SOCKET );

    /* Set a time out so accept() will just wait for a connection. */
    FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );

    /* Only one connection will be used at a time, so re-use the listening
    socket as the connected socket.  See SimpleTCPEchoServer.c for an example
    that accepts multiple connections. */
    FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_REUSE_LISTEN_SOCKET, &xReuseSocket, sizeof( xReuseSocket ) );
    
    /* Declare an xWinProperties structure. */
    WinProperties_t  xWinProps;

    /* Fill in the required buffer and window sizes. */
    /* Unit: bytes */
    xWinProps.lTxBufSize = ipconfigTCP_TX_BUFFER_LENGTH;
    /* Unit: MSS */
    xWinProps.lTxWinSize = ipconfigTCP_TX_WIN_SIZE;
    /* Unit: bytes */
    xWinProps.lRxBufSize = ipconfigTCP_RX_BUFFER_LENGTH;
    /* Unit: MSS */
    xWinProps.lRxWinSize = ipconfigTCP_RX_WIN_SIZE;

    /* Use the structure with the
    FREERTOS_SO_WIN_PROPERTIES parameter in a call to
    FreeRTOS_setsockopt(). */
    FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &xWinProps, sizeof( xWinProps ) );

    /* NOTE:  The CLI is a low bandwidth interface (typing characters is slow),
    so the TCP window properties are left at their default.  See
    SimpleTCPEchoServer.c for an example of a higher throughput TCP server that
    uses are larger RX and TX buffer. */

    /* Bind the socket to the port that the client task will send to, then
    listen for incoming connections. */
    xBindAddress.sin_port = usPort;
    xBindAddress.sin_port = FreeRTOS_htons( xBindAddress.sin_port );
    FreeRTOS_bind( xSocket, &xBindAddress, sizeof( xBindAddress ) );
    FreeRTOS_listen( xSocket, xBacklog );

    return xSocket;
}
/*-----------------------------------------------------------*/

Socket_t prvOpenUDPServerSocket( uint16_t usPort )
{
struct freertos_sockaddr xServer;
Socket_t xSocket = FREERTOS_INVALID_SOCKET;

	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
	if( xSocket != FREERTOS_INVALID_SOCKET)
	{
		/* Zero out the server structure. */
		memset( ( void * ) &xServer, 0x00, sizeof( xServer ) );

		/* Set family and port. */
		xServer.sin_port = FreeRTOS_htons( usPort );

		/* Bind the address to the socket. */
		if( FreeRTOS_bind( xSocket, &xServer, sizeof( xServer ) ) == -1 )
		{
			FreeRTOS_closesocket( xSocket );
			xSocket = FREERTOS_INVALID_SOCKET;
		}
	}

	return xSocket;
}
/*-----------------------------------------------------------*/

/*
 * from FreeRTOS function prototype: Generate psuedo random number
 * prototyped in FreeRTOS_TCP_IP.h
 */
uint32_t ulApplicationGetNextSequenceNumber( uint32_t ulSourceAddress,
                                                    uint16_t usSourcePort,
                                                    uint32_t ulDestinationAddress,
                                                    uint16_t usDestinationPort )
{
    return uxRand();
}

/* 
 * Called by FreeRTOS+TCP when the network connects or disconnects.  Disconnect
 * events are only received if implemented in the MAC driver.
 * prototyped in FreeRTOS_TCP_IP.h
 */
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )
{
static BaseType_t xTasksAlreadyCreated = pdFALSE;

    FreeRTOS_printf( ( "vApplicationIPNetworkEventHook: event %ld\n", eNetworkEvent ) );

    /* If the network has just come up...*/
    if( eNetworkEvent == eNetworkUp )
    {
        /* Create the tasks that use the IP stack if they have not already been
        created. */
        if( xTasksAlreadyCreated == pdFALSE )
        {
            /* Tasks that use the TCP/IP stack can be created here. */

            /* Start a new task to fetch logging lines and send them out. */
            #if( mainCREATE_UDP_LOGGING_TASK == 1 )
            {
                vUDPLoggingTaskCreate();
            }
            #endif

            #if( ( mainCREATE_FTP_SERVER == 1 ) || ( mainCREATE_HTTP_SERVER == 1 ) )
            {
                /* Let the server work task now it can now create the servers. */
                xTaskNotifyGive( xServerWorkTaskHandle );
            }
            #endif

            #if( mainCREATE_TCP_ECHO_CLIENT_TASKS_SINGLE == 1 )
            {
                vStartTCPEchoClientTasks_SingleTasks( mainECHO_CLIENT_TASK_STACK_SIZE, mainECHO_CLIENT_TASK_PRIORITY );
            }
            #endif

            #if( mainCREATE_SIMPLE_TCP_ECHO_SERVER == 1 )
            {
                vStartSimpleTCPServerTasks( mainECHO_SERVER_STACK_SIZE, mainECHO_SERVER_TASK_PRIORITY );
            }
            #endif

            /* Register commands with the FreeRTOS+CLI command
            interpreter via the TCP port specified by the
            TCP_CLI_PORT constant. If other commands need 
            to be registerd, their respective tasks should/could register them */
            vRegisterTCPCLICommands();
            
            task_app_TCP_CLI_create();
            
            /* Register commands with the FreeRTOS+CLI command
            interpreter via the UDP port specified by the
            UDP_CLI_PORT constant. If other commands need 
            to be registerd, their respective tasks should/could register them */
            
            #if (USE_IPERF == 1)
                vIPerfInstall();
            #endif
            
            // indicate that ethernet is connected using the LED
            task_app_led_set_delay(LED1_INDEX, LED_ETH_CONNECTED_PERIOD_0_MS, LED_ETH_CONNECTED_PERIOD_1_MS);
            xTasksAlreadyCreated = pdTRUE;
        }
    }
}
/*-----------------------------------------------------------*/

/*
 * Declared extern in FreeRTOS_DNS.h
 */
BaseType_t xApplicationDNSQueryHook( const char *pcName )
{
BaseType_t xReturn;

    /* Determine if a name lookup is for this node.  Two names are given
    to this node: that returned by pcApplicationHostnameHook() and that set
    by mainDEVICE_NICK_NAME. */
    if( strcmp( pcName, pcApplicationHostnameHook() ) == 0 )
    {
        xReturn = pdPASS;
    }
    else if( strcmp( pcName, mainDEVICE_NICK_NAME ) == 0 )
    {
        xReturn = pdPASS;
    }
    else
    {
        xReturn = pdFAIL;
    }

    return xReturn;
}
/*-----------------------------------------------------------*/

/*
 * Declared in FreeRTOS_IP.h
 */
const char *pcApplicationHostnameHook( void )
{
    /* Assign the name "rtosdemo" to this network node.  This function will be
    called during the DHCP: the machine will be registered with an IP address
    plus this name. */
    return mainHOST_NAME;
}
/*-----------------------------------------------------------*/

eDHCPCallbackAnswer_t xApplicationDHCPHook( eDHCPCallbackPhase_t eDHCPPhase, uint32_t ulIPAddress )
{
    if (PARS.USES_DHCP == 1)
    {
        return eDHCPContinue;
    }
    else
    {
        return eDHCPUseDefaults;
    }
}

// required for checking TCPIP random number generator
BaseType_t xApplicationGetRandomNumber( uint32_t * pulNumber )
{
    *pulNumber = uxRand();

    return pdTRUE;
}
