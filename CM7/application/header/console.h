/*
 * console.h
 *
 * Task for handling all UART debug port
 * 
 * Created: 11/17/2018 12:31:31 AM
 *  Author: Sofian.jafar
 */ 


#ifndef CONSOLE_H_
#define CONSOLE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "FreeRTOS_CLI.h"
#include "FreeRTOS_Sockets.h"
#include "FreeRTOS_IP.h"
#include "NetworkInterface.h"
#include "NetworkBufferManagement.h"

TaskHandle_t xCreatedAppUSBCLITask;
TaskHandle_t xCreatedAppUSBInfoTask;

void usb_uart_start(void);

#endif /* CONSOLE_H_ */