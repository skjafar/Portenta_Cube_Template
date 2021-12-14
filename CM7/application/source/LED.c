/*
 * LED.c
 *
 * Task to toggle LED to tell if system is alive
 * Created: 11/16/2018 10:46:28 PM
 *  Author: Sofian.jafar
 */ 

#include "LED.h"
#include "gpio.h"
#include "main.h"

#include "application_config.h"
#include <FreeRTOS.h>
#include "task.h"

static TaskHandle_t     xCreatedAppLed1Task, xCreatedAppLed2Task;

static volatile uint32_t LED_delay[2][2] = {{LED_STARTUP_PERIOD_0_MS, LED_STARTUP_PERIOD_1_MS},
											{LED_STARTUP_PERIOD_0_MS, LED_STARTUP_PERIOD_1_MS}};

/**
 * OS task that blinks LED1
 */
static void task_led1(void *p)
{
	(void)p;
	for (;;) {
		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		vTaskDelay(portTICK_PERIOD_MS * LED_delay[0][0]);
		HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
		vTaskDelay(portTICK_PERIOD_MS * LED_delay[0][1]);
	}
}

/**
 * OS task that blinks LED2
 */
static void task_led2(void *p)
{
	(void)p;
	for (;;) {
		HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
		vTaskDelay(portTICK_PERIOD_MS * LED_delay[1][0]);
		HAL_GPIO_TogglePin(LED_G_GPIO_Port, LED_G_Pin);
		vTaskDelay(portTICK_PERIOD_MS * LED_delay[1][1]);
	}
}

void task_app_led_pause(bool pause)
{
	if (pause) {
		vTaskSuspend(xCreatedAppLed1Task);
	} else {
		vTaskResume(xCreatedAppLed1Task);
	}
}

/**
 * \brief Create OS task for LED blinking
 */
void task_app_led_create(void)
{
	/* Create task to make led1 blink */
	if (xTaskCreate(task_led1, LED1_TASK_NAME, LED_TASK_STACK_SIZE, NULL, LED_TASK_STACK_PRIORITY, &xCreatedAppLed1Task) != pdPASS) {
		while (1) {
			;
		}
	}

	/* Create task to make led2 blink */
	if (xTaskCreate(task_led2, LED2_TASK_NAME, LED_TASK_STACK_SIZE, NULL, LED_TASK_STACK_PRIORITY, &xCreatedAppLed2Task) != pdPASS) {
		while (1) {
			;
		}
	}
}

/**
 * \brief Set delay between LED toggles
 */
void task_app_led_set_delay(uint32_t index, uint32_t delay0, uint32_t delay1)
{
	uint32_t val;

	if (delay0 > LED_MAX_PERIOD_MS)
	{
		val = LED_MAX_PERIOD_MS;
	}
	else if (delay0 < LED_MIN_PERIOD_MS)
	{
		val = LED_MIN_PERIOD_MS;
	}
	else
	{
		val = delay0;
	}
	LED_delay[index][0] = val;

	if (delay1 > LED_MAX_PERIOD_MS)
	{
		val = LED_MAX_PERIOD_MS;
	}
	else if (delay1 < LED_MIN_PERIOD_MS)
	{
		val = LED_MIN_PERIOD_MS;
	}
	else
	{
		val = delay1;
	}
	LED_delay[index][1] = val;
}