/*
 * Copyright (c) 2007 Xilinx, Inc.  All rights reserved.
 *
 * Xilinx, Inc.
 * XILINX IS PROVIDING THIS DESIGN, CODE, OR INFORMATION "AS IS" AS A
 * COURTESY TO YOU.  BY PROVIDING THIS DESIGN, CODE, OR INFORMATION AS
 * ONE POSSIBLE   IMPLEMENTATION OF THIS FEATURE, APPLICATION OR
 * STANDARD, XILINX IS MAKING NO REPRESENTATION THAT THIS IMPLEMENTATION
 * IS FREE FROM ANY CLAIMS OF INFRINGEMENT, AND YOU ARE RESPONSIBLE
 * FOR OBTAINING ANY RIGHTS YOU MAY REQUIRE FOR YOUR IMPLEMENTATION.
 * XILINX EXPRESSLY DISCLAIMS ANY WARRANTY WHATSOEVER WITH RESPECT TO
 * THE ADEQUACY OF THE IMPLEMENTATION, INCLUDING BUT NOT LIMITED TO
 * ANY WARRANTIES OR REPRESENTATIONS THAT THIS IMPLEMENTATION IS FREE
 * FROM CLAIMS OF INFRINGEMENT, IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#ifndef __PLUS_WEBSERVER_H__
#define __PLUS_WEBSERVER_H__

#define ECHO_BUFFER_LENGTH		7300

#define	ECHO_SERVER_PORT		32002

#define CLIENT_SEND_COUNT			( 1024u * 1024u )

#define ECHO_SERVER_LOOPBACK_IP_ADDRESS	0x7f000001

#define PLUS_TEST_TX_BUFSIZE				2*1460	/* Units of bytes. */
#define PLUS_TEST_TX_WINSIZE				1		/* Size in units of MSS */
#define PLUS_TEST_RX_BUFSIZE				2*1460	/* Units of bytes. */
#define PLUS_TEST_RX_WINSIZE				1		/* Size in units of MSS */

//#define PLUS_TEST_TX_BUFSIZE				4*1460	/* Units of bytes. */
//#define PLUS_TEST_TX_WINSIZE				2		/* Size in units of MSS */
//#define PLUS_TEST_RX_BUFSIZE				4*1460	/* Units of bytes. */
//#define PLUS_TEST_RX_WINSIZE				2		/* Size in units of MSS */

extern uint32_t echoServerIPAddress( void );


extern char pcPlusBuffer[ ECHO_BUFFER_LENGTH ];
extern int plus_test_active;
extern int plus_test_two_way;

void plus_echo_start(int aValue);

void plus_echo_application_thread(void *parameters);
void plus_echo_client_thread( void *parameters );


#endif
