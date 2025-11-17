#include "sensor.h"

/*************************************
*函数名称：SENSOR_GPIO_Config
*函数功能：GPIO管脚的配置
*参数：
*说明：
*			
**************************************/
void SENSOR_GPIO_Config(void)
{		
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 使能 GPIOB 和 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOA, ENABLE);


    // 初始化传感器 L2(PA4), L1(PA5), M(PA12)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化传感器 R1(PB0), R2(PB1)
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // === 修改结束 ===
}
/*************************************
*函数名称：digtal
*函数功能：获取X通道数字值
*参数：
*说明：
*			
**************************************/
unsigned char digital(unsigned char channel)//1-ADC_N	  获取X通道数字值
{
	u8 value = 0;
	switch(channel) 
	{
		case 1:  
			if(PAin(4) == 1) value = 1;
			else value = 0;  
			break;  
		case 2: 
			if(PAin(5) == 1) value = 1;
			else value = 0;  
			break;  
		case 3: 
			if(PAin(12) == 1) value = 1;
			else value = 0;  
			break;   
		case 4:  
			if(PBin(0) == 1) value = 1;
			else value = 0;  
			break;   
		case 5:
			if(PBin(1) == 1) value = 1;
			else value = 0;  
			break;
	}
	return value; 
}




