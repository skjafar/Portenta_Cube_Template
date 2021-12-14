/*
 * FreeRTOSCLIConfig.h
 *
 *  Created on: Jul 19, 2021
 *      Author: Sofian
 */

#ifndef HEADER_FREERTOSCLICONFIG_H_
#define HEADER_FREERTOSCLICONFIG_H_

/* FreeRTOS+CLI definitions. */
/* Dimensions a buffer into which command outputs can be written. The buffer
 * can be declared in the CLI code itself, to allow multiple command consoles to
 * share the same buffer. For example, an application may allow access to the
 * command interpreter by UART and by Ethernet. Sharing a buffer is done purely
 * to save RAM. Note, however, that the command console itself is not re-entrant,
 * so only one command interpreter interface can be used at any one time. For
 * that reason, no attempt at providing mutual exclusion to the buffer is
 * attempted.
 */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 400


#endif /* HEADER_FREERTOSCLICONFIG_H_ */
