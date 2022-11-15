/*
 * registers.h
 *
 * Created: 3/16/2021 1:51:40 PM
 *  Author: Sofian
 */ 


#ifndef REGISTERS_H_
#define REGISTERS_H_

#include "apptypes.h"

// struct for holding the registers
extern registers_t REGS;

/*************************************
         functions prototypes
 ************************************/
/*
 * change value of register using it's address in the struct
 */
void     vSetRegister(uint32_t address, uint32_t value, reply_status * reply);

/*
 * get value of register using it's address in the struct as uint32_t
 */
uint32_t uGetRegister(uint32_t address, reply_status * reply);

void init_Registers(void);

#endif /* REGISTERS_H_ */