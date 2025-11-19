#include "myfile.h"
volatile uint8_t data_ready = 0;

/************************主函数***************************/
int main(void)
{
		Serial_Init();
		Key_Init();
		OLED_Init();
		Timer_Init();
		Encoder_Init(); 
		SENSOR_GPIO_Config(); //循迹引脚初始化
		Motor_Init();			//电机初始化
		PWM_Init();				//占空比定时器1初始化
	
	while (1)
	{		  
			Key_Num = Key_GetNum();  	    
			menu_operation();																					//菜单调用函数
			Serial_Printf("%d,%d,%f,%f\r\n", 1,Speed_R,Location); //串口输出（自己修改变量打印到电脑上来调节速度环）
			OLED_Update();	
	}
}
/************************中断***************************/
void TIM2_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{			
		Control();      //控制函数（最主要的控制都在这里面） 
		data_ready = 1; //陀螺仪控制标志位
		Key_Tick();			//获取按键值，江科大的定时器非阻塞
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}







