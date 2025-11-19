#include "myfile.h"
#include "pid.h"

/************************菜单***************************/
uint8_t Key_Num=0;
uint8_t func_index = 0;
uint8_t Cursor=1;
int Key_Flag=0;

int Speed_Select = 0; // 0:低速, 1:中速, 2:高速
// 引用 element.c 里的速度数组 (假设你在 element.c 里定义了 Speed_Choice)
extern int Speed_Choice[3]; 
extern int Basic_Speed; // 引用 pid.c 里的基础速度
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
    // === 逻辑修改：只在第1页允许调速 ===
    
    // 按键 1 (左键/上键)
    if(Key_Num == 1)
    {         
        // 只有在未发车状态下才能调速
        if(Start_Flag == 0)
        {
             Speed_Select++; 
             if(Speed_Select > 2) Speed_Select = 0; // 循环切换 0->1->2->0
             
             // 更新基础速度 (这样发车时就会用新速度)
             // 注意：这里假设 Speed_Choice[0] 是基础速度。
             // 实际上建议直接修改 Basic_Speed，或者修改 Speed_Choice 的当前索引
             Basic_Speed = Speed_Choice[Speed_Select]; 
             
             OLED_Clear(); // 刷新屏幕显示新速度
        }
        
        // 如果你想保留翻页功能，可以写成：发车后按键1翻页，发车前按键1调速。
        // 但为了保险，建议比赛时锁定在第1页。
    }
    
    // 按键 2 (右键/下键) - 发车/停车
    if(Key_Num == 2)  
    {   
        Start_Flag = !Start_Flag; 
        OLED_Clear();
    }
    
    // 强制只显示第1页 (Homepage_1)，因为我们把翻页键占用了
    func_index = 1; 
    current_operation_index = table[func_index].current_operation;
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
    // 显示运行状态
    if (Start_Flag) 
        OLED_Printf(0,0,OLED_8X16,"== RUNNING ==");
    else 
        // 显示当前选中的速度档位
        OLED_Printf(0,0,OLED_8X16,"SET Speed: Lv%d", Speed_Select + 1);

    // 显示实际的左右轮目标速度或编码器值
    OLED_Printf(0,16,OLED_8X16,"L:%+04d R:%+04d", Speed_L, Speed_R);
    OLED_Printf(0,32,OLED_8X16,"Target: %d", Basic_Speed); 
    
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


