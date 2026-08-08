#include "pwm.h"
#include "tim.h"      

void Servo_SetAngle(uint8_t angle)
{
    uint16_t compare = 500 + (uint16_t)(angle * 2000 / 180);
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, compare);
}