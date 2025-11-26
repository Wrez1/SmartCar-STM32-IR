#include "myfile.h"
#include "store.h"
volatile uint8_t data_ready = 0;

/************************主函数***************************/
int main(void)
{
		// 1. 系统基础
    //Serial_Init();
    Key_Init();
    OLED_Init();
    
    // 2. 传感器 (输入)
    SENSOR_GPIO_Config(); 
    
    // 3. 电机与动力 (输出)
    // Motor_Init 内部已经调用了 PWM_Init，所以不用再单独调 PWM_Init
    Motor_Init();         
  
    // 4. 编码器 (输入)
    Encoder_Init(); 
    
    // 5. 最后开启定时中断 (控制核心)
    Timer_Init();
	
	Store_Init();
	while (1)
	{		  
			Key_Num = Key_GetNum();  	    
			menu_operation();																					//菜单调用函数
			//Serial_Printf("%d,%d,%f\r\n", 1,Speed_R,Location); //串口输出（自己修改变量打印到电脑上来调节速度环）
			OLED_Update();	
	}
}
/************************中断***************************/
void TIM2_IRQHandler(void)
{

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{			
		Control();      //控制函数（最主要的控制都在这里面） 
		Key_Tick();			//获取按键值，江科大的定时器非阻塞
		Menu_Tick();
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}







