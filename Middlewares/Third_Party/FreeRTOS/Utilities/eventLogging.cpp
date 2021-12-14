/*
 * eventlogging.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "eventLogging.h"

void vUDPLoggingFlush( void );

#if USE_CLOCK
	#include "setclock.h"
#endif

#if USE_MYMALLOC
#	include "myMalloc.h"
#else
	#include <FreeRTOS.h>
	#include "task.h"
	#include "timers.h"
	#include "queue.h"
	#include "semphr.h"
	#include "portable.h"
#endif

#if( __SAM4E16E__ )
void vUDPLoggingFlush( void );
#define	vFlushLogging	vUDPLoggingFlush
#endif

#if USE_LOG_EVENT

extern "C" int lUDPLoggingPrintf( const char *pcFormatString, ... );

uint32_t xTaskGetTickCount();

SEventLogging xEventLogs;

#if( STATIC_LOG_MEMORY != 0 )

SLogEvent xEvents[LOG_EVENT_COUNT];
const char *strings[LOG_EVENT_COUNT];

#endif

int iEventLogInit()
{
	if( xEventLogs.hasInit == pdFALSE )
	{
		xEventLogs.hasInit = pdTRUE;
#if USE_MYMALLOC
		xEventLogs.events = (SLogEvent *)myMalloc (sizeof xEventLogs.events[0] * LOG_EVENT_COUNT, pdFALSE);
#elif( STATIC_LOG_MEMORY != 0 )
		xEventLogs.events = xEvents;
		{
			int i;
			for (i = 0; i < LOG_EVENT_COUNT; i++) {
				strings[i] = xEventLogs.events[i].pcMessage;
			}
		}
#else
		xEventLogs.events = (SLogEvent *)pvPortMalloc (sizeof xEventLogs.events[0] * LOG_EVENT_COUNT);
#endif
		if( xEventLogs.events != NULL )
		{
			memset (xEventLogs.events, '\0', sizeof xEventLogs.events[0] * LOG_EVENT_COUNT);
			xEventLogs.initOk = pdTRUE;
		}
	}
	return xEventLogs.initOk;
}

int iEventLogClear ()
{
int rc;
	if (!iEventLogInit ()) {
		rc = 0;
	} else {
		rc = xEventLogs.writeIndex;
		xEventLogs.writeIndex = 0;
		xEventLogs.wrapped = pdFALSE;
		xEventLogs.onhold = pdFALSE;
	}
	return rc;
}

#include "hr_gettime.h"

//static uint32_t getTCTime (uint32_t *puSeconds)
//{
//uint64_t ullUsec;
//
//	ullUsec = ullGetHighResolutionTime( );
//	*puSeconds = ullUsec / 1000000ULL;
//	return ( uint32_t ) ( ullUsec % 1000000ULL );
//}

void eventLogAdd (const char *apFmt, ...)
{
int writeIndex, nextIndex;
SLogEvent *pxEvent;
va_list args;
//int rc;

	if (!iEventLogInit () || xEventLogs.onhold)
		return;
	writeIndex = xEventLogs.writeIndex++;
	if (xEventLogs.writeIndex >= LOG_EVENT_COUNT) {
#if( EVENT_MAY_WRAP == 0 )
		xEventLogs.writeIndex--;
		return;
#endif
		xEventLogs.writeIndex = 0;
		xEventLogs.wrapped = pdTRUE;
	}
	nextIndex = xEventLogs.writeIndex;
	pxEvent = &xEventLogs.events[writeIndex];

//	rc = sprintf (pxEvent->pcMessage, "%6lu ", xTaskGetTickCount());
	va_start (args, apFmt);
	vsnprintf (pxEvent->pcMessage, sizeof pxEvent->pcMessage, apFmt, args);
	va_end (args);

	pxEvent->ullTimestamp = ullGetHighResolutionTime( );

	pxEvent = &xEventLogs.events[nextIndex];
	pxEvent->pcMessage[0] = '\0';
}

void eventFreeze()
{
	xEventLogs.onhold = pdTRUE;
}

void vFlushLogging();

void eventLogDump ()
{
int count;
int index;
int i;
uint64_t ullLastTime;

//#if USE_CLOCK
//	unsigned cpuTicksSec = clk_getFreq (&clk_cpu);
//#elif defined(__AVR32__)
//	unsigned cpuTicksSec = 48000000;
//#else
//	unsigned cpuTicksSec = 1000000;
//#endif
//unsigned cpuTicksMs = cpuTicksSec / 1000;

eventLogAdd ("now");
xEventLogs.onhold = 1;

count = xEventLogs.wrapped ? LOG_EVENT_COUNT : xEventLogs.writeIndex;
index = xEventLogs.wrapped ? xEventLogs.writeIndex : 0;
ullLastTime = xEventLogs.events[index].ullTimestamp;

	lUDPLoggingPrintf("Nr:    s   ms  us  %d events\n", count);
//192.168.2.114     12.680.802 [SvrWork   ] Nr:    s   ms  us  8 events
//192.168.2.114     12.680.899 [SvrWork   ]    0:       0.000 PHY reset 1 ports
//192.168.2.114     12.704.773 [SvrWork   ]    1:       0.271 adv: 01E1 config 1200
//192.168.2.114     12.728.584 [SvrWork   ]    2:       5.151 AN start
//192.168.2.114     12.752.391 [SvrWork   ]    3: 003.570.839 AN done 00
//192.168.2.114     12.776.203 [SvrWork   ]    4:     123.856 PHY LS now 01
//192.168.2.114     12.800.011 [SvrWork   ]    5:       0.087 AN start
//192.168.2.114     12.823.819 [SvrWork   ]    6: 003.571.298 AN done 00
//192.168.2.114     12.847.629 [SvrWork   ]    7: 005.348.585 now

	for( i = 0; i < count; i++ )
	{
	SLogEvent *pxEvent;
	unsigned delta;

		pxEvent = xEventLogs.events + index;
		if( pxEvent->ullTimestamp >= ullLastTime )
		{
			if (pxEvent->ullTimestamp >= ullLastTime) {
				delta = ( unsigned ) ( pxEvent->ullTimestamp - ullLastTime );
			} else {
				delta = ( unsigned ) ( ullLastTime - pxEvent->ullTimestamp );
			}
		}
		else
		{
			delta = 0u;
		}

		// 2018-08-01 22:05:24.117 0:00:06   17:      0       0.000 4152759546 GET_DESCRIPTOR 0200
		// 2018-08-01 22:20:00.321 0:00:03   36:      1 4153639101 REQ_TYPE_CLASS 33
		// 2018-08-01 22:20:00.322 0:00:03   37:      0 4153639102 REQ_TYPE_CLASS 34

		lUDPLoggingPrintf("%4d: %6u %s\n", i, delta, pxEvent->pcMessage);
		if( ++index >= LOG_EVENT_COUNT )
		{
			index = 0;
		}
		ullLastTime = pxEvent->ullTimestamp;
		//if ((i % 8) == 0)
		//	vUDPLoggingFlush();
		vTaskDelay(20);
	}
	vTaskDelay( 200 );
	iEventLogClear ();
}

#endif /* USE_LOG_EVENT */
