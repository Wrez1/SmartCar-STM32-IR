#include "myfile.h"
#include "pid.h"
#include "Store.h" // 必须包含这个，否则报错

/************************菜单变量***************************/
uint8_t Key_Num = 0;
uint8_t func_index = 1; 
int Key_Flag = 0;
uint8_t Adjust_Index = 0; // 0:无, 1:P, 2:D, 3:Spd, 4:Save
int Stop_CoolDown = 0; 
int Speed_Index = 0; 

/************************外部引用***************************/
extern int Speed_Choice[3]; 
extern int Basic_Speed; 
extern uint8_t Start_Flag;
extern int Speed_PID[3];   
extern float Place_PD[2];  
extern int Speed_L, Speed_R;
extern int sensor_err;
extern float Location;
extern uint8_t Ring_Flag, Noline_Flag, Stop_Flag, Ten_Flag;

// 定义菜单结构
typedef struct { uint8_t Current; void (*current_operation)(void); } Key_table;
void (*current_operation_index)(); 

void Boot_animation(void);  
void Homepage_1(void);
void Homepage_2(void);
void Homepage_3(void);
void Homepage_4(void);
void Menu_Tick(void); // 倒计时函数声明

Key_table table[10] = {   
  {0,(*Boot_animation)},  
  {1,(*Homepage_1)}, 
  {2,(*Homepage_2)}, 
  {3,(*Homepage_3)}, 
  {4,(*Homepage_4)}, 
};

void Boot_animation() { OLED_ShowString(24, 2, "Ready...", OLED_8X16); }

// 给定时器调用的倒计时函数
void Menu_Tick(void)
{
    if (Stop_CoolDown > 0) Stop_CoolDown--;
}

/************************逻辑核心***************************/
void menu_operation()
{   
    // Key 1 (PA0): 翻页 / 切光标
    if(Key_Num == 1)
    {         
        if(Start_Flag == 0) 
        {
             if(func_index == 2) {
                 Adjust_Index++; 
                 // 1:P, 2:D, 3:Spd, 4:Save -> 0 -> 下一页
                 if(Adjust_Index > 4) { 
                     Adjust_Index = 0; 
                     func_index++; 
                 }
             } else {
                 func_index++;
                 if(func_index > 4) func_index = 1;
                 // 刚进第2页自动选中P
                 if(func_index == 2) Adjust_Index = 1; 
             }
        }
        else {
            func_index++;
            if(func_index > 4) func_index = 1; 
        }
        OLED_Clear();
    }
    
    // Key 2 (PA2): 加 (+) / 确认保存
    if(Key_Num == 2 && Start_Flag == 0 && func_index == 2)  
    {   
        if(Adjust_Index == 1) Place_PD[0] += 0.1f;  
        if(Adjust_Index == 2) Place_PD[1] += 0.5f;  
        if(Adjust_Index == 3) Speed_Choice[0] += 1; 
        
        // ★★★ 保存功能 ★★★
        if(Adjust_Index == 4)
        {
            OLED_Clear();
            OLED_ShowString(24, 2, "Saving...", OLED_8X16);
            OLED_Update();
            Store_Save(); // 写Flash
            Delay_ms(800);
            OLED_Clear();
        }
    }

    // Key 3 (PB10): 减 (-)
    if(Key_Num == 3 && Start_Flag == 0 && func_index == 2)  
    {   
        if(Adjust_Index == 1) Place_PD[0] -= 0.1f;
        if(Adjust_Index == 2) Place_PD[1] -= 0.5f;
        if(Adjust_Index == 3) Speed_Choice[0] -= 1;
        
        if(Place_PD[0] < 0) Place_PD[0] = 0;
        if(Place_PD[1] < 0) Place_PD[1] = 0;
        if(Speed_Choice[0] < 0) Speed_Choice[0] = 0;
    }
    
    // Key 4 (PB11): 发车
    if(Key_Num == 4)  
    {   
        if (Stop_CoolDown == 0) {
            Start_Flag = !Start_Flag; 
            Stop_CoolDown = 50;       
            if(Start_Flag) Adjust_Index = 0; 
            OLED_Clear();
        }
    }
    
    if(Start_Flag == 0) Basic_Speed = Speed_Choice[0];
    
    current_operation_index = table[func_index].current_operation;
    (*current_operation_index)();       
}

/************************显示***************************/
void Homepage_1()   
{
    if (Start_Flag) OLED_Printf(0,0,OLED_8X16,"RUNNING...");
    else            OLED_Printf(0,0,OLED_8X16,"STOP Set->Key1");

    OLED_Printf(0,16,OLED_8X16,"L:%+04d R:%+04d", Speed_L, Speed_R);
    OLED_Printf(0,32,OLED_8X16,"BaseSpd:%d", Speed_Choice[0]); 
    OLED_Update();
}

void Homepage_2()   
{
    char cursor = (Start_Flag == 0) ? '>' : ' ';
    
    if(Adjust_Index <= 3) // 显示参数
    {
        OLED_Printf(0,0, OLED_8X16, "%c P:%05.1f", (Adjust_Index==1)?cursor:' ', Place_PD[0]);
        OLED_Printf(0,16,OLED_8X16, "%c D:%05.1f", (Adjust_Index==2)?cursor:' ', Place_PD[1]);
        OLED_Printf(0,32,OLED_8X16, "%c Spd:%d",   (Adjust_Index==3)?cursor:' ', Speed_Choice[0]);
    }
    else // 显示保存界面
    {
        OLED_ShowString(16, 16, " [SAVE OK?] ", OLED_8X16);
        OLED_ShowString(16, 32, " Press Key2 ", OLED_8X16);
    }
    OLED_Update();
}

void Homepage_3()   
{
    OLED_Printf(0,0,OLED_8X16,"Err:%+d", sensor_err);
    OLED_Printf(0,16,OLED_8X16,"Loc:%05.1f", Location);
    OLED_Printf(0,32,OLED_8X16,"Pg3:Sensor"); 
    OLED_Update();
}

void Homepage_4()   
{
    OLED_Printf(0,0,OLED_8X16,"Ri:%d No:%d", Ring_Flag, Noline_Flag);
    OLED_Printf(0,16,OLED_8X16,"Te:%d St:%d", Ten_Flag, Stop_Flag);
    OLED_Printf(0,32,OLED_8X16,"Pg4:Flags"); 
    OLED_Update();
}