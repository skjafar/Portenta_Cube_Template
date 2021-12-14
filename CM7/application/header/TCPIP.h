/*
 * TCPIP.h
 *
 * Created: 11/17/2018 11:07:59 AM
 *  Author: Sofian.jafar
 */ 


#ifndef TCPIP_H_
#define TCPIP_H_


//#include "utils.h"

#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <queue.h>
#include <FreeRTOS_IP.h>
#include "FreeRTOS_Sockets.h"

#include "TCPConsole.h"
#include "FreeRTOS_CLI_Commands.h"
#include "application_commands.h"

void TCPIP_init(void);

/*
 * Open and configure the TCP socket.
 */
Socket_t prvOpenTCPServerSocket( uint16_t usPort );

/*
 * Open and configure the UDP socket.
 */
Socket_t prvOpenUDPServerSocket( uint16_t usPort );

/*
 * A connected socket is being closed.  Ensure the socket is closed at both ends
 * properly.
 */
void prvGracefulShutdown( Socket_t xSocket );

#endif /* TCPIP_H_ */
