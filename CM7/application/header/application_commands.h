/*
 * application_commands.h
 *
 * Created: 11/8/2018 10:37:13 AM
 *  Author: Sofian.jafar
 */ 


#ifndef APPLICATION_COMMANDS_H_
#define APPLICATION_COMMANDS_H_

#include <stdbool.h>
#include "apptypes.h"

void vRegisterApplicationCLICommands( void );

volatile uint32_t controllingTaskNumber;
volatile uint32_t USBTaskNumber;
volatile uint32_t TCP_CLITaskNumber;

#endif /* APPLICATION_COMMANDS_H_ */