/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.h
  * @brief   This file contains all the function prototypes for
  *          the can.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CAN_H__
#define __CAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern CAN_HandleTypeDef hcan;

/* USER CODE BEGIN Private defines */
#define CAN_MASTER_TX_ID    0x123   // 主机发送用的ID
#define CAN_SLAVE_RX_ID     0x123   // 从机接收用的ID

#define CAN_SLAVE_TX_ID     0x124   // 从机回复用的ID
#define CAN_MASTER_RX_ID    0x124   // 主机接收用的ID

#define CAN_DATA_LEN        1    // 数据长度为1字节

#define CAN_SEND_SUCCESS    1
#define CAN_SEND_FAILED     0  
/* USER CODE END Private defines */

void MX_CAN_Init(void);

/* USER CODE BEGIN Prototypes */


/**
  * @brief  初始化CAN外设（启动、配置过滤器、开启中断）
  * @param  无
  * @retval 无
  */
void CAN_Init(void);

/**
  * @brief  发送CAN数据
  * @param  ID: 目标ID（标准ID，11位）
  * @param  pData: 指向要发送数据的指针
  * @param  DLC: 数据长度（1~8）
  * @retval uint8_t: CAN_SEND_SUCCESS 或 CAN_SEND_FAILED
  */
uint8_t CAN_Send_Data(uint32_t ID, uint8_t *pData, uint8_t DLC);

/**
  * @brief  获取最近一次接收到的数据（由回调函数更新）
  * @param  无
  * @retval uint8_t: 接收到的数据
  */
uint8_t CAN_Get_Received_Data(void);

/**
  * @brief  清除接收数据标记（读取后调用，防止重复处理）
  * @param  无
  * @retval 无
  */
void CAN_Clear_Received_Flag(void);

/**
  * @brief  检查是否收到了新数据
  * @param  无
  * @retval uint8_t: 1表示有新数据，0表示没有
  */
uint8_t CAN_Is_New_Data_Received(void);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __CAN_H__ */

