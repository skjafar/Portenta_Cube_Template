/*
 * Rand.c
 *
 * Created: 11/15/2018 8:18:07 PM
 *  Author: Sofian.jafar
 */ 

#include "Rand.h"

/* Use by the pseudo random number generator. */
static UBaseType_t ulNextRand;

UBaseType_t uxRand( void )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL;

    /* Utility function to generate a pseudo random number. */

    ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
    return( ( int ) ( ulNextRand >> 16UL ) & 0x7fffUL );
}
/*-----------------------------------------------------------*/

void prvSRand( UBaseType_t ulSeed )
{
    /* Utility function to seed the pseudo random number generator. */
    ulNextRand = ulSeed;
}
/*-----------------------------------------------------------*/