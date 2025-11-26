#include "myfile.h"

// 0:无, 1:PA0, 2:PA2, 3:PB10, 4:PB11
uint8_t KeyNum = 0; 

void Key_Init(void)
{
    // 开启 GPIO 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 初始化 PA0, PA1 (Key1, Key2)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化 PB10, PB11 (Key3, Key4)
    // 这里的 PB10 原本是串口TX，现在强制由 GPIO 接管
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

uint8_t Key_GetState(void)
{
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0) == 0) return 1; // Key1
    if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_2) == 0) return 2; // Key2 (+)
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_10) == 0) return 3; // Key3 (-)
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == 0) return 4; // Key4 (Start)
    return 0;
}

void Key_Tick()
{
	static uint8_t Count;
	static uint8_t CurrState, PrevState;
	Count ++;
	if (Count >= 1) // 10ms消抖
	{
		Count = 0;
		PrevState = CurrState;
		CurrState = Key_GetState();
		if (CurrState != 0 && PrevState != CurrState)
		{
			KeyNum = CurrState;
		}
	}
}

uint8_t Key_GetNum(void)
{
	uint8_t Temp;
	if (KeyNum)
	{
		Temp = KeyNum;
		KeyNum = 0;
		return Temp;
	}
	return 0;
}