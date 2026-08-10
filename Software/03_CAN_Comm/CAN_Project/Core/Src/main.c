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
#include "stdio.h"  // 用于printf重定向
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

/* ========== 定义主从模式切换宏 ========== */
// 烧录主机板时，取消下面这行的注释
// #define MASTER

// 烧录从机板时，取消下面这行的注释
#define SLAVE

/* ========== 包含自定义头文件 ========== */
#include "can.h"

/* ========== 全局变量 ========== */
uint8_t last_key_state = 0;  // 用于按键边沿检测（防误触发）

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

/* 1. 调用CAN自定义初始化（启动+过滤器+中断） */
CAN_Init();

/* 2. 打印启动信息（通过串口） */
printf("CAN Test Started!\r\n");

#if defined(MASTER)
    printf("Mode: MASTER\r\n");
#elif defined(SLAVE)
    printf("Mode: SLAVE\r\n");
#endif

  /* USER CODE END 2 */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

   /* USER CODE BEGIN 3 */

#if defined(MASTER)
    /* ========== 主机模式逻辑 ========== */
    // 检测按键（PA0），按下时发送 0x11
    uint8_t current_key = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0); 
    
    // 边沿检测：检测到下降沿（按键按下）才触发
    if (last_key_state == GPIO_PIN_SET && current_key == GPIO_PIN_RESET)
    {
        uint8_t tx_data = 0x11;
        if (CAN_Send_Data(CAN_MASTER_TX_ID, &tx_data, 1) == CAN_SEND_SUCCESS)
        {
            printf("Sent: 0x11\r\n");
        }
        else
        {
            printf("Send failed!\r\n");
        }
        HAL_Delay(100);  // 简单防抖
    }
    last_key_state = current_key;

    // 检查是否收到从机回复的 0x22
    if (CAN_Is_New_Data_Received())
    {
        uint8_t rx = CAN_Get_Received_Data();
        if (rx == 0x22)
        {
            printf("Received ACK: 0x22\r\n");
        }
        CAN_Clear_Received_Flag();
    }

#elif defined(SLAVE)
    /* ========== 从机模式逻辑 ========== */
    // 检查是否收到主机发来的 0x11（在CAN回调中已自动回复）
    if (CAN_Is_New_Data_Received())
    {
        uint8_t rx = CAN_Get_Received_Data();
        if (rx == 0x11)
        {
            // 翻转板载LED（PC13）
            HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
            printf("Received: 0x11, LED toggled\r\n");
        }
        CAN_Clear_Received_Flag();
    }

#endif

    HAL_Delay(10);  // 主循环轮询间隔

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
int fputc(int ch, FILE *f)  // 重定向printf到串口
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
    return ch;
}
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
