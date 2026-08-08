#include "vofa.h"
#include "uart.h"
#include"usart.h"
#include <string.h>

// JustFloat 协议：把多个 float 按小端序打包成字节流，末尾加 4 字节帧尾
void Vofa_JustFloat_Send(float *data, uint8_t num)
{
    uint8_t byteBuffer[256];   // 足够容纳数据 + 4 字节帧尾
    uint16_t index = 0;

    // 把每个 float 拆成 4 个字节（小端序，和 STM32 默认一致）
    for (uint8_t i = 0; i < num; i++)
    {
        uint8_t *p = (uint8_t *)&data[i];
        byteBuffer[index++] = p[0];
        byteBuffer[index++] = p[1];
        byteBuffer[index++] = p[2];
        byteBuffer[index++] = p[3];
    }

    // 加 JustFloat 帧尾：0x00 0x00 0x80 0x7F
    byteBuffer[index++] = 0x00;
    byteBuffer[index++] = 0x00;
    byteBuffer[index++] = 0x80;
    byteBuffer[index++] = 0x7F;

    // 通过串口发送
    HAL_UART_Transmit(&huart1, byteBuffer, index, 100);
}