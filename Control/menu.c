#include "myfile.h"
#include "pid.h"

/************************菜单***************************/
uint8_t Key_Num=0;
uint8_t func_index = 0;
uint8_t Cursor=1;
int Key_Flag=0;

/************************内部配置***************************/
typedef struct
{ 
    uint8_t Current;	//当前状态索引号
    uint8_t Up;      		  //
		uint8_t Down;     	//	
    void (*current_operation)(void); //当前状态应该执行的操作
} Key_table;

void (*current_operation_index)(); //定义一个函数指针

Key_table table[100]=
{	
  {0,4,1,(*Boot_animation)},  
  {1,0,2,(*Homepage_1)},
  {2,1,3,(*Homepage_2)},
  {3,2,4,(*Homepage_3)},
  {4,3,0,(*Homepage_4)},

};
void menu_operation()
{	

	if(Key_Num==1)//左
	{         
		func_index = table[func_index].Up;
		OLED_Clear();        		
	}
	
	// === 修改 Key_Num==2 的逻辑 ===
	if(Key_Num==2)  //右 (我们定义这个键为 "启动/停止")
	{   
		Start_Flag = !Start_Flag; // 翻转启动标志 (0 变 1, 1 变 0)
		
		// 我们不再让它翻页了，这个键现在是专用的 "启动" 键
		// func_index = table[func_index].Down; 
		// OLED_Clear();
	}
	
    current_operation_index=table[func_index].current_operation;
    (*current_operation_index)();		
}
/************************显示部分***************************/
//开机动画可以自己设置
void Boot_animation()
{
	OLED_ShowString(40,32,"Hello!!!",OLED_8X16);
}

//第一页
void Homepage_1()	
{
		// === 在第一行添加启动状态显示 ===
		if (Start_Flag)
		{
			OLED_Printf(0,1,OLED_8X16,"  == RUNNING == ");
		}
		else
		{
			OLED_Printf(0,1,OLED_8X16,"  -- STOPPED -- ");
		}
		// === 添加结束 ===
		
		OLED_Printf(0,17,OLED_8X16,"Speed_R:%+05d",Speed_R);
		OLED_Printf(0,34,OLED_8X16,"PWM_L:%+02d",Speed_Out_L);
		OLED_Printf(0,51,OLED_8X16,"PWM_R:%+02d",Speed_Out_R);
		OLED_Update();
}
//第二页
void Homepage_2()	
{
		OLED_Printf(0,0,OLED_8X16,"	s_err:%+04d",sensor_err);		// 保留
		// 下面这行 f_err 已经没用了，删除
		// OLED_Printf(0,17,OLED_8X16,"f_err:%+04d",final_err);
		OLED_Printf(0,34,OLED_8X16,"P_Out:%+04d",Place_Out);	// 保留
		// 下面这行 gz 已经没用了，删除
		// OLED_Printf(0,51,OLED_8X16,"gz:%+03d",GZ);
		OLED_Update();
}
//第三页
void Homepage_3()	
{
			OLED_Printf(0,0,OLED_8X16,"	s:%+06.2f",Location);
  		   // OLED_Printf(0,17,OLED_8X16,"yaw:%+03.2f",yaw);
			OLED_Printf(0,34,OLED_8X16,"FE:%+02d",Element_Flag);
			OLED_Printf(0,51,OLED_8X16,"FR:%+02d",Ring_Flag);
}
//第四页
void Homepage_4()	
{
			OLED_Printf(0,0,OLED_8X16,"	FP:%+03d",Place_Enable);
			OLED_Printf(0,17,OLED_8X16,"FN:%+02d",Noline_Flag);
			OLED_Printf(0,34,OLED_8X16,"FT:%+02d",Ten_Flag);
			OLED_Printf(0,51,OLED_8X16,"FS:%+02d",Stop_Flag);

	
}


