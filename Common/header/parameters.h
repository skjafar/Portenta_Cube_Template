/*
 * flash.h
 *
 * Created: 2/7/2021 3:29:01 PM
 *  Author: Sofian.jafar
 */ 

#ifndef FLASH_H_
#define FLASH_H_

#include "apptypes.h"

// two lists of parameters, one is used online, the other is for reading and storing paramters in flash
extern parameters_t PARS;

/*************************************
    Static functions prototypes
 ************************************/
/*
 * change value of parameter using it's address in the struct
 */
void     vSetParameter(uint32_t address, uint32_t value, reply_status * reply);

/*
 * get value of parameter using it's address in the struct as uint32_t
 */
uint32_t uGetParameter(uint32_t address, reply_status * reply);


/*
 * write parameters struct to flash
 */
void    vWritetoFlash();

/*
 * read parameters struct from flash
 */
void    vReadFromFlash();

void    init_Parameters(void);

#endif /* FLASH_H_ */