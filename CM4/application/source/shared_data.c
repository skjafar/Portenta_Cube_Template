/*
 * shared_data.c
 * 
 * Generating data that is shared with the M7 core
 * Runs using Timer3
 * 
 * Created: 12/12/2021 3:47:36 PM
 *  Author: Sofian.jafar
 */ 

#include <math.h>
#include <shared_data.h>
#include <string.h>

#include "application_config.h"
#include "parameters.h"
#include "registers.h"

/* set SEMAPHORE 1 as the semaphore for shared data readings 
   must match definition in CM7 */
#ifndef HSEM_SHARED_DATA
#define HSEM_SHARED_DATA (1U) /* HW semaphore 1*/
#endif

// Current index of shared memory
static volatile uint16_t buff_index __attribute__( ( section( ".shared_data" ), aligned( 16 )) ) = 0;
// Memory to store shared readings
static int16_t  shared_buff[SHARED_MEMORY_BUFFER_LENGTH][SHARED_MEMORY_CHANNELS] __attribute__( ( section( ".shared_data" ), aligned( 16 ) ) );

static void init_shared_memory(void)
{    
    // initialize shared memory buffer
    memset(shared_buff, 0x00, sizeof(shared_buff));

    buff_index = 0;
}

/*
 * Function to be called everytime the timer fires
 * this writes new data intot the shared data buffer
 */
uint32_t ulTimer3Flags;
void TIM3_IRQHandler(void)
{
	ulTimer3Flags = htim3.Instance->SR;
	if( ( ulTimer3Flags & TIM_FLAG_UPDATE ) != 0 )
	{
        __HAL_TIM_CLEAR_FLAG( &htim3, TIM_FLAG_UPDATE );
        // write dummy data into the buffer
        
        for (uint32_t i = 0; i < SHARED_MEMORY_CHANNELS; i++)
        {
            shared_buff[buff_index][i] += (buff_index + 1) * (i + 1);
        }

        /* Take the semaphore
		this is only required for the buff_index variable, as it is the only one really shared between cores */
		while (HAL_HSEM_FastTake(HSEM_SHARED_DATA) == 1){}
		buff_index++;

		// If index is beyond buffer reset the index
		if (buff_index >= SHARED_MEMORY_BUFFER_LENGTH)
		{
			buff_index = 0;
		}
		// Release the semaphore
		HAL_HSEM_Release(HSEM_SHARED_DATA, 0);
	}
}

void init_shared_memory_counter(void)
{
    // initialize memory buffer and index
    init_shared_memory();
   
    // enable timer interrupt for writing new data
	HAL_NVIC_EnableIRQ( TIM3_IRQn );
}

uint16_t read_shared_memory(uint32_t channel)
{
    int32_t     buff_end        = buff_index - 1;
    
    if (buff_end < 0 )
    {
        buff_end = SHARED_MEMORY_BUFFER_LENGTH - 1;
    }
	
    return shared_buff[buff_end][channel];
}
