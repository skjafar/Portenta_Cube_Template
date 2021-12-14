/*
 * iperf_task.c
 */

#ifndef IPERF_TASK_H_

#define IPERF_TASK_H_

/* This function will start a task that will handl all IPERF requests from outside.
Call it after the IP-stack is up and running. */

void vIPerfInstall( void );

#endif
