/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "can.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "can.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

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

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 烧录主机取消注释下面这行，烧录从机注释掉它
//#define MASTER
 #define SLAVE

uint8_t last_key_state = 0;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
 
// 初始化CAN
CAN_Init();
 // 串口打印启动信息
char msg[] = "CAN Test Started!\r\n";
HAL_UART_Transmit(&huart1, (uint8_t*)msg, sizeof(msg)-1, HAL_MAX_DELAY);

#if defined(MASTER)
    char msg2[] = "Mode: MASTER\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)msg2, sizeof(msg2)-1, HAL_MAX_DELAY);
#elif defined(SLAVE)
    char msg2[] = "Mode: SLAVE\r\n";
    HAL_UART_Transmit(&huart1, (uint8_t*)msg2, sizeof(msg2)-1, HAL_MAX_DELAY);
#endif
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {    
    #if defined(MASTER)
    uint8_t current_key = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
    
    // 检测下降沿：按键按下的瞬间（高->低）
    if (last_key_state == GPIO_PIN_SET && current_key == GPIO_PIN_RESET)
    {
        
        uint8_t tx_data = 0x11;
        if (CAN_Send_Data(CAN_MASTER_TX_ID, &tx_data, 1) == CAN_SEND_SUCCESS)
        {
            char sent_msg[] = "Sent: 0x11\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)sent_msg, sizeof(sent_msg)-1, HAL_MAX_DELAY);
        }
        else
        {
            char fail_msg[] = "Send failed!\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)fail_msg, sizeof(fail_msg)-1, HAL_MAX_DELAY);
        }
        
        // 更新 last_key_state
        last_key_state = current_key;
        
        // 等待按键完全松开（防止抖动）
        while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET)
        {
            HAL_Delay(50);  // 等待按键释放（低电平保持时等待）
        }
    }
    
    // 在循环末尾更新 last_key_state
    last_key_state = current_key;

    // 接收检查（保持不变）
    if (CAN_Is_New_Data_Received())
    {
        uint8_t rx = CAN_Get_Received_Data();
        if (rx == 0x22)
        {
            char ack_msg[] = "Received ACK: 0x22\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)ack_msg, sizeof(ack_msg)-1, HAL_MAX_DELAY);
        }
        CAN_Clear_Received_Flag();
    }


    #elif defined(SLAVE)
    /* 从机逻辑 */
     if (CAN_Is_New_Data_Received())
    {
        uint8_t rx = CAN_Get_Received_Data();
        if (rx == 0x11)
        {
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            char rx_msg[] = "Received: 0x11, LED toggled\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)rx_msg, sizeof(rx_msg)-1, HAL_MAX_DELAY);

            // 回复 0x22 
            uint8_t reply = 0x22;
            if (CAN_Send_Data(CAN_SLAVE_TX_ID, &reply, 1) == CAN_SEND_SUCCESS)
            {
                char ack_sent[] = "Reply 0x22 sent\r\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)ack_sent, sizeof(ack_sent)-1, HAL_MAX_DELAY);
            }
            else
            {
                char ack_fail[] = "Reply 0x22 failed\r\n";
                HAL_UART_Transmit(&huart1, (uint8_t*)ack_fail, sizeof(ack_fail)-1, HAL_MAX_DELAY);
            }
        }
        CAN_Clear_Received_Flag();
    }
#endif

    HAL_Delay(100);  // 轮询间隔

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
#ifdef USE_FULL_ASSERT
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
