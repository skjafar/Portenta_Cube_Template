/*
 * shared_data.h
 *
 * Generating data that is shared with the M7 core
 * Runs using Timer3
 * 
 * Created: 12/12/2021 3:47:36 PM
 *  Author: Sofian.jafar
 */ 

#ifndef SHARED_DATA_H_
#define SHARED_DATA_H_

#include "main.h"
#include "tim.h"

void  init_shared_memory_counter(void);

uint16_t read_shared_memory(uint32_t channel);

#endif /* SHARED_DATA_H__ */
