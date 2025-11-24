#include "stm32f10x.h"                  // Device header
#include "PWM.h"

void Motor_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	PWM_Init();
}

void Motor_SetPWM_L(int16_t Duty)
{
	if (Duty >= 0)
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_13);   // 12 改成 13
        GPIO_ResetBits(GPIOB, GPIO_Pin_12); // 13 改成 12
        PWM_SetCompare4(Duty);
    }
    else
    {		
        // 原来是 Set 13, Reset 12
        // === 修改为 ===
        GPIO_SetBits(GPIOB, GPIO_Pin_12);   // 13 改成 12
        GPIO_ResetBits(GPIOB, GPIO_Pin_13); // 12 改成 13
        PWM_SetCompare4(-Duty);
    }
}

void Motor_SetPWM_R(int16_t Duty)
{
    if (Duty >= 0)
    {
        // 原来是 Set 15, Reset 14
        // === 修改为 ===
        GPIO_SetBits(GPIOB, GPIO_Pin_14);   // 15 改成 14
        GPIO_ResetBits(GPIOB, GPIO_Pin_15); // 14 改成 15
        PWM_SetCompare1(Duty);
    }
    else
    {		
        // 原来是 Set 14, Reset 15
        // === 修改为 ===
        GPIO_SetBits(GPIOB, GPIO_Pin_15);   // 14 改成 15
        GPIO_ResetBits(GPIOB, GPIO_Pin_14); // 15 改成 14
        PWM_SetCompare1(-Duty);
    }	
}




