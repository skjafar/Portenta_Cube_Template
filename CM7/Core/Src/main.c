/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "tim.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "application_commands.h"
#include "LED.h"
#include "console.h"

#include "FreeRTOSTimer.h"
#include "Rand.h"
#include "TCPIP.h"
#include "parameters.h"
#include "registers.h"
#include "application_config.h"
#include "shared_data.h"

#include "FreeRTOS.h"
#include "task.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void PeriphCommonClock_Config(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  // Boot up the M4 core as portenta is set to have it off by default using fuses
  HAL_RCCEx_EnableBootCore(RCC_BOOT_C2);

  /* USER CODE END 1 */
/* USER CODE BEGIN Boot_Mode_Sequence_0 */
  int64_t timeout;
/* USER CODE END Boot_Mode_Sequence_0 */

/* USER CODE BEGIN Boot_Mode_Sequence_1 */

  /* Enable I-Cache---------------------------------------------------------*/
  #if (ENABLE_ICACHE == 1)
    SCB_EnableICache();
  #endif

  /* Enable D-Cache---------------------------------------------------------*/
  // enabling this usually breaks Ethernet functionality
  #if (ENABLE_DCACHE == 1)
    SCB_EnableDCache();
  #endif

  /* Wait until CPU2 boots and enters in stop mode or timeout*/
  timeout = 0xFFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) != RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
  Error_Handler();
  }
/* USER CODE END Boot_Mode_Sequence_1 */
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* Enable the portenta oscillator pin */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  GPIO_InitTypeDef  gpio_osc_init_structure;
  gpio_osc_init_structure.Pin = GPIO_PIN_1;
  gpio_osc_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_osc_init_structure.Pull = GPIO_PULLUP;
  gpio_osc_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &gpio_osc_init_structure);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, 1);

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();
/* USER CODE BEGIN Boot_Mode_Sequence_2 */

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();
  /* When system initialization is finished, Cortex-M7 will release Cortex-M4 by means of
	HSEM notification */
  /*HW semaphore Clock enable*/
  __HAL_RCC_HSEM_CLK_ENABLE();
  /*Take HSEM */
  HAL_HSEM_FastTake(HSEM_ID_0);
  /*Release HSEM in order to notify the CPU2(CM4)*/
  HAL_HSEM_Release(HSEM_ID_0,0);
  /* wait until CPU2 wakes up from stop mode */
  timeout = 0xFFFFF;
  while((__HAL_RCC_GET_FLAG(RCC_FLAG_D2CKRDY) == RESET) && (timeout-- > 0));
  if ( timeout < 0 )
  {
	  Error_Handler();
  }
/* USER CODE END Boot_Mode_Sequence_2 */

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* Setup power supply rails 
   * This part enables all required rails on the power maanagement module via I2C*/
  uint8_t data[2];

  // set 3.1V current at 1.2A
  data[0]=0x42;
  data[1]=(1);
  HAL_I2C_Master_Transmit(&hi2c1, 0x08 << 1, data, sizeof(data), 100);

  // LDO3 to 1.2V
  data[0] = 0x52;
  data[1] = 0x9;
  HAL_I2C_Master_Transmit(&hi2c1, 0x08 << 1, data, sizeof(data), 100);
  data[0] = 0x53;
  data[1] = 0xF;
  HAL_I2C_Master_Transmit(&hi2c1, 0x08 << 1, data, sizeof(data), 100);

  // SW2 to 3.3V (SW2_VOLT)
  data[0] = 0x3B;
  data[1] = 0xF;
  HAL_I2C_Master_Transmit(&hi2c1, 0x08 << 1, data, sizeof(data),100);

  // SW1 to 3.0V (SW1_VOLT)
  data[0] = 0x35;
  data[1] = 0xF;
  HAL_I2C_Master_Transmit(&hi2c1, 0x08 << 1, data, sizeof(data),100);

  // wait for the power lines to settle
  HAL_Delay(250);

  /* Enable USB PHY pin */
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  GPIO_InitTypeDef  gpio_usb_init_structure;
  gpio_usb_init_structure.Pin = GPIO_PIN_4;
  gpio_usb_init_structure.Mode = GPIO_MODE_OUTPUT_PP;
  gpio_usb_init_structure.Pull = GPIO_PULLUP;
  gpio_usb_init_structure.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &gpio_usb_init_structure);
  HAL_Delay(10);
  HAL_GPIO_WritePin(GPIOJ, GPIO_PIN_4, 1);

  // Must be called only after 3.1V power supply has setlled and the PJ_4 is pulled high
  MX_USB_DEVICE_Init();
  
  // seed the suedo random number used by TCP
  prvSRand( ( uint32_t ) main );
  
  // initialize parameters and read them from Flash, must be done before any application code
  init_Parameters();

  // initialize registers from parameters, must be done before any application code
  init_Registers();

  // Initialize FreeRTOS+TCP
  TCPIP_init();

  // Task for toggling LED periodically
  task_app_led_create();

  // start Tasks to handle USB serial link
  usb_uart_start();

  // Start the FreeRTOS timers
  startFreeRTOSTaskTimers();
  
  // Register user defined CLI commands
  vRegisterApplicationCLICommands();
  
  /* Start scheduler */
  vTaskStartScheduler();

  /* We should never get here as control is now taken by the scheduler */

  /* Code that follows is used to indicate a failure in the startup of FreeRTOS */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    HAL_GPIO_TogglePin(LED_R_GPIO_Port, LED_R_Pin);
	  HAL_Delay(100);
	  // Not supposed to arrive here
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_SMPS_1V8_SUPPLIES_LDO);
  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}
  /** Macro to configure the PLL clock source
  */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPI2|RCC_PERIPHCLK_SPI5;
  PeriphClkInitStruct.PLL2.PLL2M = 5;
  PeriphClkInitStruct.PLL2.PLL2N = 72;
  PeriphClkInitStruct.PLL2.PLL2P = 3;
  PeriphClkInitStruct.PLL2.PLL2Q = 3;
  PeriphClkInitStruct.PLL2.PLL2R = 2;
  PeriphClkInitStruct.PLL2.PLL2RGE = RCC_PLL2VCIRANGE_2;
  PeriphClkInitStruct.PLL2.PLL2VCOSEL = RCC_PLL2VCOWIDE;
  PeriphClkInitStruct.PLL2.PLL2FRACN = 0;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL2;
  PeriphClkInitStruct.Spi45ClockSelection = RCC_SPI45CLKSOURCE_PLL2;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

void vApplicationMallocFailedHook( void )
{
    /* Called if a call to pvPortMalloc() fails because there is insufficient
    free memory available in the FreeRTOS heap.  pvPortMalloc() is called
    internally by FreeRTOS API functions that create tasks, queues, software
    timers, and semaphores.  The size of the FreeRTOS heap is set by the
    configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
    FreeRTOS_printf( ("Memory Allocation Failed") );
    configASSERT(true);
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook( TaskHandle_t xTask, char *pcTaskName )
{
    /* Called whenever memory is not found by task in its task stack */
    FreeRTOS_printf( ("%s: stack overflow", pcTaskName) );
    configASSERT(true);
}

// TODO: Remove later
void vAssertCalled( const char * pcFile,
                    uint32_t ulLine )
{
    volatile char * pcFileName = ( volatile char * ) pcFile;
    volatile uint32_t ulLineNumber = ulLine;

    ( void ) pcFileName;
    ( void ) ulLineNumber;

    taskDISABLE_INTERRUPTS();
    {
        while( 1 )
        {
        }
    }
    taskENABLE_INTERRUPTS();
}
/*-----------------------------------------------------------*/

/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

