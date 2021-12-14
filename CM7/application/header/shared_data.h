/*
 * shared_data.h
 *
 * Reading data from shared memory
 * 
 * Created: 29/8/2021 2:47:36 PM
 *  Author: Sofian.jafar
 */ 


#ifndef SHARED_DATA_H_
#define SHARED_DATA_H_

#include "main.h"
#include "tim.h"

uint16_t read_shared_memory(uint32_t channel);

#endif /* SHARED_DATA_H_ */
