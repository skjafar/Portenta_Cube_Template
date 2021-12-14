/*
    FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
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

/*
 * An optimised memcpy() and memset() in plain C.
 */

#include <string.h>
#include <stdint.h>

#ifndef SIMPLE_MEMCPY
	#define SIMPLE_MEMCPY		( 0 )
#endif

#ifndef SIMPLE_MEMSET
	#define SIMPLE_MEMSET		( 0 )
#endif

#ifndef MEMCPY_USES_LOOP_COUNTER
	#ifdef __GNUC__
		/* MEMCPY_USES_LOOP_COUNTER must be defined as 1 in case
		GCC is being used at optimisation level -O3. */
		#define MEMCPY_USES_LOOP_COUNTER	( 1 )
	#else
		#define MEMCPY_USES_LOOP_COUNTER	( 0 )
	#endif
#endif

#if( SIMPLE_MEMCPY != 0 )
void *memcpy( void *pvDest, const void *pvSource, size_t ulBytes )
{
unsigned char *pcDest = ( unsigned char * ) pvDest, *pcSource = ( unsigned char * ) pvSource;
size_t x;

	for( x = 0; x < ulBytes; x++ )
	{
		*pcDest = *pcSource;
		pcDest++;
		pcSource++;
	}

	return pvDest;
}
/*-----------------------------------------------------------*/
#endif /* SIMPLE_MEMCPY */

union xPointer {
	uint8_t *u8;
	uint16_t *u16;
	uint32_t *u32;
	uint32_t uint32;
};

#if( SIMPLE_MEMCPY == 0 )
void *memcpy( void *pvDest, const void *pvSource, size_t ulBytes )
{
union xPointer pxDestination;
union xPointer pxSource;
union xPointer pxLastSource;
uint32_t ulAlignBits;

	pxDestination.u8 = ( uint8_t * ) pvDest;
	pxSource.u8 = ( uint8_t * ) pvSource;
	pxLastSource.u8 = pxSource.u8 + ulBytes;

	ulAlignBits = ( pxDestination.uint32 & 0x03 ) ^ ( pxSource.uint32 & 0x03 );

	if( ( ulAlignBits & 0x01 ) == 0 )
	{
		if( ( ( pxSource.uint32 & 1 ) != 0 ) && ( pxSource.u8 < pxLastSource.u8 ) )
		{
			*( pxDestination.u8++ ) = *( pxSource.u8++) ;
		}
		/* 16-bit aligned here */
		if( ( ulAlignBits & 0x02 ) != 0 )
		{
			uint32_t extra = pxLastSource.uint32 & 0x01ul;

			pxLastSource.uint32 &= ~0x01ul;

			#if( MEMCPY_USES_LOOP_COUNTER != 0 )
			{
				while( ( pxSource.u16 < pxLastSource.u16 ) && ( ulBytes != 0ul ) )
				{
					*( pxDestination.u16++ ) = *( pxSource.u16++) ;
					ulBytes -= 2;
				}
			}
			#else
			{
				while( pxSource.u16 < pxLastSource.u16 )
				{
					*( pxDestination.u16++ ) = *( pxSource.u16++) ;
				}
			}
			#endif
			pxLastSource.uint32 |= extra;
		}
		else
		{
			int iCount;
			uint32_t extra;

			if( ( ( pxSource.uint32 & 2 ) != 0 ) && ( pxSource.u8 < pxLastSource.u8 - 1 ) )
			{
				*( pxDestination.u16++ ) = *( pxSource.u16++) ;
			}
			// 32-bit aligned
			extra = pxLastSource.uint32 & 0x03ul;

			pxLastSource.uint32 &= ~0x03ul;
			iCount = pxLastSource.u32 - pxSource.u32;
			while( iCount > 8 )
			{
				/* Copy 32 bytes */
				/* Normally it doesn't make sense to make this list much longer because
				the indexes will get too big, and therefore longer instructions are needed. */
				pxDestination.u32[ 0 ] = pxSource.u32[ 0 ];
				pxDestination.u32[ 1 ] = pxSource.u32[ 1 ];
				pxDestination.u32[ 2 ] = pxSource.u32[ 2 ];
				pxDestination.u32[ 3 ] = pxSource.u32[ 3 ];
				pxDestination.u32[ 4 ] = pxSource.u32[ 4 ];
				pxDestination.u32[ 5 ] = pxSource.u32[ 5 ];
				pxDestination.u32[ 6 ] = pxSource.u32[ 6 ];
				pxDestination.u32[ 7 ] = pxSource.u32[ 7 ];
				pxDestination.u32 += 8;
				pxSource.u32 += 8;
				iCount -= 8;
			}

			#if( MEMCPY_USES_LOOP_COUNTER != 0 )
			{
				while( ( pxSource.u32 < pxLastSource.u32 ) && ( ulBytes != 0ul ) )
				{
					*( pxDestination.u32++ ) = *( pxSource.u32++) ;
					ulBytes -= 4;
				}
			}
			#else
			{
				while( pxSource.u32 < pxLastSource.u32 )
				{
					*( pxDestination.u32++ ) = *( pxSource.u32++) ;
				}
			}
			#endif
			pxLastSource.uint32 |= extra;
		}
	}
	else
	{
		/* This it the worst alignment, e.g. 0x80000 and 0xA0001,
		only 8-bits copying is possible. */
		int iCount = pxLastSource.u8 - pxSource.u8;
		while( iCount > 8 )
		{
			/* Copy 8 bytes the hard way */
			pxDestination.u8[ 0 ] = pxSource.u8[ 0 ];
			pxDestination.u8[ 1 ] = pxSource.u8[ 1 ];
			pxDestination.u8[ 2 ] = pxSource.u8[ 2 ];
			pxDestination.u8[ 3 ] = pxSource.u8[ 3 ];
			pxDestination.u8[ 4 ] = pxSource.u8[ 4 ];
			pxDestination.u8[ 5 ] = pxSource.u8[ 5 ];
			pxDestination.u8[ 6 ] = pxSource.u8[ 6 ];
			pxDestination.u8[ 7 ] = pxSource.u8[ 7 ];
			pxDestination.u8 += 8;
			pxSource.u8 += 8;
			iCount -= 8;
		}
	}
	#if( MEMCPY_USES_LOOP_COUNTER != 0 )
	{
		while( ( pxSource.u8 < pxLastSource.u8 ) && ( ulBytes != 0ul ) )
		{
			*( pxDestination.u8++ ) = *( pxSource.u8++ );
			ulBytes--;
		}
	}
	#else
	{
		while( pxSource.u8 < pxLastSource.u8 )
		{
			*( pxDestination.u8++ ) = *( pxSource.u8++ );
		}
	}
	#endif
	return pvDest;
}
#endif /* SIMPLE_MEMCPY == 0 */
/*-----------------------------------------------------------*/

#if( SIMPLE_MEMSET != 0 )
void *memset( void *pvDest, int iValue, size_t ulBytes )
{
unsigned char *pcDest = ( unsigned char * ) pvDest;
size_t x;

	for( x = 0; x < ulBytes; x++ )
	{
		*pcDest = ( unsigned char ) iValue;
		pcDest++;
	}

	return pvDest;
}
#endif /* SIMPLE_MEMSET != 0 */
/*-----------------------------------------------------------*/


#if( SIMPLE_MEMSET == 0 )
void *memset(void *pvDest, int iValue, size_t ulBytes)
{
union xPointer pxDestination;
union xPointer pxLast;
uint32_t ulPattern;

	pxDestination.u8 = ( uint8_t * ) pvDest;
	pxLast.u8 = pxDestination.u8 + ulBytes;

	if( ulBytes >= 8 )
	{
	uint32_t ulAlignBits;
	uint32_t ulExtra;
	int iCount;

		ulPattern = iValue & 0xff;
		ulPattern |= ( ulPattern << 8 ) | ( ulPattern << 16 ) | ( ulPattern << 24 );
		ulAlignBits = ( pxDestination.uint32 & 0x03 );
		if( ulAlignBits != 0 )
		{
			ulAlignBits = 4 - ulAlignBits;
			while( ulAlignBits-- > 0 )
			{
				pxDestination.u8[ 0 ] = ( uint8_t )iValue;
				pxDestination.u8++;
			}
		}
		/* Strip-off the last 1 up-to 3 bytes because they can not be set in a 32-bit
		memory instruction. */
		ulExtra = pxLast.uint32 & 0x03ul;

		pxLast.uint32 &= ~0x03ul;
		iCount = ( int ) ( pxLast.u32 - pxDestination.u32 );
		while( iCount > 8 )
		{
			/* Set 8 * 4 bytes and save some increments */
			/* It won't make sense to make this list much longer than 8 assignments. */
			pxDestination.u32[ 0 ] = ulPattern;
			pxDestination.u32[ 1 ] = ulPattern;
			pxDestination.u32[ 2 ] = ulPattern;
			pxDestination.u32[ 3 ] = ulPattern;
			pxDestination.u32[ 4 ] = ulPattern;
			pxDestination.u32[ 5 ] = ulPattern;
			pxDestination.u32[ 6 ] = ulPattern;
			pxDestination.u32[ 7 ] = ulPattern;
			pxDestination.u32 += 8;
			iCount -= 8;
		}

		#if( MEMCPY_USES_LOOP_COUNTER != 0 )
		{
			while( ( pxDestination.u32 < pxLast.u32 ) && ( ulBytes != 0ul ) )
			{
				pxDestination.u32[0] = ulPattern;
				pxDestination.u32++;
				ulBytes += 4;
			}
		}
		#else
		{
			while( pxDestination.u32 < pxLast.u32 )
			{
				pxDestination.u32[0] = ulPattern;
				pxDestination.u32++;
			}
		}
		#endif
		pxLast.uint32 |= ulExtra;
	}

	#if( MEMCPY_USES_LOOP_COUNTER != 0 )
	{
		while( ( pxDestination.u8 < pxLast.u8 ) && ( ulBytes != 0ul ) )
		{
			pxDestination.u8[ 0 ] = ( uint8_t ) iValue;
			pxDestination.u8++;
			ulBytes++;
		}
	}
	#else
	{
		while( pxDestination.u8 < pxLast.u8 )
		{
			pxDestination.u8[ 0 ] = ( uint8_t ) iValue;
			pxDestination.u8++;
		}
	}
	#endif
	return pvDest;
}
#endif /* SIMPLE_MEMSET -= 0 */
/*-----------------------------------------------------------*/
