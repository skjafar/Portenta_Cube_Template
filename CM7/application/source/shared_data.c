/*
 * shared_data.c
 * 
 * Reading shared data from M4 core
 * 
 * Created: 12/12/2021 3:47:36 PM
 *  Author: Sofian.jafar
 */ 

#include <math.h>
#include <shared_data.h>
#include <shared_data.h>
#include <string.h>

#include "application_config.h"
#include "parameters.h"
#include "registers.h"

/* set SEMAPHORE 1 as the semaphore for shared data readings 
   must match definition in CM4 */
#ifndef HSEM_SHARED_DATA
#define HSEM_SHARED_DATA (1U) /* HW semaphore 1*/
#endif

// Current index of shared memory
static volatile uint16_t buff_index __attribute__( ( section( ".shared_data" ), aligned( 16 )) ) = 0;
// Memory to store shared readings
static int16_t  shared_buff[SHARED_MEMORY_BUFFER_LENGTH][SHARED_MEMORY_CHANNELS] __attribute__( ( section( ".shared_data" ), aligned( 16 ) ) );

uint16_t read_shared_memory(uint32_t channel)
{
    // Take the semaphore for accessing buff_index
    while (HAL_HSEM_FastTake(HSEM_SHARED_DATA) == 1){}
    int32_t     buff_end        = buff_index - 1;
    // Release the semaphore
    HAL_HSEM_Release(HSEM_SHARED_DATA, 0);
    
    if (buff_end < 0 )
    {
        buff_end = SHARED_MEMORY_BUFFER_LENGTH - 1;
    }
	
    return shared_buff[buff_end][channel];
}
