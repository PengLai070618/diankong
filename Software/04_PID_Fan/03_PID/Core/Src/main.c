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
#include "cmsis_os.h"
#include "adc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <string.h>
#include <stdio.h>
/* ===== 电机控制函数 ===== */
void Motor_Forward(uint16_t pwm_value)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);   // AIN1 = 高
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // AIN2 = 低
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pwm_value);
}

void Motor_Reverse(uint16_t pwm_value)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET); // AIN1 = 低
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);   // AIN2 = 高
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, pwm_value);
}

void Motor_Stop(void)
{
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
    __HAL_TIM_SetCompare(&htim3, TIM_CHANNEL_1, 0);
}

/* ===== 编码器读取函数 ===== */
int32_t Encoder_GetCount(void)
{
    return (int32_t)__HAL_TIM_GET_COUNTER(&htim2);
}

void Encoder_Reset(void)
{
    __HAL_TIM_SET_COUNTER(&htim2, 0);
}

/* ===== ADC读取函数 ===== */
uint16_t ADC_Read(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0;
}

/* ===== 串口发送函数（封装） ===== */
void UART_Send_String(char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/* ===== FreeRTOS 任务函数声明 ===== */
void Task_ADC(void *argument);
void Task_PID(void *argument);

/* ===== PID 结构体 ===== */
typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float target;
    float feedback;
    float last_error;
    float prev_error;
    float integral;
    float output;
    float output_limit;
} PID_Handle_t;

/* ===== PID 初始化函数 ===== */
void PID_Init(PID_Handle_t *pid, float Kp, float Ki, float Kd, float output_limit)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->target = 0;
    pid->feedback = 0;
    pid->last_error = 0;
    pid->prev_error = 0;
    pid->integral = 0;
    pid->output = 0;
    pid->output_limit = output_limit;
}

/* ===== 增量式 PID 计算函数 ===== */
float PID_Update(PID_Handle_t *pid, float target, float feedback)
{
    pid->target = target;
    pid->feedback = feedback;
    
    float error = target - feedback;
    
    // 增量式 PID
    float delta_u = pid->Kp * (error - pid->last_error) 
                  + pid->Ki * error 
                  + pid->Kd * (error - 2 * pid->last_error + pid->prev_error);
    
    pid->prev_error = pid->last_error;
    pid->last_error = error;
    
    // 累加输出并限幅
    pid->output += delta_u;
    if (pid->output > pid->output_limit) pid->output = pid->output_limit;
    if (pid->output < -pid->output_limit) pid->output = -pid->output_limit;
    
    return pid->output;
}


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
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
/* 启动 PWM 定时器 */
HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);

/* 启动编码器定时器*/
HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

/* STBY 使能（如果 STBY 接的是 GPIO，需要拉高） */
// HAL_GPIO_WritePin(STBY_GPIO_Port, STBY_Pin, GPIO_PIN_SET);

/* 打印启动信息 */
char msg[] = "Motor Control Started!\r\n";
UART_Send_String(msg);

/* 创建 FreeRTOS 任务 */
osThreadAttr_t attr_ADC = { .name = "TaskADC", .stack_size = 256, .priority = osPriorityNormal };
osThreadAttr_t attr_PID = { .name = "TaskPID", .stack_size = 256, .priority = osPriorityHigh };

osThreadNew(Task_ADC, NULL, &attr_ADC);
osThreadNew(Task_PID, NULL, &attr_PID);

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();  /* Call init function for freertos objects (in cmsis_os2.c) */
  MX_FREERTOS_Init();

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV4;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
/* ===== ADC 读取任务（每100ms执行一次） ===== */
void Task_ADC(void *argument)
{
    for(;;)
    {
        uint16_t adc_value = ADC_Read();
        
        char buf[30];
        sprintf(buf, "ADC: %d\r\n", adc_value);
        HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
        
        osDelay(100);
    }
}

/* ===== PID 控制任务（每100ms执行一次） ===== */
void Task_PID(void *argument)
{
    // 定义 PID 句柄
    PID_Handle_t pid_speed;
    PID_Init(&pid_speed, 0.5f, 0.1f, 0.0f, 1000.0f);  // 初始参数，后续调优
    
    // 速度转换常数
   float speed_scale = 1.0f;
    
    for(;;)
    {
        // 1. 读取电位器 → 目标速度（0~4095 映射到 0~1000）
        uint16_t adc_value = ADC_Read();
        float target_speed = (float)adc_value / 4095.0f * 1000.0f;
        
        // 2. 读取编码器 → 实际速度（用差分方式计算）
        static int32_t last_encoder = 0;
        int32_t current_encoder = Encoder_GetCount();
        float current_speed = (float)(current_encoder - last_encoder) * speed_scale;
        last_encoder = current_encoder;
        
        // 3. PID 计算
        float pid_output = PID_Update(&pid_speed, target_speed, current_speed);
        
        // 4. 应用到电机
        if (pid_output > 0) {
            Motor_Forward((uint16_t)(pid_output > 1000 ? 1000 : pid_output));
        } else if (pid_output < 0) {
            Motor_Reverse((uint16_t)(-pid_output > 1000 ? 1000 : -pid_output));
        } else {
            Motor_Stop();
        }
        
        // 5. 发送到 VOFA+（用于显示曲线）
        char buf[80];
        sprintf(buf, "%.1f,%.1f\r\n", target_speed, current_speed);
        HAL_UART_Transmit(&huart1, (uint8_t*)buf, strlen(buf), HAL_MAX_DELAY);
        
        osDelay(10);  // 10ms 控制周期
    }
}
/* USER CODE END 4 */

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1)
  {
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
