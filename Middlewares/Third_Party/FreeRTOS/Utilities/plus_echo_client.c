/*
 * A sample echo client using FreeRTOS+TCP
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

#define RECV_TMOUT		1000
#define SEND_TMOUT		1000

//static char pcRetString[ 1024 ];
#define pcRetString		pcPlusBuffer

#define STACK_ECHO_SERVER_TASK  ( 512 + 256 )
#define	PRIO_ECHO_SERVER_TASK     2

struct freertos_sockaddr xEchoServerAddress;

static void handle_connection( void );

extern int verboseLevel;

//static const char tosend[] =
//	"{\"upper SN\":926697234,\"lower "
//	"SN\":892496226,\"uuid\":\"e1dfb7a7-8bdb4fd5-8ec9-806ff78871ee\","
//	"\"build\":\"580E3EEA\",\"timeslot\":1477382400,\"nodeAddr\":2,\"metAddr\":61069,\"consT1\":4757.629,"
//	"\"consT2\":8527.651,\"prodT1\":2805.474,\"prodT2\":624.111,\"Qpos\":0.000,\"Qneg\":0.000,\"Q1\":0.000,\"Q2\":0.000,\"Q3\":0.000,\"Q4\":0.000}";

#define tosend  pcPlusBuffer

static SemaphoreHandle_t xClientSemaphore;
int plus_test_active = 1;
int plus_test_two_way = 1;

/* FreeRTOS+TCP echo server */
void plus_echo_client_thread( void *parameters )
{
	TickType_t t1 = xTaskGetTickCount();

	xClientSemaphore = xSemaphoreCreateBinary();
	configASSERT( xClientSemaphore != NULL );

	xEchoServerAddress.sin_port = FreeRTOS_htons( ECHO_SERVER_PORT );
	xEchoServerAddress.sin_addr = echoServerIPAddress();
	for( ;; )
	{
	TickType_t t2 = xTaskGetTickCount();

		if( ( t2 - t1 ) > 1000 )
		{
			if( plus_test_active != 0 )
			{
				handle_connection();
			}
			t1 = t2;
		}
		vTaskDelay( 10ul );
	}
}

static void handle_connection()
{
xWinProperties_t winProps;
struct freertos_sockaddr xBindAddress;
Socket_t xSocket;
BaseType_t xReceiveTimeOut = RECV_TMOUT;
BaseType_t xSendTimeOut = SEND_TMOUT;
int rc;

	/* create a TCP socket */
	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_STREAM, FREERTOS_IPPROTO_TCP );
	if( xSocket == NULL )
	{
		FreeRTOS_printf( ( "FreeRTOS_socket: failed\n" ) );
		return;
	}

	/* bind to port 80 at any interface */
	memset( &( xBindAddress ), '\0', sizeof xBindAddress );
	xBindAddress.sin_port = FreeRTOS_htons( 0 );

	if( FreeRTOS_bind( xSocket, &xBindAddress, sizeof( xBindAddress ) ) < 0 )
	{
		FreeRTOS_closesocket( xSocket );
		return;
	}

	/* The properties of the listening socket will be inherited by all child sockets. */
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_RCVTIMEO, &xReceiveTimeOut, sizeof( xReceiveTimeOut ) );
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_SNDTIMEO, &xSendTimeOut, sizeof( xSendTimeOut ) );

	memset(&winProps, '\0', sizeof winProps);
	// Size in units of MSS
	winProps.lTxBufSize   = 20480;
	winProps.lTxWinSize   = 8;
	winProps.lRxBufSize   = 20480;
	winProps.lRxWinSize   = 8;

	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &winProps, sizeof( winProps ) );
	FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_SET_SEMAPHORE, ( void * ) &xClientSemaphore, sizeof( xClientSemaphore ) );
	{
	WinProperties_t xWinProperties;

		memset(&xWinProperties, '\0', sizeof xWinProperties);

		xWinProperties.lTxBufSize   = PLUS_TEST_TX_BUFSIZE;	/* Units of bytes. */
		xWinProperties.lTxWinSize   = PLUS_TEST_TX_WINSIZE;	/* Size in units of MSS */
		xWinProperties.lRxBufSize   = PLUS_TEST_RX_BUFSIZE;	/* Units of bytes. */
		xWinProperties.lRxWinSize   = PLUS_TEST_RX_WINSIZE; /* Size in units of MSS */

		FreeRTOS_setsockopt( xSocket, 0, FREERTOS_SO_WIN_PROPERTIES, ( void * ) &xWinProperties, sizeof( xWinProperties ) );
	}

	rc = FreeRTOS_connect( xSocket, &xEchoServerAddress, sizeof( xEchoServerAddress ) );
	if( rc != 0 )
	{
		FreeRTOS_printf( ( "FreeRTOS_connect: rc %d\n", rc ) ) ;
		goto leave;
	}

	{
		char hasShutdown = pdFALSE;
		int length = sizeof tosend;
		int rc;
		int sendCount = 0, recvCount = 0;
		int left = CLIENT_SEND_COUNT;
		for (;;)
		{
		BaseType_t xReceiveTimeOut = 200;
		int space;

			xSemaphoreTake( xClientSemaphore, xReceiveTimeOut );
			space = FreeRTOS_tx_space( xSocket );
			while( sendCount < CLIENT_SEND_COUNT && space >= 1460 )
			{
			int sendlength = space < length ? space : length;

				if (sendlength > left)
					sendlength = left;

				rc = FreeRTOS_send( xSocket, ( void * ) tosend, sendlength, FREERTOS_MSG_DONTWAIT );
				if( ( rc < 0 ) && ( rc != -pdFREERTOS_ERRNO_EAGAIN ) )
				{
					goto leave;
				}

				if( rc < sendlength )
				{
					FreeRTOS_printf( ( "FreeRTOS_send: only sent %lu of %lu bytes\n", rc, length ) ) ;
					break;
				}
				sendCount += rc;
				left -= rc;
				space -= rc;
			}
			for (;;)
			{
				rc = FreeRTOS_recv(	xSocket, pcRetString, sizeof( pcRetString ), FREERTOS_MSG_DONTWAIT );

				if( rc > 0 )
				{
					recvCount += rc;
					if( verboseLevel )
					{
						FreeRTOS_printf( ( "FreeRTOS_recv: rc = %d: '%-16.16s'\n", rc, pcRetString ) );
					}

				} else if( ( rc == 0 ) || ( rc == -pdFREERTOS_ERRNO_EAGAIN ) )
				{
					/* Break from the for(;;) receive loop. */
					break;
				} else if( rc < 0 )
				{
					goto leave;
				}
			}
			int send_ready = !plus_test_two_way || recvCount >= CLIENT_SEND_COUNT;
			if( !hasShutdown && send_ready && !left && FreeRTOS_tx_size( xSocket ) == 0 )
			{
				hasShutdown = pdTRUE;
				FreeRTOS_shutdown( xSocket, ~0 );
			}
		}	/* for (;;) */
	}
leave:
	FreeRTOS_closesocket( xSocket );
	xSocket = NULL;
}
