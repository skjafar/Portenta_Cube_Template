/*
 * A sample echo server using FreeRTOS+TCP
 */

#include <stdio.h>
#include <string.h>
#if __MICROBLAZE__ || __PPC__
#include "xmk.h"
#endif

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* FreeRTOS+TCP includes. */

#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "plus_echo_server.h"

#define STACK_WEBSERVER_TASK  ( 512 )
#define	PRIO_WEBSERVER_TASK     1

extern int lUDPLoggingPrintf( const char *pcFormatString, ... );

char pcPlusBuffer[ ECHO_BUFFER_LENGTH ];

static void echo_recv_task( void *pvParameters );
static void echo_send_task( void *pvParameters );

void plus_echo_start(int aValue)
{
	static int hasStarted = pdFALSE;
	plus_test_active = aValue;
	if (!hasStarted) {
		hasStarted = pdTRUE;
		xTaskCreate (plus_echo_application_thread, "plus_src", STACK_WEBSERVER_TASK, NULL, PRIO_WEBSERVER_TASK, NULL);
		xTaskCreate (plus_echo_client_thread, "plus_client", STACK_WEBSERVER_TASK, NULL, PRIO_WEBSERVER_TASK, NULL);
	}
}

//#if( ipconfigMULTI_INTERFACE != 0 )
//	uint32_t echoServerIPAddress()
//	{
//		/* Return 127.0.0.1 */
//		return FreeRTOS_htonl( ECHO_SERVER_LOOPBACK_IP_ADDRESS );
//	}
//#else
//	uint32_t echoServerIPAddress()
//	{
//		/* Return 127.0.0.1 */
//		uint32_t ulIPAddress;
//		#if( ipconfigMULTI_INTERFACE != 0 )
//		FreeRTOS_GetAddressConfiguration( &ulIPAddress, NULL, NULL, NULL, NULL );
//		#else
//		FreeRTOS_GetAddressConfiguration( &ulIPAddress, NULL, NULL, NULL );
//		#endif
//		return ulIPAddress;
//	}
//#endif

typedef struct {
	Socket_t sd;
	volatile union {
		struct {
			unsigned
				recv_connected : 1,
				send_connected : 1,
				conn_closed : 1;
		};
		unsigned flags;
	};
	volatile unsigned bytes_recv;
	unsigned bytes_send;
	unsigned uSequenceNr;
	TaskHandle_t xRecvTaskHandle;
	TaskHandle_t xSendTaskHandle;
	TickType_t xStartTime;
	SemaphoreHandle_t xSemaphore;
} Connection_t;

#define STACK_ECHO_SERVER_TASK  ( 512 + 256 )
#define	PRIO_ECHO_SERVER_TASK     2

/* FreeRTOS+TCP echo server */
void plus_echo_application_thread( void *parameters )
{
	Socket_t xSocket, new_sd;
	struct freertos_sockaddr xAddress, xRemote;
	socklen_t xFromSize = sizeof xRemote;
	unsigned uSequenceNr = 0;
	BaseType_t xRecvTimeOut = 100,  xSendTimeOut = 100;

	/* create a TCP socket */
	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
	if( xSocket == NULL )
	{
		vTaskDelete( NULL );
		return;
	}

	/* bind to port 80 at any interface */
	memset( &( xAddress ), '\0', sizeof xAddress );
	xAddress.sin_port = FreeRTOS_htons( ECHO_SERVER_PORT );
	xAddress.sin_addr = echoServerIPAddress();
	xAddress.sin_len = sizeof( xAddress );
	xAddress.sin_family = FREERTOS_AF_INET;

	if( FreeRTOS_bind( xSocket, &xAddress, sizeof( xAddress ) ) < 0 )
	{
		FreeRTOS_closesocket( xSocket );
		vTaskDelete( NULL );
		return;
	}

	/* The properties of the listening socket will be inherited by all child sockets. */
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_RCVTIMEO, &xRecvTimeOut, sizeof( xRecvTimeOut ) );
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );
	{
	WinProperties_t xWinProperties;

		memset(&xWinProperties, '\0', sizeof xWinProperties);

		xWinProperties.lTxBufSize   = PLUS_TEST_TX_BUFSIZE;	/* Units of bytes. */
		xWinProperties.lTxWinSize   = PLUS_TEST_TX_WINSIZE;	/* Size in units of MSS */
		xWinProperties.lRxBufSize   = PLUS_TEST_RX_BUFSIZE;	/* Units of bytes. */
		xWinProperties.lRxWinSize   = PLUS_TEST_RX_WINSIZE; /* Size in units of MSS */

		FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &xWinProperties, sizeof( xWinProperties ) );
	}

	/* listen for incoming connections, accept at most 8 clients simultaneously. */
	FreeRTOS_listen( xSocket, 8 );

	lUDPLoggingPrintf( "Started echo server on port 8080\n" );

	for( ;; )
	{
		new_sd = FreeRTOS_accept( xSocket, &xRemote, &xFromSize );
		/* Start 2 tasks for each request: one for RX, one for TX. */
		if( new_sd != NULL )
		{
		struct freertos_sockaddr xRemoteAddress;

			FreeRTOS_GetRemoteAddress( new_sd, &xRemoteAddress );

			lUDPLoggingPrintf( "Echo client from %xip:%u\n",
				FreeRTOS_ntohl( xRemoteAddress.sin_addr ),
				FreeRTOS_ntohs( xRemoteAddress.sin_port ) );


			/* The properties of the listening socket will be inherited by all child sockets. */
			FreeRTOS_setsockopt( new_sd, 0, FREERTOS_SO_RCVTIMEO, &xRecvTimeOut, sizeof( xRecvTimeOut ) );
			FreeRTOS_setsockopt( new_sd, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

			Connection_t *t = ( Connection_t *) pvPortMalloc( sizeof *t );
			memset( t, '\0', sizeof *t );
			t->sd = new_sd;
			t->recv_connected = 1;
			t->send_connected = 1;
			t->uSequenceNr = uSequenceNr++;

			t->xSemaphore = xQueueCreate (16, 0);

			configASSERT( t->xSemaphore != NULL );

			xTaskCreate (echo_send_task, "plus_send", STACK_ECHO_SERVER_TASK, (void*)t, PRIO_ECHO_SERVER_TASK+0, &( t->xSendTaskHandle ) );
			xTaskCreate (echo_recv_task, "plus_recv", STACK_ECHO_SERVER_TASK, (void*)t, PRIO_ECHO_SERVER_TASK+1, &( t->xRecvTaskHandle ) );
		}
	}
}

extern volatile TickType_t xTickCount;

static void echo_recv_task( void *pvParameters )
{
volatile Connection_t *t = ( volatile Connection_t * ) pvParameters;
int laststatus = pdTRUE;

	t->recv_connected = 1;
	lUDPLoggingPrintf( "echo_recv: Starting Session %u\n", t->uSequenceNr );
	for( ;; )
	{
	int32_t lReturnCode = 0;

		lReturnCode = FreeRTOS_recv( t->sd, (void *)pcPlusBuffer, sizeof pcPlusBuffer, 0 );
		if( ( lReturnCode < 0 ) && ( lReturnCode != -pdFREERTOS_ERRNO_EAGAIN ) )
		{
			t->conn_closed = pdTRUE;
			/* No separate errno variable: lReturnCode contains the negative value
			of what errno would be, e.g. it might return -pdFREERTOS_ERRNO_ENOTCONN. */
			lUDPLoggingPrintf( "echo_recv: errno = %d (recv %u sent %u)\n", -lReturnCode,
				t->bytes_recv, t->bytes_send );
			break;
		}
		if( lReturnCode > 0 )
		{
			/* Bytes have been received. */
			if( t->xStartTime == 0ul )
			{
				t->xStartTime = xTickCount;
			}
			t->bytes_recv += lReturnCode;

			xSemaphoreGive( t->xSemaphore );
		}
		else
		{
			/* recv() returned 0, check the connection status. */
			int status = FreeRTOS_issocketconnected(t->sd);
			if( laststatus != status )
			{
				laststatus = status;
				lUDPLoggingPrintf( "echo_recv: status %d\n", status );
				if( status != pdTRUE )
				{
					t->conn_closed = pdTRUE;
					break;
				}
			}
		}
	}

	t->recv_connected = 0;
	/* This task doesn't use the socket any more. Now wait
	for the sending task to confirm that it stopped. */
	while( t->send_connected )
	{
		vTaskDelay( 10 );
		xSemaphoreGive( t->xSemaphore );
	}
	/* Close the socket. */
	FreeRTOS_closesocket( t->sd );
	lUDPLoggingPrintf( "echo_recv: %u bytes\n", t->bytes_recv );
	{
		TickType_t xDelta = t->xStartTime ? ( xTickCount - t->xStartTime ) : 0;
		uint32_t ulAverage = 0;
		if( xDelta != 0ul )
		{
			ulAverage = ( uint32_t ) ( ( uint64_t ) ( 1000ull * t->bytes_recv ) / ( xDelta * 1024ull ) );
		}

		lUDPLoggingPrintf( "echo_recv: %u bytes %lu KB/sec %lu ms\n", t->bytes_recv, ulAverage, xDelta );
	}

	vSemaphoreDelete( t->xSemaphore );

	vPortFree( ( void * ) t );

	vTaskDelete( NULL );
}


static void echo_send_task( void *pvParameters )
{
volatile Connection_t *t = ( volatile Connection_t * ) pvParameters;
unsigned total_sent = 0;

	lUDPLoggingPrintf( "echo_send: Starting (recv %d)\n", t->recv_connected );


	while( t->recv_connected != 0 )
	{
	int diff;
	
		diff = t->bytes_recv - t->bytes_send;
		if( plus_test_two_way && diff > 0 )
		{
			int count = ( diff > 1460 ) ? 1460 : diff;
			int rc = FreeRTOS_send( t->sd, (const void *)pcPlusBuffer, count, 0 );

			total_sent += count;
			t->bytes_send += count;
		}

		if( t->conn_closed != pdFALSE )
		{
			break;
		}
		if( !plus_test_two_way || t->bytes_recv == t->bytes_send )
		{
			xSemaphoreTake( t->xSemaphore, 2 );
		}
	}

	{
		TickType_t xDelta = t->xStartTime ? ( xTickCount - t->xStartTime ) : 0;
		uint32_t ulAverage = 0;
		if( xDelta != 0ul )
		{
			ulAverage = ( uint32_t ) ( ( uint64_t ) ( 1000ull * total_sent ) / ( xDelta * 1024ull ) );
		}

		lUDPLoggingPrintf( "echo_send: %u bytes %lu KB/sec %lu ms\n", total_sent, ulAverage, xDelta );
	}
	/* Inform the other task that the Send task is ready. */
	t->send_connected = 0;

	/* kill this task. */
	vTaskDelete( NULL );
}
