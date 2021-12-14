/*
    This file has been modified to suite the application of the portenta board.

    All rights reserved.

    FreeRTOS V9.0.0 - Copyright (C) 2016 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    ***************************************************************************
    >>!   NOTE: The modification to the GPL is included to allow you to     !<<
    >>!   distribute a combined work that includes FreeRTOS without being   !<<
    >>!   obliged to provide the source code for proprietary components     !<<
    >>!   outside of the FreeRTOS kernel.                                   !<<
    ***************************************************************************

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available on the following
    link: http://www.freertos.org/a00114.html

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that is more than just the market leader, it     *
     *    is the industry's de facto standard.                               *
     *                                                                       *
     *    Help yourself get started quickly while simultaneously helping     *
     *    to support the FreeRTOS project by purchasing a FreeRTOS           *
     *    tutorial book, reference manual, or both:                          *
     *    http://www.FreeRTOS.org/Documentation                              *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
    the FAQ page "My application does not run, what could be wrong?".  Have you
    defined configASSERT()?

    http://www.FreeRTOS.org/support - In return for receiving this top quality
    embedded software for free we request you assist our global community by
    participating in the support forum.

    http://www.FreeRTOS.org/training - Investing in training allows your team to
    be as productive as possible as early as possible.  Now you can receive
    FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
    Ltd, and the world's leading authority on the world's leading RTOS.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
    Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

    http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
    Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and commercial middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

/* FreeRTOS+TCP includes. */
#include "FreeRTOS_IP.h"
#include "FreeRTOS_Sockets.h"

#include "TCPConsole.h"
#include "TCPIP.h"

#include "application_config.h"

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE	60

/* Dimensions the buffer into which string outputs can be placed. */
#define cmdMAX_OUTPUT_SIZE	1024

/* Dimensions the buffer passed to the recv() call. */
#define cmdSOCKET_INPUT_BUFFER_SIZE 60

/* DEL acts as a backspace. */
#define cmdASCII_DEL		( 0x7F )

/* The maximum time to wait for a closing socket to close. */
// #define cmdSHUTDOWN_DELAY	( pdMS_TO_TICKS( 5000 ) )

static TaskHandle_t         xCreatedAppTCPCLITask;

/*
 * The task that runs FreeRTOS+CLI.
 */
static void prvTCPCommandInterpreterTask( void *pvParameters );

/*-----------------------------------------------------------*/

/*
 * Various buffers used by the command line interpreter.
 */
static char cOutputString[ cmdMAX_OUTPUT_SIZE ], cLocalBuffer[ cmdSOCKET_INPUT_BUFFER_SIZE ];
static char cInputString[ cmdMAX_INPUT_SIZE ], cLastInputString[ cmdMAX_INPUT_SIZE ];

/* Const messages output by the command console. */
static const char * const pcWelcomeMessage = "Portenta command server.\r\nType \"help\" to view a list of registered commands.\r\nType quit to end a session.\r\n";

/**
 * \brief Create OS task for console input/output
 */
void task_app_TCP_CLI_create(void)
{
    /* Create the task that handles the CLI. */
    if (xTaskCreate(
            prvTCPCommandInterpreterTask, TCP_CLI_TASK_NAME, TCP_CLI_TASK_STACK_SIZE, ( void * ) TCP_CLI_PORT, TCP_CLI_TASK_PRIORITY, &xCreatedAppTCPCLITask)
        != pdPASS) {
        while (1) {
            ;
        }
    }
}


void prvTCPCommandInterpreterTask( void *pvParameters )
{
    int32_t lBytes, lByte, lSent;
    char cRxedChar, cInputIndex = 0;
    BaseType_t xMoreDataToFollow;
    struct freertos_sockaddr xClient;
    Socket_t xListeningSocket, xConnectedSocket;
    socklen_t xSize = sizeof( xClient );
    
    // store the task number for deciding control priority later
    TCP_CLITaskNumber = pxGetCurrentTaskNumber();

    memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

    for( ;; )
    {
        /* Attempt to open the socket.  The port number is passed in the task
        parameter.  The strange casting is to remove compiler warnings on 32-bit
        machines.  NOTE:  The FREERTOS_SO_REUSE_LISTEN_SOCKET option is used,
        so the listening and connecting socket are the same - meaning only one
        connection will be accepted at a time, and that xListeningSocket must
        be created on each iteration. */
        xListeningSocket = prvOpenTCPServerSocket( ( uint16_t ) ( ( uint32_t ) pvParameters ) & 0xffffUL );

        /* Nothing for this task to do if the socket cannot be created. */
        if( xListeningSocket == FREERTOS_INVALID_SOCKET )
        {
            vTaskDelete( NULL );
        }

        /* Wait for an incoming connection. */
        xConnectedSocket = FreeRTOS_accept( xListeningSocket, &xClient, &xSize );

        /* The FREERTOS_SO_REUSE_LISTEN_SOCKET option is set, so the
        connected and listening socket should be the same socket. */
        configASSERT( xConnectedSocket == xListeningSocket );

        /* Send the welcome message. */
        lSent = FreeRTOS_send( xConnectedSocket,  ( void * ) pcWelcomeMessage,  strlen( pcWelcomeMessage ), 0 );

        /* Process the socket as long as it remains connected. */
        while( lSent >= 0 )
        {
            /* Receive data on the socket. */
            lBytes = FreeRTOS_recv( xConnectedSocket, cLocalBuffer, sizeof( cLocalBuffer ), 0 );

            if( lBytes >= 0 )
            {
                /* Process each received byte in turn. */
                lByte = 0;
                while( lByte < lBytes )
                {
                    /* The next character in the input buffer. */
                    cRxedChar = cLocalBuffer[ lByte ];
                    lByte++;

                    /* Newline characters are taken as the end of the command
                    string. */
                    if( cRxedChar == '\n' )
                    {
                        /* See if the command is empty, indicating that the last
                        command is to be executed again. */
                        if( cInputIndex == 0 )
                        {
                            /* Copy the last command back into the input string. */
                            strcpy( cInputString, cLastInputString );
                        }

                        /* If the command was "quit" then close the console. */
                        if( strcmp( cInputString, "quit" ) == 0 )
                        {
                            /* Fake an error code so the outer loop exits on the
                            assumption there was an error on the socket.  The
                            socket will then be shut down gracefully. */
                            lSent = -1;
                            break;
                        }

                        /* Process the input string received prior to the
                        newline. */
                        do
                        {
                            /* Pass the string to FreeRTOS+CLI. */
                            cOutputString[ 0 ] = 0x00;
                            xMoreDataToFollow = FreeRTOS_CLIProcessCommand( cInputString, cOutputString, cmdMAX_OUTPUT_SIZE );

                            /* Send the output generated by the command's
                            implementation. */
                            lSent = FreeRTOS_send( xConnectedSocket, cOutputString,  strlen( ( const char * ) cOutputString ), 0 );

                          /* Until the command does not generate any more output. */
                        } while( ( xMoreDataToFollow != pdFALSE ) && ( lSent >= 0 ) );

                        if( lSent >= 0 )
                        {
                            /* All the strings generated by the command
                            processing have been sent.  Clear the input string
                            ready to receive the next command.  Remember the
                            previous command so it can be executed again by
                            pressing [ENTER]. */
                            strcpy( cLastInputString, cInputString );
                            cInputIndex = 0;
                            memset( cInputString, 0x00, cmdMAX_INPUT_SIZE );

                            /* Transmit a spacer to make the console easier to
                            read. */
                            // lSent = FreeRTOS_send( xConnectedSocket, ( void * ) pcEndOfOutputMessage,  strlen( pcEndOfOutputMessage ), 0 );
                        }

                        if( lSent < 0 )
                        {
                            /* Socket closed? */
                            break;
                        }
                    }
                    else
                    {
                        if( cRxedChar == '\r' )
                        {
                            /* Ignore the character.  Newlines are used to
                            detect the end of the input string. */
                        }
                        else if( ( cRxedChar == '\b' ) || ( cRxedChar == cmdASCII_DEL ) )
                        {
                            /* Backspace was pressed.  Erase the last character
                            in the string - if any. */
                            if( cInputIndex > 0 )
                            {
                                cInputIndex--;
                                cInputString[ ( int ) cInputIndex ] = '\0';
                            }
                        }
                        else
                        {
                            /* A character was entered.  Add it to the string
                            entered so far.  When a \n is entered the complete
                            string will be passed to the command interpreter. */
                            if( cInputIndex < cmdMAX_INPUT_SIZE )
                            {
                                if( ( cRxedChar >= ' ' ) && ( cRxedChar <= '~' ) )
                                {
                                    cInputString[ ( int ) cInputIndex ] = cRxedChar;
                                    cInputIndex++;
                                }
                            }
                        }
                    }
                }
            }
            else
            {
                /* Socket closed? */
                break;
            }
        }

        /* Close the socket correctly. */
        prvGracefulShutdown( xListeningSocket );
    }
}
/*-----------------------------------------------------------*/






