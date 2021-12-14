/*
 * telnet.c
 */


/* Standard includes. */
#include <stdio.h>
#include <time.h>

/* FreeRTOS includes. */
#include <FreeRTOS.h>
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_DHCP.h"
#include "FreeRTOS_tcp_server.h"

#include "telnet.h"

static TelnetClient_t *pxTelnetAddSocket( Telnet_t *pxTelnet );
static void vTelnetRemove( Telnet_t * pxTelnet, TelnetClient_t *pxClient );

static TelnetClient_t *pxTelnetAddSocket( Telnet_t *pxTelnet )
{
TelnetClient_t *pxNewClient;

	pxNewClient = pvPortMalloc( sizeof( *pxNewClient ) );
	if( pxNewClient != NULL )
	{
		memset( pxNewClient, '\0', sizeof( *pxNewClient ) );

		if( pxTelnet->xClients == NULL )
		{
			pxTelnet->xClients = pxNewClient;
		}
		else
		{
		TelnetClient_t *pxClient;

			pxClient = pxTelnet->xClients;
			while( pxClient->pxNext != NULL )
			{
				pxClient = pxClient->pxNext;
			}
			pxClient->pxNext = pxNewClient;
		}
	}
	return pxNewClient;
}
/*-----------------------------------------------------------*/

static void vTelnetRemove( Telnet_t * pxTelnet, TelnetClient_t *pxClient )
{
TelnetClient_t *pxList;

	if( pxTelnet->xClients == pxClient )
	{
		pxTelnet->xClients = pxClient->pxNext;
	}
	else
	{
		pxList = pxTelnet->xClients;
		do
		{
			if( pxList->pxNext == pxClient )
			{
				pxList->pxNext = pxClient->pxNext;
				break;
			}
			pxList = pxList->pxNext;
		} while( pxList != NULL );
	}
	FreeRTOS_closesocket( pxClient->xSocket );
	vPortFree( pxClient );
}
/*-----------------------------------------------------------*/

BaseType_t xTelnetSend( Telnet_t * pxTelnet, struct freertos_sockaddr *pxAddress, const char *pcBuffer, BaseType_t xLength )
{
TelnetClient_t *pxList, *pxNext;
BaseType_t xResult = 0;
	pxList = pxTelnet->xClients;
	while( pxList != NULL )
	{
		pxNext = pxList->pxNext;

		if( ( pxAddress == NULL ) ||
			( ( pxAddress->sin_addr == pxList->xAddress.sin_addr ) && ( pxAddress->sin_port == pxList->xAddress.sin_port ) ) )
		{
			xResult = FreeRTOS_send( pxList->xSocket, pcBuffer, xLength, 0 );
			if( ( xResult < 0 ) && ( xResult != -( pdFREERTOS_ERRNO_EAGAIN ) ) )
			{
				FreeRTOS_printf( ( "xTelnetSend: client %p disconnected (rc %d)\n", pxList->xSocket, xResult ) );
				vTelnetRemove( pxTelnet, pxList );
			}
			if( pxAddress != NULL )
			{
				break;
			}
		}
		pxList = pxNext;
	}

	return xResult;
}
/*-----------------------------------------------------------*/

BaseType_t xTelnetRecv( Telnet_t * pxTelnet, struct freertos_sockaddr *pxAddress, char *pcBuffer, BaseType_t xMaxLength )
{
Socket_t xSocket;
struct freertos_sockaddr xAddress;
socklen_t xSize = sizeof( xAddress );
TelnetClient_t *pxClient, *pxNextClient;
BaseType_t xResult = 0;

	if( pxTelnet->xParentSocket != NULL )
	{
		xSocket = FreeRTOS_accept( pxTelnet->xParentSocket, &xAddress, &xSize );
		if( ( xSocket != NULL ) && ( xSocket != FREERTOS_INVALID_SOCKET ) )
		{
			FreeRTOS_printf( ( "xTelnetRead: new client %p from %xip:%u\n",
				xSocket,
				FreeRTOS_ntohl( xAddress.sin_addr ),
				FreeRTOS_ntohs( xAddress.sin_port ) ) );
			pxClient = pxTelnetAddSocket( pxTelnet );
			if( pxClient != NULL )
			{
				pxClient->xSocket = xSocket;
				memcpy( &pxClient->xAddress, &xAddress, sizeof( pxClient->xAddress ) );
			}
		}
		pxClient = pxTelnet->xClients;
		while( pxClient != NULL )
		{
			xSocket = pxClient->xSocket;
			pxNextClient = pxClient->pxNext;

			xResult = FreeRTOS_recv( xSocket, pcBuffer, xMaxLength, 0 );
			if( xResult > 0 )
			{
				if( pxAddress != NULL )
				{
					memcpy( pxAddress, &pxClient->xAddress, sizeof( *pxAddress ) );
				}
				break;
			}
			if( ( xResult < 0 ) && ( xResult != -( pdFREERTOS_ERRNO_EAGAIN ) ) )
			{
			FreeRTOS_printf( ( "xTelnetRead: client %p disconnected (rc %d)\n", xSocket, xResult ) );
				vTelnetRemove( pxTelnet, pxClient );
			}
			pxClient = pxNextClient;
		}
	}

	return xResult;
}
/*-----------------------------------------------------------*/

BaseType_t xTelnetCreate( Telnet_t * pxTelnet, BaseType_t xPortNr )
{
BaseType_t xReceiveTimeOut = 10;
struct freertos_sockaddr xBindAddress;
BaseType_t xResult = 0;

	memset( pxTelnet, '\0', sizeof( *pxTelnet ) );

	/* Attempt to open the socket. */
	pxTelnet->xParentSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
	if( ( pxTelnet->xParentSocket == FREERTOS_INVALID_SOCKET ) || ( pxTelnet->xParentSocket == NULL ) )
	{
		xResult = -pdFREERTOS_ERRNO_ENOMEM;
	}
	else
	{
		/* Set a time out so accept() will just wait for a connection. */
		xResult = FreeRTOS_setsockopt( pxTelnet->xParentSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );

		if( xResult >= 0 )
		{
			xBindAddress.sin_addr = 0;
			xBindAddress.sin_port = FreeRTOS_htons( xPortNr );
			xResult = FreeRTOS_bind( pxTelnet->xParentSocket, &xBindAddress, sizeof( xBindAddress ) );
			if( xResult >= 0 )
			{
				xResult = FreeRTOS_listen( pxTelnet->xParentSocket, 4 );
			}
		}
	}

	FreeRTOS_printf( ( "xTelnetCreate: socket created\n", xResult ) );

	return xResult;
}
/*-----------------------------------------------------------*/
