/*
 * Rand.h
 *
 * Created: 11/15/2018 8:17:52 PM
 *  Author: Sofian.jafar
 */ 


#ifndef RAND_H_
#define RAND_H_

#include "FreeRTOS.h"

/*
 * Just seeds the simple pseudo random number generator.
 */
void prvSRand( UBaseType_t ulSeed );

/*
 * return the simple pseudo random number.
 */
UBaseType_t uxRand( void );


#endif /* RAND_H_ */