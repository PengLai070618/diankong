/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    can.c
  * @brief   This file provides code for the configuration
  *          of the CAN instances.
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
#include "can.h"

/* USER CODE BEGIN 0 */
static uint8_t g_rx_data = 0;        // 存储最近接收到的数据
static uint8_t g_new_data_flag = 0; // 标志位，表示是否有新数据接收

uint8_t CAN_Get_Received_Data(void)
{
    return g_rx_data;
}

void CAN_Clear_Received_Flag(void)
{
    g_new_data_flag = 0;
}

uint8_t CAN_Is_New_Data_Received(void)
{
    return g_new_data_flag;
}


/* USER CODE END 0 */

CAN_HandleTypeDef hcan;

/* CAN init function */
void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 4;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_6TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = ENABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

void HAL_CAN_MspInit(CAN_HandleTypeDef* canHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspInit 0 */

  /* USER CODE END CAN1_MspInit 0 */
    /* CAN1 clock enable */
    __HAL_RCC_CAN1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* CAN1 interrupt Init */
    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspInit 1 */

  /* USER CODE END CAN1_MspInit 1 */
  }
}

void HAL_CAN_MspDeInit(CAN_HandleTypeDef* canHandle)
{

  if(canHandle->Instance==CAN1)
  {
  /* USER CODE BEGIN CAN1_MspDeInit 0 */

  /* USER CODE END CAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_CAN1_CLK_DISABLE();

    /**CAN GPIO Configuration
    PA11     ------> CAN_RX
    PA12     ------> CAN_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11|GPIO_PIN_12);

    /* CAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
  /* USER CODE BEGIN CAN1_MspDeInit 1 */

  /* USER CODE END CAN1_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */
/**
  * @brief  CAN初始化扩展（在MX_CAN_Init之后调用）
  * @note   完成：启动CAN、配置过滤器、开启接收中断
  * @param  无
  * @retval 无
  */
void CAN_Init(void)
{
    /* 1. 启动CAN外设 */
    if (HAL_CAN_Start(&hcan) != HAL_OK)
    {
        Error_Handler();
    }

    /* 2. 配置过滤器（接收所有ID，不过滤） */
    CAN_FilterTypeDef sFilterConfig = {0};
    sFilterConfig.FilterBank = 0;                     // 使用过滤器组0
    sFilterConfig.FilterMode = CAN_FILTERMODE_IDMASK; // 掩码模式
    sFilterConfig.FilterScale = CAN_FILTERSCALE_16BIT;// 16位过滤器

    /* ID和掩码全为0 → 接收所有消息 */
    sFilterConfig.FilterIdHigh = 0x0000;
    sFilterConfig.FilterIdLow = 0x0000;
    sFilterConfig.FilterMaskIdHigh = 0x0000;
    sFilterConfig.FilterMaskIdLow = 0x0000;

    sFilterConfig.FilterFIFOAssignment = CAN_RX_FIFO0; // 关联到FIFO0
    sFilterConfig.FilterActivation = ENABLE;           // 激活过滤器

    if (HAL_CAN_ConfigFilter(&hcan, &sFilterConfig) != HAL_OK)
    {
        Error_Handler();
    }

    /* 3. 开启FIFO0接收中断（有消息到达时触发回调） */
    if (HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
  * @brief  CAN发送数据
  * @param  ID:   目标标准ID（11位）
  * @param  pData: 指向待发送数据的指针
  * @param  DLC:   数据长度（1~8字节）
  * @retval uint8_t: CAN_SEND_SUCCESS 或 CAN_SEND_FAILED
  */
uint8_t CAN_Send_Data(uint32_t ID, uint8_t *pData, uint8_t DLC)
{
    CAN_TxHeaderTypeDef TxHeader = {0};
    uint32_t TxMailbox;

    /* 配置发送消息头 */
    TxHeader.StdId = ID;
    TxHeader.IDE = CAN_ID_STD;        // 标准ID（11位）
    TxHeader.RTR = CAN_RTR_DATA;      // 数据帧
    TxHeader.DLC = DLC;
    TxHeader.TransmitGlobalTime = DISABLE;

    /* 调用HAL库发送函数 */
    if (HAL_CAN_AddTxMessage(&hcan, &TxHeader, pData, &TxMailbox) == HAL_OK)
    {
        return CAN_SEND_SUCCESS;
    }
    return CAN_SEND_FAILED;
}

/**
  * @brief  CAN接收中断回调（HAL库自动调用）
  * @note   当FIFO0收到消息时触发，在中断上下文执行
  * @param  hcan: CAN句柄指针
  * @retval 无
  */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
{
    CAN_RxHeaderTypeDef RxHeader;
    uint8_t rx_data[8] = {0};

    /* 从FIFO0读取消息 */
    if (HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &RxHeader, rx_data) == HAL_OK)
    {
        /* 保存接收到的数据（只取第一个字节） */
        g_rx_data = rx_data[0];
        g_new_data_flag = 1;  // 标记有新数据

        /* ========== 自动回复逻辑（从机模式） ========== */
        /* 如果收到主机发来的0x11，则回复0x22 */
        if (RxHeader.StdId == CAN_MASTER_TX_ID && rx_data[0] == 0x11)
        {
            uint8_t reply_data = 0x22;
            CAN_Send_Data(CAN_SLAVE_TX_ID, &reply_data, 1);
        }
    }
}
/* USER CODE END 1 */

