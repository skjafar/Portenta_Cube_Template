/*
 * telnet.h
 */


#ifndef	TELNET_H_

#define TELNET_H_

typedef struct xTelnetClient
{
	Socket_t xSocket;
	struct freertos_sockaddr xAddress;
	struct xTelnetClient *pxNext;
} TelnetClient_t;

typedef struct
{
	Socket_t xParentSocket;
	TelnetClient_t *xClients;
} Telnet_t;

BaseType_t xTelnetCreate( Telnet_t * pxTelnet, BaseType_t xPortNr );
BaseType_t xTelnetSend( Telnet_t * pxTelnet, struct freertos_sockaddr *pxAddress, const char *pcBuffer, BaseType_t xLength );
BaseType_t xTelnetRecv( Telnet_t * pxTelnet, struct freertos_sockaddr *pxAddress, char *pcBuffer, BaseType_t xMaxLength );

#endif
