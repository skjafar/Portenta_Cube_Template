/*
 * LED.h
 *
 * Created: 11/16/2018 10:46:15 PM
 *  Author: Sofian.jafar
 */ 


#ifndef LED_H_
#define LED_H_

#include <stdint.h>
#include <stdbool.h>

void task_app_led_create();
void task_app_led_pause(bool pause);
void task_app_led_set_delay(uint32_t index, uint32_t delay0, uint32_t delay1);

#endif /* LED_H_ */
